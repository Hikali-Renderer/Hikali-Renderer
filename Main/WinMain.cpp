// Renderer/Main/WinMain.cpp
// Hikali 渲染器入口：接收启动器（Launcher）写入的 RendererConfig 初始化参数
//
// 启动器流程：写 renderer_config.json → CreateProcess 传 "--config <路径>"
// 本文件流程：解析命令行 → 读取 JSON → 校验并规格化参数 → 建窗口 → 消息循环
//
// 注意：
//   - 命令行统一从 GetCommandLineW 取，不用 WinMain 的 lpCmdLine：
//     console 变体（main.cpp）转发的是含 exe 名的整条命令行，与 lpCmdLine 语义不同，
//     从 GetCommandLineW 取则两种子系统下行为一致
//   - 配置文件路径可能含中文（%TEMP% 位于中文用户名下），全程走宽字符 API

#include <windows.h>
#include <crtdbg.h>
#include <shellapi.h>       // CommandLineToArgvW

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "Config.h"

// 正常由 BuildSettings 传入（$<BOOL:...> 展开为 0/1），此处兜底以便单文件编译
#if !defined(D3D12_SUPPORTED)
#   define D3D12_SUPPORTED 0
#endif
#if !defined(VULKAN_SUPPORTED)
#   define VULKAN_SUPPORTED 0
#endif

namespace
{

// ---------- 文本与日志 ----------
std::wstring Format(_In_z_ _Printf_format_string_ const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    const int length = _vscwprintf(format, args);
    va_end(args);

    if (length <= 0)
        return {};

    std::wstring text((size_t)length, L'\0');
    va_start(args, format);
    vswprintf_s(text.data(), (size_t)length + 1, format, args);   // +1 留给结尾空字符
    va_end(args);
    return text;
}

// Config.h / nlohmann 的错误串是 UTF-8 窄字符，不能靠 %hs 转宽：
// 那条路径按当前 locale 解码，中文会变乱码
std::wstring Utf8ToWide(const std::string& text)
{
    if (text.empty())
        return {};

    const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
    if (length <= 0)
        return {};

    std::wstring wide((size_t)length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), wide.data(), length);
    return wide;
}

bool HasStdOutput()
{
    const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    return out != nullptr && out != INVALID_HANDLE_VALUE;
}

// 始终写调试输出；另有 stdout 时（console 变体或从终端启动）再打一份。
// 不用 wprintf：它按当前 locale 转窄字符，中文在默认 "C" locale 下会丢字
void LogLine(const std::wstring& text)
{
    const std::wstring line = text + L"\n";
    OutputDebugStringW(line.c_str());

    const HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == nullptr || out == INVALID_HANDLE_VALUE)
        return;

    DWORD consoleMode = 0;
    DWORD written     = 0;
    if (GetConsoleMode(out, &consoleMode))
    {
        // 真控制台：直接写宽字符，不受控制台代码页影响
        WriteConsoleW(out, line.c_str(), (DWORD)line.size(), &written, nullptr);
        return;
    }

    // 被重定向到文件 / 管道：按 UTF-8 输出
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.size(),
                                          nullptr, 0, nullptr, nullptr);
    if (bytes <= 0)
        return;

    std::string utf8((size_t)bytes, '\0');
    WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.size(),
                        utf8.data(), bytes, nullptr, nullptr);
    WriteFile(out, utf8.data(), (DWORD)utf8.size(), &written, nullptr);
}

// GUI 子系统下（启动器用 CreateProcess 拉起）没有控制台，
// 错误必须弹窗，否则进程静默退出、用户无从判断
void ReportFatal(const std::wstring& message)
{
    LogLine(L"[错误] " + message);
    if (!HasStdOutput())
        MessageBoxW(nullptr, message.c_str(), L"Hikali 渲染器", MB_OK | MB_ICONERROR);
}

std::wstring ExecutableName()
{
    wchar_t path[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0)
        return L"Hikali.exe";
    return std::filesystem::path(path).filename().wstring();
}

std::wstring UsageText()
{
    return L"用法：" + ExecutableName() + L" [--config <配置文件路径>] [--help]\n"
           L"  --config, -c   渲染器初始化参数 JSON（由启动器写出）；省略时使用内置默认值\n"
           L"  --help,   -h   显示本帮助";
}

// ---------- 命令行解析 ----------
constexpr wchar_t kConfigPrefix[] = L"--config=";

struct CommandLineOptions
{
    std::filesystem::path configPath;          // --config 的值，空表示未指定
    bool                  showHelp = false;
    std::wstring          error;               // 非空表示解析失败
};

CommandLineOptions ParseCommandLine()
{
    CommandLineOptions opts;

    int     argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr)
    {
        opts.error = Format(L"解析命令行失败（错误码 %lu）。", GetLastError());
        return opts;
    }

    for (int i = 1; i < argc && opts.error.empty(); ++i)      // argv[0] 是 exe 路径，跳过
    {
        const std::wstring arg = argv[i];

        if (arg == L"--help" || arg == L"-h" || arg == L"/?")
        {
            opts.showHelp = true;
        }
        else if (arg == L"--config" || arg == L"-c")
        {
            if (i + 1 >= argc)
                opts.error = arg + L" 缺少配置文件路径。";
            else
                opts.configPath = argv[++i];
        }
        else if (arg.rfind(kConfigPrefix, 0) == 0)            // 兼容 --config=<路径> 写法
        {
            const std::wstring value = arg.substr(std::size(kConfigPrefix) - 1);
            if (value.empty())
                opts.error = L"--config= 缺少配置文件路径。";
            else
                opts.configPath = value;
        }
        else
        {
            opts.error = L"无法识别的参数：" + arg;
        }
    }

    LocalFree(argv);
    return opts;
}

// ---------- 参数校验与规格化 ----------
// 与启动器滑条范围保持一致，避免启动器放行的值在此被判非法
constexpr int16_t kMinWindowWidth  = 640;
constexpr int16_t kMaxWindowWidth  = 3840;
constexpr int16_t kMinWindowHeight = 480;
constexpr int16_t kMaxWindowHeight = 2160;

const wchar_t* BackendName(GraphicsBackend backend)
{
    switch (backend)
    {
    case GraphicsBackend::D3D12:  return L"D3D12";
    case GraphicsBackend::Vulkan: return L"Vulkan";
    default:                      return L"<未知>";
    }
}

bool IsBackendCompiledIn(GraphicsBackend backend)
{
    switch (backend)
    {
    case GraphicsBackend::D3D12:  return D3D12_SUPPORTED != 0;
    case GraphicsBackend::Vulkan: return VULKAN_SUPPORTED != 0;
    default:                      return false;
    }
}

// 返回本次构建实际可用的后端；两个都没编进来时返回 false
bool PickAvailableBackend(GraphicsBackend& outBackend)
{
    if (D3D12_SUPPORTED)
    {
        outBackend = GraphicsBackend::D3D12;
        return true;
    }
    if (VULKAN_SUPPORTED)
    {
        outBackend = GraphicsBackend::Vulkan;
        return true;
    }
    return false;
}

// 就地修正越界 / 不可用的参数，把每次修正记入 notes 供日志说明。
// 配置文件可能来自旧版本启动器或被手工改过，不能直接信任
void NormalizeConfig(RendererConfig& config, std::vector<std::wstring>& notes)
{
    if (config.backend != GraphicsBackend::D3D12 && config.backend != GraphicsBackend::Vulkan)
    {
        notes.push_back(Format(L"图形后端取值非法（%d），回退为 D3D12。", (int)config.backend));
        config.backend = GraphicsBackend::D3D12;
    }

    if (!IsBackendCompiledIn(config.backend))
    {
        GraphicsBackend fallback = config.backend;
        if (PickAvailableBackend(fallback))
        {
            notes.push_back(Format(L"本次构建未启用 %ls 后端，回退为 %ls。",
                                   BackendName(config.backend), BackendName(fallback)));
            config.backend = fallback;
        }
    }

    const int16_t width  = std::clamp(config.windowWidth,  kMinWindowWidth,  kMaxWindowWidth);
    const int16_t height = std::clamp(config.windowHeight, kMinWindowHeight, kMaxWindowHeight);
    if (width != config.windowWidth || height != config.windowHeight)
    {
        notes.push_back(Format(L"窗口尺寸 %dx%d 超出范围，收敛为 %dx%d。",
                               (int)config.windowWidth, (int)config.windowHeight,
                               (int)width, (int)height));
        config.windowWidth  = width;
        config.windowHeight = height;
    }

    if (config.fullScreen && !config.vsync)
        notes.push_back(L"全屏且关闭垂直同步，可能出现画面撕裂。");
}

std::wstring DescribeConfig(const RendererConfig& config)
{
    return Format(L"生效的渲染器参数：\n"
                  L"  图形后端   : %ls\n"
                  L"  窗口尺寸   : %d x %d\n"
                  L"  垂直同步   : %ls\n"
                  L"  全屏       : %ls\n"
                  L"  调试层     : %ls",
                  BackendName(config.backend),
                  (int)config.windowWidth, (int)config.windowHeight,
                  config.vsync       ? L"开" : L"关",
                  config.fullScreen  ? L"开" : L"关",
                  config.debugLayers ? L"开" : L"关");
}

// ---------- 窗口 ----------
// 后端尚未接入，窗口内容先用 GDI 画出生效参数，
// 这样启动器点下去能立刻看到参数确实传到了渲染器
std::wstring g_windowInfoText;

LRESULT CALLBACK RendererWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)         // 全屏是无边框窗口，没有关闭按钮，留 Esc 退出
        {
            DestroyWindow(hWnd);
            return 0;
        }
        break;

    case WM_ERASEBKGND:
        return 1;                        // 背景在 WM_PAINT 里一次画完，避免闪烁

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        const HDC   hdc = BeginPaint(hWnd, &ps);

        RECT client{};
        GetClientRect(hWnd, &client);

        const HBRUSH background = CreateSolidBrush(RGB(16, 16, 20));
        FillRect(hdc, &client, background);
        DeleteObject(background);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(225, 225, 232));
        const HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);   // 系统 UI 字体，含中文字形
        const HGDIOBJ oldFont = SelectObject(hdc, font);

        // 先量高度，再垂直居中
        RECT text = client;
        DrawTextW(hdc, g_windowInfoText.c_str(), -1, &text, DT_CENTER | DT_CALCRECT);
        const LONG height = text.bottom - text.top;
        RECT drawAt = client;
        drawAt.top = (client.bottom - client.top - height) / 2;
        DrawTextW(hdc, g_windowInfoText.c_str(), -1, &drawAt, DT_CENTER);

        SelectObject(hdc, oldFont);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// 按 config 建窗口。fullScreen 走无边框全屏（覆盖主显示器）：
// 独占全屏要等交换链就绪，此处先用无边框，行为上等价且不会丢失显示模式
HWND CreateRendererWindow(HINSTANCE hInstance, const RendererConfig& config)
{
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = RendererWndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);   // IDC_ARROW 是 ANSI 资源宏，需转宽
    wc.lpszClassName = L"HikaliRendererClass";
    if (RegisterClassExW(&wc) == 0)
        return nullptr;

    const std::wstring title = Format(L"Hikali 渲染器 — %ls — %d x %d",
                                      BackendName(config.backend),
                                      (int)config.windowWidth, (int)config.windowHeight);

    if (config.fullScreen)
        return CreateWindowExW(0, wc.lpszClassName, title.c_str(), WS_POPUP,
                               0, 0,
                               GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                               nullptr, nullptr, hInstance, nullptr);

    // 窗口模式：config 给的是客户区尺寸，换算成含边框的整体尺寸
    RECT rect{ 0, 0, (LONG)config.windowWidth, (LONG)config.windowHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    return CreateWindowExW(0, wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           rect.right - rect.left, rect.bottom - rect.top,
                           nullptr, nullptr, hInstance, nullptr);
}

int RunMessageLoop()
{
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

}   // namespace

// ---------- 主入口 ----------
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
#if defined(_DEBUG) || defined(DEBUG)
    // _CRTDBG_ALLOC_MEM_DF：启用调试堆分配
    // _CRTDBG_LEAK_CHECK_DF：程序退出时自动检查内存泄漏
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const CommandLineOptions opts = ParseCommandLine();

    if (!opts.error.empty())
    {
        ReportFatal(opts.error + L"\n\n" + UsageText());
        return 1;
    }

    if (opts.showHelp)
    {
        const std::wstring usage = UsageText();
        if (HasStdOutput())
            LogLine(usage);
        else
            MessageBoxW(nullptr, usage.c_str(), L"Hikali 渲染器", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // ---------- 读取启动器写入的初始化参数 ----------
    RendererConfig config;      // 先持默认值，未指定 --config 时直接沿用

    if (opts.configPath.empty())
    {
        LogLine(L"未指定 --config，使用内置默认参数。");
    }
    else
    {
        std::string loadError;
        if (!LoadRendererConfigFromFile(opts.configPath, config, &loadError))
        {
            // --config 是显式请求，读不出来就不猜：报错退出，避免用错参数跑起来
            ReportFatal(Format(L"读取配置文件失败：\n%ls\n\n原因：%ls",
                               opts.configPath.c_str(), Utf8ToWide(loadError).c_str()));
            return 2;
        }
        LogLine(L"已读取配置文件：" + opts.configPath.wstring());
    }

    std::vector<std::wstring> notes;
    NormalizeConfig(config, notes);
    for (const std::wstring& note : notes)
        LogLine(L"[提示] " + note);

    LogLine(DescribeConfig(config));

    // 两个后端都没编进来时无法继续（正常由 Renderer/CMakeLists.txt 的检查拦住）
    if (!IsBackendCompiledIn(config.backend))
    {
        ReportFatal(L"本次构建未启用任何图形后端，无法初始化渲染器。");
        return 3;
    }

    // ---------- 按参数建窗口并运行 ----------
    // 声明进程 DPI 感知，否则高分屏下系统会缩放，config 给的像素尺寸拿不准。
    // 失败不致命（多为清单已声明过），但要记下来，免得尺寸对不上时无从排查
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
        LogLine(Format(L"[提示] 设置 DPI 感知失败（错误码 %lu），窗口尺寸可能被系统缩放。",
                       GetLastError()));

    g_windowInfoText = DescribeConfig(config) +
                       L"\n\n（图形后端尚未接入，当前仅验证启动参数）\n按 Esc 或关闭窗口退出";

    const HWND hwnd = CreateRendererWindow(hInstance, config);
    if (hwnd == nullptr)
    {
        ReportFatal(Format(L"创建窗口失败（错误码 %lu）。", GetLastError()));
        return 4;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 从进程内部量客户区（不受调用方 DPI 虚拟化影响），确认参数确实落到了窗口上
    RECT client{};
    GetClientRect(hwnd, &client);
    LogLine(Format(L"渲染器窗口已创建：客户区 %ld x %ld（DPI %u），进入消息循环。",
                   client.right - client.left, client.bottom - client.top,
                   GetDpiForWindow(hwnd)));

    // TODO: 在此用 config.backend 创建设备与交换链（vsync / debugLayers 在设备初始化时生效），
    //       并把 GetMessage 换成 PeekMessage 驱动的渲染循环
    const int exitCode = RunMessageLoop();
    LogLine(L"渲染器已退出。");
    return exitCode;
}

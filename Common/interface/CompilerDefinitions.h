#pragma once

#ifdef _MSC_VER
#    if _MSC_VER >= 1917
#        define NODISCARD [[nodiscard]]
#    else
#        define NODISCARD
#    endif
#endif // _MSC_VER

#ifdef __clang__
#    if __has_feature(cxx_attributes)
#        define NODISCARD [[nodiscard]]
#    else
#        define NODISCARD
#    endif
#endif // __clang__

#ifdef __GNUC__
#    if __has_cpp_attribute(nodiscard)
#        define NODISCARD [[nodiscard]]
#    else
#        define NODISCARD
#    endif
#endif // __GNUC__
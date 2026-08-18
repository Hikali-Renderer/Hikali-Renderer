/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include "InterfaceID.h"

namespace Hikali
{
    typedef int32_t ReferenceCounterValueType;

    /// Base interface for a reference counter object that stores the number of strong and
    /// weak references and the pointer to the object. It is necessary to separate reference
    /// counters from the object to support weak pointers.
    class IReferenceCounters
    {
    public:
        /// Increments the number of strong references by 1.

        /// \return The number of strong references after incrementing the counter.
        /// \remark The counter update is thread-safe, but the caller must already
        ///         own a strong reference or otherwise externally guarantee object
        ///         lifetime. AddStrongRef() must not be used to promote a weak
        ///         reference because another thread may be releasing the final strong
        ///         reference at the same time. Use QueryObject() or RefCntWeakPtr::Lock()
        ///         for weak-to-strong promotion.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        virtual ReferenceCounterValueType AddStrongRef() = 0;


        /// Decrements the number of strong references by 1 and destroys the referenced object
        /// when the counter reaches zero. If there are no more weak references, destroys the
        /// reference counters object itself.

        /// \return The number of strong references after decrementing the counter.
        /// \remark The referenced object is destroyed when the last strong reference is released.
        ///         RefCountedObject keeps an implicit weak reference while the object is alive,
        ///         so the reference counters object remains alive until after the object
        ///         destructor returns.\n
        ///         If there are no more weak references after that, the reference counters
        ///         object itself is also destroyed.\n
        ///         The method is thread-safe and does not require explicit synchronization.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        ///         The only reliable value is 0 as the object is destroyed when the last
        ///         strong reference is released.
        virtual ReferenceCounterValueType ReleaseStrongRef() = 0;


        /// Increments the number of weak references by 1.

        /// \return The number of weak references after incrementing the counter.
        /// \remark The counter update is thread-safe, but the caller must already
        ///         own a strong reference, own a weak reference, or otherwise
        ///         externally guarantee that the reference counters object is still alive.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        virtual ReferenceCounterValueType AddWeakRef() = 0;


        /// Decrements the number of weak references by 1. If there are no more strong and weak
        /// references, destroys the reference counters object itself.

        /// \return The number of weak references after decrementing the counter.
        /// \remark The method is thread-safe and does not require explicit synchronization.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        virtual ReferenceCounterValueType ReleaseWeakRef() = 0;


        /// Queries a pointer to the IUnknown interface of the referenced object.

        /// \param [out] ppObject - Memory address where the pointer to the object
        ///                         will be stored.
        /// \remark If the object was destroyed, nullptr will be written to *ppObject.
        ///         If the object was not released, the pointer to the object's IUnknown interface
        ///         will be stored. In this case, the number of strong references to the object
        ///         will be incremented by 1.\n
        ///         This method is a safe way to promote a weak reference to a strong reference.
        ///         Direct AddStrongRef() on a raw pointer does not provide the required lifetime
        ///         synchronization.\n
        ///         The method is thread-safe and does not require explicit synchronization.
        virtual void QueryObject(struct IObject** ppObject) = 0;


        /// Returns the number of outstanding strong references.

        /// \return The number of strong references.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        ///         The only reliable value is 0 as the object is destroyed when the last
        ///         strong reference is released.
        virtual ReferenceCounterValueType GetNumStrongRefs() const = 0;


        /// Returns the number of outstanding weak references.

        /// \return The number of weak references.
        /// \remark Diligent's RefCountedObject implementation keeps one implicit
        ///         weak reference while the referenced object is alive. The returned
        ///         value includes this implicit weak reference, so one external weak
        ///         pointer to a live object is reported as two weak references.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        virtual ReferenceCounterValueType GetNumWeakRefs() const = 0;
    };
}
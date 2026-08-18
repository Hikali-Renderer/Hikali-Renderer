/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include "InterfaceID.h"
#include "ReferenceCounters.h"

namespace Hikali
{
    /// Base interface for all dynamic objects in the renderer
    struct IObject
    {
        /// Queries the specific interface.

        /// \param [in] IID - Unique identifier of the requested interface.
        /// \param [out] ppInterface - Memory address where the pointer to the requested interface will be written.
        ///                            If the interface is not supported, null pointer will be returned.
        /// \remark The method increments the number of strong references by 1. The interface must be
        ///         released by a call to Release() method when it is no longer needed.
        virtual void QueryInterface(const INTERFACE_ID& IID, IObject** ppInterface) = 0;


    #    ifndef HIKALI_SHARP_GEN
        /// Template version of QueryInterface that avoids the need to manually
        /// cast the returned interface pointer.
        template <typename DerivedType, typename = typename std::enable_if<std::is_base_of<IObject, DerivedType>::value>::type>
        void QueryInterface(const INTERFACE_ID& IID, DerivedType** ppInterface)
        {
            if (ppInterface == nullptr)
                return;
            QueryInterface(IID, reinterpret_cast<IObject**>(ppInterface));
        }
    #    endif


        /// Increments the number of strong references by 1.

        /// \remark This method is equivalent to GetReferenceCounters()->AddStrongRef().\n
        ///         The method is thread-safe and does not require explicit synchronization.
        /// \return The number of strong references after incrementing the counter.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        virtual ReferenceCounterValueType AddRef() = 0;


        /// Decrements the number of strong references by 1 and destroys the object when the
        /// counter reaches zero.

        /// \remark This method is equivalent to GetReferenceCounters()->ReleaseStrongRef().\n
        ///         The method is thread-safe and does not require explicit synchronization.
        /// \return The number of strong references after decrementing the counter.
        /// \note   In a multithreaded environment, the returned number may not be reliable
        ///         as other threads may simultaneously change the actual value of the counter.
        ///         The only reliable value is 0 as the object is destroyed when the last
        ///         strong reference is released.
        virtual ReferenceCounterValueType Release() = 0;


        /// Returns the pointer to IReferenceCounters interface of the associated
        /// reference counters object. The method does **NOT** increment
        /// the number of strong references to the returned object.
        virtual IReferenceCounters* GetReferenceCounters() const = 0;
    };
}
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! Borrowed, type-erased pipeline message box.
//!
//! [`RustTypeErasedBox`] is the Rust mirror of the C++ `TypeErasedBox`, and the
//! thing a Rust handler passes around. It **borrows** the pipeline's on-stack
//! box (`Pin<&mut TypeErasedBox>`) — there is no heap allocation and the box is
//! never copied. The concrete message is recovered by naming the type:
//!
//! ```rust,ignore
//! fn on_read(&mut self, ctx: &mut Ctx, mut msg: RustTypeErasedBox<'_>) -> HandlerResult {
//!     let bytes: BytesPtr = msg.take::<BytesPtr>(); // zero-copy move out
//!     // ...
//! }
//! ```
//!
//! [`RustTypeErasedBox::take`] is **generic** over any [`RustMessageAdapter`]
//! (no per-type C++ thunk, no type id/enum) and mirrors C++
//! `TypeErasedBox::take<T>()`: it **relocates** the value out of the inline
//! storage — moving only the handle (e.g. a `unique_ptr`'s pointer), never the
//! payload. There is no type tag, so taking the wrong type reinterprets the
//! bytes → UB/crash, the accepted price of this pipeline. The message type
//! itself is the identity.

use std::mem::size_of;
use std::pin::Pin;
use std::ptr;

use crate::adapter::BytesPtr;
use crate::adapter::RustMessageAdapter;
use crate::ffi::ffi;
use crate::ffi::ffi::TypeErasedBox;

/// Provides the dev-mode type check used by [`RustTypeErasedBox::take`].
/// Implemented once per message type via a tiny per-type C++ thunk that compares
/// `typeid` — exactly like the per-type C++ `RustMessageAdapter<T>`. In release
/// there is no type tag, so it is a no-op (`true`) and `take` reinterprets.
pub trait ErasedCheck {
    /// In dev, whether the box holds this type; in release, always `true`.
    fn holds(teb: &TypeErasedBox) -> bool;
}

impl ErasedCheck for BytesPtr {
    fn holds(teb: &TypeErasedBox) -> bool {
        ffi::rust_teb_holds_bytes(teb)
    }
}

/// A borrowed, type-erased pipeline message box (mirror of C++ `TypeErasedBox`).
/// See the module docs. It holds a borrow of the pipeline's on-stack box; no
/// allocation, no copy.
pub struct RustTypeErasedBox<'a> {
    inner: Pin<&'a mut TypeErasedBox>,
}

impl<'a> RustTypeErasedBox<'a> {
    /// Wrap a borrow of the pipeline's on-stack box.
    pub fn new(inner: Pin<&'a mut TypeErasedBox>) -> Self {
        Self { inner }
    }

    /// Take the concrete message as `M`, mirroring C++ `TypeErasedBox::take<T>()`.
    ///
    /// Generic over any [`RustMessageAdapter`] and zero-copy: the value is
    /// relocated out of the box's inline storage (moving only the handle, never
    /// the payload) and the box is left empty.
    ///
    /// Type safety mirrors C++ exactly: in **dev** the type is checked and a
    /// mismatch **panics**; in **release** the check compiles out and the take
    /// reinterprets — a wrong type is UB/crash, the accepted price of this
    /// pipeline.
    pub fn take<M: RustMessageAdapter + ErasedCheck>(&mut self) -> M {
        // Dev-mode check (mirrors C++ take<T>()'s checkAccess). Compiled out in
        // release (zero overhead), where the relocate below reinterprets.
        debug_assert!(
            M::holds(self.inner.as_ref().get_ref()),
            "RustTypeErasedBox::take: box does not hold the requested type"
        );

        // The inline storage (`SmallBuffer::inline_`) is the first member of the
        // first subobject of `TypeErasedBox`, so the box address IS the storage
        // address.
        // SAFETY: `TypeErasedBox` is not `Unpin`, but taking its address to
        // reach the inline storage does not move it.
        let storage =
            unsafe { self.inner.as_mut().get_unchecked_mut() as *mut TypeErasedBox as *mut u8 };

        // SAFETY (substrate, zero-copy — mirrors C++ `take<T>()`):
        // 1. `ptr::read` relocates the value bitwise out of the storage. This
        //    moves only the handle (e.g. the `unique_ptr`'s 8-byte pointer),
        //    never the payload — exactly like a C++ move.
        // 2. `write_bytes(0)` nulls the source so the box's destroy hook runs on
        //    a null value (a no-op), matching what the C++ move-ctor does;
        //    together with the `reset()` below this transfers ownership exactly
        //    once (no double free).
        // 3. In release, if the box does not hold `M::CppRepr` this reinterprets
        //    the bytes → UB/crash, exactly like C++ `take`. In dev the
        //    `debug_assert!` above has already panicked on a mismatch.
        let cpp = unsafe {
            let value = ptr::read(storage as *const M::CppRepr);
            ptr::write_bytes(storage, 0, size_of::<M::CppRepr>());
            value
        };
        // Flip the box to empty (destroy hook now runs on the nulled source: a
        // no-op), matching the `reset()` inside C++ `take<T>()`.
        ffi::rust_teb_reset(self.inner.as_mut());
        M::from_cpp(cpp)
    }

    /// True if the box currently holds no value.
    pub fn is_empty(&self) -> bool {
        ffi::rust_teb_is_empty(&self.inner)
    }
}

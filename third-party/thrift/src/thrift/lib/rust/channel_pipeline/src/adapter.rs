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

//! Message adapter system for Rust channel_pipeline integration.
//!
//! Each Rust-supported pipeline message type defines how to move a value
//! out of a C++ `TypeErasedBox`, expose a Rust-safe representation, and
//! rebuild the same pipeline message type. This preserves the
//! single-message-type-per-layer rule — `TypeErasedBox` itself never crosses
//! the FFI boundary.
//!
//! # Registered adapters
//!
//! Only [`BytesPtr`] (`unique_ptr<folly::IOBuf>`, ID=1) is registered today.
//! The C++ `isRegisteredRustMessageTypeId` rejects any other numeric ID before
//! invoking Rust. Native C++ pipelines that use a second message type
//! (`ParsedFrame`/`ComposedFrame` at the framing layer) have no synchronous
//! Rust handler consumer, so no second adapter is added now.
//!
//! # Extension patterns (when a concrete Rust consumer appears)
//!
//! - **Opaque C++ object**: box as `UniquePtr<T>` and expose behavior via CXX
//!   methods; never mirror the C++ layout in Rust (ABI instability → silent UB
//!   in opt builds when the C++ struct changes).
//! - **Serialized Thrift value**: serialize to IOBuf in `into_cpp` and
//!   deserialize in `from_cpp` via `cxx-thrift-utils`. Benchmark separately.
//! - **Stable C-compatible POD**: declare an intentionally stable `#[repr(C)]`
//!   shared type and keep the value within `TypeErasedBox`'s 120-byte,
//!   pointer-align, nothrow-move constraints.
//!
//! # Stable type IDs
//!
//! [`RustMessageTypeId`] values are append-only: existing IDs must never be
//! reused or reordered. This mirrors the C++ `EventEnum` append-only contract.
//! The Rust `BytesPtr=1` and C++ `kBytesPtr=1` must remain identical.

use cxx::UniquePtr;
use iobuf::folly::IOBuf;

/// Stable numeric type IDs for Rust-registered pipeline message types.
///
/// These values cross the FFI boundary and **must remain stable across
/// builds**. The C++ mirror (`RustMessageTypeId` in `RustMessageAdapter.h`)
/// carries identical numeric values and the C++ `isRegisteredRustMessageTypeId`
/// rejects any unregistered ID before the bridge is invoked.
///
/// The contract is **append-only**: new variants must be added at the end;
/// existing variants must never be reordered or reused. This mirrors the
/// pipeline's `EventEnum` append-only stability contract (the `Count` sentinel
/// gives storage size on both sides).
///
/// Currently only [`BytesPtr`] is registered (`kBytesPtr = 1` in C++).
#[repr(u32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RustMessageTypeId {
    BytesPtr = 1,
}

/// Owned handle to a `folly::IOBuf` chain crossing the FFI boundary.
///
/// `BytesPtr` is the only registered Rust message adapter for this bridge.
/// It represents a `std::unique_ptr<folly::IOBuf>` transferred by move from
/// C++ to Rust (in `on_read`/`on_write`) or from Rust back to C++ (via
/// [`CallbackContext::fire_read`] / [`CallbackContext::fire_write`]).
/// The transfer is **zero-copy**: no buffer data is copied.
///
/// A null `BytesPtr` (from [`BytesPtr::null`]) is rejected at the FFI
/// boundary — passing null to `fire_read`/`fire_write` returns
/// [`HandlerResult::Error`]. Construct non-null buffers via
/// [`CallbackContext::allocate`] or [`CallbackContext::copy_from_slice`].
///
/// The newtype prevents accidental mixing of messages from incompatible
/// pipeline layers at compile time.
#[derive(Debug)]
pub struct BytesPtr(pub UniquePtr<IOBuf>);

impl BytesPtr {
    /// Wrap a `UniquePtr<IOBuf>` — zero-copy ownership transfer.
    pub fn new(ptr: UniquePtr<IOBuf>) -> Self {
        Self(ptr)
    }

    /// Unwrap and return the inner `UniquePtr<IOBuf>`.
    pub fn into_inner(self) -> UniquePtr<IOBuf> {
        self.0
    }

    /// Create a null (empty-ownership) `BytesPtr`.
    ///
    /// A null `BytesPtr` is rejected at the FFI boundary before reaching Rust
    /// callbacks. Use [`CallbackContext::allocate`] or
    /// [`CallbackContext::copy_from_slice`] to create non-null buffers.
    pub fn null() -> Self {
        Self(UniquePtr::null())
    }

    /// Return `true` if this `BytesPtr` holds no `IOBuf`.
    pub fn is_null(&self) -> bool {
        self.0.is_null()
    }

    /// Return `true` if the chain contains zero data bytes and is not chained.
    pub fn is_empty_chain(&self) -> bool {
        if self.0.is_null() {
            return true;
        }
        self.0.length() == 0 && !self.0.isChained()
    }

    /// Return the total chain data length in bytes.
    pub fn chain_data_len(&self) -> usize {
        if self.0.is_null() {
            return 0;
        }
        self.0.computeChainDataLength()
    }

    /// Return the chain element count (number of IOBuf nodes).
    pub fn chain_element_count(&self) -> usize {
        if self.0.is_null() {
            return 0;
        }
        self.0.countChainElements()
    }

    /// Return a const reference to the underlying IOBuf for use with C++ shim
    /// clone/coalesce methods. Returns None when null.
    ///
    /// # Safety
    /// The reference is valid only as long as this BytesPtr is alive. It is
    /// used only as an argument to a synchronous C++ call on the invoking
    /// thread.
    pub(crate) fn as_iobuf_ref(&self) -> Option<&IOBuf> {
        if self.0.is_null() {
            None
        } else {
            Some(&self.0)
        }
    }

    /// Return the first-chunk data as a byte slice when available.
    ///
    /// Only valid for single-node chains. For chains, use
    /// `iobuf_shared()` + cursor, or `coalesced_copy` on the context to make
    /// a contiguous view first.
    pub fn first_chunk(&self) -> &[u8] {
        if self.0.is_null() {
            return &[];
        }
        // SAFETY:
        // (1) `self.0` is non-null (checked above).
        // (2) `IOBuf::data()` returns the first-chunk data pointer, which is
        //     valid for at least `IOBuf::length()` initialised bytes owned by
        //     this IOBuf node.  `length()` is the valid-data byte count for
        //     the first chunk; these two fields are always in sync by IOBuf
        //     invariant.
        // (3) Both `data` (non-null checked) and `len` (non-zero checked) are
        //     guarded before constructing the slice.
        // (4) The returned slice lifetime is bounded by `&self`, keeping the
        //     IOBuf alive for the entire borrow.
        unsafe {
            let data = self.0.data();
            let len = self.0.length();
            if data.is_null() || len == 0 {
                &[]
            } else {
                std::slice::from_raw_parts(data, len)
            }
        }
    }
}

/// Adapter trait — types implementing this can cross the FFI boundary.
///
/// Each conforming type provides a stable [`RustMessageTypeId`], a C++
/// representation type (`CppType`), and lossless round-trip conversion
/// functions. The conversion is always a **move** — no copying occurs on the
/// registered `BytesPtr` path.
///
/// See the module-level documentation for extension patterns.
pub trait RustMessageAdapter: Sized {
    const TYPE_ID: RustMessageTypeId;
    type CppRepr;

    /// Take ownership from C++ side (called from C++ adapter).
    fn from_cpp(cpp_repr: Self::CppRepr) -> Self;

    /// Move ownership to C++ side (called from Rust).
    fn into_cpp(self) -> Self::CppRepr;
}

impl RustMessageAdapter for BytesPtr {
    const TYPE_ID: RustMessageTypeId = RustMessageTypeId::BytesPtr;
    type CppRepr = UniquePtr<IOBuf>;

    fn from_cpp(cpp_repr: Self::CppRepr) -> Self {
        Self::new(cpp_repr)
    }

    fn into_cpp(self) -> Self::CppRepr {
        self.into_inner()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bytes_ptr_type_id() {
        assert_eq!(BytesPtr::TYPE_ID as u32, 1);
    }

    #[test]
    fn bytes_ptr_newtype_prevents_mixing() {
        // This test verifies the newtype exists and has correct type ID.
        // Actual round-trip tested in integration tests with C++.
        let type_id = BytesPtr::TYPE_ID;
        assert_eq!(type_id, RustMessageTypeId::BytesPtr);
    }
}

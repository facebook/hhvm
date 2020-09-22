// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Library to build `Custom_tag` OCaml values.

use std::convert::TryInto;
use std::ffi::{CStr, CString};
use std::mem::MaybeUninit;
use std::ops::Deref;
use std::os::raw::{c_char, c_int, c_void};
use std::rc::Rc;

use ocamlrep::from;
use ocamlrep::{Allocator, FromError, FromOcamlRep, OpaqueValue, ToOcamlRep, Value, CUSTOM_TAG};
use ocamlrep_ocamlpool::catch_unwind;

extern "C" {
    fn caml_register_custom_operations(ops: *const CustomOperations);
    fn caml_serialize_block_1(data: *const u8, len: usize);
    fn caml_serialize_int_8(x: i64);
    fn caml_deserialize_sint_8() -> i64;
    fn caml_deserialize_block_1(data: *mut u8, len: usize);
}

/// Struct containing the operations for a custom OCaml block.
///
/// This is the Rust encoding of OCaml's `struct custom_operations`.
///
/// For more information on the fields see
/// [the OCaml guide](https://caml.inria.fr/pub/docs/manual-ocaml/intfc.html#ss:c-custom-ops)
#[repr(C)]
pub struct CustomOperations {
    identifier: *const c_char,
    finalize: Option<extern "C" fn(usize) -> ()>,
    compare: Option<extern "C" fn(usize, usize) -> c_int>,
    hash: Option<extern "C" fn(usize) -> isize>,
    serialize: Option<extern "C" fn(usize, *mut usize, *mut usize) -> ()>,
    deserialize: Option<extern "C" fn(*mut c_void) -> usize>,
    compare_ext: Option<extern "C" fn(usize, usize) -> c_int>,

    /// Not implemented yet, always set to NULL.
    custom_fixed_length: *const c_void,
}

impl CustomOperations {
    /// Create a new custom block with the given identifier.
    ///
    /// All function pointers will be set to NULL by default.
    fn new(identifier: &'static CStr) -> Self {
        Self {
            identifier: identifier.as_ptr(),
            finalize: None,
            compare: None,
            hash: None,
            serialize: None,
            deserialize: None,
            compare_ext: None,
            custom_fixed_length: std::ptr::null(),
        }
    }
}

/// A wrapper around a Rust type that allows it
/// to be written into/read from OCaml memory and managed by
/// the OCaml GC.
///
/// The value still lives on the Rust heap in an `Rc`'d pointer,
/// and the `Rc`-pointer itself will be written to OCaml memory.
///
/// # Examples
///
/// Expose Rust type:
///
/// ```
/// use ocamlrep_ocamlpool::ocaml_ffi;
/// use std::cell::RefCell;
///
/// pub struct Counter(isize);
///
/// impl Counter {
///   pub fn new() -> Self {
///     Self(0)
///   }
///
///   pub fn inc(&mut self) {
///     self.0 += 1
///   }
///
///   pub fn read(&self) -> isize {
///     self.0
///   }
/// }
///
/// ocaml_ffi! {
///   fn counter_new() -> Custom<RefCell<Counter>> {
///     Custom::from(RefCell::new(Counter(0)))
///   }
///
///   fn counter_inc(counter: Custom<RefCell<Counter>>) {
///     counter.borrow_mut().inc();
///   }
///
///   fn counter_read(counter: Custom<RefCell<Counter>>) -> isize {
///     counter.borrow().read();
///   }
/// }
/// ```
///
/// From OCaml:
///
/// ```ocaml
/// type counter; (* abstract type *)
///
/// external counter_new : unit -> counter = "counter_new"
/// external counter_inc: counter -> unit = "counter_inc"
/// external counter_read : counter -> isize = "counter_read"
///
/// let () =
///   let cnt = counter_new () in (* will be dropped on GC finalization *)
///   assert (counter_read cnt == 0);
///   counter_inc cnt;
///   assert (counter_read cnt == 1)
/// ```
pub struct Custom<T: CamlSerialize>(Rc<T>);

impl<T: CamlSerialize> Custom<T> {
    /// Create a new `ToCustom` wrapper by taking ownership of the value.
    pub fn from(x: T) -> Self {
        Self::new(Rc::new(x))
    }

    /// Create a new `ToCustom` directly from an `Rc`'d value.
    pub fn new(x: Rc<T>) -> Self {
        Self(x)
    }

    /// Get a reference to the inner `Rc`
    pub fn inner(&self) -> &Rc<T> {
        &self.0
    }
}

impl<T: CamlSerialize> Deref for Custom<T> {
    type Target = T;
    fn deref(&self) -> &T {
        self.0.deref()
    }
}

/// A custom block has two words: a pointer to the CustomOperations struct,
/// and a pointer the the value. Our values are ref-counted, but an Rc pointer
/// is just pointer-sized.
#[repr(C)]
struct CustomBlockOcamlRep<T>(&'static CustomOperations, Rc<T>);

const CUSTOM_BLOCK_SIZE_IN_BYTES: usize = std::mem::size_of::<CustomBlockOcamlRep<()>>();
const CUSTOM_BLOCK_SIZE_IN_WORDS: usize =
    CUSTOM_BLOCK_SIZE_IN_BYTES / std::mem::size_of::<OpaqueValue>();

impl<T: CamlSerialize> ToOcamlRep for Custom<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let ops: &'static CustomOperations = <T as CamlSerialize>::operations();

        let mut block = alloc.block_with_size_and_tag(CUSTOM_BLOCK_SIZE_IN_WORDS, CUSTOM_TAG);

        // Safety: we don't call any method on `alloc` after this method.
        let block_ptr: *mut OpaqueValue = unsafe { alloc.block_ptr_mut(&mut block) };

        // Safety: `alloc` guarantees that the `block_ptr` returned by
        // `block_ptr_mut` is aligend to `align_of::<OpaqueValue>()` and valid
        // for reads and writes of `CUSTOM_BLOCK_SIZE_IN_WORDS *
        // size_of::<OpaqueValue>()` bytes. Since `CustomBlockOcamlRep` has size
        // `CUSTOM_BLOCK_SIZE_IN_WORDS * size_of::<OpaqueValue>()`, its
        // alignment is equal to `align_of::<OpaqueValue>()`, and no other
        // reference to our newly-allocated block can exist, it's safe for us to
        // interpret `block_ptr` as a `&mut CustomBlockOcamlRep`.
        let block_ptr = block_ptr as *mut MaybeUninit<CustomBlockOcamlRep<T>>;
        let custom_block = unsafe { block_ptr.as_mut().unwrap() };

        // Write the address of the operations struct to the first word, and the
        // pointer to the value to the second word.
        *custom_block = MaybeUninit::new(CustomBlockOcamlRep(ops, Rc::clone(&self.0)));

        block.build()
    }
}

impl<T: CamlSerialize> FromOcamlRep for Custom<T> {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let rc = rc_from_value::<T>(value)?;
        let rc = Rc::clone(rc);

        Ok(Custom::new(rc))
    }
}

/// Helper function to fetch a reference to the `Rc` from the OCaml representation
/// of a custom block.
fn rc_from_value<'a, T: CamlSerialize>(value: Value<'a>) -> Result<&'a Rc<T>, FromError> {
    let block = from::expect_block(value)?;
    from::expect_block_tag(block, CUSTOM_TAG)?;
    from::expect_block_size(block, CUSTOM_BLOCK_SIZE_IN_WORDS)?;

    // We still don't know whether this block is in fact a
    // CustomBlockOcamlRep<T>--it may be a CustomBlockOcamlRep<U>, or some
    // other custom block which happens to be the same size. We can verify
    // that the block is actually a CustomBlockOcamlRep<T> by checking that
    // it points to the correct CustomOperations struct.
    let ops = <T as CamlSerialize>::operations();
    if !std::ptr::eq(ops, block[0].to_bits() as *const CustomOperations) {
        return Err(FromError::UnexpectedCustomOps {
            expected: ops as *const _ as usize,
            actual: block[0].to_bits(),
        });
    }

    let value_ptr = value.to_bits() as *const CustomBlockOcamlRep<T>;

    // Safety: `value_ptr` is guaranteed to be aligned to
    // `align_of::<Value>()`, and our use of `expect_block_size` guarantees
    // that the pointer is valid for reads of `CUSTOM_BLOCK_SIZE_IN_WORDS *
    // `size_of::<Value>()` bytes. Since the first field points to the right
    // operations struct, we either have a valid `CustomBlockOCamlRep<T>`
    // (i.e., constructed above in our `ToOcamlRep` implementation) or
    // someone went out of their way to construct an invalid one. Assume
    // it's valid and read in the `CustomBlockOcamlRep<T>`.
    let custom_block = unsafe { value_ptr.as_ref().unwrap() };
    Ok(&custom_block.1)
}

/// Trait that allows OCaml serialization and deserialization.
///
/// If you want to support serialization/deserialization, you
/// **MUST** call `CamlSerialize::register()` when starting up
/// the program.
///
/// This will register your type in the OCaml runtime, allowing
/// deserialization.
///
/// Rust does not support different instantiations of the default
/// implementation for different implementors of trait types. Therefore,
/// you must implement `type_identifier`, `operations` and `register`
/// manually when implementing this trait for a type. You can use
/// the `caml_serialize_default_impls!()` to do that automatically:
///
/// ```
/// impl CamlSerialize for MyType {
///    caml_serialize_default_impls!();
/// }
/// ```
pub trait CamlSerialize: Sized {
    /// Get the type name.
    fn type_identifier() -> &'static CStr;

    /// Get the type's custom operations struct.
    ///
    /// Always has to return the same reference! If not, the
    /// OCaml-to-Rust conversion will fail.
    ///
    /// The returned structure is not intended to be used by
    /// a programmer. Using it directly by e.g. injecting it
    /// into OCaml custom blocks is dangerous and can cause
    /// undefined behavior. Don't do it!
    fn operations() -> &'static CustomOperations;

    /// Register the type with the OCaml system.
    ///
    /// # Safety
    ///
    /// Must not be called from multiple threads.
    ///
    /// This function interacts with the OCaml runtime, which is not thread-safe.
    /// If any other threads are attempting to interact with the OCaml runtime
    /// or its custom operations table (e.g., by invoking this function, or by
    /// executing OCaml code using custom blocks) when this function is invoked,
    /// undefined behavior will result.
    ///
    /// # Examples
    ///
    /// ```
    /// use ocamlrep_custom::CamlSerialize;
    /// use ocamlrep_ocamlpool::ocaml_ffi;
    ///
    /// struct IntBox(isize);
    ///
    /// impl CamlSerialize for IntBox {
    ///     caml_serialize_default_impls!();
    ///     fn serialize(&self) -> Vec<u8> { ... }
    ///     fn deserialize(buffer: &[u8]) -> Self { ... }
    /// }
    ///
    /// ocaml_ffi! {
    ///     fn register_custom_types() {
    ///         // Once `register_custom_types` has been invoked from OCaml, IntBox
    ///         // can be serialized and deserialized from OCaml using the Marshal
    ///         // module.
    ///         //
    ///         // Safety: this will be called from OCaml, as such nothing else will
    ///         // be interacting with the OCaml runtime.
    ///         unsafe { IntBox::register() };
    ///     }
    /// }
    /// ```
    unsafe fn register();

    /// Convert a value to an array of bytes.
    ///
    /// The default implementation panics.
    fn serialize(&self) -> Vec<u8> {
        panic!(
            "serialization not implemented for {:?}",
            Self::type_identifier()
        )
    }

    /// Deserialize a value form an array of bytes.
    ///
    /// The default implementation panics.
    fn deserialize(_data: &[u8]) -> Self {
        panic!(
            "deserialization not implemented for {:?}",
            Self::type_identifier()
        )
    }
}

#[macro_export]
macro_rules! caml_serialize_default_impls {
    () => {
        fn type_identifier() -> &'static std::ffi::CStr {
            static ONCE: std::sync::Once = std::sync::Once::new();
            static mut TYPE_NAME: Option<std::ffi::CString> = None;

            ONCE.call_once(|| {
                // Safety:
                // - We've gated initialization, so it's thread safe.
                // - We only set the constant once.
                unsafe {
                    TYPE_NAME = Some($crate::type_identifier_helper::<Self>());
                }
            });

            // Safety:
            // - By now the constant has been initialized, and once initialized
            //   it is never changes.
            // - Concurrent reads are OK.
            unsafe { TYPE_NAME.as_ref().unwrap() }
        }

        fn operations() -> &'static $crate::CustomOperations {
            static ONCE: std::sync::Once = std::sync::Once::new();
            static mut OPS_STRUCT: Option<$crate::CustomOperations> = None;

            ONCE.call_once(|| {
                // Safety:
                // - We've gated initialization, so it's thread safe.
                // - We only set the constant once.
                unsafe {
                    OPS_STRUCT = Some($crate::operations_helper::<Self>());
                }
            });

            // Safety:
            // - By now the constant has been initialized, and once initialized
            //   it is never changes.
            // - Concurrent reads are OK.
            unsafe { OPS_STRUCT.as_ref().unwrap() }
        }

        unsafe fn register() {
            static mut IS_REGISTERED: bool = false;

            // Safety: Can only be called in a single-threaded context!
            if IS_REGISTERED {
                return;
            }
            IS_REGISTERED = true;

            let ops = Self::operations();
            $crate::register_helper::<Self>(ops)
        }
    };
}

/// Helper used for the `caml_serialize_default_impls` macro
pub fn type_identifier_helper<T>() -> CString {
    let name = format!("ocamlrep.custom.{}", std::any::type_name::<T>());
    std::ffi::CString::new(name).unwrap()
}

/// Helper used for the `caml_serialize_default_impls` macro
pub fn operations_helper<T: CamlSerialize>() -> CustomOperations {
    let type_identifier = <T as CamlSerialize>::type_identifier();
    let mut ops = CustomOperations::new(type_identifier);
    ops.finalize = Some(drop_value::<T>);
    ops.serialize = Some(serialize_value::<T>);
    ops.deserialize = Some(deserialize_value::<T>);
    ops
}

/// Helper used for the `caml_serialize_default_impls` macro
///
/// Should not be used directly. Interacts with the OCaml runtime and is
/// thus unsafe to call in a multi-threaded context.
pub unsafe fn register_helper<T>(ops: &'static CustomOperations) {
    // Safety: operations struct has a static lifetime, it will live forever!
    caml_register_custom_operations(ops as *const CustomOperations);
}

/// Helper function used by `operations_helper`. Returns a finalizer for custom
/// blocks containing an `Rc<T>`.
extern "C" fn drop_value<T: CamlSerialize>(value: usize) {
    let _: usize = catch_unwind(|| {
        // Safety: We trust here that CustomOperations structs containing this
        // `drop_value` instance will only ever be referenced by custom blocks
        // matching the layout of `CustomBlockOcamlRep`. If that's so, then this
        // function should only be invoked by the OCaml runtime on a pointer to
        // a CustomBlockOcamlRep<T> created by T::to_ocamlrep. Such a pointer
        // would be aligned and valid.
        let custom_block_ptr = value as *mut CustomBlockOcamlRep<T>;
        let custom_block = unsafe { custom_block_ptr.as_mut().unwrap() };

        // The `Rc` will be dropped here, and its reference count will decrease
        // by one (possibly freeing the referenced value).

        // Safety: Since the OCaml runtime will only invoke the finalizer for a
        // value which will never again be used, it is safe to use
        // `drop_in_place` (i.e., our finalizer will only be invoked once, so we
        // won't cause a double-drop).
        unsafe {
            std::ptr::drop_in_place(&mut custom_block.1);
        }

        0
    });
}

/// Helper function for serialization. Interacts with the OCaml runtime, so must
/// only be invoked by the OCaml runtime when serializing a custom block.
extern "C" fn serialize_value<T: CamlSerialize>(
    value: usize,
    bsize_32: *mut usize,
    bsize_64: *mut usize,
) {
    let _: usize = catch_unwind(|| {
        // Safety: Only called by the OCaml runtime (we don't expose a means of
        // invoking this function from Rust), which provides some OCaml
        // CUSTOM_TAG block as the value.
        let value = unsafe { Value::from_bits(value) };

        // Only called by the OCaml runtime, when serializing
        // a Custom-object managed by the OCaml GC.
        let rc = rc_from_value::<T>(value).unwrap();

        let bytes: Vec<u8> = rc.serialize();
        let bytes_ptr = bytes.as_ptr();

        // Safety: As above, we don't expose a means of invoking this function
        // from Rust--it can only be invoked by the OCaml runtime while
        // serializing a value. It is safe to invoke OCaml serialization
        // functions in this context.
        unsafe {
            let len = bytes.len();
            caml_serialize_int_8(len.try_into().unwrap());
            caml_serialize_block_1(bytes_ptr, len);

            // The size taken up in the data-part of the custom block.
            *bsize_32 = std::mem::size_of::<u32>();
            *bsize_64 = std::mem::size_of::<u64>();
        }

        0
    });
}

/// Helper function for deserialization. Interacts with the OCaml runtime, so must
/// only be invoked by the OCaml runtime when serializing a custom block.
extern "C" fn deserialize_value<T: CamlSerialize>(data_ptr: *mut c_void) -> usize {
    catch_unwind(|| {
        // Get the serialized bytes from the input channel.
        let bytes = unsafe {
            // Safety: We don't expose a means of invoking this function from
            // Rust--`deserialize_value` can only be invoked by the OCaml
            // runtime while deserializing a custom block value. It is safe to
            // invoke OCaml deserialization functions in this context.
            let len: usize = caml_deserialize_sint_8().try_into().unwrap();

            let mut buf: Vec<u8> = Vec::with_capacity(len);

            // Safety: len <= capacity. The elements aren't initialized at this
            // time, but we trust that caml_deserialize_block_1 will fill `len`
            // bytes of the buffer.
            buf.set_len(len);

            // Safety: As above, `deserialize_value` can only be invoked by the
            // OCaml runtime during custom block deserialization.
            caml_deserialize_block_1(buf.as_mut_ptr(), len);
            buf
        };

        // Actually deserialize those bytes into a T.
        let val: T = CamlSerialize::deserialize(&bytes);

        // Safety: The OCaml runtime will give us a data buffer which is
        // usize-aligned and valid for reads and writes of bsize_32 or bsize_64
        // (as provided by `serialize_value`, above) bytes (depending on system
        // architecture). This is sufficient for `Rc<T>` (which has the size and
        // alignment of usize).
        let data_ptr = data_ptr as *mut MaybeUninit<Rc<T>>;
        let data = unsafe { data_ptr.as_mut().unwrap() };

        *data = MaybeUninit::new(Rc::new(val));

        // Return the size of the value we wrote to our output pointer. The
        // OCaml runtime will verify that it matches the expected
        // bsize_32/bsize_64 written by the serializer.
        std::mem::size_of_val(data)
    })
}

#[cfg(test)]
mod test {
    use super::*;
    use std::mem::*;

    #[test]
    fn custom_block_ocamlrep_size() {
        assert_eq!(
            size_of::<CustomBlockOcamlRep<u8>>(),
            2 * size_of::<OpaqueValue>()
        );
    }

    #[test]
    fn custom_block_ocamlrep_align() {
        assert_eq!(
            align_of::<CustomBlockOcamlRep<u8>>(),
            align_of::<OpaqueValue>()
        );
    }
}

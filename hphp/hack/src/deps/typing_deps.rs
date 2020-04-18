// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(use_unstable_features, feature(test))]

use ocamlrep::Value;
use std::collections::hash_map::DefaultHasher;
use std::convert::TryInto;
use std::ffi::CString;
use std::hash::Hasher;
use std::panic;

/// Variant types used in the naming table.
///
/// NOTE: Keep in sync with the order of the fields in `Typing_deps.ml`.
#[derive(Copy, Clone, Debug)]
pub enum DepType {
    GConst = 0,
    Fun = 1,
    Class = 2,
    Extends = 3,
    RecordDef = 4,
    Const = 5,
    Cstr = 6,
    Prop = 7,
    SProp = 8,
    Method = 9,
    SMethod = 10,
    AllMembers = 11,
    FunName = 12,
    GConstName = 13,
}

fn tag_to_dep_type(tag: u8) -> DepType {
    match tag {
        0 => DepType::GConst,
        1 => DepType::Fun,
        2 => DepType::Class,
        3 => DepType::Extends,
        4 => DepType::RecordDef,
        5 => DepType::Const,
        6 => DepType::Cstr,
        7 => DepType::Prop,
        8 => DepType::SProp,
        9 => DepType::Method,
        10 => DepType::SMethod,
        11 => DepType::AllMembers,
        12 => DepType::FunName,
        13 => DepType::GConstName,
        _ => panic!("Invalid dep type: {:?}", tag),
    }
}

fn make_hasher() -> DefaultHasher {
    Default::default()
}

/// Ensure a positive hash value, like OCaml does for `Hashtbl.hash`. Set both
/// the 31st and 32nd bits, so that the sign bit is zero for both OCaml and Rust.
fn make_positive_30_bit_int(hash: i32) -> i32 {
    hash & 0b00111111_11111111_11111111_11111111
}

fn postprocess_hash(dep_type: DepType, hash: u64) -> i32 {
    let hash = hash as i32;
    let hash = match dep_type {
        DepType::Class => {
            // For class dependencies, set the lowest bit to 1. For extends
            // dependencies, the lowest bit will be 0 (in the case below), so we'll
            // be able to convert from a class hash to its extends hash without
            // reversing the hash.
            (hash << 1) | 1
        }
        _ => {
            // Ensure that only classes have the lowest bit set to 1, so that we
            // don't try to transitively traverse the subclasses of non-class
            // dependencies.
            hash << 1
        }
    };

    // Ensure that this is a 30-bit integer. We use 1 bit for the tag when
    // storing it in shared memory, and OCaml integers are 31 bits.
    make_positive_30_bit_int(hash)
}

fn get_dep_type_hash_key(dep_type: DepType) -> u8 {
    match dep_type {
        DepType::Class | DepType::Extends => {
            // Use the same tag for classes and extends dependencies, so that we can
            // convert between them without reversing the hash.
            DepType::Class as u8
        }
        _ => dep_type as u8,
    }
}

fn hash1(dep_type: DepType, name1: &[u8]) -> i32 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    postprocess_hash(dep_type, hasher.finish())
}

fn hash2(dep_type: DepType, name1: &[u8], name2: &[u8]) -> i32 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    hasher.write(name2);
    postprocess_hash(dep_type, hasher.finish())
}

fn hash_variant_or_panic(value: Value<'_>) -> Value<'static> {
    let block = value.as_block().expect("immediate value provided");
    let dep_type: DepType = tag_to_dep_type(block.tag());

    let values = block.as_values().expect("unable to examine block");
    let hash = match values {
        // Handle `string -> 'a variant` cases.
        [s] => {
            let s = s.as_byte_string().expect("unable to examine string parameter to variant");
            hash1(dep_type, &s)
        }

        // Handle `string * string -> 'a variant` cases.
        [s1, s2] => {
            let s1 = s1.as_byte_string().expect("unable to examine first string parameter to variant");
            let s2 = s2
                .as_byte_string()
                .expect("unable to examine second string parameter to variant");
            hash2(dep_type, &s1, &s2)
        }

        slice =>
            panic!("unexpected number of elements in variant parameter slice (did you add/modify a case of `Typing_deps.variant`?): {:?}", &slice)
    };

    let hash = hash.try_into().expect("hash did not fit into isize");
    Value::int(hash)
}

/// Hashes a `Typing_deps.Dep.variant`.
///
/// # Safety
///
/// The `value` must point to a valid OCaml value. All reachable values from it must
/// also be valid OCaml values, and none may be naked pointers. `value` must not
/// be concurrently modified while this function holds a reference to it.
///
/// This function is only called from OCaml, and it is passed a value allocated
/// entirely by OCaml code, so the values reachable by the argument will all be
/// valid (and will not be naked pointers). The OCaml runtime is interrupted by
/// the FFI call to this function, none of the transitively called functions from
/// here call into the OCaml runtime, and we do not spawn threads in our OCaml
/// code, so the pointed-to value will not be concurrently modified.
#[no_mangle]
pub unsafe extern "C" fn hash_variant(value: usize) -> usize {
    let result = panic::catch_unwind(|| {
        let value = Value::from_bits(value);
        let result = hash_variant_or_panic(value);
        result.to_bits()
    });
    match result {
        Ok(result) => result,
        Err(message) => {
            // `hash_variant` is called in a hot path, so we can't afford to
            // return an `option`/`result` to OCaml. We also can't let the panic
            // propragate upward since would cross the OCaml/Rust FFI boundary.
            // Instead, we'll raise an exception.
            // NB: we leak `message` here. Ideally, this exception would
            // terminate the process and it wouldn't matter (although it's
            // possible that the caller ignores the exception).
            let message =
                CString::new(format!("{:?}", message)).expect("null byte in error message");
            ocaml::core::fail::caml_failwith(message.into_raw());
            unreachable!()
        }
    }
}

#[cfg(all(test, use_unstable_features))]
mod tests {
    extern crate test;

    use ocamlrep::{Allocator, Arena};
    use test::Bencher;

    const SHORT_CLASS_NAME: &str = "\\Foo";
    const LONG_CLASS_NAME: &str = "\\EntReasonablyLongClassNameSinceSomeClassNamesAreLong";

    #[bench]
    fn bench_hash1_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Class, SHORT_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash1_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash1(crate::DepType::Class, LONG_CLASS_NAME.as_bytes());
        });
    }

    #[bench]
    fn bench_hash2_short(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(crate::DepType::Const, SHORT_CLASS_NAME.as_bytes(), b"\\T");
        });
    }

    #[bench]
    fn bench_hash2_long(b: &mut Bencher) {
        b.iter(|| {
            crate::hash2(
                crate::DepType::Const,
                LONG_CLASS_NAME.as_bytes(),
                b"\\TSomeTypeConstant",
            );
        });
    }

    #[bench]
    fn bench_variant(b: &mut Bencher) {
        let arena = Arena::new();
        let value = {
            let mut builder = arena.block_with_size_and_tag(2, crate::DepType::Class as u8);
            Arena::set_field(&mut builder, 0, arena.add(&String::from(LONG_CLASS_NAME)));
            Arena::set_field(
                &mut builder,
                1,
                arena.add(&String::from("\\TSomeTypeConstant")),
            );
            builder.build()
        };
        b.iter(|| unsafe {
            crate::hash_variant(value.to_bits());
        });
    }
}

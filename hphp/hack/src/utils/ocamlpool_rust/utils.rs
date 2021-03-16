// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::core::mlvalues::{Size, Tag, Value};

extern "C" {
    fn ocamlpool_reserve_block(tag: Tag, size: Size) -> Value;
    fn caml_initialize(addr: *mut Value, value: Value);
    static mut ocamlpool_generation: usize;
}

// Unsafe functions in this file should be called only:
// - while being called from OCaml process
// - between ocamlpool_enter / ocamlpool_leave invocations

pub unsafe fn reserve_block(tag: Tag, size: Size) -> Value {
    ocamlpool_reserve_block(tag, size)
}

pub unsafe fn caml_set_field(obj: Value, index: usize, val: Value) {
    caml_initialize((obj as *mut Value).add(index), val);
}

// Not implementing Ocamlvalue for integer types, because Value itself is an integer too and it makes
// it too easy to accidentally treat a pointer to heap as integer and try double convert it
pub fn usize_to_ocaml(x: usize) -> Value {
    (x << 1) + 1
}

pub fn u8_to_ocaml(x: u8) -> Value {
    usize_to_ocaml(x as usize)
}

pub fn ocaml_to_isize(i: Value) -> isize {
    (i as isize) >> 1
}

pub fn get_ocamlpool_generation() -> usize {
    unsafe { ocamlpool_generation }
}

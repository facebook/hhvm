// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml::core::memory;
use ocaml::core::mlvalues::{Color, Size, Tag, Value};

extern "C" {
    fn ocamlpool_reserve_block(tag: Tag, size: Size) -> Value;
    fn ocamlpool_reserve_string(size: Size) -> Value;
    static ocamlpool_limit: *mut Value;
    static ocamlpool_bound: *mut Value;
    static mut ocamlpool_cursor: *mut Value;
    static ocamlpool_color: Color;
    static mut ocamlpool_generation: usize;
}

// Unsafe functions in this file should be called only:
// - while being called from OCaml process
// - between ocamlpool_enter / ocamlpool_leave invocations

pub unsafe fn reserve_block(tag: Tag, size: Size) -> Value {
    let result = ocamlpool_cursor.offset(-(size as isize) - 1);
    if result < ocamlpool_limit || result >= ocamlpool_bound {
        return ocamlpool_reserve_block(tag, size);
    }
    ocamlpool_cursor = result;
    *result = (tag as usize) | ocamlpool_color | (size << 10);
    return result.offset(1) as Value;
}

pub unsafe fn caml_set_field(obj: Value, index: usize, val: Value) {
    if (val & 1 == 1)
        || ((val as *const Value) >= ocamlpool_limit && (val as *const Value) <= ocamlpool_bound)
    {
        *(obj as *mut Value).offset(index as isize) = val;
    } else {
        memory::caml_initialize((obj as *mut Value).offset(index as isize), val);
    }
}

// Not implementing Ocamlvalue for integer types, because Value itself is an integer too and it makes
// it too easy to accidentally treat a pointer to heap as integer and try double convert it
pub fn usize_to_ocaml(x: usize) -> Value {
    (x << 1) + 1
}

pub fn u8_to_ocaml(x: u8) -> Value {
    usize_to_ocaml(x as usize)
}

pub fn str_to_ocaml(s: &[u8]) -> Value {
    unsafe {
        let value = ocamlpool_reserve_string(s.len());
        let mut str_ = ocaml::Str::from(ocaml::Value::new(value));
        str_.data_mut().copy_from_slice(s);
        value
    }
}

pub fn ocaml_to_isize(i: Value) -> isize {
    ((i >> 1) | (std::isize::MIN as usize & i)) as isize
}

pub unsafe fn block_field(block: &ocaml::Value, field: usize) -> ocaml::Value {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field))
}

pub unsafe fn bool_field(block: &ocaml::Value, field: usize) -> bool {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field)).i32_val() != 0
}

pub unsafe fn usize_field(block: &ocaml::Value, field: usize) -> usize {
    ocaml::Value::new(*ocaml::core::mlvalues::field(block.0, field)).i32_val() as usize
}

pub unsafe fn str_field(block: &ocaml::Value, field: usize) -> ocaml::Str {
    ocaml::Str::from(ocaml::Value::new(*ocaml::core::mlvalues::field(
        block.0, field,
    )))
}

pub fn get_ocamlpool_generation() -> usize {
    unsafe { ocamlpool_generation }
}

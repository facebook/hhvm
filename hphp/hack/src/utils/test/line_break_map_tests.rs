// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

use line_break_map::LineBreakMap;
use ocaml::caml;
use ocamlpool_rust::ocamlvalue::*;
use ocamlpool_rust::utils::*;

caml!(offset_to_file_pos_triple, |ocaml_string, ocaml_offset|, <r>, {
    ocamlpool_enter();
    let os = ocaml::Str::from(ocaml_string);
    let string = os.data();
    let offset = ocaml_to_isize(ocaml_offset.0);
    let line_break_map = LineBreakMap::new(string);
    r = ocaml::Value::new(
        line_break_map.offset_to_file_pos_triple_original(offset).ocamlvalue()
    );
    ocamlpool_leave();
} -> r);

caml!(offset_to_file_pos_triple_with_cursor, |ocaml_string, ocaml_cursor_offset, ocaml_offset|, <r>, {
    ocamlpool_enter();
    let os = ocaml::Str::from(ocaml_string);
    let string = os.data();
    let offset = ocaml_to_isize(ocaml_offset.0);
    let cursor_offset = ocaml_to_isize(ocaml_cursor_offset.0);
    let line_break_map = LineBreakMap::new(string);
    line_break_map.offset_to_file_pos_triple_original(cursor_offset);
    r = ocaml::Value::new(
        line_break_map.offset_to_file_pos_triple_original(offset).ocamlvalue()
    );
    ocamlpool_leave();
} -> r);

caml!(offset_to_line_start_offset, |ocaml_string, ocaml_offset|, <r>, {
    ocamlpool_enter();
    let os = ocaml::Str::from(ocaml_string);
    let string = os.data();
    let offset = ocaml_to_isize(ocaml_offset.0);
    let line_break_map = LineBreakMap::new(string);
    r = ocaml::Value::new(
        line_break_map.offset_to_line_start_offset(offset).ocamlvalue()
    );
    ocamlpool_leave();
} -> r);

caml!(offset_to_position, |ocaml_string, ocaml_offset|, <r>, {
    ocamlpool_enter();
    let os = ocaml::Str::from(ocaml_string);
    let string = os.data();
    let offset = ocaml_to_isize(ocaml_offset.0);
    let line_break_map = LineBreakMap::new(string);
    r = ocaml::Value::new(
        line_break_map.offset_to_position(offset).ocamlvalue()
    );
    ocamlpool_leave();
} -> r);

caml!(position_to_offset, |ocaml_string, ocaml_existing, ocaml_i, ocaml_j|, <r>, {
    ocamlpool_enter();
    let os = ocaml::Str::from(ocaml_string);
    let string = os.data();
    let existing = ocaml::Value::from(ocaml_existing).0 == ocaml::value::TRUE.0;
    let i = ocaml_to_isize(ocaml_i.0);
    let j = ocaml_to_isize(ocaml_j.0);
    let line_break_map = LineBreakMap::new(string);
    let result = line_break_map.position_to_offset(existing, i, j);
    let pair = match result {
        Ok(p) => (p, true),
        Err(_) => (0, false),
    };
    r = ocaml::Value::new(pair.ocamlvalue());
    ocamlpool_leave();
} -> r);

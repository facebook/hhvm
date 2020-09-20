// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use line_break_map::LineBreakMap;
use ocamlrep_ocamlpool::ocaml_ffi;

ocaml_ffi! {
    fn offset_to_file_pos_triple(string: String, offset: isize) -> (isize, isize, isize) {
        let line_break_map = LineBreakMap::new(string.as_bytes());
        line_break_map.offset_to_file_pos_triple_original(offset)
    }

    fn offset_to_file_pos_triple_with_cursor(
        string: String,
        cursor_offset: isize,
        offset: isize,
    ) -> (isize, isize, isize) {
        let line_break_map = LineBreakMap::new(string.as_bytes());
        line_break_map.offset_to_file_pos_triple_original(cursor_offset);
        line_break_map.offset_to_file_pos_triple_original(offset)
    }

    fn offset_to_line_start_offset(string: String, offset: isize) -> isize {
        let line_break_map = LineBreakMap::new(string.as_bytes());
        line_break_map.offset_to_line_start_offset(offset)
    }

    fn offset_to_position(string: String, offset: isize) -> (isize, isize) {
        let line_break_map = LineBreakMap::new(string.as_bytes());
        line_break_map.offset_to_position(offset)
    }

    fn position_to_offset(string: String, existing: bool, i: isize, j: isize) -> (isize, bool) {
        let line_break_map = LineBreakMap::new(string.as_bytes());
        let result = line_break_map.position_to_offset(existing, i, j);
        match result {
            Ok(p) => (p, true),
            Err(_) => (0, false),
        }
    }
}

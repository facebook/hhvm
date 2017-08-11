(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  add_trailing_commas: bool;
  indent_width: int;
  indent_with_tabs: bool;
  line_width: int;
}

let default = {
  add_trailing_commas = true;
  indent_width = 2;
  indent_with_tabs = false;
  line_width = 80;
}

let add_trailing_commas t = t.add_trailing_commas

let indent_width t = t.indent_width

let indent_with_tabs t = t.indent_with_tabs

let line_width t = t.line_width

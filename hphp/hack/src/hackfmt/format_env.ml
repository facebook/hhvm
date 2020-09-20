(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  add_trailing_commas: bool;
  indent_width: int;
  indent_with_tabs: bool;
  line_width: int;
  format_generated_code: bool;
}

let default =
  {
    add_trailing_commas = true;
    indent_width = 2;
    indent_with_tabs = false;
    line_width = 80;
    format_generated_code = false;
  }

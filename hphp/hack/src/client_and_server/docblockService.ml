(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type docblock_element =
  | HackSnippet of string
  | XhpSnippet of string
  | Markdown of string

type result = docblock_element list

(* Represents a symbol location determined by the docblock service *)
type dbs_symbol_location = {
  dbs_filename: string;
  dbs_line: int;
  dbs_column: int;
  dbs_base_class: string option;
}

type dbs_symbol_location_result = dbs_symbol_location option

(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text

(* Rust ::std::result::Result *)
type ('t, 'e) result =
  | Ok of 't
  | Err of 'e

type lowerer_result = {
  ast: (Pos.t, unit, unit, unit) Aast.program;
  comments: (Pos.t * Prim_defs.comment) list;
}

type r = (lowerer_result, string) result

external parse_and_lower_from_text_ffi : SourceText.t -> r
  = "parse_and_lower_from_text"

let parse_and_lower_from_text text = parse_and_lower_from_text_ffi text

let from_text_rust (_env : Full_fidelity_ast.env) (source_text : SourceText.t)
    : r =
  parse_and_lower_from_text source_text

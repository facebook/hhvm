(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Emit_type_hint

let kind_to_type_info tparams k =
  match k with
  | Ast.Alias h | Ast.NewType h ->
    Emit_type_hint.hint_to_type_info ~always_extended:false tparams h

let from_ast : A.typedef -> Hhas_typedef.t =
  fun ast_typedef ->
  let typedef_name = Litstr.to_string @@ snd ast_typedef.Ast.t_id in
  let tparams = Emit_body.tparams_to_strings ast_typedef.Ast.t_tparams in
  let typedef_type_info =
    kind_to_type_info tparams ast_typedef.Ast.t_kind in
  Hhas_typedef.make
    typedef_name
    typedef_type_info

let from_asts ast_typedefs =
  List.map ast_typedefs from_ast

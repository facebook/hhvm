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

let kind_to_type_info ~tparams ~namespace k =
  match k with
  | Ast.Alias h | Ast.NewType h ->
    Emit_type_hint.hint_to_type_info
      ~skipawaitable:false ~nullable:false
      ~always_extended:false ~tparams ~namespace h

let from_ast : Ast.typedef -> Hhas_typedef.t =
  fun ast_typedef ->
  let namespace = ast_typedef.Ast.t_namespace in
  let typedef_name, _ =
    Hhbc_id.Class.elaborate_id namespace ast_typedef.Ast.t_id in
  let tparams = Emit_body.tparams_to_strings ast_typedef.Ast.t_tparams in
  let typedef_type_info =
    kind_to_type_info ~tparams ~namespace ast_typedef.Ast.t_kind in
  Hhas_typedef.make
    typedef_name
    typedef_type_info

let from_asts ast_typedefs =
  List.map ast_typedefs from_ast

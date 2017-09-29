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
    let nullable = match snd h with Ast.Hoption _ -> true | _ -> false in
    Emit_type_hint.(hint_to_type_info
      ~kind:TypeDef ~skipawaitable:false ~nullable ~tparams ~namespace h)

let kind_to_type_structure ~tparams ~namespace k =
  match k with
  | Ast.Alias h | Ast.NewType h ->
    Emit_type_constant.hint_to_type_constant ~is_typedef:true ~tparams
      ~namespace h

let emit_typedef : Ast.typedef -> Hhas_typedef.t =
  fun ast_typedef ->
  let namespace = ast_typedef.Ast.t_namespace in
  let typedef_name, _ =
    Hhbc_id.Class.elaborate_id namespace ast_typedef.Ast.t_id in
  let tparams = Emit_body.tparams_to_strings ast_typedef.Ast.t_tparams in
  let typedef_type_info =
    kind_to_type_info ~tparams ~namespace ast_typedef.Ast.t_kind in
  let typedef_type_structure =
    kind_to_type_structure ~tparams ~namespace ast_typedef.Ast.t_kind
  in
  Hhas_typedef.make
    typedef_name
    typedef_type_info
    (Some typedef_type_structure)

let emit_typedefs_from_program ast =
  List.filter_map ast
  (fun d -> match d with Ast.Typedef td -> Some (emit_typedef td) | _ -> None)

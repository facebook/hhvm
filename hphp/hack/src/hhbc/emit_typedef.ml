(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module T = Aast

let kind_to_type_info ~tparams h =
  let nullable =
    match snd h with
    | Aast.Hoption _ -> true
    | _ -> false
  in
  Emit_type_hint.(
    hint_to_type_info ~kind:TypeDef ~skipawaitable:false ~nullable ~tparams h)

let kind_to_type_structure ~tparams ~is_opaque h =
  Emit_type_constant.hint_to_type_constant
    ~is_typedef:true
    ~is_opaque
    ~tparams
    ~targ_map:SMap.empty
    h

let emit_typedef ast_typedef : Hhas_typedef.t =
  let namespace = ast_typedef.T.t_namespace in
  let typedef_name = Hhbc_id.Class.from_ast_name (snd ast_typedef.T.t_name) in
  let typedef_attributes =
    Emit_attribute.from_asts namespace ast_typedef.T.t_user_attributes
  in
  let tparams = Emit_body.tparams_to_strings ast_typedef.T.t_tparams in
  let typedef_type_info = kind_to_type_info ~tparams ast_typedef.T.t_kind in
  let is_opaque = ast_typedef.T.t_vis = T.Opaque in
  let typedef_type_structure =
    kind_to_type_structure ~tparams ~is_opaque ast_typedef.T.t_kind
  in
  Hhas_typedef.make
    typedef_name
    typedef_attributes
    typedef_type_info
    (Some typedef_type_structure)

let emit_typedefs_from_program (ast : Tast.def list) =
  let aux def =
    match def with
    | T.Typedef td -> Some (emit_typedef td)
    | _ -> None
  in
  List.filter_map ast ~f:aux

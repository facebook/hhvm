(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names
module Err = Naming_phase_error

module Env = struct
  let is_systemlib Naming_phase_env.{ is_systemlib; _ } = is_systemlib

  let is_hhi Naming_phase_env.{ is_hhi; _ } = is_hhi
end

let on_hint_ (env, hint_, err_acc) =
  let err =
    if Env.is_systemlib env || Env.is_hhi env then
      err_acc
    else
      match hint_ with
      | Aast.Happly ((pos, ty_name), _)
        when String.(
               equal ty_name SN.Classes.cHH_BuiltinEnum
               || equal ty_name SN.Classes.cHH_BuiltinEnumClass
               || equal ty_name SN.Classes.cHH_BuiltinAbstractEnumClass) ->
        (Err.naming
        @@ Naming_error.Using_internal_class
             { pos; class_name = Utils.strip_ns ty_name })
        :: err_acc
      | _ -> err_acc
  in
  Ok (env, hint_, err)

let on_class_ :
      'a 'b.
      _ * ('a, 'b) Aast_defs.class_ * _ ->
      (_ * ('a, 'b) Aast_defs.class_ * _, _) result =
 fun (env, (Aast.{ c_kind; c_enum; c_name; _ } as c), err) ->
  let c =
    let pos = fst c_name in
    match c_enum with
    | Some Aast.{ e_base; _ } ->
      let enum_hint = (pos, Aast.Happly (c_name, [])) in
      let (cls_name, bounds) =
        match c_kind with
        | Ast_defs.(Cenum_class Concrete) ->
          (* Turn the base type of the enum class into MemberOf<E, base> *)
          let bounds =
            [
              ( pos,
                Aast.Happly ((pos, SN.Classes.cMemberOf), [enum_hint; e_base])
              );
            ]
          in
          (SN.Classes.cHH_BuiltinEnumClass, bounds)
        | Ast_defs.(Cenum_class Abstract) ->
          (SN.Classes.cHH_BuiltinAbstractEnumClass, [])
        | _ -> (SN.Classes.cHH_BuiltinEnum, [enum_hint])
      in
      let c_extends =
        (pos, Aast.Happly ((pos, cls_name), bounds)) :: c.Aast.c_extends
      in
      Aast.{ c with c_extends }
    | _ -> c
  in
  Ok (env, c, err)

let pass =
  Naming_phase_pass.(
    bottom_up
      Ast_transform.
        { identity with on_hint_ = Some on_hint_; on_class_ = Some on_class_ })

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (_self)
    inherit [_] Aast_defs.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_class_ env (Aast.{ c_kind; c_enum; c_name; _ } as c) =
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
                    Aast.Happly
                      ((pos, SN.Classes.cMemberOf), [enum_hint; e_base]) );
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
      super#on_class_ env c
  end

let elab f ?(env = Env.empty) elem = f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem

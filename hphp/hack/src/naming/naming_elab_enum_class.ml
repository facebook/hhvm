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
    inherit [_] Aast_defs.endo

    method on_'ex _ ex = ex

    method on_'en _ en = en

    method! on_class_ _ (Aast.{ c_kind; c_enum; c_name; _ } as cls) =
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
          (pos, Aast.Happly ((pos, cls_name), bounds)) :: cls.Aast.c_extends
        in
        Aast.{ cls with c_extends }
      | _ -> cls
  end

let elab_fun_def ?(env = Env.empty) fd = visitor#on_fun_def env fd

let elab_typedef ?(env = Env.empty) td = visitor#on_typedef env td

let elab_module_def ?(env = Env.empty) m = visitor#on_module_def env m

let elab_gconst ?(env = Env.empty) gc = visitor#on_gconst env gc

let elab_class ?(env = Env.empty) cls = visitor#on_class_ env cls

let elab_program ?(env = Env.empty) prog = visitor#on_program env prog

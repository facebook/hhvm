(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = { allow_like: bool }

  let create ?(allow_like = false) () = { allow_like }

  let empty = { allow_like = false }
end

let visitor =
  object (self)
    inherit [_] Aast_defs.reduce as super

    inherit Err.monoid

    method! on_Is _env expr hint =
      let env = Env.create ~allow_like:true () in
      super#on_Is env expr hint

    method! on_As _env expr hint is_final =
      let env = Env.create ~allow_like:true () in
      super#on_As env expr hint is_final

    method! on_Upcast _env expr hint =
      let env = Env.create ~allow_like:true () in
      super#on_Upcast env expr hint

    method! on_Hfun _env hfun = super#on_Hfun Env.empty hfun

    method! on_Happly _env tycon hints = super#on_Happly Env.empty tycon hints

    method! on_Haccess _env hint ids = super#on_Haccess Env.empty hint ids

    method! on_Habstr _env name hints = super#on_Habstr Env.empty name hints

    method! on_Hvec_or_dict _env hk_opt hv =
      super#on_Hvec_or_dict Env.empty hk_opt hv

    method! on_hint (Env.{ allow_like } as env) hint =
      match hint with
      | (pos, Aast.Hlike inner) when not allow_like ->
        self#plus (Err.like_type pos) @@ self#on_hint env inner
      | _ -> super#on_hint env hint
  end

let validate f ?init ?(env = Env.empty) elem =
  Err.from_monoid ?init @@ f env elem

let validate_program ?init ?env elem =
  validate visitor#on_program ?init ?env elem

let validate_module_def ?init ?env elem =
  validate visitor#on_module_def ?init ?env elem

let validate_class ?init ?env elem = validate visitor#on_class_ ?init ?env elem

let validate_typedef ?init ?env elem =
  validate visitor#on_typedef ?init ?env elem

let validate_fun_def ?init ?env elem =
  validate visitor#on_fun_def ?init ?env elem

let validate_gconst ?init ?env elem = validate visitor#on_gconst ?init ?env elem

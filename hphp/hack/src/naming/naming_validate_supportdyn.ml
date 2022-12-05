(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.reduce as super

    inherit Err.monoid

    method! on_Happly env (pos, ty_name) hints =
      let err_super = super#on_Happly env (pos, ty_name) hints in

      let err =
        if String.(equal ty_name SN.Classes.cSupportDyn) then
          Err.supportdyn pos
        else
          self#zero
      in
      self#plus err_super err
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

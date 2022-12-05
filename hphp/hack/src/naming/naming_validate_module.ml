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
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Aast_defs.reduce as super

    inherit Err.monoid

    method! on_module_def env (Aast.{ md_span; _ } as md) =
      let super_err = super#on_module_def env md in
      self#plus super_err
      @@ Err.naming
      @@ Naming_error.Module_declaration_outside_allowed_files md_span
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

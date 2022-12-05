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

    method! on_class_ env (Aast.{ c_kind; c_typeconsts; c_span; _ } as c) =
      let err_super = super#on_class_ env c in
      let err =
        if (not (List.is_empty c_typeconsts)) && Ast_defs.is_c_enum_class c_kind
        then
          Err.naming
          @@ Naming_error.Type_constant_in_enum_class_outside_allowed_locations
               c_span
        else
          self#zero
      in
      self#plus err_super err
  end

let validate_program ?init ?(env = Env.empty) prog =
  Err.from_monoid ?init @@ visitor#on_program env prog

let validate_module_def ?init ?(env = Env.empty) module_def =
  Err.from_monoid ?init @@ visitor#on_module_def env module_def

let validate_class ?init ?(env = Env.empty) class_ =
  Err.from_monoid ?init @@ visitor#on_class_ env class_

let validate_typedef ?init ?(env = Env.empty) typedef =
  Err.from_monoid ?init @@ visitor#on_typedef env typedef

let validate_fun_def ?init ?(env = Env.empty) fun_def =
  Err.from_monoid ?init @@ visitor#on_fun_def env fun_def

let validate_gconst ?init ?(env = Env.empty) gconst =
  Err.from_monoid ?init @@ visitor#on_gconst env gconst

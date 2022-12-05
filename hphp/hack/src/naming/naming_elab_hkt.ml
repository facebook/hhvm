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
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_hint env hint =
      match hint with
      | (pos, Aast.Habstr (name, hints)) ->
        let err =
          match hints with
          | [] -> self#zero
          | _ ->
            Err.naming
            @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name }
        in
        ((pos, Aast.Habstr (name, [])), err)
      | _ -> super#on_hint env hint
  end

let elab_program ?init ?(env = Env.empty) prog =
  let (prog, err) = visitor#on_program env prog in
  (prog, Err.from_monoid ?init err)

let elab_module_def ?init ?(env = Env.empty) module_def =
  let (module_def, err) = visitor#on_module_def env module_def in
  (module_def, Err.from_monoid ?init err)

let elab_class ?init ?(env = Env.empty) class_ =
  let (class_, err) = visitor#on_class_ env class_ in
  (class_, Err.from_monoid ?init err)

let elab_typedef ?init ?(env = Env.empty) typedef =
  let (typedef, err) = visitor#on_typedef env typedef in
  (typedef, Err.from_monoid ?init err)

let elab_fun_def ?init ?(env = Env.empty) fun_def =
  let (fun_def, err) = visitor#on_fun_def env fun_def in
  (fun_def, Err.from_monoid ?init err)

let elab_gconst ?init ?(env = Env.empty) gconst =
  let (gconst, err) = visitor#on_gconst env gconst in
  (gconst, Err.from_monoid ?init err)

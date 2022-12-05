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
    inherit [_] Naming_visitors.mapreduce as super

    method! on_hint env hint =
      let res =
        match hint with
        | (pos, Aast.Habstr (name, hints)) ->
          let err =
            match hints with
            | [] -> self#zero
            | _ ->
              Err.naming
              @@ Naming_error.Tparam_applied_to_type { pos; tparam_name = name }
          in
          Error ((pos, Aast.Habstr (name, [])), err)
        | _ -> Ok hint
      in
      match res with
      | Ok hint -> super#on_hint env hint
      | Error (hint, err) -> (hint, err)

    method! on_tparam
        env (Aast.{ tp_parameters; tp_name = (pos, tparam_name); _ } as tparam)
        =
      let (tparam, err) =
        match tp_parameters with
        | [] -> (tparam, self#zero)
        | _ ->
          let err =
            Err.naming @@ Naming_error.Tparam_with_tparam { pos; tparam_name }
          in
          (Aast.{ tparam with tp_parameters = [] }, err)
      in
      let (tparam, super_err) = super#on_tparam env tparam in
      (tparam, self#plus err super_err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem

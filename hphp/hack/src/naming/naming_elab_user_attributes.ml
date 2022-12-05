(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_user_attributes (env, us, err_acc) =
  let seen = Caml.Hashtbl.create 0 in
  let dedup (attrs, err_acc) (Aast.{ ua_name = (pos, attr_name); _ } as attr) =
    match Caml.Hashtbl.find_opt seen attr_name with
    | Some prev_pos ->
      let err =
        Err.naming
        @@ Naming_error.Duplicate_user_attribute { pos; prev_pos; attr_name }
      in
      (attrs, Err.Free_monoid.plus err_acc err)
    | _ ->
      Caml.Hashtbl.add seen attr_name pos;
      (attr :: attrs, err_acc)
  in
  let (us, err) =
    Tuple2.map_fst ~f:List.rev @@ List.fold_left us ~init:([], err_acc) ~f:dedup
  in
  Naming_phase_pass.Cont.next (env, us, err)

let pass =
  Naming_phase_pass.(
    top_down { identity with on_user_attributes = Some on_user_attributes })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem

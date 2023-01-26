(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let on_user_attributes on_error =
  let handler
        : 'a 'b.
          _ * ('a, 'b) Aast_defs.user_attribute list ->
          (_ * ('a, 'b) Aast_defs.user_attribute list, _) result =
   fun (env, us) ->
    let seen = Caml.Hashtbl.create 0 in

    let dedup (attrs, err_acc) (Aast.{ ua_name = (pos, attr_name); _ } as attr)
        =
      match Caml.Hashtbl.find_opt seen attr_name with
      | Some prev_pos ->
        let err =
          Naming_phase_error.naming
          @@ Naming_error.Duplicate_user_attribute { pos; prev_pos; attr_name }
        in
        (attrs, err :: err_acc)
      | _ ->
        Caml.Hashtbl.add seen attr_name pos;
        (attr :: attrs, err_acc)
    in
    let (us, errs) =
      Tuple2.map_fst ~f:List.rev @@ List.fold_left us ~init:([], []) ~f:dedup
    in
    List.iter ~f:on_error errs;
    Ok (env, us)
  in
  handler

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_user_attributes = Some (on_user_attributes on_error);
        })

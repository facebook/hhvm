(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module TUtils = Typing_utils
module Cls = Decl_provider.Class

(* Only applied to classes. Checks that all the requirements of the traits
 * and interfaces it uses are satisfied. *)
let check_fulfillment env class_pos get_impl (trait_pos, req_ty) =
  match TUtils.try_unwrap_class_type req_ty with
  | None -> env
  | Some (_r, (_p, req_name), _paraml) ->
    let req_pos = Typing_defs.get_pos req_ty in
    (match get_impl req_name with
    | None ->
      (Errors.add_typing_error
      @@ Typing_error.(
           primary
           @@ Primary.Unsatisfied_req
                { pos = class_pos; trait_pos; req_pos; req_name }));
      env
    | Some impl_ty ->
      let (env, ty_err_opt) =
        Typing_phase.sub_type_decl env impl_ty req_ty
        @@ Some
             (Typing_error.Reasons_callback.unsatisfied_req_callback
                ~class_pos
                ~trait_pos
                ~req_pos
                req_name)
      in

      Option.iter ~f:Errors.add_typing_error ty_err_opt;
      env)

(** Check whether a class satifies all the requirements of the traits it uses,
    namely [require extends] and [require implements]. *)
let check_class env class_pos tc =
  match Cls.kind tc with
  | Ast_defs.Cclass _ ->
    List.fold
      (Cls.all_ancestor_reqs tc)
      ~f:(fun env req ->
        check_fulfillment env class_pos (Cls.get_ancestor tc) req)
      ~init:env
  | Ast_defs.Ctrait
  | Ast_defs.Cinterface
  | Ast_defs.Cenum_class _
  | Ast_defs.Cenum ->
    env

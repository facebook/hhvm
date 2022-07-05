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

let check_require_class env class_pos tc (trait_pos, req_ty) =
  match TUtils.try_unwrap_class_type req_ty with
  | None -> env
  | Some (_r, (_p, req_name), _paraml) ->
    (* in a `require class t;` trait constraint, t must be a non-generic class
     * name.  Since lowering enforces _paraml to be empty, so it is safe to
     * ignore _param here.  Additionally we enforce that the class that uses
     * the trait with a require class constraint does not have type parameters
     * and is final.
     *)
    if String.equal req_name (Cls.name tc) && List.is_empty (Cls.tparams tc)
    then (
      if Cls.final tc then
        env
      else
        let req_pos = Typing_defs.get_pos req_ty in
        (Errors.add_typing_error
        @@ Typing_error.(
             primary
             @@ Primary.Req_class_not_final
                  { pos = class_pos; trait_pos; req_pos }));
        env
    ) else
      let req_pos = Typing_defs.get_pos req_ty in
      (Errors.add_typing_error
      @@ Typing_error.(
           primary
           @@ Primary.Unsatisfied_req_class
                { pos = class_pos; trait_pos; req_pos; req_name }));
      env

(* HHVM enformcement rejects conflicting require class and require extends
   constraints on the same class, eg:

  trait T1 { require extends C; }
  trait T2 { require class C; }
  trait T { use T1, T2; }

  The check below, only run on traits, detects and rejects such cases,
  ensuring that Hack does not accept code on which HHVM enforcement
  raises an error.
*)
let check_require_class_require_extends_conflict env class_pos tc =
  let get_name_pos ty =
    match Typing_defs.get_node ty with
    | Typing_defs.Tapply ((p, n), _) -> Some (n, p)
    | _ -> None
  in
  match Cls.all_ancestor_req_class_requirements tc with
  | [] -> env
  | rc ->
    let require_class =
      List.filter_map rc ~f:(fun (_, ty) -> get_name_pos ty)
    in
    let require_extends_implements =
      List.filter_map (Cls.all_ancestor_reqs tc) ~f:(fun (_, ty) ->
          get_name_pos ty)
    in
    List.iter require_class ~f:(fun (nc, pc) ->
        match
          List.find require_extends_implements ~f:(fun (ne, _) ->
              String.equal nc ne)
        with
        | None -> ()
        | Some (_, pe) ->
          Errors.add_typing_error
          @@ Typing_error.(
               primary
               @@ Primary.Incompatible_reqs
                    {
                      pos = class_pos;
                      req_name = nc;
                      req_class_pos = pc;
                      req_extends_pos = pe;
                    }));
    env

(** Check whether a class satifies all the requirements of the traits it uses,
    namely [require extends] and [require implements]. *)
let check_class env class_pos tc =
  match Cls.kind tc with
  | Ast_defs.Cclass _ ->
    let env =
      List.fold
        (Cls.all_ancestor_reqs tc)
        ~f:(fun env req ->
          check_fulfillment env class_pos (Cls.get_ancestor tc) req)
        ~init:env
    in
    List.fold
      (Cls.all_ancestor_req_class_requirements tc)
      ~f:(fun env req -> check_require_class env class_pos tc req)
      ~init:env
  | Ast_defs.Ctrait ->
    check_require_class_require_extends_conflict env class_pos tc
  | Ast_defs.Cinterface
  | Ast_defs.Cenum_class _
  | Ast_defs.Cenum ->
    env

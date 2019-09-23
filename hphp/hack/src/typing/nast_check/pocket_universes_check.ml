(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Base
open Aast
open Typing_defs
module Cls = Decl_provider.Class

let build ?(sep = ":@") prefix name = prefix ^ sep ^ name

(* Checks that the visited class definition does not have local duplicated
 * case types/values, and stores the position of each one *)
let check_enum err l =
  List.fold
    ~f:(fun seen (p, ty) ->
      if SMap.mem ty seen then (
        err p ty;
        seen
      ) else
        SMap.add ty p seen)
    ~init:SMap.empty
    l

type err_callback = pos -> string -> string -> string -> unit

(* Checks that the visited case types/values do not appear in any parent class *)
let check_parent
    (err_types : err_callback)
    (err_values : err_callback)
    seen_types
    seen_values
    c_extends =
  Sequence.iter
    ~f:(fun parent ->
      match Decl_provider.get_class parent with
      | None -> ()
      | Some cls ->
        let c_name = Cls.name cls in
        let c_pu_enums = Cls.pu_enums cls in
        Sequence.iter
          ~f:(fun (enum_name, pu_enum) ->
            begin
              match SMap.find_opt enum_name seen_types with
              | None -> (* does not exist locally, skip it *) ()
              | Some seen ->
                SMap.iter
                  (fun _ (_, ty) ->
                    match SMap.find_opt ty seen with
                    | Some p -> err_types p c_name enum_name ty
                    | None -> ())
                  pu_enum.tpu_case_types
            end;
            match SMap.find_opt enum_name seen_values with
            | None -> ()
            | Some seen ->
              SMap.iter
                (fun _ ((_, name), _) ->
                  match SMap.find_opt name seen with
                  | Some p -> err_values p c_name enum_name name
                  | None -> ())
                pu_enum.tpu_case_values)
          c_pu_enums)
    c_extends

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _env c =
      let c_name = snd c.c_name in
      let dup_case_type p full_name spotted =
        Errors.pu_duplication p "case type" full_name spotted
      in
      let dup_case_value p full_name spotted =
        Errors.pu_duplication p "case value" full_name spotted
      in
      (* Gather the case types and values defined in the current class *)
      let (seen_types, seen_values) =
        List.fold
          ~init:(SMap.empty, SMap.empty)
          ~f:(fun (seen_types, seen_values) pu_enum ->
            let (p, enum_name) = pu_enum.pu_name in
            let prefix = build c_name enum_name in
            if SMap.mem enum_name seen_types then
              Errors.pu_duplication p "enumeration" prefix c_name;
            let seen_types =
              let err p ty = dup_case_type p (build prefix ty) prefix in
              let s = check_enum err pu_enum.pu_case_types in
              SMap.add enum_name s seen_types
            in
            let seen_values =
              let err p ty =
                dup_case_value p (build ~sep:"::" prefix ty) prefix
              in
              let s =
                check_enum err (List.map ~f:fst pu_enum.pu_case_values)
              in
              SMap.add enum_name s seen_values
            in
            (seen_types, seen_values))
          c.c_pu_enums
      in
      let get_pp_names ?(sep = ":@") c_name enum_name name =
        (build ~sep enum_name name, build c_name enum_name)
      in
      let err_types p spotted_c_name spotted_enum_name ty =
        let (full, spotted) =
          get_pp_names spotted_c_name spotted_enum_name ty
        in
        dup_case_type p (build c_name full) spotted
      in
      let err_values p spotted_c_name spotted_enum_name value =
        let (full, spotted) =
          get_pp_names ~sep:"::" spotted_c_name spotted_enum_name value
        in
        dup_case_value p (build c_name full) spotted
      in
      let c_extends =
        match Decl_provider.get_class c_name with
        | None -> Sequence.empty
        | Some cls -> Cls.all_ancestor_names cls
      in
      (* Check that none of the gathered case types/values are already defined
       * in a parent class *)
      check_parent err_types err_values seen_types seen_values c_extends
  end

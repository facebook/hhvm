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
    ctx
    (err_types : err_callback)
    (err_values : err_callback)
    seen_types
    seen_values
    c_extends =
  List.iter
    ~f:(fun parent ->
      match Decl_provider.get_class ctx parent with
      | None -> ()
      | Some cls ->
        let c_name = Cls.name cls in
        let c_pu_enums = Cls.pu_enums cls in
        List.iter
          ~f:(fun (enum_name, pu_enum) ->
            begin
              match SMap.find_opt enum_name seen_types with
              | None -> (* does not exist locally, skip it *) ()
              | Some seen ->
                SMap.iter
                  (fun _ ((_, ty), _) ->
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

    method! at_class_ env c =
      let c_name = snd c.c_name in
      (* We restrict PU definition to only happen in a proper class.
       * This limits PU to single inheritance, which we use here by
       * collpasing parent information into single set/maps *)
      let () =
        match c.c_kind with
        | Ast_defs.Cabstract
        | Ast_defs.Cnormal ->
          ()
        | Ast_defs.Cinterface
        | Ast_defs.Ctrait
        | Ast_defs.Cenum ->
          (match c.c_pu_enums with
          | [] -> ()
          | { pu_name; _ } :: _ ->
            Errors.pu_not_in_class (fst c.c_name) (snd pu_name) c_name)
      in
      (* handy wrappers around Errors *)
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
              let s = check_enum err (List.map ~f:fst pu_enum.pu_case_types) in
              SMap.add enum_name s seen_types
            in
            let seen_values =
              let err p ty =
                dup_case_value p (build ~sep:"::" prefix ty) prefix
              in
              let s = check_enum err (List.map ~f:fst pu_enum.pu_case_values) in
              SMap.add enum_name s seen_values
            in
            (seen_types, seen_values))
          c.c_pu_enums
      in
      (* More precise wrappers around Errors *)
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
      (* Map to find the position of the atoms in the local file. Also
       * stores the pos of enum, in case some missing atom is spotted *)
      let pos_map =
        List.fold
          ~init:SMap.empty
          ~f:(fun acc { pu_name; pu_members; _ } ->
            let (p, enum_name) = pu_name in
            let atom_pos_map =
              List.fold
                ~init:SMap.empty
                ~f:(fun acc { pum_atom; _ } ->
                  let (p, name) = pum_atom in
                  SMap.add name p acc)
                pu_members
            in
            SMap.add enum_name (p, atom_pos_map) acc)
          c.c_pu_enums
      in
      match Decl_provider.get_class env.Nast_check_env.ctx c_name with
      | None -> ()
      | Some cls ->
        let c_extends = Cls.all_ancestor_names cls in
        let pu_enums = Cls.pu_enums cls in
        let keys l = SSet.of_list @@ SMap.keys l in
        (* Check that none of the gathered case types/values are already defined
         * in a parent class *)
        check_parent
          env.Nast_check_env.ctx
          err_types
          err_values
          seen_types
          seen_values
          c_extends;

        (* Gather information about atoms definitions and instances to check
         * if they are correct *)
        List.iter
          ~f:
            (fun (enum_name, { tpu_case_types; tpu_case_values; tpu_members; _ })
                 ->
            match SMap.find_opt enum_name pos_map with
            (* This enum is not defined in the current file, skip it *)
            | None -> ()
            | Some (enum_pos, atom_pos_map) ->
              let types = keys tpu_case_types in
              let exprs = keys tpu_case_values in
              SMap.iter
                (fun atom_name { tpum_types; tpum_exprs; _ } ->
                  (* I wish I add Base.Set.iter2 for that purpose :D *)
                  let atom_types = keys tpum_types in
                  let atom_exprs = keys tpum_exprs in
                  let diff_types = SSet.diff types atom_types in
                  let diff_exprs = SSet.diff exprs atom_exprs in
                  let diff_types2 = SSet.diff atom_types types in
                  let diff_exprs2 = SSet.diff atom_exprs exprs in
                  let loc = build c_name enum_name in
                  let p =
                    match SMap.find_opt atom_name atom_pos_map with
                    | None -> enum_pos
                    | Some p -> p
                  in
                  SSet.iter
                    (fun ty -> Errors.pu_atom_missing p atom_name "type" loc ty)
                    diff_types;
                  SSet.iter
                    (fun expr ->
                      Errors.pu_atom_missing p atom_name "value" loc expr)
                    diff_exprs;
                  SSet.iter
                    (fun ty -> Errors.pu_atom_unknown p atom_name "type" loc ty)
                    diff_types2;
                  SSet.iter
                    (fun expr ->
                      Errors.pu_atom_unknown p atom_name "value" loc expr)
                    diff_exprs2)
                tpu_members)
          pu_enums
  end

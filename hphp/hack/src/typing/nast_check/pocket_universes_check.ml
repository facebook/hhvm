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

(* Checks that the visited pu definition does not have local duplicated
 * case types/case expressions, and stores the position of each one *)
let check_dup ~key err l =
  List.fold
    ~f:(fun seen elt ->
      let (p, name) = key elt in
      if SMap.mem name seen then (
        err p name;
        seen
      ) else
        SMap.add name p seen)
    ~init:SMap.empty
    l

let check_dup_case_types err l =
  check_dup ~key:(fun tp -> tp.Aast.tp_name) err l

let check_dup_case_exprs err l = check_dup ~key:fst err l

(* handy wrappers around Errors *)
let dup_case_type_in_instance p full_name spotted =
  Errors.pu_duplication_in_instance p "case type" full_name spotted

let dup_case_value_in_instance p full_name spotted =
  Errors.pu_duplication_in_instance p "case value" full_name spotted

(* Checks that the visited pu definition does not have local duplicated
 * instances, and that each instances do not have duplicate case types/
 * expressions. Stores the position of each instances in a map
 *)
let check_dup_instances err prefix l =
  List.fold
    ~f:(fun seen instance ->
      let (p, name) = instance.pum_atom in
      let prefix = build prefix name in
      let err_type p tyname = dup_case_type_in_instance p tyname prefix in
      let err_expr p name = dup_case_value_in_instance p name prefix in
      let (_ : pos SMap.t) = check_dup ~key:fst err_type instance.pum_types in
      let (_ : pos SMap.t) = check_dup ~key:fst err_expr instance.pum_exprs in
      if SMap.mem name seen then (
        err p name;
        seen
      ) else
        SMap.add name p seen)
    ~init:SMap.empty
    l

type err_callback = pos -> string -> string -> string -> unit

(* Checks that the visited case types/values do not appear in any parent
 * class. The only exception is when a PU is extending (adding a case type,
 * case expr) and needs to provide the content for existing instances.
 * So this test is a bit more involved, but only check that we don't declare
 * existing case type/expressions
 *)
let check_parent
    ctx
    (err_types : err_callback)
    (err_values : err_callback)
    seen_types
    seen_values
    seen_instances
    local_pu_enums
    c_extends =
  let keys l = SSet.of_list @@ SMap.keys l in
  let diff p c_name enum_name instance_name parent =
    let prefix = build c_name enum_name in
    let prefix = build prefix instance_name in
    let local_enum =
      List.find_exn
        ~f:(fun pu -> String.equal (snd pu.pu_name) enum_name)
        local_pu_enums
    in
    let instance =
      List.find_exn
        ~f:(fun memb -> String.equal (snd memb.pum_atom) instance_name)
        local_enum.pu_members
    in
    let instance_type_keys =
      List.fold
        ~init:SSet.empty
        ~f:(fun acc (name, _) -> SSet.add (snd name) acc)
        instance.pum_types
    in
    let instance_expr_keys =
      List.fold
        ~init:SSet.empty
        ~f:(fun acc (name, _) -> SSet.add (snd name) acc)
        instance.pum_exprs
    in
    let parent_type_keys = keys parent.tpum_types in
    let parent_expr_keys = keys parent.tpum_exprs in
    let diff_types = SSet.inter instance_type_keys parent_type_keys in
    let diff_exprs = SSet.inter instance_expr_keys parent_expr_keys in
    SSet.iter (fun name -> dup_case_type_in_instance p name prefix) diff_types;
    SSet.iter (fun name -> dup_case_type_in_instance p name prefix) diff_exprs
  in
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
                  (fun _ { tp_name = (_, ty); _ } ->
                    match SMap.find_opt ty seen with
                    | Some p -> err_types p c_name enum_name ty
                    | None -> ())
                  pu_enum.tpu_case_types
            end;
            begin
              match SMap.find_opt enum_name seen_values with
              | None -> ()
              | Some seen ->
                SMap.iter
                  (fun _ ((_, name), _) ->
                    match SMap.find_opt name seen with
                    | Some p -> err_values p c_name enum_name name
                    | None -> ())
                  pu_enum.tpu_case_values
            end;
            match SMap.find_opt enum_name seen_instances with
            | None -> ()
            | Some (_, seen) ->
              SMap.iter
                (* If a parent class already has an instance, we check that
                 * their content is disjoint (because a Pu can extend
                 * a parent instance
                 *)
                  (fun name parent ->
                  match SMap.find_opt name seen with
                  | Some p -> diff p c_name enum_name name parent
                  | None -> ())
                pu_enum.tpu_members)
          c_pu_enums)
    c_extends

let check_class env c =
  let c_name = snd c.c_name in
  (* handy wrappers around Errors *)
  let dup_case_type p full_name spotted =
    Errors.pu_duplication p "case type" full_name spotted
  in
  let dup_case_value p full_name spotted =
    Errors.pu_duplication p "case value" full_name spotted
  in
  let dup_instance p full_name spotted =
    Errors.pu_duplication p "instance" full_name spotted
  in
  (* Gather the case types, values and instances defined in the current class
   * Also stores the pos of enum in the instance map, in case some missing
   * atom is spotted
   *)
  let (seen_types, seen_values, seen_instances) =
    List.fold
      ~init:(SMap.empty, SMap.empty, SMap.empty)
      ~f:(fun (seen_types, seen_values, seen_instances) pu_enum ->
        let (p, enum_name) = pu_enum.pu_name in
        let prefix = build c_name enum_name in
        if SMap.mem enum_name seen_types then
          Errors.pu_duplication p "enumeration" prefix c_name;
        (* Check that there is no multiple `case type T` with the same `T` *)
        let seen_types =
          let err p tyname = dup_case_type p tyname prefix in
          let s = check_dup_case_types err pu_enum.pu_case_types in
          SMap.add enum_name s seen_types
        in
        (* Check that there is no multiple `case T x` with the same `x` *)
        let seen_values =
          let err p name = dup_case_value p name prefix in
          let s = check_dup_case_exprs err pu_enum.pu_case_values in
          SMap.add enum_name s seen_values
        in
        (* Check that there is no multiple `:@I(...)` with the same `I,
         * and that each instance does not contain duplicated assignements
         *)
        let seen_instances =
          let err p name = dup_instance p name prefix in
          let s = check_dup_instances err prefix pu_enum.pu_members in
          SMap.add enum_name (p, s) seen_instances
        in
        (seen_types, seen_values, seen_instances))
      c.c_pu_enums
  in
  (* More precise wrappers around Errors *)
  let err_types p spotted_c_name spotted_enum_name ty =
    let spotted = build spotted_c_name spotted_enum_name in
    dup_case_type p ty spotted
  in
  let err_values p spotted_c_name spotted_enum_name value =
    let spotted = build spotted_c_name spotted_enum_name in
    dup_case_value p value spotted
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
      seen_instances
      c.c_pu_enums
      c_extends;

    (* Gather information about atoms definitions and instances to check
     * if they are correct *)
    List.iter
      ~f:
        (fun (enum_name, { tpu_case_types; tpu_case_values; tpu_members; _ }) ->
        match SMap.find_opt enum_name seen_instances with
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

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let c_name = snd c.c_name in
      (* We restrict PU definition to only happen in a proper class.
       * This limits PU to single inheritance, which we use here by
       * collpasing parent information into single set/maps *)
      match c.c_kind with
      | Ast_defs.Cabstract
      | Ast_defs.Cnormal ->
        check_class env c
      | Ast_defs.Cinterface
      | Ast_defs.Ctrait
      | Ast_defs.Cenum ->
        (match c.c_pu_enums with
        | [] -> ()
        | { pu_name; _ } :: _ ->
          Errors.pu_not_in_class (fst c.c_name) (snd pu_name) c_name)
  end

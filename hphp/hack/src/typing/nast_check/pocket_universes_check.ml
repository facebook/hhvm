(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Cls = Decl_provider.Class

(* Forewords:
 * A pocket universe is made of two main parts:
 * - the `case type / case` statements which describes the content of
 *   each universe
 * - the actual instances (that assign types / values to case statements).
 * In this file, we check that all pocket universes are structurally valid.
 * We check that:
 * - there is no duplication of description or instance assignments.
 * - all of the revelant assignments are provided
 * - non of the providded assignments are missing
 *
 * Since a PU can be extended using classes/traits, we will proceed by
 * scanning all parents of the curent class, to create a description of
 * any previoulsy defined PU, and then proceed by adding the local
 * information. During each addition, we will check that we do not duplicate
 * anything. At the end, we will check for missing / additional instances.
 *
 * Because information comes from the Decl provider and the Aast,
 * some ocaml representation don't strictly align (SMap vs list of pairs,
 * ...). Some effort has been made to share as much code as possible.
 *)

(* Helpers to build the full name of a PU *)
let build prefix name =
  let sep = ":@" in
  prefix ^ sep ^ name

let build_from_origin { pu_class; pu_enum } = build pu_class pu_enum

(* In here we only care about name and positions. Since Ast and Decl
 * representation differ on hint/decl_ty/... we only keep the relevant
 * context in this stripped version
 *)
type nast_pu_member_type = {
  npum_name: Nast.sid;
  npum_types: (pu_origin * Nast.sid) SMap.t;
  npum_exprs: (pu_origin * Nast.sid) SMap.t;
}

and nast_pu_type = {
  npu_name: Nast.sid;
  npu_is_final: bool;
  npu_is_local: bool;  (** set to true only from the current class *)
  npu_case_types: (pu_origin * Nast.sid) SMap.t;
  npu_case_values: (pu_origin * Nast.sid) SMap.t;
  npu_members: nast_pu_member_type SMap.t;
}

(* Translate Decl information (which might be in another file)
 * into nast_pu_*
 *
 * In this section we heavily replace positions to be the ones from the
 * extends / implements / uses statement, to point to a relevant position
 * in the currently checked class.
 *
 * strip_* functions are used to translate from Decl to nast_pu_* types, when
 * no previous occurence of the member/pu was found: no error check is
 * necessary. We do not report duplication in here, because it would create
 * multiple errors for the same source: Since we are in a parent, any
 * duplication will be reported when the parent is checked.
 *)
let strip_decl_member p tpum =
  let npum_types =
    SMap.map (fun (origin, (_, name), _) -> (origin, (p, name))) tpum.tpum_types
  in
  let npum_exprs =
    SMap.map (fun (origin, (_, name)) -> (origin, (p, name))) tpum.tpum_exprs
  in
  let npum_name = (p, snd tpum.tpum_atom) in
  { npum_name; npum_types; npum_exprs }

let strip_decl p tpu =
  let npu_name = (p, snd tpu.tpu_name) in
  let npu_case_types =
    SMap.map
      (fun (origin, tp) -> (origin, (p, snd tp.tp_name)))
      tpu.tpu_case_types
  in
  let npu_case_values =
    SMap.map
      (fun (origin, (_, name), _) -> (origin, (p, name)))
      tpu.tpu_case_values
  in
  {
    npu_name;
    npu_is_local = false;
    npu_is_final = tpu.tpu_is_final;
    npu_case_types;
    npu_case_values;
    npu_members = SMap.map (strip_decl_member p) tpu.tpu_members;
  }

(* We are using the origin fields to discriminate invalid "multiple
 * inheritance" scenario from valid "diamonds" where a common ancestor
 * is defining a PU. Because everything is flatten in Decl provider, we
 * rely on these fields for our analysis
 *)
let check_origin origin0 origin1 =
  let { pu_class = class0; pu_enum = enum0 } = origin0 in
  let { pu_class = class1; pu_enum = enum1 } = origin1 in
  String.equal class0 class1 && String.equal enum0 enum1

(* Generic function to update a map (might be case definitions or
 * member definitions.
 *
 * - kind: the kind of object we are processing (case type, case values,
 *   actual types or actual values). Used for error reporting.
 * - atom_name(optional): name of the member we are currently processing
 * - always_error(optional): see below for more details, but in some cases
 *   we don't perform the origin check and _always_ report an error.
 * - origin/pos/name: current information we want to add to the PU
 *   description. `pos` must be in the same file, as it is used for error
 *   reporting.
 * - map: map of case type / case value / type / value to update
 *)
let merge kind ?atom_name ?(always_error = false) origin pos name map =
  let sid = (pos, name) in
  match SMap.find_opt name map with
  | None -> SMap.add name (origin, sid) map
  | Some (existing, _) ->
    ( if always_error || not (check_origin existing origin) then
      (* multiple inheritance, report an error *)
      let prefix = build_from_origin existing in
      let (err, prefix) =
        match atom_name with
        | None -> (Errors.pu_duplication, prefix)
        | Some atom_name ->
          let prefix = build prefix atom_name in
          (Errors.pu_duplication_in_instance, prefix)
      in
      err pos kind name prefix );
    map

(* In here, we are processing a pu_member_type from an instance located
 * inside a parent of the current class. We add any type/value assignment,
 * if they are not creating invalid duplication.
 *)
let collect_member p existing tpum =
  let atom_name = snd tpum.tpum_atom in
  let npum_types =
    SMap.fold
      (fun _name (origin, tyname, _) types ->
        merge "type" ~atom_name origin p (snd tyname) types)
      tpum.tpum_types
      existing.npum_types
  in
  let npum_exprs =
    SMap.fold
      (fun _name (origin, name) exprs ->
        merge "value" ~atom_name origin p (snd name) exprs)
      tpum.tpum_exprs
      existing.npum_exprs
  in
  let npum_name = (p, snd existing.npum_name) in
  { npum_name; npum_types; npum_exprs }

(* In here, we are processing a pu_enum_type from a parent of the current
 * class. We'll add case types/ case values / members if they are valid.
 *)
let collect_parent p context tpu =
  let name = snd tpu.tpu_name in
  let npu =
    match SMap.find_opt name context with
    | None -> strip_decl p tpu
    | Some existing ->
      let npu_case_types =
        SMap.fold
          (fun _ (origin, tp) types ->
            merge "case type" origin p (snd tp.tp_name) types)
          tpu.tpu_case_types
          existing.npu_case_types
      in
      let npu_case_values =
        SMap.fold
          (fun _ (origin, vname, _) values ->
            merge "case value" origin p (snd vname) values)
          tpu.tpu_case_values
          existing.npu_case_values
      in
      let npu_members =
        SMap.fold
          (fun _ member members ->
            let name = snd member.tpum_atom in
            let member =
              match SMap.find_opt name members with
              | None -> strip_decl_member p member
              | Some nmember -> collect_member p nmember member
            in
            SMap.add name member members)
          tpu.tpu_members
          existing.npu_members
      in
      {
        npu_name = (p, snd tpu.tpu_name);
        npu_is_local = false;
        npu_is_final = tpu.tpu_is_final || existing.npu_is_final;
        npu_case_types;
        npu_case_values;
        npu_members;
      }
  in
  SMap.add name npu context

(* Translate Ast information into nast_pu_*
 *
 * In this part, we are local to the same file, so we keep the positions
 * as is. Check for duplication, and taking extra care to detect if a
 * member duplication is valid (extension) or not.
 *)

(* In here, we are processing a pu_member from an instance located
 * inside a the current class. We add any type/value assignment,
 * if they are not creating invalid duplication.
 *)
let collect_ast_member origin existing pum =
  let atom_name = snd pum.pum_atom in
  let npum_types =
    List.fold
      ~init:existing.npum_types
      ~f:(fun types (sid, _) ->
        let (p, name) = sid in
        merge "type" ~atom_name origin p name types)
      pum.pum_types
  in
  let npum_exprs =
    List.fold
      ~init:existing.npum_exprs
      ~f:(fun exprs (sid, _) ->
        let (p, name) = sid in
        merge "value" ~atom_name origin p name exprs)
      pum.pum_exprs
  in
  { npum_name = pum.pum_atom; npum_types; npum_exprs }

(*
 * strip_* functions are used to translate from Aast to nast_pu_* types, when
 * no previous occurence of the member/pu was found: no error check is
 * necessary.
 * This time, since everything is in the current class, any duplication must
 * be an error so we always report then (see ~always_error)
 *)
let strip_aast_member origin pum =
  let prefix = build_from_origin origin in
  let prefix = build prefix (snd pum.pum_atom) in
  let f msg acc (name, _) =
    let (p, id) = name in
    if SMap.mem id acc then Errors.pu_duplication_in_instance p msg id prefix;
    SMap.add id (origin, name) acc
  in
  let npum_types = List.fold ~init:SMap.empty ~f:(f "type") pum.pum_types in
  let npum_exprs = List.fold ~init:SMap.empty ~f:(f "value") pum.pum_exprs in
  { npum_name = pum.pum_atom; npum_types; npum_exprs }

let strip_aast origin pu =
  let npu_case_types =
    List.fold
      ~init:SMap.empty
      ~f:(fun acc tp ->
        let (p, name) = tp.tp_name in
        merge "case type" ~always_error:true origin p name acc)
      pu.pu_case_types
  in
  let npu_case_values =
    List.fold
      ~init:SMap.empty
      ~f:(fun acc (name, _) ->
        let (p, id) = name in
        merge "case value" ~always_error:true origin p id acc)
      pu.pu_case_values
  in
  let npu_members =
    List.fold
      ~init:SMap.empty
      ~f:(fun acc member ->
        let (p, name) = member.pum_atom in
        let prefix = build_from_origin origin in
        let () =
          if SMap.mem name acc then
            Errors.pu_duplication p "instance" name prefix
        in
        SMap.add name (strip_aast_member origin member) acc)
      pu.pu_members
  in
  {
    npu_name = pu.pu_name;
    npu_is_local = true;
    npu_is_final = pu.pu_is_final;
    npu_case_types;
    npu_case_values;
    npu_members;
  }

(* Collecting information from the current class.
 * Any duplication must be an error.
 *)
let collect c_name context pu =
  let (p, name) = pu.pu_name in
  let origin = { pu_class = c_name; pu_enum = snd pu.pu_name } in
  let npu =
    match SMap.find_opt name context with
    | None -> strip_aast origin pu
    | Some existing ->
      let () =
        if existing.npu_is_local then
          Errors.pu_duplication p "enumeration" name c_name
      in
      let npu_case_types =
        List.fold
          ~init:existing.npu_case_types
          ~f:(fun types tp ->
            let (p, name) = tp.tp_name in
            merge "case type" ~always_error:true origin p name types)
          pu.pu_case_types
      in
      let npu_case_values =
        List.fold
          ~init:existing.npu_case_values
          ~f:(fun values (vname, _) ->
            let (p, name) = vname in
            merge "case value" ~always_error:true origin p name values)
          pu.pu_case_values
      in
      let npu_members =
        List.fold
          ~init:existing.npu_members
          ~f:(fun members member ->
            let (_, name) = member.pum_atom in
            let member =
              match SMap.find_opt name members with
              | None -> strip_aast_member origin member
              | Some nmember -> collect_ast_member origin nmember member
            in
            SMap.add name member members)
          pu.pu_members
      in
      {
        npu_name = pu.pu_name;
        npu_is_local = true;
        npu_is_final = pu.pu_is_final || existing.npu_is_final;
        npu_case_types;
        npu_case_values;
        npu_members;
      }
  in
  SMap.add name npu context

(* Extract pos and name from a parent (trait/extends/...) *)
let get_parent_name_and_pos = function
  | (p, Happly ((_, name), _)) -> Some (p, name)
  | _ -> None

let check_class env c =
  let c_name = snd c.c_name in
  (* trait check: only instances are allowed *)
  let () =
    match c.c_kind with
    | Ast_defs.Ctrait ->
      List.iter
        ~f:(fun pu ->
          (match pu.pu_case_types with
          | [] -> ()
          | { tp_name; _ } :: _ -> Errors.pu_case_in_trait (fst tp_name) "type");
          match pu.pu_case_values with
          | [] -> ()
          | (name, _) :: _ -> Errors.pu_case_in_trait (fst name) "value")
        c.c_pu_enums
    | _ -> ()
  in
  (* Scan all direct parents (but interfaces) to collect PU info and
   * build a vision of the current universe.
   * If some illegal duplication is spotted, report it (extension is allowed,
   * duplication is not).
   * Since all parents will be checked, we only check for duplication.
   * Missing and unknown elements will be checked on the final PU info.
   *)
  let f init hint =
    let open Option in
    let res =
      get_parent_name_and_pos hint >>= fun (p, name) ->
      Decl_provider.get_class env.Nast_check_env.ctx name >>| fun parent ->
      let pu_enums = Cls.pu_enums parent in
      List.fold
        ~init
        ~f:(fun ctx (_, enum) -> collect_parent p ctx enum)
        pu_enums
    in
    Option.value ~default:init res
  in
  let pu_context = List.fold ~init:SMap.empty ~f c.c_extends in
  let pu_context = List.fold ~init:pu_context ~f c.c_uses in
  let pu_context =
    List.fold ~init:pu_context ~f:(fun acc (h, _) -> f acc h) c.c_reqs
  in
  (* Now, scan the local PU information, and check for duplication too *)
  let (pu_context : nast_pu_type SMap.t) =
    List.fold
      ~init:pu_context
      ~f:(fun info enum -> collect c_name info enum)
      c.c_pu_enums
  in
  (* Finally, now that we have a faithful representation of the PUs, we
   * can look for missing or unknown instance elements
   *)
  let keys l = SSet.of_list @@ SMap.keys l in
  let check_missing_and_unknown msg atom_pos atom_name prefix ref actual =
    let ref_keys = keys ref in
    let actual_keys = keys actual in
    let missing = SSet.diff ref_keys actual_keys in
    let unknown = SSet.diff actual_keys ref_keys in
    SSet.iter
      (fun key -> Errors.pu_atom_missing atom_pos atom_name msg prefix key)
      missing;
    SSet.iter
      (fun key -> Errors.pu_atom_unknown atom_pos atom_name msg prefix key)
      unknown
  in
  SMap.iter
    (fun _ npu ->
      let prefix = build c_name (snd npu.npu_name) in
      SMap.iter
        (fun _ nmember ->
          let (p, name) = nmember.npum_name in
          let check msg = check_missing_and_unknown msg p name prefix in
          check "type" npu.npu_case_types nmember.npum_types;
          check "value" npu.npu_case_values nmember.npum_exprs)
        npu.npu_members)
    pu_context

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let c_name = snd c.c_name in
      (* We restrict PU definition to only happen in a proper class.
       * This limits PU to single inheritance, which we use here by
       * collpasing parent information into single set/maps *)
      match c.c_kind with
      | Ast_defs.Ctrait
      | Ast_defs.Cabstract
      | Ast_defs.Cnormal ->
        check_class env c
      | Ast_defs.Cinterface
      | Ast_defs.Cenum ->
        (match c.c_pu_enums with
        | [] -> ()
        | { pu_name; _ } :: _ ->
          Errors.pu_not_in_class (fst c.c_name) (snd pu_name) c_name)
  end

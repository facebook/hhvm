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
  npum_origin: pu_origin;
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
  let npum_origin = tpum.tpum_origin in
  { npum_name; npum_origin; npum_types; npum_exprs }

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
  let npum_origin = existing.npum_origin in
  { npum_name; npum_origin; npum_types; npum_exprs }

(* In here, we are processing a pu_enum_type from a parent of the current
 * class. We'll add case types/ case values / members if they are valid.
 *)
let collect_parent p context tpu =
  let tpu_name = snd tpu.tpu_name in
  let npu =
    match SMap.find_opt tpu_name context with
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
  SMap.add tpu_name npu context

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
  let (p, atom_name) = pum.pum_atom in
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
  (* Prevent repetition of empty atoms, e.g.
   * class C {
   *   enum E {
   *     case string key;
   *     :@K(key = "K");
   *   }
   * }
   *
   * class D extends C {
   *   enum E {
   *     :@K(); // not completely invalid, but strange duplication
   *   }
   * }
   *)
  let prefix = build_from_origin existing.npum_origin in
  if List.is_empty pum.pum_exprs && List.is_empty pum.pum_types then
    Errors.pu_duplication p "instance" atom_name prefix;
  { npum_name = pum.pum_atom; npum_origin = origin; npum_types; npum_exprs }

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
  { npum_name = pum.pum_atom; npum_origin = origin; npum_types; npum_exprs }

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
          ~f:(fun nmembers member ->
            let (_, name) = member.pum_atom in
            let nmember =
              match SMap.find_opt name nmembers with
              | None -> strip_aast_member origin member
              | Some nmember -> collect_ast_member origin nmember member
            in
            SMap.add name nmember nmembers)
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

(* We only allow instances in traits, no `case type` or `case` statements *)
let check_if_trait c =
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

(* Because we are compiling without time information, we have to deal
 * with a special case where a class inherits from PU via traits, but
 * don't declare anything locally. To correctly fix the possible ambiguity
 * we rely on a user annotation to give us the missing pieces.
 *
 * This will be removed/simplified when types in compilation will be a thing.
 *
 * Here we just check if an annotation is necessary and missing, so we can
 * suggest it to the user, or if it is present but useless
 *
 * The attribute format is the following:
 * <<__Pu("enum_name:field0, ..., fieldn", "enum_name:field0,...,fieldp"...)>>
 *)

let process_user_attributes attrs =
  let split_enum s = String.split ~on:':' s in
  let split_fields pos s =
    let fields = String.split ~on:',' s in
    let fields =
      List.fold
        ~init:SSet.empty
        ~f:(fun fields s ->
          let s = String.strip s in
          if String.length s = 0 then
            fields
          else if SSet.mem s fields then (
            Errors.pu_attribute_dup pos "field" s;
            fields
          ) else
            SSet.add s fields)
        fields
    in
    fields
  in
  let get = function
    | (pos, String s) ->
      begin
        match split_enum s with
        | [enum; fields] ->
          let enum = String.strip enum in
          let fields = split_fields pos fields in
          Some (pos, enum, fields)
        | _ -> None
      end
    | _ -> None
  in
  List.fold
    ~init:SMap.empty
    ~f:(fun acc { ua_name; ua_params } ->
      let (pos, ua_name) = ua_name in
      if String.equal ua_name SN.UserAttributes.uaPu then
        List.fold
          ~init:acc
          ~f:(fun acc expr ->
            match get expr with
            | None ->
              Errors.pu_attribute_invalid pos;
              acc
            | Some (pos, enum, fields) ->
              if SMap.mem enum acc then (
                Errors.pu_attribute_dup pos "attribute" enum;
                acc
              ) else
                SMap.add enum (pos, fields) acc)
          ua_params
      else
        acc)
    attrs

type attribute_info = {
  pos: Pos.t;
  missing: SSet.t;
  unknown: SSet.t;
  useless: SSet.t;
}

(* Check if a Pu attribute is needed or not, to help compilation involving
 * traits.
 *
 * If it is missing, returns the necessary attribute to be added.
 * If it is present and valid, ok
 * If it is present and mentions useless/unknown fields, report that
 *)
let validate_attributes pu_members npu attribute =
  (* fields used by the local code *)
  let local_fields =
    List.fold
      ~init:SSet.empty
      ~f:(fun fields { pum_exprs; _ } ->
        List.fold
          ~init:fields
          ~f:(fun fields ((_, name), _) -> SSet.add name fields)
          pum_exprs)
      pu_members
  in
  (* fields gathered from the context (from traits parents) *)
  let context_fields =
    match npu with
    | None -> SSet.empty
    | Some npu -> SSet.of_list @@ SMap.keys npu.npu_case_values
  in
  (* fields mentionned in the attribute, if any *)
  let (pos, attribute_fields) =
    match attribute with
    | None -> (Pos.none, SSet.empty)
    | Some (pos, fields) -> (pos, fields)
  in
  let useless = SSet.inter attribute_fields local_fields in
  let unknown = SSet.diff attribute_fields context_fields in
  let missing = SSet.diff context_fields local_fields in
  let missing = SSet.diff missing attribute_fields in
  { pos; missing; unknown; useless }

let generate_attribute enum missing =
  let fields = SSet.elements missing in
  let fields = String.concat ~sep:", " fields in
  sprintf "\"%s: %s\"" enum fields

let check_attributes c_pos c_name c_pu_enums attributes pu_context =
  (* Check every pu_member structure against the attributes to see
   * if they are in sync. Gather the one that are ok, so we can
   * display the correct attribute that is necessary on the class
   *)
  let validate_attributes name pu_members npu attribute =
    let { pos; missing; unknown; useless } =
      validate_attributes pu_members npu attribute
    in
    SSet.iter
      (fun s -> Errors.pu_attribute_err pos "useless" c_name name s)
      useless;
    SSet.iter
      (fun s -> Errors.pu_attribute_err pos "unknown" c_name name s)
      unknown;
    missing
  in
  let update_missings name missing missings =
    if SSet.is_empty missing then
      missings
    else
      generate_attribute name missing :: missings
  in
  (* Here, we collect the name of all Pu blocks that are in the current
   * class, and any missing information returned byt `validate_attributes`.
   * Unknown and useless attributes are directly reported as errors.
   *)
  let (seen, missing) =
    List.fold
      ~init:(SSet.empty, [])
      ~f:(fun (seen, missings) pu_enum ->
        let name = snd pu_enum.pu_name in
        let npu = SMap.find_opt name pu_context in
        let attribute = SMap.find_opt name attributes in
        let missing =
          validate_attributes name pu_enum.pu_members npu attribute
        in
        let missings = update_missings name missing missings in
        let seen = SSet.add name seen in
        (seen, missings))
      c_pu_enums
  in
  (* Now, we check the more general pu definition (`pu_context`) to
   * gather more missing information. We use the `seen` set to
   * avoid looking twice at local information, and only focus on what
   * comes from the extended definition.
   *
   * Since pu_enum duplication is already caught by the nast check, we don't
   * need to update the `seen` set in this case.
   *)
  let missing =
    SMap.fold
      (fun name npu missings ->
        if SSet.mem name seen then
          missings
        else
          let attribute = SMap.find_opt name attributes in
          let missing = validate_attributes name [] (Some npu) attribute in
          update_missings name missing missings)
      pu_context
      missing
  in
  if not (List.is_empty missing) then
    let s = String.concat missing ~sep:", " in
    Errors.pu_attribute_suggestion c_pos c_name s

let check_class env c =
  let (c_pos, c_name) = c.c_name in
  (* trait check: only instances are allowed *)
  let () = check_if_trait c in
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
  let pu_context = List.fold ~init:SMap.empty ~f c.c_uses in
  let pu_from_traits = not (SMap.is_empty pu_context) in
  let pu_context = List.fold ~init:pu_context ~f c.c_extends in
  let pu_context =
    List.fold ~init:pu_context ~f:(fun acc (h, _) -> f acc h) c.c_reqs
  in
  (* Attribute checking: see T65158651 for why we need to check this
   * annotated attributes
   *)
  let () =
    (* Process attributes in the file *)
    let attributes = process_user_attributes c.c_user_attributes in
    if pu_from_traits then
      check_attributes c_pos c_name c.c_pu_enums attributes pu_context
    else if not (SMap.is_empty attributes) then
      Errors.pu_attribute_not_necessary c_pos c_name
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

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open Typing_defs
module Cls = Decl_provider.Class

type member_class =
  | Class_set of SSet.t
  | Subclasses_of of string

type action_internal =
  | IClass of string
  | IRecord of string
  | IMember of member_class * member
  | IFunction of string
  | IGConst of string

let member_class_to_string (mc : member_class) : string =
  match mc with
  | Subclasses_of s -> "Subclasses_of " ^ s
  | Class_set ss -> "Class_set " ^ (SSet.elements ss |> String.concat ~sep:",")

let action_internal_to_string (action : action_internal) : string =
  match action with
  | IClass s -> "IClass " ^ s
  | IRecord s -> "IRecord " ^ s
  | IFunction s -> "IFunction " ^ s
  | IGConst s -> "IGConst " ^ s
  | IMember (member_class, member) ->
    Printf.sprintf
      "IMember(%s,%s)"
      (member_class_to_string member_class)
      (ServerCommandTypes.Find_refs.member_to_string member)

let process_fun_id target_fun id =
  if String.equal target_fun (snd id) then
    Pos.Map.singleton (fst id) (snd id)
  else
    Pos.Map.empty

let check_if_extends_class ctx target_class_name class_name =
  let class_ = Decl_provider.get_class ctx class_name in
  match class_ with
  | Some cls
    when Cls.has_ancestor cls target_class_name
         || Cls.requires_ancestor cls target_class_name ->
    true
  | _ -> false

let is_target_class ctx target_classes class_name =
  match target_classes with
  | Class_set s -> SSet.mem s class_name
  | Subclasses_of s ->
    String.equal s class_name || check_if_extends_class ctx s class_name

let process_member_id
    ctx target_classes target_member class_name mid ~is_method ~is_const =
  let member_name = snd mid in
  let is_target =
    match target_member with
    | Method target_name -> is_method && String.equal member_name target_name
    | Property target_name ->
      (not is_method)
      && (not is_const)
      && String.equal (String_utils.lstrip member_name "$") target_name
    | Class_const target_name ->
      is_const && String.equal member_name target_name
    | Typeconst _ -> false
  in
  if is_target && is_target_class ctx target_classes class_name then
    Pos.Map.singleton (fst mid) (class_name ^ "::" ^ snd mid)
  else
    Pos.Map.empty

let process_class_id target_class cid mid_option =
  if String.equal target_class (snd cid) then
    let class_name =
      match mid_option with
      | None -> snd cid
      | Some n -> snd cid ^ "::" ^ snd n
    in
    Pos.Map.singleton (fst cid) class_name
  else
    Pos.Map.empty

let process_taccess
    ctx target_classes target_typeconst (class_name, tconst_name, p) =
  if
    is_target_class ctx target_classes class_name
    && String.equal target_typeconst tconst_name
  then
    Pos.Map.singleton p (class_name ^ "::" ^ tconst_name)
  else
    Pos.Map.empty

let process_gconst_id target_gconst id =
  if String.equal target_gconst (snd id) then
    Pos.Map.singleton (fst id) (snd id)
  else
    Pos.Map.empty

let add_if_extends_class ctx target_class_name class_name acc =
  if check_if_extends_class ctx target_class_name class_name then
    SSet.add acc class_name
  else
    acc

let find_child_classes ctx target_class_name naming_table files =
  SharedMem.invalidate_caches ();
  Relative_path.Set.fold files ~init:SSet.empty ~f:(fun fn acc ->
      try
        let { FileInfo.classes; _ } =
          Naming_table.get_file_info_unsafe naming_table fn
        in
        List.fold_left classes ~init:acc ~f:(fun acc cid ->
            add_if_extends_class ctx target_class_name (snd cid) acc)
      with Caml.Not_found -> acc)

let get_origin_class_name ctx class_name member =
  let origin =
    match member with
    | Method method_name ->
      begin
        match Decl_provider.get_class ctx class_name with
        | Some class_ ->
          let get_origin_class meth =
            match meth with
            | Some meth -> Some meth.ce_origin
            | None -> None
          in
          let origin = get_origin_class (Cls.get_method class_ method_name) in
          if Option.is_some origin then
            origin
          else
            get_origin_class (Cls.get_smethod class_ method_name)
        | None -> None
      end
    | Property _
    | Class_const _
    | Typeconst _ ->
      None
  in
  Option.value origin ~default:class_name

let get_child_classes_files ctx class_name =
  match Naming_provider.get_type_kind ctx class_name with
  | Some Naming_types.TClass ->
    (* Find the files that contain classes that extend class_ *)
    let cid_hash = Typing_deps.Dep.make (Typing_deps.Dep.Class class_name) in
    let extend_deps =
      Decl_compare.get_extend_deps
        cid_hash
        (Typing_deps.DepSet.singleton cid_hash)
    in
    Typing_deps.get_files extend_deps
  | _ -> Relative_path.Set.empty

let get_deps_set ctx classes =
  SSet.fold
    classes
    ~f:
      begin
        fun class_name acc ->
        match Naming_provider.get_type_path ctx class_name with
        | None -> acc
        | Some fn ->
          let dep = Typing_deps.Dep.Class class_name in
          let ideps = Typing_deps.get_ideps dep in
          let files = Typing_deps.get_files ideps in
          let files = Relative_path.Set.add files fn in
          Relative_path.Set.union files acc
      end
    ~init:Relative_path.Set.empty

let get_deps_set_function ctx f_name =
  match Naming_provider.get_fun_path ctx f_name with
  | Some fn ->
    let dep = Typing_deps.Dep.Fun f_name in
    let ideps = Typing_deps.get_ideps dep in
    let files = Typing_deps.get_files ideps in
    Relative_path.Set.add files fn
  | None -> Relative_path.Set.empty

let get_deps_set_gconst ctx cst_name =
  match Naming_provider.get_const_path ctx cst_name with
  | Some fn ->
    let dep = Typing_deps.Dep.GConst cst_name in
    let ideps = Typing_deps.get_ideps dep in
    let files = Typing_deps.get_files ideps in
    Relative_path.Set.add files fn
  | None -> Relative_path.Set.empty

let fold_one_tast ctx target acc symbol =
  let module SO = SymbolOccurrence in
  let SO.{ type_; pos; name; _ } = symbol in
  Pos.Map.union acc
  @@
  match (target, type_) with
  | (IMember (classes, Typeconst tc), SO.Typeconst (c_name, tc_name)) ->
    process_taccess ctx classes tc (c_name, tc_name, pos)
  | (IMember (classes, member), SO.Method (c_name, m_name))
  | (IMember (classes, member), SO.ClassConst (c_name, m_name))
  | (IMember (classes, member), SO.Property (c_name, m_name)) ->
    let mid = (pos, m_name) in
    let is_method =
      match type_ with
      | SO.Method _ -> true
      | _ -> false
    in
    let is_const =
      match type_ with
      | SO.ClassConst _ -> true
      | _ -> false
    in
    process_member_id ctx classes member c_name mid ~is_method ~is_const
  | (IFunction fun_name, SO.Function) -> process_fun_id fun_name (pos, name)
  | (IClass c, SO.Class) -> process_class_id c (pos, name) None
  | (IGConst cst_name, SO.GConst) -> process_gconst_id cst_name (pos, name)
  | _ -> Pos.Map.empty

let find_refs
    (ctx : Provider_context.t)
    (target : action_internal)
    (acc : (string * Pos.t) list)
    (files : Relative_path.t list) : (string * Pos.t) list =
  (* The helper function 'results_from_tast' takes a tast, looks at all *)
  (* use-sites in the tast e.g. "foo(1)" is a use-site of symbol foo,   *)
  (* and returns a map from use-site-position to name of the symbol.    *)
  let results_from_tast (_file, tast) : string Pos.Map.t =
    IdentifySymbolService.all_symbols ctx tast
    |> List.filter ~f:(fun symbol -> not symbol.SymbolOccurrence.is_declaration)
    |> List.fold ~init:Pos.Map.empty ~f:(fold_one_tast ctx target)
  in
  (* These are the tasts for all the 'fileinfo_l' passed in *)
  let tasts_of_files : (Relative_path.t * Tast.program) list =
    List.map files ~f:(fun path ->
        let (_ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        (path, tast))
  in
  Hh_logger.debug "find_refs.target: %s" (action_internal_to_string target);
  if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
    List.iter tasts_of_files (fun (file, tast) ->
        let fn = Relative_path.to_absolute file in
        let tast = Tast.show_program tast in
        let len = String.length tast in
        let tast = String_utils.truncate 2048 tast in
        Hh_logger.debug "find_refs.tast: %s\nlen=%d\n%s\n\n\n\n" fn len tast);
  (* A list of all use-sites with their string-name of target *)
  let results : string Pos.Map.t list =
    List.map ~f:results_from_tast tasts_of_files
  in
  (* Turn that list into a map from use-sites to string-name-of-target *)
  let results : string Pos.Map.t =
    List.fold results ~init:Pos.Map.empty ~f:Pos.Map.union
  in
  (* We actually just want a list of string-name-of-target, use-site-position *)
  (* Some callers e.g. LSP will simply discard the string-name-of-target.     *)
  let acc_results : (string * Pos.t) list =
    Pos.Map.fold (fun p str acc -> (str, p) :: acc) results acc
  in
  acc_results

let find_refs_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(target : action_internal) : (string * Pos.Map.key) list =
  let symbols = IdentifySymbolService.all_symbols_ctx ~ctx ~entry in
  let results =
    symbols
    |> List.filter ~f:(fun symbol -> not symbol.SymbolOccurrence.is_declaration)
    |> List.fold ~init:Pos.Map.empty ~f:(fold_one_tast ctx target)
  in
  Pos.Map.fold (fun p str acc -> (str, p) :: acc) results []

let parallel_find_refs workers files target ctx =
  MultiWorker.call
    workers
    ~job:(find_refs ctx target)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers files)

let get_definitions ctx = function
  | IMember (Class_set classes, Method method_name) ->
    SSet.fold classes ~init:[] ~f:(fun class_name acc ->
        match Decl_provider.get_class ctx class_name with
        | Some class_ ->
          let add_meth get acc =
            match get method_name with
            | Some meth when String.equal meth.ce_origin (Cls.name class_) ->
              let pos = get_pos @@ Lazy.force meth.ce_type in
              (method_name, pos) :: acc
            | _ -> acc
          in
          let acc = add_meth (Cls.get_method class_) acc in
          let acc = add_meth (Cls.get_smethod class_) acc in
          acc
        | None -> acc)
  | IMember (Class_set classes, Class_const class_const_name) ->
    SSet.fold classes ~init:[] ~f:(fun class_name acc ->
        match Decl_provider.get_class ctx class_name with
        | Some class_ ->
          let add_class_const get acc =
            match get class_const_name with
            | Some class_const
              when String.equal class_const.cc_origin (Cls.name class_) ->
              let pos = class_const.cc_pos in
              (class_const_name, pos) :: acc
            | _ -> acc
          in
          let acc = add_class_const (Cls.get_const class_) acc in
          acc
        | None -> acc)
  | IClass class_name ->
    Option.value
      ~default:[]
      (Naming_provider.get_type_kind ctx class_name >>= function
       | Naming_types.TClass ->
         Decl_provider.get_class ctx class_name >>= fun class_ ->
         Some [(class_name, Cls.pos class_)]
       | Naming_types.TTypedef ->
         Decl_provider.get_typedef ctx class_name >>= fun type_ ->
         Some [(class_name, type_.td_pos)]
       | Naming_types.TRecordDef ->
         Decl_provider.get_record_def ctx class_name >>= fun rd ->
         Some [(class_name, rd.rdt_pos)])
  | IRecord record_name ->
    begin
      match Decl_provider.get_record_def ctx record_name with
      | Some rd -> [(record_name, rd.rdt_pos)]
      | None -> []
    end
  | IFunction fun_name ->
    begin
      match Decl_provider.get_fun ctx fun_name with
      | Some { fe_pos; _ } -> [(fun_name, fe_pos)]
      | _ -> []
    end
  | IGConst _
  | IMember (Subclasses_of _, _)
  | IMember (_, (Property _ | Typeconst _)) ->
    (* this code path is used only in ServerRefactor, we can update it at some
       later time *)
    []

let find_references ctx workers target include_defs files =
  let len = List.length files in
  Hh_logger.debug "find_references: %d files" len;
  let results =
    if len < 10 then
      find_refs ctx target [] files
    else
      parallel_find_refs workers files target ctx
  in
  let () =
    Hh_logger.debug "find_references: %d results" (List.length results)
  in
  if include_defs then
    let defs = get_definitions ctx target in
    let () = Hh_logger.debug "find_references: +%d defs" (List.length defs) in
    List.rev_append defs results
  else
    results

let get_dependent_files_function ctx _workers f_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_function ctx f_name

let get_dependent_files_gconst ctx _workers cst_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_gconst ctx cst_name

let get_dependent_files ctx _workers input_set =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set ctx input_set

let result_to_ide_message x =
  Option.map x ~f:(fun (symbol_name, references) ->
      let references = List.map references ~f:Ide_api_types.pos_to_file_range in
      (symbol_name, references))

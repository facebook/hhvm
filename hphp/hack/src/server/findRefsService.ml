(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Option.Monad_infix
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open Typing_defs


(* The class containing the member can be specified in two ways:
 * - Class_set - as an explicit, pre-computed set of names, which are then
 *   compared using string comparison
 * - Subclasses_of - the class's name, in which comparison will use the
 *   subtyping relation
 *)
type member_class =
  | Class_set of SSet.t
  | Subclasses_of of string

type action_internal  =
  | IClass of string
  | IMember of member_class * member
  | IFunction of string
  | IGConst of string


let process_fun_id target_fun id =
  if target_fun = (snd id)
  then Pos.Map.singleton (fst id) (snd id)
  else Pos.Map.empty

let check_if_extends_class tcopt target_class_name class_name =
  let class_ = Typing_lazy_heap.get_class tcopt class_name in
  match class_ with
  | Some { Typing_defs.tc_ancestors = imps; tc_req_ancestors_extends = req_extends; _ }
      when (SMap.mem imps target_class_name || SSet.mem req_extends target_class_name) -> true
  | _ -> false

let is_target_class tcopt target_classes class_name =
  match target_classes with
  | Class_set s -> SSet.mem s class_name
  | Subclasses_of s ->
    s = class_name || check_if_extends_class tcopt s class_name

let process_member_id tcopt target_classes target_member class_name mid
    ~is_method ~is_const =
  let member_name = snd mid in
  let is_target = match target_member with
    | Method target_name  -> is_method && (member_name = target_name)
    | Property target_name ->
      (not is_method) && (not is_const) &&
        ((String_utils.lstrip member_name "$") = target_name)
    | Class_const target_name -> is_const && (member_name = target_name)
    | Typeconst _ -> false
  in
  if is_target && is_target_class tcopt target_classes class_name
  then Pos.Map.singleton (fst mid) (class_name ^ "::" ^ (snd mid))
  else Pos.Map.empty

let process_class_id target_class cid mid_option =
   if target_class = (snd cid)
   then begin
     let class_name = match mid_option with
     | None -> snd cid
     | Some n -> (snd cid)^"::"^(snd n) in
     Pos.Map.singleton (fst cid) class_name
   end
   else Pos.Map.empty

let process_taccess tcopt target_classes target_typeconst
    (class_name, tconst_name, p) =
  if (is_target_class tcopt target_classes class_name) &&
    (target_typeconst = tconst_name)
  then Pos.Map.singleton p (class_name ^ "::" ^ tconst_name)
  else Pos.Map.empty

let process_gconst_id target_gconst id =
  if target_gconst = (snd id)
  then Pos.Map.singleton (fst id) (snd id)
  else Pos.Map.empty

let add_if_extends_class tcopt target_class_name class_name acc =
  if check_if_extends_class tcopt target_class_name class_name
  then SSet.add acc class_name else acc

let find_child_classes tcopt target_class_name files_info files =
  SharedMem.invalidate_caches();
  Relative_path.Set.fold files ~init:SSet.empty ~f:begin fun fn acc ->
    (try
      let { FileInfo.classes; _ } =
        Relative_path.Map.find_unsafe files_info fn in
      List.fold_left classes ~init:acc ~f:begin fun acc cid ->
        add_if_extends_class tcopt target_class_name (snd cid) acc
      end
    with Not_found ->
      acc)
  end

let get_origin_class_name tcopt class_name member =
  let origin = match member with
    | Method method_name ->
      begin match Typing_lazy_heap.get_class tcopt class_name with
      | Some class_ ->
        let get_origin_class meths meth = match SMap.get meths meth with
          | Some meth -> Some meth.ce_origin
          | None -> None
        in
        let origin_from_methods = get_origin_class class_.tc_methods method_name in
        let origin_from_smethods = get_origin_class class_.tc_smethods method_name in
        let origin = Option.first_some origin_from_methods origin_from_smethods in
        origin
      | None -> None
      end
    | Property _ | Class_const _ | Typeconst _ -> None
  in
  Option.value origin ~default:class_name

let get_child_classes_files class_name =
  match Naming_heap.TypeIdHeap.get class_name with
  | Some (_, `Class) ->
    (* Find the files that contain classes that extend class_ *)
    let cid_hash =
      Typing_deps.Dep.make (Typing_deps.Dep.Class class_name) in
    let extend_deps =
      Decl_compare.get_extend_deps cid_hash
        (Typing_deps.DepSet.singleton cid_hash)
    in
    Typing_deps.get_files extend_deps
  | _ ->
    Relative_path.Set.empty

let get_deps_set classes =
  let get_filename class_name =
    Naming_heap.TypeIdHeap.get class_name >>= fun (pos, _) ->
    Some (FileInfo.get_pos_filename pos)
  in
  SSet.fold classes ~f:begin fun class_name acc ->
    match get_filename class_name with
    | None -> acc
    | Some fn ->
      let dep = Typing_deps.Dep.Class class_name in
      let bazooka = Typing_deps.get_bazooka dep in
      let files = Typing_deps.get_files bazooka in
      let files = Relative_path.Set.add files fn in
      Relative_path.Set.union files acc
  end ~init:Relative_path.Set.empty

let get_deps_set_function f_name =
  match Naming_heap.FunPosHeap.get f_name with
  | Some pos ->
    let fn = FileInfo.get_pos_filename pos in
    let dep = Typing_deps.Dep.Fun f_name in
    let bazooka = Typing_deps.get_bazooka dep in
    let files = Typing_deps.get_files bazooka in
    Relative_path.Set.add files fn
  | None ->
    Relative_path.Set.empty

let get_deps_set_gconst cst_name =
  match Naming_heap.ConstPosHeap.get cst_name with
  | Some pos ->
    let fn = FileInfo.get_pos_filename pos in
    let dep = Typing_deps.Dep.GConst cst_name in
    let bazooka = Typing_deps.get_bazooka dep in
    let files = Typing_deps.get_files bazooka in
    Relative_path.Set.add files fn
  | None ->
    Relative_path.Set.empty

let find_refs tcopt target acc fileinfo_l =
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  let tasts = ServerIdeUtils.recheck tcopt fileinfo_l in
  let results =
    let module SO = SymbolOccurrence in
    List.fold tasts ~init:Pos.Map.empty ~f:begin fun acc (_, tast) ->
      IdentifySymbolService.all_symbols tast
      |> List.filter ~f:(fun symbol -> not symbol.SO.is_declaration)
      |> List.fold ~init:Pos.Map.empty ~f:begin fun acc symbol ->
        let SO.{type_; pos; name; _} = symbol in
        Pos.Map.union acc @@
        match target, type_ with
        | IMember (classes, Typeconst tc), SO.Typeconst (c_name, tc_name) ->
          process_taccess tcopt classes tc (c_name, tc_name, pos)
        | IMember (classes, member), SO.Method (c_name, m_name)
        | IMember (classes, member), SO.ClassConst (c_name, m_name)
        | IMember (classes, member), SO.Property (c_name, m_name) ->
          let mid = (pos, m_name) in
          let is_method = match type_ with SO.Method _ -> true | _ -> false in
          let is_const = match type_ with SO.ClassConst _ -> true | _ -> false in
          process_member_id tcopt classes member c_name mid
            ~is_method ~is_const
        | IFunction fun_name, SO.Function ->
          process_fun_id fun_name (pos, name)
        | IClass c, SO.Class ->
          process_class_id c (pos, name) None
        | IGConst cst_name, SO.GConst ->
          process_gconst_id cst_name (pos, name)
        | _ -> Pos.Map.empty
      end
      |> Pos.Map.union acc
    end
  in
  Pos.Map.fold (fun p str acc -> (str, p) :: acc) results acc

let parallel_find_refs workers fileinfo_l target tcopt =
  MultiWorker.call
    workers
    ~job:(find_refs tcopt target)
    ~neutral:([])
    ~merge:(List.rev_append)
    ~next:(MultiWorker.next workers fileinfo_l)

let get_definitions tcopt = function
  | IMember (Class_set classes, Method method_name) ->
    SSet.fold classes ~init:[] ~f:begin fun class_name acc ->
      match Typing_lazy_heap.get_class tcopt class_name with
      | Some class_ ->
        let add_meth meths acc = match SMap.get meths method_name with
          | Some meth when meth.ce_origin = class_.tc_name ->
            let pos = Reason.to_pos (fst @@ Lazy.force meth.ce_type) in
            (method_name, pos) :: acc
          | _ -> acc
        in
        let acc = add_meth class_.tc_methods acc in
        let acc = add_meth class_.tc_smethods acc in
        acc
      | None -> acc
    end
  | IMember (Class_set classes, Class_const class_const_name) ->
    SSet.fold classes ~init:[] ~f:begin fun class_name acc ->
      match Typing_lazy_heap.get_class tcopt class_name with
      | Some class_ ->
        let add_class_const class_consts acc = match SMap.get class_consts class_const_name with
          | Some class_const when class_const.cc_origin = class_.tc_name ->
            let pos = class_const.cc_pos in
            (class_const_name, pos) :: acc
          | _ -> acc
        in
        let acc = add_class_const class_.tc_consts acc in
        acc
      | None -> acc
    end
  | IClass class_name ->
    Option.value ~default:[] begin Naming_heap.TypeIdHeap.get class_name >>=
    function (_, `Class) -> Typing_lazy_heap.get_class tcopt class_name >>=
      fun class_ -> Some([(class_name, class_.tc_pos)])
    | (_, `Typedef) -> Typing_lazy_heap.get_typedef tcopt class_name >>=
      fun type_ -> Some([class_name, type_.td_pos])
    end
  | IFunction fun_name ->
    begin match Typing_lazy_heap.get_fun tcopt fun_name with
      | Some fun_ -> [fun_name, fun_.ft_pos]
      | None -> []
    end
  | IGConst _
  | IMember (Subclasses_of _, _)
  | IMember (_, (Property _ | Typeconst _)) ->
    (* this code path is used only in ServerRefactor, we can update it at some
       later time *)
    []

let find_references tcopt workers target include_defs
      files_info files =
  let fileinfo_l = Relative_path.Set.fold files ~f:begin fun fn acc ->
    match Relative_path.Map.get files_info fn with
    | Some fi -> (fn, fi) :: acc
    | None -> acc
  end ~init:[] in
  let results =
    if List.length fileinfo_l < 10 then
      find_refs tcopt target [] fileinfo_l
    else
      parallel_find_refs workers fileinfo_l target tcopt
    in
  if include_defs then
    let defs = get_definitions tcopt target in
    List.rev_append defs results
  else
    results

let get_dependent_files_function _workers f_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_function f_name

let get_dependent_files_gconst _workers cst_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_gconst cst_name

let get_dependent_files _workers input_set =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set input_set

let result_to_ide_message x =
    Option.map x ~f:begin fun (symbol_name, references) ->
      let references =
        List.map references ~f:Ide_api_types.pos_to_file_range in
      (symbol_name, references)
    end

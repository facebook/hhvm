(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Reordered_argument_collections
open Typing_defs

type member = Ai.ServerFindRefs.member =
  | Method of string
  | Property of string
  | Class_const of string
  | Typeconst of string

type action = Ai.ServerFindRefs.action =
  | Class of string
  | Member of string * member
  | Function of string

type action_internal  =
  | IClass of string
  | IMember of SSet.t * member
  | IFunction of string

type result = (string * Pos.absolute) list

let process_fun_id results_acc target_fun id =
  if target_fun = (snd id)
  then results_acc := Pos.Map.add (fst id) (snd id) !results_acc

let process_member_id results_acc target_classes target_member
    class_ id _ _ ~is_method ~is_const =
  let member_name = snd id in
  let is_target = match target_member with
    | Method target_name  -> is_method && (member_name = target_name)
    | Property target_name ->
      (not is_method) && (not is_const) &&
        ((Utils.lstrip member_name "$") = target_name)
    | Class_const target_name -> is_const && (member_name = target_name)
    | Typeconst _ -> false
  in
  if not is_target then () else
  let class_name = class_.Typing_defs.tc_name in
  if SSet.mem target_classes class_name then
    results_acc :=
      Pos.Map.add (fst id) (class_name ^ "::" ^ (snd id)) !results_acc

let process_constructor results_acc target_classes target_member class_ _ p =
  process_member_id
    results_acc target_classes target_member class_ (p, "__construct")
    () () ~is_method:true ~is_const:false

let process_class_id results_acc target_classes cid mid_option =
   if (SSet.mem target_classes (snd cid))
   then begin
     let class_name = match mid_option with
     | None -> snd cid
     | Some n -> (snd cid)^"::"^(snd n) in
     results_acc := Pos.Map.add (fst cid) class_name !results_acc
   end

let process_taccess results_acc target_classes target_typeconst
    class_ typeconst p =
  let class_name = class_.tc_name in
  let tconst_name = (snd typeconst.ttc_name) in
  if (SSet.mem target_classes class_name) &&
    (target_typeconst = tconst_name) then
  results_acc :=
    Pos.Map.add p (class_name ^ "::" ^ tconst_name) !results_acc

let attach_hooks results_acc = function
  | IMember (classes, ((Method _ | Property _ | Class_const _) as member)) ->
    let process_member_id =
      process_member_id results_acc classes member in
    Typing_hooks.attach_cmethod_hook process_member_id;
    Typing_hooks.attach_smethod_hook process_member_id;
    Typing_hooks.attach_constructor_hook
      (process_constructor results_acc classes member);
  | IMember (classes, Typeconst t) ->
    Typing_hooks.attach_taccess_hook (process_taccess results_acc classes t)
  | IFunction fun_name ->
    Typing_hooks.attach_fun_id_hook (process_fun_id results_acc fun_name)
  | IClass c ->
    let classes = SSet.singleton c in
    Decl_hooks.attach_class_id_hook (process_class_id results_acc classes)

let detach_hooks () =
  Decl_hooks.remove_all_hooks ();
  Typing_hooks.remove_all_hooks ()

let check_if_extends_class tcopt target_class_name class_name acc =
  let class_ = Typing_lazy_heap.get_class tcopt class_name in
  match class_ with
  | None -> acc
  | Some { Typing_defs.tc_ancestors = imps; _ }
      when SMap.mem target_class_name imps -> SSet.add acc class_name
  | _ -> acc

let find_child_classes tcopt target_class_name files_info files =
  SharedMem.invalidate_caches();
  Relative_path.Set.fold files ~init:SSet.empty ~f:begin fun fn acc ->
    (try
      let { FileInfo.classes; _ } =
        Relative_path.Map.find_unsafe fn files_info in
      List.fold_left classes ~init:acc ~f:begin fun acc cid ->
        check_if_extends_class tcopt target_class_name (snd cid) acc
      end
    with Not_found ->
      acc)
  end

let get_child_classes_files tcopt class_name =
  match Typing_lazy_heap.get_class tcopt class_name with
  | Some class_ ->
    (* Find the files that contain classes that extend class_ *)
    let cid_hash =
      Typing_deps.Dep.make (Typing_deps.Dep.Class class_.tc_name) in
    let extend_deps =
      Decl_compare.get_extend_deps cid_hash
        (Typing_deps.DepSet.singleton cid_hash)
    in
    Typing_deps.get_files extend_deps
  | _ ->
    Relative_path.Set.empty

let get_deps_set tcopt classes =
  SSet.fold classes ~f:begin fun class_name acc ->
    match Typing_lazy_heap.get_class tcopt class_name with
    | Some class_ ->
        (* Get all files with dependencies on this class *)
        let fn = Pos.filename class_.tc_pos in
        let dep = Typing_deps.Dep.Class class_.tc_name in
        let bazooka = Typing_deps.get_bazooka dep in
        let files = Typing_deps.get_files bazooka in
        let files = Relative_path.Set.add files fn in
        Relative_path.Set.union files acc
    | None -> acc
  end ~init:Relative_path.Set.empty

let get_deps_set_function tcopt f_name =
  match Typing_lazy_heap.get_fun tcopt f_name with
  | Some fun_ ->
    let fn = Pos.filename fun_.ft_pos in
    let dep = Typing_deps.Dep.Fun f_name in
    let bazooka = Typing_deps.get_bazooka dep in
    let files = Typing_deps.get_files bazooka in
    Relative_path.Set.add files fn
  | None ->
    Relative_path.Set.empty

let find_refs target acc fileinfo_l =
  let results_acc = ref Pos.Map.empty in
  attach_hooks results_acc target;
  let tcopt = TypecheckerOptions.permissive in
  ServerIdeUtils.recheck tcopt fileinfo_l;
  detach_hooks ();
  Pos.Map.fold begin fun p str acc ->
    (str, p) :: acc
  end !results_acc acc

let parallel_find_refs workers fileinfo_l target =
  MultiWorker.call
    workers
    ~job:(find_refs target)
    ~neutral:([])
    ~merge:(List.rev_append)
    ~next:(Bucket.make fileinfo_l)

let get_definitions tcopt = function
  | IMember (classes, Method method_name) ->
    SSet.fold classes ~init:[] ~f:begin fun class_name acc ->
      match Typing_lazy_heap.get_class tcopt class_name with
      | Some class_ ->
        let add_meth meths acc = match SMap.get meths method_name with
          | Some meth when meth.ce_origin = class_.tc_name ->
            let pos = Reason.to_pos (fst meth.ce_type) in
            (method_name, pos) :: acc
          | _ -> acc
        in
        let acc = add_meth class_.tc_methods acc in
        let acc = add_meth class_.tc_smethods acc in
        acc
      | None -> acc
    end
  | IClass class_name ->
    begin match Typing_lazy_heap.get_class tcopt class_name with
      | Some class_ -> [(class_name, class_.tc_pos)]
      | None -> []
    end
  | IFunction fun_name ->
    begin match Typing_lazy_heap.get_fun tcopt fun_name with
      | Some fun_ -> [fun_name, fun_.ft_pos]
      | None -> []
    end
  | IMember (_, (Property _ | Class_const _ | Typeconst _)) ->
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
      find_refs target [] fileinfo_l
    else
      parallel_find_refs workers fileinfo_l target
    in
  if include_defs then
    let defs = get_definitions tcopt target in
    List.rev_append defs results
  else
    results

let get_dependent_files_function tcopt _workers f_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_function tcopt f_name

let get_dependent_files tcopt _workers input_set =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set tcopt input_set

let print results =
  List.iter (List.rev results) (fun (s, p) ->
    Printf.printf "%s %s\n" s (Pos.string p)
  )

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes.Method_jumps
open Typing_defs
module Cls = Decl_provider.Class

let string_filter_to_method_jump_filter = function
  | "No_filter" -> Some No_filter
  | "Class" -> Some Class
  | "Interface" -> Some Interface
  | "Trait" -> Some Trait
  | _ -> None

(* Used so the given input doesn't need the `\`. *)
let add_ns name =
  if Char.equal name.[0] '\\' then
    name
  else
    "\\" ^ name

let get_overridden_methods ctx origin_class get_or_method dest_class acc =
  match Decl_provider.get_class ctx dest_class with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    acc
  | Decl_entry.Found dest_class ->
    (* Check if each destination method exists in the origin *)
    List.fold
      (Cls.methods dest_class)
      ~init:acc
      ~f:(fun acc (m_name, de_mthd) ->
        (* Filter out inherited methods *)
        if not (String.equal de_mthd.ce_origin (Cls.name dest_class)) then
          acc
        else
          let or_mthd = get_or_method m_name in
          match or_mthd with
          | Some or_mthd when String.equal or_mthd.ce_origin origin_class ->
            let get_pos (lazy ty) =
              ty
              |> Typing_defs.get_pos
              |> Naming_provider.resolve_position ctx
              |> Pos.to_absolute
            in
            {
              orig_name = m_name;
              orig_pos = get_pos or_mthd.ce_type;
              dest_name = m_name;
              dest_pos = get_pos de_mthd.ce_type;
              orig_p_name = origin_class;
              dest_p_name = Cls.name dest_class;
            }
            :: acc
          | Some _
          | None ->
            acc)

let check_if_extends_class_and_find_methods
    ctx target_class_name get_method target_class_pos class_name acc =
  let class_ = Decl_provider.get_class ctx class_name in
  match class_ with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    acc
  | Decl_entry.Found c when Cls.has_ancestor c target_class_name ->
    let acc =
      get_overridden_methods ctx target_class_name get_method class_name acc
    in
    {
      orig_name = target_class_name;
      orig_pos =
        Pos.to_absolute
        @@ Naming_provider.resolve_position ctx
        @@ target_class_pos;
      dest_name = Cls.name c;
      dest_pos =
        Pos.to_absolute @@ Naming_provider.resolve_position ctx @@ Cls.pos c;
      orig_p_name = "";
      dest_p_name = "";
    }
    :: acc
  | _ -> acc

let filter_extended_classes
    ctx target_class_name get_method target_class_pos acc classes =
  List.fold_left classes ~init:acc ~f:(fun acc id ->
      check_if_extends_class_and_find_methods
        ctx
        target_class_name
        get_method
        target_class_pos
        id.FileInfo.name
        acc)

let find_extended_classes_in_files
    ctx target_class_name get_method target_class_pos acc classes =
  List.fold_left classes ~init:acc ~f:(fun acc classes ->
      filter_extended_classes
        ctx
        target_class_name
        get_method
        target_class_pos
        acc
        classes)

(* Might raise {!Naming_table.File_info_not_found} *)
let find_extended_classes_in_files_parallel
    ctx workers target_class_name get_method target_class_pos naming_table files
    =
  let classes =
    Relative_path.Set.fold files ~init:[] ~f:(fun fn acc ->
        let { FileInfo.ids = { FileInfo.classes; _ }; _ } =
          Naming_table.get_file_info_exn naming_table fn
        in
        classes :: acc)
  in
  if List.length classes > 10 then
    MultiWorker.call
      workers
      ~job:
        (find_extended_classes_in_files
           ctx
           target_class_name
           get_method
           target_class_pos)
      ~merge:List.rev_append
      ~neutral:[]
      ~next:(MultiWorker.next workers classes)
  else
    find_extended_classes_in_files
      ctx
      target_class_name
      get_method
      target_class_pos
      []
      classes

(* Find child classes.
   Might raise {!Naming_table.File_info_not_found} *)
let get_child_classes_and_methods ctx cls ~filter naming_table workers =
  (match filter with
  | No_filter -> ()
  | _ -> failwith "Method jump filters not implemented for finding children");
  let files = FindRefsService.get_child_classes_files ctx (Cls.name cls) in
  find_extended_classes_in_files_parallel
    ctx
    workers
    (Cls.name cls)
    (Cls.get_method cls)
    (Cls.pos cls)
    naming_table
    files

let class_passes_filter ~filter cls =
  match (filter, Cls.kind cls) with
  | (No_filter, _) -> true
  | (Class, Ast_defs.Cclass _) -> true
  | (Interface, Ast_defs.Cinterface) -> true
  | (Trait, Ast_defs.Ctrait) -> true
  | _ -> false

(* Find ancestor classes *)
let get_ancestor_classes_and_methods ctx cls ~filter acc =
  let class_ = Decl_provider.get_class ctx (Cls.name cls) in
  match class_ with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []
  | Decl_entry.Found cls ->
    List.fold (Cls.all_ancestor_names cls) ~init:acc ~f:(fun acc k ->
        let class_ = Decl_provider.get_class ctx k in
        match class_ with
        | Decl_entry.Found c when class_passes_filter ~filter c ->
          let acc =
            get_overridden_methods
              ctx
              (Cls.name cls)
              (Cls.get_method cls)
              (Cls.name c)
              acc
          in
          {
            orig_name = Utils.strip_ns (Cls.name cls);
            orig_pos =
              Cls.pos cls
              |> Naming_provider.resolve_position ctx
              |> Pos.to_absolute;
            dest_name = Utils.strip_ns (Cls.name c);
            dest_pos =
              Cls.pos c
              |> Naming_provider.resolve_position ctx
              |> Pos.to_absolute;
            orig_p_name = "";
            dest_p_name = "";
          }
          :: acc
        | _ -> acc)

(*  Returns a list of the ancestor or child
 *  classes and methods for a given class.
 *  Might raise {!Naming_table.File_info_not_found}
 *)
let get_inheritance ctx class_ ~filter ~find_children naming_table workers =
  let class_ = add_ns class_ in
  let class_ = Decl_provider.get_class ctx class_ in
  match class_ with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []
  | Decl_entry.Found c ->
    if find_children then
      get_child_classes_and_methods ctx c ~filter naming_table workers
    else
      get_ancestor_classes_and_methods ctx c ~filter []

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json
open Typing_deps

type inheritanceInfo = {
  parentName: string;
  childName: string;
}

let convert_inheritance_to_json (inheritances : inheritanceInfo list) :
    json option =
  match inheritances with
  | [] -> None
  | inheritances ->
    let facts =
      List.map
        ~f:(fun inheritance_info ->
          (* TODO: parentName/childName schema still subject to change *)
          JSON_Object
            [
              ( "key",
                JSON_Object
                  [
                    ("parentName", JSON_String inheritance_info.parentName);
                    ("childName", JSON_String inheritance_info.childName);
                  ] );
            ])
        inheritances
    in
    (* TODO: predicate name still subject to change *)
    Some
      (JSON_Array
         [
           JSON_Object
             [
               ("predicate", JSON_String "hackdependency.inheritanceInfo.1");
               ("facts", JSON_Array facts);
             ];
         ])

let convert_deps_to_json (deps : (Dep.variant * Dep.variant) HashSet.t) :
    json option =
  let inheritance_list =
    HashSet.fold
      (fun (dep_left, dep_right) acc ->
        (* TODO: deal with all types of dependencies *)
        match dep_left with
        | Dep.Extends _ ->
          let parent = Dep.extract_name dep_left in
          let child = Dep.extract_name dep_right in
          let inheritance = { parentName = parent; childName = child } in
          inheritance :: acc
        | Dep.GConst _
        | Dep.GConstName _
        | Dep.Const _
        | Dep.AllMembers _
        | Dep.Class _
        | Dep.RecordDef _
        | Dep.Fun _
        | Dep.FunName _
        | Dep.Prop _
        | Dep.SProp _
        | Dep.Method _
        | Dep.SMethod _
        | Dep.Cstr _ ->
          acc)
      deps
      []
  in
  convert_inheritance_to_json inheritance_list

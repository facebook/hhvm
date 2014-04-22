(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

let add_ns name =
  if name.[0] = '\\' then name else "\\" ^ name

let strip_ns results =
  List.map begin fun (s, p) -> ((Utils.strip_ns s), p) end results

let search class_names method_name files genv env oc =
  let files_list = SSet.fold (fun x y -> x :: y) files [] in
  (* Get all the references to the provided method name and classes in the files *)
  let res = FindRefsService.find_references genv.ServerEnv.workers class_names
      method_name files_list in
  let res = strip_ns res in
  Marshal.to_channel oc res []

let search_function function_name genv env oc =
  let function_name = add_ns function_name in
  let files = FindRefsService.get_dependent_files_function
      genv.ServerEnv.workers function_name in
  search None (Some function_name) files genv env oc

let search_method class_name method_name genv env oc =
  let class_name = add_ns class_name in
  (* Find all the classes that extend this one *)
  let all_classes = FindRefsService.get_child_classes
      genv.ServerEnv.workers env.ServerEnv.files_info class_name in
  let all_classes = SSet.add class_name all_classes in
  (* Get all the files that reference those classes *)
  let files = FindRefsService.get_dependent_files
      genv.ServerEnv.workers all_classes in
  search (Some all_classes) (Some method_name) files genv env oc

let search_class class_name genv env oc =
  let class_name = add_ns class_name in
  let files = FindRefsService.get_dependent_files
      genv.ServerEnv.workers (SSet.singleton class_name) in
  search (Some (SSet.singleton class_name)) None files genv env oc

let go action genv env oc =
  (match action with
  | ServerMsg.Method (class_name, method_name) ->
      search_method class_name method_name genv env oc
  | ServerMsg.Function function_name ->
      search_function function_name genv env oc
  | ServerMsg.Class class_name ->
      search_class class_name genv env oc);
  flush oc

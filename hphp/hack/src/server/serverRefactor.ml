(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open ServerEnv
open ServerRefactorTypes

let get_fixme_patches codes (env: env) =
  let fixmelist = Errors.get_applied_fixmes env.errorl in
  let poslist = Fixmes.get_unused_fixmes codes fixmelist env.files_info in
  List.map ~f:(fun pos -> Remove (Pos.to_absolute pos)) poslist

let go action genv env =
  let find_refs_action, new_name = match action with
    | ClassRename (old_name, new_name) ->
        FindRefsService.Class old_name, new_name
    | MethodRename (class_name, old_name, new_name) ->
        FindRefsService.Member (class_name, FindRefsService.Method old_name),
          new_name
    | FunctionRename (old_name, new_name) ->
      FindRefsService.Function old_name, new_name in
  let include_defs = true in
  let refs = ServerFindRefs.get_refs find_refs_action include_defs genv env in
  let changes = List.fold_left refs ~f:begin fun acc x ->
    let replacement = {
      pos  = Pos.to_absolute (snd x);
      text = new_name;
    } in
    let patch = Replace replacement in
    patch :: acc
  end ~init:[] in
  changes

let go_ide definitions new_name genv env =
  let open SymbolDefinition in
  match definitions with
  | (_, Some definition) :: [] -> begin
    let {full_name; kind; _} = definition in
    let pieces = Str.split (Str.regexp "::") full_name in
    match kind, pieces with
    | Function, [function_name] ->
      let command =
        ServerRefactorTypes.FunctionRename (function_name, new_name) in
      go command genv env
    | Class, [class_name] ->
      let command =
        ServerRefactorTypes.ClassRename (class_name, new_name) in
      go command genv env
    | Method, [class_name; method_name] ->
      let command =
        ServerRefactorTypes.MethodRename (class_name, method_name, new_name) in
      go command genv env
    | _, _ -> [] end
  | _ -> [] (* We have 0 or >1 definitions so correct behavior is unknown *)

(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(* TODO: Check the context to make sure we're actually completing a class
   member access.*)
(* TODO: Possibly find a way to avoid doing the "AUTO332" thing *)
let autocomplete_class_member
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t) =
  let open Ide_api_types in
  let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
  let content = File_content.edit_file_unsafe file_content edits in
  (* TODO: Call the method for getting global fns/classes separately *)
  (* TODO: Only run the parser/typechecker hooks for class members *)
  let ac_results = ServerAutoComplete.auto_complete
    ~tcopt
    ~delimit_on_namespaces:true
    content
  in
  let open Utils.With_complete_flag in
  List.map ac_results.value ~f: begin fun r ->
    let open AutocompleteService in
    r.res_name
  end

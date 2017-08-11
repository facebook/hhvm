(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)


(*****************************************************************************)
(* Code for auto-completion *)
(*****************************************************************************)

let get_results ~(tcopt:TypecheckerOptions.t) ~(delimit_on_namespaces:bool) _ (file_info:FileInfo.t)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t =
  let {
    FileInfo.n_funs = content_funs; n_classes = content_classes; _
  } = FileInfo.simplify file_info in
  AutocompleteService.get_results ~tcopt ~delimit_on_namespaces ~content_funs ~content_classes

let auto_complete ~(tcopt:TypecheckerOptions.t) ~(delimit_on_namespaces:bool) (content:string)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t =
  AutocompleteService.attach_hooks();
  let result =
    ServerIdeUtils.declare_and_check content ~f:(get_results ~tcopt ~delimit_on_namespaces) tcopt in
  AutocompleteService.detach_hooks();
  result

let auto_complete_at_position
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t=
  let open Ide_api_types in
  (* TODO: Avoid doing the "AUTO332" thing by modifying autocomplete service to accept a position *)
  let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
  let content = File_content.edit_file_unsafe file_content edits in
  auto_complete ~tcopt ~delimit_on_namespaces:true content

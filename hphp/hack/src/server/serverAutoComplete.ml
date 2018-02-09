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

let get_results
    ~(tcopt:TypecheckerOptions.t)
    ~(delimit_on_namespaces:bool)
    ~(autocomplete_context: AutocompleteTypes.legacy_autocomplete_context)
    _
    (file_info:FileInfo.t)
    _
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t =
  let {
    FileInfo.n_funs = content_funs; n_classes = content_classes; _
  } = FileInfo.simplify file_info in
  AutocompleteService.get_results
    ~tcopt ~delimit_on_namespaces ~content_funs ~content_classes ~autocomplete_context

let auto_complete
    ~(tcopt:TypecheckerOptions.t)
    ~(delimit_on_namespaces:bool)
    ~(autocomplete_context: AutocompleteTypes.legacy_autocomplete_context)
    (content:string)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t =
  AutocompleteService.attach_hooks();
  let result =
    ServerIdeUtils.declare_and_check content
      ~f:(get_results ~tcopt ~delimit_on_namespaces ~autocomplete_context) tcopt in
  AutocompleteService.detach_hooks();
  result

let context_xhp_classname_regex = Str.regexp ".*<[a-zA-Z_0-9:]*$"
let context_xhp_member_regex = Str.regexp ".*->[a-zA-Z_0-9:]*$"

let get_autocomplete_context
    (content:string)
    (pos:File_content.position)
  : AutocompleteTypes.legacy_autocomplete_context =
  (* This function retrieves the current line of text up to the position,   *)
  (* and determines whether it's something like "<nt:te" or "->:attr".      *)
  (* This is a dumb implementation. Would be better to replace it with FFP. *)
  if pos.File_content.column = 1 then { AutocompleteTypes.
    is_xhp_classname = false;
    is_instance_member = false;
  } else
  let pos_start = { pos with File_content.column = 1; } in
  let (offset_start, offset) = File_content.get_offsets content (pos_start, pos) in
  let text = String.sub content offset_start (offset - offset_start) in
  (* text is the text from the start of the line up to the caret position *)
  let is_xhp_classname = Str.string_match context_xhp_classname_regex text 0 in
  let is_instance_member = Str.string_match context_xhp_member_regex text 0
  in
  { AutocompleteTypes.is_xhp_classname; is_instance_member; }


let auto_complete_at_position
  ~(file_content:string)
  ~(pos:File_content.position)
  ~(tcopt:TypecheckerOptions.t)
  : AutocompleteTypes.complete_autocomplete_result list Utils.With_complete_flag.t=
  let open File_content in
  (* TODO: Avoid doing the "AUTO332" thing by modifying autocomplete service to accept a position *)
  let autocomplete_context = get_autocomplete_context file_content pos in
  let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
  let content = File_content.edit_file_unsafe file_content edits in
  auto_complete ~tcopt ~delimit_on_namespaces:true ~autocomplete_context content

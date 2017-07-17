(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module ContextParser = FfpAutocompleteContextParser
module Syntax = Full_fidelity_positioned_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module Lowerer = Full_fidelity_ast
open Core

(* TODO: Check the context to make sure we're actually completing a class
   member access.*)
(* TODO: Possibly find a way to avoid doing the "AUTO332" thing *)
(* TODO: Change this such that instead of running the entire autocomplete,
  we run a minimal version of the autocomplete that only gets us what we need *)
let autocomplete_class_member
  ~(file_content:string)
  ~(pos:Ide_api_types.position)
  ~(tcopt:TypecheckerOptions.t) =
  let open Ide_api_types in
  let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
  let content = File_content.edit_file_unsafe file_content edits in
  List.map (ServerAutoComplete.auto_complete tcopt content) ~f: begin fun r ->
    let open AutocompleteService in
    r.res_name
  end

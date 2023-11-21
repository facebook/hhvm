(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let param_doc_re_str = {|@param[ ]+\([^ ]+\)[ ]+\(.+\)|}

let param_doc_re = Str.regexp param_doc_re_str

type param_info = {
  param_name: string;
  param_desc: string;
}

let parse_param_docs (line : string) : param_info option =
  if Str.string_match param_doc_re line 0 then
    let param_name = Str.matched_group 1 line in
    let param_desc = Str.matched_group 2 line in
    Some { param_name; param_desc }
  else
    None

(* convert all multiline @attribute descriptions to singleline ones *)
let parse_documentation_attributes (documentation_lines : string list) :
    string list =
  let attributes_info =
    List.fold
      ~init:([], None)
      ~f:(fun (documentation_attributes, current_attribute) line ->
        let trimmed_line = Stdlib.String.trim line in
        if Str.string_match (Str.regexp "@.*") trimmed_line 0 then
          (* new attribute *)
          match current_attribute with
          | Some current_attribute ->
            (* add previous attribute to the list and start building up this one *)
            (current_attribute :: documentation_attributes, Some trimmed_line)
          | None -> (documentation_attributes, Some trimmed_line)
        else
          (* either continuing attribute or not related to attributes *)
          match current_attribute with
          | Some current_attribute ->
            let new_current_attribute =
              current_attribute ^ " " ^ trimmed_line
            in
            (documentation_attributes, Some new_current_attribute)
          | None -> (documentation_attributes, None))
      documentation_lines
  in
  match snd @@ attributes_info with
  | Some current_attribute ->
    (* add final attribute *)
    current_attribute :: (fst @@ attributes_info)
  | None -> fst @@ attributes_info

let get_param_docs ~(docblock : string) : string String.Map.t =
  let documentation_lines = Str.split (Str.regexp "\n") docblock in
  let documentation_attributes =
    parse_documentation_attributes documentation_lines
  in
  List.fold
    ~init:String.Map.empty
    ~f:(fun param_docs line ->
      let split = parse_param_docs line in
      match split with
      | Some param_info ->
        let param_name =
          if String.is_prefix param_info.param_name ~prefix:"$" then
            param_info.param_name
          else
            "$" ^ param_info.param_name
        in
        Map.set ~key:param_name ~data:param_info.param_desc param_docs
      | None -> param_docs)
    documentation_attributes

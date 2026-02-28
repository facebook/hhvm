(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type notebook_number = Notebook_number of string

let notebook_number_pat = {|N[0-9]+|}

let notebook_number_regexp = Str.regexp notebook_number_pat

let create_notebook_number (raw : string) : (notebook_number, string) result =
  if not (Str.string_match notebook_number_regexp raw 0) then
    Error
      (Printf.sprintf
         "Invalid notebook number: %s. Expected the notebook number to match regexp /^%s$/"
         raw
         notebook_number_pat)
  else
    Ok (Notebook_number raw)

let mangle (Notebook_number prefix) (hack_source_code : string) : string =
  let rename (name : string) : string =
    if String.is_prefix name ~prefix || String.is_empty name then
      name
    else
      (* safe because we checked for empty above *)
      let first_char = String.nget name 0 in
      if Char.is_uppercase first_char then
        String.uppercase prefix ^ name
      else begin
        let lowercase_prefix = String.lowercase prefix in
        match String.chop_prefix name ~prefix:"gen_" with
        | Some name -> Printf.sprintf "gen_%s_%s" lowercase_prefix name
        | None -> Printf.sprintf "%s_%s" lowercase_prefix name
      end
  in

  DeclarationsRewriter.rename_decls ~rename ~hack_source_code

let unmangle (Notebook_number prefix) (hack_source_code : string) : string =
  let uppercase_prefix = String.uppercase prefix in
  let lowercase_prefix = String.lowercase prefix in
  let rename (name : string) : string =
    List.fold
      [
        uppercase_prefix (* example: N1234MyClass to MyClass *);
        Printf.sprintf "%s_" lowercase_prefix (* example: n1234_foo to foo *);
        Printf.sprintf
          "gen_%s_"
          lowercase_prefix (* example: gen_n1234_foo to foo*);
      ]
      ~init:name
      ~f:(fun name prefix ->
        Option.value (String.chop_prefix name ~prefix) ~default:name)
  in
  DeclarationsRewriter.rename_decls ~rename ~hack_source_code

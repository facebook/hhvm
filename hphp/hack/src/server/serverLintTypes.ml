(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Lint = Lints_core

type result = Pos.absolute Lint.t list

let output_json ?(pretty = false) oc el =
  let errors_json = List.map el ~f:Lint.to_json in
  let res =
    Hh_json.JSON_Object
      [
        ("errors", Hh_json.JSON_Array errors_json);
        ("version", Hh_json.JSON_String Hh_version.version);
      ]
  in
  Out_channel.output_string oc (Hh_json.json_to_string ~pretty res);
  Out_channel.flush stderr

let output_text oc el format =
  (* Essentially the same as type error output, except that we only have one
   * message per error, and no additional 'typing reasons' *)
  (if List.is_empty el then
    Out_channel.output_string oc "No lint errors!\n"
  else
    let f =
      match format with
      | Errors.Context -> Lint.to_contextual_string
      | Errors.Raw
      | Errors.Plain ->
        Lint.to_string
      | Errors.Highlighted -> Lint.to_highlighted_string
    in
    let sl = List.map el ~f in
    List.iter sl ~f:(fun s -> Printf.fprintf oc "%s\n%!" s));
  Out_channel.flush oc

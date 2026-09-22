(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
  notebook_number: string;
  kernelspec: Yojson.Safe.t;
}

let metadata_regexp = Str.regexp {|^ *//@bento-notebook:\(.*\)$|}

let of_comment (line : string) : t option =
  let open Option.Let_syntax in
  try
    let* json_string =
      if Str.string_match metadata_regexp line 0 then
        Some (Str.matched_group 1 line)
      else
        None
    in
    let* obj =
      match Yojson.Safe.from_string json_string with
      | `Assoc obj -> Some obj
      | _ -> None
      | exception Yojson.Json_error _ -> None
    in
    let* name_json =
      List.Assoc.find obj ~equal:String.equal "notebook_number"
    in
    let* notebook_number =
      match name_json with
      | `String name -> Some name
      | _ -> None
    in
    let+ kernelspec = List.Assoc.find obj ~equal:String.equal "kernelspec" in
    { notebook_number; kernelspec }
  with
  | Yojson.Json_error _ -> None

let to_comment { notebook_number; kernelspec } =
  Printf.sprintf "//@bento-notebook:%s"
  @@ Yojson.Safe.to_string
       (`Assoc
         [
           ("notebook_number", `String notebook_number);
           ("kernelspec", kernelspec);
         ])

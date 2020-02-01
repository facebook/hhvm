(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let telemetry_to_multiline (telemetry : Telemetry.t) : string =
  telemetry
  |> Telemetry.to_string
  |> Hh_json.json_of_string
  |> Hh_json.json_to_multiline

(** e.g. inspecting "foo.bar" will return the integer foo.bar inside telemetry.
We're in a test so we will enforce that the telemetry has the right shape:
foo must be present, and "bar" if present must be null or an integer.
Telemetry often has missing fields, so we allow "bar" to be absent or
to have the value None. *)
let int_opt (telemetry : Telemetry.t) (path : string) : int option =
  let json = telemetry |> Telemetry.to_string |> Hh_json.json_of_string in
  let accessors = Str.split (Str.regexp "\\.") path in
  let rec drill (json : Hh_json.json) (accessors : string list) : int option =
    match accessors with
    | [] -> failwith "empty path provided"
    | [key] ->
      begin
        match Hh_json_helpers.Jget.val_opt (Some json) key with
        | Some (Hh_json.JSON_Number s) -> Some (int_of_string s) (* may raise *)
        | Some Hh_json.JSON_Null -> None
        | None -> None
        | _ ->
          failwith
            (Printf.sprintf
               "%s not correct: %s in %s"
               key
               path
               (telemetry_to_multiline telemetry))
      end
    | key :: rest ->
      let obj = Hh_json_helpers.Jget.obj_opt (Some json) key in
      (match obj with
      | None ->
        failwith
          (Printf.sprintf
             "%s not correct: %s in %s"
             key
             path
             (telemetry_to_multiline telemetry))
      | Some obj -> drill obj rest)
  in
  drill json accessors

(** e.g. `inspect json "foo.bar"` will return the int foo.bar, and fail if
it's not an int or if either foo/bar don't exist. *)
let int_exn (telemetry : Telemetry.t) (path : string) : int =
  match int_opt telemetry path with
  | Some i -> i
  | None ->
    failwith
      (Printf.sprintf
         "not found: %s in %s"
         path
         (telemetry_to_multiline telemetry))

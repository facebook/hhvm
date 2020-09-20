(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let telemetry_to_multiline (telemetry : Telemetry.t) : string =
  telemetry |> Telemetry.to_json |> Hh_json.json_to_multiline

(** e.g. drilling for "foo.bar" will return that field.
Raises exception if foo is absent or not an object, and if bar is absent *)
let value_exn (telemetry : Telemetry.t) (path : string) : Hh_json.json =
  let json = telemetry |> Telemetry.to_json in
  let accessors = Str.split (Str.regexp "\\.") path in
  let rec drill (json : Hh_json.json) (accessors : string list) : Hh_json.json =
    match accessors with
    | [] -> failwith "empty path provided"
    | [key] ->
      begin
        match Hh_json_helpers.Jget.val_opt (Some json) key with
        | None ->
          failwith
            (Printf.sprintf
               "%s not found: %s in %s"
               key
               path
               (telemetry_to_multiline telemetry))
        | Some v -> v
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

(** e.g. inspecting "foo.bar"` will return the int foo.bar, and fail if
it's not an int or if either foo/bar don't exist. *)
let int_exn (telemetry : Telemetry.t) (path : string) : int =
  value_exn telemetry path |> Hh_json.get_number_int_exn

(** e.g. inspecting "foo.bar"` will return the float foo.bar, and fail if
it's not a float or if either foo/bar don't exist. *)
let float_exn (telemetry : Telemetry.t) (path : string) : float =
  value_exn telemetry path |> Hh_json.get_number_exn |> float_of_string

(** e.g. to tell whether "foo.bar" exists *)
let is_absent (telemetry : Telemetry.t) (path : string) : bool =
  try
    let (_ : Hh_json.json) = value_exn telemetry path in
    false
  with _ -> true

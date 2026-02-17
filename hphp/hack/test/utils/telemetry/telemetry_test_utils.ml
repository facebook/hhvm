(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let telemetry_to_multiline (telemetry : Telemetry.t) : string =
  Telemetry.to_string ~pretty:true telemetry

(** e.g. drilling for "foo.bar" will return that field.
Raises exception if foo is absent or not an object, and if bar is absent *)
let value_exn (telemetry : Telemetry.t) (path : string) : Yojson.Safe.t =
  let json = telemetry |> Telemetry.to_yojson in
  let accessors = Str.split (Str.regexp "\\.") path in
  let rec drill (json : Yojson.Safe.t) (accessors : string list) : Yojson.Safe.t
      =
    match accessors with
    | [] -> failwith "empty path provided"
    | [key] -> begin
      match json with
      | `Assoc kvs -> begin
        match List.assoc_opt key kvs with
        | None ->
          failwith
            (Printf.sprintf
               "%s not found: %s in %s"
               key
               path
               (telemetry_to_multiline telemetry))
        | Some v -> v
      end
      | _ ->
        failwith
          (Printf.sprintf
             "%s not an object: %s in %s"
             key
             path
             (telemetry_to_multiline telemetry))
    end
    | key :: rest -> begin
      match json with
      | `Assoc kvs -> begin
        match List.assoc_opt key kvs with
        | None ->
          failwith
            (Printf.sprintf
               "%s not correct: %s in %s"
               key
               path
               (telemetry_to_multiline telemetry))
        | Some obj -> drill obj rest
      end
      | _ ->
        failwith
          (Printf.sprintf
             "%s not an object: %s in %s"
             key
             path
             (telemetry_to_multiline telemetry))
    end
  in
  drill json accessors

(** e.g. inspecting "foo.bar"` will return the int foo.bar, and fail if
it's not an int or if either foo/bar don't exist. *)
let int_exn (telemetry : Telemetry.t) (path : string) : int =
  match value_exn telemetry path with
  | `Int i -> i
  | `Intlit s -> int_of_string s
  | v ->
    failwith
      (Printf.sprintf
         "expected int at %s, got %s"
         path
         (Yojson.Safe.to_string v))

(** e.g. inspecting "foo.bar"` will return the float foo.bar, and fail if
it's not a float or if either foo/bar don't exist. *)
let float_exn (telemetry : Telemetry.t) (path : string) : float =
  match value_exn telemetry path with
  | `Float f -> f
  | `Int i -> float_of_int i
  | `Intlit s -> float_of_string s
  | v ->
    failwith
      (Printf.sprintf
         "expected float at %s, got %s"
         path
         (Yojson.Safe.to_string v))

(** e.g. to tell whether "foo.bar" exists *)
let is_absent (telemetry : Telemetry.t) (path : string) : bool =
  try
    let (_ : Yojson.Safe.t) = value_exn telemetry path in
    false
  with
  | _ -> true

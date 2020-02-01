(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type t = key_value_pair list

and key_value_pair = string * Hh_json.json

let create () : t = []

let to_string (telemetry : t) : string =
  Hh_json.json_to_string (Hh_json.JSON_Object telemetry)

let string_ (key : string) (value : string) : string * Hh_json.json =
  (key, Hh_json.JSON_String value)

let string_
    ?(truncate : int option) (telemetry : t) ~(key : string) ~(value : string) :
    t =
  let value =
    match truncate with
    | None -> value
    | Some truncate -> String_utils.truncate truncate value
  in
  string_ key value :: telemetry

let array_
    ?(truncate_elems : int option)
    ?(truncate_len : int option)
    (telemetry : t)
    ~(key : string)
    ~(value : string list) : t =
  let value =
    match truncate_elems with
    | None -> value
    | Some truncate_elems -> List.take value truncate_elems
  in
  let value =
    match truncate_len with
    | None -> value
    | Some truncate_len ->
      List.map ~f:(fun s -> String_utils.truncate truncate_len s) value
  in
  let value = List.map ~f:(fun s -> Hh_json.JSON_String s) value in
  (key, Hh_json.JSON_Array value) :: telemetry

let bool_ (key : string) (value : bool) : key_value_pair =
  (key, Hh_json.JSON_Bool value)

let bool_ (telemetry : t) ~(key : string) ~(value : bool) : t =
  bool_ key value :: telemetry

let int_opt (telemetry : t) ~(key : string) ~(value : int option) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> (key, Hh_json.int_ value) :: telemetry

let int_ (telemetry : t) ~(key : string) ~(value : int) : t =
  (key, Hh_json.int_ value) :: telemetry

let object_ (telemetry : t) ~(key : string) ~(value : t) : t =
  (key, Hh_json.JSON_Object value) :: telemetry

let duration_seconds ?(key : string = "duration") (seconds : float) :
    key_value_pair =
  let ms = int_of_float (1000.0 *. seconds) in
  (key, Hh_json.int_ ms)

let duration ~(start_time : float) : key_value_pair =
  duration_seconds (Unix.gettimeofday () -. start_time)

let duration (telemetry : t) ~(start_time : float) : t =
  duration start_time :: telemetry

let float_ (telemetry : t) ~(key : string) ~(value : float) : t =
  (key, Hh_json.float_ value) :: telemetry

let float_opt (telemetry : t) ~(key : string) ~(value : float option) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> (key, Hh_json.float_ value) :: telemetry

let error ~(stack : string option) (e : string) : key_value_pair =
  let vals = [("message", Hh_json.JSON_String e)] in
  let vals =
    match stack with
    | None -> vals
    | Some stack -> ("stack", Hh_json.JSON_String stack) :: vals
  in
  ("error", Hh_json.JSON_Object vals)

let exception_ (e : Exception.t) : key_value_pair =
  error
    ~stack:(Some (Exception.get_backtrace_string e))
    (Exception.get_ctor_string e)

let error_with_stack (telemetry : t) ~(stack : string) ~(e : string) : t =
  error ~stack:(Some stack) e :: telemetry

let error (telemetry : t) ~(e : string) : t = error ~stack:None e :: telemetry

let exception_ (telemetry : t) ~(e : Exception.t) : t =
  exception_ e :: telemetry

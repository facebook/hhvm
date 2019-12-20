(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = key_value_pair list

and key_value_pair = string * Hh_json.json

let create () : t = []

let to_string (telemetry : t) : string =
  Hh_json.json_to_string (Hh_json.JSON_Object telemetry)

let string_ (key : string) (value : string) : string * Hh_json.json =
  (key, Hh_json.JSON_String value)

let string_ (telemetry : t) ~(key : string) ~(value : string) : t =
  string_ key value :: telemetry

let bool_ (key : string) (value : bool) : key_value_pair =
  (key, Hh_json.JSON_Bool value)

let bool_ (telemetry : t) ~(key : string) ~(value : bool) : t =
  bool_ key value :: telemetry

let int_opt (key : string) (value : int option) : key_value_pair =
  match value with
  | None -> (key, Hh_json.JSON_Null)
  | Some value -> (key, Hh_json.int_ value)

let int_opt (telemetry : t) ~(key : string) ~(value : int option) : t =
  int_opt key value :: telemetry

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

let float_ (key : string) (value : float) : key_value_pair =
  (key, Hh_json.float_ value)

let float_ (telemetry : t) ~(key : string) ~(value : float) : t =
  float_ key value :: telemetry

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

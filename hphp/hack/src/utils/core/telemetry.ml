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

let compare (left : key_value_pair) (right : key_value_pair) : int =
  String.compare (fst left) (fst right)

let create () : t = []

let to_json (telemetry : t) : Hh_json.json = Hh_json.JSON_Object telemetry

let to_string (telemetry : t) : string =
  to_json telemetry |> Hh_json.json_to_string

let string_
    ?(truncate : int option) (telemetry : t) ~(key : string) ~(value : string) :
    t =
  let value =
    match truncate with
    | None -> value
    | Some truncate -> String_utils.truncate truncate value
  in
  (key, Hh_json.JSON_String value) :: telemetry

let string_opt
    ?(truncate : int option)
    (telemetry : t)
    ~(key : string)
    ~(value : string option) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> string_ ?truncate telemetry ~key ~value

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

let int_ (telemetry : t) ~(key : string) ~(value : int) : t =
  (key, Hh_json.int_ value) :: telemetry

let int_opt (telemetry : t) ~(key : string) ~(value : int option) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> int_ telemetry ~key ~value

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
  | Some value -> float_ telemetry ~key ~value

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
  let stack = Exception.clean_stack stack in
  error ~stack:(Some stack) e :: telemetry

let error (telemetry : t) ~(e : string) : t = error ~stack:None e :: telemetry

let exception_ (telemetry : t) ~(e : Exception.t) : t =
  exception_ e :: telemetry

let quick_gc_stat () : t =
  let stat = Gc.quick_stat () in
  let bytes_per_word = Sys.word_size / 8 in
  let bytes_per_wordf = bytes_per_word |> float_of_int in
  let open Gc.Stat in
  create ()
  |> float_ ~key:"minor_bytes" ~value:(stat.minor_words *. bytes_per_wordf)
  |> float_ ~key:"promoted_bytes" ~value:(stat.promoted_words *. bytes_per_wordf)
  |> float_ ~key:"major_bytes" ~value:(stat.major_words *. bytes_per_wordf)
  |> int_ ~key:"minor_collections" ~value:stat.minor_collections
  |> int_ ~key:"major_collections" ~value:stat.major_collections
  |> int_ ~key:"heap_bytes" ~value:(stat.heap_words * bytes_per_word)
  |> int_ ~key:"compactions" ~value:stat.compactions
  |> int_ ~key:"top_heap_bytes" ~value:(stat.top_heap_words * bytes_per_word)

let rec diff (telemetry : t) ~(prev : t) : t =
  let telemetry = List.sort telemetry ~compare in
  let prev = List.sort prev ~compare in
  let acc = [] in
  diff_already_sorted telemetry ~prev acc

and diff_already_sorted (current : t) ~(prev : t) (acc : t) : t =
  match (current, prev) with
  | ([], []) -> acc
  | (c :: cs, []) -> acc |> diff_no_prev c |> diff_already_sorted cs ~prev:[]
  | ([], p :: ps) -> acc |> diff_no_current p |> diff_already_sorted [] ~prev:ps
  | (c :: cs, p :: ps) when compare c p < 0 ->
    acc |> diff_no_prev c |> diff_already_sorted cs ~prev:(p :: ps)
  | (c :: cs, p :: ps) when compare c p > 0 ->
    acc |> diff_no_current p |> diff_already_sorted (c :: cs) ~prev:ps
  | (c :: cs, p :: ps) ->
    acc |> diff_both c p |> diff_already_sorted cs ~prev:ps

and diff_no_prev ((key, val_c) : key_value_pair) (acc : t) : t =
  (key, val_c) :: (key ^ ":prev", Hh_json.JSON_Null) :: acc

and diff_no_current ((key, val_p) : key_value_pair) (acc : t) : t =
  let open Hh_json in
  match val_p with
  | JSON_Object elems ->
    let elems =
      elems |> List.fold ~init:[] ~f:(fun acc e -> diff_no_current e acc)
    in
    (key, JSON_Null) :: (key ^ ":prev", JSON_Object elems) :: acc
  | _ -> (key, Hh_json.JSON_Null) :: (key ^ ":prev", val_p) :: acc

and diff_both
    ((key, val_c) : key_value_pair) ((_key, val_p) : key_value_pair) (acc : t) :
    t =
  let open Hh_json in
  match (val_c, val_p) with
  | (JSON_Object elems_c, JSON_Object elems_p) ->
    (key, JSON_Object (diff elems_c elems_p)) :: acc
  | (JSON_Object _, _)
  | (_, JSON_Object _)
  | (JSON_Array _, _)
  | (_, JSON_Array _) ->
    (key, val_c) :: acc
  | (JSON_Bool val_c, JSON_Bool val_p) when val_c = val_p ->
    (key, JSON_Bool val_c) :: acc
  | (JSON_String val_c, JSON_String val_p) when val_c = val_p ->
    (key, JSON_String val_c) :: acc
  | (JSON_Number val_c, JSON_Number val_p) when val_c = val_p ->
    (key, JSON_Number val_c) :: acc
  | (JSON_Null, JSON_Null) -> (key, JSON_Null) :: acc
  | (_, _) -> (key, val_c) :: (key ^ ":prev", val_p) :: acc

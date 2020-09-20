(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type key_value_pair = string * Hh_json.json [@@deriving show]

(** This list is in reverse order (i.e. most recent first) *)
type t = key_value_pair list [@@deriving show]

(* Ignore - we only use the generated `pp_key_value_pair` in deriving `show` for t *)
let _ = show_key_value_pair

let compare (left : key_value_pair) (right : key_value_pair) : int =
  String.compare (fst left) (fst right)

let create () : t = []

let to_json (telemetry : t) : Hh_json.json =
  Hh_json.JSON_Object (List.rev telemetry)

let to_string (telemetry : t) : string =
  to_json telemetry |> Hh_json.json_to_string

let string_
    ?(truncate : int option) ~(key : string) ~(value : string) (telemetry : t) :
    t =
  let value =
    match truncate with
    | None -> value
    | Some truncate -> String_utils.truncate truncate value
  in
  (key, Hh_json.JSON_String value) :: telemetry

let string_opt
    ?(truncate : int option)
    ~(key : string)
    ~(value : string option)
    (telemetry : t) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> string_ ?truncate telemetry ~key ~value

let string_list
    ?(truncate_elems : int option)
    ?(truncate_len : int option)
    ~(key : string)
    ~(value : string list)
    (telemetry : t) : t =
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

let object_list ~(key : string) ~(value : t list) (telemetry : t) : t =
  let value = List.map ~f:to_json value in
  (key, Hh_json.JSON_Array value) :: telemetry

let bool_ ~(key : string) ~(value : bool) (telemetry : t) : t =
  (key, Hh_json.JSON_Bool value) :: telemetry

let int_ ~(key : string) ~(value : int) (telemetry : t) : t =
  (key, Hh_json.int_ value) :: telemetry

let int_opt ~(key : string) ~(value : int option) (telemetry : t) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> int_ telemetry ~key ~value

let object_ ~(key : string) ~(value : t) (telemetry : t) : t =
  (key, Hh_json.JSON_Object (List.rev value)) :: telemetry

let object_opt ~(key : string) ~(value : t option) (telemetry : t) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> object_ ~key ~value telemetry

let duration ?(key : string = "duration") ~(start_time : float) (telemetry : t)
    : t =
  let seconds = Unix.gettimeofday () -. start_time in
  let ms = int_of_float (1000.0 *. seconds) in
  (key, Hh_json.int_ ms) :: telemetry

let float_ ~(key : string) ~(value : float) (telemetry : t) : t =
  (key, Hh_json.float_ value) :: telemetry

let float_opt ~(key : string) ~(value : float option) (telemetry : t) : t =
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

let error_with_stack ~(stack : string) ~(e : string) (telemetry : t) : t =
  let stack = Exception.clean_stack stack in
  error ~stack:(Some stack) e :: telemetry

let error ~(e : string) (telemetry : t) : t = error ~stack:None e :: telemetry

let exception_ ~(e : Exception.t) (telemetry : t) : t =
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

let rec diff ~(all : bool) (telemetry : t) ~(prev : t) : t =
  let telemetry = List.sort telemetry ~compare in
  let prev = List.sort prev ~compare in
  let acc = [] in
  diff_already_sorted telemetry ~prev ~all acc

and diff_already_sorted (current : t) ~(prev : t) ~(all : bool) (acc : t) : t =
  match (current, prev, all) with
  | ([], [], _) -> acc
  | (c :: cs, [], true) ->
    acc |> diff_no_prev c |> diff_already_sorted cs ~prev:[] ~all
  | (_, [], false) -> acc
  | ([], p :: ps, true) ->
    acc |> diff_no_current p |> diff_already_sorted [] ~prev:ps ~all
  | ([], _, false) -> acc
  | (c :: cs, p :: ps, true) when compare c p < 0 ->
    acc |> diff_no_prev c |> diff_already_sorted cs ~prev:(p :: ps) ~all
  | (c :: cs, p :: ps, false) when compare c p > 0 ->
    acc |> diff_no_current p |> diff_already_sorted (c :: cs) ~prev:ps ~all
  | (c :: cs, p :: ps, _) ->
    acc |> diff_both ~all c p |> diff_already_sorted cs ~prev:ps ~all

and diff_no_prev ((key, val_c) : key_value_pair) (acc : t) : t =
  (key, val_c) :: (key ^ "__prev", Hh_json.JSON_Null) :: acc

and diff_no_current ((key, val_p) : key_value_pair) (acc : t) : t =
  let open Hh_json in
  match val_p with
  | JSON_Object elems ->
    let elems =
      elems |> List.fold ~init:[] ~f:(fun acc e -> diff_no_current e acc)
    in
    (key, JSON_Null) :: (key ^ "__prev", JSON_Object elems) :: acc
  | _ -> (key, Hh_json.JSON_Null) :: (key ^ "__prev", val_p) :: acc

and acc_if b elem acc =
  if b then
    elem :: acc
  else
    acc

and diff_both
    ~(all : bool)
    ((key, val_c) : key_value_pair)
    ((_key, val_p) : key_value_pair)
    (acc : t) : t =
  let open Hh_json in
  match (val_c, val_p) with
  | (JSON_Object elems_c, JSON_Object elems_p) ->
    let elems = diff ~all elems_c ~prev:elems_p in
    acc_if
      (all || not (List.is_empty elems))
      (key, JSON_Object (diff ~all elems_c ~prev:elems_p))
      acc
  | (JSON_Object _, _)
  | (_, JSON_Object _)
  | (JSON_Array _, _)
  | (_, JSON_Array _) ->
    acc_if all (key, val_c) acc
  | (JSON_Bool val_c, JSON_Bool val_p) when Bool.equal val_c val_p ->
    acc_if all (key, JSON_Bool val_c) acc
  | (JSON_String val_c, JSON_String val_p) when String.equal val_c val_p ->
    acc_if all (key, JSON_String val_c) acc
  | (JSON_Number val_c, JSON_Number val_p) when String.equal val_c val_p ->
    acc_if all (key, JSON_Number val_c) acc
  | (JSON_Null, JSON_Null) -> acc_if all (key, JSON_Null) acc
  | (JSON_Number c, JSON_Number p) ->
    (* JSON_Numbers are strings - maybe ints, maybe floats, maybe we
    can't parse them or they're outside ocaml maximum range *)
    begin
      try
        let (c, p) = (int_of_string c, int_of_string p) in
        (key ^ "__diff", int_ (c - p)) :: acc_if all (key, int_ c) acc
      with _ ->
        begin
          try
            let (c, p) = (float_of_string c, float_of_string p) in
            (key ^ "__diff", float_ (c -. p)) :: acc_if all (key, float_ c) acc
          with _ ->
            (key, JSON_Number c) :: (key ^ "__prev", JSON_Number p) :: acc
        end
    end
  | (_, _) -> (key, val_c) :: (key ^ "__prev", val_p) :: acc

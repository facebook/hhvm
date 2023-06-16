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

let empty = []

let create () : t = empty

let to_json (telemetry : t) : Hh_json.json =
  Hh_json.JSON_Object (List.rev telemetry)

let to_string ?(pretty = false) (telemetry : t) : string =
  to_json telemetry |> Hh_json.json_to_string ~pretty

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
    ?(truncate_list : int option)
    ?(truncate_each_string : int option)
    ~(key : string)
    ~(value : string list)
    (telemetry : t) : t =
  let value =
    match truncate_list with
    | None -> value
    | Some truncate_list -> List.take value truncate_list
  in
  let value =
    match truncate_each_string with
    | None -> value
    | Some truncate_each_string ->
      List.map ~f:(fun s -> String_utils.truncate truncate_each_string s) value
  in
  let value = List.map ~f:(fun s -> Hh_json.JSON_String s) value in
  (key, Hh_json.JSON_Array value) :: telemetry

let string_list_opt
    ?(truncate_list : int option)
    ?(truncate_each_string : int option)
    ~(key : string)
    ~(value : string list option)
    (telemetry : t) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value ->
    string_list ?truncate_list ?truncate_each_string telemetry ~key ~value

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

let int_list
    ?(truncate_list : int option)
    ~(key : string)
    ~(value : int list)
    (telemetry : t) : t =
  let value =
    match truncate_list with
    | None -> value
    | Some truncate_list -> List.take value truncate_list
  in
  let value = List.map ~f:(fun i -> Hh_json.int_ i) value in
  (key, Hh_json.JSON_Array value) :: telemetry

let json_ ~(key : string) ~(value : Hh_json.json) (telemetry : t) : t =
  (key, value) :: telemetry

let object_ ~(key : string) ~(value : t) (telemetry : t) : t =
  (key, Hh_json.JSON_Object (List.rev value)) :: telemetry

let object_opt ~(key : string) ~(value : t option) (telemetry : t) : t =
  match value with
  | None -> (key, Hh_json.JSON_Null) :: telemetry
  | Some value -> object_ ~key ~value telemetry

let duration
    ?(key : string = "duration")
    ~(start_time : float)
    ?(end_time : float option)
    (telemetry : t) : t =
  let end_time = Option.value end_time ~default:(Unix.gettimeofday ()) in
  let seconds = end_time -. start_time in
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
  let bytes_per_word = Stdlib.Sys.word_size / 8 in
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

let diff ~(all : bool) ?(suffix_keys = true) (telemetry : t) ~(prev : t) : t =
  let (prev_suffix, diff_suffix) =
    if suffix_keys then
      ("__prev", "__diff")
    else
      ("", "")
  in
  let rec diff (telemetry : t) ~(prev : t) : t =
    let telemetry = List.sort telemetry ~compare in
    let prev = List.sort prev ~compare in
    let acc = [] in
    diff_already_sorted telemetry ~prev acc
  and diff_already_sorted (current : t) ~(prev : t) (acc : t) : t =
    match (current, prev, all) with
    | ([], [], _) -> acc
    | (c :: cs, [], true) ->
      acc |> diff_no_prev c |> diff_already_sorted cs ~prev:[]
    | (_, [], false) -> acc
    | ([], p :: ps, true) ->
      acc |> diff_no_current p |> diff_already_sorted [] ~prev:ps
    | ([], _, false) -> acc
    | (c :: cs, p :: ps, true) when compare c p < 0 ->
      acc |> diff_no_prev c |> diff_already_sorted cs ~prev:(p :: ps)
    | (c :: cs, p :: ps, false) when compare c p > 0 ->
      acc |> diff_no_current p |> diff_already_sorted (c :: cs) ~prev:ps
    | (c :: cs, p :: ps, _) ->
      acc |> diff_both c p |> diff_already_sorted cs ~prev:ps
  and diff_no_prev ((key, val_c) : key_value_pair) (acc : t) : t =
    (key, val_c) :: (key ^ "__prev", Hh_json.JSON_Null) :: acc
  and diff_no_current ((key, val_p) : key_value_pair) (acc : t) : t =
    let open Hh_json in
    match val_p with
    | JSON_Object elems ->
      let elems =
        elems |> List.fold ~init:[] ~f:(fun acc e -> diff_no_current e acc)
      in
      (key, JSON_Null) :: (key ^ prev_suffix, JSON_Object elems) :: acc
    | _ -> (key, Hh_json.JSON_Null) :: (key ^ prev_suffix, val_p) :: acc
  and acc_if b elem acc =
    if b then
      elem :: acc
    else
      acc
  and diff_both
      ((key, val_c) : key_value_pair) ((_key, val_p) : key_value_pair) (acc : t)
      : t =
    let open Hh_json in
    match (val_c, val_p) with
    | (JSON_Object elems_c, JSON_Object elems_p) ->
      let elems = diff elems_c ~prev:elems_p in
      acc_if
        (all || not (List.is_empty elems))
        (key, JSON_Object (diff elems_c ~prev:elems_p))
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
    | (JSON_Number c, JSON_Number p) -> begin
      (* JSON_Numbers are strings - maybe ints, maybe floats, maybe we
         can't parse them or they're outside ocaml maximum range *)
      try
        let (c, p) = (int_of_string c, int_of_string p) in
        (key ^ diff_suffix, int_ (c - p)) :: acc_if all (key, int_ c) acc
      with
      | _ -> begin
        try
          let (c, p) = (float_of_string c, float_of_string p) in
          (key ^ diff_suffix, float_ (c -. p)) :: acc_if all (key, float_ c) acc
        with
        | _ -> (key, JSON_Number c) :: (key ^ prev_suffix, JSON_Number p) :: acc
      end
    end
    | (_, _) -> (key, val_c) :: (key ^ prev_suffix, val_p) :: acc
  in

  diff telemetry ~prev

let merge (telemetry1 : t) (telemetry2 : t) : t = telemetry2 @ telemetry1

let rec add (telemetry1 : t) (telemetry2 : t) : t =
  let telemetry1 = List.sort telemetry1 ~compare in
  let telemetry2 = List.sort telemetry2 ~compare in
  add_already_sorted telemetry1 telemetry2 []

and add_already_sorted (telemetry1 : t) (telemetry2 : t) (acc : t) : t =
  match (telemetry1, telemetry2) with
  | ([], []) -> acc
  | (t :: telemetry, [])
  | ([], t :: telemetry) ->
    let acc = add_single t acc in
    add_already_sorted telemetry [] acc
  | (t1 :: telemetry1, t2 :: _) when compare t1 t2 < 0 ->
    let acc = add_single t1 acc in
    add_already_sorted telemetry1 telemetry2 acc
  | (t1 :: _, t2 :: telemetry2) when compare t1 t2 > 0 ->
    let acc = add_single t2 acc in
    add_already_sorted telemetry1 telemetry2 acc
  | (t1 :: telemetry1, t2 :: telemetry2) ->
    let acc = add_elems t1 t2 acc in
    add_already_sorted telemetry1 telemetry2 acc

and add_single ((key, value) : key_value_pair) (acc : t) : t =
  let open Hh_json in
  match value with
  | JSON_Number _ -> (key, value) :: acc
  | JSON_Object elems ->
    let elems = add elems [] in
    if not (List.is_empty elems) then
      (key, JSON_Object elems) :: acc
    else
      acc
  | JSON_Array _
  | JSON_Bool _
  | JSON_Null
  | JSON_String _ ->
    acc

and add_elems
    ((key, val1) : key_value_pair) ((_key, val2) : key_value_pair) (acc : t) : t
    =
  let open Hh_json in
  match (val1, val2) with
  | (JSON_Number n1, JSON_Number n2) ->
    (try
       let n1 = int_of_string n1 in
       let n2 = int_of_string n2 in
       (key, int_ (n1 + n2)) :: acc
     with
    | _ ->
      let n1 = float_of_string n1 in
      let n2 = float_of_string n2 in
      (key, float_ (n1 +. n2)) :: acc)
  | (JSON_Object elems1, JSON_Object elems2) ->
    let elems = add elems1 elems2 in
    if not @@ List.is_empty elems then
      (key, JSON_Object elems) :: acc
    else
      acc
  | ( ( JSON_Number _ | JSON_Object _ | JSON_Array _ | JSON_Bool _ | JSON_Null
      | JSON_String _ ),
      _ ) ->
    acc

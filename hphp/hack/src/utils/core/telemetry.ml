(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Make Yojson.Safe.t usable with [@@deriving show] *)
type yojson = Yojson.Safe.t

let pp_yojson fmt json = Format.pp_print_string fmt (Yojson.Safe.to_string json)

let _show_yojson json = Yojson.Safe.to_string json

type key_value_pair = string * yojson [@@deriving show]

(** This list is in reverse order (i.e. most recent first) *)
type t = key_value_pair list [@@deriving show]

let of_yojson_opt (json : Yojson.Safe.t) : t option =
  match json with
  | `Assoc kvs -> Some kvs
  | _ -> None

(* Ignore - we only use the generated `pp_key_value_pair` in deriving `show` for t *)
let _ = show_key_value_pair

let compare (left : key_value_pair) (right : key_value_pair) : int =
  String.compare (fst left) (fst right)

let empty = []

let create () : t = empty

let to_yojson (telemetry : t) : Yojson.Safe.t = `Assoc (List.rev telemetry)

let to_string ?(pretty = false) (telemetry : t) : string =
  let json = to_yojson telemetry in
  if pretty then
    Yojson.Safe.pretty_to_string json
  else
    Yojson.Safe.to_string json

let string_
    ?(truncate : int option) ~(key : string) ~(value : string) (telemetry : t) :
    t =
  let value =
    match truncate with
    | None -> value
    | Some truncate -> String_utils.truncate truncate value
  in
  (key, `String value) :: telemetry

let string_opt
    ?(truncate : int option)
    ~(key : string)
    ~(value : string option)
    (telemetry : t) : t =
  match value with
  | None -> (key, `Null) :: telemetry
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
  let value = List.map ~f:(fun s -> `String s) value in
  (key, `List value) :: telemetry

let string_list_opt
    ?(truncate_list : int option)
    ?(truncate_each_string : int option)
    ~(key : string)
    ~(value : string list option)
    (telemetry : t) : t =
  match value with
  | None -> (key, `Null) :: telemetry
  | Some value ->
    string_list ?truncate_list ?truncate_each_string telemetry ~key ~value

let object_list ~(key : string) ~(value : t list) (telemetry : t) : t =
  let value = List.map ~f:to_yojson value in
  (key, `List value) :: telemetry

let bool_ ~(key : string) ~(value : bool) (telemetry : t) : t =
  (key, `Bool value) :: telemetry

let int_ ~(key : string) ~(value : int) (telemetry : t) : t =
  (key, `Int value) :: telemetry

let int_opt ~(key : string) ~(value : int option) (telemetry : t) : t =
  match value with
  | None -> (key, `Null) :: telemetry
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
  let value = List.map ~f:(fun i -> `Int i) value in
  (key, `List value) :: telemetry

let json ~key ~(value : Yojson.Safe.t) telemetry : t = (key, value) :: telemetry

let json_opt ~key ~(value : Yojson.Safe.t option) telemetry : t =
  let value =
    match value with
    | None -> `Null
    | Some v -> v
  in
  (key, value) :: telemetry

let object_ ~(key : string) ~(value : t) (telemetry : t) : t =
  (key, `Assoc (List.rev value)) :: telemetry

let object_opt ~(key : string) ~(value : t option) (telemetry : t) : t =
  match value with
  | None -> (key, `Null) :: telemetry
  | Some value -> object_ ~key ~value telemetry

let duration
    ?(key : string = "duration")
    ~(start_time : float)
    ?(end_time : float option)
    (telemetry : t) : t =
  let end_time = Option.value end_time ~default:(Unix.gettimeofday ()) in
  let seconds = end_time -. start_time in
  let ms = int_of_float (1000.0 *. seconds) in
  (key, `Int ms) :: telemetry

let add_duration
    ?(key : string = "duration")
    ~(start_time : float)
    ?(end_time : float option)
    (telemetry : t) : t =
  let end_time = Option.value end_time ~default:(Unix.gettimeofday ()) in
  let seconds = end_time -. start_time in
  let new_duration_ms = int_of_float (1000.0 *. seconds) in

  let (previous_duration_ms, filtered_telemetry) =
    List.fold_right
      telemetry
      ~init:(0, [])
      ~f:(fun ((cur_key, cur_value) as cur_entry) (acc_duration, acc_telemtry)
         ->
        match cur_value with
        | `Int cur_value when String.equal cur_key key ->
          (cur_value + acc_duration, acc_telemtry)
        | _ -> (acc_duration, cur_entry :: acc_telemtry))
  in
  let overall_duration_ms = new_duration_ms + previous_duration_ms in

  (key, `Int overall_duration_ms) :: filtered_telemetry

let with_duration ~(description : string) (telemetry : t) (f : unit -> 'a) :
    'a * t =
  let start_time = Unix.gettimeofday () in
  let res = f () in
  let end_time = Unix.gettimeofday () in
  let seconds = end_time -. start_time in
  let ms = int_of_float (1000.0 *. seconds) in
  let key =
    let suffix = "duration_ms" in
    if String.is_empty description then
      suffix
    else
      Printf.sprintf "%s_%s" description suffix
  in
  let telemetry = (key, `Int ms) :: telemetry in
  (res, telemetry)

let float_ ~(key : string) ~(value : float) (telemetry : t) : t =
  if Float.is_nan value || Float.is_inf value then
    (key, `Null) :: telemetry
  else
    (key, `Float value) :: telemetry

let float_opt ~(key : string) ~(value : float option) (telemetry : t) : t =
  match value with
  | None -> (key, `Null) :: telemetry
  | Some value -> float_ telemetry ~key ~value

let float_list ~(key : string) ~(value : float list) (telemetry : t) : t =
  let value =
    List.map
      ~f:(fun f ->
        if Float.is_nan f || Float.is_inf f then
          `Null
        else
          `Float f)
      value
  in
  (key, `List value) :: telemetry

let error ~(stack : string option) (e : string) : key_value_pair =
  let vals = [("message", `String e)] in
  let vals =
    match stack with
    | None -> vals
    | Some stack -> ("stack", `String stack) :: vals
  in
  ("error", `Assoc vals)

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

(** Extract an int from a Yojson number *)
let yojson_to_int_opt : Yojson.Safe.t -> int option = function
  | `Int i -> Some i
  | `Intlit s -> Int.of_string_opt s
  | _ -> None

(** Extract a float from a Yojson number *)
let yojson_to_float_opt : Yojson.Safe.t -> float option = function
  | `Int i -> Some (float_of_int i)
  | `Float f -> Some f
  | `Intlit s -> Float.of_string_opt s
  | _ -> None

(** Check if two Yojson values are equal (shallow, for telemetry diff) *)
let yojson_equal (a : Yojson.Safe.t) (b : Yojson.Safe.t) : bool =
  match (a, b) with
  | (`Int a, `Int b) -> a = b
  | (`Float a, `Float b) -> Float.(a = b)
  | (`String a, `String b) -> String.equal a b
  | (`Bool a, `Bool b) -> Bool.equal a b
  | (`Null, `Null) -> true
  | _ -> false

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
    (key, val_c) :: (key ^ "__prev", `Null) :: acc
  and diff_no_current ((key, val_p) : key_value_pair) (acc : t) : t =
    match val_p with
    | `Assoc elems ->
      let elems =
        elems |> List.fold ~init:[] ~f:(fun acc e -> diff_no_current e acc)
      in
      (key, `Null) :: (key ^ prev_suffix, `Assoc elems) :: acc
    | _ -> (key, `Null) :: (key ^ prev_suffix, val_p) :: acc
  and acc_if b elem acc =
    if b then
      elem :: acc
    else
      acc
  and diff_both
      ((key, val_c) : key_value_pair) ((_key, val_p) : key_value_pair) (acc : t)
      : t =
    match (val_c, val_p) with
    | (`Assoc elems_c, `Assoc elems_p) ->
      let elems = diff elems_c ~prev:elems_p in
      acc_if
        (all || not (List.is_empty elems))
        (key, `Assoc (diff elems_c ~prev:elems_p))
        acc
    | (`Assoc _, _)
    | (_, `Assoc _)
    | (`List _, _)
    | (_, `List _) ->
      acc_if all (key, val_c) acc
    | _ when yojson_equal val_c val_p -> acc_if all (key, val_c) acc
    | _ ->
      (* Try numeric diff *)
      (match (yojson_to_int_opt val_c, yojson_to_int_opt val_p) with
      | (Some c, Some p) ->
        (key ^ diff_suffix, `Int (c - p)) :: acc_if all (key, `Int c) acc
      | _ ->
        (match (yojson_to_float_opt val_c, yojson_to_float_opt val_p) with
        | (Some c, Some p) ->
          (key ^ diff_suffix, `Float (c -. p)) :: acc_if all (key, `Float c) acc
        | _ -> (key, val_c) :: (key ^ prev_suffix, val_p) :: acc))
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
  match value with
  | `Int _
  | `Float _ ->
    (key, value) :: acc
  | `Assoc elems ->
    let elems = add elems [] in
    if not (List.is_empty elems) then
      (key, `Assoc elems) :: acc
    else
      acc
  | _ -> acc

and add_elems
    ((key, val1) : key_value_pair) ((_key, val2) : key_value_pair) (acc : t) : t
    =
  match (val1, val2) with
  | (`Int n1, `Int n2) -> (key, `Int (n1 + n2)) :: acc
  | (`Float n1, `Float n2) -> (key, `Float (n1 +. n2)) :: acc
  | (`Int n1, `Float n2) -> (key, `Float (float_of_int n1 +. n2)) :: acc
  | (`Float n1, `Int n2) -> (key, `Float (n1 +. float_of_int n2)) :: acc
  | (`Assoc elems1, `Assoc elems2) ->
    let elems = add elems1 elems2 in
    if not @@ List.is_empty elems then
      (key, `Assoc elems) :: acc
    else
      acc
  | _ -> acc

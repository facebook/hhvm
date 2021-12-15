(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Base.Option.Monad_infix

let lines path =
  try Some (Str.split (Str.regexp_string "\n") (Path.cat (Path.make path))) with
  | _ -> None

let read_status pid_str =
  let colon = Str.regexp_string ":" in
  let to_pairs line =
    match Str.split colon line with
    | [k; v] -> Some (Caml.String.trim k, Caml.String.trim v)
    | _ -> None
  in
  lines (Printf.sprintf "/proc/%s/status" pid_str)
  |> Option.map ~f:(List.filter_map ~f:to_pairs)

(*
  Convert the human-readable format of /proc/[pid]/status back to bytes
  Because the machine-readable /proc/[pid]/stat,statm files do not have the
  VmHWM we need
*)
let to_bytes_opt str =
  match Str.split (Str.regexp_string " ") str with
  | [amount; "kB"] -> Some (int_of_string amount * 1024)
  | _ -> None

let get_status_entry status ~key =
  List.Assoc.find status ~equal:String.( = ) key >>= to_bytes_opt

let get_vm_rss () = read_status "self" >>= get_status_entry ~key:"VmRSS"

let get_vm_hwm () = read_status "self" >>= get_status_entry ~key:"VmHWM"

(** Some interesting files are made of lines like "a  : b". This function
reads the file and splits it into [(a,b),...] pairs. *)
let colon_file_to_pairs ~(fn : string) : (string * string) list =
  match Sys_utils.cat_or_failed fn with
  | None -> []
  | Some text ->
    let colon = Str.regexp_string ":" in
    Str.split (Str.regexp_string "\n") text
    |> List.filter_map ~f:(fun line ->
           match Str.split colon line with
           | [k; v] -> Some (String.strip k, String.strip v)
           | _ -> None)

(** If we wish to store an arbitrary string as a json key, let's make it
friendly for json path accessors, by retaining only alphanumerics,
replacing everything else with _, and coalescing multiple _.
E.g. "foo:bar" will become "foo_bar". *)
let json_friendly_key (k : string) : string =
  k
  |> Str.global_replace (Str.regexp "[^a-zA-Z0-9]") "_"
  |> Str.global_replace (Str.regexp "_+") "_"

(** Turns an assoc-list [(k,v),...] in an SMap. If there are multiple entries for a key
and they're all the same then we'll keep it, k->v. If there are multiple entries for
a key and any of them differ then we'll disregard them all, and add a new k__count->num
for the total number of values. This is kind of arbitrary, but it's nice for coalescing
the file /proc/cpuinfo! For simplicity we assume no existing keys had the form "k_count",
an assumption which by the way is enforced by [json_friendly_key] *)
let assoc_to_dict (keyvals : (string * string) list) : string SMap.t =
  (* first, turn it into a map {k -> [v1,v2,...]} for all values associated with a key *)
  let with_value_lists =
    List.fold keyvals ~init:SMap.empty ~f:(fun acc (k, v) ->
        match SMap.find_opt k acc with
        | Some values -> SMap.add k (v :: values) acc
        | None -> SMap.add k [v] acc)
  in
  (* now coalesce multiple values *)
  SMap.fold
    (fun k values acc ->
      let sorted = List.dedup_and_sort ~compare:String.compare values in
      match sorted with
      | [] -> acc
      | [v] -> SMap.add k v acc
      | _ -> SMap.add (k ^ "__count") (List.length values |> string_of_int) acc)
    with_value_lists
    SMap.empty

let get_host_hw_telemetry () =
  (* this regexp will turn arbitrary ("key", "<number> <suffix>") into ("key__suffix", "<number>"),
     most usefully for thing like "Mem : 12345 kB" which becomes ("Mem__kB", "12345"). *)
  let re_suffix = Str.regexp "\\([0-9.]+\\) \\([a-zA-Z]+\\)" in
  let file_to_telemetry fn =
    let dict =
      colon_file_to_pairs ~fn
      |> List.map ~f:(fun (k, v) -> (json_friendly_key k, v))
      |> List.map ~f:(fun (k, v) ->
             if Str.string_match re_suffix v 0 then
               (k ^ "__" ^ Str.matched_group 2 v, Str.matched_group 1 v)
             else
               (k, v))
      |> assoc_to_dict
    in
    SMap.fold
      (fun key value acc -> Telemetry.string_ ~key ~value acc)
      dict
      (Telemetry.create ())
  in
  let sandcastle_capabilities =
    match
      Sys_utils.cat_or_failed "/data/sandcastle/run/sandcastle.capabilities"
    with
    | None -> Hh_json.JSON_Null
    | Some s ->
      (try Hh_json.json_of_string s with
      | _ -> Hh_json.string_ s)
  in
  Telemetry.create ()
  |> Telemetry.object_ ~key:"meminfo" ~value:(file_to_telemetry "/proc/meminfo")
  |> Telemetry.object_ ~key:"cpuinfo" ~value:(file_to_telemetry "/proc/cpuinfo")
  |> Telemetry.json_
       ~key:"sandcastle_capabilities"
       ~value:sandcastle_capabilities

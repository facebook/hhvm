(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Coverage_level
open Hh_json
open Utils

let result_to_json r = JAssoc (CLMap.elements r |>
    List.map (fun (k, v) -> string_of_level k, JInt v))

let rec entry_to_json = function
  | Leaf r -> JAssoc [
      "type"   , JString "file";
      "result" , result_to_json r;
    ]
  | Node (r, el) -> JAssoc [
      "type"     , JString "directory";
      "result"   , result_to_json r;
      "children" , JAssoc (SMap.elements (SMap.map entry_to_json el));
    ]

let print_json r_opt =
  let json = match r_opt with
  | Some e -> entry_to_json e
  | None -> JAssoc [ "internal_error", JBool true ]
  in print_string (json_to_string json)

(* Calculate the percentage of code we have covered as a ratio of typed
 * expressions : total expressions. partial_weight is a number between
 * 0 and 1. *)
let calc_percentage partial_weight ctr =
  let total = CLMap.fold (fun k v acc -> v + acc) ctr 0 in
  let mult = function
    | Unchecked -> 0.0
    | Partial -> partial_weight
    | Checked -> 1.0
  in
  let score = CLMap.fold
    (fun k v acc -> mult k *. float_of_int v +. acc) ctr 0.0 in
  if total = 0
  then 1.0
  else score /. float_of_int total

let print_pretty_entry = function
  | Leaf r
  | Node (r, _) ->
      CLMap.iter (fun k v ->
        Printf.printf "%s: %d\n" (String.capitalize (string_of_level k)) v) r;
      Printf.printf "Checked / Total: %f\n" (calc_percentage 0.0 r);
      Printf.printf "(Checked + Partial * 0.5) / Total: %f\n"
        (calc_percentage 0.5 r);
      flush stdout

let print_pretty = function
  | None -> Printf.fprintf stderr "Internal error\n%!"
  | Some e -> print_pretty_entry e

let go json = if json then print_json else print_pretty

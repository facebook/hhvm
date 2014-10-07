(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module CL = Coverage_level

(* Looks inside an assoc list for a key k that's mapped to value v and replaces
 * v with the value of `f v` *)
let update_assoc f k xs =
  List.map (fun (k', v) ->
    if k = k'
    then (k, f v)
    else (k', v)) xs

(* Count the number of expressions of each kind of Coverage_level. *)
let count_exprs pos_level_l =
  let pos_level_l = List.map CL.make pos_level_l in
  List.fold_left (fun c (_, lvl) -> update_assoc ((+) 1) lvl c)
                 CL.empty_counter pos_level_l

(* Calculate the percentage of code we have covered as a ratio of typed
 * expressions : total expressions. Partially-typed expressions count as half
 * a typed expression. *)
let calc_percentage ctr =
  let total = List.fold_left (fun acc x -> acc + snd x) 0 ctr in
  let mult = function
    | CL.Unchecked -> 0.0
    | CL.Partial -> 0.5
    | CL.Checked -> 1.0
  in
  let score = List.fold_left
    (fun acc x -> mult (fst x) *. float_of_int (snd x) +. acc) 0.0 ctr in
  score /. float_of_int total

let go_ fn =
  Typing_defs.accumulate_types := true;
  ServerIdeUtils.recheck [fn];
  let pos_ty_l = !(Typing_defs.type_acc) in
  Typing_defs.accumulate_types := false;
  Typing_defs.type_acc := [];
  let counter = count_exprs pos_ty_l in
  { CL.counter = counter;
    CL.percentage = calc_percentage counter;
  }

let go fn oc =
  let result =
    if !(Typing_defs.type_acc) <> []
    then None
    else Some (go_ fn)
  in
  Marshal.to_channel oc result [];
  flush oc

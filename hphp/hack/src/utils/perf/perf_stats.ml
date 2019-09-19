(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns the requested number of quantiles (i.e., the least value
 * such that index/count values are smaller).  Assumes the input list is sorted
 * unless compare argument is given to first sort the list according to it.
 *)
let quantiles ?compare xs count =
  let a = Array.of_list xs in
  begin
    match compare with
    | None -> ()
    | Some cmp -> Array.sort cmp a
  end;
  let n = Array.length a in
  let step = float_of_int (n - 1) /. float_of_int count in
  let to_idx count = int_of_float (float_of_int count *. step) in
  let rec work count acc =
    if count = 0 then
      acc
    else
      let q = a.(to_idx count) in
      (work [@tailrec]) (count - 1) (q :: acc)
  in
  work count []

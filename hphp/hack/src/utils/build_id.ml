(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

external get_build_id : unit -> string = "hh_get_build_id"
external get_build_time : unit -> string = "hh_get_build_time"

let build_id_ohai = get_build_id ()
let to_months = function
  | "Jan" -> 0
  | "Feb" -> 1
  | "Mar" -> 2
  | "Apr" -> 3
  | "May" -> 4
  | "Jun" -> 5
  | "Jul" -> 6
  | "Aug" -> 7
  | "Sep" -> 8
  | "Oct" -> 9
  | "Nov" -> 10
  | "Dec" -> 11
  | _ -> assert false  (* Should never happen *)

let build_time =
  try
    let build_time = get_build_time () in
    ignore @@ Unix.mktime (
      Scanf.sscanf build_time "%s %d %d %d:%d:%d"
        (fun month day year hour min sec ->
          Unix.({
            tm_sec = sec;
            tm_min = min;
            tm_hour = hour;
            tm_mday = day;
            tm_mon = to_months month;
            tm_year = year - 1900;
            tm_wday = -1; (* Ignore this field for now. *)
            tm_yday = -1; (* Ignore this field for now. *)
            tm_isdst = true; (* Ignore this field for now. *)
          })
        ));
    0
  with _ -> -1

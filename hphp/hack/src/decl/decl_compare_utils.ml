(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let log_class_diff (class_name : string) (diff : ClassDiff.t) : unit =
  let buf = Buffer.create 512 in
  let fmt = Format.formatter_of_buffer buf in
  Format.pp_set_margin fmt 120;
  Format.fprintf
    fmt
    "%s  @[<2>%s:@ %a@]@?"
    (String.make 35 ' ') (* indentation hack (width of log timestamp) *)
    (Utils.strip_ns class_name)
    ClassDiff.pp
    diff;
  let diffstr = Buffer.contents buf in
  (* indentation hack *)
  Hh_logger.log "  %s" (Caml.String.trim diffstr)

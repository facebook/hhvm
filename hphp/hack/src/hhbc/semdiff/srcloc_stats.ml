(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Here we record differences in the source locations *)

open Hhbc_destruct

module Hast = Hhbc_ast
module Utils = Semdiff_utils

let mismatch_loc_instrs = ref SSet.empty
let missing_loc_on_right_instrs = ref SSet.empty
let missing_loc_on_left_instrs = ref SSet.empty

let locs_mismatch loc loc' =
  loc.Hast.line_begin != loc'.Hast.line_begin
    || loc.Hast.line_end != loc'.Hast.line_end

let check_srcloc prog prog' pc pc' =
  let ((hs, ip), (hs', ip')) = (pc, pc') in
  let string_of_inst_parser = uAnyInst $> Utils.string_of_instruction_type in
  let srcloc_inst_pattern = uSrcLoc $$ string_of_inst_parser in
  let srcloc_on_both_sides_action =
    srcloc_inst_pattern $*$ srcloc_inst_pattern
    $>> (fun ((loc, s), (loc', s')) ((_, n), (_, n')) ->
      if s = s' && locs_mismatch loc loc' then
        mismatch_loc_instrs := SSet.add s !mismatch_loc_instrs;
      ((hs, n - 1), (hs', n' - 1))) in
  let srcloc_only_on_left_action =
    srcloc_inst_pattern $*$ string_of_inst_parser
    $>> (fun ((_loc, s), s') ((_, n), (_, n')) ->
      if s = s' then
        missing_loc_on_right_instrs := SSet.add s !missing_loc_on_right_instrs;
      ((hs, n - 1), (hs', n' - 1))) in
  let srcloc_only_on_right_action =
    string_of_inst_parser $*$ srcloc_inst_pattern
    $>> (fun (s, (_loc, s')) ((_, n), (_, n')) ->
      if s = s' then
        missing_loc_on_left_instrs := SSet.add s !missing_loc_on_left_instrs;
      ((hs, n - 1), (hs', n' - 1))) in
  let parse_any_action =
    parse_any
    $>> (fun _ _ -> (pc, pc')) in
  let bigmatch_action = bigmatch [
    srcloc_on_both_sides_action;
    srcloc_only_on_left_action;
    srcloc_only_on_right_action;
    parse_any_action;
  ] in
  bigmatch_action ((prog, ip),(prog', ip'))

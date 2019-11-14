(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hhas_symbol_refs

let empty_symbol_refs =
  {
    includes = IncludePathSet.empty;
    constants = SSet.empty;
    functions = SSet.empty;
    classes = SSet.empty;
  }

let symbol_refs = ref empty_symbol_refs

let get_symbol_refs () = !symbol_refs

let set_symbol_refs s = symbol_refs := s

let reset () = set_symbol_refs empty_symbol_refs

let add_include inc =
  let srs = !symbol_refs in
  set_symbol_refs { srs with includes = IncludePathSet.add inc srs.includes }

let add_constant cid =
  let s = Hhbc_id.Const.to_raw_string cid in
  if s <> "" then
    let srs = !symbol_refs in
    set_symbol_refs { srs with constants = SSet.add s srs.constants }

let add_function fid =
  let s = Hhbc_id.Function.to_raw_string fid in
  if s <> "" then
    let srs = !symbol_refs in
    set_symbol_refs { srs with functions = SSet.add s srs.functions }

let add_class cid =
  let s = Hhbc_id.Class.to_raw_string cid in
  if s <> "" then
    let srs = !symbol_refs in
    set_symbol_refs { srs with classes = SSet.add s srs.classes }

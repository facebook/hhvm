(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hhas_symbol_refs

let empty_symbol_refs =
{ includes = SSet.empty
; constants = SSet.empty
; functions = SSet.empty
; classes = SSet.empty
}

let symbol_refs = ref empty_symbol_refs

let get_symbol_refs () = !symbol_refs

let set_symbol_refs s = symbol_refs := s

let reset () = set_symbol_refs empty_symbol_refs

let add_include s =
  let srs = !symbol_refs in
  set_symbol_refs { srs with includes = SSet.add s srs.includes }

let add_constant s =
  let srs = !symbol_refs in
  set_symbol_refs { srs with constants = SSet.add s srs.constants }

let add_function s =
  let srs = !symbol_refs in
  set_symbol_refs { srs with functions = SSet.add s srs.functions }

let add_class s =
  let srs = !symbol_refs in
  set_symbol_refs { srs with classes = SSet.add s srs.classes }

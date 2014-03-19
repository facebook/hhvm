(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(**
 * Code called from parser.mly and used to keep track of what needs to be removed
 * or modified in order to get Hack code to run on non-HHVM engines.
 *)

type items =
  | IType
  | ITuple

let is_enabled = ref false

let empty_list: (Pos.t * items) list = []
let _items = ref empty_list

let add (pos: Pos.t) (item:items): unit =
  if !is_enabled then begin
    _items := (pos, item) :: !_items;
  end else
    ()

let remove (): unit =
  if !is_enabled then begin
    _items := List.tl !_items;
  end else
    ()

let to_list (): (Pos.t * items) list =
  !_items

let enable (): unit =
  is_enabled := true


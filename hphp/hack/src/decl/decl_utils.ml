(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections
open Typing_defs

module N = Nast

let unwrap_class_hint = function
  | (_, N.Happly ((pos, class_name), type_parameters)) ->
      pos, class_name, type_parameters
  | p, N.Habstr _ ->
      Errors.expected_class ~suffix:" or interface but got a generic" p;
      Pos.none, "", []
  | p, _ ->
      Errors.expected_class ~suffix:" or interface" p;
      Pos.none, "", []

let unwrap_class_type = function
  | r, Tapply (name, tparaml) -> r, name, tparaml
  | r, Tgeneric _ ->
      let p = Typing_reason.to_pos r in
      Errors.expected_class ~suffix:" or interface but got a generic" p;
      r, (p, ""), []
  | r, _ ->
      let p = Typing_reason.to_pos r in
      Errors.expected_class ~suffix:" or interface" p;
      r, (p, ""), []

(* Given sets A and B return a tuple (AnB, A\B), i.e split A into the part
 * that is common with B, and which is unique to A *)
let split_sets defs split_if_in_defs =
  SSet.partition (SSet.mem split_if_in_defs) defs

(* Map split_sets over all sets in FileInfo *)
let split_defs defs split_if_in_defs =
  let open FileInfo in
  let n_funs1, n_funs2 =
    split_sets defs.n_funs split_if_in_defs.n_funs in
  let n_classes1, n_classes2 =
    split_sets defs.n_classes split_if_in_defs.n_classes in
  let n_types1, n_types2 =
    split_sets defs.n_types split_if_in_defs.n_types in
  let n_consts1, n_consts2 =
    split_sets defs.n_consts split_if_in_defs.n_consts in

  let r1 = {
    n_funs    = n_funs1;
    n_classes = n_classes1;
    n_types   = n_types1;
    n_consts  = n_consts1;
  } in
  let r2 = {
    n_funs    = n_funs2;
    n_classes = n_classes2;
    n_types   = n_types2;
    n_consts  = n_consts2;
  } in
  r1, r2

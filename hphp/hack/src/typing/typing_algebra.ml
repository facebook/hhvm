(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module TySet = Typing_set

let factorize_common_types tyl1 tyl2 =
  (* Factorize common types, for example
    (A | B) & (A | C) = A | (B & C)
    and
    (A | B | C1 | D1) & (A | B | C2 | D2) = A | B | ((C1 | D1) & (C2 | D2))

    return the common types, followed by the unique types from each input list
  *)
  let tys1 = TySet.of_list tyl1 in
  let tys2 = TySet.of_list tyl2 in
  let common_tys = TySet.inter tys1 tys2 in
  let tys1' = TySet.diff tys1 common_tys in
  let tys2' = TySet.diff tys2 common_tys in
  let tyl1' = TySet.elements tys1' in
  let tyl2' = TySet.elements tys2' in
  let common_tyl = TySet.elements common_tys in
  (common_tyl, tyl1', tyl2')

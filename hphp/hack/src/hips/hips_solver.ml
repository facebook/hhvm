(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hips_types

let substitute
    (interconstr : 'entity inter_constraint)
    (_ : ('intra_constraint, 'entity) constraint_ list SMap.t) :
    ('intra_constraint, 'entity) constraint_ list =
  match interconstr with
  | Arg (_, _, _) -> []
  | Ret (_, _) -> []

let analyse (_ : ('intra_constraint, 'entity) constraint_ list SMap.t) :
    'intra_constraint list =
  []

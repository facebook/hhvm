(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type mode =
  | MMerge
  | MSolve
  | MExport
  | MRewrite

type result =
  | RMerge of unit
  | RSolve of unit
  | RExport of unit
  | RRewrite of ServerRefactorTypes.patch list
  | RError of string

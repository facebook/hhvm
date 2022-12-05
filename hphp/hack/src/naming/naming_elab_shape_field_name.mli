(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* We permit class constants to be used as shape field names. Here we replace
   uses of `self` with the class to which they refer or `unknown` if the shape
   is not defined within the context of a class *)
include Naming_phase_sigs.Elabidation

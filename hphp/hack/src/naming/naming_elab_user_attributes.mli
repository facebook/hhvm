(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This combined elaboration and validation pass will deduplicate user attributes
   and raise errors for each duplicate
*)
include Naming_phase_sigs.Elabidation

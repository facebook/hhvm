(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Elaborates `$$`, `$this` and `$_` to `Dollardollar`, `This` and
   `Lplaceholder` *)
include Naming_phase_sigs.Elaboration

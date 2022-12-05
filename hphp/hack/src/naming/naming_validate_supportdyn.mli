(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This validation pass will warn when use of \\HH\\supportdyn. This pass
   must be run before `everything-sdt` elaboration which will add this class.
   The pass is intended for use when the `--enable-supportdyn-hint` typechecker
   option is _not_ set
*)
val pass : (Naming_phase_env.t, Naming_phase_error.t list) Naming_phase_pass.t

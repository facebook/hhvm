(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * Takes a script and rewrites all classes with the user attribute <<__PPL>>
 *)
val rewrite_ppl_classes : Syntax.t -> Syntax.t

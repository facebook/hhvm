(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = private {
  (* path to be used in the glean index *)
  path: string;
  cst: Full_fidelity_positioned_syntax.t;
  tast: Tast.program;
  source_text: Full_fidelity_source_text.t;
}

val create :
  Provider_context.t ->
  Relative_path.t ->
  root_path:string ->
  hhi_path:string ->
  t

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = private {
  path: Relative_path.t;
  tast: Tast.program;
  source_text: Full_fidelity_source_text.t option;
}

val create : Provider_context.t -> Relative_path.t -> t

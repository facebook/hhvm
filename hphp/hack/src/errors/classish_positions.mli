(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type 'pos classish_information = {
  classish_start: 'pos;
  classish_end: 'pos;
}

val classish_information :
  Full_fidelity_positioned_syntax.t ->
  Full_fidelity_source_text.t ->
  Relative_path.t ->
  Pos.t classish_information SMap.t

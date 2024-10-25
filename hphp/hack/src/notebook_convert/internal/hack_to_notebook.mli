(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

val hack_to_notebook :
  Full_fidelity_positioned_syntax.t -> (Hh_json.json, string) result

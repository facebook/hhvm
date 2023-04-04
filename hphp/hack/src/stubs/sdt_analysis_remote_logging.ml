(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = unit

let create ~strategy:_ ~log_remotely:_ ~tag:_ = ()

let submit_patch_result _ ~patched_ids:_ ~error_count:_ ~target_kind:_ = ()

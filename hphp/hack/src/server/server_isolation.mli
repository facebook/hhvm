(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Seed files: files under the project root that no other file references.
    Returns unique paths sorted by {!Relative_path.compare}. *)
val go : ServerEnv.genv -> ServerEnv.env -> Relative_path.t list

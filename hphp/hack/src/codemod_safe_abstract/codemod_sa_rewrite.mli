(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(** Update the file on disk to patch Safe Abstract warnings *)
val rewrite : Relative_path.t -> Codemod_sa_warning.t list -> string -> string

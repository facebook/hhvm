(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val initialize_lease : unit -> unit

val process_file : Relative_path.t -> Typing_deps.Mode.t -> Errors.t

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Reference implementation using OCaml blobs to store state. Loads the
state from the given path on disk.

If the path does not exist, it is atomically created and loaded.

Not production-usable -- it is not safe for concurrent consumers. *)
val make : Path.t -> Incremental.state

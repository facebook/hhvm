(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go : MultiWorker.worker list option ->
  get_next: Relative_path.t list Bucket.next -> ParserOptions.t -> unit

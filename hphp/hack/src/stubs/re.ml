(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let initialize_lease ~acquire_new_lease:_ = failwith "not implemented"

let process_files
    (_ : ReEnv.t)
    (_ : Relative_path.t list)
    (_ : Typing_deps.Mode.t)
    (_ : string option) =
  failwith "not implemented"

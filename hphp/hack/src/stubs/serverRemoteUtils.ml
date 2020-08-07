(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let start_typing_delegate _ env = env

let should_do_remote _ _ ~file_count:_ _ = false

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type handle

let spawn ~root:_ ~ss_dir:_ ~hhdg_path:_ _ = failwith "start not implemented"

let join _ = failwith "join_handle not implemented"

let cancel _ = failwith "cancel not implemented"

let is_finished _ = failwith "is_finished not implemented"

let get_fd _ = failwith "get_fd not implemented"

let get_re_session_id _ = failwith "get_re_session_id not implemented"

let recv _ = failwith "recv not implemented"

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type status = StatusUnknown [@@deriving enum, show]

type info = {
  id: string;
  status: status;
  (* Enqueued timestamp *)
  created_t: float;
  (* Dequeued timestamp; 0 if still in queue  *)
  started_t: float;
}
[@@deriving show]

let begin_get_info () : info option Future.t = Future.of_value None

let end_get_info (future : info option Future.t option) =
  ignore future;
  None

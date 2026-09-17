(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Attach auxiliary definitions and required language features to a filled
    template. Some synchronous entrypoint programs place the definitions in a
    separate file in the same module, using the test harness's multifile format. *)
val render : definitions:string list -> string -> string

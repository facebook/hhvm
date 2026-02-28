(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* given the path and the content of a file, return the package the file belongs to *)
val get_package : Provider_context.t -> string -> string -> string * bool

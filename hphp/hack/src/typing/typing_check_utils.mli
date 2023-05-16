(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_file :
  Provider_context.t ->
  Relative_path.t ->
  FileInfo.t ->
  Tast.def list * Errors.t

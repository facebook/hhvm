(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type patches = ServerRenameTypes.patch list

val patch_location_collection_handler : Tast_visitor.handler_base

val get_patches :
  ?is_test:bool ->
  files_info:'a ->
  fold:
    ('a ->
    init:patches ->
    f:(Relative_path.t -> 'c -> patches -> patches) ->
    patches) ->
  patches

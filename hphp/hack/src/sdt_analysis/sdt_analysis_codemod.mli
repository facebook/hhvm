(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Sdt_analysis_types

val patches_of_nadable : Summary.nadable -> ServerRenameTypes.patch list

(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type saved_decls [@@deriving show]

val export_class_decls : Provider_context.t -> SSet.t -> saved_decls

val import_class_decls : saved_decls -> SSet.t

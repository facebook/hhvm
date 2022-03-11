(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external fold_classes_in_files :
  root:string ->
  Relative_path.t list ->
  Decl_defs.decl_class_type list Relative_path.Map.t
  = "fold_classes_in_files_ffi"

external show_decl_class_type : Decl_defs.decl_class_type -> string
  = "show_decl_class_type_ffi"

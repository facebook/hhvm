(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external decl_folded_class_in_file :
  Relative_path.t -> Decl_defs.decl_class_type list
  = "decl_folded_class_in_file_ffi"

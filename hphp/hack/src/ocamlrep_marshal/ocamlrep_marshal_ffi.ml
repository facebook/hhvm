(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external to_string : 'a -> Marshal.extern_flags list -> string
  = "ocamlrep_marshal_output_value_to_string"

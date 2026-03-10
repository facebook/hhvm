(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Return [true] if the position falls in generated code.
    In partially-generated files, returns false iff in a MANUAL section.
    Cheap to call for many positions in the same file.
*)
val is_generated : Pos.t -> bool

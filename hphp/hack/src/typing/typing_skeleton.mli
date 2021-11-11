(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [of_method name meth] generates source code for a method skeleton that matches [meth]. *)
val of_method : string -> Typing_defs.class_elt -> string

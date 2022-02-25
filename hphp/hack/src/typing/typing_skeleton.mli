(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [of_method name meth ~is_static] generates source code for a method skeleton
    that matches [meth], with the appropriate static-ness w.r.t. [~is_static] *)
val of_method : string -> Typing_defs.class_elt -> is_static:bool -> string

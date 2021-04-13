(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Perform a number of inheritance checks:
    - check [override] annotations
    - check multiple uses of the same trait
    - check proper use of the [extends] keyword between two classes or two interfaces
    - check if any member of the class was detected to be cyclic
      during linearization *)
val check_class : Typing_env_types.env -> Pos.t -> Decl_provider.Class.t -> unit

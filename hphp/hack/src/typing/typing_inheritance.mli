(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check class members against the full set of members it
    inherited, including those which were overridden. *)
val check_class : Typing_env_types.env -> Decl_provider.Class.t -> unit

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

(** Initialise shape analysis environment *)
val init : Tast.saved_env -> env

(** Record a shape analysis constraint *)
val add_constraint : env -> constraint_ -> env

(** Ignore all existing constraints. The intention of this is to prevent
    unnecessary duplication of constraints when multiple environments need to
    be merged. *)
val reset_constraints : env -> env

(** Find an entity that a local variable points to *)
val get_local : Local_id.t -> env -> entity

(** Set an entity to a local variable *)
val set_local : Local_id.t -> entity -> env -> env

(** The first environment is the parent environment. The others are combined.
    This is useful in branching code. *)
val union : env -> env -> env -> env

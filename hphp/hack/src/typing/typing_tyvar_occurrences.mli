(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

module Log : sig
  val as_value : t -> Typing_log_value.value
end

val init : t

val get_tyvar_occurrences : t -> Tvid.t -> Tvid.Set.t

val get_tyvars_in_tyvar : t -> Tvid.t -> Tvid.Set.t

val contains_unsolved_tyvars : t -> Tvid.t -> bool

val make_tyvars_occur_in_tyvar : t -> Tvid.Set.t -> occur_in:Tvid.t -> t

val make_tyvar_no_more_occur_in_tyvar : t -> Tvid.t -> no_more_in:Tvid.t -> t

(** Update the tyvar occurrences after unbinding a type variable. *)
val unbind_tyvar : t -> Tvid.t -> t

val occurs_in : t -> Tvid.t -> in_:Tvid.t -> bool

(** remove variable from the occurrence structure.
    Does not try and be clever, especially does not perform any
    type simplification. *)
val remove_var : t -> Tvid.t -> t

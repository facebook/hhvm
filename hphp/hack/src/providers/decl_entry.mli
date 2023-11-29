(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** When querying a decl, we have 3 possible states. *)
type 'a t =
  | DoesNotExist
      (** The decleration does not exist in the current repo state.
          This sould result in a symbol not found error. *)
  | NotYetAvailable
      (** We can't find it in the abstraction/layer that should be
          providing access to symbols in the current repo, but that
          doesn't necessarily mean it doesn't exist at all in the repo.
          The type checker should continue type checking but should avoid
          expensive behavior like backtracking as a subsequent type check
          is likely to be invoked with more informaiton available.
          Avoid expensive operations like backtracking. *)
  | Found of 'a  (** Decl entry was found. *)

val of_option_or_does_not_exist : 'a option -> 'a t

val to_option : 'a t -> 'a option

val bind : 'a t -> ('a -> 'b t) -> 'b t

val map : 'a t -> f:('a -> 'b) -> 'b t

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** We raise [Defer d.php] if we're typechecking some file a.php, and find
    we need to fetch some decl D from d.php, but the typechecking of a.php had already
    needed just too many other decls. *)
exception Defer of Relative_path.t

(** A [deferment] is a file which contains a decl that we need to fetch before
    we continue with our scheduled typechecking work. The handler of exception [Defer d.php]
    will typically call [add_deferments d.php]. *)
type deferment = Relative_path.t

type deferments_t = Relative_path.Set.t

type state = {
  enabled: bool;
  deferments: deferments_t;
  counter: int;
      (** Counter for decls needing to be computed out of the ASTs. *)
  threshold_opt: int option;
      (** If [counter] goes beyond this threshold, we raise and defer the typechecking. *)
}

let state : state ref =
  ref
    {
      enabled = true;
      deferments = Relative_path.Set.empty;
      counter = 0;
      threshold_opt = None;
    }

let reset ~(enable : bool) ~(threshold_opt : int option) : unit =
  state :=
    {
      enabled = enable;
      counter = 0;
      deferments = Relative_path.Set.empty;
      threshold_opt;
    }

(** Increment the counter of decls needing computing. *)
let increment_counter () : unit =
  if !state.enabled then state := { !state with counter = !state.counter + 1 }

(** Call [raise_if_should_defer d.php] if you're typechecking some file a.php,
    and discover that you need to fetch yet another decl "D" from file d.php.
    This will raise if the counter for computed decls is over the set up threshold. *)
let raise_if_should_defer ~(file_with_decl : Relative_path.t) : unit =
  match (!state.enabled, !state.threshold_opt) with
  | (true, Some threshold) when !state.counter >= threshold ->
    raise (Defer file_with_decl)
  | _ -> ()

(** [add_deferment d.php] is called for a file "d.php" which contains a decl
    that we need before we can proceed with our normal typechecking work. *)
let add_deferment ~(d : deferment) : unit =
  state :=
    { !state with deferments = Relative_path.Set.add !state.deferments d }

(** "deferments" are files which contain decls that we need to fetch
    before we can get on with our regular typechecking work. *)
let get_deferments () : deferment list =
  !state.deferments |> Relative_path.Set.elements

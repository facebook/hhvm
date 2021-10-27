(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** A [deferment] is a file which contains a decl that we need to fetch before
    we continue with our scheduled typechecking work. The handler of exception [Defer (d.php, "\\D")]
    will typically call [add_deferment ~d:(d.php, "\\D")]. *)
type deferment = Relative_path.t * string [@@deriving show, ord]

(** We raise [Defer] when a file requires a number of decls above the threshold *)
exception Defer

type state = {
  enabled: bool;
  counter: int;
      (** Counter for decls needing to be computed out of the ASTs. *)
  declaration_threshold_opt: int option;
      (** If [counter] goes beyond this threshold, we raise and defer the typechecking. *)
  memory_mb_threshold_opt: int option;
      (** If memory goes beyond this threshold, we raise and defer the typechecking. *)
}

let state : state ref =
  ref
    {
      enabled = false;
      counter = 0;
      declaration_threshold_opt = None;
      memory_mb_threshold_opt = None;
    }

let reset
    ~(enable : bool)
    ~(declaration_threshold_opt : int option)
    ~(memory_mb_threshold_opt : int option) : unit =
  state :=
    {
      enabled = enable;
      counter = 0;
      declaration_threshold_opt;
      memory_mb_threshold_opt;
    }

(** Increment the counter of decls needing computing. *)
let increment_counter () : unit =
  if !state.enabled then state := { !state with counter = !state.counter + 1 }

(** Call [raise_if_should_defer ()] if you're typechecking some file,
    and discover that you need to fetch yet another class.
    This will raise if the counter for computed class decls is over the set up threshold. *)
let raise_if_should_defer () : unit =
  match
    ( !state.enabled,
      !state.declaration_threshold_opt,
      !state.memory_mb_threshold_opt )
  with
  | (true, Some declaration_threshold, _)
    when !state.counter >= declaration_threshold ->
    raise Defer
  | (true, _, Some memory_mb_threshold)
    when let word_bytes = Sys.word_size / 8 in
         let megabyte = 1024 * 1024 in
         Gc.((quick_stat ()).Stat.heap_words) * word_bytes / megabyte
         >= memory_mb_threshold ->
    raise Defer
  | _ -> increment_counter ()

let with_deferred_decls
    ~enable ~declaration_threshold_opt ~memory_mb_threshold_opt f =
  reset ~enable ~declaration_threshold_opt ~memory_mb_threshold_opt;
  let cleanup () =
    reset
      ~enable:false
      ~declaration_threshold_opt:None
      ~memory_mb_threshold_opt:None
  in
  try
    let result = f () in
    cleanup ();
    Ok result
  with
  | Defer ->
    cleanup ();
    Error ()

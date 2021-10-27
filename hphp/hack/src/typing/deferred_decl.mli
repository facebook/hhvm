(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Ideally, this exception should not be part of this mli, but there is one
   place where we catch all exceptions and we need to ignore this one. TODO: clean that up somehow *)
exception Defer

(** A [deferment] is a file which contains a decl that we need to fetch before
    we continue with our scheduled typechecking work. The handler of exception [Defer (d.php, "\\D")]
    will typically call [add_deferment ~d:(d.php, "\\D")]. *)
type deferment = Relative_path.t * string [@@deriving show, ord]

(** Call [raise_if_should_defer ()] if you're typechecking some file,
    and discover that you need to fetch yet another class.
    This will raise if the counter for computed class decls is over the set up threshold. *)
val raise_if_should_defer : unit -> unit

val with_deferred_decls :
  enable:bool ->
  declaration_threshold_opt:int option ->
  memory_mb_threshold_opt:int option ->
  (unit -> 'res) ->
  ('res, unit) Result.t

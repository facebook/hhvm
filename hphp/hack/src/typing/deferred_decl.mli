(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We raise [Defer d.php] if we're typechecking some file a.php, and find
    we need to fetch some decl D from d.php, but the typechecking of a.php had already
    needed just too many other decls. *)
exception Defer of Relative_path.t

(** A [deferment] is a file which contains a decl that we need to fetch before
    we continue with our scheduled typechecking work. The handler of exception [Defer d.php]
    will typically call [add_deferments d.php]. *)
type deferment = Relative_path.t

(** Reset the internal state and the set of accumulated deferments. *)
val reset : enable:bool -> threshold_opt:int option -> unit

(** [add_deferment "d.php"] is called for a file "d.php" which contains a decl
    that we need before we can proceed with our normal typechecking work. *)
val add_deferment : d:deferment -> unit

(** "deferments" are files which contain decls that we need to fetch
    before we can get on with our regular typechecking work. *)
val get_deferments : unit -> deferment list

(** Increment the counter of decls needing computing. *)
val increment_counter : unit -> unit

(** Call [raise_if_should_defer "d.php"] if you're typechecking some file a.php,
    and discover that you need to fetch yet another decl "D" from file d.php.
    This will raise if the counter for computed decls is over the set up threshold. *)
val raise_if_should_defer : file_with_decl:deferment -> unit

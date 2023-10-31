(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_rc : Sqlite3.db -> Sqlite3.Rc.t -> unit

val column_str : Sqlite3.stmt -> int -> string

val column_blob : Sqlite3.stmt -> int -> string

val column_int64 : Sqlite3.stmt -> int -> int64

val column_str_option : Sqlite3.stmt -> int -> string option

val column_blob_option : Sqlite3.stmt -> int -> string option

val column_int64_option : Sqlite3.stmt -> int -> int64 option

module StatementCache : sig
  type t = {
    db: Sqlite3.db;
    statements: (string, Sqlite3.stmt) Base.Hashtbl.t;
  }

  val make : db:Sqlite3.db -> t

  val close : t -> unit

  val make_stmt : t -> string -> Sqlite3.stmt
end

module Data_shorthands : sig
  val opt_text : string option -> Sqlite3.Data.t

  val opt_int : int option -> Sqlite3.Data.t

  val opt_bool : bool option -> Sqlite3.Data.t

  val text : string -> Sqlite3.Data.t

  val int : int -> Sqlite3.Data.t

  val bool : bool -> Sqlite3.Data.t
end

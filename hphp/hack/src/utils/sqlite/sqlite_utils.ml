(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core

(** Check a sqlite result, and crash if it is invalid *)
let check_rc (db : Sqlite3.db) (rc : Sqlite3.Rc.t) : unit =
  match rc with
  | Sqlite3.Rc.OK
  | Sqlite3.Rc.DONE ->
    ()
  | _ ->
    failwith
      (Printf.sprintf
         "SQLite operation failed: %s (%s)"
         (Sqlite3.Rc.to_string rc)
         (Sqlite3.errmsg db))

let to_str_exn (value : Sqlite3.Data.t) : string =
  match value with
  | Sqlite3.Data.TEXT s -> s
  | _ ->
    raise
      (Invalid_argument
         (Printf.sprintf
            "Expected a string, but was %s"
            (Sqlite3.Data.to_string_debug value)))

let to_blob_exn (value : Sqlite3.Data.t) : string =
  match value with
  | Sqlite3.Data.BLOB s -> s
  | _ ->
    raise
      (Invalid_argument
         (Printf.sprintf
            "Expected a blob, but was %s"
            (Sqlite3.Data.to_string_debug value)))

(** Convert a sqlite data value to an Int64, or raise an exception *)
let to_int64_exn (value : Sqlite3.Data.t) : int64 =
  match value with
  | Sqlite3.Data.INT (i : int64) -> i
  | _ ->
    raise
      (Invalid_argument
         (Printf.sprintf
            "Expected an int, but was %s"
            (Sqlite3.Data.to_string_debug value)))

let column_str stmt idx = to_str_exn (Sqlite3.column stmt idx)

let column_blob stmt idx = to_blob_exn (Sqlite3.column stmt idx)

let column_int64 stmt idx = to_int64_exn (Sqlite3.column stmt idx)

let optional f g stmt idx =
  let data = f stmt idx in
  match data with
  | Sqlite3.Data.NULL -> None
  | _ -> Some (g data)

let column_str_option = optional Sqlite3.column to_str_exn

let column_blob_option = optional Sqlite3.column to_blob_exn

let column_int64_option = optional Sqlite3.column to_int64_exn

module StatementCache = struct
  type t = {
    db: Sqlite3.db;
    statements: (string, Sqlite3.stmt) Hashtbl.t;
  }

  let make ~db = { db; statements = Hashtbl.Poly.create () }

  (** Prepared statements must be finalized before we can close the database
  connection, or else an exception is thrown. Call this function before
  attempting `Sqlite3.close_db`. *)
  let close t =
    Hashtbl.iter t.statements ~f:(fun stmt ->
        Sqlite3.finalize stmt |> check_rc t.db);
    Hashtbl.clear t.statements

  let make_stmt t query =
    let stmt =
      Hashtbl.find_or_add t.statements query ~default:(fun () ->
          Sqlite3.prepare t.db query)
    in
    (* Clear any previous bindings for prepared statement parameters. *)
    Sqlite3.reset stmt |> check_rc t.db;
    stmt
end

module Data_shorthands = struct
  let opt_text = Sqlite3.Data.opt_text

  let opt_int = Sqlite3.Data.opt_int

  let opt_bool = Sqlite3.Data.opt_bool

  let text s = opt_text (Some s)

  let int i = opt_int (Some i)

  let bool b = opt_bool (Some b)
end

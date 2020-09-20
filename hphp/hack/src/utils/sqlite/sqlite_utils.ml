(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

(* Check a sqlite result, and crash if it is invalid *)
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

(* Gather a database and prepared statement into a tuple *)
let prepare_or_reset_statement
    (db_opt_ref : Sqlite3.db option ref)
    (stmt_ref : Sqlite3.stmt option ref)
    (sql_command_text : string) : Sqlite3.stmt =
  let db = Option.value_exn !db_opt_ref in
  let stmt =
    match !stmt_ref with
    | Some s ->
      Sqlite3.reset s |> check_rc db;
      s
    | None ->
      let s = Sqlite3.prepare db sql_command_text in
      stmt_ref := Some s;
      s
  in
  stmt

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

(* Convert a sqlite data value to an Int64, or raise an exception *)
let to_int64_exn (value : Sqlite3.Data.t) : int64 =
  match value with
  | Sqlite3.Data.INT (i : int64) -> i
  | _ ->
    raise
      (Invalid_argument
         (Printf.sprintf
            "Expected an int, but was %s"
            (Sqlite3.Data.to_string_debug value)))

(* Coerce a value to an Int64, and ignore errors *)
let to_int64 (value : Sqlite3.Data.t) : int64 =
  (try to_int64_exn value with Invalid_argument _ -> 0L)

(* Convert a sqlite data value to an ocaml int, or raise an exception *)
let to_int_exn (value : Sqlite3.Data.t) : int =
  match value with
  | Sqlite3.Data.INT i ->
    begin
      match Int64.to_int i with
      | Some num -> num
      | None ->
        raise (Invalid_argument "Attempt to coerce sqlite value to ocaml int")
    end
  | _ -> raise (Invalid_argument "Attempt to coerce sqlite value to ocaml int")

(* Convert a sqlite data value to an ocaml int, and ignore errors *)
let to_int (value : Sqlite3.Data.t) : int =
  (try to_int_exn value with Invalid_argument _ -> 0)

(* To save a bool to sqlite have to convert it to int64 *)
let bool_to_sqlite (value : bool) : Sqlite3.Data.t =
  match value with
  | true -> Sqlite3.Data.INT 1L
  | false -> Sqlite3.Data.INT 0L

(* Convert a sqlite value to a bool *)
let to_bool_exn (value : Sqlite3.Data.t) : bool =
  match value with
  | Sqlite3.Data.INT 0L -> false
  | Sqlite3.Data.INT 1L -> true
  | _ -> raise (Invalid_argument "Attempt to coerce sqlite value to ocaml bool")

(* Convert a sqlite data value to an ocaml int, and ignore errors *)
let to_bool (value : Sqlite3.Data.t) : bool =
  (try to_bool_exn value with Invalid_argument _ -> false)

let column_str stmt idx = to_str_exn (Sqlite3.column stmt idx)

let column_blob stmt idx = to_blob_exn (Sqlite3.column stmt idx)

let column_int64 stmt idx = to_int64_exn (Sqlite3.column stmt idx)

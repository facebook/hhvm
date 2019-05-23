(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel

(* Check a sqlite result, and crash if it is invalid *)
let check_rc (rc: Sqlite3.Rc.t): unit =
  if rc <> Sqlite3.Rc.OK && rc <> Sqlite3.Rc.DONE
  then failwith
    (Printf.sprintf "SQLite operation failed: %s"
      (Sqlite3.Rc.to_string rc))
;;

(* Gather a database and prepared statement into a tuple *)
let prepare_or_reset_statement
    (db: Sqlite3.db)
    (stmt_ref: Sqlite3.stmt option ref)
    (sql_command_text: string) =
  let stmt = match !stmt_ref with
    | Some s ->
      Sqlite3.reset s |> check_rc;
      s
    | None ->
      let s = Sqlite3.prepare db sql_command_text in
      stmt_ref := Some s;
      s
  in
  stmt
;;

(* Convert a sqlite data value to an Int64, or raise an exception *)
let to_int64_exn (value: Sqlite3.Data.t): int64 =
  match value with
  | Sqlite3.Data.INT (i: int64) -> i
  | _ -> raise (Invalid_argument
    "Attempt to coerce sqlite value to Int64")
;;

(* Coerce a value to an Int64, and ignore errors *)
let to_int64 (value: Sqlite3.Data.t): int64 =
  try to_int64_exn value with Invalid_argument _ -> 0L
;;

(* Convert a sqlite data value to an ocaml int, or raise an exception *)
let to_int_exn (value: Sqlite3.Data.t): int =
  match value with
  | Sqlite3.Data.INT i ->
    begin
      match Int64.to_int i with
      | Some num -> num
      | None -> raise (Invalid_argument
        "Attempt to coerce sqlite value to ocaml int")
    end
  | _ -> raise (Invalid_argument
    "Attempt to coerce sqlite value to ocaml int")
;;

(* Convert a sqlite data value to an ocaml int, and ignore errors *)
let to_int (value: Sqlite3.Data.t): int =
  try to_int_exn value with Invalid_argument _ -> 0
;;

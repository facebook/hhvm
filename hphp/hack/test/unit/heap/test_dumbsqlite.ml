let () = Printexc.record_backtrace true

module D = Dumbsqlite

let sqlerr_wrapper line {D.sqlerr} =
  Printf.eprintf "Line %d: Sqlerr (%d)\n%!" line sqlerr;
  {D.sqlerr}

let sqlerr_pair_wrapper ~to_string line ({D.sqlerr}, x) =
  Printf.eprintf "Line %d: Sqlerr (%d) Res (%s)\n%!" line sqlerr (to_string x);
  ({D.sqlerr}, x)

let main () =
  (* get temporary path and remove it *)
  let dbpath = Filename.temp_file "test_dumb_sqlite_" "db" in
  Unix.unlink dbpath;
  assert (D.caml_dumb_sqlite_open ~index:0 ~path:dbpath ~readonly:false |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_prepare
    ~index:0 ~s_index:0 ~sql:"CREATE TABLE t(x INTEGER PRIMARY KEY ASC);"
    |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_step ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 101});
  assert (D.caml_dumb_sqlite_finalize ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_close ~index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  (* check that we created a db *)
  assert (Sys.file_exists dbpath);
  (* open another db connection attempt to insert one row *)
  assert (D.caml_dumb_sqlite_open ~index:0 ~path:dbpath ~readonly:false |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_prepare
    ~index:0 ~s_index:0 ~sql:"INSERT INTO t VALUES (1);" |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_step ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 101});
  assert (D.caml_dumb_sqlite_finalize ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_close ~index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  (* read value out of db. *)
  assert (D.caml_dumb_sqlite_open ~index:0 ~path:dbpath ~readonly:false |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_prepare
    ~index:0 ~s_index:0 ~sql:"SELECT * FROM t;" |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_step ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 100});
  (* check that we get back 1 *)
  assert (
    (D.caml_dumb_sqlite_column_int64 ~s_index:0 ~column:0
    |> sqlerr_pair_wrapper __LINE__ ~to_string:Int64.to_string) =
    ({D.sqlerr = 0}, Int64.of_string "1"));
  assert (D.caml_dumb_sqlite_finalize ~s_index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  assert (D.caml_dumb_sqlite_close ~index:0 |> sqlerr_wrapper __LINE__
    = {D.sqlerr = 0});
  Unix.unlink dbpath;
  ()

let () = main ()

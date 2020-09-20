(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let foo_contents =
  {|<?hh //strict
  class Foo {
    public function foo (Bar $b) : int {
      return $b->toString();
    }
  }

  function f1(Foo $x) : void { }
  function f2(foo $x) : void { }
|}

let bar_contents =
  {|<?hh //strict
  class Bar {
    public function toString() : string {
      return "bar";
    }
  }
|}

type setup = {
  ctx: Provider_context.t;
  foo_path: Relative_path.t;
  foo_contents: string;
  bar_path: Relative_path.t;
  bar_contents: string;
  nonexistent_path: Relative_path.t;
  naming_table: Naming_table.t;
}

let fake_dir = Filename.concat Sys_utils.temp_dir_name "fake"

let in_fake_dir path = Filename.concat fake_dir path

(** This lays down some files on disk. Sets up a forward naming table.
Sets up the provider's reverse naming table. Returns an empty context, plus
information about the files on disk. The [sqlite] flag determines
whether we return a ctx where the naming table is backed by sqlite, or
all in memory. *)
let setup ~(sqlite : bool) (tcopt : GlobalOptions.t) : setup =
  (* Set up a simple fake repo *)
  Disk.mkdir_p @@ in_fake_dir "root/";
  Relative_path.set_path_prefix
    Relative_path.Root
    (Path.make @@ in_fake_dir "root/");

  (* We'll need to parse these files in order to create a naming table, which
    will be used for look up of symbols in type checking. *)
  Disk.write_file ~file:(in_fake_dir "root/Foo.php") ~contents:foo_contents;
  Disk.write_file ~file:(in_fake_dir "root/Bar.php") ~contents:bar_contents;
  let foo_path = Relative_path.from_root ~suffix:"Foo.php" in
  let bar_path = Relative_path.from_root ~suffix:"Bar.php" in
  let nonexistent_path = Relative_path.from_root ~suffix:"Nonexistent.php" in
  (* Parsing produces the file infos that the naming table module can use
    to construct the forward naming table (files-to-symbols) *)
  let popt = ParserOptions.default in
  let (file_infos, _errors, _failed_parsing) =
    Parsing_service.go
      None
      Relative_path.Set.empty
      ~get_next:(MultiWorker.next None [foo_path; bar_path])
      popt
      ~trace:true
  in
  let naming_table = Naming_table.create file_infos in
  (* Construct the reverse naming table (symbols-to-files) *)
  let fast = Naming_table.to_fast naming_table in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.get ())
  in
  Relative_path.Map.iter
    fast
    ~f:(fun (name : Relative_path.t) (info : FileInfo.names) ->
      Naming_global.ndecl_file_fast
        ctx
        name
        ~funs:info.FileInfo.n_funs
        ~classes:info.FileInfo.n_classes
        ~record_defs:info.FileInfo.n_record_defs
        ~typedefs:info.FileInfo.n_types
        ~consts:info.FileInfo.n_consts);

  let (ctx, naming_table) =
    if sqlite then (
      let db_name = Filename.temp_file "server_naming" ".sqlite" in
      let (_save_result : Naming_sqlite.save_result) =
        Naming_table.save naming_table db_name
      in
      (* Now, I want a fresh ctx with no reverse-naming entries in it,
      and I want it to be backed by a sqlite naming database. *)
      Provider_backend.set_local_memory_backend_with_defaults ();
      let sqlite_ctx =
        Provider_context.empty_for_tool
          ~popt:(Provider_context.get_popt ctx)
          ~tcopt:(Provider_context.get_tcopt ctx)
          ~backend:(Provider_backend.get ())
      in
      (sqlite_ctx, Naming_table.load_from_sqlite sqlite_ctx db_name)
    ) else
      (ctx, naming_table)
  in

  {
    ctx;
    naming_table;
    foo_path;
    foo_contents;
    bar_path;
    bar_contents;
    nonexistent_path;
  }

(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude
open Asserter

type env = {
  popt: ParserOptions.t;
  tcopt: TypecheckerOptions.t;
  local_memory: Provider_backend.local_memory;
}

let make_empty_ctx { popt; tcopt; local_memory } =
  Provider_context.empty_for_tool
    ~popt
    ~tcopt
    ~backend:(Provider_backend.Local_memory local_memory)
    ~deps_mode:(Typing_deps_mode.InMemoryMode None)

let show_env { local_memory; _ } =
  let open Provider_backend in
  let shallow =
    Shallow_decl_cache.fold
      local_memory.shallow_decl_cache
      ~init:[]
      ~f:(fun element acc ->
        match element with
        | Shallow_decl_cache.Element
            (Shallow_decl_cache_entry.Shallow_class_decl name, _) ->
          name :: acc)
  in
  let folded =
    Folded_class_cache.fold
      local_memory.folded_class_cache
      ~init:[]
      ~f:(fun element acc ->
        match element with
        | Folded_class_cache.Element
            (Folded_class_cache_entry.Folded_class_decl name, _) ->
          name :: acc)
  in
  let decl =
    Decl_cache.fold local_memory.decl_cache ~init:[] ~f:(fun element acc ->
        match element with
        | Decl_cache.Element (Decl_cache_entry.Class_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Fun_decl name, _) -> name :: acc
        | Decl_cache.Element (Decl_cache_entry.Gconst_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Module_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Typedef_decl name, _) ->
          name :: acc)
  in
  let concat names =
    names
    |> List.map ~f:Utils.strip_ns
    |> List.sort ~compare:String.compare
    |> String.concat ~sep:","
  in
  Printf.sprintf
    "[Shallow]%s [Folded]%s [Decl]%s"
    (concat shallow)
    (concat folded)
    (concat decl)

let make_entry_ctx env path contents =
  let ctx = make_empty_ctx env in
  Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents

let run_test (repo : (Relative_path.t * string) list) ~(f : env -> unit) : unit
    =
  let tcopt =
    GlobalOptions.set
      ~tco_sticky_quarantine:true
      ~tco_lsp_invalidation:true
      GlobalOptions.default
  in
  Provider_backend.set_local_memory_backend_with_defaults_for_test ();
  let local_memory =
    match Provider_backend.get () with
    | Provider_backend.Local_memory local_memory -> local_memory
    | _ -> failwith "expected local_memory"
  in
  let env = { popt = tcopt; tcopt; local_memory } in
  let ctx = make_empty_ctx env in
  Tempfile.with_real_tempdir (fun path ->
      Relative_path.set_path_prefix Relative_path.Root path;
      (* Lay down disk files and initialize the reverse naming table *)
      List.iter repo ~f:(fun (path, contents) ->
          Disk.write_file ~file:(Relative_path.to_absolute path) ~contents;
          let fileinfo =
            Direct_decl_utils.direct_decl_parse ctx path
            |> Option.value_exn
            |> Direct_decl_utils.decls_to_fileinfo path
          in
          let (_dupes : Relative_path.Set.t) =
            Naming_global.ndecl_file_and_get_conflict_files
              ctx
              path
              fileinfo.FileInfo.ids
          in
          ());
      f env)

let change_class path name =
  if not (String.is_prefix name ~prefix:"\\") then
    failwith "names should start with \\";
  FileInfo.
    {
      path;
      old_ids = Some { empty_ids with classes = [(Full Pos.none, name, None)] };
      new_ids = Some { empty_ids with classes = [(Full Pos.none, name, None)] };
      new_pfh_hash = None;
    }

let () =
  EventLogger.init_fake ();
  let repo =
    [
      ( "a.php",
        "<?hh\n function fa():void {}\n interface A {}\n function test(A $_, B $_, C $_, D $_, E $_):void {fa();fb();fc();fd();fe();}\n"
      );
      ("b.php", "<?hh\n function fb():void {}\n class B implements A {}\n");
      ("c.php", "<?hh\n function fc():void {}\n trait C {require extends B;}\n");
      ("d.php", "<?hh\n function fd():void {}\n class D extends B {use C;}\n");
      ("e.php", "<?hh\n function fe():void {}\n class E extends D {}\n");
    ]
    |> List.map ~f:(fun (suffix, contents) ->
           (Relative_path.from_root ~suffix, contents))
  in
  run_test repo ~f:(fun env ->
      (* Typecheck a.php. This will bring in lots of decls. *)
      let (path, contents) = List.hd_exn repo in
      let (ctx, entry) = make_entry_ctx env path contents in
      let entries = Relative_path.Map.singleton path entry in
      let local_memory = env.local_memory in

      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      String_asserter.assert_equals
        "[Shallow]A,B,C,D,E [Folded]A,B,C,D,E [Decl]A,B,C,D,E,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After typecheck a.php, expected these items in cache";

      (* One by one, invalidate each class and see what it removes from cache *)
      (* A *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\A"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]B,C,D,E [Folded]C [Decl]C,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After invalidate A, expected these items in cache";
      (* B *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\B"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After invalidate B, expected these items in cache";
      (* C *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\C"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,B,D,E [Folded]A,B [Decl]A,B,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After invalidate C, expected these items in cache";
      (* D *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\D"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,B,C,E [Folded]A,B,C [Decl]A,B,C,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After invalidate D, expected these items in cache";
      (* E *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\E"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,B,C,D [Folded]A,B,C,D [Decl]A,B,C,D,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After invalidate E, expected these items in cache";

      (* Get everything in cache once again *)
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in

      (* Now we're going to work on B, altering it in quarantine *)
      let (path, contents) = List.hd_exn (List.tl_exn repo) in
      let (ctx, _entry_b) = make_entry_ctx env path contents in
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A,B,C,D,E [Decl]A,B,C,D,E,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine with original B, expected these items in cache";
          let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A,B,C,D,E [Decl]A,B,C,D,E,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine typecheck a.php with original B, expected these items in cache");
      (* Change class B from "implements A" to "implements AA" *)
      let (ctx, _entry_b) =
        make_entry_ctx
          env
          path
          "<?hh\n function fb():void {}\n class B implements AA {}\n"
      in
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine with modified B, expected these items in cache";
          let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine typecheck with modified B, expected these items in cache");
      (* Save that modified "class B implements AA" and see what gets invalidated from cache *)
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\B"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After saving modified B, expected these items in cache";
      let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
      (* Restore B to its original form *)
      let (ctx, _entry_b) = make_entry_ctx env path contents in
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine with restored B, expected these items in cache";
          let _ = Tast_provider.compute_tast_unquarantined ~ctx ~entry in
          String_asserter.assert_equals
            "[Shallow]A,B,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
            (show_env env)
            "After quarantine typecheck with restored B, expected these items in cache");
      (* Save that restored "class B implements A" and see what gets invalidated from cache *)
      let _telemetry =
        Provider_utils.invalidate_upon_file_changes
          ~ctx
          ~local_memory
          ~changes:[change_class path "\\B"]
          ~entries
      in
      String_asserter.assert_equals
        "[Shallow]A,C,D,E [Folded]A [Decl]A,fa,fb,fc,fd,fe,test"
        (show_env env)
        "After saving restored B, expected these items in cache";

      ());
  ()

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

let change_class path name =
  if not (String.is_prefix name ~prefix:"\\") then
    failwith "names should start with \\";
  FileInfo.
    {
      path;
      old_ids =
        Some
          {
            empty_ids with
            classes = [FileInfo.{ pos = Full Pos.none; name; decl_hash = None }];
          };
      new_ids =
        Some
          {
            empty_ids with
            classes = [FileInfo.{ pos = Full Pos.none; name; decl_hash = None }];
          };
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
  let open Common_provider in
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

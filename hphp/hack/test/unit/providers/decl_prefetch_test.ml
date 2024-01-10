(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Asserter
open Common_provider

let clear_caches (env : Common_provider.env) : unit =
  let Provider_backend.{ decl_cache; folded_class_cache; shallow_decl_cache; _ }
      =
    env.Common_provider.local_memory
  in
  Provider_backend.Decl_cache.clear decl_cache;
  Provider_backend.Folded_class_cache.clear folded_class_cache;
  Provider_backend.Shallow_decl_cache.clear shallow_decl_cache;
  ()

let a_path = Relative_path.from_root ~suffix:"a.php"

let b_path = Relative_path.from_root ~suffix:"b.php"

let c_path = Relative_path.from_root ~suffix:"c.php"

let x_path = Relative_path.from_root ~suffix:"x.php"

let repo =
  [
    (a_path, "<?hh\n interface A {}\n");
    (b_path, "<?hh\n interface B {}\n");
    (c_path, "<?hh\n class C implements A,B {}\n");
  ]

let test_decl_provider () =
  run_test repo ~f:(fun env ->
      let ctx = make_empty_ctx env in

      (* Can we fold the decl correctly? *)
      let c = Decl_provider.get_class ctx "\\C" |> Decl_entry.to_option in
      Bool_asserter.assert_equals true (Option.is_some c) "expected to fold c";
      String_asserter.assert_equals
        "[Shallow]A,B,C [Folded]A,B,C [Decl]C"
        (show_env env)
        "After get_class C, expected these items in cache";

      clear_caches env;

      (* What happens if we try to fold but a.php file doesn't exist on disk? *)
      Disk.rm_dir_tree (Relative_path.to_absolute a_path);
      let e =
        try Ok (Decl_provider.get_class ctx "\\C" |> Decl_entry.to_option) with
        | exn -> Error (Exn.to_string exn)
      in
      begin
        match e with
        | Ok None -> failwith "Expected Decl_not_found not absence"
        | Ok (Some _) -> failwith "Expected Decl_not_found not success"
        | Error msg when String.is_substring msg ~substring:"Decl_not_found" ->
          ()
        | Error msg -> failwith ("Expected Decl_not_found not " ^ msg)
      end;

      ());
  ()

(** Runs Direct_decl_parser.Concurrent on the list of paths.
Returns parsed_file list in the same order as the paths that were given. *)
let concurrent_get_pfs ~ctx (paths : Relative_path.t list) :
    Direct_decl_parser.parsed_file list =
  let module Metadata = struct
    type t = string
  end in
  let module Concurrent = Direct_decl_parser.Concurrent (Metadata) in
  let opts =
    Provider_context.get_popt ctx |> DeclParserOptions.from_parser_options
  in
  let root = Relative_path.path_of_prefix Relative_path.Root |> Path.make in
  let handle = Concurrent.start ~opts ~root ~hhi:root ~tmp:root ~dummy:root in
  let rec loop acc earlier_opt =
    match earlier_opt with
    | None -> acc
    | Some earlier ->
      let next_earlier =
        Concurrent.enqueue_next_and_get_earlier_results handle []
      in
      loop (earlier :: acc) next_earlier
  in
  let paths =
    List.mapi paths ~f:(fun i path -> (path, Printf.sprintf "%02d" i, None))
  in
  let earlier_opt =
    Concurrent.enqueue_next_and_get_earlier_results handle paths
  in
  let acc = loop [] earlier_opt in
  acc
  |> List.sort ~compare:(fun (_, i1, _) (_, i2, _) -> String.compare i1 i2)
  |> List.map ~f:(fun (_, _, pf) -> pf)

(** Converts a list of parsed-files into a list of all top-level names declared in those files,
sorted alphabetically *)
let pfs_to_decls (pfs : Direct_decl_parser.parsed_file list) : string list =
  pfs
  |> List.map ~f:(fun pf -> pf.Direct_decl_parser.pf_decls)
  |> List.concat
  |> List.map ~f:(fun (name, _decl) -> name)
  |> List.sort ~compare:String.compare

let test_concurrent_parser () =
  run_test repo ~f:(fun env ->
      let ctx = Common_provider.make_empty_ctx env in

      (* single file that exists *)
      let pfs = concurrent_get_pfs ~ctx [a_path] in
      Int_asserter.assert_equals
        1
        (List.length pfs)
        "Expected a.php to produce one parsed_files";
      String_asserter.assert_list_equals
        ["\\A"]
        (pfs_to_decls pfs)
        "Expected a.php to produce decls A";

      (* single file that doesn't exist *)
      let pfs = concurrent_get_pfs ~ctx [x_path] in
      Int_asserter.assert_equals
        1
        (List.length pfs)
        "Expected x.php to produce one parsed_files";
      String_asserter.assert_list_equals
        []
        (pfs_to_decls pfs)
        "Expected a.php to produce no decls";

      (* two files that both exist *)
      let pfs = concurrent_get_pfs ~ctx [a_path; b_path] in
      Int_asserter.assert_equals
        2
        (List.length pfs)
        "Expected a.php, b.php to produce two parsed_files";
      String_asserter.assert_list_equals
        ["\\A"; "\\B"]
        (pfs_to_decls pfs)
        "Expected a.php, b.php to produce decls A,B";

      (* two files one of which doesn't exist *)
      let pfs = concurrent_get_pfs ~ctx [a_path; x_path] in
      Int_asserter.assert_equals
        2
        (List.length pfs)
        "Expected a.php, x.php to produce two parsed_files";
      String_asserter.assert_list_equals
        ["\\A"]
        (pfs_to_decls pfs)
        "Expected a.php, x.php to produce decls A";

      ());
  ()

let () =
  EventLogger.init_fake ();
  test_decl_provider ();
  test_concurrent_parser ();
  ()

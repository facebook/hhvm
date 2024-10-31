(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)
open Hh_prelude
open SearchUtils
open SearchTypes
open Test_harness
module Args = Test_harness_common_args
module SA = Asserter.String_asserter
module IA = Asserter.Int_asserter

let test_string : string = "example_id"

let assert_glean_autocomplete_results
    ~(query_text : string)
    ~(kind : FileInfo.si_kind option)
    ~(expected : int)
    ~(sienv_ref : si_env ref) : unit =
  let context =
    match kind with
    | Some FileInfo.SI_Interface
    | Some FileInfo.SI_Enum ->
      Actype
      (* the `Acid` context rules out interfaces+enums, so we pick one that allows them *)
    | _ -> Acid
  in
  let max_results = 10 in
  (* Search for the symbol using Glean *)
  let (results, _is_complete) =
    SymbolIndex.find_matching_symbols
      ~sienv_ref
      ~query_text
      ~max_results
      ~context
      ~kind_filter:kind
  in
  IA.assert_equals
    expected
    (List.length results)
    (Printf.sprintf "Should be %d result(s) for [%s]" expected query_text);
  let (custom_search_results, _is_complete) =
    CustomSearchService.search_symbols
      ~sienv_ref
      ~query_text
      ~max_results
      ~context
      ~kind_filter:kind
  in
  IA.assert_equals
    expected
    (List.length custom_search_results)
    (Printf.sprintf
       "Should be %d custom result(s) for [%s]"
       expected
       query_text);
  let print_fullname =
    match custom_search_results with
    | [] -> Printf.sprintf "The list is empty."
    | first_item :: _ -> Printf.sprintf "%s" first_item.si_fullname
  in

  SA.assert_equals
    print_fullname
    "bk:ig:ndx:bullet-cell-w-learn-more"
    (Printf.sprintf
       "%s is incorrect! Was expecting the si_fullname to be bk:ig:ndx:bullet-cell-w-learn-more"
       print_fullname)

let run_index_builder (harness : Test_harness.t) : si_env =
  let hhi_folder = Hhi.get_hhi_root () in
  Relative_path.set_path_prefix Relative_path.Root harness.repo_dir;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");
  Relative_path.set_path_prefix Relative_path.Hhi hhi_folder;
  let repo_path = Path.to_string harness.repo_dir in
  let (hhconfig, _, _) =
    ServerConfig.load
      ~silent:true
      ~from:""
      ~ai_options:None
      ~cli_config_overrides:[]
  in
  let popt = ServerConfig.parser_options hhconfig in
  let tcopt = ServerConfig.typechecker_options hhconfig in
  let ctx =
    Provider_context.empty_for_test
      ~popt
      ~tcopt
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 SharedMem.default_config
  in

  (* Scan the repo folder *)
  let sienv =
    SymbolIndex.initialize
      ~gleanopt:GleanOptions.default
      ~namespace_map:[]
      ~provider_name:"CustomIndex"
      ~quiet:true
  in
  let paths_with_addenda =
    Find.find
      ~file_only:true
      ~filter:FindUtils.file_filter
      [Path.make repo_path]
    |> List.filter_map ~f:(fun path ->
           let path = Relative_path.create_detect_prefix path in
           let decls = Direct_decl_utils.direct_decl_parse ctx path in
           match decls with
           | None -> None
           | Some decls ->
             let addenda = Direct_decl_parser.decls_to_addenda decls in
             Some (path, addenda, SearchUtils.TypeChecker))
  in
  let sienv = SymbolIndexCore.update_from_addenda ~sienv ~paths_with_addenda in

  sienv

(* Test the glean autocomplete query to correctly prefix xhp components *)
let test_builder_names (harness : Test_harness.t) : bool =
  let sienv = run_index_builder harness in
  let sienv_ref = ref sienv in
  assert_glean_autocomplete_results
    ~query_text:":bk:ig:ndx:bullet-cell-w-learn"
    ~kind:(Some FileInfo.SI_Class)
    ~expected:1
    ~sienv_ref;

  (* All good *)
  true

(* Main test suite *)
let tests args =
  let harness_config =
    {
      Test_harness.hh_server = args.Args.hh_server;
      hh_client = args.Args.hh_client;
      template_repo = args.Args.template_repo;
    }
  in
  [
    ( "test_builder_names",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_builder_names );
  ]

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  EventLogger.init_fake ();
  Unit_test.run_all (tests args)

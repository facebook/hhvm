open Core_kernel
open IndexBuilderTypes
open SearchUtils
open Test_harness
module Args = Test_harness_common_args
module SA = Asserter.String_asserter
module IA = Asserter.Int_asserter

let rec assert_docblock_markdown
    (expected : DocblockService.result) (actual : DocblockService.result) : unit
    =
  DocblockService.(
    match (expected, actual) with
    | ([], []) -> ()
    | ([], _) -> failwith "Expected end of list"
    | (_, []) -> failwith "Expected markdown item"
    | (exp_hd :: exp_list, act_hd :: act_list) ->
      let () =
        match (exp_hd, act_hd) with
        | (Markdown exp_txt, Markdown act_txt) ->
          SA.assert_equals exp_txt act_txt "Markdown text should match"
        | (XhpSnippet exp_txt, XhpSnippet act_txt) ->
          SA.assert_equals exp_txt act_txt "XhpSnippet should match"
        | (HackSnippet exp_txt, HackSnippet act_txt) ->
          SA.assert_equals exp_txt act_txt "HackSnippet should match"
        | _ -> failwith "Type of item does not match"
      in
      assert_docblock_markdown exp_list act_list)

let assert_ns_matches (expected_ns : string) (actual : SearchUtils.si_results) :
    unit =
  let found =
    List.fold actual ~init:false ~f:(fun acc item ->
        item.si_name = expected_ns || acc)
  in
  ( if not found then
    let results_str =
      List.fold actual ~init:"" ~f:(fun acc item -> acc ^ ";" ^ item.si_name)
    in
    let msg =
      Printf.sprintf "Did not find [%s] in [%s]" expected_ns results_str
    in
    IA.assert_equals 0 1 msg );
  ()

let assert_autocomplete
    ~(query_text : string) ~(kind : si_kind) ~(expected : int) ~(sienv : si_env)
    : unit =
  (* Search for the symbol *)
  let results =
    SymbolIndex.find_matching_symbols
      ~sienv
      ~query_text
      ~max_results:100
      ~kind_filter:(Some kind)
      ~context:None
  in
  (* Verify correct number of results *)
  IA.assert_equals
    expected
    (List.length results)
    (Printf.sprintf "Should be %d result(s) for [%s]" expected query_text);
  if expected > 0 then (
    let r = List.hd_exn results in
    (* Assert string match *)
    SA.assert_equals
      query_text
      r.si_name
      (Printf.sprintf "Should be able to find autocomplete for '%s'" query_text);

    (* Assert kind match *)
    IA.assert_equals
      (kind_to_int kind)
      (kind_to_int r.si_kind)
      (Printf.sprintf "Mismatch in kind for '%s'" query_text)
  )

let run_index_builder (harness : Test_harness.t) : si_env =
  let hhi_folder = Hhi.get_hhi_root () in
  Relative_path.set_path_prefix Relative_path.Root harness.repo_dir;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");
  Relative_path.set_path_prefix Relative_path.Hhi hhi_folder;
  let repo_path = Path.to_string harness.repo_dir in
  (* Set up initial variables *)
  let fn = Filename.temp_file "autocomplete." ".db" in
  let file_opt = Some fn in
  let ctxt =
    {
      repo_folder = repo_path;
      sqlite_filename = file_opt;
      text_filename = None;
      json_filename = None;
      json_repo_name = None;
      json_chunk_size = 0;
      custom_service = None;
      custom_repo_name = None;
      set_paths_for_worker = false;
      hhi_root_folder = Some hhi_folder;
      silent = true;
    }
  in
  (* Scan the repo folder and produce answers in sqlite *)
  IndexBuilder.go ctxt None;
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt:GleanOptions.default
      ~namespace_map:[]
      ~provider_name:"SqliteIndex"
      ~quiet:true
      ~ignore_hh_version:false
      ~savedstate_file_opt:file_opt
      ~workers:None
  in
  Hh_logger.log "Built Sqlite database [%s]" fn;
  sienv

let test_sqlite_plus_local (harness : Test_harness.t) : bool =
  let sienv = run_index_builder harness in

  (* Find one of each major type *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~sienv;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:1 ~sienv;
  assert_autocomplete
    ~query_text:"some_long_function_name"
    ~kind:SI_Function
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"ClassToBeIdentified"
    ~kind:SI_Class
    ~expected:1
    ~sienv;
  Hh_logger.log "First pass complete";

  (* Now, let's remove a few files and try again - assertions should change *)
  let bar1path = Relative_path.from_root ~suffix:"/bar_1.php" in
  let foo3path = Relative_path.from_root ~suffix:"/foo_3.php" in
  let s = Relative_path.Set.empty in
  let s = Relative_path.Set.add s bar1path in
  let s = Relative_path.Set.add s foo3path in
  let sienv = SymbolIndex.remove_files ~sienv ~paths:s in
  Hh_logger.log "Removed files";

  (* Two of these have been removed! *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~sienv;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:0 ~sienv;
  assert_autocomplete
    ~query_text:"some_long_function_name"
    ~kind:SI_Function
    ~expected:0
    ~sienv;
  assert_autocomplete
    ~query_text:"ClassToBeIdentified"
    ~kind:SI_Class
    ~expected:1
    ~sienv;
  Hh_logger.log "Second pass complete";

  (* Add the files back! *)
  let nobigtrait_id =
    (FileInfo.File (FileInfo.Class, bar1path), "\\NoBigTrait")
  in
  let some_long_function_name_id =
    (FileInfo.File (FileInfo.Class, foo3path), "\\some_long_function_name")
  in
  let bar1fileinfo =
    {
      FileInfo.hash = None;
      file_mode = None;
      funs = [];
      classes = [nobigtrait_id];
      record_defs = [];
      typedefs = [];
      consts = [];
      comments = Some [];
    }
  in
  let foo3fileinfo =
    {
      FileInfo.hash = None;
      file_mode = None;
      funs = [some_long_function_name_id];
      classes = [];
      record_defs = [];
      typedefs = [];
      consts = [];
      comments = Some [];
    }
  in
  let changelist =
    [
      (bar1path, Full bar1fileinfo, TypeChecker);
      (foo3path, Full foo3fileinfo, TypeChecker);
    ]
  in
  let init_id = Random_id.short_string () in
  let env = ServerEnvBuild.make_env ~init_id ServerConfig.default_config in
  let ctx = Provider_utils.ctx_from_server_env env in
  let sienv = SymbolIndex.update_files ~ctx ~sienv ~paths:changelist in
  let n = LocalSearchService.count_local_fileinfos sienv in
  Hh_logger.log "Added back; local search service now contains %d files" n;

  (* Find one of each major type *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~sienv;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:1 ~sienv;
  assert_autocomplete
    ~query_text:"some_long_function_name"
    ~kind:SI_Function
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"ClassToBeIdentified"
    ~kind:SI_Class
    ~expected:1
    ~sienv;
  Hh_logger.log "Third pass complete";

  (* If we got here, all is well *)
  true

(* Test the ability of the index builder to capture a variety of
 * names and kinds correctly *)
let test_builder_names (harness : Test_harness.t) : bool =
  let sienv = run_index_builder harness in

  (* Assert that we can capture all kinds of symbols *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~sienv;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:1 ~sienv;
  assert_autocomplete
    ~query_text:"some_long_function_name"
    ~kind:SI_Function
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"ClassToBeIdentified"
    ~kind:SI_Class
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"CONST_SOME_COOL_VALUE"
    ~kind:SI_GlobalConstant
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"IMyFooInterface"
    ~kind:SI_Interface
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"SomeTypeAlias"
    ~kind:SI_Typedef
    ~expected:1
    ~sienv;
  assert_autocomplete
    ~query_text:"FbidMapField"
    ~kind:SI_Enum
    ~expected:1
    ~sienv;

  (* XHP is considered a class at the moment - this may change.
   * Note that XHP classes are saved WITH a leading colon.
   * Storing them without the leading colon results in incorrect
   * text for XHP class autocomplete. *)
  assert_autocomplete
    ~query_text:":xhp:helloworld"
    ~kind:SI_Class
    ~expected:1
    ~sienv;

  (* All good *)
  true

(* Ensure that the namespace map handles common use cases *)
let test_namespace_map (harness : Test_harness.t) : bool =
  NamespaceSearchService.(
    let _ = harness in
    let sienv = SearchUtils.quiet_si_env in
    (* Register a namespace and fetch it back exactly *)
    register_namespace ~sienv ~namespace:"HH\\Lib\\Str\\fb";
    let ns = find_exact_match ~sienv ~namespace:"HH" in
    SA.assert_equals "\\HH" ns.nss_full_namespace "Basic match";
    let ns = find_exact_match ~sienv ~namespace:"HH\\Lib" in
    SA.assert_equals "\\HH\\Lib" ns.nss_full_namespace "Basic match";
    let ns = find_exact_match ~sienv ~namespace:"HH\\Lib\\Str" in
    SA.assert_equals "\\HH\\Lib\\Str" ns.nss_full_namespace "Basic match";
    let ns = find_exact_match ~sienv ~namespace:"HH\\Lib\\Str\\fb" in
    SA.assert_equals "\\HH\\Lib\\Str\\fb" ns.nss_full_namespace "Basic match";

    (* Fetch back case insensitive *)
    let ns = find_exact_match ~sienv ~namespace:"hh" in
    SA.assert_equals "\\HH" ns.nss_full_namespace "Case insensitive";
    let ns = find_exact_match ~sienv ~namespace:"hh\\lib" in
    SA.assert_equals "\\HH\\Lib" ns.nss_full_namespace "Case insensitive";
    let ns = find_exact_match ~sienv ~namespace:"hh\\lib\\str" in
    SA.assert_equals "\\HH\\Lib\\Str" ns.nss_full_namespace "Case insensitive";
    let ns = find_exact_match ~sienv ~namespace:"hh\\lib\\str\\FB" in
    SA.assert_equals
      "\\HH\\Lib\\Str\\fb"
      ns.nss_full_namespace
      "Case insensitive";

    (* Register an alias and verify that it works as expected *)
    register_alias ~sienv ~alias:"Str" ~target:"HH\\Lib\\Str";
    let ns = find_exact_match ~sienv ~namespace:"Str" in
    SA.assert_equals "\\HH\\Lib\\Str" ns.nss_full_namespace "Alias search";
    let ns = find_exact_match ~sienv ~namespace:"Str\\fb" in
    SA.assert_equals "\\HH\\Lib\\Str\\fb" ns.nss_full_namespace "Alias search";

    (* Register an alias with a leading backslash *)
    register_alias ~sienv ~alias:"StrFb" ~target:"\\HH\\Lib\\Str\\fb";
    let ns = find_exact_match ~sienv ~namespace:"StrFb" in
    SA.assert_equals "\\HH\\Lib\\Str\\fb" ns.nss_full_namespace "Alias search";

    (* Add a new namespace under an alias, and make sure it's visible *)
    register_namespace ~sienv ~namespace:"StrFb\\SecureRandom";
    let ns =
      find_exact_match ~sienv ~namespace:"\\hh\\lib\\str\\fb\\securerandom"
    in
    SA.assert_equals
      "\\HH\\Lib\\Str\\fb\\SecureRandom"
      ns.nss_full_namespace
      "Late bound namespace";

    (* Should always be able to find root *)
    let ns = find_exact_match ~sienv ~namespace:"\\" in
    SA.assert_equals "\\" ns.nss_full_namespace "Find root";
    let ns = find_exact_match ~sienv ~namespace:"" in
    SA.assert_equals "\\" ns.nss_full_namespace "Find root";
    let ns = find_exact_match ~sienv ~namespace:"\\\\" in
    SA.assert_equals "\\" ns.nss_full_namespace "Find root";

    (* Test partial matches *)
    let matches = find_matching_namespaces ~sienv ~query_text:"st" in
    assert_ns_matches "Str" matches;

    (* Assuming we're in a leaf node, find matches under that leaf node *)
    let matches = find_matching_namespaces ~sienv ~query_text:"StrFb\\Secu" in
    assert_ns_matches "SecureRandom" matches;

    (* Special case: empty string always provides zero matches *)
    let matches = find_matching_namespaces ~sienv ~query_text:"" in
    IA.assert_equals 0 (List.length matches) "Empty string / zero matches";

    (* Special case: single backslash should show at least these root namespaces *)
    let matches = find_matching_namespaces ~sienv ~query_text:"\\" in
    assert_ns_matches "Str" matches;
    assert_ns_matches "StrFb" matches;
    assert_ns_matches "HH" matches;

    (* Normal use case *)
    Hh_logger.log "Reached the hh section";
    let matches = find_matching_namespaces ~sienv ~query_text:"hh" in
    assert_ns_matches "Lib" matches;
    let matches = find_matching_namespaces ~sienv ~query_text:"\\HH\\" in
    assert_ns_matches "Lib" matches;

    true)

(* Rapid unit tests to verify docblocks are found and correct *)
let test_docblock_finder (harness : Test_harness.t) : bool =
  let _ = harness in
  let init_id = Random_id.short_string () in
  let env = ServerEnvBuild.make_env ~init_id ServerConfig.default_config in
  let handle = SharedMem.init ~num_workers:0 SharedMem.default_config in
  ignore (handle : SharedMem.handle);

  (* Search for docblocks for various items *)
  let ctx = Provider_utils.ctx_from_server_env env in
  Relative_path.set_path_prefix Relative_path.Root harness.repo_dir;
  let path =
    Relative_path.create_detect_prefix
      (Path.to_string (Path.concat harness.repo_dir "/bar_1.php"))
  in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let docblock =
    ServerDocblockAt.go_docblock_ctx
      ~ctx
      ~entry
      ~line:6
      ~column:7
      ~kind:SI_Trait
  in
  assert_docblock_markdown
    [DocblockService.Markdown "This is a docblock for NoBigTrait"]
    docblock;
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
    ( "test_sqlite_plus_local",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_sqlite_plus_local );
    ( "test_builder_names",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_builder_names );
    ( "test_namespace_map",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_namespace_map );
    ( "test_docblock_finder",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_docblock_finder );
  ]

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  EventLogger.init_fake ();
  Unit_test.run_all (tests args)

open Core_kernel
open IndexBuilder
open SearchUtils
open Test_harness

module Args = Test_harness_common_args

let assert_autocomplete
    ~(query_text: string)
    ~(kind: si_kind)
    ~(expected: int)
    ~(env: local_tracking_env ref): unit =

  (* Search for the symbol *)
  let results = SymbolIndex.find_matching_symbols
    ~query_text
    ~max_results:100
    ~kind_filter:(Some kind)
    ~env:!env in

  (* Verify correct number of results *)
  Asserter.Int_asserter.assert_equals
    expected (List.length results)
    (Printf.sprintf "Should be %d result(s) for [%s]" expected query_text);
  if (expected > 0) then begin
    let r = List.hd_exn results in

    (* Assert string match *)
    Asserter.String_asserter.assert_equals query_text r.si_name
      (Printf.sprintf "Should be able to find autocomplete for '%s'" query_text);

    (* Assert kind match *)
    Asserter.Int_asserter.assert_equals (kind_to_int kind) (kind_to_int r.si_kind)
      (Printf.sprintf "Mismatch in kind for '%s'" query_text);
  end;
;;

let run_index_builder (harness: Test_harness.t): unit =
  Relative_path.set_path_prefix Relative_path.Root harness.repo_dir;
  let repo_path = Path.to_string harness.repo_dir in

  (* Set up initial variables *)
  let fn = Filename.temp_file "autocomplete." ".db" in
  let file_opt = Some fn in
  let ctxt = {
    repo_folder = repo_path;
    sqlite_filename = file_opt;
    text_filename = None;
    json_filename = None;
    json_chunk_size = 0;
    custom_service = None;
    custom_repo_name = None;
    include_builtins = true;
  } in

  (* Scan the repo folder and produce answers in sqlite *)
  IndexBuilder.go ctxt None;
  SymbolIndex.set_search_provider
    ~quiet:false
    ~provider_name:"SqliteIndex"
    ~savedstate_file_opt:file_opt
    ~workers:None;
  Hh_logger.log "Built Sqlite database [%s]" fn;
;;

let test_sqlite_plus_local (harness: Test_harness.t): bool =
  run_index_builder harness;
  let env = ref {
    lte_fileinfos = Relative_path.Map.empty;
    lte_filenames = Relative_path.Map.empty;
    lte_tombstones = Tombstone_set.empty;
  } in

  (* Find one of each major type *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:1 ~env;
  assert_autocomplete ~query_text:"some_long_function_name" ~kind:SI_Function ~expected:1 ~env;
  assert_autocomplete ~query_text:"ClassToBeIdentified" ~kind:SI_Class ~expected:1 ~env;
  Hh_logger.log "First pass complete";

  (* Now, let's remove a few files and try again - assertions should change *)
  let bar1path = Relative_path.from_root "/bar_1.php" in
  let foo3path = Relative_path.from_root "/foo_3.php" in
  let s = Relative_path.Set.empty in
  let s = Relative_path.Set.add s bar1path in
  let s = Relative_path.Set.add s foo3path in
  SymbolIndex.remove_files s env;
  Hh_logger.log "Removed files";

  (* Two of these have been removed! *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:0 ~env;
  assert_autocomplete ~query_text:"some_long_function_name" ~kind:SI_Function ~expected:0 ~env;
  assert_autocomplete ~query_text:"ClassToBeIdentified" ~kind:SI_Class ~expected:1 ~env;
  Hh_logger.log "Second pass complete";

  (* Add the files back! *)
  let nobigtrait_id =
    ((FileInfo.File (FileInfo.Class, bar1path)), "\\NoBigTrait") in
  let some_long_function_name_id =
    ((FileInfo.File (FileInfo.Class, foo3path)), "\\some_long_function_name") in
  let bar1fileinfo = { FileInfo.
    hash = None;
    file_mode = None;
    funs = [];
    classes = [nobigtrait_id];
    typedefs = [];
    consts = [];
    comments = Some [];
  } in
  let foo3fileinfo = { FileInfo.
    hash = None;
    file_mode = None;
    funs = [some_long_function_name_id];
    classes = [];
    typedefs = [];
    consts = [];
    comments = Some [];
  } in
  let changelist = (bar1path, Full bar1fileinfo, TypeChecker)
    :: [(foo3path, Full foo3fileinfo, TypeChecker)] in
  SymbolIndex.update_files None changelist env;
  let n = LocalSearchService.count_local_fileinfos !env in
  Hh_logger.log "Added back; local search service now contains %d files" n;

  (* Find one of each major type *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"some_long_function_name" ~kind:SI_Function ~expected:1 ~env;
  assert_autocomplete ~query_text:"ClassToBeIdentified" ~kind:SI_Class ~expected:1 ~env;
  Hh_logger.log "Third pass complete";

  (* If we got here, all is well *)
  true
;;

(* Test the ability of the index builder to capture a variety of
 * names and kinds correctly *)
let test_builder_names (harness: Test_harness.t): bool =
  run_index_builder harness;
  let env = ref {
    lte_fileinfos = Relative_path.Map.empty;
    lte_filenames = Relative_path.Map.empty;
    lte_tombstones = Tombstone_set.empty;
  } in

  (* Assert that we can capture all kinds of symbols *)
  assert_autocomplete ~query_text:"UsesA" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"NoBigTrait" ~kind:SI_Trait ~expected:1 ~env;
  assert_autocomplete ~query_text:"some_long_function_name" ~kind:SI_Function ~expected:1 ~env;
  assert_autocomplete ~query_text:"ClassToBeIdentified" ~kind:SI_Class ~expected:1 ~env;
  assert_autocomplete ~query_text:"CONST_SOME_COOL_VALUE" ~kind:SI_GlobalConstant ~expected:1 ~env;
  assert_autocomplete ~query_text:"IMyFooInterface" ~kind:SI_Interface ~expected:1 ~env;
  assert_autocomplete ~query_text:"SomeTypeAlias" ~kind:SI_Typedef ~expected:1 ~env;
  assert_autocomplete ~query_text:"FbidMapField" ~kind:SI_Enum ~expected:1 ~env;

  (* XHP is considered a class at the moment - this may change *)
  assert_autocomplete ~query_text:":xhp:helloworld" ~kind:SI_Class ~expected:1 ~env;

  (* All good *)
  true
;;

(* Main test suite *)
let tests args =
  let harness_config = {
    Test_harness.hh_server = args.Args.hh_server;
    hh_client = args.Args.hh_client;
    template_repo = args.Args.template_repo;
  } in
  [
  ("test_sqlite_plus_local", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_sqlite_plus_local);
  ("test_builder_names", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_builder_names);
  ]
;;

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  let () = HackEventLogger.client_init (args.Args.template_repo) in
  Unit_test.run_all (tests args)

open Hh_prelude
open SearchUtils
open SearchTypes
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

let assert_ns_matches (expected_ns : string) (actual : SearchTypes.si_item list)
    : unit =
  let found =
    List.fold actual ~init:false ~f:(fun acc item ->
        String.equal item.si_name expected_ns || acc)
  in
  (if not found then
    let results_str =
      List.fold actual ~init:"" ~f:(fun acc item -> acc ^ ";" ^ item.si_name)
    in
    let msg =
      Printf.sprintf "Did not find [%s] in [%s]" expected_ns results_str
    in
    IA.assert_equals 0 1 msg);
  ()

let assert_autocomplete
    ~(query_text : string)
    ~(kind : FileInfo.si_kind)
    ~(expected : int)
    ~(sienv_ref : si_env ref) : unit =
  let context =
    match kind with
    | FileInfo.SI_Interface
    | FileInfo.SI_Enum ->
      Actype
      (* the `Acid` context rules out interfaces+enums, so we pick one that allows them *)
    | _ -> Acid
  in
  (* Search for the symbol *)
  let (results, _is_complete) =
    SymbolIndex.find_matching_symbols
      ~sienv_ref
      ~query_text
      ~max_results:100
      ~kind_filter:(Some kind)
      ~context
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
  let options = ServerArgs.default_options ~root:repo_path in
  let (hhconfig, _) = ServerConfig.load ~silent:true options in
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
      ~provider_name:"LocalIndex"
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

(* Test the ability of the index builder to capture a variety of
 * names and kinds correctly *)
let test_builder_names (harness : Test_harness.t) : bool =
  let sienv = run_index_builder harness in
  let sienv_ref = ref sienv in

  (* Assert that we can capture all kinds of symbols *)
  assert_autocomplete
    ~query_text:"UsesA"
    ~kind:FileInfo.SI_Class
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"NoBigTrait"
    ~kind:FileInfo.SI_Trait
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"some_long_function_name"
    ~kind:FileInfo.SI_Function
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"ClassToBeIdentified"
    ~kind:FileInfo.SI_Class
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"CONST_SOME_COOL_VALUE"
    ~kind:FileInfo.SI_GlobalConstant
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"IMyFooInterface"
    ~kind:FileInfo.SI_Interface
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"SomeTypeAlias"
    ~kind:FileInfo.SI_Typedef
    ~expected:1
    ~sienv_ref;
  assert_autocomplete
    ~query_text:"FbidMapField"
    ~kind:FileInfo.SI_Enum
    ~expected:1
    ~sienv_ref;

  (* XHP is considered a class at the moment - this may change.
   * Note that XHP classes are saved WITH a leading colon.
   * Storing them without the leading colon results in incorrect
   * text for XHP class autocomplete. *)
  assert_autocomplete
    ~query_text:":xhp:helloworld"
    ~kind:FileInfo.SI_Class
    ~expected:1
    ~sienv_ref;

  (* All good *)
  true

(* Rapid unit tests to verify docblocks are found and correct *)
let test_docblock_finder (harness : Test_harness.t) : bool =
  let _ = harness in
  let init_id = Random_id.short_string () in
  let env =
    ServerEnvBuild.make_env
      ~init_id
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
      ServerConfig.default_config
  in
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
      ~kind:FileInfo.SI_Trait
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
    ( "test_builder_names",
      fun () ->
        Test_harness.run_test
          ~stop_server_in_teardown:false
          harness_config
          test_builder_names );
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

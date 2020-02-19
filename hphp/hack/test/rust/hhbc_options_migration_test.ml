open Core_kernel
open OUnit2

(* Useful debugging, since no ppx printer for Hhbc_options.t *)
let _assert_opts_equal caml rust =
  assert_equal
    Hhbc_options.(constant_folding caml)
    Hhbc_options.(constant_folding rust);
  assert_equal
    Hhbc_options.(optimize_null_checks caml)
    Hhbc_options.(optimize_null_checks rust);
  assert_equal
    Hhbc_options.(max_array_elem_size_on_the_stack caml)
    Hhbc_options.(max_array_elem_size_on_the_stack rust);
  assert_equal
    Hhbc_options.(aliased_namespaces caml)
    Hhbc_options.(aliased_namespaces rust);
  assert_equal
    Hhbc_options.(source_mapping caml)
    Hhbc_options.(source_mapping rust);
  assert_equal Hhbc_options.(relabel caml) Hhbc_options.(relabel rust);
  assert_equal
    Hhbc_options.(enable_uniform_variable_syntax caml)
    Hhbc_options.(enable_uniform_variable_syntax rust);
  assert_equal
    Hhbc_options.(php7_ltr_assign caml)
    Hhbc_options.(php7_ltr_assign rust);
  assert_equal
    Hhbc_options.(hack_arr_compat_notices caml)
    Hhbc_options.(hack_arr_compat_notices rust);
  assert_equal
    Hhbc_options.(hack_arr_dv_arrs caml)
    Hhbc_options.(hack_arr_dv_arrs rust);
  assert_equal
    Hhbc_options.(dynamic_invoke_functions caml)
    Hhbc_options.(dynamic_invoke_functions rust);
  assert_equal
    Hhbc_options.(repo_authoritative caml)
    Hhbc_options.(repo_authoritative rust);
  assert_equal
    Hhbc_options.(jit_enable_rename_function caml)
    Hhbc_options.(jit_enable_rename_function rust);
  assert_equal
    Hhbc_options.(enable_coroutines caml)
    Hhbc_options.(enable_coroutines rust);
  assert_equal
    Hhbc_options.(hacksperimental caml)
    Hhbc_options.(hacksperimental rust);
  assert_equal Hhbc_options.(doc_root caml) Hhbc_options.(doc_root rust);
  assert_equal
    Hhbc_options.(include_search_paths caml)
    Hhbc_options.(include_search_paths rust);
  assert_equal
    Hhbc_options.(include_roots caml)
    Hhbc_options.(include_roots rust);
  assert_equal
    Hhbc_options.(log_extern_compiler_perf caml)
    Hhbc_options.(log_extern_compiler_perf rust);
  assert_equal
    Hhbc_options.(enable_intrinsics_extension caml)
    Hhbc_options.(enable_intrinsics_extension rust);
  assert_equal
    Hhbc_options.(phpism_disallow_execution_operator caml)
    Hhbc_options.(phpism_disallow_execution_operator rust);
  assert_equal
    Hhbc_options.(phpism_disable_nontoplevel_declarations caml)
    Hhbc_options.(phpism_disable_nontoplevel_declarations rust);
  assert_equal
    Hhbc_options.(phpism_disable_static_closures caml)
    Hhbc_options.(phpism_disable_static_closures rust);
  assert_equal
    Hhbc_options.(phpism_disable_halt_compiler caml)
    Hhbc_options.(phpism_disable_halt_compiler rust);
  assert_equal
    Hhbc_options.(emit_func_pointers caml)
    Hhbc_options.(emit_func_pointers rust);
  assert_equal
    Hhbc_options.(emit_cls_meth_pointers caml)
    Hhbc_options.(emit_cls_meth_pointers rust);
  assert_equal
    Hhbc_options.(emit_inst_meth_pointers caml)
    Hhbc_options.(emit_inst_meth_pointers rust);
  assert_equal
    Hhbc_options.(emit_meth_caller_func_pointers caml)
    Hhbc_options.(emit_meth_caller_func_pointers rust);
  assert_equal
    Hhbc_options.(rx_is_enabled caml)
    Hhbc_options.(rx_is_enabled rust);
  assert_equal
    Hhbc_options.(disable_lval_as_an_expression caml)
    Hhbc_options.(disable_lval_as_an_expression rust);
  assert_equal
    Hhbc_options.(enable_pocket_universes caml)
    Hhbc_options.(enable_pocket_universes rust);
  assert_equal
    Hhbc_options.(array_provenance caml)
    Hhbc_options.(array_provenance rust);
  assert_equal
    Hhbc_options.(enable_class_level_where_clauses caml)
    Hhbc_options.(enable_class_level_where_clauses rust);
  assert_equal
    Hhbc_options.(disable_legacy_soft_typehints caml)
    Hhbc_options.(disable_legacy_soft_typehints rust);
  assert_equal
    Hhbc_options.(allow_new_attribute_syntax caml)
    Hhbc_options.(allow_new_attribute_syntax rust);
  assert_equal
    Hhbc_options.(disable_legacy_attribute_syntax caml)
    Hhbc_options.(disable_legacy_attribute_syntax rust);
  assert_equal
    Hhbc_options.(const_default_func_args caml)
    Hhbc_options.(const_default_func_args rust);
  assert_equal
    Hhbc_options.(const_static_props caml)
    Hhbc_options.(const_static_props rust);
  assert_equal
    Hhbc_options.(abstract_static_props caml)
    Hhbc_options.(abstract_static_props rust);
  assert_equal
    Hhbc_options.(disable_unset_class_const caml)
    Hhbc_options.(disable_unset_class_const rust);
  assert_equal
    Hhbc_options.(disallow_func_ptrs_in_constants caml)
    Hhbc_options.(disallow_func_ptrs_in_constants rust);
  assert_equal
    Hhbc_options.(emit_generics_ub caml)
    Hhbc_options.(emit_generics_ub rust);
  assert_equal
    Hhbc_options.(check_int_overflow caml)
    Hhbc_options.(check_int_overflow rust);
  ()

let json_override_2bools =
  "
{
\"hhvm.hack.lang.enable_coroutines\": { \"global_value\": \"0\" },
\"hhvm.hack.lang.hacksperimental\": { \"global_value\": \"1\" }
}
"

(* Sanity-check test data: if this fails other tests are meaningless *)
let test_override_2bools_sanity_check _ =
  let opts =
    Hhbc_options.get_options_from_config
      (Some (Hh_json.json_of_string json_override_2bools))
  in
  assert_equal false Hhbc_options.(enable_coroutines opts);
  assert_equal true Hhbc_options.(hacksperimental opts);

  (* Sanity check that we're actually overriding *)
  assert_equal
    Hhbc_options.(enable_coroutines opts)
    (not Hhbc_options.(enable_coroutines default));
  assert_equal
    Hhbc_options.(hacksperimental opts)
    (not Hhbc_options.(hacksperimental default));
  ()

(* FFI tests *)

let test_override_2bools_configs_to_json_ffi _ =
  let opts = Options_ffi.from_configs ~jsons:[json_override_2bools] ~args:[] in
  assert_equal false Hhbc_options.(enable_coroutines opts);
  assert_equal true Hhbc_options.(hacksperimental opts);
  ()

let test_ffi_defaults_consistent _ =
  let rust = Options_ffi.get_default () in
  let caml = Hhbc_options.default in
  assert_equal caml rust;
  ()

let test_aliased_namespaces_object_configs_to_json_ffi _ =
  let opts =
    Options_ffi.from_configs
      ~args:[]
      ~jsons:
        [
          "
      { \"hhvm.aliased_namespaces\": { \"global_value\": {\"foo\": \"bar\"} } }
      ";
        ]
  in
  assert_equal [("foo", "bar")] Hhbc_options.(aliased_namespaces opts);
  ()

let test_aliased_namespaces_empty_configs_to_json_ffi _ =
  (* Note: HackC must understand [] as {} for hhvm.aliased_namespaces *)
  let opts =
    Options_ffi.from_configs
      ~args:[]
      ~jsons:
        [
          "
      { \"hhvm.aliased_namespaces\": { \"global_value\": [] } }
      ";
        ]
  in
  assert_equal [] Hhbc_options.(aliased_namespaces opts);

  (* Test forward compatibility; i.e., deserialization from {} (empty map) *)
  let opts =
    Options_ffi.from_configs
      ~args:[]
      ~jsons:
        [
          "
      { \"hhvm.aliased_namespaces\": { \"global_value\": {} } }
      ";
        ]
  in
  assert_equal [] Hhbc_options.(aliased_namespaces opts);
  ()

let caml_from_configs ~jsons ~args =
  Hhbc_options.apply_config_overrides_statelessly args jsons

let test_json_configs_stackable _ =
  (* See hhbc/options.rs test_options_de_multiple_jsons for comments *)
  let json_configs =
    List.rev
      [
        "
  {
     \"hhvm.hack.lang.enable_coroutines\": { \"global_value\": \"0\" },
     \"hhvm.rx_is_enabled\": { \"global_value\": \"0\" }
  }
      ";
        "
  {
     \"hhvm.hack.lang.hacksperimental\": { \"global_value\": \"1\" },
     \"hhvm.rx_is_enabled\": { \"global_value\": \"1\" }
  }
      ";
      ]
  in
  let caml_configs =
    List.map ~f:(fun s -> Some (Hh_json.json_of_string s)) json_configs
  in
  (* Sanity checks *)
  let caml_opts = caml_from_configs ~jsons:caml_configs ~args:[] in
  (* set to 0 in the first JSON, so it must stay 0 *)
  assert_equal false Hhbc_options.(enable_coroutines caml_opts);

  (* set to 1 in the second JSON, so it must stay 1 *)
  assert_equal true Hhbc_options.(hacksperimental caml_opts);

  (* set to 0 in the first JSON, then to 1 in the second JSON; so it's 1 now *)
  assert_equal true Hhbc_options.(rx_is_enabled caml_opts);

  (* Verify Rust implementation behind FFI gives the same results *)
  let rust_opts = Options_ffi.from_configs json_configs [] in
  assert_equal
    Hhbc_options.(enable_coroutines caml_opts)
    Hhbc_options.(enable_coroutines rust_opts);
  assert_equal
    Hhbc_options.(hacksperimental caml_opts)
    Hhbc_options.(hacksperimental rust_opts);
  assert_equal
    Hhbc_options.(rx_is_enabled caml_opts)
    Hhbc_options.(rx_is_enabled rust_opts);
  ()

let test_no_overrides _ =
  let caml_opts = caml_from_configs ~jsons:[] ~args:[] in
  let rust_opts = Options_ffi.from_configs ~jsons:[] ~args:[] in
  _assert_opts_equal caml_opts rust_opts;
  ()

let () =
  "hhbc_options_migration"
  >::: [
         "test_override_2bools_sanity_check"
         >:: test_override_2bools_sanity_check;
         "test_override_2bools_single_JSON_FFI"
         >:: test_override_2bools_configs_to_json_ffi;
         "test_ffi_defaults_consistent" >:: test_ffi_defaults_consistent;
         "test_aliased_namespaces_object_configs_to_json_ffi"
         >:: test_aliased_namespaces_object_configs_to_json_ffi;
         "test_aliased_namespaces_empty_configs_to_json_ffi"
         >:: test_aliased_namespaces_empty_configs_to_json_ffi;
         "test_json_configs_stackable" >:: test_json_configs_stackable;
         "test_no_overrides" >:: test_no_overrides;
       ]
  |> run_test_tt_main

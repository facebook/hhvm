(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree

open Core
open OUnit

type test_case = {
  name: string;
  source: string;
  expected: string;
  test_function: string -> string;
}

(** This actually uses the classic parser, not the full-fidelity parser. *)
let sanity_test_classic_parser source =
  let errorl, result, _ =
    Errors.do_ begin fun () ->
      Parser_hack.program_with_default_popt
        (Relative_path.default)
        source
    end
  in
  if not (Errors.is_empty errorl) then begin
    let errors = Errors.get_error_list errorl in
    List.fold_left errors ~init:"" ~f:begin fun acc e ->
      acc ^ (Errors.to_string (Errors.to_absolute e)) ^ "\n"
    end
  end
  else Debug.dump_ast (Ast.AProgram result.Parser_hack.ast)

let full_fidelity_to_classic source =
  let source = SourceText.make source in
  let syntax_tree = SyntaxTree.make source in
  let classic_ast = Classic_ast_mapper.from_tree syntax_tree in
  let str = Debug.dump_ast (Ast.AProgram classic_ast) in
  str

let simple_source_1 =
"<?hh
function foo(int $a, int $b): int {
  return $a + $b;
}
"

(** Output from hh_single_parse. *)
let simple_expected_1 = "(AProgram
 ((Fun
   ((f_mode: Mpartial) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p int) ())))) (f_name: (p \"\\\\foo\"))
    (f_params:
     (((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p $a)) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))
      ((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p $b)) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))))
    (f_body:
     ((Return p ((p (Binop Plus (p (Lvar (p $a))) (p (Lvar (p $b)))))))))
    (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_class_uses: (SMap ())) (ns_fun_uses: (SMap ()))
      (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

let simple_source_2 =
"<?hh
function foo(int $a, int $b): void {
}"

(** Output from hh_single_parse. *)
let simple_expected_2 = "(AProgram
 ((Fun
   ((f_mode: Mpartial) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p foo))
    (f_params:\n     (((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p \"\")) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))
      ((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p \"\")) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))))
    (f_body: ()) (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_class_uses: (SMap ())) (ns_fun_uses: (SMap ()))
      (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

let test_data = [
  {
    name = "sanity_test_classic_parser";
    source = simple_source_1;
    expected = simple_expected_1;
    test_function = sanity_test_classic_parser;
  };
  {
    name = "Basic test full fidelity to classic AST";
    source = simple_source_2;
    expected = simple_expected_2;
    test_function = full_fidelity_to_classic;
  };
]

let driver test () =
  let actual = test.test_function test.source in
  assert_equal test.expected actual

let run_test test =
  test.name >:: (driver test)

let run_tests tests =
  List.map tests ~f:run_test

let test_suite =
  "Classic_mapper_suite" >::: (run_tests test_data)

let main () =
  EventLogger.init (Daemon.devnull ()) 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  run_test_tt_main test_suite

let _ = main ()

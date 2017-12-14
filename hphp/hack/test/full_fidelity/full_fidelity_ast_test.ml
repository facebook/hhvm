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
module Lowerer = Full_fidelity_ast

open Hh_core
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
  let stats = Stats_container.new_container () in
  let path = Relative_path.create Relative_path.Dummy "<inline>" in
  let classic_ast =
    Lowerer.(from_text_with_legacy (make_env ~stats path) source) in
  let str = Debug.dump_ast (Ast.AProgram classic_ast.Parser_hack.ast) in
  str

let comment_compare source =
  let errorl, legacy, _ =
    Errors.do_ begin fun () ->
      Parser_hack.program_with_default_popt
        (Relative_path.default)
        source
    end
  in
  if not (Errors.is_empty errorl) then begin
    let errors = Errors.get_error_list errorl in
    let errors = List.fold_left errors ~init:"" ~f:begin fun acc e ->
        acc ^ (Errors.to_string (Errors.to_absolute e)) ^ "\n"
      end
    in
    Printf.eprintf ("Legacy parser gave errors:\n%s") errors;
    exit (-1);
  end;

  let errorl, result, _ =
    let path = Relative_path.default in
    let env = Lowerer.make_env ~include_line_comments:true path in
    let source_text = SourceText.make path source in
    Errors.do_ @@ fun () -> Lowerer.from_text env source_text
  in
  if not (Errors.is_empty errorl) then begin
    let errors = Errors.get_error_list errorl in
    let errors = List.fold_left errors ~init:"" ~f:begin fun acc e ->
        acc ^ (Errors.to_string (Errors.to_absolute e)) ^ "\n"
      end
    in
    Printf.eprintf ("Full fidelity parser gave errors:\n%s") errors;
    exit (-1);
  end;

  Debug.dump_ast (Ast.AProgram legacy.Parser_hack.ast) ^
    Debug.dump_ast (Ast.AProgram result.Lowerer.ast)


let simple_source_1 =
"<?hh
function foo(int $a, int $b): int {
  return $a + $b;
}
"

(** Output from hh_single_parse. *)
let simple_expected_1 =
"(AProgram\n ((Fun\n   ((f_mode: Mpartial) (f_tparams: ()) (f_ret_by_ref: false)\n    (f_ret: ((p (Happly (p int) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())\n    (f_params:\n     (((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)\n       (param_is_variadic: false) (param_id: (p $a)) (param_expr: ())\n       (param_modifier: ()) (param_callconv: ()) (param_user_attributes: ()))\n      ((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)\n       (param_is_variadic: false) (param_id: (p $b)) (param_expr: ())\n       (param_modifier: ()) (param_callconv: ()) (param_user_attributes: ()))))\n    (f_body:\n     ((p (Return ((p (Binop Plus (p (Lvar (p $a))) (p (Lvar (p $b))))))))))\n    (f_user_attributes: ()) (f_fun_kind: FSync)\n    (f_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (f_span: p)))))"

let simple_source_2 =
"<?hh
function foo(int $a, int $b): void {
}"

(** Output from hh_single_parse. *)
let simple_expected_2 =
"(AProgram\n ((Stmt (p (Markup (p \"\") ())))\n  (Fun\n   ((f_mode: Mpartial) (f_tparams: ()) (f_ret_by_ref: false)\n    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())\n    (f_params:\n     (((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)\n       (param_is_variadic: false) (param_id: (p $a)) (param_expr: ())\n       (param_modifier: ()) (param_callconv: ()) (param_user_attributes: ()))\n      ((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)\n       (param_is_variadic: false) (param_id: (p $b)) (param_expr: ())\n       (param_modifier: ()) (param_callconv: ()) (param_user_attributes: ()))))\n    (f_body: ((p \"\"))) (f_user_attributes: ()) (f_fun_kind: FSync)\n    (f_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (f_span: p)))))"

let comment_scraper_1 =
"<?hh // strict
function foo(): void {
  // UNSAFE
  bar();
  /* This does something bad */
}"

let comment_scraper_expected =
"(AProgram\n ((Fun\n   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)\n    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())\n    (f_params: ())\n    (f_body: ((p Unsafe) (p (Expr (p (Call (p (Id (p bar))) () () ()))))))\n    (f_user_attributes: ()) (f_fun_kind: FSync)\n    (f_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (f_span: p)))))(AProgram\n ((Stmt (p (Markup (p \"\") ())))\n  (Fun\n   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)\n    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())\n    (f_params: ())\n    (f_body: ((p Unsafe) (p (Expr (p (Call (p (Id (p bar))) () () ()))))))\n    (f_user_attributes: ()) (f_fun_kind: FSync)\n    (f_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (f_span: p)))))"

let type_annotated_function_call =
"<?hh // strict

foo(bar<TypesMakeThis,AnAnnotated>($function_call));
baz(quz<ButThisShouldBe,Two($expressions));
"

let type_annotated_function_call_expected =
"(AProgram
 ((Stmt (p (Markup (p \"\") ())))
  (Stmt
   (p (Expr
    (p
     (Call (p (Id (p foo)))
      ((p
        (Call
         (p
          (Id_type_arguments (p bar)
           ((p (Happly (p TypesMakeThis) ()))
            (p (Happly (p AnAnnotated) ())))))
         ((p (Lvar (p $function_call)))) ())))
      ())))))
  (Stmt
   (p (Expr
    (p
     (Call (p (Id (p baz)))
      ((p (Binop Lt (p (Id (p quz))) (p (Id (p ButThisShouldBe)))))
       (p (Call (p (Id (p Two))) ((p (Lvar (p $expressions)))) ()))))"

let global_keyword =
"<?hh // strict
function foo(): void {
  global $x;
}"

let global_keyword_expected =
"(AProgram\n ((Stmt (p (Markup (p \"\") ())))\n  (Fun\n   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)\n    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())\n    (f_params: ()) (f_body: ((p (Global_var ((p (Id (p $x))))))))\n    (f_user_attributes: ()) (f_fun_kind: FSync)\n    (f_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (f_span: p)))))"
(*"(AProgram
 ((Stmt (p (Markup (p \"\") ())))
  (Fun
   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
    (f_params: ()) (f_body: ((Global_var ((p (Id (p $x)))))))
    (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))"
*)
let string_literals =
"<?hh // strict
final class StringsThatArentNumericLiterals {
  const string BINARY = \"0b101010\";
  const string FEINT_BINARY = \"0b101012\";
  const string HEXADECIMAL = \"0x2A\";
  const string FEINT_HEXADECIMAL = \"0x2G\";
  const string OCTAL = \"052\";
  const string FEINT_OCTAL = \"058\";
}
"

let string_literals_expected =
"(AProgram\n ((Stmt (p (Markup (p \"\") ())))\n  (Class\n   ((c_mode: Mstrict) (c_user_attributes: ()) (c_final: true)\n    (c_kind: Cnormal) (c_is_xhp: false)\n    (c_name: (p \"\\\\StringsThatArentNumericLiterals\")) (c_tparams: ())\n    (c_extends: ()) (c_implements: ())\n    (c_body:\n     ((Const ((p (Happly (p string) ())))\n       (((p BINARY) (p (String (p 0b101010))))))\n      (Const ((p (Happly (p string) ())))\n       (((p FEINT_BINARY) (p (String (p 0b101012))))))\n      (Const ((p (Happly (p string) ())))\n       (((p HEXADECIMAL) (p (String (p 0x2A))))))\n      (Const ((p (Happly (p string) ())))\n       (((p FEINT_HEXADECIMAL) (p (String (p 0x2G))))))\n      (Const ((p (Happly (p string) ()))) (((p OCTAL) (p (String (p 052))))))\n      (Const ((p (Happly (p string) ())))\n       (((p FEINT_OCTAL) (p (String (p 058))))))))\n    (c_namespace:\n     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))\n      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))\n    (c_enum: ()) (c_span: p)))))"

let test_data =
  [ { name = "sanity_test_classic_parser"
    ; source = simple_source_1
    ; expected = simple_expected_1
    ; test_function = sanity_test_classic_parser
    }
  ; { name = "Basic test full fidelity to classic AST"
    ; source = simple_source_2
    ; expected = simple_expected_2
    ; test_function = full_fidelity_to_classic
    }
  ; { name = "Comment scraper test"
    ; source = comment_scraper_1
    ; expected = comment_scraper_expected
    ; test_function = comment_compare
    }
  ; { name = "Global keyword"
    ; source = global_keyword
    ; expected = global_keyword_expected
    ; test_function = full_fidelity_to_classic
    }
  ; { name = "String literals"
    ; source = string_literals
    ; expected = string_literals_expected
    ; test_function = full_fidelity_to_classic
    }
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
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  run_test_tt_main test_suite
let _ = main ()

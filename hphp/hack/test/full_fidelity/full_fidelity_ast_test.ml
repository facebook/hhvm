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
  let path = Relative_path.create Relative_path.Dummy "<inline>" in
  let classic_ast = Full_fidelity_ast.from_text_with_legacy path source in
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
    Errors.do_ @@ fun () ->
      Full_fidelity_ast.from_text
        ~include_line_comments:true
        Relative_path.default
        (Full_fidelity_source_text.make source)
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
    Debug.dump_ast (Ast.AProgram result.Full_fidelity_ast.ast)


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
    (f_ret: ((p (Happly (p int) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
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
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

let simple_source_2 =
"<?hh
function foo(int $a, int $b): void {
}"

(** Output from hh_single_parse. *)
let simple_expected_2 = "(AProgram
 ((Stmt (Markup (p \"\") ()))
  (Fun
   ((f_mode: Mpartial) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
    (f_params:
     (((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p $a)) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))
      ((param_hint: ((p (Happly (p int) ())))) (param_is_reference: false)
       (param_is_variadic: false) (param_id: (p $b)) (param_expr: ())
       (param_modifier: ()) (param_user_attributes: ()))))
    (f_body: (\"\")) (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

let comment_scraper_1 =
"<?hh // strict
function foo(): void {
  // UNSAFE
  bar();
  /* This does something bad */
}"

let comment_scraper_expected =
"(AProgram
 ((Fun
   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
    (f_params: ())
    (f_body: (Unsafe (Expr (p (Call (p (Id (p bar))) () () ())))))
    (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))(AProgram
 ((Stmt (Markup (p \"\") ()))
  (Fun
   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
    (f_params: ()) (f_body: ((Expr (p (Call (p (Id (p bar))) () () ())))))
    (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

let type_annotated_function_call =
"<?hh // strict

foo(bar<TypesMakeThis,AnAnnotated>($function_call));
baz(quz<ButThisShouldBe,Two($expressions));
"

let type_annotated_function_call_expected =
"(AProgram
 ((Stmt (Markup (p \"\") ()))
  (Stmt
   (Expr
    (p
     (Call (p (Id (p foo)))
      ((p
        (Call
         (p
          (Id_type_arguments (p bar)
           ((p (Happly (p TypesMakeThis) ()))
            (p (Happly (p AnAnnotated) ())))))
         ((p (Lvar (p $function_call)))) ())))
      ()))))
  (Stmt
   (Expr
    (p
     (Call (p (Id (p baz)))
      ((p (Binop Lt (p (Id (p quz))) (p (Id (p ButThisShouldBe)))))
       (p (Call (p (Id (p Two))) ((p (Lvar (p $expressions)))) ())))"

let global_keyword =
"<?hh // strict
function foo(): void {
  global $x;
}"

let global_keyword_expected =
"(AProgram
 ((Stmt (Markup (p \"\") ()))
  (Fun
   ((f_mode: Mstrict) (f_tparams: ()) (f_ret_by_ref: false)
    (f_ret: ((p (Happly (p void) ())))) (f_name: (p \"\\\\foo\")) (f_constrs: ())
    (f_params: ()) (f_body: ((Global_var ((p (Id (p $x)))))))
    (f_user_attributes: ()) (f_fun_kind: FSync)
    (f_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (f_span: p)))))"

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
"(AProgram
 ((Stmt (Markup (p \"\") ()))
  (Class
   ((c_mode: Mstrict) (c_user_attributes: ()) (c_final: true)
    (c_kind: Cnormal) (c_is_xhp: false)
    (c_name: (p \"\\\\StringsThatArentNumericLiterals\")) (c_tparams: ())
    (c_extends: ()) (c_implements: ())
    (c_body:
     ((Const ((p (Happly (p string) ())))
       (((p BINARY) (p (String (p 0b101010))))))
      (Const ((p (Happly (p string) ())))
       (((p FEINT_BINARY) (p (String (p 0b101012))))))
      (Const ((p (Happly (p string) ())))
       (((p HEXADECIMAL) (p (String (p 0x2A))))))
      (Const ((p (Happly (p string) ())))
       (((p FEINT_HEXADECIMAL) (p (String (p 0x2G))))))
      (Const ((p (Happly (p string) ()))) (((p OCTAL) (p (String (p 052))))))
      (Const ((p (Happly (p string) ())))
       (((p FEINT_OCTAL) (p (String (p 058))))))))
    (c_namespace:
     ((ns_name: \"\") (ns_ns_uses: (SMap ())) (ns_class_uses: (SMap ()))
      (ns_fun_uses: (SMap ())) (ns_const_uses: (SMap ()))))
    (c_enum: ()) (c_span: p)))))"

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

(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax

open EditableSyntax
open CoroutineSyntax
open Coroutine_state_machine_generator

(**
 * To desguar a coroutine, a parameter representing the subsequent continuation
 * is generated.
 *
 * To avoid conflicting with variadic function types, the contination parameter
 * is generated at the beginning rather than the end of the parameter list.
 *)
let compute_parameter_list parameter_list return_type =
  let coroutine_parameter_syntax =
    make_continuation_parameter_syntax return_type in
  prepend_to_comma_delimited_syntax_list
    coroutine_parameter_syntax
    parameter_list

(**
 * If the provided function declaration header is for a coroutine, rewrites the
 * parameter list and return type as necessary to implement the coroutine.
 *)
let rewrite_function_decl_header
    ({ function_parameter_list; function_type; _; } as node) =
  let make_syntax node = make_syntax (FunctionDeclarationHeader node) in
  make_syntax
    { node with
      function_coroutine = make_missing ();
      function_type = make_coroutine_result_type_syntax function_type;
      function_parameter_list =
        compute_parameter_list function_parameter_list function_type;
    }

let parameter_to_arg param =
  match syntax param with
  | ListItem { list_item; list_separator } ->
    begin
    match syntax list_item with
    | ParameterDeclaration { parameter_name; _; } ->
      let variable_expression = parameter_name in
      make_syntax (VariableExpression { variable_expression } )
    | _ -> failwith "expected parameter declaration in parameter list"
    end
  | _ -> failwith "expected parameter declaration in parameter list"

let parameter_list_to_arg_list function_parameter_list =
  let function_parameter_list = syntax_node_to_list function_parameter_list in
  List.map parameter_to_arg function_parameter_list

(**
 * One of the following, depending on whether the coroutine method is static or
 * not:
 *
 *   - inst_meth($this, "methodName_GeneratedStateMachine")
 *   - class_meth("ClassName", "methodName_GeneratedStateMachine")
 *)
let make_state_machine_method_reference_syntax
    classish_name
    function_name
    method_node =
  let method_name =
    make_string_literal_syntax (make_state_machine_method_name function_name) in
  if is_static_method method_node then
    make_function_call_expression_syntax
      class_meth_syntax
      [ make_string_literal_syntax classish_name; method_name; ]
  else
    make_function_call_expression_syntax
      inst_meth_syntax
      [ this_syntax; method_name; ]

(**
 * Rewrites a coroutine body to instantiate the closure corresponding to the
 * coroutine, pass in or set any necessary variables, and return the result from
 * invoking resume (with a null argument).
 *)

 let rewrite_coroutine_body
     classish_name
     function_name
     { function_parameter_list; function_type; _}
     rewritten_body =
   (* $param1, $param2 *)
   let arg_list = parameter_list_to_arg_list function_parameter_list in

  (* ($closure, $data, $exception) ==> { body } *)
  let lambda_signature = make_closure_lambda_signature
      classish_name
      function_name
      function_type in
  let lambda = make_lambda_syntax lambda_signature rewritten_body in
  let classname = make_closure_classname classish_name function_name in
  (* $continuation,
    ($closure, $data, $exception) ==> { body },
    $param1, $param2 *)
  let parameters =
    continuation_variable_syntax ::
    lambda ::
    arg_list in
  (* new Closure($continuation, ...) *)
  let new_closure_syntax = make_object_creation_expression_syntax
    classname parameters in
  (* new Closure($continuation, ...)->resume *)
  let select_resume_member_syntax =
    make_member_selection_expression_syntax
      new_closure_syntax
      resume_member_name_syntax in

  (* new Closure($continuation, ...)->resume(null) *)
  let call_resume_with_null_syntax =
    make_function_call_expression_syntax
      select_resume_member_syntax
      [null_syntax] in
  (* new Closure($continuation, ...)->resume(null); *)
  let resume_statement_syntax =
    make_expression_statement_syntax call_resume_with_null_syntax in
  (* SuspendedCoroutineResult::create() *)
  let suspended_marker_expression =
    make_static_function_call_expression_syntax
      suspended_coroutine_result_classname_syntax
      suspended_member_name
      [] in
  (* return SuspendedCoroutineResult::create(); *)
  let return_syntax =
    make_return_statement_syntax suspended_marker_expression in
  make_list [resume_statement_syntax; return_syntax]

let try_to_rewrite_coroutine_body
    classish_name
    function_name
    methodish_function_body
    header_node
    rewritten_body =
  match syntax methodish_function_body with
  | CompoundStatement node ->
      let compound_statements = rewrite_coroutine_body
        classish_name
        function_name
        header_node
        rewritten_body in

      Some (make_syntax (CompoundStatement { node with compound_statements }))
  | Missing ->
      Some (methodish_function_body)
  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let maybe_rewrite_methodish_declaration
    classish_name
    function_name
    ({ methodish_function_body; _; } as method_node)
    header_node
    rewritten_body =
  let make_syntax method_node =
    make_syntax (MethodishDeclaration method_node) in
  Option.map
    (try_to_rewrite_coroutine_body classish_name function_name
      methodish_function_body header_node rewritten_body)
    ~f:(fun methodish_function_body ->
      make_syntax
        { method_node with
          methodish_function_decl_header =
            rewrite_function_decl_header header_node;
          methodish_function_body;
        })

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

(**
 * To desguar a coroutine, a parameter representing the subsequent continuation
 * is generated.
 *
 * To avoid conflicting with variadic function types, the contination parameter
 * is generated at the beginning rather than the end of the parameter list.
 *)
let compute_parameter_list parameter_list return_type =
  match syntax parameter_list with
  | Missing ->
      (* No parameters *)
      let coroutine_parameter_syntax =
        make_continuation_parameter_syntax return_type in
      Some [make_list_item coroutine_parameter_syntax (make_missing ())]
  | SyntaxList syntax_list ->
      (* One or more parameters *)
      let coroutine_parameter_syntax =
        make_continuation_parameter_syntax return_type in
      Some (
        make_list_item coroutine_parameter_syntax comma_syntax :: syntax_list
      )
  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the provided function declaration header is for a coroutine, rewrites the
 * parameter list and return type as necessary to implement the coroutine.
 *)
let rewrite_function_decl_header
    ({ function_parameter_list; function_type; _; } as node) =
  let make_syntax node = make_syntax (FunctionDeclarationHeader node) in
  Option.map
    (compute_parameter_list function_parameter_list function_type)
    (fun function_parameter_list ->
      make_syntax
        { node with
          function_coroutine = make_missing ();
          function_type = make_coroutine_result_type_syntax function_type;
          function_parameter_list = make_list function_parameter_list;
        })

let parameter_to_arg param =
  match syntax param with
  | ListItem { list_item; list_separator } ->
    begin
    match syntax list_item with
    | ParameterDeclaration { parameter_name; _; } ->
      let variable_expression = parameter_name in
      let list_item =
        make_syntax (VariableExpression { variable_expression } ) in
      make_syntax (ListItem { list_item; list_separator })
    | _ -> failwith "expected parameter declaration in parameter list"
    end
  | _ -> failwith "expected parameter declaration in parameter list"

let parameter_list_to_arg_list function_parameter_list =
  let function_parameter_list = syntax_node_to_list function_parameter_list in
  List.map parameter_to_arg function_parameter_list

(**
 * Rewrites a coroutine body to instantiate the closure corresponding to the
 * coroutine, pass in or set any necessary variables, and return the result from
 * invoking resume (with a null argument).
 *)
  let rewrite_coroutine_body
    classish_name function_name function_parameter_list =
    let arg_list = parameter_list_to_arg_list function_parameter_list in
    let call_inst_method_on_state_machine_syntax =
      make_function_call_expression_syntax
        inst_meth_syntax
        [
          this_syntax;
          make_string_literal_syntax
            (make_state_machine_method_name function_name);
        ] in
    let classname = make_closure_classname classish_name function_name in
    let parameters =
      continuation_variable_syntax ::
      call_inst_method_on_state_machine_syntax ::
      arg_list in
    let new_closure_syntax = make_object_creation_expression_syntax
      classname parameters in
    let select_resume_member_syntax =
      make_member_selection_expression_syntax
        new_closure_syntax
        resume_member_name_syntax in
    let call_resume_with_null_syntax =
      make_function_call_expression_syntax
        select_resume_member_syntax
        [null_syntax] in
    let resume_statement_syntax =
      make_expression_statement call_resume_with_null_syntax in
    let suspended_marker_expression =
      make_static_function_call_expression_syntax
        suspended_coroutine_result_classname
        suspended_member_name
        [] in
    let return_syntax =
      make_return_statement_syntax suspended_marker_expression in
    make_list [resume_statement_syntax; return_syntax]

let try_to_rewrite_coroutine_body
    classish_name
    function_name
    { methodish_function_body; _; }
    function_parameter_list =
  match syntax methodish_function_body with
  | CompoundStatement node ->
      let compound_statements = rewrite_coroutine_body classish_name
        function_name function_parameter_list in
      Some (make_syntax (CompoundStatement { node with compound_statements }))
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
    method_node
    ({ function_parameter_list; _; } as header_node) =
  let make_syntax method_node =
    make_syntax (MethodishDeclaration method_node) in
  Option.map2
    (rewrite_function_decl_header header_node)
    (try_to_rewrite_coroutine_body
        classish_name function_name method_node function_parameter_list)
    ~f:(fun methodish_function_decl_header methodish_function_body ->
      make_syntax
        { method_node with
          methodish_function_decl_header;
          methodish_function_body;
        })

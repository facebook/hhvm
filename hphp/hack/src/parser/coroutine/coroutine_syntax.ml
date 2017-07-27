(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

(**
 * Contains syntaxes used in coroutine code generation.
 *)

module EditableSyntax = Full_fidelity_editable_syntax
module EditableTrivia = Full_fidelity_editable_trivia
module TokenKind = Full_fidelity_token_kind

open EditableSyntax


(* Common factories *)

let make_syntax syntax =
  make syntax EditableSyntaxValue.NoValue

let single_space = [EditableTrivia.make_whitespace " "]

let make_token_syntax ?text token_kind =
  let text = Option.value text ~default:(TokenKind.to_string token_kind) in
  make_syntax @@
    Token (EditableToken.make token_kind text single_space single_space)

let make_name_syntax text =
  make_token_syntax TokenKind.Name ~text

let make_variable_syntax text =
  make_token_syntax TokenKind.Variable ~text


(* General syntaxes *)

let left_brace_syntax =
  make_token_syntax TokenKind.LeftBrace

let right_brace_syntax =
  make_token_syntax TokenKind.RightBrace

let left_paren_syntax =
  make_token_syntax TokenKind.LeftParen

let right_paren_syntax =
  make_token_syntax TokenKind.RightParen

let left_angle_syntax =
  make_token_syntax TokenKind.LessThan

let right_angle_syntax =
  make_token_syntax TokenKind.GreaterThan

let comma_syntax =
  make_token_syntax TokenKind.Comma

let colon_colon_syntax =
  make_token_syntax TokenKind.ColonColon

let lambda_arrow_syntax =
  make_token_syntax TokenKind.EqualEqualGreaterThan

let semicolon_syntax =
  make_token_syntax TokenKind.Semicolon

let colon_syntax =
  make_token_syntax TokenKind.Colon

let assignment_operator_syntax =
  make_token_syntax TokenKind.Equal

let not_syntax =
  make_token_syntax TokenKind.Exclamation

let not_identical_syntax =
  make_token_syntax TokenKind.ExclamationEqualEqual

let nullable_syntax =
  make_token_syntax TokenKind.Question

let mixed_syntax =
  make_token_syntax TokenKind.Mixed

let mixed_type =
  make_simple_type_specifier mixed_syntax

let global_syntax =
  make_name_syntax "Global"

let unit_syntax =
  make_name_syntax "CoroutineUnit"

let unit_type_syntax =
  make_simple_type_specifier unit_syntax

let int_syntax =
  make_token_syntax TokenKind.Int

let void_syntax =
  make_token_syntax TokenKind.Void

let null_syntax =
  make_token_syntax TokenKind.NullLiteral

let true_expression_syntax =
  make_token_syntax TokenKind.BooleanLiteral ~text:"true"
    |> make_literal_expression

let private_syntax =
  make_token_syntax TokenKind.Private

let public_syntax =
  make_token_syntax TokenKind.Public

let static_syntax =
  make_token_syntax TokenKind.Static

let try_keyword_syntax =
  make_token_syntax TokenKind.Try

let finally_keyword_syntax =
  make_token_syntax TokenKind.Finally

let if_keyword_syntax =
  make_token_syntax TokenKind.If

let else_keyword_syntax =
  make_token_syntax TokenKind.Else

let while_keyword_syntax =
  make_token_syntax TokenKind.While

let switch_keyword_syntax =
  make_token_syntax TokenKind.Switch

let case_keyword_syntax =
  make_token_syntax TokenKind.Case

let class_keyword_syntax =
  make_token_syntax TokenKind.Class

let default_keyword_syntax =
  make_token_syntax TokenKind.Default

let function_keyword_syntax =
  make_token_syntax TokenKind.Function

let goto_keyword_syntax =
  make_token_syntax TokenKind.Goto

let extends_syntax =
  make_token_syntax TokenKind.Extends

let parent_syntax =
  make_token_syntax TokenKind.Parent

let return_keyword_syntax =
  make_token_syntax TokenKind.Return

let throw_keyword_syntax =
  make_token_syntax TokenKind.Throw

let break_statement_syntax =
  make_break_statement
    (make_token_syntax TokenKind.Break)
    (* break_level *) (make_missing ())
    semicolon_syntax

let new_keyword_syntax =
  make_token_syntax TokenKind.New

let member_selection_syntax =
  make_token_syntax TokenKind.MinusGreaterThan

let this_syntax =
  make_variable_syntax "$this"

let exception_type_syntax =
  make_name_syntax "Exception"

let constructor_member_name =
  "__construct"


(* Syntax helper functions *)

let rec delimit delimit_syntax = function
  | [] -> []
  | hd :: [] -> [make_list_item hd (make_missing ())]
  | hd :: tl -> (make_list_item hd delimit_syntax) :: delimit delimit_syntax tl

let make_delimited_list delimit_syntax items =
  make_list (delimit delimit_syntax items)

let prepend_to_comma_delimited_syntax_list prepend_syntax syntax_list_syntax =
  let list_item = make_list_item prepend_syntax comma_syntax in
  let syntax_list = list_item :: syntax_node_to_list syntax_list_syntax in
  make_list syntax_list

let get_list_item node =
  match syntax node with
  | ListItem { list_item; _; } -> list_item
  | _ -> failwith "get_list_item: Was not a ListItem"

let get_type_parameter_list node =
  match syntax node with
  | TypeParameters { type_parameters_parameters; _; } ->
      type_parameters_parameters
        |> syntax_node_to_list
        |> Core_list.map ~f:get_list_item
  | Missing -> []
  | _ -> failwith "get_type_parameter_list: Was not a TypeParameters"

let is_static_method { methodish_modifiers; _; } =
  methodish_modifiers
    |> syntax_node_to_list
    |> Core_list.exists ~f:is_static

let string_of_name_token node =
  match syntax node with
  | Token { EditableToken.kind = TokenKind.Name; text; _; } -> text
  | _ -> failwith "string_of_name_token: Was not a Name Token"


(* Syntax creation functions *)

let make_parenthesized_expression_syntax expression_syntax =
  make_parenthesized_expression
    left_paren_syntax
    expression_syntax
    right_paren_syntax

let make_nullable_type_specifier_syntax type_syntax =
  make_nullable_type_specifier nullable_syntax type_syntax

let make_string_literal_syntax string_literal =
  make_literal_expression
    (make_token_syntax
      TokenKind.DoubleQuotedStringLiteral
      ~text:(Printf.sprintf "\"%s\"" string_literal))

let make_int_literal_syntax value =
  make_literal_expression
    (make_token_syntax TokenKind.DecimalLiteral ~text:(string_of_int value))

let make_qualified_name_syntax name =
  make_qualified_name_expression (make_name_syntax name)

let make_expression_statement_syntax expression_syntax =
  make_expression_statement expression_syntax semicolon_syntax

let make_compound_statement_syntax compound_statements =
  make_compound_statement
    left_brace_syntax
    (make_list compound_statements)
    right_brace_syntax

let make_try_finally_statement_syntax try_compound_statement finally_body =
  make_try_statement
    try_keyword_syntax
    try_compound_statement
    (make_missing()) (* catches *)
    (make_finally_clause
      finally_keyword_syntax
      finally_body)

let make_return_statement_syntax expression_syntax =
  make_return_statement return_keyword_syntax expression_syntax semicolon_syntax

let make_return_missing_statement_syntax =
  make_return_statement_syntax (make_missing())

let make_throw_statement_syntax expression_syntax =
  make_throw_statement throw_keyword_syntax expression_syntax semicolon_syntax

let make_assignment_syntax_variable left assignment_expression_syntax =
  let assignment_binary_expression =
    make_binary_expression
      left
      assignment_operator_syntax
      assignment_expression_syntax in
  make_expression_statement_syntax assignment_binary_expression

let make_not_null_syntax operand =
  make_binary_expression operand not_identical_syntax null_syntax

let make_assignment_syntax receiver_variable assignment_expression_syntax =
  let receiver_variable_syntax =
    make_variable_syntax receiver_variable in
  make_assignment_syntax_variable
    receiver_variable_syntax assignment_expression_syntax

let make_object_creation_expression_syntax classname arguments =
  let classname_syntax = make_name_syntax classname in
  let arguments_syntax = make_delimited_list comma_syntax arguments in
  make_object_creation_expression
    new_keyword_syntax
    classname_syntax
    left_paren_syntax
    arguments_syntax
    right_paren_syntax

let make_function_call_expression_syntax receiver_syntax argument_list =
  let argument_list_syntax = make_delimited_list comma_syntax argument_list in
  make_function_call_expression
    receiver_syntax
    left_paren_syntax
    argument_list_syntax
    right_paren_syntax

let make_static_function_call_expression_syntax
    receiver_syntax
    member_name
    argument_list =
  let member_syntax = make_name_syntax member_name in
  let receiver_syntax =
    make_scope_resolution_expression
      receiver_syntax
      colon_colon_syntax
      member_syntax in
  make_function_call_expression_syntax receiver_syntax argument_list

(**
 * parent::__construct(argument_list);
 *)
let make_construct_parent_syntax argument_list =
  let expression_syntax =
    make_static_function_call_expression_syntax
      parent_syntax
      constructor_member_name
      argument_list in
  make_expression_statement_syntax expression_syntax

let make_member_selection_expression_syntax receiver_syntax member_syntax =
  make_member_selection_expression
    receiver_syntax
    member_selection_syntax
    member_syntax

let make_parameter_declaration_syntax
    ?(visibility_syntax = make_missing ())
    parameter_type_syntax
    parameter_variable =
  let parameter_variable_syntax =
    make_variable_syntax parameter_variable in
  make_parameter_declaration
    (* attribute *)  (make_missing ())
    visibility_syntax
    parameter_type_syntax
    parameter_variable_syntax
    (* default value *)  (make_missing ())

let make_type_parameters_syntax type_parameter_list =
  match type_parameter_list with
  | [] ->
      make_missing ()
  | _ ->
      make_type_parameters
        left_angle_syntax
        (make_delimited_list comma_syntax type_parameter_list)
        right_angle_syntax

let make_type_specifier_syntax classname type_parameter_list =
  let classname_syntax = make_name_syntax classname in
  let type_parameters_syntax =
    make_type_parameters_syntax type_parameter_list in
  if is_missing type_parameters_syntax then
    make_simple_type_specifier classname_syntax
  else
    make_generic_type_specifier classname_syntax type_parameters_syntax

let make_functional_type_syntax argument_types return_type_syntax =
  let argument_types_syntax = make_delimited_list comma_syntax argument_types in
  make_closure_type_specifier
    left_paren_syntax
    (* coroutine *) (make_missing ())
    function_keyword_syntax
    left_paren_syntax
    argument_types_syntax
    right_paren_syntax
    colon_syntax
    return_type_syntax
    right_paren_syntax

(* TODO(tingley): Determine if it's worth tightening visibility here. *)
let make_classish_declaration_syntax
    classname
    type_parameter_list
    extends_list
    classish_body =
  let classname_syntax = make_name_syntax classname in
  let extends_list_syntax =
    make_delimited_list comma_syntax extends_list in
  make_classish_declaration
    (* classish_attribute *) (make_missing ())
    (* classish_modifiers *) (make_missing ())
    class_keyword_syntax
    classname_syntax
    (make_type_parameters_syntax type_parameter_list)
    (if is_missing extends_list_syntax then
      make_missing ()
    else
      extends_syntax)
    extends_list_syntax
    (* classish_implements_keyword *) (make_missing ())
    (* classish_implements_list *) (make_missing ())
    (make_compound_statement_syntax classish_body)

(* TODO(tingley): Determine if it's worth tightening visibility here. *)
let make_methodish_declaration_with_body_syntax
    function_decl_header_syntax
    function_body =
  make_methodish_declaration
    (* methodish_attribute *) (make_missing ())
    (make_list [public_syntax])
    function_decl_header_syntax
    function_body
    (* methodish_semicolon *) (make_missing ())

let make_lambda_signature_syntax lambda_parameters lambda_type =
  let lambda_parameters = make_delimited_list comma_syntax lambda_parameters in
  make_lambda_signature
    left_paren_syntax
    lambda_parameters
    right_paren_syntax
    (if is_missing lambda_type then make_missing () else colon_syntax )
    lambda_type

let make_lambda_syntax lambda_signature lambda_body =
  make_lambda_expression
    (make_missing()) (* async *)
    (make_missing()) (* coroutine *)
    lambda_signature
    lambda_arrow_syntax
    lambda_body

let make_methodish_declaration_syntax
    function_decl_header_syntax
    function_body =
  make_methodish_declaration_with_body_syntax
    function_decl_header_syntax
    (make_compound_statement_syntax function_body)

let make_function_decl_header_syntax
    name
    parameter_list
    return_type_syntax =
  let name_syntax = make_name_syntax name in
  let parameter_list_syntax = make_delimited_list comma_syntax parameter_list in
  make_function_declaration_header
    (* function_async *) (make_missing ())
    (* function_coroutine *) (make_missing ())
    function_keyword_syntax
    (* function_ampersand *) (make_missing ())
    name_syntax
    (* function_type_parameter_list *) (make_missing ())
    left_paren_syntax
    parameter_list_syntax
    right_paren_syntax
    (if is_missing return_type_syntax then make_missing () else colon_syntax)
    return_type_syntax
    (* function_where_clause *) (make_missing ())

let make_constructor_decl_header_syntax name parameter_list =
  make_function_decl_header_syntax name parameter_list (make_missing ())

let make_property_declaration_syntax type_syntax declaration_syntax =
  make_property_declaration
    (make_list [ public_syntax ])
    type_syntax
    declaration_syntax
    semicolon_syntax

let make_if_syntax condition_syntax true_statements =
  make_if_statement
    if_keyword_syntax
    left_paren_syntax
    condition_syntax
    right_paren_syntax
    (make_compound_statement_syntax true_statements)
    (make_missing ())
    (make_missing ())

let make_while_syntax condition_syntax body_statements =
  make_while_statement
    while_keyword_syntax
    left_paren_syntax
    condition_syntax
    right_paren_syntax
    (make_compound_statement_syntax body_statements)

(**
 * while (true) {
 *   body
 * }
 *)
let make_while_true_syntax =
  make_while_syntax true_expression_syntax

let make_not_syntax operand_syntax =
  make_prefix_unary_expression
    not_syntax
    (make_parenthesized_expression_syntax operand_syntax)

(**
 * if (!condition_syntax) {
 *   break;
 * }
 *)
let make_break_unless_syntax do_not_break_condition_syntax =
  make_if_syntax
    (make_not_syntax do_not_break_condition_syntax)
    [ break_statement_syntax; ]

let make_label_declaration_syntax label_name_syntax =
  make_goto_label label_name_syntax colon_syntax

let make_goto_statement_syntax label_name_syntax =
  make_goto_statement goto_keyword_syntax label_name_syntax semicolon_syntax

(* case 0: *)
let make_int_case_label_syntax number =
  let number = make_int_literal_syntax number in
  make_case_label case_keyword_syntax number colon_syntax

(* default: *)
let default_label_syntax =
  make_default_label default_keyword_syntax colon_syntax


(* Coroutine-specific syntaxes *)

let continuation_variable =
  "$coroutineContinuation_generated"

let continuation_variable_syntax =
  make_variable_syntax continuation_variable

let make_closure_base_type_syntax function_type =
  make_type_specifier_syntax "ClosureBase" [ function_type; ]

let make_continuation_type_syntax function_type =
  make_type_specifier_syntax "CoroutineContinuation" [ function_type; ]

let make_continuation_parameter_syntax
    ?visibility_syntax
    function_type =
  make_parameter_declaration_syntax
    ?visibility_syntax
    (make_continuation_type_syntax function_type)
    continuation_variable

let make_coroutine_result_type_syntax function_type =
  make_type_specifier_syntax "CoroutineResult" [ function_type; ]

let suspended_coroutine_result_classname_syntax =
  make_qualified_name_syntax "SuspendedCoroutineResult"

let suspended_member_name =
  "create"

let is_suspended_member_name =
  "isSuspended"

let is_suspended_member_syntax =
  make_name_syntax is_suspended_member_name

let get_result_member_name =
  "getResult"

let get_result_member_syntax =
  make_name_syntax get_result_member_name

(* SuspendedCoroutineResult::create() *)
let create_suspended_coroutine_result_syntax =
  make_static_function_call_expression_syntax
    suspended_coroutine_result_classname_syntax
    suspended_member_name
    []

(* ClassName_FunctionName_Lambda_1_GeneratedClosure *)
let make_closure_classname context =
  let classish_name = context.Coroutine_context.classish_name in
  (* TODO: Is there a better thing to do for coroutines that are inside no
  class? *)
  let classish_name = if is_missing classish_name then
    global_syntax else classish_name in
  let function_name = context.Coroutine_context.function_name in
  let lambda_name = match context.Coroutine_context.lambda_count with
    | None -> ""
    | Some c -> Printf.sprintf "Lambda_%i_" c in
  Printf.sprintf
    "%s_%s_%sGeneratedClosure"
    (string_of_name_token classish_name)
    (string_of_name_token function_name)
    lambda_name

(**
 * Given a list of TypeParameters syntaxes, combines them into a single
 * list of type_parameters.
 TODO: What if there is a name collision?
 *)
let make_closure_type_parameters context =
  let classish_type_parameters =
    context.Coroutine_context.classish_type_parameters in
  let function_type_parameter_list =
    context.Coroutine_context.function_type_parameter_list in
  (get_type_parameter_list classish_type_parameters) @
    (get_type_parameter_list function_type_parameter_list)

(*
    ClassName_FunctionName_GeneratedClosure<TClass, TFunc>
*)
let make_closure_type_syntax context =
  make_type_specifier_syntax
    (make_closure_classname context)
    (make_closure_type_parameters context)

let closure_variable =
  "$closure"

(* $closure *)
let closure_variable_syntax =
  make_variable_syntax closure_variable

(* $closure->name *)
let closure_name_syntax name =
  let name_syntax = make_name_syntax name in
  make_member_selection_expression_syntax closure_variable_syntax name_syntax

let make_closure_parameter_syntax context =
  make_parameter_declaration_syntax
    (make_closure_type_syntax context)
    closure_variable

let resume_member_name =
  "resume"

let resume_member_name_syntax =
  make_name_syntax resume_member_name

let resume_with_exception_member_name =
  "resumeWithException"

let do_resume_member_name =
  "doResume"

let do_resume_member_name_syntax =
  make_name_syntax do_resume_member_name

let coroutine_data_variable =
  "$coroutineData"

let coroutine_data_variable_syntax =
  make_variable_syntax coroutine_data_variable

let coroutine_result_variable =
  "$coroutineResult"

let coroutine_result_variable_syntax =
  make_variable_syntax coroutine_result_variable

let make_coroutine_result_data_member_name index =
  Printf.sprintf "coroutineResultData%d" index

let make_coroutine_result_data_variable index =
  "$" ^ (make_coroutine_result_data_member_name index)

let make_coroutine_result_data_member_name_syntax index =
  make_name_syntax (make_coroutine_result_data_member_name index)

let coroutine_data_type_syntax =
  mixed_syntax

let coroutine_data_parameter_syntax =
  make_parameter_declaration_syntax
    coroutine_data_type_syntax
    coroutine_data_variable

let exception_variable =
  "$exception_generated"

let exception_variable_syntax =
  make_variable_syntax exception_variable

let nullable_exception_type_syntax =
  make_nullable_type_specifier_syntax exception_type_syntax

let exception_parameter_syntax =
  make_parameter_declaration_syntax
    exception_type_syntax
    exception_variable

let nullable_exception_parameter_syntax =
  make_parameter_declaration_syntax
    nullable_exception_type_syntax
    exception_variable

let throw_unimplemented_syntax reason =
  let create_exception_syntax =
    make_object_creation_expression_syntax
      "Exception"
      [make_string_literal_syntax reason] in
  make_throw_statement_syntax create_exception_syntax

let make_state_machine_method_name function_name =
  Printf.sprintf "%s_GeneratedStateMachine" function_name

let state_machine_member_name =
  "stateMachineFunction"

let state_machine_member_name_syntax =
  make_name_syntax state_machine_member_name

let state_machine_variable_name =
  "$" ^ state_machine_member_name

let state_machine_variable_name_syntax =
  make_variable_syntax state_machine_variable_name

let make_state_machine_parameter_syntax context function_type =
  let state_machine_type_syntax =
    make_functional_type_syntax
      [
        make_closure_type_syntax context;
        mixed_syntax;
        nullable_exception_type_syntax;
      ]
      (make_coroutine_result_type_syntax function_type) in
  make_parameter_declaration_syntax
    ~visibility_syntax:private_syntax
    state_machine_type_syntax
    state_machine_variable_name

let inst_meth_syntax =
  make_qualified_name_syntax "inst_meth"

let class_meth_syntax =
  make_qualified_name_syntax "class_meth"

let make_member_with_unknown_type_declaration_syntax variable_name =
  let variable_syntax = make_variable_syntax variable_name in
  let declaration_syntax =
    make_property_declarator variable_syntax (make_missing ()) in
  make_property_declaration_syntax (make_missing ()) declaration_syntax

let coroutine_unit_classname_syntax =
  make_qualified_name_syntax "CoroutineUnit"

(* CoroutineUnit::create() *)
let coroutine_unit_call_syntax =
  make_static_function_call_expression_syntax
    coroutine_unit_classname_syntax
    "create"
    []

let next_label =
  "nextLabel"

(* $closure->nextLabel *)
let label_syntax =
  closure_name_syntax next_label

(* nextLabel *)
let label_name_syntax =
  make_name_syntax next_label

(* $closure->nextLabel = x; *)
let set_next_label_syntax number =
  let number = make_int_literal_syntax number in
  make_assignment_syntax_variable label_syntax number

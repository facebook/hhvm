(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module PositionedToken = Full_fidelity_positioned_token
module SyntaxUtilities =
  Full_fidelity_syntax_utilities.WithSyntax(PositionedSyntax)
module SyntaxError = Full_fidelity_syntax_error
module TokenKind = Full_fidelity_token_kind

open PositionedSyntax

type accumulator = {
  errors : SyntaxError.t list;
}

(* Turns a syntax node into a list of nodes; if it is a separated syntax
   list then the separators are filtered from the resulting list. *)
let syntax_to_list include_separators node  =
  let rec aux acc syntax_list =
    match syntax_list with
    | [] -> acc
    | h :: t ->
      begin
        match syntax h with
        | ListItem { list_item; list_separator } ->
          let acc = list_item :: acc in
          let acc =
            if include_separators then (list_separator :: acc ) else acc in
          aux acc t
        | _ -> aux (h :: acc) t
      end in
  match syntax node with
  | Missing -> [ ]
  | SyntaxList s -> List.rev (aux [] s)
  | ListItem { list_item; list_separator } ->
    if include_separators then [ list_item; list_separator ] else [ list_item ]
  | _ -> [ node ]

let syntax_to_list_no_separators = syntax_to_list false
let syntax_to_list_with_separators = syntax_to_list true

let assert_last_in_list assert_fun node =
  let rec aux lst =
    match lst with
    | []
    | _ :: [] -> None
    | h :: _ when assert_fun h -> Some h
    | _ :: t -> aux t in
  aux (syntax_to_list_no_separators node)

(* If an ellipsis appears in a position other than the last element in the
   list then return it. *)
let misplaced_ellipsis params =
  assert_last_in_list is_ellipsis params

let is_variadic node =
  begin match syntax node with
    | DecoratedExpression { decorated_expression_decorator; _; } ->
      is_ellipsis decorated_expression_decorator
    | _ -> false
  end

let misplaced_variadic_param params =
  let is_variadic node =
    begin match syntax node with
    | ParameterDeclaration { param_attr; param_visibility; param_type;
      param_name; param_default } ->
        is_variadic param_name
    | _ -> false
    end
  in
  assert_last_in_list is_variadic params

let misplaced_variadic_arg args =
  assert_last_in_list is_variadic args

(* If a list ends with an ellipsis followed by a comma, return it *)
let ends_with_ellipsis_comma params =
  let rec aux params =
    match params with
    | [] -> None
    | x :: y :: [] when is_ellipsis x && is_comma y -> Some y
    | _ :: t -> aux t in
  aux (syntax_to_list_with_separators params)

(* True or false: the first item in this list matches the predicate? *)
let matches_first f items =
  match items with
  | h :: _ when f h -> true
  | _ -> false

(* test a node is a syntaxlist and that the list contains an element
 * satisfying a given predicate *)
let list_contains_predicate p node =
  match syntax node with
  | SyntaxList lst ->
    List.exists p lst
  | _ -> false

(* test a node is a syntaxlist and that the list contains multiple elements
 * satisfying a given predicate *)
let list_contains_multiple_predicate p node =
  match syntax node with
  | SyntaxList lst ->
    let count_fun acc el = if p el then acc + 1 else acc in
    (List.fold_left count_fun 0 lst) > 1
  | _ -> false

let list_contains_duplicate node =
  let module SyntaxMap = Map.Make (
    struct
      type t = PositionedSyntax.t
      let compare a b = match syntax a, syntax b with
      | Token x, Token y ->
        Full_fidelity_positioned_token.(compare (kind x) (kind y))
      | _, _ -> Pervasives.compare a b
    end
  ) in
  match syntax node with
  | SyntaxList lst ->
    let check_fun (tbl, acc) el =
      if SyntaxMap.mem el tbl then (tbl, true)
      else (SyntaxMap.add el () tbl, acc)
    in
    let (_, result) = List.fold_left check_fun (SyntaxMap.empty, false) lst in
    result
  | _ ->  false

let token_kind node =
  match syntax node with
  | Token t -> Some (PositionedToken.kind t)
  | _ -> None

let rec containing_classish_kind parents =
  match parents with
  | [] -> None
  | h :: t ->
    begin
      match syntax h with
      | ClassishDeclaration c -> token_kind c.classish_token
      | _ -> containing_classish_kind t
    end

(* tests whether the methodish contains a modifier that satisfies [p] *)
let methodish_modifier_contains_helper p node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let node = syntax.methodish_modifiers in
    (list_contains_predicate p node || p node)
  | _ -> false

(* tests whether the methodish contains > 1 modifier that satisfies [p] *)
let methodish_modifier_multiple_helper p node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let node = syntax.methodish_modifiers in
    list_contains_multiple_predicate p node
  | _ -> false

(* test the methodish node contains the Final keyword *)
let methodish_contains_final node =
  methodish_modifier_contains_helper is_final node

(* test the methodish node contains the Abstract keyword *)
let methodish_contains_abstract node =
  methodish_modifier_contains_helper is_abstract node

(* test the methodish node contains the Static keyword *)
let methodish_contains_static node =
  methodish_modifier_contains_helper is_static node

(* test the methodish node contains the Private keyword *)
let methodish_contains_private node =
  methodish_modifier_contains_helper is_private node

let is_visibility x =
  is_public x || is_private x || is_protected x

(* test the methodish node contains any non-visibility modifiers *)
let methodish_contains_non_visibility node =
  let is_non_visibility x = not (is_visibility x) in
  methodish_modifier_contains_helper is_non_visibility node

(* checks if a methodish decl has multiple visibility modifiers *)
let methodish_multiple_visibility node =
  methodish_modifier_multiple_helper is_visibility node

(* Given a function declaration header, confirm that it is a constructor
 * and that the methodish containing it has a static keyword *)
let class_constructor_has_static node parents =
  let label = node.function_name in
  (is_construct label) && (matches_first methodish_contains_static parents)

(* Given a function declaration header, confirm that it is NOT a constructor
 * and that the header containing it has visibility modifiers in parameters
 *)
let class_non_constructor_has_visibility_param node parents =
  let label = node.function_name in
  let params = node.function_params in
  let has_visibility node =
    match syntax node with
    | ParameterDeclaration node ->
      node.param_visibility |> is_missing |> not
    | _ -> false
  in
  ( not (is_construct label)) &&
  ( list_contains_predicate has_visibility params ||
    has_visibility params)

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-empty parameters *)
let class_destructor_has_param node parents =
  let label = node.function_name in
  let param = node.function_params in
  (is_destruct label) && not (is_missing param)

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-visibility modifiers *)
let class_destructor_has_non_visibility_modifier node parents =
  let label = node.function_name in
  (is_destruct label) &&
  (matches_first methodish_contains_non_visibility parents)

(* check that a constructor or a destructor is type annotated *)
let class_constructor_destructor_has_non_void_type node parents =
  let label = node.function_name in
  let type_ano = node.function_type in
  let function_colon = node.function_colon in
  let is_missing = is_missing type_ano && is_missing function_colon in
  let is_void = match syntax type_ano with
    | SimpleTypeSpecifier spec ->
      is_void spec
    | _ -> false
  in
  (is_construct label || is_destruct label) &&
  not (is_missing || is_void)

(* whether a methodish has duplicate modifiers *)
let methodish_duplicate_modifier node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let modifiers = syntax.methodish_modifiers in
    list_contains_duplicate modifiers
  | _ -> false

(* whether a methodish decl has body *)
let methodish_has_body node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let body = syntax.methodish_function_body in
    not (is_missing body)
  | _ -> false

(* Test whether node is an abstract method with a body.
 * Here node is the methodish node *)
let methodish_abstract_with_body node =
  let is_abstract = methodish_contains_abstract node in
  let has_body = methodish_has_body node in
  is_abstract && has_body

(* Test whether node is a non-abstract method without a body.
 * Here node is the methodish node *)
let methodish_non_abstract_without_body node =
  let non_abstract = not (methodish_contains_abstract node) in
  let not_has_body = not (methodish_has_body node) in
  non_abstract && not_has_body

(* Test whether node is a method that is both abstract and private
 *)
let methodish_abstract_conflict_with_private node =
  let is_abstract = methodish_contains_abstract node in
  let has_private = methodish_contains_private node in
  is_abstract && has_private

(* Test whether node is a method that is both abstract and final
 *)
let methodish_abstract_conflict_with_final node =
  let is_abstract = methodish_contains_abstract node in
  let has_final = methodish_contains_final node in
  is_abstract && has_final

let statement_directly_in_switch parents =
  match parents with
  | l :: c :: s :: _ when (is_compound_statement c) && (is_list l) &&
    (is_switch_statement s) ->
    true
  | c :: s :: _ when (is_compound_statement c) && (is_switch_statement s) ->
    true
  | _ -> false

let first_statement compound =
  match syntax compound with
  | CompoundStatement { compound_statements; _ } ->
    begin
      match syntax compound_statements with
      | Missing -> None (* Empty block *)
      | SyntaxList (first :: _ ) -> Some first
      | _ -> Some compound_statements (* Singleton statement in a block *)
    end
  | _ -> None

let switch_first_is_label compound =
  match first_statement compound with
  | None -> true
  | Some statement ->
    begin
      match syntax statement with
      | DefaultStatement _
      | CaseStatement _ -> true
      | _ -> false
    end

let rec parameter_type_is_required parents =
  match parents with
  | h :: _ when is_function h -> true
  | h :: _ when is_anonymous_function h -> false (* TODO: Lambda? *)
  | _ :: t -> parameter_type_is_required t
  | [] -> false

let rec break_is_legal parents =
  match parents with
  | h :: _ when is_anonymous_function h -> false
  | h :: _ when is_switch_statement h -> true
  | h :: _ when is_loop_statement h -> true
  | _ :: t -> break_is_legal t
  | [] -> false

let rec continue_is_legal parents =
  match parents with
  | h :: _ when is_anonymous_function h -> false
  | h :: _ when is_loop_statement h -> true
  | _ :: t -> continue_is_legal t
  | [] -> false

let is_bad_xhp_attribute_name name =
  (String.contains name ':') || (String.contains name '-')

let xhp_errors node _parents =
(* An attribute name cannot contain - or :, but we allow this in the lexer
   because it's easier to have one rule for tokenizing both attribute and
   element names. *)
  match syntax node with
  |  XHPAttribute attr when
    (is_bad_xhp_attribute_name (PositionedSyntax.text attr.xhp_attr_name)) ->
      let s = start_offset attr.xhp_attr_name in
      let e = end_offset attr.xhp_attr_name in
      [ SyntaxError.make s e SyntaxError.error2002 ]
  | _ -> [ ]

let classish_duplicate_modifiers node =
  list_contains_duplicate node

let type_contains_array_in_strict is_strict node =
  is_array node && is_strict

(* helper since there are so many kinds of errors *)
let produce_error acc check node error error_node =
  if check node then
    let s = start_offset error_node in
    let e = end_offset error_node in
    (SyntaxError.make s e error) :: acc
  else acc

let produce_error_parents acc check node parents error error_node =
  if check node parents then
    let s = start_offset error_node in
    let e = end_offset error_node in
    (SyntaxError.make s e error) :: acc
  else acc

(* use [check] to check properties of function *)
let function_header_check_helper check node parents =
  match syntax node with
  | FunctionDeclarationHeader node -> check node parents
  | _ -> false

let produce_error_for_header acc check node error error_node =
  produce_error_parents acc (function_header_check_helper check) node
    error error_node

let methodish_errors node parents =
  match syntax node with
  (* TODO how to narrow the range of error *)
  | FunctionDeclarationHeader header ->
    let errors = [] in
    let params = header.function_params in
    let type_ano = header.function_type in
    let errors =
      produce_error_for_header errors class_destructor_has_param node parents
      SyntaxError.error2011 params in
    let errors =
      produce_error_for_header errors
      class_constructor_destructor_has_non_void_type
      node parents SyntaxError.error2018 type_ano in
    let errors =
      produce_error_for_header errors class_non_constructor_has_visibility_param
      node parents SyntaxError.error2010 params in
    errors
  | MethodishDeclaration md ->
    let header_node = md.methodish_function_decl_header in
    let modifiers = md.methodish_modifiers in
    let errors = [] in
    let errors =
      produce_error_for_header errors class_constructor_has_static header_node
      [node] SyntaxError.error2009 modifiers in
    let errors =
      produce_error_for_header errors
      class_destructor_has_non_visibility_modifier header_node [node]
      SyntaxError.error2012 modifiers in
    let errors =
      produce_error errors methodish_multiple_visibility node
      SyntaxError.error2017 modifiers in
    let errors =
      produce_error errors methodish_duplicate_modifier node
      SyntaxError.error2013 modifiers in
    let fun_body = md.methodish_function_body in
    let errors =
      produce_error errors methodish_abstract_with_body node
      SyntaxError.error2014 fun_body in
    let fun_semicolon = md.methodish_semicolon in
    let errors =
      produce_error errors methodish_non_abstract_without_body node
      SyntaxError.error2015 fun_semicolon in
    let errors =
      produce_error errors methodish_abstract_conflict_with_private
      node SyntaxError.error2016 modifiers in
    let errors =
      produce_error errors methodish_abstract_conflict_with_final
      node SyntaxError.error2019 modifiers in
    errors
  | _ -> [ ]

let params_errors params =
  let errors = match misplaced_ellipsis params with
  | None -> []
  | Some ellipsis ->
    let s = start_offset ellipsis in
    let e = end_offset ellipsis in
    [ SyntaxError.make s e SyntaxError.error2021 ] in
  let errors =
    if errors = [] then
      match ends_with_ellipsis_comma params with
      | None -> []
      | Some comma ->
        let s = start_offset comma in
        let e = end_offset comma in
        [ SyntaxError.make s e SyntaxError.error2022 ]
    else
      errors
  in
    match misplaced_variadic_param params with
    | None -> errors
    | Some param ->
      let s = start_offset param in
      let e = end_offset param in
      ( SyntaxError.make s e SyntaxError.error2033 ) :: errors

let parameter_errors node parents is_strict =
  match syntax node with
  | ParameterDeclaration p
    when is_strict && (is_missing p.param_type) &&
    (parameter_type_is_required parents) ->
      let s = start_offset node in
      let e = end_offset node in
      [ SyntaxError.make s e SyntaxError.error2001 ]
  | FunctionDeclarationHeader { function_params; _ } ->
    params_errors function_params
  | AnonymousFunction { anonymous_params; _ } ->
    params_errors anonymous_params
  | _ -> []

let function_errors node _parents is_strict =
  match syntax node with
  | FunctionDeclarationHeader f ->
    let label = f.function_name in
    let is_function = not (is_construct label) && not (is_destruct label) in
    if is_strict && is_missing f.function_type && is_function then
      (* Where do we want to report the error? Probably on the right paren. *)
      let rparen = f.function_right_paren in
      let s = start_offset rparen in
      let e = end_offset rparen in
      [ SyntaxError.make s e SyntaxError.error2001 ]
    else
      [ ]
  | _ -> [ ]

let statement_errors node parents =
  let result = match syntax node with
  | CaseStatement _
    when not (statement_directly_in_switch parents) ->
    Some (node, SyntaxError.error2003)
  | DefaultStatement _
    when not (statement_directly_in_switch parents) ->
    Some (node, SyntaxError.error2004)
    (* TODO: Detect when there is more than one default. *)
  | BreakStatement _
    when not (break_is_legal parents) ->
    Some (node, SyntaxError.error2005)
  | ContinueStatement _
    when not (continue_is_legal parents) ->
    Some (node, SyntaxError.error2006)
  | TryStatement { catch_clauses; finally_clause; _ }
    when (is_missing catch_clauses) && (is_missing finally_clause) ->
    Some (node, SyntaxError.error2007)
  | SwitchStatement { switch_compound_statement; _ }
    when not (switch_first_is_label switch_compound_statement) ->
    Some (switch_compound_statement, SyntaxError.error2008)
  | _ -> None in
  match result with
  | None -> [ ]
  | Some (error_node, error_message) ->
    let s = start_offset error_node in
    let e = end_offset error_node in
    [ SyntaxError.make s e error_message ]

let property_errors node is_strict =
  match syntax node with
  | PropertyDeclaration p when is_strict && is_missing (p.prop_type) ->
      let s = start_offset node in
      let e = end_offset node in
      [ SyntaxError.make s e SyntaxError.error2001 ]
  | _ -> [ ]

let expression_errors node =
  match syntax node with
  | SubscriptExpression s when is_left_brace s.subscript_left ->
    let s = start_offset node in
    let e = end_offset node in
    [ SyntaxError.make s e SyntaxError.error2020 ]
  | FunctionCallExpression s ->
    begin match misplaced_variadic_arg s.function_call_arguments with
      | Some h ->
        let s = start_offset h in
        let e = end_offset h in
        [ SyntaxError.make s e SyntaxError.error2033 ]
      | None -> [ ]
    end
  | _ -> []

let require_errors node parents =
  match syntax node with
  | RequireClause p ->
    begin
      match (containing_classish_kind parents, token_kind p.require_kind) with
      | (Some TokenKind.Class, Some TokenKind.Extends) ->
        let s = start_offset node in
        let e = end_offset node in
        [ SyntaxError.make s e SyntaxError.error2029 ]
      | (Some TokenKind.Interface, Some TokenKind.Implements)
      | (Some TokenKind.Class, Some TokenKind.Implements) ->
        let s = start_offset node in
        let e = end_offset node in
        [ SyntaxError.make s e SyntaxError.error2030 ]
      | _ -> []
    end
  | _ -> [ ]

let classish_errors node parents =
  match syntax node with
  | ClassishDeclaration c ->
    begin
      let modifiers = c.classish_modifiers in
      let acc = [] in
      produce_error acc classish_duplicate_modifiers modifiers
      SyntaxError.error2031 modifiers
    end
  | _ -> [ ]

let type_errors node parents is_strict =
  match syntax node with
  | SimpleTypeSpecifier t ->
    let acc = [ ] in
    produce_error acc (type_contains_array_in_strict is_strict) t
    SyntaxError.error2032 t
  | _ -> [ ]

let find_syntax_errors node is_strict =
  let folder acc node parents =
    let param_errs = parameter_errors node parents is_strict in
    let func_errs = function_errors node parents is_strict in
    let xhp_errs = xhp_errors node parents in
    let statement_errs = statement_errors node parents in
    let methodish_errs = methodish_errors node parents in
    let property_errs = property_errors node is_strict in
    let expr_errs = expression_errors node in
    let require_errs = require_errors node parents in
    let classish_errors = classish_errors node parents in
    let type_errors = type_errors node parents is_strict in
    let errors = acc.errors @ param_errs @ func_errs @
      xhp_errs @ statement_errs @ methodish_errs @ property_errs @
      expr_errs @ require_errs @ classish_errors @ type_errors in
    { errors } in
  let acc = SyntaxUtilities.parented_fold_pre folder { errors = [] } node in
  List.sort SyntaxError.compare acc.errors

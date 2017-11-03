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
open PositionedSyntax

module PositionedToken = Full_fidelity_positioned_token
module SyntaxError = Full_fidelity_syntax_error
module TokenKind = Full_fidelity_token_kind

module SN = Naming_special_names

type location = {
  start_offset: int;
  end_offset: int
}

let make_location s e =
  { start_offset = start_offset s; end_offset = end_offset e }

let make_location_of_node n =
  make_location n n

type namespace_type =
  | Unspecified
  | Bracketed of location
  | Unbracketed of location

type used_names = {
  t_classes: (location * bool) SMap.t;
  t_namespaces: (location * bool) SMap.t;
  t_functions: (location * bool) SMap.t;
  t_constants: (location * bool) SMap.t;
}

let empty_names = {
  t_classes = SMap.empty;
  t_namespaces = SMap.empty;
  t_functions = SMap.empty;
  t_constants = SMap.empty;
}

let empty_trait_require_clauses = SMap.empty

let get_short_name_from_qualified_name name alias =
  if String.length alias <> 0 then alias
  else
  try
    let i = String.rindex name '\\' in
    String.sub name (i + 1) (String.length name - i - 1)
  with Not_found -> name

type accumulator = {
  errors : SyntaxError.t list;
  namespace_type : namespace_type;
  has_namespace_prefix: bool;
  names: used_names;
  trait_require_clauses: TokenKind.t SMap.t;
}

let make_acc
  acc errors namespace_type names has_namespace_prefix trait_require_clauses =
  if acc.errors == errors &&
     acc.namespace_type == namespace_type &&
     acc.names == names &&
     acc.has_namespace_prefix = has_namespace_prefix &&
     acc.trait_require_clauses = trait_require_clauses
  then acc
  else { errors
       ; namespace_type
       ; names
       ; has_namespace_prefix
       ; trait_require_clauses
       }

let fold_child_nodes f node parents acc =
  PositionedSyntax.children node
  |> Core_list.fold_left ~init:acc ~f:(fun acc c -> f acc c (node :: parents))

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

let is_decorated_expression ~f node =
  begin match syntax node with
    | DecoratedExpression { decorated_expression_decorator; _ } ->
      f decorated_expression_decorator
    | _ -> false
  end

let test_decorated_expression_child ~f node =
  begin match syntax node with
    | DecoratedExpression { decorated_expression_expression; _ } ->
      f decorated_expression_expression
    | _ -> false
  end

let is_reference_expression node =
  is_decorated_expression ~f:is_ampersand node

let is_variadic_expression node =
  is_decorated_expression ~f:is_ellipsis node

let is_reference_variadic node =
  is_variadic_expression node &&
  test_decorated_expression_child node ~f:is_reference_expression

let is_double_variadic node =
  is_variadic_expression node &&
  test_decorated_expression_child node ~f:is_variadic_expression

let is_double_reference node =
  is_reference_expression node &&
  test_decorated_expression_child node ~f:is_reference_expression

let is_variadic_parameter_variable node =
  (* TODO: This shouldn't be a decorated *expression* because we are not
  expecting an expression at all. We're expecting a declaration. *)
  is_variadic_expression node

let is_variadic_parameter_declaration node =
  begin match syntax node with
  | VariadicParameter _ -> true
  | ParameterDeclaration { parameter_name; _ } ->
      is_variadic_parameter_variable parameter_name
  | _ -> false
  end

let misplaced_variadic_param params =
  assert_last_in_list is_variadic_parameter_declaration params

let misplaced_variadic_arg args =
  assert_last_in_list is_variadic_expression args

(* If a list ends with a variadic parameter followed by a comma, return it *)
let ends_with_variadic_comma params =
  let rec aux params =
    match params with
    | [] -> None
    | x :: y :: [] when is_variadic_parameter_declaration x && is_comma y ->
      Some y
    | _ :: t -> aux t in
  aux (syntax_to_list_with_separators params)

(* Extract variadic parameter from a parameter list *)
let variadic_param params =
  let rec aux params =
    match params with
    | [] -> None
    | x :: _ when is_variadic_parameter_declaration x -> Some x
    | _ :: t -> aux t in
  aux (syntax_to_list_with_separators params)

let is_parameter_with_default_value param =
  match syntax param with
  | ParameterDeclaration { parameter_default_value; _ } ->
    not (is_missing parameter_default_value)
  | _ -> false

let param_missing_default_value params =
  (* TODO: This error is also reported in the type checker; when we switch
  over to the FFP, we can remove the error detection from the type checker. *)
  let rec aux seen_default params =
    match params with
    | [] -> None
    | x :: t ->
      if is_variadic_parameter_declaration x then
        None (* Stop looking. If this happens to not be the last parameter,
          we'll give an error in a different check. *)
      else
        let has_default = is_parameter_with_default_value x in
        if seen_default && not has_default then
          Some x (* We saw a defaulted parameter, and this one has no
            default value. Give an error, and stop looking for more. *)
        else
          aux has_default t in
  aux false (syntax_to_list_no_separators params)

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

(* Helper function for common code pattern *)
let is_token_kind node kind =
  (token_kind node) = Some kind

let rec containing_classish_kind parents =
  match parents with
  | [] -> None
  | h :: t ->
    begin
      match syntax h with
      | ClassishDeclaration c -> token_kind c.classish_keyword
      | _ -> containing_classish_kind t
    end

(* tests whether the methodish contains a modifier that satisfies [p] *)
let methodish_modifier_contains_helper p node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let node = syntax.methodish_modifiers in
    list_contains_predicate p node
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
let methodish_contains_non_visibility hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
  let is_non_visibility x = not (is_visibility x) in
  methodish_modifier_contains_helper is_non_visibility node

(* checks if a methodish decl has multiple visibility modifiers *)
let methodish_multiple_visibility hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
  methodish_modifier_multiple_helper is_visibility node

(* Given a function declaration header, confirm that it is a constructor
 * and that the methodish containing it has a static keyword *)
let class_constructor_has_static hhvm_compat_mode node parents =
  if hhvm_compat_mode then false
  else
  let label = node.function_name in
  (is_construct label) && (matches_first methodish_contains_static parents)

(* Given a function declaration header, confirm that it is NOT a constructor
 * and that the header containing it has visibility modifiers in parameters
 *)
let class_non_constructor_has_visibility_param node parents =
  let has_visibility node =
    match syntax node with
    | ParameterDeclaration { parameter_visibility; _ } ->
      parameter_visibility |> is_missing |> not
    | _ -> false
  in
  let label = node.function_name in
  let params = syntax_to_list_no_separators node.function_parameter_list in
  (not (is_construct label)) && (List.exists has_visibility params)

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-empty parameters *)
let class_destructor_has_param hhvm_compat_mode node parents =
  let label = node.function_name in
  let param = node.function_parameter_list in
  not hhvm_compat_mode && (is_destruct label) && not (is_missing param)

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-visibility modifiers *)
let class_destructor_has_non_visibility_modifier hhvm_compat_mode node parents =
  let label = node.function_name in
  not hhvm_compat_mode &&
  (is_destruct label) &&
  (matches_first (methodish_contains_non_visibility hhvm_compat_mode) parents)

(* check that a constructor or a destructor is type annotated *)
let class_constructor_destructor_has_non_void_type hhvm_compat_node node parents =
  if hhvm_compat_node then false
  else
  let label = node.function_name in
  let type_ano = node.function_type in
  let function_colon = node.function_colon in
  let is_missing = is_missing type_ano && is_missing function_colon in
  let is_void = match syntax type_ano with
    | SimpleTypeSpecifier spec ->
      is_void spec.simple_type_specifier
    | _ -> false
  in
  (is_construct label || is_destruct label) &&
  not (is_missing || is_void)

(* whether a methodish has duplicate modifiers *)
let methodish_duplicate_modifier hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
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

(* By checking the third parent of a methodish node, tests whether the methodish
 * node is inside an interface. *)
let methodish_inside_interface parents =
  match parents with
  | _ :: _ :: p3 :: _ ->
    begin match syntax p3 with
    | ClassishDeclaration { classish_keyword; _ } ->
      is_token_kind classish_keyword TokenKind.Interface
    | _ -> false
    end
  | _ -> false

(* Test whether node is an abstract method with a body.
 * Here node is the methodish node *)
let methodish_abstract_with_body hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
  let is_abstract = methodish_contains_abstract node in
  let has_body = methodish_has_body node in
  is_abstract && has_body

(* Test whether node is a non-abstract method without a body.
 * Here node is the methodish node
 * And methods inside interfaces are inherently considered abstract *)
let methodish_non_abstract_without_body hhvm_compat_mode node parents =
  if hhvm_compat_mode then false
  else
  let non_abstract = not (methodish_contains_abstract node
      || methodish_inside_interface parents) in
  let not_has_body = not (methodish_has_body node) in
  non_abstract && not_has_body

let methodish_in_interface_has_body hhvm_compat_mode node parents =
  not hhvm_compat_mode &&
  methodish_inside_interface parents && methodish_has_body node

(* Test whether node is a method that is both abstract and private
 *)
let methodish_abstract_conflict_with_private hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
  let is_abstract = methodish_contains_abstract node in
  let has_private = methodish_contains_private node in
  is_abstract && has_private

(* Test whether node is a method that is both abstract and final
 *)
let methodish_abstract_conflict_with_final hhvm_compat_mode node =
  if hhvm_compat_mode then false
  else
  let is_abstract = methodish_contains_abstract node in
  let has_final = methodish_contains_final node in
  is_abstract && has_final

let rec parameter_type_is_required parents =
  match parents with
  | h :: _ when is_function_declaration h -> true
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

let using_statement_function_scoped_is_legal parents =
  match parents with
  | _ :: c :: h :: _
    when is_compound_statement c &&
        (is_function_declaration h || is_methodish_declaration h) -> true
  | _ -> false

let is_bad_xhp_attribute_name name =
  (String.contains name ':') || (String.contains name '-')

let make_error_from_nodes
  ?(error_type=SyntaxError.ParseError) start_node end_node error =
  let s = start_offset start_node in
  let e = end_offset end_node in
  SyntaxError.make ~error_type s e error

let make_error_from_node ?(error_type=SyntaxError.ParseError) node error =
  make_error_from_nodes ~error_type node node error

let is_invalid_xhp_attr_enum_item_literal literal_expression =
  match syntax literal_expression with
  | Token t -> begin
      Full_fidelity_token_kind.(match PositionedToken.kind t with
      | DecimalLiteral | SingleQuotedStringLiteral
      | DoubleQuotedStringLiteral -> false
      | _ -> true)
    end
  | _ -> true

let is_invalid_xhp_attr_enum_item node =
  match syntax node with
  | LiteralExpression {literal_expression} ->
      is_invalid_xhp_attr_enum_item_literal literal_expression
  | _ -> true

let xhp_errors node _parents hhvm_compat_mode errors =
(* An attribute name cannot contain - or :, but we allow this in the lexer
   because it's easier to have one rule for tokenizing both attribute and
   element names. *)
  match syntax node with
  |  XHPAttribute attr when
    not hhvm_compat_mode &&
    (is_bad_xhp_attribute_name
    (PositionedSyntax.text attr.xhp_attribute_name)) ->
      make_error_from_node attr.xhp_attribute_name SyntaxError.error2002 :: errors
  |  XHPEnumType enumType when
    not hhvm_compat_mode &&
    (is_missing enumType.xhp_enum_values) ->
      make_error_from_node enumType.xhp_enum_values SyntaxError.error2055 :: errors
  |  XHPEnumType enumType when
    not hhvm_compat_mode ->
    let invalid_enum_items = List.filter is_invalid_xhp_attr_enum_item
      (syntax_to_list_no_separators enumType.xhp_enum_values) in
    let mapper errors item =
      make_error_from_node item SyntaxError.error2063 :: errors in
    List.fold_left mapper errors invalid_enum_items
  |  XHPExpression
    { xhp_open =
      { syntax = XHPOpen { xhp_open_name; _ }; _ }
    ; xhp_close =
      { syntax = XHPClose { xhp_close_name; _ }; _ }
    ; _ } when text xhp_open_name <> text xhp_close_name ->
    make_error_from_node node (SyntaxError.error2070
      ~open_tag:(text xhp_open_name)
      ~close_tag:(text xhp_close_name)) :: errors
  | _ -> errors

let classish_duplicate_modifiers hhvm_compat_mode node =
  not hhvm_compat_mode && list_contains_duplicate node

let type_contains_array_in_strict is_strict hhvm_compat_mode node =
  not hhvm_compat_mode && is_array node && is_strict

(* helper since there are so many kinds of errors *)
let produce_error acc check node error error_node =
  if check node then
    (make_error_from_node error_node error) :: acc
  else acc

let produce_error_from_check acc check node error =
  match check node with
  | Some error_node ->
    (make_error_from_node error_node error) :: acc
  | _ -> acc

let produce_error_parents acc check node parents error error_node =
  if check node parents then
    (make_error_from_node error_node error) :: acc
  else acc

(* use [check] to check properties of function *)
let function_header_check_helper check node parents =
  match syntax node with
  | FunctionDeclarationHeader node -> check node parents
  | _ -> false

let produce_error_for_header acc check node error error_node =
  produce_error_parents acc (function_header_check_helper check) node
    error error_node

(* Given a ClassishDeclaration node, test whether or not it contains
 * an invalid use of 'implements'. *)
let classish_invalid_implements_keyword hhvm_compat_mode cd_node =
  (* Invalid if uses 'implements' and isn't a class. *)
  not hhvm_compat_mode &&
  token_kind cd_node.classish_implements_keyword = Some TokenKind.Implements &&
    token_kind cd_node.classish_keyword <> Some TokenKind.Class

(* Given a ClassishDeclaration node, test whether or not it's a trait
 * invoking the 'extends' keyword. *)
let classish_invalid_extends_keyword hhvm_compat_mode cd_node =
  (* Invalid if uses 'extends' and is a trait. *)
  not hhvm_compat_mode &&
  token_kind cd_node.classish_extends_keyword = Some TokenKind.Extends &&
    token_kind cd_node.classish_keyword = Some TokenKind.Trait

(* Given a ClassishDeclaration node, test whether or not length of
 * extends_list is appropriate for the classish_keyword. *)
let classish_invalid_extends_list  hhvm_compat_mode cd_node =
  (* Invalid if is a class and has list of length greater than one. *)
  not hhvm_compat_mode &&
  token_kind cd_node.classish_keyword = Some TokenKind.Class &&
    token_kind cd_node.classish_extends_keyword = Some TokenKind.Extends &&
    match syntax_to_list_no_separators cd_node.classish_extends_list with
    | [x1] -> false
    | _ -> true (* General bc empty list case is already caught by error1007 *)

(* Given a particular TokenKind.(Trait/Interface/Class), tests if a given
 * classish_declaration node is both of that kind and declared abstract. *)
let is_classish_kind_declared_abstract hhvm_compat_mode classish_kind cd_node =
  if hhvm_compat_mode then false
  else
  match syntax cd_node with
  | ClassishDeclaration { classish_keyword; classish_modifiers; _ }
    when is_token_kind classish_keyword classish_kind ->
      list_contains_predicate is_abstract classish_modifiers
  | _ -> false

(* Given a function_declaration_header node, returns its function_name
 * as a string opt. *)
let extract_function_name header_node =
  (* The '_' arm of this match will never be reached, but the type checker
   * doesn't allow a direct extraction of function_name from
   * function_declaration_header. *)
   match syntax header_node with
   | FunctionDeclarationHeader fdh ->
     Some (PositionedSyntax.text fdh.function_name)
   | _ -> None

(* Return, as a string opt, the name of the function with the earliest
 * declaration node in the list of parents. *)
let first_parent_function_name parents =
  Hh_core.List.find_map parents ~f:begin fun node ->
    match syntax node with
    | FunctionDeclaration {function_declaration_header = header; _ }
    | MethodishDeclaration {methodish_function_decl_header = header; _ } ->
      extract_function_name header
    | _ -> None
  end

(* Returns the first ClassishDeclaration node in the list of parents,
 * or None if there isn't one. *)
let first_parent_classish_node classish_kind parents =
  Hh_core.List.find_map parents ~f:begin fun node ->
  match syntax node with
  | ClassishDeclaration cd
    when is_token_kind cd.classish_keyword classish_kind -> Some node
  | _ -> None
  end

(* Return, as a string opt, the name of the earliest Class in the list of
 * parents. *)
let first_parent_class_name parents =
  let parent_class_decl = first_parent_classish_node TokenKind.Class parents in
  Option.value_map parent_class_decl ~default:None ~f:begin fun node ->
    match syntax node with
    | ClassishDeclaration cd -> Some (PositionedSyntax.text cd.classish_name)
    | _ -> None (* This arm is never reached  *)
  end

(* Given a classish_ or methodish_ declaration node, returns the modifier node
   from its list of modifiers, or None if there isn't one. *)
let extract_keyword modifier declaration_node =
  match syntax declaration_node with
  | ClassishDeclaration { classish_modifiers = modifiers_list ; _ }
  | MethodishDeclaration { methodish_modifiers = modifiers_list ; _ } ->
    Hh_core.List.find ~f:modifier
        (syntax_to_list_no_separators modifiers_list)
  | _ -> None

(* Wrapper function that uses above extract_keyword function to test if node
   contains is_abstract keyword *)
let is_abstract_declaration declaration_node =
  not (Option.is_none (extract_keyword is_abstract declaration_node))

(* Wrapper function that uses above extract_keyword function to test if node
   contains is_final keyword *)
let is_final_declaration declaration_node =
  not (Option.is_none (extract_keyword is_final declaration_node))

(* Given a methodish_declaration node and a list of parents, tests if that
 * node declares an abstract method inside of a no-nabstract class. *)
let abstract_method_in_nonabstract_class hhvm_compat_mode  md_node parents =
  if hhvm_compat_mode then false
  else
  let is_abstract_method = is_abstract_declaration md_node in
  let parent_class_node = first_parent_classish_node TokenKind.Class parents in
  let is_in_nonabstract_class =
    match parent_class_node with
    | None -> false
    | Some node -> not (is_abstract_declaration node) in
  is_abstract_method && is_in_nonabstract_class

(* Given a list of parents, tests if the immediate classish parent is an
 * interface. *)
let is_inside_interface parents =
  Option.is_some (first_parent_classish_node TokenKind.Interface parents)

(* Given a list of parents, tests if the immediate classish parent is a
 * trait. *)
let is_inside_trait parents =
  Option.is_some (first_parent_classish_node TokenKind.Trait parents)

(* Given a methodish_declaration node and a list of parents, tests if that
 * node declares an abstract method inside of an interface. *)
let abstract_method_in_interface hhvm_compat_mode  md_node parents =
  not hhvm_compat_mode &&
  is_abstract_declaration md_node && is_inside_interface parents

(* Tests if md_node is either explicitly declared abstract or is
 * defined inside an interface *)
let is_generalized_abstract_method md_node parents =
  is_abstract_declaration md_node || is_inside_interface parents

(* Returns the 'async'-annotation syntax node from the methodish_declaration
 * node. The returned node may have syntax kind 'Missing', but it will only be
 * None if something other than a methodish_declaration node was provided as
 * input. *)
let extract_async_node md_node =
  match syntax md_node with
  | MethodishDeclaration { methodish_function_decl_header; _ } -> begin
    match syntax methodish_function_decl_header with
    | FunctionDeclarationHeader { function_async ; _ } -> Some function_async
    | _ ->
      failwith("Encountered an improperly defined methodish_function_decl_" ^
      "header: expected it to match FunctionDeclarationHeader with field " ^
      "function_async.")
    end
  | _ -> None (* Only method declarations have async nodes *)

let make_name_already_used_error node name short_name original_location
  report_error =
  let original_location_error =
    SyntaxError.make
      original_location.start_offset
      original_location.end_offset
      SyntaxError.original_definition in
  let s = start_offset node in
  let e = end_offset node in
  SyntaxError.make
    ~child:(Some original_location_error) s e (report_error ~name ~short_name)

(* Given a node and its parents, tests if the node declares a method that is
 * both abstract and async. *)
let is_abstract_and_async_method md_node parents =
  let async_node = extract_async_node md_node in
  match async_node with
  | None -> false
  | Some async_node ->
    is_generalized_abstract_method md_node parents
        && not (is_missing async_node)

(* Returns the visibility modifier node from a list, or None if the
 * list doesn't contain one. *)
let extract_visibility_node modifiers_list =
  Hh_core.List.find ~f:is_visibility (syntax_to_list_no_separators
    modifiers_list)

let extract_callconv_node node =
  match syntax node with
  | ParameterDeclaration { parameter_call_convention; _ } ->
    Some parameter_call_convention
  | _ -> None

(* Tests if visibility modifiers of the node are allowed on
 * methods inside an interface. *)
let has_valid_interface_visibility node =
  match syntax node with
  | MethodishDeclaration { methodish_modifiers; _ } ->
    let visibility_kind = extract_visibility_node methodish_modifiers in
    let is_valid_methodish_visibility kind =
      (is_token_kind kind TokenKind.Public) in
    (* Defaulting to 'true' allows omitting visibility in method_declarations *)
    Option.value_map visibility_kind
      ~f:is_valid_methodish_visibility ~default:true
  | _ -> true (* If not a methodish declaration, is vacuously valid *)

(* Tests if a given node is a method definition (inside an interface) with
 * either private or protected visibility. *)
let invalid_methodish_visibility_inside_interface hhvm_compat_mode node parents =
  not hhvm_compat_mode &&
  is_inside_interface parents && not (has_valid_interface_visibility node)

(* Test if (a list_expression is the left child of a binary_expression,
 * and the operator is '=') *)
let is_left_of_simple_assignment le_node p1 =
  match syntax p1 with
  | BinaryExpression { binary_left_operand; binary_operator; _ } ->
    le_node == binary_left_operand  &&
        is_token_kind binary_operator TokenKind.Equal
  | _ -> false

(* Test if a list_expression is the value clause of a foreach_statement *)
let is_value_of_foreach le_node p1 =
  match syntax p1 with
  | ForeachStatement { foreach_value; _ } -> le_node == foreach_value
  | _ -> false

let is_invalid_list_expression le_node parents =
  match parents with
  | p1 :: _ when is_left_of_simple_assignment le_node p1 -> false
  | p1 :: _ when is_value_of_foreach le_node p1 -> false
  (* checking p3 is sufficient to test if le_node is a nested list_expression *)
  | _ :: _ :: p3 :: _ when is_list_expression p3 -> false
  | _ -> true (* All other deployments of list_expression are invalid *)

(* Given a node, checks if it is a concrete ConstDeclaration *)
let is_concrete_const declaration =
  match syntax declaration with
  | ConstDeclaration x -> is_missing x.const_abstract
  | _ -> false

(* Given a node, checks if it is a abstract ConstDeclaration *)
let is_abstract_const declaration =
  match syntax declaration with
  | ConstDeclaration x -> not (is_missing x.const_abstract)
  | _ -> false

(* Given a ConstDeclarator node, test whether it is concrete, but has no
   initializer. *)
let concrete_no_initializer hhvm_compat_mode cd_node parents =
  if hhvm_compat_mode then false
  else
  let is_concrete =
    match parents with
    | _ :: _ :: p3 :: _ when is_concrete_const p3 -> true
    | _ -> false
    in
  let has_no_initializer =
    is_missing cd_node.constant_declarator_initializer in
  is_concrete && has_no_initializer

(* Given a ConstDeclarator node, test whether it is abstract, but has an
   initializer. *)
let abstract_with_initializer cd_node parents =
  let is_abstract =
    match parents with
    | _ :: _ :: p3 :: _ when is_abstract_const p3 -> true
    | _ -> false
    in
  let has_initializer =
    not (is_missing cd_node.constant_declarator_initializer) in
  is_abstract && has_initializer

(* Tests if Property contains a modifier p *)
let property_modifier_contains_helper p node =
  match syntax node with
  | PropertyDeclaration syntax ->
    let node = syntax.property_modifiers in
    list_contains_predicate p node
  | _ -> false

(* Tests if parent class is both abstract and final *)
let abstract_final_parent_class parents =
  let parent = first_parent_classish_node TokenKind.Class parents in
    match parent with
    | None -> false
    | Some node -> (is_abstract_declaration node) && (is_final_declaration node)

(* Given a PropertyDeclaration node, tests whether parent class is abstract
  final but child variable is non-static *)
let abstract_final_with_inst_var hhvm_compat_mode cd parents =
  not hhvm_compat_mode &&
  (abstract_final_parent_class parents) &&
    not (property_modifier_contains_helper is_static cd)

(* Given a MethodishDeclaration, tests whether parent class is abstract final
    but child method is non-static *)
let abstract_final_with_method hhvm_compat_mode cd parents =
  not hhvm_compat_mode &&
  (abstract_final_parent_class parents) && not (methodish_contains_static cd)

let methodish_errors node parents is_hack hhvm_compat_mode errors =
  match syntax node with
  (* TODO how to narrow the range of error *)
  | FunctionDeclarationHeader { function_parameter_list; function_type; _} ->
    let errors =
      produce_error_for_header errors
      (class_destructor_has_param hhvm_compat_mode) node parents
      SyntaxError.error2011 function_parameter_list in
    let errors =
      produce_error_for_header errors
      (class_constructor_destructor_has_non_void_type hhvm_compat_mode)
      node parents SyntaxError.error2018 function_type in
    let errors =
      produce_error_for_header errors class_non_constructor_has_visibility_param
      node parents SyntaxError.error2010 function_parameter_list in
    errors
  | MethodishDeclaration md ->
    let header_node = md.methodish_function_decl_header in
    let modifiers = md.methodish_modifiers in
    let errors =
      produce_error_for_header errors
      (class_constructor_has_static hhvm_compat_mode) header_node
      [node] SyntaxError.error2009 modifiers in
    let errors =
      let missing_modifier is_hack hhvm_compat_mode modifiers =
        not hhvm_compat_mode && is_hack && is_missing modifiers in
      produce_error errors (missing_modifier is_hack hhvm_compat_mode) modifiers
      SyntaxError.error2054 header_node in
    let errors =
      produce_error_for_header errors
      (class_destructor_has_non_visibility_modifier hhvm_compat_mode)
      header_node [node]
      SyntaxError.error2012 modifiers in
    let errors =
      produce_error errors (methodish_multiple_visibility hhvm_compat_mode) node
      SyntaxError.error2017 modifiers in
    let errors =
      produce_error errors (methodish_duplicate_modifier hhvm_compat_mode) node
      SyntaxError.error2013 modifiers in
    let fun_body = md.methodish_function_body in
    let errors =
      produce_error errors (methodish_abstract_with_body hhvm_compat_mode) node
      SyntaxError.error2014 fun_body in
    let fun_semicolon = md.methodish_semicolon in
    let errors =
      produce_error errors
      (methodish_non_abstract_without_body hhvm_compat_mode node) parents
      SyntaxError.error2015 fun_semicolon in
    let errors =
      produce_error errors
      (methodish_abstract_conflict_with_private hhvm_compat_mode)
      node SyntaxError.error2016 modifiers in
    let errors =
      produce_error errors
      (methodish_abstract_conflict_with_final hhvm_compat_mode)
      node SyntaxError.error2019 modifiers in
    let errors =
      produce_error errors
      (methodish_in_interface_has_body hhvm_compat_mode node) parents
      SyntaxError.error2041 md.methodish_function_body in
    let errors =
      let class_name = Option.value (first_parent_class_name parents)
        ~default:"" in
      let method_name = Option.value (extract_function_name
        md.methodish_function_decl_header) ~default:"" in
      (* default will never be used, since existence of abstract_keyword is a
       * necessary condition for the production of an error. *)
      let abstract_keyword = Option.value (extract_keyword is_abstract node)
        ~default:node in
      produce_error errors
      (abstract_method_in_nonabstract_class hhvm_compat_mode node) parents
      (SyntaxError.error2044 class_name method_name) abstract_keyword in
    let errors =
      let abstract_keyword = Option.value (extract_keyword is_abstract node)
        ~default:node in
      produce_error errors
      (abstract_method_in_interface hhvm_compat_mode node) parents
      SyntaxError.error2045 abstract_keyword in
    let errors =
      let async_annotation = Option.value (extract_async_node node)
        ~default:node in
      produce_error errors
      (is_abstract_and_async_method node) parents
      SyntaxError.error2046 async_annotation in
    let errors =
      let visibility_node = Option.value (extract_visibility_node modifiers)
        ~default:node in
      let visibility_text = PositionedSyntax.text visibility_node in (* is this option? *)
      produce_error errors
      (invalid_methodish_visibility_inside_interface hhvm_compat_mode node)
      parents (SyntaxError.error2047 visibility_text) visibility_node in
    errors
  | _ -> errors

let is_hashbang text =
  let text = PositionedSyntax.text text in
  let r = Str.regexp "^#!.*\n" in
  Str.string_match r text 0 && Str.matched_string text = text

let markup_errors node is_hack_file hhvm_compat_mode errors =
  match syntax node with
  | MarkupSection { markup_prefix; markup_text; _ }
    (* only report the error on the first markup section of a hack file *)
    when is_hack_file && (is_missing markup_prefix) &&
    (* hashbang is allowed before <?hh *)
    (width markup_text) > 0 && not (is_hashbang markup_text) ->
    make_error_from_node node SyntaxError.error1001 :: errors
  | MarkupSection { markup_prefix; markup_text; _ }
    when is_hack_file && (token_kind markup_prefix) = Some TokenKind.QuestionGreaterThan ->
    make_error_from_node node SyntaxError.error2067 :: errors
  | MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name; }
    when not hhvm_compat_mode && not is_hack_file
    && ((token_kind markup_suffix_less_than_question) = Some TokenKind.LessThanQuestion)
    && ((PositionedSyntax.text markup_suffix_name) <> "php") ->
    make_error_from_node node SyntaxError.error2068 :: errors
  | _ -> errors

let default_value_params is_hack hhvm_compat_mode params =
  match param_missing_default_value params with
  | Some param when not hhvm_compat_mode && is_hack -> Some param
  | _ -> None

(* Test if the parameter is missing a type annotation but one is required *)
let missing_param_type_check is_strict hhvm_compat_mode p parents =
  let is_required = parameter_type_is_required parents in
  not hhvm_compat_mode && is_strict && is_missing p.parameter_type && is_required

(* If a variadic parameter has a default value, return it *)
let variadic_param_with_default_value params =
  Option.filter (variadic_param params) ~f:is_parameter_with_default_value

let is_parameter_with_callconv param =
  match syntax param with
  | ParameterDeclaration { parameter_call_convention; _ } ->
    not (is_missing parameter_call_convention)
  | _ -> false

(* If a variadic parameter is marked inout, return it *)
let variadic_param_with_callconv params =
  Option.filter (variadic_param params) ~f:is_parameter_with_callconv

(* If an inout parameter has a default, return the default *)
let param_with_callconv_has_default node =
  match syntax node with
  | ParameterDeclaration { parameter_default_value; _ } when
    is_parameter_with_callconv node &&
    is_parameter_with_default_value node -> Some parameter_default_value
  | _ -> None

let is_byref_expression node =
  is_decorated_expression ~f:is_ampersand node

let is_byref_parameter_variable node =
  (* TODO: This shouldn't be a decorated *expression* because we are not
  expecting an expression at all. We're expecting a declaration. *)
  is_byref_expression node

(* If an inout parameter is passed by reference, return it *)
let param_with_callconv_is_byref node =
  match syntax node with
  | ParameterDeclaration { parameter_name; _ } when
    is_parameter_with_callconv node &&
    is_byref_parameter_variable parameter_name -> Some node
  | _ -> None
let params_errors params is_hack hhvm_compat_mode errors =
  let errors =
    produce_error_from_check errors (default_value_params is_hack hhvm_compat_mode)
    params SyntaxError.error2066 in
  let errors =
    produce_error_from_check errors ends_with_variadic_comma
    params SyntaxError.error2022 in
  let errors =
    produce_error_from_check errors misplaced_variadic_param
    params SyntaxError.error2021 in
  let errors =
    produce_error_from_check errors variadic_param_with_default_value
    params SyntaxError.error2065 in
  let errors =
    produce_error_from_check errors variadic_param_with_callconv
    params SyntaxError.error2073 in
  errors

let decoration_errors node errors =
  let errors = produce_error errors is_reference_variadic node SyntaxError.variadic_reference node in
  let errors = produce_error errors is_double_variadic node SyntaxError.double_variadic node in
  let errors = produce_error errors is_double_reference node SyntaxError.double_reference node in
  errors

let parameter_errors node parents is_strict is_hack hhvm_compat_mode errors =
  match syntax node with
  | ParameterDeclaration p ->
    let errors =
      produce_error_parents errors (missing_param_type_check is_strict hhvm_compat_mode)
      p parents SyntaxError.error2001 node in
    let callconv_text = Option.value (extract_callconv_node node) ~default:node
      |> PositionedSyntax.text in
    let errors =
      produce_error_from_check errors param_with_callconv_has_default
      node (SyntaxError.error2074 callconv_text) in
    let errors =
      produce_error_from_check errors param_with_callconv_is_byref
      node (SyntaxError.error2075 callconv_text) in
    errors
  | FunctionDeclarationHeader { function_parameter_list; _ }
    when not hhvm_compat_mode ->
    params_errors function_parameter_list is_hack hhvm_compat_mode errors
  | AnonymousFunction { anonymous_parameters; _ }
    when not hhvm_compat_mode ->
    params_errors anonymous_parameters is_hack hhvm_compat_mode errors
  | DecoratedExpression _ -> decoration_errors node errors
  | _ -> errors

let missing_type_annot_check is_strict hhvm_compat_mode f =
  let label = f.function_name in
  let is_function = not (is_construct label) && not (is_destruct label) in
  not hhvm_compat_mode && is_strict && is_missing f.function_type && is_function

let function_reference_check is_strict hhvm_compat_mode f =
  not hhvm_compat_mode && is_strict && not (is_missing f.function_ampersand)

let function_errors node parents is_strict hhvm_compat_mode names errors =
  match syntax node with
  | FunctionDeclarationHeader f ->
    let errors =
      produce_error errors (missing_type_annot_check is_strict hhvm_compat_mode) f
      SyntaxError.error2001 f.function_right_paren in
    let errors =
      produce_error errors (function_reference_check is_strict hhvm_compat_mode) f
      SyntaxError.error2064 f.function_ampersand in
    let names =
      match parents with
      | { syntax = FunctionDeclaration _; _ } :: _ ->
        let function_name = text f.function_name in
        let location = make_location_of_node f.function_name in
        { names with
          t_functions = SMap.add function_name (location, true) names.t_functions }
      | _ -> names in
    names, errors
  | _ -> names, errors

let statement_errors node parents hhvm_compat_mode errors =
  let result = match syntax node with
  | BreakStatement _
    when not (hhvm_compat_mode || break_is_legal parents) ->
    Some (node, SyntaxError.error2005)
  | ContinueStatement _
    when not (hhvm_compat_mode || continue_is_legal parents) ->
    Some (node, SyntaxError.error2006)
  | TryStatement { try_catch_clauses; try_finally_clause; _ }
    when (is_missing try_catch_clauses) && (is_missing try_finally_clause) ->
    Some (node, SyntaxError.error2007)
  | UsingStatementFunctionScoped _
    when not (using_statement_function_scoped_is_legal parents) ->
    Some (node, SyntaxError.using_st_function_scoped_top_level)
  | _ -> None in
  match result with
  | None -> errors
  | Some (error_node, error_message) ->
    make_error_from_node error_node error_message :: errors

let missing_property_check is_strict hhvm_compat_mode p =
  not hhvm_compat_mode && is_strict && is_missing (p.property_type)

let invalid_var_check is_hack hhvm_compat_mode p =
  not hhvm_compat_mode && is_hack && (is_var p.property_modifiers)

let property_errors node is_strict is_hack hhvm_compat_mode errors =
  match syntax node with
  | PropertyDeclaration p ->
      let errors =
        produce_error errors (missing_property_check is_strict hhvm_compat_mode) p
        SyntaxError.error2001 node in
      let errors =
        produce_error errors (invalid_var_check is_hack hhvm_compat_mode) p
        SyntaxError.error2053 p.property_modifiers in
      errors
  | _ -> errors

let string_starts_with_int s =
  if String.length s = 0 then false else
  try let _ = int_of_string (String.make 1 s.[0]) in true with _ -> false

let check_collection_element m error_text errors =
  match syntax m with
  | PrefixUnaryExpression {
      prefix_unary_operator = {
        syntax = Token { PositionedToken.kind = TokenKind.Ampersand; _ }; _
      }; _
    } -> make_error_from_node m error_text :: errors
  | _ -> errors

let check_collection_member errors m =
  match syntax m with
  | ElementInitializer { element_key; element_value; _ } ->
    let errors =
      check_collection_element element_key
        SyntaxError.reference_not_allowed_on_key errors in
    let errors =
      check_collection_element element_value
        SyntaxError.reference_not_allowed_on_value errors in
    errors
  | _ ->
    check_collection_element m
      SyntaxError.reference_not_allowed_on_element errors

let check_collection_members members errors =
  syntax_to_list_no_separators members
  |> Core_list.fold_left ~init:errors ~f:check_collection_member

let invalid_shape_initializer_name node errors =
  match syntax node with
  | LiteralExpression { literal_expression = expr } ->
    let is_str =
      begin match token_kind expr with
      | Some TokenKind.SingleQuotedStringLiteral -> true
      (* TODO: Double quoted string are only legal
       * if they contain no encapsulated expressions. *)
      | Some TokenKind.DoubleQuotedStringLiteral -> true
      | _ -> false
      end
    in
    if not is_str
    then make_error_from_node node SyntaxError.error2059 :: errors else begin
      let str = text expr in
      if string_starts_with_int str
      then make_error_from_node node SyntaxError.error2060 :: errors
      else errors
    end
  | QualifiedNameExpression _
  | ScopeResolutionExpression _ -> errors
  | _ -> make_error_from_node node SyntaxError.error2059 :: errors

let invalid_shape_field_check node errors =
  match syntax node with
  | FieldInitializer { field_initializer_name; _} ->
    invalid_shape_initializer_name field_initializer_name errors
  | _ -> make_error_from_node node SyntaxError.error2059 :: errors

let is_in_magic_method parents =
  match first_parent_function_name parents with
  | None -> false
  | Some s -> SSet.mem s SN.Members.as_set

let is_in_finally_block parents =
  Hh_core.List.exists parents ~f:(fun node ->
    match syntax node with
    | FinallyClause _ -> true
    | _ -> false)

let expression_errors node parents is_hack is_hack_file hhvm_compat_mode errors =
  match syntax node with
  | LiteralExpression {
      literal_expression = {
        syntax = Token {
          PositionedToken.kind = (
            TokenKind.DecimalLiteral |
            TokenKind.HexadecimalLiteral
          ) as kind
        ; _}
      ; _} as e
    ; _} when is_hack_file ->
    let text = text e in
    begin try ignore (Int64.of_string text); errors
    with _ ->
      let error_text =
        if kind = TokenKind.DecimalLiteral
        then SyntaxError.error2071 text
        else SyntaxError.error2072 text in
      make_error_from_node node error_text :: errors
    end
  | SafeMemberSelectionExpression _ when not is_hack ->
    make_error_from_node node SyntaxError.error2069 :: errors
  | SubscriptExpression { subscript_left_bracket; _}
    when not hhvm_compat_mode && is_left_brace subscript_left_bracket ->
    make_error_from_node node SyntaxError.error2020 :: errors
  | FunctionCallExpression { function_call_argument_list; _} ->
    begin match misplaced_variadic_arg function_call_argument_list with
      | Some h ->
        make_error_from_node h SyntaxError.error2033 :: errors
      | None -> errors
    end
  | ObjectCreationExpression oce when not hhvm_compat_mode && is_hack ->
    if is_missing oce.object_creation_left_paren &&
        is_missing oce.object_creation_right_paren
    then
      let start_node = oce.object_creation_new_keyword in
      let end_node = oce.object_creation_type in
      let constructor_name = PositionedSyntax.text oce.object_creation_type in
      make_error_from_nodes start_node end_node
        (SyntaxError.error2038 constructor_name) :: errors
    else
      errors
  | ListExpression le
    when not hhvm_compat_mode && (is_invalid_list_expression node parents) ->
    make_error_from_node node SyntaxError.error2040 :: errors
  | ShapeExpression { shape_expression_fields; _} ->
    List.fold_right invalid_shape_field_check
      (syntax_to_list_no_separators shape_expression_fields) errors
  | DecoratedExpression
    { decorated_expression_decorator = decorator
    ; decorated_expression_expression =
      { syntax = PrefixUnaryExpression { prefix_unary_operator = operator; _ }
      ; _ }
    }
    when is_inout decorator && is_ampersand operator ->
    make_error_from_node node SyntaxError.error2076 :: errors
  | VectorIntrinsicExpression { vector_intrinsic_members = m; _ }
  | DictionaryIntrinsicExpression { dictionary_intrinsic_members = m; _ }
  | KeysetIntrinsicExpression { keyset_intrinsic_members = m; _ } ->
    let errors =
      if not is_hack
      then make_error_from_node node SyntaxError.hsl_in_php :: errors
      else errors in
    check_collection_members m errors
  | VarrayIntrinsicExpression { varray_intrinsic_members = m; _ }
  | DarrayIntrinsicExpression { darray_intrinsic_members = m; _ } ->
    let errors =
      if not is_hack
      then make_error_from_node node SyntaxError.vdarray_in_php :: errors
      else errors in
    check_collection_members m errors
  | YieldFromExpression _
  | YieldExpression _ ->
    let errors =
      if is_in_magic_method parents then
      make_error_from_node node SyntaxError.yield_in_magic_methods :: errors
      else errors in
    let errors =
      if is_in_finally_block parents then
      make_error_from_node ~error_type:SyntaxError.RuntimeError
        node SyntaxError.yield_in_finally_block :: errors
      else errors in
    errors
  | _ -> errors (* Other kinds of expressions currently produce no expr errors. *)

let require_errors node parents hhvm_compat_mode trait_use_clauses errors =
  match syntax node with
  | RequireClause p ->
    let name = text p.require_name in
    let req_kind = token_kind p.require_kind in
    let trait_use_clauses, errors =
      match SMap.get name trait_use_clauses, req_kind with
      | None, Some tk -> SMap.add name tk trait_use_clauses, errors
      | Some tk1, Some tk2 when tk1 = tk2 -> (* duplicate, it is okay *)
        trait_use_clauses, errors
      | _ -> (* Conflicting entry *)
        trait_use_clauses,
        make_error_from_node node
          (SyntaxError.conflicting_trait_require_clauses ~name) :: errors
    in
    let errors =
      match (containing_classish_kind parents, req_kind) with
      | (Some TokenKind.Class, Some TokenKind.Extends)
        when not hhvm_compat_mode ->
        make_error_from_node node SyntaxError.error2029 :: errors
      | (Some TokenKind.Interface, Some TokenKind.Implements)
      | (Some TokenKind.Class, Some TokenKind.Implements) ->
        make_error_from_node node SyntaxError.error2030 :: errors
      | _ -> errors
    in
    trait_use_clauses, errors
  | _ -> trait_use_clauses, errors

let classish_errors node parents hhvm_compat_mode names errors =
  match syntax node with
  | ClassishDeclaration cd ->
    let errors =
      produce_error errors
      (classish_duplicate_modifiers hhvm_compat_mode) cd.classish_modifiers
      SyntaxError.error2031 cd.classish_modifiers in
    let errors =
      produce_error errors
      (classish_invalid_implements_keyword hhvm_compat_mode) cd
      SyntaxError.error2035 cd.classish_implements_keyword in
    let errors =
      produce_error errors
      (classish_invalid_extends_keyword hhvm_compat_mode) cd
      SyntaxError.error2036 cd.classish_extends_keyword in
    let errors =
      produce_error errors (classish_invalid_extends_list hhvm_compat_mode) cd
      SyntaxError.error2037 cd.classish_extends_list in
    let errors =
      (* Extra setup for the the customized error message. *)
      let keyword_str = Option.value_map (token_kind cd.classish_keyword)
        ~default:"" ~f:TokenKind.to_string in
      let declared_name_str = PositionedSyntax.text cd.classish_name in
      let function_str = Option.value (first_parent_function_name parents)
        ~default:"" in
      (* To avoid iterating through the whole parents list again, do a simple
       * check on function_str rather than a harder one on cd or parents. *)
      produce_error errors
      (fun str -> not hhvm_compat_mode && String.length str != 0) function_str
      (SyntaxError.error2039 keyword_str declared_name_str function_str)
      cd.classish_keyword in
    let errors =
      (* default will never be used, since existence of abstract_keyword is a
       * necessary condition for the production of an error. *)
      let abstract_keyword = Option.value (extract_keyword is_abstract node)
        ~default:node in
      produce_error errors
      (is_classish_kind_declared_abstract hhvm_compat_mode TokenKind.Interface)
      node SyntaxError.error2042 abstract_keyword in
    let errors =
      (* default will never be used, since existence of abstract_keyword is a
       * necessary condition for the production of an error. *)
      let abstract_keyword = Option.value (extract_keyword is_abstract node)
        ~default:node in
      produce_error errors
      (is_classish_kind_declared_abstract hhvm_compat_mode TokenKind.Trait)
      node SyntaxError.error2043 abstract_keyword in
    let names, errors =
      match token_kind cd.classish_keyword with
      | Some TokenKind.Class | Some TokenKind.Trait ->
        let name = text cd.classish_name in
        let location = make_location_of_node cd.classish_name in
        begin match SMap.get name names.t_classes with
        | Some (location, false) ->
          let error =
            make_name_already_used_error cd.classish_name name name location
              SyntaxError.type_name_is_already_in_use in
          names, error :: errors
        | _ ->
          let names =
            { names with
              t_classes = SMap.add name (location, true) names.t_classes} in
          names, errors
        end
      | _ ->
        names, errors in
    names, errors
  | _ -> names, errors

let class_element_errors node parents errors =
  match syntax node with
  | ConstDeclaration _ when is_inside_trait parents ->
    make_error_from_node node SyntaxError.const_in_trait :: errors
  | _ -> errors

let type_errors node parents is_strict hhvm_compat_mode errors =
  match syntax node with
  | SimpleTypeSpecifier t ->
    produce_error errors (type_contains_array_in_strict is_strict hhvm_compat_mode)
    t.simple_type_specifier SyntaxError.error2032 t.simple_type_specifier
  | _ -> errors

let alias_errors node errors =
  match syntax node with
  | AliasDeclaration {alias_keyword; alias_constraint; _} when
    token_kind alias_keyword = Some TokenKind.Type &&
    not (is_missing alias_constraint) ->
      make_error_from_node alias_keyword SyntaxError.error2034 :: errors
  | _ -> errors

let is_invalid_group_use_clause clause =
  match syntax clause with
  | NamespaceUseClause { namespace_use_clause_kind = kind; _ } ->
    not (is_missing kind)
  | _ -> false

let is_invalid_group_use_prefix prefix =
  token_kind prefix <> Some TokenKind.NamespacePrefix

let group_use_errors node errors =
  match syntax node with
  | NamespaceGroupUseDeclaration
    { namespace_group_use_prefix = prefix
    ; namespace_group_use_clauses = clauses
    ; _} ->
      let invalid_clauses = List.filter is_invalid_group_use_clause
        (syntax_to_list_no_separators clauses) in
      let mapper errors clause =
        make_error_from_node clause SyntaxError.error2049 :: errors in
      let invalid_clause_errors =
        List.fold_left mapper errors invalid_clauses in
      produce_error invalid_clause_errors is_invalid_group_use_prefix prefix
        SyntaxError.error2048 prefix
  | _ -> errors

let use_class_or_namespace_clause_errors
  is_hack is_global_namespace kind (names, errors) cl =

  match syntax cl with
  | NamespaceUseClause {
      namespace_use_name  = name;
      namespace_use_alias = alias; _
    } ->
    let name_text = text name in
    let short_name = get_short_name_from_qualified_name name_text (text alias) in

    let do_check ~error_on_global_redefinition names errors
      get_map update_map report_error =

      let map = get_map names in
      match SMap.get short_name map with
      | Some (location, is_definition) ->
        if not is_definition
           || (error_on_global_redefinition && is_global_namespace)
        then
          let error =
            make_name_already_used_error name name_text
              short_name location report_error in
          names, error :: errors
        else
          names, errors
      | None ->
        let new_entry = make_location_of_node name, false in
        update_map names (SMap.add short_name new_entry map), errors in

    begin match syntax kind with
    | Token { PositionedToken.kind = TokenKind.Namespace; _ } ->
      do_check ~error_on_global_redefinition:false names errors
        (fun n -> n.t_namespaces)
        (fun n v -> { n with t_namespaces = v })
        SyntaxError.name_is_already_in_use

    | Token { PositionedToken.kind = TokenKind.Type; _ } ->
      do_check ~error_on_global_redefinition:false names errors
        (fun n -> n.t_classes)
        (fun n v -> { n with t_classes = v })
        SyntaxError.type_name_is_already_in_use

    | Token { PositionedToken.kind = TokenKind.Function; _ } ->
      do_check ~error_on_global_redefinition:true names errors
        (fun n -> n.t_functions)
        (fun n v -> { n with t_functions = v })
        SyntaxError.function_name_is_already_in_use

    | Token { PositionedToken.kind = TokenKind.Const; _ } ->
      do_check ~error_on_global_redefinition:true names errors
        (fun n -> n.t_constants)
        (fun n v -> { n with t_constants = v })
        SyntaxError.const_name_is_already_in_use

    | Missing ->
      let errors =
        if name_text = "strict"
        then
          let message =
            if is_hack then SyntaxError.strict_namespace_hh
            else SyntaxError.strict_namespace_not_hh in
          make_error_from_node name message :: errors
        else errors in

      let names, errors =
        let location = make_location_of_node name in
        match SMap.get short_name names.t_classes with
        | Some (loc, _) ->
          let error =
            make_name_already_used_error name name_text short_name loc
              SyntaxError.name_is_already_in_use in
            names, error :: errors
        | None ->
          let t_classes = SMap.add short_name (location, false) names.t_classes in
          let t_namespaces =
            if SMap.mem short_name names.t_namespaces
            then names.t_namespaces
            else SMap.add short_name (location, false) names.t_namespaces in
          { names with t_classes; t_namespaces }, errors in

      names, errors
    | _ ->
      names, errors
    end
  | _ ->
    names, errors

let is_global_in_const_decl init =
  match syntax init with
  | SimpleInitializer { simple_initializer_value; _ } ->
    begin match syntax simple_initializer_value with
    | VariableExpression { variable_expression } ->
      SN.Superglobals.is_superglobal @@ text variable_expression
    | _ -> false
    end
  | _ -> false

let namespace_use_declaration_errors node is_hack is_global_namespace names errors =
  match syntax node with
  | NamespaceUseDeclaration {
      namespace_use_kind = kind;
      namespace_use_clauses = clauses; _ }
  | NamespaceGroupUseDeclaration {
      namespace_group_use_kind = kind;
      namespace_group_use_clauses = clauses; _ } ->
    let f =
      use_class_or_namespace_clause_errors is_hack is_global_namespace kind in
    List.fold_left f (names, errors) (syntax_to_list_no_separators clauses)
  | _ -> names, errors

let const_decl_errors node parents hhvm_compat_mode names errors =
  match syntax node with
  | ConstantDeclarator cd ->
    let errors =
      produce_error_parents errors
      (concrete_no_initializer hhvm_compat_mode) cd parents
      SyntaxError.error2050 cd.constant_declarator_initializer in
    let errors =
      produce_error_parents errors abstract_with_initializer cd parents
      SyntaxError.error2051 cd.constant_declarator_initializer in
    let errors =
      produce_error errors is_global_in_const_decl cd.constant_declarator_initializer
      SyntaxError.global_in_const_decl cd.constant_declarator_initializer in
    let constant_name = text cd.constant_declarator_name in
    let location = make_location_of_node cd.constant_declarator_name in
    let names = {
      names with t_constants =
        SMap.add constant_name (location, true) names.t_constants } in
    names, errors
  | _ -> names, errors

let abstract_final_class_nonstatic_var_error node parents hhvm_compat_mode errors =
  match syntax node with
  | PropertyDeclaration cd ->
    produce_error_parents errors
    (abstract_final_with_inst_var hhvm_compat_mode) node parents
    SyntaxError.error2061 cd.property_modifiers
  | _ -> errors

let abstract_final_class_nonstatic_method_error node parents hhvm_compat_mode errors =
  match syntax node with
  | MethodishDeclaration cd ->
    produce_error_parents errors
    (abstract_final_with_method hhvm_compat_mode) node parents
    SyntaxError.error2062 cd.methodish_function_decl_header
  | _ -> errors

let mixed_namespace_errors node namespace_type errors =
  match syntax node with
  | NamespaceBody { namespace_left_brace; namespace_right_brace; _ } ->
    let s = start_offset namespace_left_brace in
    let e = end_offset namespace_right_brace in
    begin match namespace_type with
    | Unbracketed { start_offset; end_offset } ->
      let child = Some
        (SyntaxError.make start_offset end_offset SyntaxError.error2057)
      in
      SyntaxError.make ~child s e SyntaxError.error2052 :: errors
    | _ -> errors
    end
  | NamespaceEmptyBody { namespace_semicolon; _ } ->
    let s = start_offset namespace_semicolon in
    let e = end_offset namespace_semicolon in
    begin match namespace_type with
    | Bracketed { start_offset; end_offset } ->
      let child = Some
        (SyntaxError.make start_offset end_offset SyntaxError.error2056)
      in
      SyntaxError.make ~child s e SyntaxError.error2052 :: errors
    | _ -> errors
    end
  | _ -> errors

let find_syntax_errors ?positioned_syntax ~enable_hh_syntax hhvm_compatiblity_mode syntax_tree =
  let is_strict = SyntaxTree.is_strict syntax_tree in
  let is_hack_file = (SyntaxTree.language syntax_tree = "hh") in
  let is_hack = is_hack_file || enable_hh_syntax in
  let rec folder acc node parents =
    let { errors
        ; namespace_type
        ; names
        ; has_namespace_prefix
        ; trait_require_clauses
        } = acc in
    let errors =
      markup_errors node is_hack_file hhvm_compatiblity_mode errors in
    let errors =
      parameter_errors node parents is_strict is_hack hhvm_compatiblity_mode errors in
    let names, errors =
      function_errors node parents is_strict hhvm_compatiblity_mode names errors in
    let errors =
      xhp_errors node parents hhvm_compatiblity_mode errors in
    let errors =
      statement_errors node parents hhvm_compatiblity_mode errors in
    let errors =
      methodish_errors node parents is_hack hhvm_compatiblity_mode errors in
    let errors =
      property_errors node is_strict is_hack hhvm_compatiblity_mode errors in
    let errors =
      expression_errors node parents is_hack is_hack_file hhvm_compatiblity_mode errors in
    let trait_require_clauses, errors =
      require_errors node parents hhvm_compatiblity_mode trait_require_clauses errors in
    let names, errors =
      classish_errors node parents hhvm_compatiblity_mode names errors in
    let errors =
      class_element_errors node parents errors in
    let errors =
      type_errors node parents is_strict hhvm_compatiblity_mode errors in
    let errors = alias_errors node errors in
    let errors = group_use_errors node errors in
    let names, errors =
      const_decl_errors node parents hhvm_compatiblity_mode names errors in
    let errors =
      abstract_final_class_nonstatic_var_error node parents hhvm_compatiblity_mode errors in
    let errors =
      abstract_final_class_nonstatic_method_error node parents hhvm_compatiblity_mode errors in
    let errors =
      mixed_namespace_errors node namespace_type errors in
    let names, errors =
      namespace_use_declaration_errors node is_hack (not has_namespace_prefix)
        names errors in

    match syntax node with
    | NamespaceBody { namespace_left_brace; namespace_right_brace; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Bracketed (make_location namespace_left_brace namespace_right_brace)
        else namespace_type in
      (* reset names/namespace_type before diving into namespace body *)
      let has_namespace_prefix = has_namespace_prefix ||
        match parents with
        | { syntax = NamespaceDeclaration { namespace_name; _ }; _ } :: _ ->
          not (is_missing namespace_name)
        | _ -> false in
      let acc1 =
        make_acc
          acc errors namespace_type empty_names
          has_namespace_prefix empty_trait_require_clauses
      in
      let acc1 = fold_child_nodes folder node parents acc1 in
      (* resume with old set of names and pull back
        accumulated errors/last seen namespace type *)
        make_acc
          acc acc1.errors namespace_type acc.names
          acc.has_namespace_prefix acc.trait_require_clauses
    | NamespaceEmptyBody { namespace_semicolon; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Unbracketed (make_location_of_node namespace_semicolon)
        else namespace_type
      in
      (* consider the rest of file to be the part of the namespace:
         reset names and namespace type, keep errors *)
      let acc =
        make_acc
          acc errors namespace_type empty_names
          has_namespace_prefix empty_trait_require_clauses
      in
      fold_child_nodes folder node parents acc
    | ClassishDeclaration _ ->
      (* Reset the trait require clauses *)
      let acc =
        make_acc
          acc errors namespace_type names
          has_namespace_prefix empty_trait_require_clauses
      in
      fold_child_nodes folder node parents acc
    | _ ->
      let acc =
        make_acc
          acc errors namespace_type names
          has_namespace_prefix trait_require_clauses
      in
      fold_child_nodes folder node parents acc in

  let node =
    match positioned_syntax with
    | Some n -> n
    | None -> PositionedSyntax.from_tree syntax_tree in
  let acc = fold_child_nodes folder node []
    { errors = []
    ; namespace_type = Unspecified
    ; names = empty_names
    ; has_namespace_prefix = false
    ; trait_require_clauses = empty_trait_require_clauses
    } in
  acc.errors

type error_level = Minimum | Typical | Maximum | HHVMCompatibility

let parse_errors ?(enable_hh_syntax=false) ?(level=Typical) ?positioned_syntax syntax_tree =
  (*
  Minimum: suppress cascading errors; no second-pass errors if there are
  any first-pass errors.
  Typical: suppress cascading errors; give second pass errors always.
  Maximum: all errors
  *)
  let errors1 = match level with
  | Maximum -> SyntaxTree.all_errors syntax_tree
  | _ -> SyntaxTree.errors syntax_tree in
  let errors2 =
    if level = Minimum && errors1 <> [] then []
    else find_syntax_errors
      ~enable_hh_syntax
      ?positioned_syntax (level = HHVMCompatibility) syntax_tree in
  List.sort SyntaxError.compare (Core_list.append errors1 errors2)

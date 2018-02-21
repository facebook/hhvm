(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
open Syntax

module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Syntax)

module Token = Syntax.Token
module SyntaxError = Full_fidelity_syntax_error
module TokenKind = Full_fidelity_token_kind

module SN = Naming_special_names

type location = {
  start_offset: int;
  end_offset: int
}

let text node =
  Option.value ~default:"<text_extraction_failure>" (Syntax.extract_text node)

let make_location s e =
  match position Relative_path.default s, position Relative_path.default e with
  | Some s, Some e ->
    let start_offset, _ = Pos.info_raw s in
    let _, end_offset = Pos.info_raw e in
    { start_offset ; end_offset }
  | _ ->
    failwith "Could not determine positions of parse tree nodes."

let make_location_of_node n =
  make_location n n

let start_offset n =
  let s = Syntax.position Relative_path.default n in
  let s, _ = Pos.info_raw (Option.value ~default:Pos.none s) in
  s

let end_offset n =
  let e = Syntax.position Relative_path.default n in
  let _, e = Pos.info_raw (Option.value ~default:Pos.none e) in
  e

type namespace_type =
  | Unspecified
  | Bracketed of location
  | Unbracketed of location

type name_kind =
  | Name_use    (* `use` construct *)
  | Name_def (* definition e.g. `class` or `trait` *)
  | Name_implicit_use (* implicit `use` e.g. HH type in type hint *)

type first_use_or_def = {
  f_location: location;
  f_kind: name_kind;
  f_name: string;
}

type error_level =
  Minimum | Typical | Maximum | HHVMCompatibility | HHVMCompatibilitySystemLib

let is_hhvm_compat level =
  level = HHVMCompatibility || level = HHVMCompatibilitySystemLib

let is_systemlib_compat level = level = HHVMCompatibilitySystemLib

let global_namespace_name = "\\"

let combine_names n1 n2 =
  assert (String.length n1 > 0 && String.length n2 > 0);
  let has_leading_slash = String.get n2 0 = '\\' in
  let len = String.length n1 in
  let has_trailing_slash = String.get n1 (len - 1) = '\\' in
  match has_leading_slash, has_trailing_slash with
  | true, true -> n1 ^ (String.sub n2 1 (String.length n2 - 1))
  | false, false -> n1 ^ "\\" ^ n2
  | _ -> n1 ^ n2

let make_first_use_or_def ~kind location namespace_name name =
  { f_location = location; f_kind = kind; f_name = combine_names namespace_name name }

type used_names = {
  t_classes: first_use_or_def SMap.t;
  t_namespaces: first_use_or_def SMap.t;
  t_functions: first_use_or_def SMap.t;
  t_constants: first_use_or_def SMap.t;
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
  namespace_name: string;
  names: used_names;
  trait_require_clauses: TokenKind.t SMap.t;
}

let make_acc
  acc errors namespace_type names namespace_name trait_require_clauses =
  if acc.errors == errors &&
     acc.namespace_type == namespace_type &&
     acc.names == names &&
     acc.namespace_name = namespace_name &&
     acc.trait_require_clauses = trait_require_clauses
  then acc
  else { errors
       ; namespace_type
       ; names
       ; namespace_name
       ; trait_require_clauses
       }

let fold_child_nodes ?(cleanup = (fun x -> x)) f node parents acc =
  Syntax.children node
  |> Core_list.fold_left ~init:acc ~f:(fun acc c -> f acc c (node :: parents))
  |> cleanup

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

(* Test two levels in case ...& or &... hiding under *)
let rec is_reference_expression node =
  is_decorated_expression ~f:is_ampersand node ||
  test_decorated_expression_child node ~f:is_reference_expression

let rec is_variadic_expression node =
  is_decorated_expression ~f:is_ellipsis node ||
  test_decorated_expression_child node ~f:is_variadic_expression

let is_reference_variadic node =
  is_decorated_expression ~f:is_ellipsis node &&
  test_decorated_expression_child node ~f:is_reference_expression

let is_double_variadic node =
  is_decorated_expression ~f:is_ellipsis node &&
  test_decorated_expression_child node ~f:is_variadic_expression

let is_double_reference node =
  is_decorated_expression ~f:is_ampersand node &&
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
      type t = Syntax.t
      let compare a b = match syntax a, syntax b with
      | Token x, Token y ->
        Token.(compare (kind x) (kind y))
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
  | Token t -> Some (Token.kind t)
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

let modifiers_of_function_decl_header_exn node =
  match syntax node with
  | FunctionDeclarationHeader { function_modifiers = m; _ } -> m
  | _ -> failwith "expected to get FunctionDeclarationHeader"

let get_modifiers_of_methodish_declaration node =
  match syntax node with
  | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
    Some (modifiers_of_function_decl_header_exn header)
  | _ -> None

(* tests whether the methodish contains a modifier that satisfies [p] *)
let methodish_modifier_contains_helper p node =
  get_modifiers_of_methodish_declaration node
    |> Option.exists ~f:(list_contains_predicate p)

(* tests whether the methodish contains > 1 modifier that satisfies [p] *)
let methodish_modifier_multiple_helper p node =
  get_modifiers_of_methodish_declaration node
    |> Option.value_map ~default:false ~f:(list_contains_multiple_predicate p)

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

let is_not_public_visibility x =
  is_private x || is_protected x

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
  match node with
  | FunctionDeclarationHeader node ->
    let label = node.function_name in
    (is_construct label) && (matches_first methodish_contains_static parents)
  | _ -> false

(* Given a function declaration header, confirm that it is NOT a constructor
 * and that the header containing it has visibility modifiers in parameters
 *)
let class_non_constructor_has_visibility_param node parents =
  match node with
  | FunctionDeclarationHeader node ->
    let has_visibility node =
      match syntax node with
      | ParameterDeclaration { parameter_visibility; _ } ->
        parameter_visibility |> is_missing |> not
      | _ -> false
    in
    let label = node.function_name in
    let params = syntax_to_list_no_separators node.function_parameter_list in
    (not (is_construct label)) && (List.exists has_visibility params)
  | _ -> false

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-empty parameters *)
let class_destructor_has_param hhvm_compat_mode node parents =
  match node with
  | FunctionDeclarationHeader node ->
    let label = node.function_name in
    let param = node.function_parameter_list in
    not hhvm_compat_mode && (is_destruct label) && not (is_missing param)
  | _ -> false

(* Given a function declaration header, confirm that it is a destructor
 * and that the methodish containing it has non-visibility modifiers *)
let class_destructor_has_non_visibility_modifier hhvm_compat_mode node parents =
  match node with
  | FunctionDeclarationHeader node ->
    let label = node.function_name in
    not hhvm_compat_mode &&
    (is_destruct label) &&
    (matches_first (methodish_contains_non_visibility hhvm_compat_mode) parents)
  | _ -> false

let async_magic_method node parents =
  match node with
  | FunctionDeclarationHeader node ->
    SSet.mem (String.lowercase_ascii @@ text node.function_name) SN.Members.as_set &&
    list_contains_predicate is_async node.function_modifiers
  | _ -> false

(* check that a constructor or a destructor is type annotated *)
let class_constructor_destructor_has_non_void_type hhvm_compat_node node parents =
  if hhvm_compat_node then false
  else
  match node with
  | FunctionDeclarationHeader node ->
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
  | _ -> false

(* whether a methodish has duplicate modifiers *)
let methodish_duplicate_modifier node =
  get_modifiers_of_methodish_declaration node
    |> Option.value_map ~default:false ~f:list_contains_duplicate

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
  | h :: _ when (is_anonymous_function h) || (is_lambda_expression h) -> false
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
  | _ ::
    { syntax = CompoundStatement _; _ } ::
    { syntax = (
        FunctionDeclaration  _ |
        MethodishDeclaration _ |
        AnonymousFunction    _ |
        LambdaExpression     _); _ } :: _ -> true
  | _ -> false

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
      Full_fidelity_token_kind.(match Token.kind t with
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
  match syntax node with
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

let classish_duplicate_modifiers node =
  list_contains_duplicate node

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
  check (syntax node) parents

let produce_error_for_header acc check node error error_node =
  produce_error_parents acc (function_header_check_helper check) node
    error error_node

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

let is_reserved_keyword is_hack classish_name =
  let name = text classish_name in
  (* TODO: What else goes here? *)
  match String.lowercase_ascii name with
  | "eval" | "isset" | "unset" | "empty" | "const" | "new"
  | "and"  | "or"    | "xor"  | "as" | "print" | "throw"
  | "array" | "instanceof" | "trait" | "class" | "interface"
  | "static" -> true
  | "using" | "inout" when is_hack -> true
  | _ -> false

(* Given a function_declaration_header node, returns its function_name
 * as a string opt. *)
let extract_function_name header_node =
  (* The '_' arm of this match will never be reached, but the type checker
   * doesn't allow a direct extraction of function_name from
   * function_declaration_header. *)
   match syntax header_node with
   | FunctionDeclarationHeader fdh ->
     Syntax.extract_text fdh.function_name
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

(* Returns the whether the current context is in an active class scope *)
let is_in_active_class_scope parents =
  Hh_core.List.exists parents ~f:begin fun node ->
  match syntax node with
  | ClassishDeclaration cd -> true
  | _ -> false
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
    | ClassishDeclaration cd -> Syntax.extract_text cd.classish_name
    | _ -> None (* This arm is never reached  *)
  end

(* Given a classish_ or methodish_ declaration node, returns the modifier node
   from its list of modifiers, or None if there isn't one. *)
let extract_keyword modifier declaration_node =
  let aux modifiers_list =
    Hh_core.List.find ~f:modifier (syntax_to_list_no_separators modifiers_list)
  in

  match syntax declaration_node with
  | ClassishDeclaration { classish_modifiers = modifiers_list ; _ } ->
    aux modifiers_list
  | _ ->
    Option.bind (get_modifiers_of_methodish_declaration declaration_node) aux

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
  get_modifiers_of_methodish_declaration md_node
  |> Option.value_map ~default:[] ~f:syntax_to_list_no_separators
  |> Hh_core.List.find ~f:is_async

let first_parent_function_declaration parents =
  Hh_core.List.find_map parents ~f:begin fun node ->
    match syntax node with
    | FunctionDeclaration { function_declaration_header = header; _ }
    | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
      begin match syntax header with
      | FunctionDeclarationHeader fdh -> Some (fdh.function_parameter_list, fdh.function_modifiers)
      | _ -> None
      end
    | _ -> None
    end

let first_parent_function_attributes_contains parents name =
  Hh_core.List.exists parents ~f:begin fun node ->
    match syntax node with
    | FunctionDeclaration { function_attribute_spec = {
        syntax = AttributeSpecification {
          attribute_specification_attributes; _ }; _ }; _ }
    | MethodishDeclaration { methodish_attribute = {
        syntax = AttributeSpecification {
          attribute_specification_attributes; _ }; _ }; _ } ->
      let attrs =
        syntax_to_list_no_separators attribute_specification_attributes in
      Hh_core.List.exists attrs
        ~f:(function { syntax = Attribute { attribute_name; _}; _} ->
          text attribute_name = name | _ -> false)
    | _ -> false
    end

let is_parameter_with_callconv param =
  match syntax param with
  | ParameterDeclaration { parameter_call_convention; _ } ->
    not @@ is_missing parameter_call_convention
  | ClosureParameterTypeSpecifier { closure_parameter_call_convention; _ } ->
    not @@ is_missing closure_parameter_call_convention
  | VariadicParameter { variadic_parameter_call_convention; _ } ->
    not @@ is_missing variadic_parameter_call_convention
  | _ -> false

let has_inout_params parents =
  match first_parent_function_declaration parents with
  | Some (function_parameter_list, _) ->
    let params = syntax_to_list_no_separators function_parameter_list in
    Hh_core.List.exists params ~f:is_parameter_with_callconv
  | _ -> false

let is_inside_async_method parents =
  match first_parent_function_declaration parents with
  | Some (_,m) ->
    syntax_to_list_no_separators m
    |> Hh_core.List.exists ~f:is_async
  | None -> false

let make_name_already_used_error node name short_name original_location
  report_error =
  let name = Utils.strip_ns name in
  let original_location_error =
    SyntaxError.make
      original_location.start_offset
      original_location.end_offset
      SyntaxError.original_definition in
  let s = start_offset node in
  let e = end_offset node in
  SyntaxError.make
    ~child:(Some original_location_error) s e (report_error ~name ~short_name)

let check_type_name_reference name_text location is_hack names errors =
  if not (is_hack && Hh_autoimport.is_hh_autoimport name_text) || SMap.mem name_text names.t_classes
  then names, errors
  else
    let def = make_first_use_or_def ~kind:Name_implicit_use location "HH" name_text in
    let names = { names with t_classes = SMap.add name_text def names.t_classes} in
    names, errors

let check_type_hint node is_hack names errors =
  let rec check (names, errors) node =
    let names, errors =
      Core_list.fold_left (Syntax.children node) ~f:check ~init:(names, errors) in
    match syntax node with
    | SimpleTypeSpecifier { simple_type_specifier = s; _ }
    | GenericTypeSpecifier { generic_class_type = s; _ } ->
      check_type_name_reference (text s) (make_location_of_node node) is_hack names errors
    | _ ->
      names, errors
  in
    check (names, errors) node

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
  | ClosureParameterTypeSpecifier { closure_parameter_call_convention; _ } ->
    Some closure_parameter_call_convention
  | VariadicParameter { variadic_parameter_call_convention; _ } ->
    Some variadic_parameter_call_convention
  | _ -> None

(* Tests if visibility modifiers of the node are allowed on
 * methods inside an interface. *)
let has_valid_interface_visibility node =
  (* If not a methodish declaration, is vacuously valid *)
  get_modifiers_of_methodish_declaration node
  |> Option.value_map ~default:true ~f:(fun methodish_modifiers ->
    let visibility_kind = extract_visibility_node methodish_modifiers in
    let is_valid_methodish_visibility kind =
      (is_token_kind kind TokenKind.Public) in
    (* Defaulting to 'true' allows omitting visibility in method_declarations *)
    Option.value_map visibility_kind
      ~f:is_valid_methodish_visibility ~default:true)

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
let concrete_no_initializer hhvm_compat_mode init parents =
  if hhvm_compat_mode then false
  else
  let is_concrete =
    match parents with
    | _ :: _ :: p3 :: _ when is_concrete_const p3 -> true
    | _ -> false
    in
  let has_no_initializer =
    is_missing init in
  is_concrete && has_no_initializer

(* Given a ConstDeclarator node, test whether it is abstract, but has an
   initializer. *)
let abstract_with_initializer init parents =
  let is_abstract =
    match parents with
    | _ :: _ :: p3 :: _ when is_abstract_const p3 -> true
    | _ -> false
    in
  let has_initializer =
    not (is_missing init) in
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

let is_byref_expression node =
  is_decorated_expression ~f:is_ampersand node

let is_byref_parameter_variable node =
  (* TODO: This shouldn't be a decorated *expression* because we are not
  expecting an expression at all. We're expecting a declaration. *)
  is_byref_expression node

let is_param_by_ref node =
  match syntax node with
  | ParameterDeclaration { parameter_name; _ } ->
    is_byref_parameter_variable parameter_name
  | _ -> false

let special_method_param_errors node parents errors =
  match syntax node with
  | FunctionDeclarationHeader {function_name; function_parameter_list; _}
    when SSet.mem (String.lowercase_ascii @@ text function_name)
                  SN.Members.as_set ->
    let params = syntax_to_list_no_separators function_parameter_list in
    let len = Hh_core.List.length params in
    let name = text function_name in
    let full_name = match first_parent_class_name parents with
      | None -> name
      | Some c_name -> c_name ^ "::" ^ name ^ "()"
    in
    let s = String.lowercase_ascii name in
    let num_args_opt =
      match s with
      | _ when s = SN.Members.__call && len <> 2 -> Some 2
      | _ when s = SN.Members.__callStatic && len <> 2 -> Some 2
      | _ when s = SN.Members.__get && len <> 1 -> Some 1
      | _ when s = SN.Members.__set && len <> 2 -> Some 2
      | _ when s = SN.Members.__isset && len <> 1 -> Some 1
      | _ when s = SN.Members.__unset && len <> 1 -> Some 1
      | _ -> None
    in
    let errors = match num_args_opt with
      | None -> errors
      | Some n ->
        make_error_from_node
          node (SyntaxError.invalid_number_of_args full_name n) :: errors
    in
    let errors = if (s = SN.Members.__call
                  || s = SN.Members.__callStatic
                  || s = SN.Members.__get
                  || s = SN.Members.__set
                  || s = SN.Members.__isset
                  || s = SN.Members.__unset)
                  && Hh_core.List.exists ~f:is_param_by_ref params then
        make_error_from_node
          node (SyntaxError.invalid_args_by_ref full_name) :: errors
      else errors
    in
    errors
  | _ -> errors

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
    let modifiers = modifiers_of_function_decl_header_exn header_node in
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
      produce_error_for_header errors async_magic_method header_node [node]
      SyntaxError.async_magic_method modifiers in
    let errors =
      produce_error errors (methodish_multiple_visibility hhvm_compat_mode) node
      SyntaxError.error2017 modifiers in
    let errors =
      produce_error errors methodish_duplicate_modifier node
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
      let visibility_text = text visibility_node in (* is this option? *)
      produce_error errors
      (invalid_methodish_visibility_inside_interface hhvm_compat_mode node)
      parents (SyntaxError.error2047 visibility_text) visibility_node in
    let errors =
      special_method_param_errors
        md.methodish_function_decl_header parents errors in
    errors
  | _ -> errors

let is_hashbang text =
  match Syntax.extract_text text with
  | None -> false
  | Some text ->
    let r = Str.regexp "^#!.*\n" in
    let count = List.length @@ String_utils.split_on_newlines text in
    count = 1 && Str.string_match r text 0 && Str.matched_string text = text

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
    && (Syntax.extract_text markup_suffix_name <> Some "php") ->
    make_error_from_node node SyntaxError.error2068 :: errors
  | _ -> errors

let default_value_params is_hack hhvm_compat_mode params =
  match param_missing_default_value params with
  | Some param when not hhvm_compat_mode && is_hack -> Some param
  | _ -> None

let is_in_namespace parents =
  Hh_core.List.exists parents ~f:(fun node ->
    match syntax node with
    | NamespaceDeclaration {namespace_name; _}
      when not @@ is_missing namespace_name && text namespace_name <> "" -> true
    | _ -> false)

let class_has_a_construct_method parents =
  match first_parent_classish_node TokenKind.Class parents with
  | Some ({ syntax = ClassishDeclaration
            { classish_body =
              { syntax = ClassishBody
                { classish_body_elements = methods; _}; _}; _}; _}) ->
    let methods = syntax_to_list_no_separators methods in
    Hh_core.List.exists methods ~f:(function
      { syntax = MethodishDeclaration
          { methodish_function_decl_header =
            { syntax = FunctionDeclarationHeader
              { function_name; _}; _}; _}; _} ->
        String.lowercase_ascii @@ text function_name = SN.Members.__construct
      | _ -> false)
  | _ -> false

let is_in_construct_method parents =
  match first_parent_function_name parents, first_parent_class_name parents with
  | None, _ -> false
  (* Function name is __construct *)
  | Some s, _ when String.lowercase_ascii s = SN.Members.__construct -> true
  (* Function name is same as class name *)
  | Some s1, Some s2 ->
    not @@ is_in_namespace parents &&
    not @@ class_has_a_construct_method parents &&
    String.lowercase_ascii s1 = String.lowercase_ascii s2
  | _ -> false

(* Test if the parameter is missing a type annotation but one is required *)
let missing_param_type_check is_strict hhvm_compat_mode parameter_type parents =
  let is_required = parameter_type_is_required parents in
  not hhvm_compat_mode && is_strict && is_missing parameter_type && is_required

(* If a variadic parameter has a default value, return it *)
let variadic_param_with_default_value params =
  Option.filter (variadic_param params) ~f:is_parameter_with_default_value

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

(* If an inout parameter is passed by reference, return it *)
let param_with_callconv_is_byref node =
  match syntax node with
  | ParameterDeclaration { parameter_name; _ } when
    is_parameter_with_callconv node &&
    is_byref_parameter_variable parameter_name -> Some node
  | _ -> None

let params_errors params is_hack hhvm_compat_mode namespace_name names errors =
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
  let param_list = syntax_to_list_no_separators params in
  let has_inout_param, has_reference_param, has_inout_and_ref_param =
    Hh_core.List.fold_right param_list ~init:(false, false, false)
      ~f:begin fun p (b1, b2, b3) ->
        let is_inout = is_parameter_with_callconv p in
        let is_ref = is_param_by_ref p in
        b1 || is_inout, b2 || is_ref, b3 || (is_inout && is_ref)
      end
  in
  let errors = if has_inout_param && has_reference_param then
    let error_type = if has_inout_and_ref_param then
      SyntaxError.ParseError else SyntaxError.RuntimeError in
    make_error_from_node ~error_type
      params SyntaxError.fn_with_inout_and_ref_params :: errors
    else errors
  in
  names, errors

let decoration_errors node errors =
  let errors = produce_error errors is_double_variadic node SyntaxError.double_variadic node in
  let errors = produce_error errors is_double_reference node SyntaxError.double_reference node in
  errors

let parameter_errors node parents is_strict is_hack hhvm_compat_mode namespace_name names errors =
  match syntax node with
  | ParameterDeclaration p ->
    let errors =
      produce_error_parents errors (missing_param_type_check is_strict hhvm_compat_mode)
      p.parameter_type parents SyntaxError.error2001 node in
    let callconv_text = Option.value (extract_callconv_node node) ~default:node
      |> text in
    let errors =
      produce_error_from_check errors param_with_callconv_has_default
      node (SyntaxError.error2074 callconv_text) in
    let errors =
      produce_error_from_check errors param_with_callconv_is_byref
      node (SyntaxError.error2075 callconv_text) in
    let names, errors =
      check_type_hint p.parameter_type is_hack names errors in
    let errors = if is_parameter_with_callconv node then
      begin
        let errors = if is_inside_async_method parents then
        make_error_from_node ~error_type:SyntaxError.RuntimeError
          node SyntaxError.inout_param_in_async :: errors
        else errors in
        let errors =
          if is_in_construct_method parents then
          make_error_from_node ~error_type:SyntaxError.RuntimeError
            node SyntaxError.inout_param_in_construct :: errors
          else errors in
        let errors = if first_parent_function_attributes_contains
              parents SN.UserAttributes.uaMemoize then
          make_error_from_node ~error_type:SyntaxError.RuntimeError
            node SyntaxError.memoize_with_inout :: errors
          else errors in
        errors
      end else errors
    in
    let errors =
      if not is_hack &&
         is_variadic_expression p.parameter_name &&
         not (is_missing p.parameter_type) then
        (* Strip & and ..., reference will always come before variadic *)
        let name = String_utils.lstrip (text p.parameter_name) "&" in
        let name = String_utils.lstrip name "..." in
        let type_ = text p.parameter_type in
        make_error_from_node node
          (SyntaxError.variadic_param_with_type_in_php name type_) :: errors
      else errors in
    let errors =
      if is_reference_variadic p.parameter_name then
        make_error_from_node node SyntaxError.variadic_reference :: errors
      else errors in
    names, errors
  | FunctionDeclarationHeader { function_parameter_list = params; _ }
  | AnonymousFunction { anonymous_parameters = params; _ }
  | ClosureTypeSpecifier { closure_parameter_list = params; _ } ->
    params_errors params is_hack hhvm_compat_mode namespace_name names errors
  | LambdaExpression
    { lambda_signature = {syntax = LambdaSignature { lambda_parameters; _ }; _}
    ; _
    } -> params_errors lambda_parameters is_hack hhvm_compat_mode namespace_name names errors
  | DecoratedExpression _ -> names, decoration_errors node errors
  | _ -> names, errors

let function_errors node parents is_strict hhvm_compat_mode errors =
  match syntax node with
  | FunctionDeclarationHeader f ->
    let missing_type_annot_check is_strict hhvm_compat_mode _ =
      let label = f.function_name in
      let is_function = not (is_construct label) && not (is_destruct label) in
      not hhvm_compat_mode && is_strict && is_missing f.function_type && is_function in

    let function_reference_check is_strict hhvm_compat_mode _ =
      not hhvm_compat_mode && is_strict && not (is_missing f.function_ampersand) in

    let errors =
      produce_error errors (missing_type_annot_check is_strict hhvm_compat_mode) ()
      SyntaxError.error2001 f.function_right_paren in
    let errors =
      produce_error errors (function_reference_check is_strict hhvm_compat_mode) ()
      SyntaxError.error2064 f.function_ampersand in
    errors
  | _ -> errors

let redeclaration_errors node parents syntax_tree namespace_name names errors =
  match syntax node with
  | FunctionDeclarationHeader f when not (is_missing f.function_name)->
    begin match parents with
      | { syntax = FunctionDeclaration _; _}
        :: _ :: {syntax = NamespaceBody _; _} :: _
      | [{ syntax = FunctionDeclaration _; _ }; _; _]
      | { syntax = MethodishDeclaration _; _ } :: _ ->
        let function_name = text f.function_name in
        let location = make_location_of_node f.function_name in
        let def = make_first_use_or_def
          ~kind:Name_def location namespace_name function_name in
        let errors =
          match SMap.get function_name names.t_functions with
          | Some { f_location = { start_offset; _}; f_kind = Name_def; _} ->
            let text = SyntaxTree.text syntax_tree in
            let line, _ =
              Full_fidelity_source_text.offset_to_position text start_offset in
            let path =
              Relative_path.to_absolute @@
                Full_fidelity_source_text.file_path text in
            let loc = path ^ ":" ^ string_of_int line in
            let err, error_type =
              match first_parent_class_name parents with
              | None ->
                SyntaxError.redeclaration_of_function ~name:function_name ~loc,
                SyntaxError.RuntimeError
              | Some class_name ->
                let full_name = class_name ^ "::" ^ function_name in
                SyntaxError.redeclaration_of_method ~name:full_name,
                SyntaxError.ParseError
            in
            make_error_from_node ~error_type node err :: errors
          | _ -> errors
        in
        { names with
          t_functions = SMap.add function_name def names.t_functions }, errors
      | _ -> names, errors
    end
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

let property_errors node is_strict is_hack hhvm_compat_mode errors =
  match syntax node with
  | PropertyDeclaration p ->
      let missing_property_check is_strict hhvm_compat_mode _ =
        not hhvm_compat_mode && is_strict && is_missing (p.property_type) in
      let invalid_var_check is_hack hhvm_compat_mode _ =
        not hhvm_compat_mode && is_hack && (is_var p.property_modifiers) in

      let errors =
        produce_error errors (missing_property_check is_strict hhvm_compat_mode) ()
        SyntaxError.error2001 node in
      let errors =
        produce_error errors (invalid_var_check is_hack hhvm_compat_mode) ()
        SyntaxError.error2053 p.property_modifiers in

      let modifiers = syntax_to_list_no_separators p.property_modifiers in
      let errors = if Hh_core.List.exists ~f:is_final modifiers then
        make_error_from_node node SyntaxError.final_property :: errors
        else errors in
      errors
  | _ -> errors

let string_starts_with_int s =
  if String.length s = 0 then false else
  try let _ = int_of_string (String.make 1 s.[0]) in true with _ -> false

let check_collection_element m error_text errors =
  match syntax m with
  | PrefixUnaryExpression
    { prefix_unary_operator = { syntax = Token token; _ }; _ }
    when Token.kind token = TokenKind.Ampersand ->
      make_error_from_node m error_text :: errors
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
  | QualifiedName _
  | ScopeResolutionExpression _ -> errors
  | Token _ when is_name node -> errors
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

let function_call_argument_errors node errors =
  match syntax node with
  | DecoratedExpression
    { decorated_expression_decorator = { syntax = Token token ; _ }
    ; decorated_expression_expression = expression
    } when Token.kind token = TokenKind.Inout ->
      let result =
        match syntax expression with
        | BinaryExpression _ ->
          Some (true, SyntaxError.fun_arg_inout_set)
        | QualifiedName _ ->
          Some (true, SyntaxError.fun_arg_inout_const)
        | Token _ when is_name expression ->
          Some (true, SyntaxError.fun_arg_inout_const)
        (* TODO: Maybe be more descriptive in error messages *)
        | ScopeResolutionExpression _
        | FunctionCallExpression _
        | MemberSelectionExpression _
        | SafeMemberSelectionExpression _
        | SubscriptExpression
          { subscript_receiver = {
            syntax =
              (MemberSelectionExpression _ | ScopeResolutionExpression _)
              ; _
            }; _ } -> Some (true, SyntaxError.fun_arg_invalid_arg)
        | SubscriptExpression { subscript_receiver; _ }
          when SN.Superglobals.is_superglobal @@ text subscript_receiver ->
            Some (false, SyntaxError.fun_arg_inout_containers)
        | _ -> None
      in
      begin match result with
      | None -> errors
      | Some (is_parse_error, e) ->
        let error_type = if is_parse_error then
          SyntaxError.ParseError else SyntaxError.RuntimeError in
        make_error_from_node ~error_type node e :: errors
      end
  | _ -> errors

let function_call_on_xhp_name_errors node errors =
  match syntax node with
  | MemberSelectionExpression { member_name = name; _ }
  | SafeMemberSelectionExpression { safe_member_name = name; _ } ->
    begin match syntax name with
    | Token token when Token.kind token = TokenKind.XHPClassName ->
      let e =
        make_error_from_node node SyntaxError.method_calls_on_xhp_attributes in
      e :: errors
    | _ -> errors
    end
  | _ -> errors

let expression_errors node parents is_hack is_hack_file hhvm_compat_mode errors =
  let is_decimal_or_hexadecimal_literal token =
    match Token.kind token with
    | TokenKind.DecimalLiteral | TokenKind.HexadecimalLiteral -> true
    | _ -> false
  in
  match syntax node with
  | LiteralExpression { literal_expression = {syntax = Token token; _} as e ; _}
    when is_hack_file && is_decimal_or_hexadecimal_literal token ->
    let text = text e in
    begin try ignore (Int64.of_string text); errors
    with _ ->
      let error_text =
        if Token.kind token = TokenKind.DecimalLiteral
        then SyntaxError.error2071 text
        else SyntaxError.error2072 text in
      make_error_from_node node error_text :: errors
    end
  | SafeMemberSelectionExpression _ when not is_hack ->
    make_error_from_node node SyntaxError.error2069 :: errors
  | SubscriptExpression { subscript_left_bracket; _}
    when not hhvm_compat_mode && is_left_brace subscript_left_bracket ->
    make_error_from_node node SyntaxError.error2020 :: errors
  | HaltCompilerExpression { halt_compiler_argument_list = args; _ } ->
    let errors =
      if Core_list.is_empty (syntax_to_list_no_separators args) then errors
      else make_error_from_node node SyntaxError.no_args_in_halt_compiler :: errors in
    let errors =
      match parents with
      (* expression statement -> syntax list -> script *)
      | [_; _; _] -> errors
      | _ -> make_error_from_node node SyntaxError.halt_compiler_top_level_only :: errors in
    errors
  | FunctionCallExpression {
      function_call_argument_list = arg_list;
      function_call_receiver; _
    } ->
    let errors =
      match misplaced_variadic_arg arg_list with
      | Some h ->
        make_error_from_node h SyntaxError.error2033 :: errors
      | None -> errors
    in
    let arg_list = syntax_to_list_no_separators arg_list in
    let errors = Hh_core.List.fold_right arg_list ~init:errors
      ~f:(fun p acc -> function_call_argument_errors p acc)
    in
    let errors =
      function_call_on_xhp_name_errors function_call_receiver errors in
    errors
  | ConstructorCall ctr_call when not hhvm_compat_mode && is_hack ->
    if is_missing ctr_call.constructor_call_left_paren &&
        is_missing ctr_call.constructor_call_right_paren
    then
      let start_node = ctr_call.constructor_call_type in
      let end_node = ctr_call.constructor_call_type in
      let constructor_name = text ctr_call.constructor_call_type in
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
    if not is_hack then
      (* In php, vec[0] would be a subscript, where vec would be a constant *)
      match syntax_to_list_no_separators m with
      | _ :: _ :: _ -> (* 2 elements or more *)
        make_error_from_node node SyntaxError.list_as_subscript :: errors
      | _ -> errors
    else check_collection_members m errors
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
    let errors =
      if has_inout_params parents then
      let e =
        if is_inside_async_method parents
        then SyntaxError.inout_param_in_async_generator
        else SyntaxError.inout_param_in_generator in
      make_error_from_node ~error_type:SyntaxError.RuntimeError node e :: errors
      else errors in
    errors
  | ScopeResolutionExpression
    { scope_resolution_qualifier = qualifier
    ; scope_resolution_name = name
    ; _ } ->
      let is_dynamic_name, is_self_or_parent =
        match syntax qualifier, token_kind qualifier with
        | (LiteralExpression _ | QualifiedName _), _ -> false, false
        | _, Some TokenKind.Name
        | _, Some TokenKind.XHPClassName
        | _, Some TokenKind.Static -> false, false
        | _, Some TokenKind.Self
        | _, Some TokenKind.Parent -> false, true
        | _ -> true, false
      in
      let is_name_class = String.lowercase_ascii @@ text name = "class" in
      let errors = if is_dynamic_name && is_name_class then
        make_error_from_node
          node SyntaxError.coloncolonclass_on_dynamic :: errors
        else errors in
      let errors = if is_self_or_parent && is_name_class &&
          not @@ is_in_active_class_scope parents
        then make_error_from_node ~error_type:SyntaxError.RuntimeError
          node (SyntaxError.self_or_parent_colon_colon_class_outside_of_class
            @@ text qualifier) :: errors
        else errors in
      errors
  | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
    when token_kind prefix_unary_operator = Some TokenKind.Ampersand ->
    begin match syntax prefix_unary_operand with
      | ScopeResolutionExpression { scope_resolution_name; _}
        when token_kind scope_resolution_name = Some TokenKind.Name ->
        make_error_from_node node
          SyntaxError.reference_to_static_scope_resolution :: errors
      | _ -> errors
    end
  (* TODO(T21285960): Remove this bug-port, stemming from T22184312 *)
  | LambdaExpression { lambda_async; lambda_coroutine; lambda_signature; _ }
    when hhvm_compat_mode
      && not (is_missing lambda_async)
      && trailing_width lambda_async = 0
      && full_width lambda_coroutine = 0
      && leading_width lambda_signature = 0
      -> failwith "syntax error, unexpected T_LAMBDA_ARROW";
    (* End of bug-port *)
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

let check_type_name syntax_tree name namespace_name name_text location names errors =
  begin match SMap.get name_text names.t_classes with
  | Some { f_location = location; f_kind; f_name }
    when combine_names namespace_name name_text <> f_name && f_kind <> Name_def ->
    let text = SyntaxTree.text syntax_tree in
    let line_num, _ =
      Full_fidelity_source_text.offset_to_position
        text location.start_offset in
    let long_name_text = combine_names namespace_name name_text in
    let error =
      make_name_already_used_error name long_name_text name_text location
        (match f_kind with
          | Name_implicit_use -> SyntaxError.declared_name_is_already_in_use_implicit_hh ~line_num
          | Name_use -> SyntaxError.declared_name_is_already_in_use ~line_num
          | Name_def -> SyntaxError.type_name_is_already_in_use) in
    names, error :: errors
  | _ ->
    let def =
      make_first_use_or_def ~kind:Name_def location namespace_name name_text in
    let names =
      { names with
        t_classes = SMap.add name_text def names.t_classes} in
    names, errors
  end

let classish_errors node parents syntax_tree is_hack hhvm_compat_mode namespace_name names errors =
  match syntax node with
  | ClassishDeclaration cd ->
    (* Given a ClassishDeclaration node, test whether or not it contains
     * an invalid use of 'implements'. *)
    let classish_invalid_implements_keyword hhvm_compat_mode _ =
      (* Invalid if uses 'implements' and isn't a class. *)
      not hhvm_compat_mode &&
      token_kind cd.classish_implements_keyword = Some TokenKind.Implements &&
        token_kind cd.classish_keyword <> Some TokenKind.Class in

    (* Given a ClassishDeclaration node, test whether or not it's a trait
     * invoking the 'extends' keyword. *)
    let classish_invalid_extends_keyword hhvm_compat_mode _ =
      (* Invalid if uses 'extends' and is a trait. *)
      not hhvm_compat_mode &&
      token_kind cd.classish_extends_keyword = Some TokenKind.Extends &&
        token_kind cd.classish_keyword = Some TokenKind.Trait in

    (* Given a ClassishDeclaration node, test whether or not length of
     * extends_list is appropriate for the classish_keyword. *)
    let classish_invalid_extends_list  hhvm_compat_mode _ =
      (* Invalid if is a class and has list of length greater than one. *)
      not hhvm_compat_mode &&
      token_kind cd.classish_keyword = Some TokenKind.Class &&
        token_kind cd.classish_extends_keyword = Some TokenKind.Extends &&
        match syntax_to_list_no_separators cd.classish_extends_list with
        | [x1] -> false
        | _ -> true (* General bc empty list case is already caught by error1007 *) in

    let errors =
      produce_error errors
      classish_duplicate_modifiers cd.classish_modifiers
      SyntaxError.error2031 cd.classish_modifiers in
    let errors =
      produce_error errors
      (classish_invalid_implements_keyword hhvm_compat_mode) ()
      SyntaxError.error2035 cd.classish_implements_keyword in
    let errors =
      produce_error errors
      (classish_invalid_extends_keyword hhvm_compat_mode) ()
      SyntaxError.error2036 cd.classish_extends_keyword in
    let errors =
      produce_error errors (classish_invalid_extends_list hhvm_compat_mode) ()
      SyntaxError.error2037 cd.classish_extends_list in
    let errors =
      (* Extra setup for the the customized error message. *)
      let keyword_str = Option.value_map (token_kind cd.classish_keyword)
        ~default:"" ~f:TokenKind.to_string in
      let declared_name_str =
        Option.value ~default:"" (Syntax.extract_text cd.classish_name)
      in
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
    let errors =
      produce_error errors
      (is_reserved_keyword is_hack) cd.classish_name
      SyntaxError.reserved_keyword_as_class_name cd.classish_name in
    let name = text cd.classish_name in
    let errors =
      match syntax cd.classish_body with
      | ClassishBody {classish_body_elements = methods; _} ->
        let methods = syntax_to_list_no_separators methods in
        let has_abstract_fn =
          Hh_core.List.exists methods ~f:methodish_contains_abstract in
        let has_non_public_method =
          Hh_core.List.exists methods
            ~f:(methodish_modifier_contains_helper is_not_public_visibility) in
        let errors =
          if has_abstract_fn &&
             is_token_kind cd.classish_keyword TokenKind.Class &&
             not (list_contains_predicate is_abstract cd.classish_modifiers)
          then make_error_from_node node
                (SyntaxError.class_with_abstract_method name) :: errors
          else errors in
        let errors =
          if has_non_public_method &&
             is_token_kind cd.classish_keyword TokenKind.Interface
          then make_error_from_node node
            SyntaxError.interface_has_non_public_method :: errors
          else errors in
        errors
      | _ -> errors in
    let names, errors =
      match token_kind cd.classish_keyword with
      | Some TokenKind.Class | Some TokenKind.Trait
        when not (is_missing cd.classish_name)->
        let location = make_location_of_node cd.classish_name in
        check_type_name syntax_tree cd.classish_name namespace_name name location names errors
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

let alias_errors syntax_tree node namespace_name names errors =
  match syntax node with
  | AliasDeclaration ad ->
    let errors =
      if token_kind ad.alias_keyword = Some TokenKind.Type &&
        not (is_missing ad.alias_constraint)
      then make_error_from_node ad.alias_keyword SyntaxError.error2034 :: errors
      else errors in
    if is_missing ad.alias_name then names,errors
    else
    let name = text ad.alias_name in
    let location = make_location_of_node ad.alias_name in
    check_type_name syntax_tree ad.alias_name namespace_name name location names errors
  | _ -> names, errors

let is_invalid_group_use_clause kind clause =
  match syntax clause with
  | NamespaceUseClause { namespace_use_clause_kind = clause_kind; _ } ->
    if is_missing kind
    then
      begin match syntax clause_kind with
      | Missing -> false
      | Token token when let k = Token.kind token in
                         TokenKind.(k = Function || k = Const) -> false
      | _ -> true
      end
    else not (is_missing clause_kind)
  | _ -> false

let is_invalid_group_use_prefix prefix =
  not (is_namespace_prefix prefix)

let group_use_errors node errors =
  match syntax node with
  | NamespaceGroupUseDeclaration
    { namespace_group_use_prefix = prefix
    ; namespace_group_use_clauses = clauses
    ; namespace_group_use_kind = kind
    ; _} ->
      let errors =
        let invalid_clauses = List.filter (is_invalid_group_use_clause kind)
          (syntax_to_list_no_separators clauses) in
        let mapper errors clause =
          make_error_from_node clause SyntaxError.error2049 :: errors in
        List.fold_left mapper errors invalid_clauses in
      produce_error errors is_invalid_group_use_prefix prefix
        SyntaxError.error2048 prefix
  | _ -> errors

let use_class_or_namespace_clause_errors
  is_hack is_global_namespace namespace_prefix
  kind syntax_tree (names, errors) cl =

  match syntax cl with
  | NamespaceUseClause {
      namespace_use_name  = name;
      namespace_use_alias = alias; _
    } when not (is_missing name) ->
    let name_text = text name in
    let qualified_name =
      match namespace_prefix with
      | None -> combine_names global_namespace_name name_text
      | Some p -> combine_names p name_text in
    let short_name = get_short_name_from_qualified_name name_text (text alias) in

    let do_check ~error_on_global_redefinition names errors
      get_map update_map report_error =

      let map = get_map names in
      match SMap.get short_name map with
      | Some { f_location = location; f_kind; _ } ->
        if (f_kind <> Name_def
           || (error_on_global_redefinition && is_global_namespace))
        then
          let error =
            make_name_already_used_error name name_text
              short_name location report_error in
          names, error :: errors
        else
          names, errors
      | None ->
        let new_use =
          make_first_use_or_def
            ~kind:Name_use
            (make_location_of_node name)
            global_namespace_name qualified_name in
        update_map names (SMap.add short_name new_use map), errors in

    begin match syntax kind with
    | Token token ->
      let open TokenKind in
      (match Token.kind token with
      | Namespace ->
        do_check ~error_on_global_redefinition:false names errors
          (fun n -> n.t_namespaces)
          (fun n v -> { n with t_namespaces = v })
          SyntaxError.namespace_name_is_already_in_use

      | Type ->
        do_check ~error_on_global_redefinition:false names errors
          (fun n -> n.t_classes)
          (fun n v -> { n with t_classes = v })
          SyntaxError.type_name_is_already_in_use

      | Function ->
        do_check ~error_on_global_redefinition:true names errors
          (fun n -> n.t_functions)
          (fun n v -> { n with t_functions = v })
          SyntaxError.function_name_is_already_in_use

      | Const ->
        do_check ~error_on_global_redefinition:true names errors
          (fun n -> n.t_constants)
          (fun n v -> { n with t_constants = v })
          SyntaxError.const_name_is_already_in_use
      | _ ->
        names, errors
      )
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
        | Some { f_location = loc; f_name; f_kind; _ } ->
          if qualified_name = f_name && f_kind = Name_def then names, errors
          else
            let err_msg =
              if is_hack && f_kind <> Name_def then
                let text = SyntaxTree.text syntax_tree in
                let line_num, _ =
                  Full_fidelity_source_text.offset_to_position
                    text loc.start_offset in
                if f_kind = Name_implicit_use
                then SyntaxError.name_is_already_in_use_implicit_hh ~line_num
                else SyntaxError.name_is_already_in_use_hh ~line_num
              else SyntaxError.name_is_already_in_use_php
            in
            let error = make_name_already_used_error
              name name_text short_name loc err_msg in
            names, error :: errors
        | None ->
          let new_use =
            make_first_use_or_def ~kind:Name_use location global_namespace_name qualified_name in
          let t_classes = SMap.add short_name new_use names.t_classes in
          let t_namespaces =
            if SMap.mem short_name names.t_namespaces
            then names.t_namespaces
            else SMap.add short_name new_use names.t_namespaces in
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

let namespace_use_declaration_errors
  node is_hack is_global_namespace syntax_tree names errors =
  match syntax node with
  | NamespaceUseDeclaration {
      namespace_use_kind = kind;
      namespace_use_clauses = clauses; _ } ->
    let f =
      use_class_or_namespace_clause_errors
        is_hack is_global_namespace None kind syntax_tree in
    List.fold_left f (names, errors) (syntax_to_list_no_separators clauses)
  | NamespaceGroupUseDeclaration {
      namespace_group_use_kind = kind;
      namespace_group_use_clauses = clauses;
      namespace_group_use_prefix = prefix; _ } ->
    let f =
      use_class_or_namespace_clause_errors
        is_hack is_global_namespace (Some (text prefix)) kind syntax_tree in
    List.fold_left f (names, errors) (syntax_to_list_no_separators clauses)
  | _ -> names, errors

let rec check_constant_expression errors node =
  let is_namey token =
    match Token.kind token with
    TokenKind.Name -> true
    | _ -> false
  in
  let is_good_scope_resolution_qualifier node =
    match syntax node with
    | QualifiedName _ -> true
    | Token token ->
      let open TokenKind in
      (match Token.kind token with
      | XHPClassName | Name | Self | Parent | Static -> true
      | _ -> false
      )
    | _ -> false
  in
  let is_good_scope_resolution_name node =
    match syntax node with
    | QualifiedName _ -> true
    | Token token ->
      let open TokenKind in
      (match Token.kind token with
      | Name | Trait | Extends | Implements | Static
      | Abstract | Final | Private | Protected | Public | Or | And | Global
      | Goto | Instanceof | Insteadof | Interface | Namespace | New | Try | Use
      | Var | List | Clone | Include | Include_once | Throw | Array | Tuple
      | Print | Echo | Require | Require_once | Return | Else | Elseif | Default
      | Break | Continue | Switch | Yield | Function | If | Finally | For
      | Foreach | Case | Do | While | As | Catch | Empty | Using | Class
      | NullLiteral | Super | Where
        -> true
      | _ -> false
      )
    | _ -> false
  in
  match syntax node with
  | Missing
  | QualifiedName _
  | LiteralExpression _
    -> errors
  | Token token when is_namey token -> errors
  | PrefixUnaryExpression
    { prefix_unary_operand
    ; prefix_unary_operator = { syntax = Token token ; _ }
    } when ( let open TokenKind in
             match Token.kind token with
             | Exclamation | Plus | Minus | Tilde -> true
             | _ -> false
           ) ->
      check_constant_expression errors prefix_unary_operand
  | BinaryExpression
    { binary_left_operand
    ; binary_right_operand
    ; binary_operator = { syntax = Token token ; _ }
    ; _ } when ( let open TokenKind in
                 match Token.kind token with
                 | BarBar | AmpersandAmpersand | Carat | And | Or | Xor
                 | Bar | Ampersand | Dot | Plus | Minus | Star | Slash | Percent
                 | LessThanLessThan | GreaterThanGreaterThan | StarStar
                 | EqualEqual | EqualEqualEqual | ExclamationEqual
                 | ExclamationEqualEqual | GreaterThan | GreaterThanEqual
                 | LessThan | LessThanEqual | LessThanEqualGreaterThan
                 | QuestionColon
                   -> true
                 | _ -> false
               ) ->
    let errors = check_constant_expression errors binary_left_operand in
    let errors = check_constant_expression errors binary_right_operand in
    errors
  | ConditionalExpression {
      conditional_test;
      conditional_consequence;
      conditional_alternative; _
    } ->
    let errors = check_constant_expression errors conditional_test in
    let errors = check_constant_expression errors conditional_consequence in
    let errors = check_constant_expression errors conditional_alternative in
    errors
  | SimpleInitializer { simple_initializer_value = e; _ }
  | ParenthesizedExpression { parenthesized_expression_expression = e; _} ->
    check_constant_expression errors e
  | CollectionLiteralExpression
    { collection_literal_name =
      { syntax =
        ( SimpleTypeSpecifier
            { simple_type_specifier = { syntax = Token token; _ } }
        | GenericTypeSpecifier
            { generic_class_type = { syntax = Token token; _ }; _ }
        )
      ; _
      }
    ; collection_literal_initializers = lst
    ; _
    } when is_namey token ->
      syntax_to_list_no_separators lst
      |> Core_list.fold_left ~init:errors ~f:check_constant_expression
  | TupleExpression { tuple_expression_items = lst; _ }
  | KeysetIntrinsicExpression { keyset_intrinsic_members = lst; _}
  | VarrayIntrinsicExpression { varray_intrinsic_members = lst; _ }
  | DarrayIntrinsicExpression { darray_intrinsic_members = lst; _ }
  | VectorIntrinsicExpression { vector_intrinsic_members = lst; _ }
  | DictionaryIntrinsicExpression { dictionary_intrinsic_members = lst; _}
  | ArrayIntrinsicExpression { array_intrinsic_members = lst; _}
  | ArrayCreationExpression { array_creation_members = lst; _ }
  | ShapeExpression { shape_expression_fields = lst; _ } ->
    syntax_to_list_no_separators lst
    |> Core_list.fold_left ~init:errors ~f:check_constant_expression
  | ElementInitializer { element_key = n; element_value = v; _ }
  | FieldInitializer { field_initializer_name = n; field_initializer_value = v; _ } ->
    let errors = check_constant_expression errors n in
    let errors = check_constant_expression errors v in
    errors
  | ScopeResolutionExpression
    { scope_resolution_qualifier
    ; scope_resolution_name
    ; _ } when is_good_scope_resolution_qualifier scope_resolution_qualifier &&
               is_good_scope_resolution_name scope_resolution_name
      -> errors
  | _ ->
    (make_error_from_node node SyntaxError.invalid_constant_initializer) :: errors

let const_decl_errors node parents hhvm_compat_mode namespace_name names errors =
  match syntax node with
  | ConstantDeclarator cd ->
    let errors =
      produce_error_parents errors
      (concrete_no_initializer hhvm_compat_mode) cd.constant_declarator_initializer parents
      SyntaxError.error2050 cd.constant_declarator_initializer in
    let errors =
      produce_error_parents errors abstract_with_initializer cd.constant_declarator_initializer parents
      SyntaxError.error2051 cd.constant_declarator_initializer in
    let errors =
      produce_error errors is_global_in_const_decl cd.constant_declarator_initializer
      SyntaxError.global_in_const_decl cd.constant_declarator_initializer in
    let errors =
      check_constant_expression errors cd.constant_declarator_initializer in
    let errors =
      match syntax cd.constant_declarator_initializer with
      | SimpleInitializer { simple_initializer_value = { syntax =
          LiteralExpression { literal_expression = { syntax =
            SyntaxList _; _}}; _}; _} ->
            make_error_from_node
              node SyntaxError.invalid_constant_initializer :: errors
      | _ -> errors in
    if is_missing cd.constant_declarator_name
    then names, errors
    else
    let constant_name = text cd.constant_declarator_name in
    let location = make_location_of_node cd.constant_declarator_name in
    let def =
      make_first_use_or_def ~kind:Name_def location namespace_name constant_name in
    let errors =
      match SMap.get constant_name names.t_constants with
      | None -> errors
      | Some _ ->
        (* Only error if this is inside a class *)
        begin match first_parent_class_name parents with
          | None -> errors
          | Some class_name ->
            let full_name = class_name ^ "::" ^ constant_name in
            make_error_from_node
              node (SyntaxError.redeclation_of_const full_name) :: errors
        end
    in
    let names = {
      names with t_constants =
        SMap.add constant_name def names.t_constants } in
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

let mixed_namespace_errors node parents systemlib_compat namespace_type errors =
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
  | NamespaceDeclaration { namespace_body; _ } ->
    let is_first_decl, has_code_outside_namespace =
      match parents with
      | [{ syntax = SyntaxList _; _} as decls; { syntax = Script _; _}] ->
        let decls = syntax_to_list_no_separators decls in
        let decls = if not systemlib_compat then decls else
          (* Drop everything before yourself *)
          fst @@ Hh_core.List.fold_right decls
            ~init:([], false)
            ~f:(fun n (l, seen as acc) ->
              if seen then acc else (n::l, n == node))
        in
        let rec is_first l =
          match l with
          | { syntax = MarkupSection {markup_text; _}; _} :: rest
            when width markup_text = 0 || is_hashbang markup_text ->
            is_first rest
          | { syntax = DeclareDirectiveStatement _; _} :: rest
          | { syntax = DeclareBlockStatement _; _} :: rest
          | { syntax = NamespaceUseDeclaration _; _} :: rest -> is_first rest
          | { syntax = NamespaceDeclaration _; _} :: _ -> true
          | _ -> false
        in
        let has_code_outside_namespace =
          not (is_namespace_empty_body namespace_body) &&
          Hh_core.List.exists decls
            ~f:(function | { syntax = MarkupSection { markup_text; _}; _}
                           when width markup_text = 0
                             || is_hashbang markup_text -> false
                         | { syntax = NamespaceDeclaration _; _}
                         | { syntax = DeclareDirectiveStatement _; _}
                         | { syntax = DeclareBlockStatement _; _}
                         | { syntax = ExpressionStatement {
                            expression_statement_expression =
                            { syntax = HaltCompilerExpression _; _}; _}; _}
                         | { syntax = EndOfFile _; _}
                         | { syntax = NamespaceUseDeclaration _; _} -> false
                         | _ -> true)
        in
        is_first decls, has_code_outside_namespace
      | _ -> true, false
    in
    let errors = if not is_first_decl then
      make_error_from_node node
        SyntaxError.namespace_decl_first_statement :: errors else errors
    in
    let errors = if has_code_outside_namespace then
      make_error_from_node node
        SyntaxError.code_outside_namespace :: errors else errors
    in
    errors
  | _ -> errors

let enum_errors node errors =
  match syntax node with
  | Enumerator { enumerator_name = name; _}
      when String.lowercase_ascii @@ text name = "class" ->
    make_error_from_node node SyntaxError.enum_elem_name_is_class :: errors
  | _ -> errors

let does_op_create_write_on_left = function
  | Some (TokenKind.Equal
        | TokenKind.BarEqual
        | TokenKind.PlusEqual
        | TokenKind.StarEqual
        | TokenKind.StarStarEqual
        | TokenKind.SlashEqual
        | TokenKind.DotEqual
        | TokenKind.MinusEqual
        | TokenKind.PercentEqual
        | TokenKind.CaratEqual
        | TokenKind.AmpersandEqual
        | TokenKind.LessThanLessThanEqual
        | TokenKind.GreaterThanGreaterThanEqual) -> true
  | _ -> false

let assignment_errors node errors =
  match syntax node with
  | BinaryExpression
    { binary_left_operand = loperand
    ; binary_operator = op
    ; binary_right_operand = roperand
    } when does_op_create_write_on_left (token_kind op) ->
    let result = match syntax loperand with
      | SafeMemberSelectionExpression _ ->
        Some SyntaxError.safe_member_selection_in_write
      | MemberSelectionExpression { member_name; _ }
        when token_kind member_name = Some TokenKind.XHPClassName ->
        Some SyntaxError.safe_member_selection_in_write
      | VariableExpression { variable_expression }
        when String.lowercase_ascii (text variable_expression)
          = SN.SpecialIdents.this ->
        Some SyntaxError.reassign_this
      | _ -> None
    in
    begin match result with
    | None -> errors
    | Some error_message ->
      make_error_from_node loperand error_message :: errors
    end
  | _ -> errors

let trait_use_alias_item_errors node errors =
  let is_public_private_protected_or_final token =
    let open TokenKind in
    match Token.kind token with
    | Public | Private | Protected | Final -> true
    | _ -> false
  in
  match syntax node with
  | TraitUseAliasItem { trait_use_alias_item_modifiers = l; _ } ->
    syntax_to_list_no_separators l
    |> Core_list.fold_left ~init:errors ~f:(fun errors n ->
      match syntax n with
      | Token token when is_public_private_protected_or_final token -> errors
      | _ ->
        let e =
          make_error_from_node n
            SyntaxError.trait_alias_rule_allows_only_final_and_visibility_modifiers in
        e :: errors)
  | _ -> errors

let declare_errors node parents hhvm_compat_mode errors =
  match syntax node with
  | DeclareDirectiveStatement { declare_directive_expression = expr; _}
  | DeclareBlockStatement { declare_block_expression = expr; _} ->
    let errors =
      match syntax expr with
      | BinaryExpression
        { binary_left_operand = loper
        ; binary_operator = op
        ; _} when token_kind op = Some TokenKind.Equal
             && String.lowercase_ascii @@ text loper = "strict_types" ->
        (* Checks if there are only other declares nodes
         * in front of the node in question *)
        let rec is_only_declares_nodes = function
          | ({ syntax = DeclareDirectiveStatement _; _} as e) :: es
          | ({ syntax = DeclareBlockStatement _; _} as e) :: es ->
            e == node || is_only_declares_nodes es
          | _ -> false
        in
        let errors =
          match parents with
          | [{ syntax = SyntaxList (
               { syntax = MarkupSection {markup_text; _}; _} :: items); _} ; _]
            when width markup_text = 0 && is_only_declares_nodes items ->
            errors
          | _ ->
            make_error_from_node
              node SyntaxError.strict_types_first_statement :: errors
        in
        let errors =
          match syntax node with
          | DeclareBlockStatement _ when not hhvm_compat_mode ->
            make_error_from_node ~error_type:SyntaxError.RuntimeError
              node SyntaxError.strict_types_in_declare_block_mode :: errors
          | _ -> errors
        in
        errors
      | _ -> errors
    in
    errors
  | _ -> errors

let get_namespace_name parents current_namespace_name =
  match parents with
  | { syntax = NamespaceDeclaration { namespace_name = ns; _ }; _ } :: _ ->
    if is_missing ns then current_namespace_name
    else combine_names current_namespace_name (text ns)
  | _ -> current_namespace_name

let find_syntax_errors ~enable_hh_syntax error_level syntax_tree =
  let hhvm_compatibility_mode = is_hhvm_compat error_level in
  let systemlib_compat = is_systemlib_compat error_level in
  let is_strict = SyntaxTree.is_strict syntax_tree in
  let is_hack_file = SyntaxTree.is_hack syntax_tree in
  let is_hack = is_hack_file || enable_hh_syntax in
  let rec folder acc node parents =
    let { errors
        ; namespace_type
        ; names
        ; namespace_name
        ; trait_require_clauses
        } = acc in
    let errors =
      markup_errors node is_hack_file hhvm_compatibility_mode errors in
    let names, errors =
      parameter_errors node parents is_strict is_hack hhvm_compatibility_mode namespace_name names errors in
    let errors =
      function_errors node parents is_strict hhvm_compatibility_mode errors in
    let names, errors =
      redeclaration_errors node parents syntax_tree namespace_name names errors in
    let errors =
      xhp_errors node parents hhvm_compatibility_mode errors in
    let errors =
      statement_errors node parents hhvm_compatibility_mode errors in
    let errors =
      methodish_errors node parents is_hack hhvm_compatibility_mode errors in
    let errors =
      property_errors node is_strict is_hack hhvm_compatibility_mode errors in
    let errors =
      expression_errors node parents is_hack is_hack_file hhvm_compatibility_mode errors in
    let trait_require_clauses, errors =
      require_errors node parents hhvm_compatibility_mode trait_require_clauses errors in
    let names, errors =
      classish_errors node parents syntax_tree is_hack hhvm_compatibility_mode namespace_name names errors in
    let errors =
      class_element_errors node parents errors in
    let errors =
      type_errors node parents is_strict hhvm_compatibility_mode errors in
    let names, errors = alias_errors syntax_tree node namespace_name names errors in
    let errors = group_use_errors node errors in
    let names, errors =
      const_decl_errors node parents hhvm_compatibility_mode namespace_name names errors in
    let errors =
      abstract_final_class_nonstatic_var_error node parents hhvm_compatibility_mode errors in
    let errors =
      abstract_final_class_nonstatic_method_error node parents hhvm_compatibility_mode errors in
    let errors =
      mixed_namespace_errors node parents systemlib_compat namespace_type errors in
    let names, errors =
      namespace_use_declaration_errors node is_hack
        (namespace_name = global_namespace_name) syntax_tree names errors in
    let errors = enum_errors node errors in
    let errors = assignment_errors node errors in
    let errors = declare_errors node parents hhvm_compatibility_mode errors in
    let errors = trait_use_alias_item_errors node errors in

    match syntax node with
    | NamespaceBody { namespace_left_brace; namespace_right_brace; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Bracketed (make_location namespace_left_brace namespace_right_brace)
        else namespace_type in
      (* reset names/namespace_type before diving into namespace body *)
      let namespace_name = get_namespace_name parents namespace_name in
      let acc1 =
        make_acc
          acc errors namespace_type empty_names
          namespace_name empty_trait_require_clauses
      in
      let acc1 = fold_child_nodes folder node parents acc1 in
      (* resume with old set of names and pull back
        accumulated errors/last seen namespace type *)
        make_acc
          acc acc1.errors namespace_type acc.names
          acc.namespace_name acc.trait_require_clauses
    | NamespaceEmptyBody { namespace_semicolon; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Unbracketed (make_location_of_node namespace_semicolon)
        else namespace_type
      in
      let namespace_name = get_namespace_name parents namespace_name in
      (* consider the rest of file to be the part of the namespace:
         reset names and namespace type, keep errors *)
      let acc =
        make_acc
          acc errors namespace_type empty_names
          namespace_name empty_trait_require_clauses
      in
      fold_child_nodes folder node parents acc
    | ClassishDeclaration _
    | AnonymousClass _ ->
      (* Reset the trait require clauses *)
      (* Reset the const declarations *)
      (* Reset the function declarations *)
      let cleanup =
        fun new_acc ->
          { new_acc with names =
            { new_acc.names with t_constants = acc.names.t_constants;
                                 t_functions = acc.names.t_functions }} in
      let names = { names with t_constants = SMap.empty;
                               t_functions = SMap.empty } in
      let acc =
        make_acc
          acc errors namespace_type names
          namespace_name empty_trait_require_clauses
      in
      fold_child_nodes ~cleanup folder node parents acc
    | _ ->
      let acc =
        make_acc
          acc errors namespace_type names
          namespace_name trait_require_clauses
      in
      fold_child_nodes folder node parents acc in
  let acc = fold_child_nodes folder (SyntaxTree.root syntax_tree) []
    { errors = []
    ; namespace_type = Unspecified
    ; names = empty_names
    ; namespace_name = global_namespace_name
    ; trait_require_clauses = empty_trait_require_clauses
    } in
  acc.errors

let parse_errors_impl ?(enable_hh_syntax=false) ?(level=Typical) syntax_tree =
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
    else find_syntax_errors ~enable_hh_syntax level syntax_tree in
  List.sort SyntaxError.compare (Core_list.append errors1 errors2)

let parse_errors ?(enable_hh_syntax=false) ?(level=Typical) syntax_tree =
  Stats_container.wrap_nullary_fn_timing
    ?stats:(Stats_container.get_instance ())
    ~key:"full_fidelity_parse_errors:parse_errors"
    ~f:(fun () -> parse_errors_impl ~enable_hh_syntax ~level syntax_tree)
end (* WithSyntax *)

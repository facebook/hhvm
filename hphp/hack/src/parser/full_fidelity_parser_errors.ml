(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module WithSmartConstructors(SCI : SmartConstructors.SmartConstructors_S
  with type r = Syntax.t
  with module Token = Syntax.Token
) = struct

open Syntax

module SyntaxTree_ = Full_fidelity_syntax_tree
  .WithSyntax(Syntax)
module SyntaxTree = SyntaxTree_.WithSmartConstructors(SCI)

module Token = Syntax.Token
module SyntaxError = Full_fidelity_syntax_error
module SyntaxKind = Full_fidelity_syntax_kind
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
  f_global: bool;
}

type error_level = Minimum | Typical | Maximum

type hhvm_compat_mode = NoCompat | HHVMCompat | SystemLibCompat

type context =
  { active_classish           : Syntax.t option
  ; active_methodish          : Syntax.t option
  ; active_callable           : Syntax.t option
  ; active_callable_attr_spec : Syntax.t option
  (* true if active callable is reactive if it is a function or method, or there is a reactive
   * proper ancestor (including lambdas) but not beyond the enclosing function or method *)
  ; active_is_rx_or_enclosing_for_lambdas : bool
  ; active_const              : Syntax.t option
  (* Named (not anonymous) namespaces that the current expression is enclosed within. *)
  ; nested_namespaces         : Syntax.t list
}

type env =
  { syntax_tree          : SyntaxTree.t
  ; level                : error_level
  ; hhvm_compat_mode     : hhvm_compat_mode
  ; enable_hh_syntax     : bool
  ; is_hh_file           : bool
  ; is_strict            : bool
  ; codegen              : bool
  ; hhi_mode             : bool
  ; parser_options       : ParserOptions.t
  ; context              : context
  }

let make_env
  ?(level                = Typical         )
  ?(hhvm_compat_mode     = NoCompat        )
  ?(enable_hh_syntax     = false           )
  ?(hhi_mode             = false           )
  ~(parser_options : ParserOptions.t)
  (syntax_tree : SyntaxTree.t)
  ~(codegen : bool)
  : env
  =
    let context =
      { active_classish = None
      ; active_methodish = None
      ; active_callable = None
      ; active_callable_attr_spec = None
      ; active_is_rx_or_enclosing_for_lambdas = false
      ; active_const = None
      ; nested_namespaces = []
      } in
    { syntax_tree
    ; level
    ; hhvm_compat_mode
    ; enable_hh_syntax
    ; is_hh_file = SyntaxTree.is_hack syntax_tree
    ; is_strict = SyntaxTree.is_strict syntax_tree
    ; codegen
    ; hhi_mode
    ; parser_options
    ; context
    }

and is_hhvm_compat env = env.hhvm_compat_mode <> NoCompat

and is_systemlib_compat env = env.hhvm_compat_mode = SystemLibCompat

and is_hack env = env.is_hh_file || env.enable_hh_syntax

let is_hh_file env = env.is_hh_file

let is_typechecker env =
  is_hack env && (not env.codegen)

let is_strict env =
  match SyntaxTree.mode env.syntax_tree with
  | Some mode -> FileInfo.is_strict mode
  | None -> false

let global_namespace_name = "\\"

let combine_names n1 n2 =
  let has_leading_slash = String.length n2 > 0 && String.get n2 0 = '\\' in
  let len = String.length n1 in
  let has_trailing_slash = String.get n1 (len - 1) = '\\' in
  match has_leading_slash, has_trailing_slash with
  | true, true -> n1 ^ (String.sub n2 1 (String.length n2 - 1))
  | false, false -> n1 ^ "\\" ^ n2
  | _ -> n1 ^ n2

let make_first_use_or_def ~kind ?(is_method=false) location namespace_name name =
  {
    f_location = location;
    f_kind = kind;
    f_name = combine_names namespace_name name;
    f_global = not is_method && namespace_name = global_namespace_name;
  }

type 'a strmap =
  | YesCase of 'a SMap.t
  | NoCase of 'a LSMap.t


type used_names = {
  t_classes: first_use_or_def strmap;    (* NoCase *)
  t_namespaces: first_use_or_def strmap; (* NoCase *)
  t_functions: first_use_or_def strmap;  (* NoCase *)
  t_constants: first_use_or_def strmap;  (* YesCase *)
}

let empty_names = {
  t_classes = NoCase LSMap.empty;
  t_namespaces = NoCase LSMap.empty;
  t_functions = NoCase LSMap.empty;
  t_constants = YesCase SMap.empty;
}

let strmap_mem : string -> 'a strmap -> bool =
  fun k m ->
  begin match m with
  | NoCase m -> LSMap.mem k m
  | YesCase m -> SMap.mem k m
  end

let strmap_add : string -> 'a -> 'a strmap -> 'a strmap =
  fun k v m ->
  begin match m with
  | NoCase m' -> NoCase (LSMap.add k v m')
  | YesCase m' -> YesCase (SMap.add k v m')
  end

let strmap_get : string -> 'a strmap -> 'a option =
  fun k m ->
  begin match m with
  | NoCase m' -> LSMap.get k m'
  | YesCase m' -> SMap.get k m'
  end

let strmap_find_first_opt : (string -> bool) -> 'a strmap -> (string * 'a) option =
  fun k m ->
  begin match m with
  | NoCase m' -> LSMap.find_first_opt k m'
  | YesCase m' -> SMap.find_first_opt k m'
  end

let strmap_filter : (string -> 'a -> bool) -> 'a strmap -> 'a strmap =
  fun f m ->
  begin match m with
  | NoCase m' -> NoCase (LSMap.filter f m')
  | YesCase m' -> YesCase (SMap.filter f m')
  end

let strmap_union : 'a strmap -> 'a strmap -> 'a strmap =
  fun x y ->
  begin match x, y with
  | NoCase x', NoCase y' -> NoCase (LSMap.union x' y')
  | YesCase x', YesCase y' -> YesCase (SMap.union x' y')
  | NoCase _, YesCase _ -> failwith "cannot union NoCase and YesCase."
  | YesCase _, NoCase _ -> failwith "cannot union YesCase and NoCase."
  end

let empty_trait_require_clauses = NoCase LSMap.empty

let get_short_name_from_qualified_name name alias =
  if String.length alias <> 0 then alias
  else match String.rindex name '\\' with
    | Some i -> String.sub name (i + 1) (String.length name - i - 1)
    | None -> name

type accumulator = {
  errors : SyntaxError.t list;
  namespace_type : namespace_type;
  namespace_name: string;
  names: used_names;
  trait_require_clauses: TokenKind.t strmap;
  is_in_concurrent_block: bool;
}

let make_acc
  acc errors namespace_type names namespace_name trait_require_clauses
  is_in_concurrent_block =
  if phys_equal acc.errors errors &&
     phys_equal acc.namespace_type namespace_type &&
     phys_equal acc.names names &&
     phys_equal acc.namespace_name namespace_name &&
     phys_equal acc.trait_require_clauses trait_require_clauses &&
     acc.is_in_concurrent_block = is_in_concurrent_block
  then acc
  else { errors
       ; namespace_type
       ; names
       ; namespace_name
       ; trait_require_clauses
       ; is_in_concurrent_block
       }

let fold_child_nodes ?(cleanup = (fun x -> x)) env f node parents acc =
  Syntax.children node
  |> List.fold_left ~init:acc ~f:(fun acc c -> f env acc c (node :: parents))
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

(* test a node is a syntaxlist and that the list contains an element
 * satisfying a given predicate *)
let list_contains_predicate p node =
  match syntax node with
  | SyntaxList lst ->
    List.exists ~f:p lst
  | _ -> false

(* test a node is a syntaxlist and that the list contains multiple elements
 * satisfying a given predicate *)
let list_contains_multiple_predicate p node =
  match syntax node with
  | SyntaxList lst ->
    let count_fun acc el = if p el then acc + 1 else acc in
    (List.fold_left ~f:count_fun ~init:0 lst) > 1
  | _ -> false

let list_contains_duplicate node =
  let module SyntaxMap = Caml.Map.Make (
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
    let (_, result) = List.fold_left ~f:check_fun ~init:(SyntaxMap.empty, false) lst in
    result
  | _ ->  false

let is_empty_list_or_missing node =
  match syntax node with
  | SyntaxList [] | Missing -> true
  | _ -> false

let token_kind node =
  match syntax node with
  | Token t -> Some (Token.kind t)
  | _ -> None

(* Helper function for common code pattern *)
let is_token_kind node kind =
  (token_kind node) = Some kind

let active_classish_kind context =
  Option.value_map context.active_classish ~default:None ~f:(function
    | { syntax = ClassishDeclaration cd; _ } -> token_kind cd.classish_keyword
    | _ -> None)

let modifiers_of_function_decl_header_exn node =
  match syntax node with
  | FunctionDeclarationHeader { function_modifiers = m; _ } -> m
  | _ -> failwith "expected to get FunctionDeclarationHeader"

let get_modifiers_of_declaration node =
  match syntax node with
  | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
    Some (modifiers_of_function_decl_header_exn header)
  | PropertyDeclaration { property_modifiers; _} ->
    Some (property_modifiers)
  | _ -> None

(* tests whether the methodish contains a modifier that satisfies [p] *)
let methodish_modifier_contains_helper p node =
  get_modifiers_of_declaration node
    |> Option.exists ~f:(list_contains_predicate p)

(* tests whether the methodish contains > 1 modifier that satisfies [p] *)
let methodish_modifier_multiple_helper p node =
  get_modifiers_of_declaration node
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

let contains_async_not_last mods =
  let mod_list = syntax_to_list_no_separators mods in
  List.exists ~f:is_async mod_list
    && not @@ is_async @@ List.nth_exn mod_list (List.length mod_list - 1)

let has_static node context f =
  match node with
  | FunctionDeclarationHeader node ->
    let label = node.function_name in
    (f label) &&
      context.active_methodish |> Option.exists ~f:methodish_contains_static
  | _ -> false

(* checks if a methodish decl or property has multiple visibility modifiers *)
let declaration_multiple_visibility node =
  methodish_modifier_multiple_helper is_visibility node

(* Given a function declaration header, confirm that it is a constructor
 * and that the methodish containing it has a static keyword *)

let is_clone label =
  String.lowercase (text label) = SN.SpecialFunctions.clone

let class_constructor_has_static node context =
  has_static node context is_construct

let class_destructor_cannot_be_static node context =
  has_static node context is_destruct

let clone_cannot_be_static node context =
  has_static node context is_clone

(* Given a function declaration header, confirm that it is NOT a constructor
 * and that the header containing it has visibility modifiers in parameters
 *)
let class_non_constructor_has_visibility_param node _context =
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
    (not (is_construct label)) && (List.exists ~f:has_visibility params)
  | _ -> false

(* check that a constructor or a destructor is type annotated *)
let class_constructor_destructor_has_non_void_type env node _context =
  if not (is_typechecker env) then false
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
let async_magic_method node _context =
  match node with
  | FunctionDeclarationHeader node ->
    let name = String.lowercase @@ text node.function_name in
    begin match name with
    | _ when name = String.lowercase SN.Members.__disposeAsync -> false
    | _ when SSet.mem name SN.Members.as_lowercase_set ->
      list_contains_predicate is_async node.function_modifiers
    | _ -> false
    end
  | _ -> false


let clone_destruct_takes_no_arguments _method_name node _context =
  match node with
  | FunctionDeclarationHeader { function_parameter_list = l; function_name = name; _} ->
    let num_params = List.length (syntax_to_list_no_separators l) in
    ((is_clone name) || (is_destruct name)) && num_params <> 0
  | _ -> false

(* whether a methodish has duplicate modifiers *)
let methodish_duplicate_modifier node =
  get_modifiers_of_declaration node
    |> Option.value_map ~default:false ~f:list_contains_duplicate

(* whether a methodish decl has body *)
let methodish_has_body node =
  match syntax node with
  | MethodishDeclaration syntax ->
    let body = syntax.methodish_function_body in
    not (is_missing body)
  | _ -> false

(* whether a methodish decl is native *)
let methodish_is_native node =
  match syntax node with
  | MethodishDeclaration { methodish_attribute = {
      syntax = AttributeSpecification {
        attribute_specification_attributes = attrs; _}; _}; _} ->
    let attrs = syntax_to_list_no_separators attrs in
    List.exists attrs
      ~f:(function { syntax = ConstructorCall {constructor_call_type; _}; _} ->
            String.lowercase @@ text constructor_call_type = "__native"
          | _ -> false)
  | _ -> false

(* By checking the third parent of a methodish node, tests whether the methodish
 * node is inside an interface. *)
let methodish_inside_interface context =
  context.active_classish |> Option.exists ~f:(fun parent_classish ->
    match syntax parent_classish with
    | ClassishDeclaration cd when token_kind cd.classish_keyword = Some TokenKind.Interface -> true
    | _ -> false
    )
(* Test whether node is a non-abstract method without a body and not native.
 * Here node is the methodish node
 * And methods inside interfaces are inherently considered abstract *)
let methodish_non_abstract_without_body_not_native env node _ =
  let non_abstract = not (methodish_contains_abstract node
      || methodish_inside_interface env.context) in
  let not_has_body = not (methodish_has_body node) in
  let not_native = not (methodish_is_native node) in
  let not_hhi  = not (env.hhi_mode) in
  not_hhi && non_abstract && not_has_body && not_native

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

let methodish_abstract_inside_interface context node =
  let is_abstract = methodish_contains_abstract node in
  let is_in_interface = methodish_inside_interface context in
  is_abstract && is_in_interface

let using_statement_function_scoped_is_legal parents =
  match parents with
  (* using is allowed in the toplevel, and also in toplevel async blocks *)
  | _ ::
    { syntax = CompoundStatement _; _ } ::
    { syntax = (
        FunctionDeclaration  _ |
        MethodishDeclaration _ |
        AnonymousFunction    _ |
        LambdaExpression     _ |
        AwaitableCreationExpression _); _ } :: _ -> true
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

let xhp_errors env node errors =
  match syntax node with
  | XHPEnumType enumType when
  (is_typechecker env) &&
  (is_missing enumType.xhp_enum_values) ->
    make_error_from_node enumType.xhp_enum_values SyntaxError.error2055 :: errors
  | XHPEnumType enumType when
    (is_typechecker env) ->
    let invalid_enum_items = List.filter ~f:is_invalid_xhp_attr_enum_item
      (syntax_to_list_no_separators enumType.xhp_enum_values) in
    let mapper errors item =
      make_error_from_node item SyntaxError.error2063 :: errors in
    List.fold_left ~f:mapper ~init:errors invalid_enum_items
  | XHPExpression
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

let produce_error_context acc check node context error error_node =
  if check node context then
    (make_error_from_node error_node error) :: acc
  else acc

let produce_error_for_header acc check node error error_node =
  (* use [check] to check properties of function *)
  let function_header_check_helper check node = check (syntax node) in
  produce_error_context acc (function_header_check_helper check) node
    error error_node


let cant_be_classish_name env name =
  match String.lowercase name with
  | "false" | "null" | "parent" | "self" | "true" ->
    true
  | "callable" | "classname" | "darray" | "this" | "varray"
    when is_hack env -> true
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

(* Return, as a string opt, the name of the function or method given the context *)
let first_parent_function_name context =
  (* Note: matching on either is sound because functions and/or methods cannot be nested *)
  match context.active_methodish with
  | Some { syntax = FunctionDeclaration { function_declaration_header = header; _ }; _ }
  | Some { syntax = MethodishDeclaration { methodish_function_decl_header = header; _ }; _ }
    -> extract_function_name header
  | _ -> None

(* Given a particular TokenKind.(Trait/Interface), tests if a given
 * classish_declaration node is both of that kind and declared abstract. *)
let is_classish_kind_declared_abstract env cd_node =
  if not (is_hack env) then false
  else
  match syntax cd_node with
  | ClassishDeclaration { classish_keyword; classish_modifiers; _ }
    when is_token_kind classish_keyword TokenKind.Trait
      || is_token_kind classish_keyword TokenKind.Interface ->
      list_contains_predicate is_abstract classish_modifiers
  | _ -> false

let is_immediately_in_lambda context =
  Option.value_map context.active_callable ~default:false ~f:(fun node ->
    match syntax node with
    | AnonymousFunction _
    | Php7AnonymousFunction _
    | LambdaExpression _
    | AwaitableCreationExpression _ -> true
    | _ -> false
    )

(* Returns the whether the current context is in an active class scope *)
let is_in_active_class_scope context = Option.is_some context.active_classish

(* Returns the first ClassishDeclaration node in the given context,
 * or None if there isn't one or classish_kind does not match. *)
let first_parent_classish_node classish_kind context =
  Option.value_map context.active_classish ~default:None ~f:(function
  { syntax = ClassishDeclaration cd; _ } as node ->
    Some (node, is_token_kind cd.classish_keyword classish_kind)
  | _ -> None
  ) |> Option.value_map ~default:None
         ~f:(function (n, true) -> Some n | _ -> None)

(* Return, as a string opt, the name of the closest enclosing classish entity in
  the given context (not just Classes ) *)
let active_classish_name context =
  context.active_classish |> Option.value_map ~default:None ~f:(fun node ->
    match syntax node with
    | ClassishDeclaration cd -> Syntax.extract_text cd.classish_name
    | _ -> None
    )

(* Return, as a string opt, the name of the Class in the given context *)
let first_parent_class_name context =
  context.active_classish |> Option.value_map ~default:None ~f:(fun parent_classish ->
    match syntax parent_classish with
    | ClassishDeclaration cd when token_kind cd.classish_keyword = Some TokenKind.Class ->
      active_classish_name context
    | _ -> None (* This arm is never reached  *)
    )

(* Given a classish_ or methodish_ declaration node, returns the modifier node
   from its list of modifiers, or None if there isn't one. *)
let extract_keyword modifier declaration_node =
  let aux modifiers_list =
    List.find ~f:modifier (syntax_to_list_no_separators modifiers_list)
  in

  match syntax declaration_node with
  | ClassishDeclaration { classish_modifiers = modifiers_list ; _ } ->
    aux modifiers_list
  | _ ->
    Option.bind (get_modifiers_of_declaration declaration_node) aux

(* Wrapper function that uses above extract_keyword function to test if node
   contains is_abstract keyword *)
let is_abstract_declaration declaration_node =
  not (Option.is_none (extract_keyword is_abstract declaration_node))

(* Given a context, tests if the immediate classish parent is an
 * interface. *)
let is_inside_interface context =
  Option.is_some (first_parent_classish_node TokenKind.Interface context)

(* Given a context, tests if the immediate classish parent is a
 * trait. *)
let is_inside_trait context =
  Option.is_some (first_parent_classish_node TokenKind.Trait context)

(* Tests if md_node is either explicitly declared abstract or is
 * defined inside an interface *)
let is_generalized_abstract_method md_node context =
  is_abstract_declaration md_node || is_inside_interface context

(* Returns the 'async'-annotation syntax node from the methodish_declaration
 * node. The returned node may have syntax kind 'Missing', but it will only be
 * None if something other than a methodish_declaration node was provided as
 * input. *)
let extract_async_node md_node =
  get_modifiers_of_declaration md_node
  |> Option.value_map ~default:[] ~f:syntax_to_list_no_separators
  |> List.find ~f:is_async

let get_params_for_enclosing_callable context =
  context.active_callable |> Option.bind ~f:(fun callable ->
    match syntax callable with
    | FunctionDeclaration { function_declaration_header = header; _ }
    | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
      begin match syntax header with
      | FunctionDeclarationHeader fdh ->
        Some fdh.function_parameter_list
      | _ -> None
      end
    | LambdaExpression { lambda_signature; _} ->
      begin match syntax lambda_signature with
      | LambdaSignature { lambda_parameters; _ } ->
        Some lambda_parameters
      | _ -> None
      end
    | _ -> None
    )

let first_parent_function_attributes_contains context name =
  match context.active_methodish with
  | Some { syntax = FunctionDeclaration { function_attribute_spec = {
      syntax = AttributeSpecification {
        attribute_specification_attributes; _ }; _ }; _ }; _ }
  | Some { syntax = MethodishDeclaration { methodish_attribute = {
      syntax = AttributeSpecification {
        attribute_specification_attributes; _ }; _ }; _ }; _ }
    ->
      let attrs =
        syntax_to_list_no_separators attribute_specification_attributes in
      List.exists attrs
        ~f:(function { syntax = ConstructorCall { constructor_call_type; _}; _} ->
          text constructor_call_type = name | _ -> false)
  | _ -> false
let is_parameter_with_callconv param =
  match syntax param with
  | ParameterDeclaration { parameter_call_convention; _ } ->
    not @@ is_missing parameter_call_convention
  | ClosureParameterTypeSpecifier { closure_parameter_call_convention; _ } ->
    not @@ is_missing closure_parameter_call_convention
  | VariadicParameter { variadic_parameter_call_convention; _ } ->
    not @@ is_missing variadic_parameter_call_convention
  | _ -> false

let has_inout_params context =
  match get_params_for_enclosing_callable context with
  | Some function_parameter_list ->
    let params = syntax_to_list_no_separators function_parameter_list in
    List.exists params ~f:is_parameter_with_callconv
  | _ -> false

let is_inside_async_method context =
  Option.value_map context.active_callable ~f:(fun node ->
    match syntax node with
    | FunctionDeclaration { function_declaration_header = header; _ }
    | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
      begin match syntax header with
      | FunctionDeclarationHeader fdh ->
        List.exists ~f:is_async @@
          syntax_to_list_no_separators fdh.function_modifiers
      | _ -> false
      end
    | AnonymousFunction { anonymous_async_keyword; _ } ->
      not @@ is_missing anonymous_async_keyword
    | Php7AnonymousFunction { php7_anonymous_async_keyword; _ } ->
      not @@ is_missing php7_anonymous_async_keyword
    | LambdaExpression { lambda_async; _ } ->
      not @@ is_missing lambda_async
    | AwaitableCreationExpression _ -> true
    | _ -> false
    ) ~default:false

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

let check_type_name_reference env name_text location names errors =
  if not (is_hack env && Hh_autoimport.is_hh_autoimport name_text)
    || strmap_mem name_text names.t_classes
  then names, errors
  else
    let def = make_first_use_or_def ~kind:Name_implicit_use location "HH" name_text in
    let names = { names with t_classes = strmap_add name_text def names.t_classes} in
    names, errors

let check_type_hint env node names errors =
  let rec check (names, errors) node =
    let names, errors =
      List.fold_left (Syntax.children node) ~f:check ~init:(names, errors) in
    match syntax node with
    | SimpleTypeSpecifier { simple_type_specifier = s; _ }
    | GenericTypeSpecifier { generic_class_type = s; _ } ->
      check_type_name_reference env (text s) (make_location_of_node node) names errors
    | _ ->
      names, errors
  in
    check (names, errors) node

(* Given a node and its context, tests if the node declares a method that is
 * both abstract and async. *)
let is_abstract_and_async_method md_node context =
  let async_node = extract_async_node md_node in
  match async_node with
  | None -> false
  | Some async_node ->
    is_generalized_abstract_method md_node context
        && not (is_missing async_node)

let extract_callconv_node node =
  match syntax node with
  | ParameterDeclaration { parameter_call_convention; _ } ->
    Some parameter_call_convention
  | ClosureParameterTypeSpecifier { closure_parameter_call_convention; _ } ->
    Some closure_parameter_call_convention
  | VariadicParameter { variadic_parameter_call_convention; _ } ->
    Some variadic_parameter_call_convention
  | _ -> None

(* Given a node, checks if it is a abstract ConstDeclaration *)
let is_abstract_const declaration =
  match syntax declaration with
  | ConstDeclaration x -> not (is_missing x.const_abstract)
  | _ -> false

(* Given a ConstDeclarator node, test whether it is abstract, but has an
   initializer. *)
let constant_abstract_with_initializer init context =
  let is_abstract =
    match context.active_const with
    | Some p_const_declaration
      when is_abstract_const p_const_declaration -> true
    | _ -> false
    in
  let has_initializer =
    not (is_missing init) in
  is_abstract && has_initializer

(* Given a node, checks if it is a concrete ConstDeclaration *)
let is_concrete_const declaration =
  match syntax declaration with
  | ConstDeclaration x -> is_missing x.const_abstract
  | _ -> false

(* Given a ConstDeclarator node, test whether it is concrete, but has no
   initializer. *)
let constant_concrete_without_initializer init context =
  let is_concrete = match context.active_const with
    | Some p_const_declaration ->
      is_concrete_const p_const_declaration
    | _ -> false in
  is_concrete && is_missing init

(* Given a PropertyDeclaration node, tests whether parent class is abstract
  final but child variable is non-static *)

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

let attribute_constructor_name node =
  match syntax node with
  | ListItem {
      list_item = { syntax = ConstructorCall { constructor_call_type; _ }; _ }; _
    } -> Syntax.extract_text constructor_call_type
  | _ -> None

let attribute_specification_contains node name =
  match syntax node with
  | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
    List.exists (syntax_node_to_list attrs) ~f:begin fun n ->
      attribute_constructor_name n = Some name
    end
  | _ -> false

let fold_attribute_spec node ~f ~init =
  match syntax node with
  | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
    List.fold (syntax_node_to_list attrs) ~init ~f
  | _ -> init

let methodish_contains_attribute node attribute =
  match node with
  | MethodishDeclaration { methodish_attribute = attr_spec; _ } ->
    attribute_specification_contains attr_spec attribute
  | _ -> false

let methodish_memoize_lsb_on_non_static node errors =
  if methodish_contains_attribute (syntax node) SN.UserAttributes.uaMemoizeLSB  &&
     not (methodish_contains_static node)
  then
    let e = make_error_from_node node SyntaxError.memoize_lsb_on_non_static
    in e :: errors
  else errors

let function_declaration_contains_attribute node attribute =
  match syntax node with
  | FunctionDeclaration { function_attribute_spec = attr_spec; _ } ->
    attribute_specification_contains attr_spec attribute
  | _ -> false

let methodish_contains_memoize env node _context =
  (is_typechecker env) && is_inside_interface env.context
    && (methodish_contains_attribute node SN.UserAttributes.uaMemoize)

let is_some_reactivity_attribute_name name =
  name = SN.UserAttributes.uaReactive ||
  name = SN.UserAttributes.uaShallowReactive ||
  name = SN.UserAttributes.uaLocalReactive ||
  name = SN.UserAttributes.uaNonRx

let attribute_matches_criteria f n =
  match syntax n with
    | ListItem {
        list_item = {
          syntax = ConstructorCall {
            constructor_call_type;
            constructor_call_argument_list = args; _
          }; _
        }; _
      } ->
      begin match Syntax.extract_text constructor_call_type with
        | Some n when f n -> Some args
        | _ -> None
      end
    | _ -> None

let is_some_reactivity_attribute n =
  attribute_matches_criteria is_some_reactivity_attribute_name n
  |> Option.is_some

let attribute_first_reactivity_annotation attr_spec =
  match syntax attr_spec with
  | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
    List.find (syntax_node_to_list attrs) ~f:is_some_reactivity_attribute
  | _ -> None


let attribute_has_reactivity_annotation attr_spec =
  Option.is_some (attribute_first_reactivity_annotation attr_spec)

let attribute_missing_reactivity_for_condition attr_spec =
  let has_attr attr = attribute_specification_contains attr_spec attr in
  not (attribute_has_reactivity_annotation attr_spec) && (
    has_attr SN.UserAttributes.uaOnlyRxIfImpl ||
    has_attr SN.UserAttributes.uaAtMostRxAsArgs
  )

let error_if_memoize_function_returns_mutable attrs errors =
  let (has_memoize, mutable_node, mut_return_node) =
    fold_attribute_spec attrs ~init:(false, None, None) ~f:(
      fun ((has_memoize, mutable_node, mut_return_node) as acc) node ->
        match attribute_constructor_name node with
        | Some n when n = SN.UserAttributes.uaMutableReturn ->
          (has_memoize, mutable_node, (Some node))
        | Some n when n = SN.UserAttributes.uaMemoize ||
                      n = SN.UserAttributes.uaMemoizeLSB ->
          (true, mutable_node, mut_return_node)
        | Some n when n = SN.UserAttributes.uaMutable ->
          (has_memoize, Some node, mut_return_node)
        | _ -> acc
    ) in
  if has_memoize
  then
    let errors =
      match mutable_node with
      | Some n ->
        make_error_from_node
          n (SyntaxError.mutable_parameter_in_memoize_function ~is_this:true) :: errors
      | None -> errors in
    let errors =
      match mut_return_node with
      | Some n ->
        make_error_from_node n SyntaxError.mutable_return_in_memoize_function :: errors
      | None -> errors in
    errors
  else errors

let methodish_missing_reactivity_for_condition node =
  match syntax node with
  | MethodishDeclaration { methodish_attribute = attr_spec; _ } ->
    attribute_missing_reactivity_for_condition attr_spec
  | _ -> false

let methodish_contains_owned_mutable_attribute node =
  methodish_contains_attribute (syntax node) SN.UserAttributes.uaOwnedMutable

let check_nonrx_annotation node errors =
  let err_decl () =
    let e = make_error_from_node node
      SyntaxError.invalid_non_rx_argument_for_declaration in
    e :: errors in
  let err_lambda () =
    let e = make_error_from_node node
      SyntaxError.invalid_non_rx_argument_for_lambda in
    e :: errors in
  let attr_spec =
    match syntax node with
    | MethodishDeclaration { methodish_attribute = s; _ }
    | FunctionDeclaration { function_attribute_spec = s; _ } ->
      Some (syntax s, true)
    | AnonymousFunction { anonymous_attribute_spec = s; _ }
    | Php7AnonymousFunction { php7_anonymous_attribute_spec = s; _ }
    | LambdaExpression { lambda_attribute_spec = s; _ }
    | AwaitableCreationExpression { awaitable_attribute_spec = s; _ } ->
      Some (syntax s, false)
    | _ -> None in
  match attr_spec with
  | Some (AttributeSpecification { attribute_specification_attributes = attrs; _ }, is_decl) ->
    (* try find argument list *)
    let args =
      List.find_map (syntax_node_to_list attrs)
        ~f:(attribute_matches_criteria ((=)SN.UserAttributes.uaNonRx)) in
    begin match args with
    (* __NonRx attribute not found *)
    | None ->  errors
    (* __NonRx attribute is found and argument list is empty.
      This is ok for lambdas but error for declarations *)
    | Some { syntax = Missing; _  } ->
      if is_decl then err_decl ()
      else errors
    (* __NonRx attribute is found with single string argument.
      This is ok for declarations for not allowed for lambdas *)
    | Some { syntax = SyntaxList [
        { syntax = ListItem {
            list_item = { syntax = LiteralExpression {
                literal_expression = { syntax = Token token; _ }; _
                }; _ }; _
          }; _ }
        ]; _  } when Token.kind token = TokenKind.DoubleQuotedStringLiteral ||
                     Token.kind token = TokenKind.SingleQuotedStringLiteral ->
      if is_decl then errors
      else err_lambda ()
    (* __NonRx attribute is found but argument list is not suitable
      nor for declarations, neither for lambdas *)
    | Some _ ->
      if is_decl then err_decl ()
      else err_lambda ()
    end
  | _ -> errors

let function_missing_reactivity_for_condition node =
  match syntax node with
  | FunctionDeclaration { function_attribute_spec = attr_spec; _ } ->
    attribute_missing_reactivity_for_condition attr_spec
  | _ -> false

let function_declaration_contains_only_rx_if_impl_attribute node =
  function_declaration_contains_attribute node SN.UserAttributes.uaOnlyRxIfImpl

let function_declaration_contains_owned_mutable_attribute node =
  function_declaration_contains_attribute node SN.UserAttributes.uaOwnedMutable

let attribute_multiple_reactivity_annotations attr_spec =
  let rec check l seen =
    match l with
    | [] -> false
    | x :: xs when is_some_reactivity_attribute x ->
      if seen then true else check xs true
    | _ :: xs -> check xs seen in
  match syntax attr_spec with
  | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
    check (syntax_node_to_list attrs) false
  | _ -> false

let methodish_multiple_reactivity_annotations node =
  match syntax node with
  | MethodishDeclaration { methodish_attribute = attr_spec; _ } ->
    attribute_multiple_reactivity_annotations attr_spec
  | _ -> false

let function_multiple_reactivity_annotations node =
  match syntax node with
  | FunctionDeclaration { function_attribute_spec = attr_spec; _ } ->
    attribute_multiple_reactivity_annotations attr_spec
  | _ -> false

let function_declaration_header_memoize_lsb context errors =
  match context.active_methodish, context.active_classish with
  | Some node, None  (* a function, not a method *)
    when function_declaration_contains_attribute node SN.UserAttributes.uaMemoizeLSB ->
    let e = make_error_from_node node SyntaxError.memoize_lsb_on_non_method
    in e :: errors
  | _ -> errors

let special_method_param_errors node context errors =
  match syntax node with
  | FunctionDeclarationHeader {function_name; function_parameter_list; _}
    when SSet.mem (String.lowercase @@ text function_name)
                  SN.Members.as_lowercase_set ->
    let params = syntax_to_list_no_separators function_parameter_list in
    let len = List.length params in
    let name = text function_name in
    let full_name = match first_parent_class_name context with
      | None -> name
      | Some c_name -> c_name ^ "::" ^ name ^ "()"
    in
    let s = String.lowercase name in
    let num_args_opt =
      match s with
      | _ when s = SN.Members.__call && len <> 2 -> Some 2
      | _ when s = String.lowercase SN.Members.__callStatic && len <> 2 -> Some 2
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
                  || s = String.lowercase SN.Members.__callStatic
                  || s = SN.Members.__get
                  || s = SN.Members.__set
                  || s = SN.Members.__isset
                  || s = SN.Members.__unset)
                  && List.exists ~f:is_param_by_ref params then
        make_error_from_node
          node (SyntaxError.invalid_args_by_ref full_name) :: errors
      else errors
    in
    errors
  | _ -> errors

let is_in_reified_class context =
  match context.active_classish with
  | Some { syntax = ClassishDeclaration {
      classish_type_parameters = {
        syntax = TypeParameters {
          type_parameters_parameters = l; _ }; _}; _ }; _ } ->
      syntax_to_list_no_separators l
      |> List.exists ~f:(fun p ->
          match syntax p with
          | TypeParameter { type_reified; _} -> not @@ is_missing type_reified
          | _ -> false
         )
  | _ -> false

let methodish_errors env node errors =
  match syntax node with
  (* TODO how to narrow the range of error *)
  | FunctionDeclarationHeader { function_parameter_list; function_type; _} ->
    let errors =
      produce_error_for_header errors
      (class_constructor_destructor_has_non_void_type env)
      node env.context SyntaxError.error2018 function_type in
    let errors =
      produce_error_for_header errors class_non_constructor_has_visibility_param
      node env.context SyntaxError.error2010 function_parameter_list in
    let errors =
      function_declaration_header_memoize_lsb env.context errors in
    errors
  | FunctionDeclaration fd ->
    let function_attrs = fd.function_attribute_spec in
    let errors =
      produce_error errors
      function_multiple_reactivity_annotations node
      SyntaxError.multiple_reactivity_annotations function_attrs in
    let errors =
      error_if_memoize_function_returns_mutable function_attrs errors in
    let errors =
      produce_error errors
      function_declaration_contains_only_rx_if_impl_attribute node
      SyntaxError.functions_cannot_implement_reactive function_attrs in
    let errors =
      check_nonrx_annotation node errors in
    let errors =
      produce_error errors
      function_missing_reactivity_for_condition node
      SyntaxError.missing_reactivity_for_condition function_attrs in
    let errors =
      produce_error errors
      function_declaration_contains_owned_mutable_attribute node
      SyntaxError.misplaced_owned_mutable function_attrs in
    errors
  | MethodishDeclaration md ->
    let header_node = md.methodish_function_decl_header in
    let modifiers = modifiers_of_function_decl_header_exn header_node in
    let class_name = Option.value (active_classish_name env.context)
      ~default:"" in
    let method_name = Option.value (extract_function_name
      md.methodish_function_decl_header) ~default:"" in
    let method_attrs = md.methodish_attribute in
    let errors =
      error_if_memoize_function_returns_mutable method_attrs errors in
    let errors =
      produce_error_for_header errors
      (methodish_contains_memoize env)
      node env.context SyntaxError.interface_with_memoize header_node in
    let errors =
      produce_error_for_header errors
      (class_constructor_has_static) header_node
      env.context (SyntaxError.error2009 class_name method_name) modifiers in
    let errors =
      produce_error_for_header errors
      (class_destructor_cannot_be_static)
      header_node env.context
      (SyntaxError.class_destructor_cannot_be_static class_name method_name) modifiers in
    let errors =
      produce_error_for_header errors async_magic_method header_node env.context
      (SyntaxError.async_magic_method ~name:method_name) modifiers in
    let errors =
      produce_error_for_header errors
      (clone_destruct_takes_no_arguments method_name) header_node env.context
      (SyntaxError.clone_destruct_takes_no_arguments class_name method_name) modifiers in
    let errors =
      produce_error_for_header errors (clone_cannot_be_static) header_node env.context
      (SyntaxError.clone_cannot_be_static class_name method_name) modifiers in
    let errors =
      produce_error errors declaration_multiple_visibility node
      SyntaxError.error2017 modifiers in
    let errors =
      produce_error errors methodish_duplicate_modifier node
      SyntaxError.error2013 modifiers in
    let errors =
      if methodish_contains_static node && (
        attribute_specification_contains method_attrs SN.UserAttributes.uaMutable ||
        attribute_specification_contains method_attrs SN.UserAttributes.uaMaybeMutable
      )
      then make_error_from_node node
        SyntaxError.mutability_annotation_on_static_method :: errors
      else errors in
    let fun_semicolon = md.methodish_semicolon in
    let errors =
      produce_error errors
      (methodish_non_abstract_without_body_not_native env node) ()
      (SyntaxError.error2015 class_name method_name) fun_semicolon in
    let errors =
      produce_error errors
      methodish_abstract_conflict_with_private
      node (SyntaxError.error2016 class_name method_name) modifiers in
    let errors =
      produce_error errors
      methodish_abstract_conflict_with_final
      node (SyntaxError.error2019 class_name method_name) modifiers in
    let errors =
      if not (is_strict env && is_typechecker env) then errors else
      produce_error errors
      (methodish_abstract_inside_interface env.context)
      node SyntaxError.error2045 modifiers in
    let errors =
      methodish_memoize_lsb_on_non_static node errors in
    let errors =
      let async_annotation = Option.value (extract_async_node node)
        ~default:node in
      produce_error errors
      (is_abstract_and_async_method node) env.context
      SyntaxError.error2046 async_annotation in
    let errors =
      if is_typechecker env
      then produce_error errors
        contains_async_not_last
        modifiers SyntaxError.async_not_last modifiers
      else errors in
    let errors =
      special_method_param_errors
      md.methodish_function_decl_header env.context errors in
    let errors =
      produce_error errors
      methodish_multiple_reactivity_annotations node
      SyntaxError.multiple_reactivity_annotations method_attrs in
    let errors =
      check_nonrx_annotation node errors in
    let errors =
      produce_error errors
      methodish_missing_reactivity_for_condition node
      SyntaxError.missing_reactivity_for_condition method_attrs in
    let errors =
      produce_error errors
      methodish_contains_owned_mutable_attribute node
      SyntaxError.misplaced_owned_mutable method_attrs in
    errors
  | _ -> errors

let is_hashbang text =
  match Syntax.extract_text text with
  | None -> false
  | Some text ->
    let r = Str.regexp "^#!.*\n" in
    let count = List.length @@ String_utils.split_on_newlines text in
    count = 1 && Str.string_match r text 0 && Str.matched_string text = text

let class_has_a_construct_method context =
  match first_parent_classish_node TokenKind.Class context with
  | Some ({ syntax = ClassishDeclaration
            { classish_body =
              { syntax = ClassishBody
                { classish_body_elements = methods; _}; _}; _}; _}) ->
    let methods = syntax_to_list_no_separators methods in
    List.exists methods ~f:(function
      { syntax = MethodishDeclaration
          { methodish_function_decl_header =
            { syntax = FunctionDeclarationHeader
              { function_name; _}; _}; _}; _} ->
        String.lowercase @@ text function_name = SN.Members.__construct
      | _ -> false)
  | _ -> false

let is_in_construct_method context = if is_immediately_in_lambda context then false else
  match first_parent_function_name context, first_parent_class_name context with
  | None, _ -> false
  (* Function name is __construct *)
  | Some s, _ when String.lowercase s = SN.Members.__construct -> true
  (* Function name is same as class name *)
  | Some s1, Some s2 ->
    context.nested_namespaces = [] &&
    not @@ class_has_a_construct_method context &&
    String.lowercase s1 = String.lowercase s2
  | _ -> false


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

let params_errors _env params _namespace_name names errors =
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
    List.fold_right param_list ~init:(false, false, false)
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

let parameter_rx_errors context errors node =
  match syntax node with
  | ParameterDeclaration { parameter_attribute = spec; parameter_name = name; _ } ->
    let has_owned_mutable =
      attribute_specification_contains spec SN.UserAttributes.uaOwnedMutable in
    let errors =
      let has_mutable =
          attribute_specification_contains spec SN.UserAttributes.uaMutable in
      let has_maybemutable =
          attribute_specification_contains spec SN.UserAttributes.uaMaybeMutable in
      let errors =
        match has_mutable, has_owned_mutable, has_maybemutable with
        | true, true, _ ->
          make_error_from_node node
            SyntaxError.conflicting_mutable_and_owned_mutable_attributes :: errors
        | true, _, true ->
          make_error_from_node node
            SyntaxError.conflicting_mutable_and_maybe_mutable_attributes :: errors
        | _, true, true ->
          make_error_from_node node
            SyntaxError.conflicting_owned_mutable_and_maybe_mutable_attributes :: errors
        | _ -> errors in
      let errors =
        if (has_mutable || has_owned_mutable || has_maybemutable) &&
           is_variadic_expression name
        then make_error_from_node name SyntaxError.vararg_and_mutable :: errors
        else errors in
      let is_inout = is_parameter_with_callconv node in
      let errors =
        if is_inout && (has_mutable || has_maybemutable || has_owned_mutable)
        then make_error_from_node
          node SyntaxError.mutability_annotation_on_inout_parameter :: errors
        else errors in

      let errors =
        if has_owned_mutable || has_mutable
        then begin
          let attrs = context.active_callable_attr_spec in
          let active_is_rx = context.active_is_rx_or_enclosing_for_lambdas in
          let parent_func_is_memoize =
            Option.value_map attrs ~default:false ~f:(fun spec ->
              attribute_specification_contains spec SN.UserAttributes.uaMemoize ||
              attribute_specification_contains spec SN.UserAttributes.uaMemoize
            ) in
          let errors =
            if has_owned_mutable && not active_is_rx
            then make_error_from_node node
              SyntaxError.mutably_owned_attribute_on_non_rx_function :: errors
            else errors
          in
          let errors =
            if has_mutable && parent_func_is_memoize
            then make_error_from_node node
              (SyntaxError.mutable_parameter_in_memoize_function ~is_this:false) :: errors
            else errors
          in
          errors
        end
        else errors in
      errors
    in
    errors
  | _ -> errors

  let does_binop_create_write_on_left = function
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
          | TokenKind.GreaterThanGreaterThanEqual
          | TokenKind.QuestionQuestionEqual) -> true
    | _ -> false

  let does_unop_create_write = function
    | Some (TokenKind.PlusPlus | TokenKind.MinusMinus | TokenKind.Ampersand) -> true
    | _ -> false

  let does_decorator_create_write = function
    | Some (TokenKind.Inout) -> true
    | _ -> false

  type lval_type = LvalTypeNone | LvalTypeNonFinal | LvalTypeFinal

  let node_lval_type node parents =
    let rec is_in_final_lval_position node parents =
      match parents with
      | ExpressionStatement _ :: _ -> true
      | ForStatement {
        for_initializer = { syntax = for_initializer; _ };
        for_end_of_loop = { syntax = for_end_of_loop; _ }; _
      } :: _
        when phys_equal for_initializer node || phys_equal for_end_of_loop node ->
        true
      | (UsingStatementFunctionScoped { using_function_expression = { syntax = e; _ }; _ }
        | UsingStatementBlockScoped { using_block_expressions = { syntax = e; _ }; _ }) :: _
        when phys_equal e node -> true
      | (SyntaxList _ | ListItem _) as node :: parents ->
        is_in_final_lval_position node parents
      | _ -> false
    in
    let rec get_arg_call_node_with_parents node parents =
      match parents with
      | (SyntaxList _ | ListItem _) as next_node :: next_parents ->
        get_arg_call_node_with_parents next_node next_parents
      | (FunctionCallExpression {
          function_call_argument_list = { syntax = arg_list; _ }; _ }
      ) as call_expression :: parents when phys_equal arg_list node ->
        Some (call_expression, parents)
      | _ -> None
    in
    let lval_ness_of_function_arg next_node next_parents =
      (match get_arg_call_node_with_parents next_node next_parents with
      | None -> LvalTypeNone
      | Some (call_node, call_parents) ->
        if is_in_final_lval_position call_node call_parents
          then LvalTypeFinal
          else (match call_parents with
            | BinaryExpression {
              binary_operator = token;
              binary_right_operand = { syntax = rval; _ };
            _ } as next_node :: next_parents
              when phys_equal rval call_node &&
                   does_binop_create_write_on_left (token_kind token) ->
               if is_in_final_lval_position next_node next_parents
                then LvalTypeFinal
                else LvalTypeNonFinal
            | _ -> LvalTypeNonFinal)
      ) in
    match parents with
    | DecoratedExpression {
      decorated_expression_decorator = token;
      decorated_expression_expression = { syntax = lval; _ }
    } as next_node :: next_parents
      when phys_equal lval node &&
           does_decorator_create_write (token_kind token) ->
      lval_ness_of_function_arg next_node next_parents

    | PrefixUnaryExpression {
      prefix_unary_operator = token;
      prefix_unary_operand = { syntax = lval; _ }
    } :: _
      when phys_equal lval node &&
           Some TokenKind.Ampersand = token_kind token ->
      LvalTypeNone

    | (PrefixUnaryExpression {
      prefix_unary_operator = token;
      prefix_unary_operand = { syntax = lval; _ }
    }
    | PostfixUnaryExpression {
      postfix_unary_operator = token;
      postfix_unary_operand = { syntax = lval; _ }
    }) as next_node :: next_parents
      when phys_equal lval node &&
           does_unop_create_write (token_kind token) ->
      if is_in_final_lval_position next_node next_parents
        then LvalTypeFinal
        else LvalTypeNonFinal

    | BinaryExpression {
      binary_operator = token;
      binary_left_operand = { syntax = lval; _ };
    _ } as next_node :: next_parents
      when phys_equal lval node &&
           does_binop_create_write_on_left (token_kind token) ->
      if is_in_final_lval_position next_node next_parents
        then LvalTypeFinal
        else LvalTypeNonFinal

    | ForeachStatement {
      foreach_key = { syntax = key; _ };
      foreach_value = { syntax = value; _ };
    _ } :: _
      when phys_equal key node || phys_equal value node ->
      LvalTypeFinal

    | _ -> LvalTypeNone

  let lval_errors env syntax_node parents errors =
    if not (ParserOptions.disable_lval_as_an_expression env.parser_options) then errors else
    let node = syntax syntax_node in
    let parents = List.map ~f:syntax parents in
    match node_lval_type node parents with
    | LvalTypeFinal
    | LvalTypeNone -> errors
    | LvalTypeNonFinal ->
      make_error_from_node syntax_node SyntaxError.lval_as_expression :: errors


let parameter_errors env node namespace_name names errors =
  match syntax node with
  | ParameterDeclaration p ->

    let callconv_text = Option.value (extract_callconv_node node) ~default:node
      |> text in
    let errors =
      produce_error_from_check errors param_with_callconv_has_default
      node (SyntaxError.error2074 callconv_text) in
    let errors = parameter_rx_errors env.context errors node in
    let errors =
      produce_error_from_check errors param_with_callconv_is_byref
      node (SyntaxError.error2075 callconv_text) in
    let names, errors =
      check_type_hint env p.parameter_type names errors in
    let errors = if is_parameter_with_callconv node then
      begin
        let errors =
          if is_inside_async_method env.context then
          make_error_from_node ~error_type:SyntaxError.RuntimeError
            node SyntaxError.inout_param_in_async :: errors
          else errors in
        let errors =
          if is_in_construct_method env.context then
          make_error_from_node ~error_type:SyntaxError.RuntimeError
            node SyntaxError.inout_param_in_construct :: errors
          else errors in
        let inMemoize = first_parent_function_attributes_contains
          env.context SN.UserAttributes.uaMemoize in
        let inMemoizeLSB = first_parent_function_attributes_contains
          env.context SN.UserAttributes.uaMemoizeLSB in
        let errors = if (inMemoize || inMemoizeLSB) &&
              not @@ is_immediately_in_lambda env.context then
          make_error_from_node ~error_type:SyntaxError.RuntimeError
            node SyntaxError.memoize_with_inout :: errors
          else errors in
        errors
      end else errors
    in
    let errors =
      if not (is_hack env) &&
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
  | Php7AnonymousFunction { php7_anonymous_parameters = params; _ }
  | ClosureTypeSpecifier { closure_parameter_list = params; _ }
  | LambdaExpression
    { lambda_signature = {syntax = LambdaSignature { lambda_parameters = params; _ }; _}
    ; _
    } ->
    let errors =
      syntax_to_list_no_separators params
      |> List.fold_left ~init:errors ~f:(parameter_rx_errors env.context) in
    params_errors env params namespace_name names errors
  | DecoratedExpression _ -> names, decoration_errors node errors
  | _ -> names, errors


let redeclaration_errors env node parents namespace_name names errors =
  match syntax node with
  | FunctionDeclarationHeader f when not (is_missing f.function_name)->
    begin match parents with
      | { syntax = FunctionDeclaration _; _}
        :: _ :: {syntax = NamespaceBody _; _} :: _
      | [{ syntax = FunctionDeclaration _; _ }; _; _]
      | { syntax = MethodishDeclaration _; _ } :: _
      | { syntax = MethodishTraitResolution _; _ } :: _ ->
        let function_name = text f.function_name in
        let location = make_location_of_node f.function_name in
        let is_method = match parents with
          | { syntax = MethodishDeclaration _; _ } :: _ -> true
          | _ -> false
        in
        let def = make_first_use_or_def ~is_method
          ~kind:Name_def location namespace_name function_name in
        let errors =
          match strmap_get function_name names.t_functions with
          | Some { f_location = { start_offset; _}; f_kind = Name_def; f_global; _ }
              when f_global = def.f_global ->
            let text = SyntaxTree.text env.syntax_tree in
            let line, _ =
              Full_fidelity_source_text.offset_to_position text start_offset in
            let path =
              Relative_path.to_absolute @@
                Full_fidelity_source_text.file_path text in
            let loc = path ^ ":" ^ string_of_int line in
            let err, error_type =
              match first_parent_class_name env.context with
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
          t_functions = strmap_add function_name def names.t_functions }, errors
      | _ ->
        (* Only check this in strict mode. *)
        if not (is_typechecker env && is_strict env) then names, errors else
        let error = make_error_from_node
          ~error_type:SyntaxError.ParseError
          node
          SyntaxError.decl_outside_global_scope
        in
        names, error :: errors
    end
  | _ -> names, errors

let is_foreach_in_for for_initializer =
  match syntax_node_to_list for_initializer with
  | ( { syntax = ListItem { list_item = item; _ }; _ } :: _) ->
    is_as_expression item
  | _ -> false

let statement_errors env node parents errors =
  let result = match syntax node with
  | TryStatement { try_catch_clauses; try_finally_clause; _ }
    when (is_missing try_catch_clauses) && (is_missing try_finally_clause) ->
    Some (node, SyntaxError.error2007)
  | UsingStatementFunctionScoped _
    when not (using_statement_function_scoped_is_legal parents) ->
    Some (node, SyntaxError.using_st_function_scoped_top_level)
  | ForStatement { for_initializer ; _ }
    when is_foreach_in_for for_initializer ->
    Some (node, SyntaxError.for_with_as_expression)
  | CaseLabel {
    case_colon = {
      syntax = Token m;
      _;
    } as colon;
    _
  }
  | DefaultLabel {
    default_colon = {
      syntax = Token m;
      _;
    } as colon;
    _
  } when is_typechecker env &&  Token.kind m <> TokenKind.Colon ->
    Some (colon, SyntaxError.error1020)
  | _ -> None in
  match result with
  | None -> errors
  | Some (error_node, error_message) ->
    make_error_from_node error_node error_message :: errors

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
  |> List.fold_left ~init:errors ~f:check_collection_member

let invalid_shape_initializer_name env node errors =
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
    then make_error_from_node node SyntaxError.invalid_shape_field_name :: errors
    else errors
  | ScopeResolutionExpression _ -> errors
  | QualifiedName _ ->
      if is_typechecker env then
      make_error_from_node node SyntaxError.invalid_shape_field_name :: errors
      else errors
  | Token _ when is_name node ->
      if is_typechecker env then
        make_error_from_node node SyntaxError.invalid_shape_field_name :: errors
      else
        errors
  | _ -> make_error_from_node node SyntaxError.invalid_shape_field_name :: errors

let invalid_shape_field_check env node errors =
  match syntax node with
  | FieldInitializer { field_initializer_name; _} ->
    invalid_shape_initializer_name env field_initializer_name errors
  | _ -> make_error_from_node node SyntaxError.invalid_shape_field_name :: errors

let is_in_unyieldable_magic_method context =
  match first_parent_function_name context with
  | None -> false
  | Some s ->
    let s = String.lowercase s in
    begin match s with
    | _ when s = SN.Members.__call -> false
    | _ when s = SN.Members.__invoke -> false
    | _ when s = String.lowercase SN.Members.__callStatic -> false
    | _ -> SSet.mem s SN.Members.as_lowercase_set
    end

let function_call_argument_errors ~in_constructor_call node errors =
  match syntax node with
  | PrefixUnaryExpression
    { prefix_unary_operator = { syntax = Token token; _ }
    ; prefix_unary_operand
    } when Token.kind token = TokenKind.Ampersand &&
      SN.Superglobals.is_superglobal @@ text prefix_unary_operand ->
        make_error_from_node node SyntaxError.error2078 :: errors
  | DecoratedExpression
    { decorated_expression_decorator = { syntax = Token token ; _ }
    ; decorated_expression_expression = expression
    } when Token.kind token = TokenKind.Inout ->
      let result =
        if in_constructor_call then Some (true, SyntaxError.inout_param_in_construct) else
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

let function_call_on_xhp_name_errors env node errors =
  match syntax node with
  | MemberSelectionExpression { member_object; member_name = name; _ }
  | SafeMemberSelectionExpression {
      safe_member_object=member_object;
      safe_member_name = name; _ } ->
    let errors =
    begin match syntax member_object with
    | XHPExpression _ when is_typechecker env ->
      let e =
        make_error_from_node node SyntaxError.method_calls_on_xhp_expression in
      e::errors
    | _ -> errors
    end in
    begin match syntax name with
    | Token token when Token.kind token = TokenKind.XHPClassName ->
      let e =
        make_error_from_node node SyntaxError.method_calls_on_xhp_attributes in
      e :: errors
    | _ -> errors
    end
  | _ -> errors

let no_async_before_lambda_body env body_node errors =
  match syntax body_node with
  | AwaitableCreationExpression _ when not env.codegen ->
    (make_error_from_node body_node SyntaxError.no_async_before_lambda_body)
      :: errors
  | _ -> errors

let no_memoize_attribute_on_lambda node errors =
  match syntax node with
  | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
    List.fold (syntax_node_to_list attrs) ~init:errors ~f:begin fun errors n ->
      match syntax n with
      | ListItem {
          list_item = ({ syntax = ConstructorCall { constructor_call_type; _ }; _ } as attr);_
        } ->
        begin match Syntax.extract_text constructor_call_type with
        | Some n when n = SN.UserAttributes.uaMemoize ->
          let e =
            make_error_from_node attr SyntaxError.memoize_on_lambda in
          e::errors
        | Some n when n = SN.UserAttributes.uaMemoizeLSB ->
          let e =
            make_error_from_node attr SyntaxError.memoize_on_lambda in
          e::errors
        | _ -> errors
        end
      | _ -> errors
    end
  | _ -> errors

let is_assignment node =
  match syntax node with
  | BinaryExpression { binary_operator = { syntax = Token token; _ }; _ } ->
    Token.kind token = TokenKind.Equal
  | _ -> false

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

let new_variable_errors node =
  let rec helper node ~inside_scope_resolution =
    match syntax node with
    | SimpleTypeSpecifier _ -> []
    | VariableExpression _ -> []
    | GenericTypeSpecifier _ -> []
    | PipeVariableExpression _ -> []
    | SubscriptExpression { subscript_index = { syntax = Missing; _ }; _ } ->
      [ make_error_from_node node SyntaxError.instanceof_missing_subscript_index ]
    | SubscriptExpression { subscript_receiver; _ } ->
      helper subscript_receiver ~inside_scope_resolution
    | MemberSelectionExpression { member_object; _ } ->
      if inside_scope_resolution
      then [
        make_error_from_node node SyntaxError.instanceof_memberselection_inside_scoperesolution
      ]
      else helper member_object ~inside_scope_resolution
    | ScopeResolutionExpression
      {
        scope_resolution_qualifier;
        scope_resolution_name = { syntax = Token name; _ };
        _
      } when is_good_scope_resolution_qualifier scope_resolution_qualifier
        && Token.kind name = TokenKind.Variable -> []
    | ScopeResolutionExpression
      {
        scope_resolution_qualifier;
        scope_resolution_name = { syntax = Token name; _ };
        _
      } when Token.kind name = TokenKind.Variable ->
      helper scope_resolution_qualifier ~inside_scope_resolution:true
    | ScopeResolutionExpression _ ->
      [ make_error_from_node node SyntaxError.instanceof_invalid_scope_resolution ]

    | _ ->
      let error_msg = SyntaxError.instanceof_new_unknown_node
        (SyntaxKind.to_string @@ kind node) in
      [ make_error_from_node node error_msg ]
  in
  helper node ~inside_scope_resolution:false

let class_type_designator_errors node =
  if is_good_scope_resolution_qualifier node then [] else
  match syntax node with
  | ParenthesizedExpression
    {
      parenthesized_expression_expression = {
        syntax = PrefixUnaryExpression { prefix_unary_operator = { syntax = Token token; _ }; _ };
        _
      };
      _
    }
    when Token.kind token = TokenKind.Ampersand ->
    [make_error_from_node node SyntaxError.instanceof_reference]
  | ParenthesizedExpression _ ->
    (* A parenthesized expression that evaluates to a string or object is a
       valid RHS for instanceof and new. *)
    []
  | _ -> new_variable_errors node

let rec check_reference node errors =
  match syntax node with
  | ScopeResolutionExpression { scope_resolution_name; _}
    when token_kind scope_resolution_name = Some TokenKind.Name ->
    make_error_from_node node
      SyntaxError.reference_to_static_scope_resolution :: errors
  | PrefixUnaryExpression { prefix_unary_operator; _ }
    when token_kind prefix_unary_operator <> Some TokenKind.Dollar ->
    make_error_from_node node SyntaxError.nested_unary_reference :: errors
  | FunctionCallExpression _
  | ListExpression _
  | MemberSelectionExpression _
  | ObjectCreationExpression _
  | PipeVariableExpression _
  | SafeMemberSelectionExpression _
  | ScopeResolutionExpression _
  | VariableExpression _ -> errors
  | Token token when Token.kind token = TokenKind.Variable -> errors
  | PrefixUnaryExpression {
    prefix_unary_operator = { syntax = Token token; _ };
    prefix_unary_operand = {
      syntax = PrefixUnaryExpression { prefix_unary_operator = op; _ };
      _
    }
  } when Token.kind token = TokenKind.Dollar && token_kind op = Some TokenKind.Dollar ->
    errors
  | PrefixUnaryExpression {
    prefix_unary_operator = { syntax = Token token; _ };
    prefix_unary_operand = { syntax = BracedExpression _ | VariableExpression _; _ }
  } when Token.kind token = TokenKind.Dollar -> errors
  | ParenthesizedExpression { parenthesized_expression_expression; _ } ->
    check_reference parenthesized_expression_expression errors
  | _ -> make_error_from_node node SyntaxError.invalid_reference :: errors

let rec_walk ~init ~f node =
  let rec rec_walk_impl parents init node =
    let new_init, continue_walk = f init node parents in
    if continue_walk then
      List.fold_left
        ~init:new_init
        ~f:(rec_walk_impl ((syntax node) :: parents))
        (Syntax.children node)
    else new_init in
  rec_walk_impl [] init node

let does_binop_create_write_on_left = function
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
        | TokenKind.GreaterThanGreaterThanEqual
        | TokenKind.QuestionQuestionEqual) -> true
  | _ -> false

let find_invalid_lval_usage errors nodes =
  let get_errors = rec_walk ~f:(fun errors syntax_node parents ->
    let node = syntax syntax_node in
    match node with
    | AnonymousFunction _
    | Php7AnonymousFunction _
    | LambdaExpression _
    | AwaitableCreationExpression _ ->
      errors, false
    | _ ->
      let errors = match node_lval_type node parents with
      | LvalTypeFinal
      | LvalTypeNone -> errors
      | LvalTypeNonFinal ->
        make_error_from_node syntax_node SyntaxError.lval_as_expression :: errors in
      errors, true
  ) in

  List.fold_left
    ~init:errors
    ~f:(fun acc node -> get_errors ~init:acc node)
    nodes

type binop_allows_await_in_positions =
  | BinopAllowAwaitBoth
  | BinopAllowAwaitLeft
  | BinopAllowAwaitRight
  | BinopAllowAwaitNone

let get_positions_binop_allows_await t =
  (match token_kind t with
  | None -> BinopAllowAwaitNone
  | Some t -> (match t with
  | TokenKind.And
  | TokenKind.Or
  | TokenKind.BarBar
  | TokenKind.AmpersandAmpersand
  | TokenKind.QuestionColon
  | TokenKind.QuestionQuestion -> BinopAllowAwaitLeft
  | TokenKind.Equal
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
  | TokenKind.GreaterThanGreaterThanEqual -> BinopAllowAwaitRight
  | TokenKind.Xor
  | TokenKind.Plus
  | TokenKind.Minus
  | TokenKind.Star
  | TokenKind.Slash
  | TokenKind.StarStar
  | TokenKind.EqualEqualEqual
  | TokenKind.LessThan
  | TokenKind.GreaterThan
  | TokenKind.Percent
  | TokenKind.Dot
  | TokenKind.EqualEqual
  | TokenKind.ExclamationEqual
  | TokenKind.LessThanGreaterThan
  | TokenKind.ExclamationEqualEqual
  | TokenKind.LessThanEqual
  | TokenKind.LessThanEqualGreaterThan
  | TokenKind.GreaterThanEqual
  | TokenKind.BarGreaterThan (* Custom handling *)
  | TokenKind.Ampersand
  | TokenKind.Bar
  | TokenKind.LessThanLessThan
  | TokenKind.GreaterThanGreaterThan
  | TokenKind.Carat -> BinopAllowAwaitBoth
  | TokenKind.QuestionQuestionEqual
  | _ -> BinopAllowAwaitNone
  ))

let unop_allows_await t =
  (match token_kind t with
  | None -> false
  | Some t -> (match t with
  | TokenKind.Exclamation
  | TokenKind.Tilde
  | TokenKind.Plus
  | TokenKind.Minus
  | TokenKind.At
  | TokenKind.Clone
  | TokenKind.Print -> true
  | _ -> false
  ))

let await_as_an_expression_errors await_node parents errors =
  let rec go parents node =
    let n, tail =
      match parents with
        | head :: tail -> head, tail
        | _ -> failwith "Unexpect missing parent"
      in
    match syntax n with
    (* statements that root for the concurrently executed await expressions *)
    | ExpressionStatement _
    | ReturnStatement _
    | UnsetStatement _
    | EchoStatement _
    | ThrowStatement _ -> errors
    | IfStatement { if_condition; _ }
      when phys_equal node if_condition -> errors
    | ForStatement { for_initializer; _ }
      when phys_equal node for_initializer -> errors
    | SwitchStatement { switch_expression; _ }
      when phys_equal node switch_expression -> errors
    | ForeachStatement { foreach_collection; _ }
      when phys_equal node foreach_collection -> errors
    | UsingStatementBlockScoped { using_block_expressions; _ }
      when phys_equal node using_block_expressions -> errors
    | UsingStatementFunctionScoped { using_function_expression; _ }
      when phys_equal node using_function_expression -> errors
    | LambdaExpression { lambda_body; _ }
      when phys_equal node lambda_body -> errors

    (* Unary based expressions have their own custom fanout *)
    | PrefixUnaryExpression { prefix_unary_operator = operator; _ }
    | PostfixUnaryExpression { postfix_unary_operator = operator; _ }
    | DecoratedExpression { decorated_expression_decorator = operator; _ }
      when unop_allows_await operator -> go tail n
    (*
       left or right operand of binary expressions are considered legal locations
       if operator is not short-circuiting and containing expression
       is in legal location *)
    | BinaryExpression {
        binary_left_operand = l;
        binary_right_operand = r;
        binary_operator = op;
      }
        when
          (match get_positions_binop_allows_await op with
            | BinopAllowAwaitBoth -> true
            | BinopAllowAwaitLeft -> phys_equal node l
            | BinopAllowAwaitRight -> phys_equal node r
            | BinopAllowAwaitNone -> false) ->
        go tail n
    (* test part of conditional expression is considered legal location if
       conditional expression itself is in legal location *)
    | ConditionalExpression {
        conditional_test = test; _
      } when phys_equal node test -> go tail n
    | FunctionCallExpression {
        function_call_receiver = r;
        function_call_argument_list = args; _
      }
        when phys_equal node r ||
             (phys_equal node args && not (is_safe_member_selection_expression r))
        -> go tail n
    (* object of member selection expression or safe member selection expression
       is in legal position if member selection expression itself is in legal position *)
    | SafeMemberSelectionExpression {
        safe_member_object = o; _
      } when phys_equal node o -> go tail n
    (* These are nodes where any position is valid *)
    | CastExpression _
    | MemberSelectionExpression _
    | ScopeResolutionExpression _
    | InstanceofExpression _
    | IsExpression _
    | AsExpression _
    | NullableAsExpression _
    | EmptyExpression _
    | IssetExpression _
    | ParenthesizedExpression _
    | BracedExpression _
    | EmbeddedBracedExpression _
    | CollectionLiteralExpression _
    | ObjectCreationExpression _
    | ConstructorCall _
    | ShapeExpression _
    | TupleExpression _
    | ArrayCreationExpression _
    | ArrayIntrinsicExpression _
    | DarrayIntrinsicExpression _
    | DictionaryIntrinsicExpression _
    | KeysetIntrinsicExpression _
    | VarrayIntrinsicExpression _
    | VectorIntrinsicExpression _
    | ElementInitializer _
    | FieldInitializer _
    | SubscriptExpression _
    | EmbeddedSubscriptExpression _
    | YieldExpression _
    | SyntaxList _
    | ListItem _ -> go tail n
    (* otherwise report error and bail out *)
    | _ ->
      make_error_from_node
        await_node SyntaxError.invalid_await_position :: errors
  in

  let errors = go parents await_node in
  let is_in_concurrent = List.exists
    ~f:(fun parent -> match syntax parent with
      | ConcurrentStatement _ -> true
      | _ -> false
    ) parents in
  let errors = if is_in_concurrent then errors else
    let await_node_statement_parent = List.find
      ~f:(fun parent -> match syntax parent with
        | ExpressionStatement _
        | ReturnStatement _
        | UnsetStatement _
        | EchoStatement _
        | ThrowStatement _
        | IfStatement _
        | ForStatement _
        | SwitchStatement _
        | ForeachStatement _ -> true
        | _ -> false
      ) parents in
    match await_node_statement_parent with
    | Some x -> find_invalid_lval_usage errors [x]
    (* We must have already errored in "go" *)
    | None -> errors
  in
  errors

let node_has_await_child = rec_walk ~init:false ~f:(fun acc node _ ->
  let is_new_scope = match syntax node with
    | AnonymousFunction _
    | Php7AnonymousFunction _
    | LambdaExpression _
    | AwaitableCreationExpression _ ->
      true
    | _ -> false in
  if is_new_scope then false, false else
  let is_await n = match syntax n with
  | PrefixUnaryExpression { prefix_unary_operator = op; _ }
    when token_kind op = Some TokenKind.Await -> true
  | _ -> false in
  let found_await = acc || is_await node in
  found_await, not found_await
)

let expression_errors env _is_in_concurrent_block namespace_name node parents errors =
  let is_decimal_or_hexadecimal_literal token =
    match Token.kind token with
    | TokenKind.DecimalLiteral | TokenKind.HexadecimalLiteral -> true
    | _ -> false
  in
  match syntax node with
  (* It is ambiguous what `instanceof (A)` means: either instanceof a type A
   * or instanceof a type whose name is what the constant A evaluates to.
   * We therefore simply disallow this. *)
  | InstanceofExpression
    { instanceof_right_operand =
      { syntax = ParenthesizedExpression
        { parenthesized_expression_expression =
          { syntax = Token _; _ } as in_paren
        ; _ }
      ; _ }
    ; _ } when not env.codegen ->
    let in_paren = text in_paren in
    make_error_from_node node (SyntaxError.instanceof_paren in_paren) :: errors
  (* We parse the right hand side of `new` and `instanceof` as a generic
     expression, but PHP (and therefore Hack) only allow a certain subset of
     expressions, so we should verify here that the expression we parsed is in
     that subset.
     Refer: https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#instanceof-operator*)
  | ConstructorCall ctr_call ->
    let typechecker_errors =
      if is_typechecker env then
        match parents with
        (* list item -> syntax list -> attribute *)
        | _ :: _ :: a :: _ when
          is_attribute_specification a || is_file_attribute_specification a ->
          []
        | _ ->
          if (is_missing ctr_call.constructor_call_left_paren ||
              is_missing ctr_call.constructor_call_right_paren)
          then
            let node = ctr_call.constructor_call_type in
            let constructor_name = text ctr_call.constructor_call_type in
            [make_error_from_node node (SyntaxError.error2038 constructor_name)]
          else []
      else []
    in
    let designator_errors = class_type_designator_errors ctr_call.constructor_call_type in
    let func_errors =
      let arg_list = syntax_to_list_no_separators ctr_call.constructor_call_argument_list in
      List.fold_right arg_list ~init:[]
        ~f:(fun p acc -> function_call_argument_errors ~in_constructor_call:true p acc)
    in
    typechecker_errors @ designator_errors @ func_errors @ errors
  | InstanceofExpression { instanceof_right_operand = operand; _ } ->
    (class_type_designator_errors operand) @ errors
  | LiteralExpression { literal_expression = {syntax = Token token; _} as e ; _}
    when env.is_hh_file && is_decimal_or_hexadecimal_literal token ->
    let text = text e in
    begin try ignore (Int64.of_string text); errors
    with _ ->
      let error_text =
        if Token.kind token = TokenKind.DecimalLiteral
        then SyntaxError.error2071 text
        else SyntaxError.error2072 text in
      make_error_from_node node error_text :: errors
    end
  | SafeMemberSelectionExpression _ when not (is_hack env) ->
    make_error_from_node node SyntaxError.error2069 :: errors
  | SubscriptExpression { subscript_left_bracket; _}
    when (is_typechecker env)
      && is_left_brace subscript_left_bracket ->
    make_error_from_node node SyntaxError.error2020 :: errors
  | HaltCompilerExpression { halt_compiler_argument_list = args; _ } ->
    let errors =
      if List.is_empty (syntax_to_list_no_separators args) then errors
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
    let errors = List.fold_right arg_list ~init:errors
      ~f:(fun p acc -> function_call_argument_errors ~in_constructor_call:false p acc)
    in
    let errors =
      function_call_on_xhp_name_errors env function_call_receiver errors in
    errors
  | ListExpression { list_members; _ }
      when is_missing list_members && is_hhvm_compat env ->
    begin match parents with
      | { syntax = ForeachStatement { foreach_value; _ }; _ } :: _
          when phys_equal node foreach_value ->
        make_error_from_node ~error_type:SyntaxError.RuntimeError node
          SyntaxError.error2077 :: errors
      | _ -> errors
    end
  | ListExpression _ ->
    begin match parents with
      | e :: _ when is_return_statement e ->
      make_error_from_node node SyntaxError.list_must_be_lvar :: errors
      | _ -> errors
    end
  | ShapeExpression { shape_expression_fields; _} ->
    List.fold_right ~f:(invalid_shape_field_check env)
      (syntax_to_list_no_separators shape_expression_fields) ~init:errors
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
    if not (is_hack env) then
      (* In php, vec[0] would be a subscript, where vec would be a constant *)
      match syntax_to_list_no_separators m with
      | _ :: _ :: _ -> (* 2 elements or more *)
        make_error_from_node node SyntaxError.list_as_subscript :: errors
      | _ -> errors
    else check_collection_members m errors
  | ArrayCreationExpression  { array_creation_members = m; _ }
  | ArrayIntrinsicExpression { array_intrinsic_members = m; _ } ->
    check_collection_members m errors
  | VarrayIntrinsicExpression { varray_intrinsic_members = m; _ }
  | DarrayIntrinsicExpression { darray_intrinsic_members = m; _ } ->
    let errors =
      if not (is_hack env)
      then make_error_from_node node SyntaxError.vdarray_in_php :: errors
      else errors in
    check_collection_members m errors
  | YieldFromExpression _
  | YieldExpression _ ->
    let errors =
      if is_in_unyieldable_magic_method env.context then
      make_error_from_node node SyntaxError.yield_in_magic_methods :: errors
      else errors in
    let errors = match env.context.active_callable with
      | Some _ -> errors
      | None -> make_error_from_node node SyntaxError.yield_outside_function :: errors
    in
    let errors =
      if has_inout_params env.context then
      let e =
        if is_inside_async_method env.context
        then SyntaxError.inout_param_in_async_generator
        else SyntaxError.inout_param_in_generator in
      make_error_from_node ~error_type:SyntaxError.RuntimeError node e :: errors
      else errors in
    errors
  | ScopeResolutionExpression
    { scope_resolution_qualifier = qualifier
    ; scope_resolution_name = name
    ; _ } ->
      let is_dynamic_name, is_self_or_parent, is_valid =
        (* PHP langspec allows string literals, variables
          qualified names, static, self and parent as valid qualifiers *)
        (* We do not allow string literals in hack *)
        match syntax qualifier, token_kind qualifier with
        | LiteralExpression _, _ ->
          false, false, not (is_typechecker env)
        | QualifiedName _, _ -> false, false, true
        | _, Some TokenKind.Name
        | _, Some TokenKind.XHPClassName
        | _, Some TokenKind.Static -> false, false, true
        | _, Some TokenKind.Self
        | _, Some TokenKind.Parent -> false, true, true
        (* ${}::class *)
        | PrefixUnaryExpression {
          prefix_unary_operator = op; _
        }, _ when token_kind op = Some TokenKind.Dollar ->
          true, false, true
        | PipeVariableExpression _, _
        | VariableExpression _, _
        | SimpleTypeSpecifier _, _
        | GenericTypeSpecifier _, _ -> true, false, true
        | _ -> true, false, not (is_typechecker env)
      in
      let errors = if not is_valid then
        make_error_from_node
          node SyntaxError.invalid_scope_resolution_qualifier :: errors
        else errors in
      let is_name_class = String.lowercase @@ text name = "class" in
      let errors = if is_dynamic_name && is_name_class then
        make_error_from_node
          node SyntaxError.coloncolonclass_on_dynamic :: errors
        else errors in
      let text_name = text qualifier in
      let is_name_namespace = String.lowercase @@ text_name = "namespace" in
      let errors = if is_name_namespace
        then make_error_from_node ~error_type:SyntaxError.ParseError
          node (SyntaxError.namespace_not_a_classname)::errors
        else errors in
      let errors = if is_self_or_parent && is_name_class &&
          not @@ is_in_active_class_scope env.context
        then make_error_from_node ~error_type:SyntaxError.RuntimeError
          node (SyntaxError.self_or_parent_colon_colon_class_outside_of_class
            @@ text qualifier) :: errors
        else errors in
      errors
  | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
    when token_kind prefix_unary_operator = Some TokenKind.Ampersand ->
    check_reference prefix_unary_operand errors
  | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
    when token_kind prefix_unary_operator = Some TokenKind.Dollar ->
    let original_node = node in
    let rec check_prefix_unary_dollar node =
      match syntax node with
      | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
        when token_kind prefix_unary_operator = Some TokenKind.Dollar ->
        check_prefix_unary_dollar prefix_unary_operand
      | BracedExpression _
      | SubscriptExpression _
      | VariableExpression _ -> errors (* these ones are valid *)
      | LiteralExpression _
      | PipeVariableExpression _ -> errors (* these ones get caught later *)
      | _ -> make_error_from_node original_node SyntaxError.dollar_unary :: errors
    in
    check_prefix_unary_dollar prefix_unary_operand
  (* TODO(T21285960): Remove this bug-port, stemming from T22184312 *)
  | LambdaExpression { lambda_async; lambda_coroutine; lambda_signature; _ }
    when is_hhvm_compat env
      && not (is_missing lambda_async)
      && trailing_width lambda_async = 0
      && full_width lambda_coroutine = 0
      && leading_width lambda_signature = 0
      ->
        make_error_from_node node
          (SyntaxError.error1057 "==>") :: errors
    (* End of bug-port *)
  | IsExpression
    { is_right_operand = hint
    ; _
    }
  | AsExpression
    { as_right_operand = hint
    ; _
    } ->
    let n = match syntax node with IsExpression _ -> "is" | _ -> "as" in
    begin match syntax hint with
      | ClosureTypeSpecifier _ when env.hhvm_compat_mode <> NoCompat ->
        make_error_from_node hint
          (SyntaxError.invalid_is_as_expression_hint n "Callable") :: errors
      | SoftTypeSpecifier _ ->
        make_error_from_node hint
          (SyntaxError.invalid_is_as_expression_hint n "Soft") :: errors
      | _ -> errors
    end
  | ConditionalExpression
    { conditional_test
    ; conditional_consequence = cons
    ; conditional_alternative
    ; _ } ->
    let errors =
      if is_missing cons && is_typechecker env
      then make_error_from_node node SyntaxError.elvis_operator_space :: errors
      else errors in
    let errors =
      if is_conditional_expression conditional_test && is_typechecker env
      then make_error_from_node node SyntaxError.nested_ternary :: errors
      else errors in
    begin match conditional_alternative with
    | { syntax = LambdaExpression { lambda_body; _ }; _ }
      when is_conditional_expression lambda_body && is_typechecker env ->
      make_error_from_node node SyntaxError.nested_ternary :: errors
    | _ -> errors end
  | LambdaExpression
    { lambda_attribute_spec = s
    ; lambda_body = body
    ; _ } ->
    let errors = no_memoize_attribute_on_lambda s errors in
    no_async_before_lambda_body env body errors
  | AnonymousFunction { anonymous_attribute_spec = s; _ }
  | Php7AnonymousFunction { php7_anonymous_attribute_spec = s; _ }
  | AwaitableCreationExpression { awaitable_attribute_spec = s; _ }
    -> no_memoize_attribute_on_lambda s errors
  | CollectionLiteralExpression
    { collection_literal_name = n
    ; collection_literal_initializers = initializers
    ; _ } ->
    let is_standard_collection lc_name =
      lc_name = "pair" || lc_name = "vector" || lc_name = "map" ||
      lc_name = "set"  || lc_name = "immvector" || lc_name = "immmap" ||
      lc_name = "immset" in
    let is_qualified_std_collection l r =
      token_kind l = Some TokenKind.Name &&
      token_kind r = Some TokenKind.Name &&
      String.lowercase (text l) = "hh" &&
      is_standard_collection (String.lowercase (text r)) in
    let status =
      match syntax n with
      (* non-qualified name *)
      | SimpleTypeSpecifier { simple_type_specifier = ({
          syntax = Token t; _
        } as n);_ }
      | GenericTypeSpecifier { generic_class_type = ({
          syntax = Token t; _
        } as n);_ }
        when Token.kind t = TokenKind.Name ->
        begin match String.lowercase (text n) with
        | "dict" | "vec" | "keyset" -> `InvalidBraceKind
        | n -> if is_standard_collection n then `ValidClass n else `InvalidClass
        end
      (* qualified name *)
      | SimpleTypeSpecifier { simple_type_specifier = {
          syntax = QualifiedName { qualified_name_parts = parts; _ }; _
        };_ }
      | GenericTypeSpecifier { generic_class_type = {
          syntax = QualifiedName { qualified_name_parts = parts; _ }; _
        };_ } ->
        begin match syntax_to_list false parts with
        (* HH\Vector in global namespace *)
        | [l; r]
          when namespace_name = global_namespace_name &&
          is_qualified_std_collection l r ->
          `ValidClass (String.lowercase (text r))
        (* \HH\Vector *)
        | [{ syntax = Missing; _}; l; r]
          when is_qualified_std_collection l r ->
          `ValidClass (String.lowercase (text r))
        | _ -> `InvalidClass
        end
      | _ -> `InvalidClass in
    let num_initializers =
      List.length (syntax_to_list_no_separators initializers) in
    let errors = begin match status with
    | `ValidClass "pair" when num_initializers <> 2 ->
      let msg =
        if num_initializers = 0
        then SyntaxError.pair_initializer_needed
        else SyntaxError.pair_initializer_arity
      in
      let e =
        make_error_from_node node msg ~error_type:SyntaxError.RuntimeError in
      e :: errors
    | `ValidClass _ -> errors
    | `InvalidBraceKind ->
      let e =
        make_error_from_node node SyntaxError.invalid_brace_kind_in_collection_initializer in
      e :: errors
    | `InvalidClass ->
      let e =
        make_error_from_node node SyntaxError.invalid_class_in_collection_initializer in
      e :: errors
    end in
    check_collection_members initializers errors
  | PipeVariableExpression _
    when ParserOptions.enable_await_as_an_expression env.parser_options ->
    let closest_pipe_operator = List.find_exn parents ~f:begin fun node ->
      match syntax node with
      | BinaryExpression { binary_operator; _ }
        when token_kind binary_operator = Some TokenKind.BarGreaterThan ->
        true
      | _ -> false
      end in

    let closest_pipe_left_operand = match syntax closest_pipe_operator with
    | BinaryExpression { binary_left_operand; _ } -> binary_left_operand
    | _ -> failwith "Unexpected non-BinaryExpression" in

    (* If the left side of the pipe operation contains an await then we treat
       the $$ as an await *)
    if node_has_await_child closest_pipe_left_operand
    then await_as_an_expression_errors node parents errors
    else errors
  | DecoratedExpression { decorated_expression_decorator = op; _ }
  | PrefixUnaryExpression { prefix_unary_operator = op; _ }
    when token_kind op = Some TokenKind.Await ->
    if ParserOptions.enable_await_as_an_expression env.parser_options
      then await_as_an_expression_errors node parents errors
      else
    begin match parents with
      | si :: le :: _ when is_simple_initializer si && is_let_statement le ->
        errors
      | le :: _ when is_lambda_expression le -> errors
      | rs :: _ when is_return_statement rs -> errors
      | es :: _ when is_expression_statement es -> errors
      | be :: es :: _
        when is_binary_expression be && is_assignment be &&
          is_expression_statement es -> errors
      | li :: l :: us :: _
        when is_list_item li && is_list l &&
          (is_using_statement_block_scoped us ||
           is_using_statement_function_scoped us) -> errors
      | be :: li :: l :: us :: _
        when is_binary_expression be && is_assignment be &&
          is_list_item li && is_list l &&
          (is_using_statement_block_scoped us ||
           is_using_statement_function_scoped us) -> errors
      | be :: us :: _
         when is_binary_expression be && is_assignment be &&
           (is_using_statement_block_scoped us ||
            is_using_statement_function_scoped us) -> errors
      | us :: _
         when (is_using_statement_block_scoped us ||
               is_using_statement_function_scoped us) -> errors
      | _ -> make_error_from_node node SyntaxError.invalid_await_use :: errors
    end
  | _ -> errors (* Other kinds of expressions currently produce no expr errors. *)

let check_repeated_properties full_name (errors, p_names) prop =
  match syntax prop with
  | PropertyDeclaration { property_declarators; _} ->
    let declarators = syntax_to_list_no_separators property_declarators in
      List.fold declarators ~init:(errors, p_names)
        ~f:begin fun (errors, p_names) prop ->
            match syntax prop with
            | PropertyDeclarator {property_name; _} ->
                let prop_name = text property_name in
                (* If the property name is empty, then there was an earlier
                  parsing error that should supercede this one. *)
                if prop_name = "" then errors, p_names else
                let errors, p_names =
                  if SSet.mem prop_name p_names
                  then make_error_from_node prop
                  (SyntaxError.redeclaration_error
                      ((Utils.strip_ns full_name) ^ "::" ^ prop_name)) :: errors, p_names
                  else errors, SSet.add prop_name p_names in
                errors, p_names
            | _ -> errors, p_names
          end
  | _ -> (errors, p_names)
let require_errors _env node trait_use_clauses errors =
  match syntax node with
  | RequireClause p ->
    let name = text p.require_name in
    let req_kind = token_kind p.require_kind in
    let trait_use_clauses, errors =
      match strmap_get name trait_use_clauses, req_kind with
      | None, Some tk -> strmap_add name tk trait_use_clauses, errors
      | Some tk1, Some tk2 when tk1 = tk2 -> (* duplicate, it is okay *)
        trait_use_clauses, errors
      | _ -> (* Conflicting entry *)
        trait_use_clauses,
        make_error_from_node node
          (SyntaxError.conflicting_trait_require_clauses ~name) :: errors
    in
    let errors =
      match (active_classish_kind _env.context, req_kind) with
      | (Some TokenKind.Interface, Some TokenKind.Implements)
      | (Some TokenKind.Class, Some TokenKind.Implements) ->
        make_error_from_node node SyntaxError.error2030 :: errors
      | _ -> errors
    in
    trait_use_clauses, errors
  | _ -> trait_use_clauses, errors

let check_type_name syntax_tree name namespace_name name_text location names errors =
  begin match strmap_get name_text names.t_classes with
  | Some { f_location = location; f_kind; f_name; _ }
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
        t_classes = strmap_add name_text def names.t_classes} in
    names, errors
  end

let get_type_params_and_emit_shadowing_errors l errors =
  syntax_to_list_no_separators l
  |> List.fold_right ~init:(SSet.empty, errors)
      ~f:(fun p (s, e) -> match syntax p with
        | TypeParameter { type_reified; type_name; _}
          when not @@ is_missing type_reified ->
          let name = text type_name in
          if SSet.mem name s then
            (s, make_error_from_node p SyntaxError.shadowing_reified :: e)
          else (SSet.add name s, e)
        | _ -> s, e)

let reified_parameter_errors node errors =
  match syntax node with
  | FunctionDeclarationHeader {
    function_type_parameter_list = {
      syntax = TypeParameters {
        type_parameters_parameters; _}; _}; _} ->
        snd @@ get_type_params_and_emit_shadowing_errors
          type_parameters_parameters errors
  | _ -> errors

let class_reified_param_errors env node errors =
  match syntax node with
  | ClassishDeclaration cd ->
    let reified_params, errors = match syntax cd.classish_type_parameters with
      | TypeParameters { type_parameters_parameters; _ } ->
        get_type_params_and_emit_shadowing_errors
          type_parameters_parameters errors
      | _ -> SSet.empty, errors in
    let add_error e acc = match syntax e with
      | TypeParameter { type_reified; type_name; _}
        when not @@ is_missing type_reified &&
          SSet.mem (text type_name) reified_params ->
          make_error_from_node e SyntaxError.shadowing_reified :: acc
      |_ -> acc in
    let check_method e acc = match syntax e with
      | MethodishDeclaration {
          methodish_function_decl_header =  {
            syntax = FunctionDeclarationHeader {
              function_type_parameter_list = {
                syntax = TypeParameters {
                  type_parameters_parameters; _}; _}; _}; _}; _} ->
        syntax_to_list_no_separators type_parameters_parameters
        |> List.fold_right ~init:acc ~f:add_error
      | _ -> acc in
    let errors = match syntax cd.classish_body with
      | ClassishBody { classish_body_elements; _} ->
        syntax_to_list_no_separators classish_body_elements
        |> List.fold_right ~init:errors ~f:check_method
      | _ -> errors in
    let errors = if is_token_kind cd.classish_keyword TokenKind.Interface &&
       not @@ SSet.is_empty reified_params then
         make_error_from_node node SyntaxError.reified_in_interface :: errors
       else errors in
    errors
  | PropertyDeclaration _ ->
    if methodish_contains_static node && is_in_reified_class env.context then
      make_error_from_node node SyntaxError.static_property_in_reified_class
      :: errors else errors
  | _ -> errors

let attr_spec_contains_sealed node =
  attribute_specification_contains node SN.UserAttributes.uaSealed

let classish_errors env node namespace_name names errors =
  match syntax node with
  | ClassishDeclaration cd ->
    (* Given a ClassishDeclaration node, test whether or not it's a trait
     * invoking the 'extends' keyword. *)
    let classish_invalid_extends_keyword _ =
      (* Invalid if uses 'extends' and is a trait. *)
      token_kind cd.classish_extends_keyword = Some TokenKind.Extends &&
        token_kind cd.classish_keyword = Some TokenKind.Trait in

    (* Given a sealed ClassishDeclaration node, test whether all the params
     * are classnames. *)
    let classish_sealed_arg_not_classname _env _ =
      match cd.classish_attribute.syntax with
      | AttributeSpecification { attribute_specification_attributes = attrs; _ } ->
        let attrs = syntax_to_list_no_separators attrs in
        List.exists attrs (fun e ->
          match syntax e with
          | ConstructorCall {constructor_call_argument_list; constructor_call_type; _ } ->
            text constructor_call_type = SN.UserAttributes.uaSealed &&
            List.exists (syntax_to_list_no_separators constructor_call_argument_list) (fun e ->
              match syntax e with
              | ScopeResolutionExpression {scope_resolution_name; _ } ->
                text scope_resolution_name <> "class"
              | _ -> true)
          | _ -> false)
      | _ -> false in

    let classish_is_sealed =
      attr_spec_contains_sealed cd.classish_attribute in

    (* Given a ClassishDeclaration node, test whether or not length of
     * extends_list is appropriate for the classish_keyword. *)
    let classish_invalid_extends_list env _ =
      (* Invalid if is a class and has list of length greater than one. *)
      (is_typechecker env) &&
      token_kind cd.classish_keyword = Some TokenKind.Class &&
        token_kind cd.classish_extends_keyword = Some TokenKind.Extends &&
        match syntax_to_list_no_separators cd.classish_extends_list with
        | [_] -> false
        | _ -> true (* General bc empty list case is already caught by error1007 *) in
    let abstract_keyword =
      Option.value (extract_keyword is_abstract node) ~default:node  in
    let errors = produce_error errors
      (is_classish_kind_declared_abstract env)
      node SyntaxError.error2042 abstract_keyword in
    (* Given a ClassishDeclaration node, test whether it is sealed and final. *)
    let classish_sealed_final _env _ =
      list_contains_predicate is_final cd.classish_modifiers &&
      classish_is_sealed in
    let errors = produce_error errors (classish_invalid_extends_list env) ()
      SyntaxError.error2037 cd.classish_extends_list in

    let errors =
      match attribute_first_reactivity_annotation cd.classish_attribute with
      | Some n ->
        make_error_from_node n SyntaxError.misplaced_reactivity_annotation :: errors
      | None ->
        errors in
    let errors =
      produce_error errors
      classish_duplicate_modifiers cd.classish_modifiers
      SyntaxError.error2031 cd.classish_modifiers in
    let errors =
      produce_error errors
      (classish_sealed_arg_not_classname env) ()
      SyntaxError.sealed_val_not_classname cd.classish_attribute in
    let errors =
      produce_error errors
      classish_invalid_extends_keyword ()
      SyntaxError.error2036 cd.classish_extends_keyword in
    let errors =
      produce_error errors
      (classish_sealed_final env) ()
      SyntaxError.sealed_final cd.classish_attribute in
    let errors =
      let classish_name = text cd.classish_name in
      produce_error errors
      (cant_be_classish_name env) classish_name
      (SyntaxError.reserved_keyword_as_class_name classish_name)
      cd.classish_name in
    let errors =
      if is_token_kind cd.classish_keyword TokenKind.Interface &&
        not (is_missing cd.classish_implements_keyword)
      then make_error_from_node node
        SyntaxError.interface_implements :: errors
      else errors in
    let name = text cd.classish_name in
    let errors =
      match syntax cd.classish_body with
      | ClassishBody {classish_body_elements = methods; _} ->
        let methods = syntax_to_list_no_separators methods in
        let declared_name_str =
          Option.value ~default:"" (Syntax.extract_text cd.classish_name) in
        let full_name = combine_names namespace_name declared_name_str in
        let errors, _ =
          List.fold methods ~f:(check_repeated_properties full_name)
            ~init:(errors, SSet.empty) in
        let has_abstract_fn =
          List.exists methods ~f:methodish_contains_abstract in
        let has_private_method =
          List.exists methods
            ~f:(methodish_modifier_contains_helper is_private) in
        let has_multiple_xhp_category_decls =
          let cats = List.filter methods ~f:(fun m ->
            match syntax m with
            | XHPCategoryDeclaration _ -> true
            | _ -> false) in
          match cats with
          | [] | [_] -> false
          | _ -> true in
        let errors =
          if has_abstract_fn &&
             is_token_kind cd.classish_keyword TokenKind.Class &&
             not (list_contains_predicate is_abstract cd.classish_modifiers)
          then make_error_from_node node
                (SyntaxError.class_with_abstract_method name) :: errors
          else errors in
        let errors =
          if has_private_method &&
             is_token_kind cd.classish_keyword TokenKind.Interface
          then make_error_from_node node
            SyntaxError.interface_has_private_method :: errors
          else errors in
        let errors =
          if has_multiple_xhp_category_decls
          then make_error_from_node node
            SyntaxError.xhp_class_multiple_category_decls :: errors
          else errors in
        errors
      | _ -> errors in
    let names, errors =
      match token_kind cd.classish_keyword with
      | Some TokenKind.Class | Some TokenKind.Trait
        when not (is_missing cd.classish_name)->
        let location = make_location_of_node cd.classish_name in
        check_type_name env.syntax_tree cd.classish_name namespace_name name location names errors
      | _ ->
        names, errors in
    names, errors
  | _ -> names, errors

let class_element_errors env node errors =
  match syntax node with
  | ConstDeclaration _ when is_inside_trait env.context ->
    make_error_from_node node SyntaxError.const_in_trait :: errors
  | ConstDeclaration { const_visibility; _ }
    when not (is_missing const_visibility) && env.is_hh_file && not env.codegen ->
      make_error_from_node node SyntaxError.const_visibility :: errors
  | _ -> errors


let alias_errors env node namespace_name names errors =
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
    let errors = match ad.alias_type with
    | {
      syntax = TypeConstant _;
      _;
    } when is_typechecker env ->
      make_error_from_node ad.alias_type
        SyntaxError.type_alias_to_type_constant :: errors
    | _ -> errors in
    check_type_name env.syntax_tree ad.alias_name namespace_name name location names errors
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

let group_use_errors _env node errors =
  match syntax node with
  | NamespaceGroupUseDeclaration
    { namespace_group_use_prefix = prefix
    ; namespace_group_use_clauses = clauses
    ; namespace_group_use_kind = kind
    ; _} ->
      let errors =
        let invalid_clauses = List.filter ~f:(is_invalid_group_use_clause kind)
          (syntax_to_list_no_separators clauses) in
        let mapper errors clause =
          make_error_from_node clause SyntaxError.error2049 :: errors in
        List.fold_left ~f:mapper ~init:errors invalid_clauses in
      produce_error errors is_invalid_group_use_prefix prefix
        SyntaxError.error2048 prefix
  | _ -> errors

let use_class_or_namespace_clause_errors
  env is_global_namespace namespace_prefix
  kind (names, errors) cl =

  match syntax cl with
  | NamespaceUseClause {
      namespace_use_name  = name;
      namespace_use_alias = alias;
      namespace_use_clause_kind; _
    } when not (is_missing name) ->
    let kind = if is_missing kind then namespace_use_clause_kind else kind in
    let name_text = text name in
    let qualified_name =
      match namespace_prefix with
      | None -> combine_names global_namespace_name name_text
      | Some p -> combine_names p name_text in
    let short_name = get_short_name_from_qualified_name name_text (text alias) in

    let do_check ?(case_sensitive = false) ~error_on_global_redefinition names errors
      get_map update_map report_error =
      (* We store the original names in the SMap of names for error messaging purposes
        but we check for case sensitivity specifically. *)
      let find_name name =
        if case_sensitive
        then short_name = name
        else (String.lowercase short_name) = String.lowercase name in
      let map = get_map names in
      match strmap_find_first_opt find_name map with
      | Some (_, { f_location = location; f_kind; f_global; _ }) ->
        if (f_kind <> Name_def
           || (error_on_global_redefinition && (is_global_namespace || f_global)))
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
        update_map names (strmap_add short_name new_use map), errors
    in

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
        do_check ~case_sensitive:true ~error_on_global_redefinition:true names errors
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
            if is_hack env then SyntaxError.strict_namespace_hh
            else SyntaxError.strict_namespace_not_hh in
          make_error_from_node name message :: errors
        else errors in

      let names, errors =
        let location = make_location_of_node name in
        match strmap_get short_name names.t_classes with
        | Some { f_location = loc; f_name; f_kind; _ } ->
          if qualified_name = f_name && f_kind = Name_def then names, errors
          else
            let err_msg =
              if is_hack env && f_kind <> Name_def then
                let text = SyntaxTree.text env.syntax_tree in
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
          let t_classes = strmap_add short_name new_use names.t_classes in
          let t_namespaces =
            if strmap_mem short_name names.t_namespaces
            then names.t_namespaces
            else strmap_add short_name new_use names.t_namespaces in
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
  env node is_global_namespace names errors =
  match syntax node with
  | NamespaceUseDeclaration {
      namespace_use_kind = kind;
      namespace_use_clauses = clauses; _ } ->
    let f =
      use_class_or_namespace_clause_errors
        env is_global_namespace None kind in
    List.fold_left ~f ~init:(names, errors) (syntax_to_list_no_separators clauses)
  | NamespaceGroupUseDeclaration {
      namespace_group_use_kind = kind;
      namespace_group_use_clauses = clauses;
      namespace_group_use_prefix = prefix; _ } ->
    let f =
      use_class_or_namespace_clause_errors
        env is_global_namespace (Some (text prefix)) kind in
    List.fold_left ~f ~init:(names, errors) (syntax_to_list_no_separators clauses)
  | _ -> names, errors


let rec check_constant_expression errors node =
  let is_namey token =
    match Token.kind token with
    TokenKind.Name -> true
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
      |> List.fold_left ~init:errors ~f:check_constant_expression
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
    |> List.fold_left ~init:errors ~f:check_constant_expression
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

let check_static_in_initializer initializer_ =
  match syntax initializer_ with
  | SimpleInitializer {
    simple_initializer_value = {
      syntax = ScopeResolutionExpression {
        scope_resolution_qualifier = { syntax = Token t; _ };
        scope_resolution_name = name;
        _}; _}; _
  } ->
    begin match Token.kind t with
      | TokenKind.Static -> true
      | TokenKind.Parent when (String.lowercase @@ text name = "class") -> true
      | _ -> false
    end
  | _ -> false

let const_decl_errors env node namespace_name names errors =
  match syntax node with
  | ConstantDeclarator cd ->
    let errors =
      produce_error_context errors constant_abstract_with_initializer
      cd.constant_declarator_initializer env.context
      SyntaxError.error2051 cd.constant_declarator_initializer in
    let errors =
      produce_error_context errors constant_concrete_without_initializer
      cd.constant_declarator_initializer env.context SyntaxError.error2050
      cd.constant_declarator_initializer in
    let errors =
      produce_error errors is_global_in_const_decl cd.constant_declarator_initializer
      SyntaxError.global_in_const_decl cd.constant_declarator_initializer in
    let errors =
      check_constant_expression errors cd.constant_declarator_initializer in
    let errors =
      produce_error errors check_static_in_initializer cd.constant_declarator_initializer
      SyntaxError.parent_static_const_decl cd.constant_declarator_initializer in
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
      match strmap_get constant_name names.t_constants with
      | None -> errors
      | Some _ ->
        (* Only error if this is inside a class *)
        begin match first_parent_class_name env.context with
          | None -> errors
          | Some class_name ->
            let full_name = class_name ^ "::" ^ constant_name in
            make_error_from_node
              node (SyntaxError.redeclaration_error full_name) :: errors
        end
    in
    let names = {
      names with t_constants =
        strmap_add constant_name def names.t_constants } in
    names, errors
  | _ -> names, errors


let class_property_visibility_errors env node errors =
  match syntax node with
  | PropertyDeclaration { property_modifiers; _ } ->
    let first_parent_name = Option.value (first_parent_class_name env.context)
      ~default:"" in
    let errors =
      produce_error errors
        declaration_multiple_visibility node
        (SyntaxError.property_has_multiple_visibilities first_parent_name) property_modifiers
    in
    let errors =
      produce_error errors
        is_empty_list_or_missing property_modifiers
        SyntaxError.property_requires_visibility node
    in
    errors
  | _ -> errors


let mixed_namespace_errors env node parents namespace_type errors =
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
        let decls = if not (is_systemlib_compat env) then decls else
          (* Drop everything before yourself *)
          fst @@ List.fold_right decls
            ~init:([], false)
            ~f:(fun n (l, seen as acc) ->
              if seen then acc else (n::l, phys_equal n node))
        in
        let rec is_first l =
          match l with
          | { syntax = MarkupSection {markup_text; _}; _} :: rest
            when width markup_text = 0 || is_hashbang markup_text ->
            is_first rest
          | { syntax = ( DeclareDirectiveStatement _
                       | DeclareBlockStatement _
                       | NamespaceUseDeclaration _
                       | FileAttributeSpecification _
                       ); _} :: rest ->
            is_first rest
          | { syntax = NamespaceDeclaration _; _} :: _ -> true
          | _ -> false
        in
        let has_code_outside_namespace =
          not (is_namespace_empty_body namespace_body) &&
          List.exists decls
            ~f:(function | { syntax = MarkupSection { markup_text; _}; _}
                           when width markup_text = 0
                             || is_hashbang markup_text -> false
                         | { syntax = NamespaceDeclaration _; _}
                         | { syntax = DeclareDirectiveStatement _; _}
                         | { syntax = DeclareBlockStatement _; _}
                         | { syntax = ExpressionStatement {
                            expression_statement_expression =
                            { syntax = HaltCompilerExpression _; _}; _}; _}
                         | { syntax = FileAttributeSpecification _; _}
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

let enumerator_errors node errors =
  match syntax node with
  | Enumerator { enumerator_name = name; enumerator_value = value; _} ->
    let errors = if String.lowercase @@ text name = "class" then
      make_error_from_node node SyntaxError.enum_elem_name_is_class :: errors
      else errors in
    let errors = check_constant_expression errors value in
    errors
  | _ -> errors

let enum_decl_errors node errors =
  match syntax node with
  EnumDeclaration
   { enum_attribute_spec = attrs
   (*
   ; enum_name           = name
   ; enum_base           = base
   ; enum_type           = constr
   ; enum_enumerators    = enums
   *)
   ; _ } ->
    if attr_spec_contains_sealed attrs then
      make_error_from_node node SyntaxError.sealed_enum :: errors
      else errors
  | _ -> errors

let assignment_errors _env node errors =
  let append_errors node errors error =
    make_error_from_node node error :: errors
  in
  let rec check_lvalue ?(allow_reassign_this=false) loperand errors : SyntaxError.t list =
    let err = append_errors loperand errors in
    match syntax loperand with
      | ListExpression { list_members = members; _ } ->
        let members = syntax_to_list_no_separators members in
        List.fold_left ~f:(fun e n -> check_lvalue n e) ~init:errors members
      | SafeMemberSelectionExpression _ ->
        err (SyntaxError.not_allowed_in_write "?-> operator")
      | MemberSelectionExpression { member_name; _ }
        when token_kind member_name = Some TokenKind.XHPClassName ->
        err (SyntaxError.not_allowed_in_write "->: operator")
      | VariableExpression { variable_expression }
        when not allow_reassign_this
          && String.lowercase (text variable_expression) = SN.SpecialIdents.this ->
        err SyntaxError.reassign_this
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.Clone ->
        err (SyntaxError.not_allowed_in_write "Clone")
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.Await ->
        err (SyntaxError.not_allowed_in_write "Await")
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.Suspend ->
        err (SyntaxError.not_allowed_in_write "Suspend")
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.QuestionQuestion ->
        err (SyntaxError.not_allowed_in_write "?? operator")
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.BarGreaterThan ->
        err (SyntaxError.not_allowed_in_write "|> operator")
      | DecoratedExpression { decorated_expression_decorator = op; _ }
        when token_kind op = Some TokenKind.Inout ->
        err (SyntaxError.not_allowed_in_write "Inout")
      | ParenthesizedExpression { parenthesized_expression_expression = e; _} ->
        check_lvalue ~allow_reassign_this e errors
      | SubscriptExpression { subscript_receiver = e; _ } ->
        check_lvalue ~allow_reassign_this:true e errors
      | LambdaExpression _ | AnonymousFunction _ | Php7AnonymousFunction _
      | AwaitableCreationExpression _
      | ArrayIntrinsicExpression _ | ArrayCreationExpression _
      | DarrayIntrinsicExpression _
      | VarrayIntrinsicExpression _
      | ShapeExpression _
      | CollectionLiteralExpression _
      | GenericTypeSpecifier _
      | YieldExpression _ | YieldFromExpression _
      | CastExpression _
      | BinaryExpression _
      | ConditionalExpression _
      | InstanceofExpression _
      | IsExpression _
      | AsExpression _ | NullableAsExpression _
      | ConstructorCall _ | AnonymousClass _
      | XHPExpression _
      | InclusionExpression _
      | TupleExpression _
      | LiteralExpression _ ->
        let kind = kind loperand in
        let kind_str = Full_fidelity_syntax_kind.to_string kind in
        err (SyntaxError.not_allowed_in_write kind_str)
      | PrefixUnaryExpression { prefix_unary_operator = op; _ }
      | PostfixUnaryExpression { postfix_unary_operator = op; _ } ->
        begin match token_kind op with
          | Some TokenKind.At | Some TokenKind.Dollar -> errors
          | Some TokenKind.Ampersand ->
            err (SyntaxError.not_allowed_in_write "&")
          | _ -> err (SyntaxError.not_allowed_in_write "Unary expression")
        end (* match *)
      (* FIXME: Array_get ((_, Class_const _), _) is not a valid lvalue. *)
      | _ -> errors
        (* Ideally we should put all the rest of the syntax here so everytime
         * a new syntax is added people need to consider whether the syntax
         * can be a valid lvalue or not. However, there are too many of them. *)
  in
  let check_rvalue roperand errors : SyntaxError.t list =
    match syntax roperand with
    | PrefixUnaryExpression { prefix_unary_operator = op; _ }
        when token_kind op = Some TokenKind.Ampersand ->
      append_errors roperand errors (SyntaxError.references_not_allowed)
    | _ -> errors
  in
  match syntax node with
  | (PrefixUnaryExpression
    { prefix_unary_operator = op
    ; prefix_unary_operand = loperand
    }
  | PostfixUnaryExpression
    { postfix_unary_operator = op
    ; postfix_unary_operand = loperand
    }
  ) when does_unop_create_write (token_kind op) ->
    check_lvalue ~allow_reassign_this:true loperand errors
  | DecoratedExpression
    { decorated_expression_decorator = op
    ; decorated_expression_expression = loperand
    } when does_decorator_create_write (token_kind op) ->
    check_lvalue ~allow_reassign_this:true loperand errors
  | BinaryExpression
    { binary_left_operand = loperand
    ; binary_operator = op
    ; binary_right_operand = roperand
    } when does_binop_create_write_on_left (token_kind op) ->
      let errors = check_lvalue loperand errors in
      check_rvalue roperand errors
  | ForeachStatement
    { foreach_key = k;
      foreach_value = v;
      _
    } ->
    check_lvalue k @@ check_lvalue v errors
  | _ -> errors


let declare_errors _env node parents errors =
  match syntax node with
  | DeclareDirectiveStatement { declare_directive_expression = expr; _}
  | DeclareBlockStatement { declare_block_expression = expr; _} ->
    let errors =
      match syntax expr with
      | BinaryExpression { binary_right_operand = r; _ } ->
        check_constant_expression errors r
      | _ -> errors in
    let errors =
      match syntax expr with
      | BinaryExpression
        { binary_left_operand = loper
        ; binary_operator = op
        ; _} when token_kind op = Some TokenKind.Equal
             && String.lowercase @@ text loper = "strict_types" ->
        (* Checks if there are only other declares nodes
         * in front of the node in question *)
        let rec is_only_declares_nodes = function
          | ({ syntax = DeclareDirectiveStatement _; _} as e) :: es
          | ({ syntax = DeclareBlockStatement _; _} as e) :: es ->
            phys_equal e node || is_only_declares_nodes es
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
        errors
      | _ -> errors
    in
    errors
  | _ -> errors

let dynamic_method_call_errors node errors =
  match syntax node with
  | FunctionCallExpression
    {
      function_call_type_args = type_args;
      function_call_receiver = receiver; _ } when not (is_missing type_args) ->
    let is_dynamic = match syntax receiver with
      | ScopeResolutionExpression { scope_resolution_name = name; _ }
      | MemberSelectionExpression { member_name = name; _ }
      | SafeMemberSelectionExpression { safe_member_name = name; _ } ->
        is_token_kind name TokenKind.Variable
      | _ -> false
    in
    if not is_dynamic then errors else
    (make_error_from_node node SyntaxError.no_type_parameters_on_dynamic_method_calls) :: errors
  | _ -> errors

let get_namespace_name context current_namespace_name =
  match context.nested_namespaces with
  | { syntax = NamespaceDeclaration { namespace_name = ns; _ }; _ } :: _ ->
    if is_missing ns then current_namespace_name
    else combine_names current_namespace_name (text ns)
  | _ -> current_namespace_name

let is_invalid_hack_mode env errors =
  if SyntaxTree.mode env.syntax_tree = None then
    let root = SyntaxTree.root env.syntax_tree in
    let e = make_error_from_node root SyntaxError.invalid_hack_mode in
    e::errors
  else
    errors

let find_syntax_errors env =
  let has_rx_attr_mutable_hack attrs =
    attribute_first_reactivity_annotation attrs
    |> Option.value_map ~default:false ~f:(fun a ->
      match attribute_matches_criteria ((<>) SN.UserAttributes.uaNonRx) a with
      | Some _ -> true
      | None -> false
    ) in
  let rec folder env acc node parents =
    let { errors
        ; namespace_type
        ; names
        ; namespace_name
        ; trait_require_clauses
        ; is_in_concurrent_block
        } = acc in
    let env =
      { env with context =
        let node_syntax = syntax node in
        match node_syntax with
        | ClassishDeclaration _ ->
          { env.context with active_classish = Some node }
        | FunctionDeclaration { function_attribute_spec = s; _ }
        | MethodishDeclaration { methodish_attribute = s; _ } ->
          (* named functions *)
          { env.context with
            (* a _single_ variable suffices as they cannot be nested *)
            active_methodish = Some node;
            (* inspect the rx attribute directly. *)
            active_is_rx_or_enclosing_for_lambdas = has_rx_attr_mutable_hack s;
            active_callable = Some node;
            active_callable_attr_spec = Some s;
          }
        | AnonymousFunction { anonymous_attribute_spec = s; _ }
        | Php7AnonymousFunction { php7_anonymous_attribute_spec = s; _ }
        | LambdaExpression { lambda_attribute_spec = s; _ }
        | AwaitableCreationExpression { awaitable_attribute_spec = s; _ } ->
          (* preserve context when entering lambdas (and anonymous functions) *)
          { env.context with
            active_callable = Some node;
            active_callable_attr_spec = Some s;
          }
        | ConstDeclaration _ ->
          { env.context with active_const = Some node }
        | NamespaceDeclaration { namespace_name; _ }
          when not @@ is_missing namespace_name && text namespace_name <> "" ->
          { env.context with
            nested_namespaces = node :: env.context.nested_namespaces;
          }
        | _ -> env.context
      } in
    let names, errors =
      parameter_errors env node namespace_name names errors in
    let trait_require_clauses, names, errors =
      match syntax node with
      | TryStatement _
      | UsingStatementFunctionScoped _
      | ForStatement _
      | CaseLabel _
      | DefaultLabel _ ->
        let errors = statement_errors env node parents errors in
        trait_require_clauses, names, errors
      | MethodishDeclaration _
      | FunctionDeclaration _
      | FunctionDeclarationHeader _ ->
        let errors = reified_parameter_errors node errors in
        let names, errors =
          redeclaration_errors env node parents namespace_name names errors in
        let errors =
          methodish_errors env node errors in
        trait_require_clauses, names, errors
      | ArrayCreationExpression _
      | ArrayIntrinsicExpression _ ->
        let errors =
          expression_errors env is_in_concurrent_block namespace_name node parents errors in
        trait_require_clauses, names, errors
      | InstanceofExpression _
      | LiteralExpression _
      | SafeMemberSelectionExpression _
      | HaltCompilerExpression _
      | FunctionCallExpression _
      | ListExpression _
      | ShapeExpression _
      | DecoratedExpression _
      | VectorIntrinsicExpression _
      | DictionaryIntrinsicExpression _
      | KeysetIntrinsicExpression _
      | VarrayIntrinsicExpression _
      | DarrayIntrinsicExpression _
      | YieldFromExpression _
      | YieldExpression _
      | ScopeResolutionExpression _
      | PrefixUnaryExpression _
      | LambdaExpression _
      | IsExpression _
      | AsExpression _
      | AnonymousFunction _
      | Php7AnonymousFunction _
      | SubscriptExpression _
      | ConstructorCall _
      | AwaitableCreationExpression _
      | PipeVariableExpression _
      | ConditionalExpression _
      | CollectionLiteralExpression _
      | VariableExpression _ ->
        let errors = dynamic_method_call_errors node errors in
        let errors =
          expression_errors env is_in_concurrent_block namespace_name node parents errors in
        let errors = check_nonrx_annotation node errors in
        let errors = assignment_errors env node errors in
        trait_require_clauses, names, errors
      | RequireClause _ ->
        let trait_require_clauses, errors =
          require_errors env node trait_require_clauses errors in
        trait_require_clauses, names, errors
      | ClassishDeclaration _ ->
        let names, errors =
          classish_errors env node namespace_name names errors in
        let errors = class_reified_param_errors env node errors in
        trait_require_clauses, names, errors
      | ConstDeclaration _ ->
        let errors =
          class_element_errors env node errors in
        trait_require_clauses, names, errors
      | AliasDeclaration _ ->
        let names, errors = alias_errors env node namespace_name names errors in
        trait_require_clauses, names, errors
      | ConstantDeclarator _ ->
        let names, errors =
          const_decl_errors env node namespace_name names errors in
        trait_require_clauses, names, errors
      | NamespaceBody _
      | NamespaceEmptyBody _
      | NamespaceDeclaration _ ->
        let errors =
          mixed_namespace_errors env node parents namespace_type errors in
        trait_require_clauses, names, errors
      | NamespaceUseDeclaration _
      | NamespaceGroupUseDeclaration _ ->
        let errors = group_use_errors env node errors in
        let names, errors =
          namespace_use_declaration_errors env node
            (namespace_name = global_namespace_name) names errors in
        trait_require_clauses, names, errors
      | PropertyDeclaration _ ->
        let errors = class_property_visibility_errors env node errors in
        let errors = class_reified_param_errors env node errors in
        trait_require_clauses, names, errors
      | EnumDeclaration _ ->
        let errors = enum_decl_errors node errors in
        trait_require_clauses, names, errors
      | Enumerator _ ->
        let errors = enumerator_errors node errors in
        trait_require_clauses, names, errors
      | PostfixUnaryExpression _
      | BinaryExpression _
      | ForeachStatement _ ->
        let errors = assignment_errors env node errors in
        trait_require_clauses, names, errors
      | DeclareDirectiveStatement _
      | DeclareBlockStatement _ ->
        let errors = declare_errors env node parents errors in
        trait_require_clauses, names, errors
      | XHPEnumType _
      | XHPExpression _ ->
        let errors = xhp_errors env node errors in
        trait_require_clauses, names, errors
      | PropertyDeclarator { property_initializer = init; _ } ->
        let errors =
          produce_error errors check_static_in_initializer init
          SyntaxError.parent_static_prop_decl init in
        let errors = check_constant_expression errors init in
        trait_require_clauses, names, errors

      | XHPClassAttribute { xhp_attribute_decl_initializer = init; _ } ->
        let errors = check_constant_expression errors init in
        trait_require_clauses, names, errors
      | _ -> trait_require_clauses, names, errors in

    let errors = lval_errors env node parents errors in

    match syntax node with
    | LambdaExpression _
    | AwaitableCreationExpression _
    | AnonymousFunction _
    | Php7AnonymousFunction _ ->
      (* reset is_in_concurrent_block for functions *)
      let acc1 =
        make_acc
          acc errors namespace_type names
          namespace_name trait_require_clauses
          false in
      (* analyze the body of lambda block *)
      let acc1 = fold_child_nodes env folder node parents acc1 in
      (* adjust is_in_concurrent_block in final result *)
      make_acc
        acc acc1.errors acc1.namespace_type acc1.names
        acc1.namespace_name acc1.trait_require_clauses
        is_in_concurrent_block

    | ConcurrentStatement { concurrent_statement = { syntax = statement; _ }; _ } ->
      (* issue error if concurrent blocks are nested *)
      let errors =
        if is_in_concurrent_block
        then make_error_from_node
          node SyntaxError.nested_concurrent_blocks :: errors
        else errors in

      (* issue error if concurrent block isn't well formed
         - must have at least two statements
         - must be a compound statement
         - must only contain expression statements
         - statement without await
      *)
      let errors = (match statement with
      | CompoundStatement { compound_statements = statements; _ } ->
        let statement_list = syntax_to_list_no_separators statements in
        let errors = (match statement_list with
        | _ :: _ :: _ -> errors
        | _ -> make_error_from_node node
          SyntaxError.less_than_two_statements_in_concurrent_block :: errors
        ) in

        let errors = List.fold_left ~init:errors ~f:(fun errors n ->
          match syntax n with
          | ExpressionStatement { expression_statement_expression = se; _ } ->
            if node_has_await_child se then errors else
              make_error_from_node n
                SyntaxError.statement_without_await_in_concurrent_block :: errors
          | _ -> make_error_from_node n SyntaxError.invalid_syntax_concurrent_block :: errors
        ) statement_list in

        find_invalid_lval_usage errors statement_list
      | _ -> make_error_from_node node SyntaxError.invalid_syntax_concurrent_block :: errors) in

      (* adjust is_in_concurrent_block in accumulator to dive into the
         concurrent block *)
      let acc1 =
        make_acc
          acc errors namespace_type names
          namespace_name trait_require_clauses
          true in
      (* analyze the body of concurrent block *)
      let acc1 = fold_child_nodes env folder node parents acc1 in
      (* adjust is_in_concurrent_block in final result *)
      make_acc
        acc acc1.errors acc1.namespace_type acc1.names
        acc1.namespace_name acc1.trait_require_clauses
        is_in_concurrent_block

    | NamespaceBody { namespace_left_brace; namespace_right_brace; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Bracketed (make_location namespace_left_brace namespace_right_brace)
        else namespace_type in
      (* reset names/namespace_type before diving into namespace body,
         keeping global function names *)
      let namespace_name = get_namespace_name env.context namespace_name in
      let is_global _ f = f.f_global in
      let global_funs names = strmap_filter is_global names.t_functions in
      let new_names = {empty_names with t_functions = global_funs names} in
      let acc1 =
        make_acc
          acc errors namespace_type new_names
          namespace_name empty_trait_require_clauses
          is_in_concurrent_block
      in
      let acc1 = fold_child_nodes env folder node parents acc1 in
      (* add newly declared global functions to the old set of names *)
      let old_names =
        {acc.names with t_functions =
          strmap_union (global_funs acc1.names) acc.names.t_functions}
      in
      (* resume with old set of names and pull back
        accumulated errors/last seen namespace type *)
        make_acc
          acc acc1.errors namespace_type old_names
          acc.namespace_name acc.trait_require_clauses
          acc.is_in_concurrent_block
    | NamespaceEmptyBody { namespace_semicolon; _ } ->
      let namespace_type =
        if namespace_type = Unspecified
        then Unbracketed (make_location_of_node namespace_semicolon)
        else namespace_type
      in
      let namespace_name = get_namespace_name env.context namespace_name in
      (* consider the rest of file to be the part of the namespace:
         reset names and namespace type, keep errors *)
      let acc =
        make_acc
          acc errors namespace_type empty_names
          namespace_name empty_trait_require_clauses
          is_in_concurrent_block
      in
      fold_child_nodes env folder node parents acc
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
      let names = { names with t_constants = YesCase SMap.empty;
                               t_functions = NoCase LSMap.empty } in
      let acc =
        make_acc
          acc errors namespace_type names
          namespace_name empty_trait_require_clauses
          is_in_concurrent_block
      in
      fold_child_nodes ~cleanup env folder node parents acc
    | _ ->
      let acc =
        make_acc
          acc errors namespace_type names
          namespace_name trait_require_clauses
          is_in_concurrent_block
      in
      fold_child_nodes env folder node parents acc in
  let errors =
    if is_typechecker env then is_invalid_hack_mode env []
    else []
  in
  let acc = fold_child_nodes env folder (SyntaxTree.root env.syntax_tree) []
    { errors
    ; namespace_type = Unspecified
    ; names = empty_names
    ; namespace_name = global_namespace_name
    ; trait_require_clauses = empty_trait_require_clauses
    ; is_in_concurrent_block = false
    } in
  acc.errors

let parse_errors_impl env =
  (*
  Minimum: suppress cascading errors; no second-pass errors if there are
  any first-pass errors.
  Typical: suppress cascading errors; give second pass errors always.
  Maximum: all errors
  *)
  let errors1 = match env.level with
  | Maximum -> SyntaxTree.all_errors env.syntax_tree
  | _ -> SyntaxTree.errors env.syntax_tree in
  let errors2 =
    if env.level = Minimum && errors1 <> [] then []
    else find_syntax_errors env in
  List.sort SyntaxError.compare (List.append errors1 errors2)

let parse_errors env =
  Stats_container.wrap_nullary_fn_timing
    ?stats:(Stats_container.get_instance ())
    ~key:"full_fidelity_parse_errors:parse_errors"
    ~f:(fun () -> parse_errors_impl env)

end (* WithSmartConstructors *)

include WithSmartConstructors(SyntaxSmartConstructors.WithSyntax(Syntax))

end (* WithSyntax *)

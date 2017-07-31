(**
* Copyright (c) 2016, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the "hack" directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*
*)

(* The general approach of the pretty printer is to give as much flexibility in
 * terms of where a newline can occur as possible, while maintaining a sensible
 * layout that is considerred human readable and "normal"
 * An optional break is introduced whenever it is possible for a line break to
 * occur in real life. Small components are grouped together so that each
 * component decide their layouts individually.
 * Single line comments is the exception to everything: a newline is forced at
 * the end of a single line comments even if the line otherwise fits. This rule
 * overrides the default behaviour of some operators, such as immediate_cons ^^^
 * which otherwise does not even introduce a break.
 * Design decisions include:
 *  1. Each statement is separated from others using a line break
 *  2. Brackets/parenthesis/braces are either both on different lines with child
 *     or both on the same line with the entire child on one line as well
 *  3. binary operators can be on different lines with arguments
 *  4. unary operators must be on the same line as the argument
 *  5. semicolons have to be on the same line as the last line of the statement
 *     that it ends
 *)

(* The main data type that is used in the pretty printer is a 5 tuple:
 *
 *   (l_trivia, l_single, doc, r_trivia, r_single)
 *
 *   l_trivia: the doc generated from the leading trivia of the syntax node
 *   l_single: whether the leading trivia contains a single line comment
 *   doc     : the doc generated from the main body of the syntax node
 *   r_trivia: the doc generated from trailing trivia of the syntax node
 *   r_single: whether the trailing trivia contains a single line comment
 *)
open Pretty_printing_library_sig
open Limited_width_pretty_printing_library
(* utility functions to make combinators look simpler
 * See Lindig's paper for reference *)
module Utility (P : Library) = struct
  include P

  let space = text " "

  let absorb_nil_cons doc1 doc2 delimiter =
    match doc1, doc2 with
    | Nil, _ -> doc2
    | _, Nil -> doc1
    | _, _ -> doc1 ^^ delimiter ^^ doc2

  let break_cons doc1 doc2 = absorb_nil_cons doc1 doc2 break

  let space_cons doc1 doc2 = absorb_nil_cons doc1 doc2 space

  let choose_cons must is_empty x y =
    let delimiter =
      if must then must_break
      else if is_empty then breakwith ""
      else break
    in
    absorb_nil_cons x y delimiter


  let group_doc (x_lead, x_lead_single, x, x_trail, x_trail_single) =
    let optional_group = if x = Nil then x else group x in
    (x_lead, x_lead_single, optional_group, x_trail, x_trail_single)

  let combine (l_lead, l_lead_single, l, l_trail, l_trail_single) =
    let front_part = choose_cons l_lead_single false l_lead (group l) in
    space_cons front_part l_trail

  (* higher order function to combine two docs with leading and trailing
   * comments. This function creates a front part and a back part in a fixed
   * way and takes a function that combines the two parts in different ways *)
  let doc_combinor x y combinor =
    let (l_lead, l_lead_single, l, l_trail, l_trail_single) = x in
    let (r_lead, r_lead_single, r, r_trail, r_trail_single) = y in
    let front_p = space_cons l l_trail in
    let end_p = choose_cons r_lead_single false r_lead r in
    match l, r with
    | Nil, _ -> (r_lead, r_lead_single, r, r_trail, r_trail_single)
    | _, Nil -> (l_lead, l_lead_single, l, l_trail, l_trail_single)
    | _, _ ->
      (l_lead, l_lead_single, combinor front_p end_p, r_trail, r_trail_single)

  let doc_cons_opt empty x y =
    let (_, _, _, _, l_trail_single) = x in
    let combinor front_part end_part =
      choose_cons l_trail_single empty front_part end_part in
    doc_combinor x y combinor

  let doc_cons = doc_cons_opt false
  let (^|) = doc_cons

  let doc_cons_empty = doc_cons_opt true
  let (^^|) = doc_cons_empty

  let immediate_cons x y =
    let (_, _, _, _, l_trail_single) = x in
    let (_, r_lead_single, _, _, _) = y in
    let combinor front_part end_part =
      if l_trail_single || r_lead_single then
        choose_cons true false front_part end_part
      else front_part ^^ end_part
    in
    doc_combinor x y combinor
  let (^^^) = immediate_cons

  (* indent [r] by [indent] after [l] using the suitable line break *)
  let choose_indent_doc empty x y indent =
    let (_, _, _, _, l_trail_single) = x in
    let combinor front_part end_part =
      let break_choice =
       if l_trail_single then must_break
       else if empty then breakwith ""
       else break
      in
      front_part ^^ nest indent (break_choice ^^ end_part)
    in
    doc_combinor x y combinor

  (* put a break before [r] and nest [r] by [indent] in vertical layout *)
  let indent_doc = choose_indent_doc false

  let indent_doc_no_space = choose_indent_doc true

  (* typically we will want to indent block [blk] that is enclosed by
  * [l] and [r] by amount [indt] *)
  let indent_block l blk r indt =
    group_doc (indent_doc l blk indt ^| r)

  let indent_block_no_space l blk r indt =
    group_doc ((indent_doc_no_space l blk indt) ^^| r)

  let add_break (a, b, c, d, e) = (a, b, c, d, true)
end

module LineConf = struct
 let line_width = 80
end
module Comparator = WidthConstrainedDocComparator(LineConf)
module Core = Pretty_printing_library.Make(Comparator)
module Printer = Utility(Core)
module Syntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
open Syntax
open Printer

let get_doc_from_trivia trivia_lst allow_break =
  (* generate a document from a list of trivias. Return the doc, as well
   * as whether the trivia list contains a single line comment *)
  let module Trivia = Full_fidelity_editable_trivia in
  let module Kind = Full_fidelity_trivia_kind in
  let handle_trivia trivia = match Trivia.kind trivia with
    | Kind.WhiteSpace -> (nil, false)
    | Kind.EndOfLine -> (nil, false)
    | Kind.ExtraTokenError
    | Kind.Unsafe
    | Kind.FallThrough
    | Kind.SingleLineComment ->
      (* no code after comments *)
      (text (Trivia.text trivia), true)
    | Kind.FixMe
    | Kind.IgnoreError
    | Kind.UnsafeExpression
    | Kind.DelimitedComment ->
      (text (Trivia.text trivia), false)
  in
  let concat = if allow_break then break_cons else space_cons in
  let fold_fun x y =
    let (a, c1) = x in
    let (b, c2) = y in
    let result = concat a b in
    (result, c1 || c2)
  in
  (* no group here, since all breaks are compulsory. Will group on top level *)
  List.fold_left fold_fun (nil, false) (List.map handle_trivia trivia_lst)

let from_token x =
  let front_trivias = EditableToken.leading x in
  let end_trivias = EditableToken.trailing x in
  let (front_doc, front_single) = get_doc_from_trivia front_trivias true in
  let (end_doc, end_single) = get_doc_from_trivia end_trivias false in
  let doc = text (EditableToken.text x) in
  (front_doc, front_single, doc, end_doc, end_single)

(* create a 5-tuple (see top of file) from x with no trivias *)
let make_simple x = (nil, false, x, nil, false)
let indt = 2
let missing = make_simple nil
let error_header = make_simple (text "Error:")
let space = make_simple (text " ")
let colon = make_simple (text ":")
let comma = make_simple (text ",")
let l_square = make_simple (text "[")
let r_square = make_simple (text "]")
let question = make_simple (text "?")
let ellipsis = make_simple (text "...")

let rec get_doc node =
  match syntax node with
  | Missing -> missing
  | MarkupSection x ->
    group_doc (get_doc x.markup_prefix ^|
    get_doc x.markup_text ^|
    get_doc x.markup_suffix ^|
    get_doc x.markup_expression)
  | MarkupSuffix x ->
    group_doc (get_doc x.markup_suffix_less_than_question ^^^
    get_doc x.markup_suffix_name)

  | Token x -> from_token x

  | SyntaxList x -> get_from_children x
  | ErrorSyntax { error_error } -> get_doc error_error
  | LiteralExpression x ->
    begin
    match syntax x.literal_expression with
    | SyntaxList l -> get_from_children_no_space l
    | _ -> get_doc x.literal_expression
    end
  | VariableExpression x -> get_doc x.variable_expression
  | QualifiedNameExpression x -> get_doc x.qualified_name_expression
  | PipeVariableExpression x -> get_doc x.pipe_variable_expression
  | ListItem x -> (get_doc x.list_item) ^^^ (get_doc x.list_separator)
  | EndOfFile { end_of_file_token } -> get_doc end_of_file_token
  | Script x -> get_doc x.script_declarations
  | ClassishDeclaration
    { classish_attribute; classish_modifiers; classish_keyword;
      classish_name; classish_type_parameters; classish_extends_keyword;
      classish_extends_list; classish_implements_keyword;
      classish_implements_list; classish_body } ->
    let attr = add_break (get_doc classish_attribute) in
    let preface = group_doc (
      get_doc classish_modifiers ^|
      get_doc classish_keyword
    ) in

    let name_and_generics =
      let name = get_doc classish_name in
      let type_params = get_doc classish_type_parameters in
      group_doc (indent_doc name type_params indt)
    in

    let extends =
      let extends_token = get_doc classish_extends_keyword in
      let extends_list = get_doc classish_extends_list in
      group_doc (indent_doc extends_token extends_list indt)
    in

    let implements =
      let implements_token = get_doc classish_implements_keyword in
      let implements_list = get_doc classish_implements_list in
      group_doc (indent_doc implements_token implements_list indt)
    in

    let body = get_doc classish_body in

    (* TODO: Make this better *)
    attr ^^^
    group_doc (
      group_doc (
        preface ^|
        name_and_generics
      ) ^|
      group_doc (
        extends ^|
        implements
      ) ^|
      body
    )
  | ClassishBody x ->
    let left = get_doc x.classish_body_left_brace in
    let right = get_doc x.classish_body_right_brace in
    let body = get_doc x.classish_body_elements in
    indent_block_no_space left body right indt
  | XHPRequired { xhp_required_at; xhp_required_keyword } ->
    let a = get_doc xhp_required_at in
    let r = get_doc xhp_required_keyword in
    a ^^^ r
  | XHPChildrenDeclaration {
      xhp_children_keyword;
      xhp_children_expression;
      xhp_children_semicolon } ->
    let c = get_doc xhp_children_keyword in
    let e = get_doc xhp_children_expression in
    let s = get_doc xhp_children_semicolon in
    c ^| e ^^^ s
  | XHPChildrenParenthesizedList {
      xhp_children_list_left_paren;
      xhp_children_list_xhp_children;
      xhp_children_list_right_paren } ->
    let l = get_doc xhp_children_list_left_paren in
    let c = get_doc xhp_children_list_xhp_children in
    let r = get_doc xhp_children_list_right_paren in
    l ^| c ^| r
  | XHPCategoryDeclaration {
    xhp_category_keyword;
    xhp_category_categories;
    xhp_category_semicolon } ->
    let c = get_doc xhp_category_keyword in
    let l = get_doc xhp_category_categories in
    let s = get_doc xhp_category_semicolon in
    c ^| l ^^^ s
  | XHPEnumType x ->
    let e = get_doc x.xhp_enum_keyword in
    let l = get_doc x.xhp_enum_left_brace in
    let v = get_doc x.xhp_enum_values in
    let r = get_doc x.xhp_enum_right_brace in
    group_doc (e ^| l ^| v ^| r)
  | XHPClassAttributeDeclaration {
    xhp_attribute_keyword;
    xhp_attribute_attributes;
    xhp_attribute_semicolon } ->
    let attr = get_doc xhp_attribute_keyword in
    let attrs = get_doc xhp_attribute_attributes in
    let semi = get_doc xhp_attribute_semicolon in
    group_doc (attr ^| attrs ^^^ semi)
  | XHPClassAttribute
    { xhp_attribute_decl_type; xhp_attribute_decl_name;
      xhp_attribute_decl_initializer; xhp_attribute_decl_required } ->
    let t = get_doc xhp_attribute_decl_type in
    let n = get_doc xhp_attribute_decl_name in
    let i = get_doc xhp_attribute_decl_initializer in
    let r = get_doc xhp_attribute_decl_required in
    group_doc (t ^| n ^| i ^| r)
  | XHPSimpleClassAttribute { xhp_simple_class_attribute_type } ->
    get_doc xhp_simple_class_attribute_type
  | TraitUseAliasItem {
    trait_use_alias_item_aliasing_name;
    trait_use_alias_item_keyword;
    trait_use_alias_item_visibility;
    trait_use_alias_item_aliased_name;
    } ->
    let n = get_doc trait_use_alias_item_aliasing_name in
    let k = get_doc trait_use_alias_item_keyword in
    let v = get_doc trait_use_alias_item_visibility in
    let ns = get_doc trait_use_alias_item_aliased_name in
    n ^| k ^| v ^| ns
  | TraitUsePrecedenceItem {
    trait_use_precedence_item_name;
    trait_use_precedence_item_keyword;
    trait_use_precedence_item_removed_names;
    } ->
    let n = get_doc trait_use_precedence_item_name in
    let k = get_doc trait_use_precedence_item_keyword in
    let ns = get_doc trait_use_precedence_item_removed_names in
    n ^| k ^| ns
  | TraitUseConflictResolution {
    trait_use_conflict_resolution_keyword;
    trait_use_conflict_resolution_names;
    trait_use_conflict_resolution_left_brace;
    trait_use_conflict_resolution_clauses;
    trait_use_conflict_resolution_right_brace;
    } ->
    let use = get_doc trait_use_conflict_resolution_keyword in
    let name_list = get_doc trait_use_conflict_resolution_names in
    let lbrace = get_doc trait_use_conflict_resolution_left_brace in
    let clauses = get_doc trait_use_conflict_resolution_clauses in
    let rbrace = get_doc trait_use_conflict_resolution_right_brace in
    use ^| name_list ^^^ lbrace ^| clauses ^^^ rbrace
  | TraitUse {
    trait_use_keyword;
    trait_use_names;
    trait_use_semicolon;
    } ->
    let use = get_doc trait_use_keyword in
    let name_list = get_doc trait_use_names in
    let semi = get_doc trait_use_semicolon in
    use ^| name_list ^^^ semi
  | RequireClause x ->
    let r = get_doc x.require_keyword in
    let k = get_doc x.require_kind in
    let n = get_doc x.require_name in
    let s = get_doc x.require_semicolon in
    r ^| k ^| n ^^^ s
  | ConstDeclaration {
    const_abstract;
    const_keyword;
    const_type_specifier;
    const_declarators;
    const_semicolon } ->
    let abstr = get_doc const_abstract in
    let token = get_doc const_keyword in
    let ty = get_doc const_type_specifier in
    let lst = get_doc const_declarators in
    let semi = get_doc const_semicolon in
    group_doc (abstr ^| token ^| ty ) ^| lst ^^^ semi
  | ConstantDeclarator x ->
    let name = get_doc x.constant_declarator_name in
    let init = get_doc x.constant_declarator_initializer in
    group_doc (name ^| init)
  | TypeConstDeclaration x ->
    let abstr = get_doc x.type_const_abstract in
    let const = get_doc x.type_const_keyword in
    let type_ = get_doc x.type_const_type_keyword in
    let name = get_doc x.type_const_name in
    let type_constraint = get_doc x.type_const_type_constraint in
    let equal = get_doc x.type_const_equal in
    let type_spec = get_doc x.type_const_type_specifier in
    let semicolon = get_doc x.type_const_semicolon in
    group_doc (
      group_doc (abstr ^| const ^| type_ ^| name) ^|
      type_constraint ^|
      equal ^|
      group_doc (type_spec ^^^ semicolon)
    )
  | EnumDeclaration {
    enum_attribute_spec;
    enum_keyword;
    enum_name;
    enum_colon;
    enum_base;
    enum_type;
    enum_left_brace;
    enum_enumerators;
    enum_right_brace } ->
    let attrs = get_doc enum_attribute_spec in
    let en = get_doc enum_keyword in
    let na = get_doc enum_name in
    let co = get_doc enum_colon in
    let ba = get_doc enum_base in
    let ty = get_doc enum_type in
    let lb = get_doc enum_left_brace in
    let es = get_doc enum_enumerators in
    let rb = get_doc enum_right_brace in
    (* TODO: This could be a lot better. Add indentation, etc. *)
    attrs ^| en ^| na ^| co ^| ba ^| ty ^| lb ^| es ^| rb
  | Enumerator x ->
    let n = get_doc x.enumerator_name in
    let e = get_doc x.enumerator_equal in
    let v = get_doc x.enumerator_value in
    let semicolon = get_doc x.enumerator_semicolon in
    n ^| e ^| v ^^^ semicolon
  | AliasDeclaration x ->
    (* TODO: What's the best way to ensure that there's a newline between the
    attribute and the alias declaration proper? *)
    let attr = get_doc x.alias_attribute_spec in
    let a = get_doc x.alias_keyword in
    let n = get_doc x.alias_name in
    let generic = get_doc x.alias_generic_parameter in
    let c = get_doc x.alias_constraint in
    let e = get_doc x.alias_equal in
    let t = get_doc x.alias_type in
    let s = get_doc x.alias_semicolon in
    attr ^| a ^| n ^| generic ^| c ^| e ^| t ^^^ s
  | PropertyDeclaration
    { property_modifiers; property_type;
      property_declarators; property_semicolon } ->
    let m = get_doc property_modifiers in
    let t = get_doc property_type in
    let d = get_doc property_declarators in
    let s = get_doc property_semicolon in
    m ^| t ^| d ^^^ s
  | PropertyDeclarator { property_name; property_initializer } ->
    let n = get_doc property_name in
    let i = get_doc property_initializer in
    n ^| i
  | NamespaceDeclaration x ->
    let t = get_doc x.namespace_keyword in
    let n = get_doc x.namespace_name in
    let b = get_doc x.namespace_body in
    t ^| n ^| b
  | NamespaceBody x ->
    let left = get_doc x.namespace_left_brace in
    let body = get_doc x.namespace_declarations in
    let right = get_doc x.namespace_right_brace in
    indent_block_no_space left body right indt |> add_break
  | NamespaceEmptyBody x ->
    get_doc x.namespace_semicolon
  | NamespaceUseDeclaration x ->
    let u = get_doc x.namespace_use_keyword in
    let k = get_doc x.namespace_use_kind in
    let c = get_doc x.namespace_use_clauses in
    let s = get_doc x.namespace_use_semicolon in
    u ^| k ^| c ^^^ s
  | NamespaceUseClause x ->
    let k = get_doc x.namespace_use_clause_kind in
    let n = get_doc x.namespace_use_name in
    let a = get_doc x.namespace_use_as in
    let l = get_doc x.namespace_use_alias in
    k ^| n ^| a ^| l
  | NamespaceGroupUseDeclaration x ->
    let u = get_doc x.namespace_group_use_keyword in
    let k = get_doc x.namespace_group_use_kind in
    let p = get_doc x.namespace_group_use_prefix in
    let l = get_doc x.namespace_group_use_left_brace in
    let c = get_doc x.namespace_group_use_clauses in
    let r = get_doc x.namespace_group_use_right_brace in
    let s = get_doc x.namespace_group_use_semicolon in
    u ^| k ^| p ^| l ^| c ^| r ^^^ s
  | FunctionDeclaration x ->
      let attr = get_doc x.function_attribute_spec in
      let header = get_doc x.function_declaration_header in
      let body = x.function_body in
      let after_attr = handle_compound_inline_brace header body missing in
      group_doc (attr ^| after_attr)
  | FunctionDeclarationHeader
    { function_async;
      function_coroutine;
      function_keyword;
      function_ampersand;
      function_name;
      function_type_parameter_list;
      function_left_paren;
      function_parameter_list;
      function_right_paren;
      function_colon;
      function_type;
      function_where_clause }
   ->
    let preface = group_doc (
      get_doc function_async
        ^| get_doc function_coroutine
        ^| get_doc function_keyword
    ) in
    let name_and_generics =
      let type_params = get_doc function_type_parameter_list in
      let ampersand = get_doc function_ampersand in
      let name = get_doc function_name in
      group_doc (indent_doc (ampersand ^^^ name) type_params indt)
    in
    let parameters =
      let left = get_doc function_left_paren in
      let right = get_doc function_right_paren in
      let params = get_doc function_parameter_list in
      indent_block_no_space left params right indt
    in
    let type_declaration =
      let fun_colon = get_doc function_colon in
      let fun_type = get_doc function_type in
      let where_clause = get_doc function_where_clause in
      group_doc (fun_colon ^| fun_type ^| where_clause)
    in
    group_doc (
      group_doc ( group_doc (preface ^| name_and_generics) ^^| parameters )
      ^| type_declaration
    )
  | WhereClause { where_clause_keyword; where_clause_constraints } ->
    let w = get_doc where_clause_keyword in
    let c = get_doc where_clause_constraints in
    w ^| c
  | WhereConstraint { where_constraint_left_type; where_constraint_operator ;
    where_constraint_right_type } ->
    let l = get_doc where_constraint_left_type in
    let o = get_doc where_constraint_operator in
    let r = get_doc where_constraint_right_type in
    l ^| o ^| r
  | MethodishDeclaration
    { methodish_attribute; methodish_modifiers; methodish_function_decl_header;
      methodish_function_body; methodish_semicolon } ->
    let methodish_attr = get_doc methodish_attribute in
    let methodish_modifiers = get_doc methodish_modifiers in
    let function_header = get_doc methodish_function_decl_header in
    let body_node = methodish_function_body in
    let semicolon = get_doc methodish_semicolon in
    let before_body = group_doc (methodish_modifiers ^| function_header) in
    let after_attr =
      handle_compound_inline_brace before_body body_node missing in
    let after_attr = after_attr ^^^ semicolon in
    group_doc (methodish_attr ^| after_attr)
  | DecoratedExpression x ->
    let decorator = get_doc x.decorated_expression_decorator in
    let expression = get_doc x.decorated_expression_expression in
    group_doc (decorator ^^^ expression)
  | ParameterDeclaration {
    parameter_attribute;
    parameter_visibility;
    parameter_type;
    parameter_name;
    parameter_default_value } ->
    let attr = get_doc parameter_attribute in
    let visibility = get_doc parameter_visibility in
    let parameter_type = get_doc parameter_type in
    let parameter_name = get_doc parameter_name in
    let parameter_default = get_doc parameter_default_value in
    group_doc
      ( attr ^| visibility ^| parameter_type ^| parameter_name
      ^| parameter_default )
  | VariadicParameter { variadic_parameter_ellipsis } ->
      get_doc variadic_parameter_ellipsis
  | AttributeSpecification {
      attribute_specification_left_double_angle;
      attribute_specification_attributes;
      attribute_specification_right_double_angle } ->
    let left = get_doc attribute_specification_left_double_angle in
    let specs = get_doc attribute_specification_attributes in
    let right = get_doc attribute_specification_right_double_angle in
    indent_block_no_space left specs right indt
  | Attribute x ->
    let name = get_doc x.attribute_name in
    let left = get_doc x.attribute_left_paren in
    let right = get_doc x.attribute_right_paren in
    let values = get_doc x.attribute_values in
    let left_part = group_doc (name ^^| left) in
    indent_block_no_space left_part values right indt
  | InclusionExpression x ->
    let rq = get_doc x.inclusion_require in
    let fn = get_doc x.inclusion_filename in
    rq ^| fn
  | InclusionDirective x ->
    let ex = get_doc x.inclusion_expression in
    let se = get_doc x.inclusion_semicolon in
    ex ^^^ se
  | CompoundStatement x ->
    let left = get_doc x.compound_left_brace in
    let right = get_doc x.compound_right_brace in
    let body = get_doc x.compound_statements in
    indent_block_no_space left body right indt |> add_break
  | ExpressionStatement {
    expression_statement_expression;
    expression_statement_semicolon } ->
    let body = get_doc expression_statement_expression in
    let semicolon = get_doc expression_statement_semicolon in
    (* semicolon always follows the last line *)
    body ^^^ semicolon |> group_doc |> add_break
  | UnsetStatement {
    unset_keyword;
    unset_left_paren;
    unset_variables;
    unset_right_paren;
    unset_semicolon} ->
    let u = get_doc unset_keyword in
    let l = get_doc unset_left_paren in
    let v = get_doc unset_variables in
    let r = get_doc unset_right_paren in
    let s = get_doc unset_semicolon in
    group_doc (u ^^^ l ^^^ v ^^^ r ^^^ s)
  | WhileStatement
    { while_keyword; while_left_paren; while_condition; while_right_paren;
      while_body } ->
    let keyword = get_doc while_keyword in
    let left = get_doc while_left_paren in
    let condition = get_doc while_condition in
    let right = get_doc while_right_paren in
    let left_part = group_doc (keyword ^^| left) in
    let start_block = indent_block_no_space left_part condition right indt in
    handle_compound_brace_prefix_indent start_block while_body indt
    |> add_break
  | IfStatement
    { if_keyword; if_left_paren; if_condition; if_right_paren; if_statement;
      if_elseif_clauses; if_else_clause }->
    let keyword = get_doc if_keyword in
    let left = get_doc if_left_paren in
    let condition = get_doc if_condition in
    let right = get_doc if_right_paren in
    let if_stmt = if_statement in
    let elseif_clause = get_doc if_elseif_clauses in
    let else_clause = get_doc if_else_clause in
    let left_part = group_doc (keyword ^^| left) in
    let start_block = indent_block_no_space left_part condition right indt in
    let if_statement =
      handle_compound_brace_prefix_indent start_block if_stmt indt in
    let if_statement = add_break if_statement in
    group_doc (if_statement ^| elseif_clause ^| else_clause)
  | ElseifClause
    { elseif_keyword; elseif_left_paren; elseif_condition; elseif_right_paren;
      elseif_statement }  ->
    let keyword = get_doc elseif_keyword in
    let left = get_doc elseif_left_paren in
    let condition = get_doc elseif_condition in
    let right = get_doc elseif_right_paren in
    let elif_statement_syntax = elseif_statement in
    let left_part = group_doc (keyword ^^| left) in
    let start_block = indent_block_no_space left_part condition right indt in
    handle_compound_brace_prefix_indent start_block elif_statement_syntax indt
    |> add_break
  | ElseClause x ->
    let keyword = get_doc x.else_keyword in
    let statement = x.else_statement in
    handle_compound_brace_prefix_indent keyword statement indt
    |> add_break
  | TryStatement
    { try_keyword;
      try_compound_statement;
      try_catch_clauses;
      try_finally_clause } ->
    let keyword = get_doc try_keyword in
    let compound_stmt = try_compound_statement in
    let try_part =
      handle_compound_brace_prefix_indent keyword compound_stmt indt in
    let catch_clauses = get_doc try_catch_clauses in
    let finally_clause = get_doc try_finally_clause in
    group_doc (try_part ^| catch_clauses ^| finally_clause)
    |> add_break
  | CatchClause {
    catch_keyword;
    catch_left_paren;
    catch_type;
    catch_variable;
    catch_right_paren;
    catch_body } ->
    let keyword = get_doc catch_keyword in
    let left = get_doc catch_left_paren in
    let ty = get_doc catch_type in
    let var = get_doc catch_variable in
    let param = ty ^| var in
    let right = get_doc catch_right_paren in
    let stmt = catch_body in
    let front_part = group_doc (keyword ^| left) in
    let before_stmt = indent_block_no_space front_part param right indt in
    handle_compound_brace_prefix_indent before_stmt stmt indt
    |> add_break
  | FinallyClause {
    finally_keyword;
    finally_body } ->
    let keyword = get_doc finally_keyword in
    handle_compound_brace_prefix_indent keyword finally_body indt
    |> add_break
  | DoStatement {
    do_keyword;
    do_body;
    do_while_keyword;
    do_left_paren;
    do_condition;
    do_right_paren;
    do_semicolon } ->
    let keyword = get_doc do_keyword in
    let statement = do_body in
    let while_keyword = get_doc do_while_keyword in
    let left = get_doc do_left_paren in
    let right = get_doc do_right_paren in
    let condition = get_doc do_condition in
    let semicolon = get_doc do_semicolon in
    let statement_part =
      handle_compound_brace_prefix_indent keyword statement indt |> add_break in
    let left_part = group_doc (while_keyword ^^| left) in
    let condition_part = indent_block_no_space left_part condition right indt in
    group_doc (statement_part ^| condition_part) ^^^ semicolon
  | ForStatement {
    for_keyword;
    for_left_paren;
    for_initializer;
    for_first_semicolon;
    for_control;
    for_second_semicolon;
    for_end_of_loop;
    for_right_paren;
    for_body } ->
    let keyword = get_doc for_keyword in
    let left_paren = get_doc for_left_paren in
    let initializer_expr = get_doc for_initializer in
    let first_semicolon = get_doc for_first_semicolon in
    let control_expr = get_doc for_control in
    let second_semicolon = get_doc for_second_semicolon in
    let end_of_loop_expr = get_doc for_end_of_loop in
    let right_paren = get_doc for_right_paren in
    let statement = for_body in
    let left_part = group_doc (keyword ^^| left_paren) in
    let for_expressions =
      control_expr ^^^ second_semicolon ^| end_of_loop_expr in
    let for_expressions = if is_missing for_control
      then first_semicolon ^^| for_expressions
      else first_semicolon ^| for_expressions in
    let for_expressions = group_doc (initializer_expr ^^^ for_expressions) in
    let start_block =
      indent_block_no_space left_part for_expressions right_paren indt in
    handle_compound_brace_prefix_indent start_block statement indt |> add_break
  | ForeachStatement {
    foreach_keyword;
    foreach_left_paren;
    foreach_collection;
    foreach_await_keyword;
    foreach_as;
    foreach_key;
    foreach_arrow;
    foreach_value;
    foreach_right_paren;
    foreach_body }->
    let keyword = get_doc foreach_keyword in
    let left = get_doc foreach_left_paren in
    let right = get_doc foreach_right_paren in
    let collection_name = get_doc foreach_collection in
    let await_keyword = get_doc foreach_await_keyword in
    let as_keyword = get_doc foreach_as in
    let key = get_doc foreach_key in
    let arrow = get_doc foreach_arrow in
    let value = get_doc foreach_value in
    let statement = foreach_body in
    let left_part = group_doc (keyword ^| left) in
    let arrow_part = group_doc (group_doc (key ^| arrow) ^| value) in
    let as_part = group_doc (await_keyword ^| as_keyword) in
    let as_part = group_doc (collection_name ^| as_part) in
    let middle_part = group_doc (as_part ^| arrow_part) in
    let start_block = indent_block_no_space left_part middle_part right indt in
    handle_compound_brace_prefix_indent start_block statement indt |> add_break
  | SwitchStatement {
    switch_keyword;
    switch_left_paren;
    switch_expression;
    switch_right_paren;
    switch_left_brace;
    switch_sections;
    switch_right_brace } ->
    let keyword = get_doc switch_keyword in
    let lparen= get_doc switch_left_paren in
    let expr = get_doc switch_expression in
    let rparen = get_doc switch_right_paren in
    let lbrace = get_doc switch_left_brace in
    let sections = get_doc switch_sections in
    let rbrace = get_doc switch_right_brace in
    (* TODO Fix this *)
    let h = keyword ^| lparen ^| expr ^| rparen ^| space ^| lbrace in
    let h = add_break h in
    h ^| sections ^| rbrace
  | SwitchSection {
    switch_section_labels;
    switch_section_statements;
    switch_section_fallthrough } ->
    (* TODO Fix this *)
    let labels = get_doc switch_section_labels in
    let statements = get_doc switch_section_statements in
    let fallthrough = get_doc switch_section_fallthrough in
    (add_break labels) ^| (add_break statements) ^| (add_break fallthrough)
  | SwitchFallthrough {
    fallthrough_keyword;
    fallthrough_semicolon
  } ->
    let f = get_doc fallthrough_keyword in
    let s = get_doc fallthrough_semicolon in
    f ^^^ s
  | ScopeResolutionExpression x ->
    let q = get_doc x.scope_resolution_qualifier in
    let o = get_doc x.scope_resolution_operator in
    let n = get_doc x.scope_resolution_name in
    group_doc (q ^^^ o ^^^ n)
  | MemberSelectionExpression
    { member_object; member_operator; member_name } ->
    let ob = get_doc member_object in
    let op = get_doc member_operator in
    let nm = get_doc member_name in
    group_doc (ob ^^^ op ^^^ nm)
  | SafeMemberSelectionExpression
    { safe_member_object; safe_member_operator; safe_member_name } ->
    let ob = get_doc safe_member_object in
    let op = get_doc safe_member_operator in
    let nm = get_doc safe_member_name in
    group_doc (ob ^^^ op ^^^ nm)
  | EmbeddedMemberSelectionExpression
    { embedded_member_object;
      embedded_member_operator;
      embedded_member_name } ->
    let ob = get_doc embedded_member_object in
    let op = get_doc embedded_member_operator in
    let nm = get_doc embedded_member_name in
    group_doc (ob ^^^ op ^^^ nm)
  | YieldExpression x ->
    let y = get_doc x.yield_keyword in
    let o = get_doc x.yield_operand in
    group_doc (y ^| o)
  | YieldFromExpression x ->
    let y = get_doc x.yield_from_yield_keyword in
    let f = get_doc x.yield_from_from_keyword in
    let o = get_doc x.yield_from_operand in
    group_doc (y ^| f ^| o)
  | CastExpression x ->
    let l = get_doc x.cast_left_paren in
    let t = get_doc x.cast_type in
    let r = get_doc x.cast_right_paren in
    let o = get_doc x.cast_operand in
    group_doc (l ^^^ t ^^^ r ^^^ o)
  | LambdaExpression {
      lambda_async;
      lambda_coroutine;
      lambda_signature;
      lambda_arrow;
      lambda_body;
    } ->
    let async = get_doc lambda_async in
    let coroutine = get_doc lambda_coroutine in
    let signature = get_doc lambda_signature in
    let arrow = get_doc lambda_arrow in
    let body = get_doc lambda_body in
    group_doc (async ^| coroutine ^| signature ^| arrow ^| body)
  | LambdaSignature
    { lambda_left_paren; lambda_parameters; lambda_right_paren;
      lambda_colon; lambda_type } ->
    let left = get_doc lambda_left_paren in
    let params = get_doc lambda_parameters in
    let right = get_doc lambda_right_paren in
    let colon = get_doc lambda_colon in
    let ty = get_doc lambda_type in
    group_doc (left ^| params ^| right ^| colon ^| ty)
  | AnonymousFunction
    { anonymous_static_keyword;
      anonymous_async_keyword;
      anonymous_coroutine_keyword;
      anonymous_function_keyword;
      anonymous_left_paren;
      anonymous_parameters;
      anonymous_right_paren;
      anonymous_colon;
      anonymous_type;
      anonymous_use;
      anonymous_body } ->
    let static = get_doc anonymous_static_keyword in
    let async = get_doc anonymous_async_keyword in
    let coroutine = get_doc anonymous_coroutine_keyword in
    let fn = get_doc anonymous_function_keyword in
    let left = get_doc anonymous_left_paren in
    let params = get_doc anonymous_parameters in
    let right = get_doc anonymous_right_paren in
    let colon = get_doc anonymous_colon in
    let return_type = get_doc anonymous_type in
    let preface = group_doc ( static ^| async ^| coroutine ^| fn ) in
    let parameters = indent_block_no_space left params right indt in
    let type_declaration = group_doc (colon ^| return_type) in
    let uses = get_doc anonymous_use in
    let body = anonymous_body in
    let before_body =
      group_doc (
        group_doc ( group_doc preface ^^| parameters )
        ^| type_declaration ^| uses
      ) in
      handle_compound_inline_brace before_body body missing
  | AnonymousFunctionUseClause x ->
    let u = get_doc x.anonymous_use_keyword in
    let l = get_doc x.anonymous_use_left_paren in
    let v = get_doc x.anonymous_use_variables in
    let r = get_doc x.anonymous_use_right_paren in
    u ^| l ^^^ v ^^^ r
  | PrefixUnaryExpression
    { prefix_unary_operator; prefix_unary_operand } ->
    if is_separable_prefix prefix_unary_operator then
      get_doc prefix_unary_operator ^| get_doc prefix_unary_operand
    else
      get_doc prefix_unary_operator ^^^ get_doc prefix_unary_operand
  | PostfixUnaryExpression
    { postfix_unary_operand; postfix_unary_operator } ->
    get_doc postfix_unary_operand ^^^ get_doc postfix_unary_operator
  | BinaryExpression x ->
    let left = get_doc x.binary_left_operand in
    let op = get_doc x.binary_operator in
    let right = get_doc x.binary_right_operand in
    group_doc (left ^| op ^| right)
  | InstanceofExpression x ->
    let left = get_doc x.instanceof_left_operand in
    let op = get_doc x.instanceof_operator in
    let right = get_doc x.instanceof_right_operand in
    group_doc (left ^| op ^| right)
  | ConditionalExpression x ->
    let tst = get_doc x.conditional_test in
    let qm = get_doc x.conditional_question in
    let con = get_doc x.conditional_consequence in
    let col = get_doc x.conditional_colon in
    let alt = get_doc x.conditional_alternative in
    (* TODO: Could this be improved? *)
    group_doc ( tst ^| qm ^| con ^| col ^| alt )
  | FunctionCallExpression {
    function_call_receiver;
    function_call_left_paren;
    function_call_argument_list;
    function_call_right_paren } ->
    let receiver = get_doc function_call_receiver in
    let lparen = get_doc function_call_left_paren in
    let args = get_doc function_call_argument_list in
    let rparen = get_doc function_call_right_paren in
    receiver ^^^ lparen ^^^ args ^^^ rparen
  | EvalExpression {
    eval_keyword;
    eval_left_paren;
    eval_argument;
    eval_right_paren } ->
    let keyword = get_doc eval_keyword in
    let lparen = get_doc eval_left_paren in
    let arg = get_doc eval_argument in
    let rparen = get_doc eval_right_paren in
    keyword ^^^ lparen ^^^ arg ^^^ rparen
  | EmptyExpression {
    empty_keyword;
    empty_left_paren;
    empty_argument;
    empty_right_paren } ->
    let keyword = get_doc empty_keyword in
    let lparen = get_doc empty_left_paren in
    let arg = get_doc empty_argument in
    let rparen = get_doc empty_right_paren in
    keyword ^^^ lparen ^^^ arg ^^^ rparen
  | IssetExpression {
    isset_keyword;
    isset_left_paren;
    isset_argument_list;
    isset_right_paren } ->
    let keyword = get_doc isset_keyword in
    let lparen = get_doc isset_left_paren in
    let args = get_doc isset_argument_list in
    let rparen = get_doc isset_right_paren in
    keyword ^^^ lparen ^^^ args ^^^ rparen
  | DefineExpression {
    define_keyword;
    define_left_paren;
    define_argument_list;
    define_right_paren } ->
    let keyword = get_doc define_keyword in
    let lparen = get_doc define_left_paren in
    let args = get_doc define_argument_list in
    let rparen = get_doc define_right_paren in
    keyword ^^^ lparen ^^^ args ^^^ rparen
  | ParenthesizedExpression {
    parenthesized_expression_left_paren;
    parenthesized_expression_expression;
    parenthesized_expression_right_paren } ->
    let left = get_doc parenthesized_expression_left_paren in
    let expr = get_doc parenthesized_expression_expression in
    let right = get_doc parenthesized_expression_right_paren in
    indent_block_no_space left expr right indt
  | BracedExpression {
    braced_expression_left_brace;
    braced_expression_expression;
    braced_expression_right_brace } ->
    let left = get_doc braced_expression_left_brace in
    let expr = get_doc braced_expression_expression in
    let right = get_doc braced_expression_right_brace in
    indent_block_no_space left expr right indt
  | EmbeddedBracedExpression {
    embedded_braced_expression_left_brace;
    embedded_braced_expression_expression;
    embedded_braced_expression_right_brace } ->
    let left = get_doc embedded_braced_expression_left_brace in
    let expr = get_doc embedded_braced_expression_expression in
    let right = get_doc embedded_braced_expression_right_brace in
    indent_block_no_space left expr right indt
  | ListExpression
    { list_keyword; list_left_paren; list_members; list_right_paren } ->
    let keyword = get_doc list_keyword in
    let left_paren = get_doc list_left_paren in
    let members = get_doc list_members in
    let right_paren = get_doc list_right_paren in
    let left = group_doc (keyword ^| left_paren) in
    indent_block_no_space left members right_paren indt
  | CollectionLiteralExpression x ->
    let token = get_doc x.collection_literal_name in
    let left_brace = get_doc x.collection_literal_left_brace in
    let expression_list = get_doc x.collection_literal_initializers in
    let right_brace = get_doc x.collection_literal_right_brace in
    token ^| left_brace ^| expression_list ^| right_brace
  | ObjectCreationExpression
    { object_creation_new_keyword;
      object_creation_type;
      object_creation_left_paren;
      object_creation_argument_list;
      object_creation_right_paren } ->
    let n = get_doc object_creation_new_keyword in
    let c = get_doc object_creation_type in
    let l = get_doc object_creation_left_paren in
    let a = get_doc object_creation_argument_list in
    let r = get_doc object_creation_right_paren in
    n ^| c ^^^ l ^^^ a ^^^ r
  | FieldInitializer
    { field_initializer_name; field_initializer_arrow;
      field_initializer_value }->
    let n = get_doc field_initializer_name in
    let a = get_doc field_initializer_arrow in
    let v = get_doc field_initializer_value in
    n ^| a ^| v
  | ShapeExpression
    { shape_expression_keyword; shape_expression_left_paren;
      shape_expression_fields; shape_expression_right_paren } ->
    let sh = get_doc shape_expression_keyword in
    let lp = get_doc shape_expression_left_paren in
    let fs = get_doc shape_expression_fields in
    let rp = get_doc shape_expression_right_paren in
    sh ^| lp ^^^ fs ^^^ rp
  | TupleExpression
    { tuple_expression_keyword; tuple_expression_left_paren;
       tuple_expression_items; tuple_expression_right_paren } ->
    let tu = get_doc tuple_expression_keyword in
    let lp = get_doc tuple_expression_left_paren in
    let xs = get_doc tuple_expression_items in
    let rp = get_doc tuple_expression_right_paren in
    tu ^| lp ^^^ xs ^^^ rp
  | ArrayCreationExpression x ->
    let left_bracket = get_doc x.array_creation_left_bracket in
    let right_bracket = get_doc x.array_creation_right_bracket in
    let members = get_doc x.array_creation_members in
    indent_block_no_space left_bracket members right_bracket indt
  | ArrayIntrinsicExpression {
    array_intrinsic_keyword;
    array_intrinsic_left_paren;
    array_intrinsic_members;
    array_intrinsic_right_paren } ->
    let keyword = get_doc array_intrinsic_keyword in
    let left = get_doc array_intrinsic_left_paren in
    let members = get_doc array_intrinsic_members in
    let right = get_doc array_intrinsic_right_paren in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | DarrayIntrinsicExpression {
    darray_intrinsic_keyword;
    darray_intrinsic_left_bracket;
    darray_intrinsic_members;
    darray_intrinsic_right_bracket } ->
    let keyword = get_doc darray_intrinsic_keyword in
    let left = get_doc darray_intrinsic_left_bracket in
    let members = get_doc darray_intrinsic_members in
    let right = get_doc darray_intrinsic_right_bracket in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | DictionaryIntrinsicExpression {
    dictionary_intrinsic_keyword;
    dictionary_intrinsic_left_bracket;
    dictionary_intrinsic_members;
    dictionary_intrinsic_right_bracket } ->
    let keyword = get_doc dictionary_intrinsic_keyword in
    let left = get_doc dictionary_intrinsic_left_bracket in
    let members = get_doc dictionary_intrinsic_members in
    let right = get_doc dictionary_intrinsic_right_bracket in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | KeysetIntrinsicExpression {
    keyset_intrinsic_keyword;
    keyset_intrinsic_left_bracket;
    keyset_intrinsic_members;
    keyset_intrinsic_right_bracket } ->
    let keyword = get_doc keyset_intrinsic_keyword in
    let left = get_doc keyset_intrinsic_left_bracket in
    let members = get_doc keyset_intrinsic_members in
    let right = get_doc keyset_intrinsic_right_bracket in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | VarrayIntrinsicExpression {
    varray_intrinsic_keyword;
    varray_intrinsic_left_bracket;
    varray_intrinsic_members;
    varray_intrinsic_right_bracket } ->
    let keyword = get_doc varray_intrinsic_keyword in
    let left = get_doc varray_intrinsic_left_bracket in
    let members = get_doc varray_intrinsic_members in
    let right = get_doc varray_intrinsic_right_bracket in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | VectorIntrinsicExpression {
    vector_intrinsic_keyword;
    vector_intrinsic_left_bracket;
    vector_intrinsic_members;
    vector_intrinsic_right_bracket } ->
    let keyword = get_doc vector_intrinsic_keyword in
    let left = get_doc vector_intrinsic_left_bracket in
    let members = get_doc vector_intrinsic_members in
    let right = get_doc vector_intrinsic_right_bracket in
    let left_part = group_doc (keyword ^^| left) in
    indent_block_no_space left_part members right indt
  | ElementInitializer x ->
    let k = get_doc x.element_key in
    let a = get_doc x.element_arrow in
    let v = get_doc x.element_value in
    k ^| a ^| v
  | SubscriptExpression x ->
    let receiver = get_doc x.subscript_receiver in
    let left = get_doc x.subscript_left_bracket in
    let index = get_doc x.subscript_index in
    let right = get_doc x.subscript_right_bracket in
    receiver ^^^ left ^^^ index ^^^ right
  | EmbeddedSubscriptExpression x ->
    let receiver = get_doc x.embedded_subscript_receiver in
    let left = get_doc x.embedded_subscript_left_bracket in
    let index = get_doc x.embedded_subscript_index in
    let right = get_doc x.embedded_subscript_right_bracket in
    receiver ^^^ left ^^^ index ^^^ right
  | AwaitableCreationExpression x ->
    let async = get_doc x.awaitable_async in
    let coroutine = get_doc x.awaitable_coroutine in
    let stmt = x.awaitable_compound_statement in
    handle_compound_brace_prefix_indent (async ^| coroutine) stmt indt
  | XHPExpression x ->
    let left = get_doc x.xhp_open in
    let expr = get_doc x.xhp_body in
    let right = get_doc x.xhp_close in
    left ^^^ expr ^^^ right
  | XHPOpen {
    xhp_open_left_angle;
    xhp_open_name;
    xhp_open_attributes;
    xhp_open_right_angle } ->
    let left = get_doc xhp_open_left_angle in
    let name = get_doc xhp_open_name in
    let attrs = get_doc xhp_open_attributes in
    let right = get_doc xhp_open_right_angle in
    group_doc (group_doc (indent_doc (left ^^^ name) attrs indt) ^| right)
  | XHPAttribute
    { xhp_attribute_name; xhp_attribute_equal; xhp_attribute_expression }->
    let name = get_doc xhp_attribute_name in
    let equals = get_doc xhp_attribute_equal in
    let expr = get_doc xhp_attribute_expression in
    group_doc (group_doc (name ^^| equals) ^^| expr)
  | XHPClose x ->
    let left = get_doc x.xhp_close_left_angle in
    let name = get_doc x.xhp_close_name in
    let right = get_doc x.xhp_close_right_angle in
    left ^^^ name ^^^ right
  | TypeConstant x ->
    let left = get_doc x.type_constant_left_type in
    let right = get_doc x.type_constant_right_type in
    let separator = get_doc x.type_constant_separator in
    left ^^^ separator ^^^ right
  | SimpleTypeSpecifier x -> get_doc x.simple_type_specifier
  | TypeConstraint { constraint_keyword; constraint_type } ->
    let k = get_doc constraint_keyword in
    let t = get_doc constraint_type in
    k ^| t
  | TypeParameter x ->
    let variance = get_doc x.type_variance in
    let name = get_doc x.type_name in
    let constraints = get_doc x.type_constraints in
    variance ^^^ name ^| constraints
  | NullableTypeSpecifier x ->
    let qm = get_doc x.nullable_question in
    let ty = get_doc x.nullable_type in
    qm ^^^ ty
  | SoftTypeSpecifier x ->
    let a = get_doc x.soft_at in
    let t = get_doc x.soft_type in
    a ^^^ t
  | GenericTypeSpecifier
    { generic_class_type;
      generic_argument_list } ->
    let name = get_doc generic_class_type in
    let argument = get_doc generic_argument_list in
    group_doc (indent_doc_no_space name argument indt)
  | VarrayTypeSpecifier {
      varray_keyword;
      varray_left_angle;
      varray_type;
      varray_trailing_comma;
      varray_right_angle
    } ->
    let ar = get_doc varray_keyword in
    let la = get_doc varray_left_angle in
    let ty = get_doc varray_type in
    let oc = get_doc varray_trailing_comma in
    let ra = get_doc varray_right_angle in
    ar ^^^ la ^^^ ty ^^^ oc ^^^ ra
  | VectorArrayTypeSpecifier {
      vector_array_keyword;
      vector_array_left_angle;
      vector_array_type;
      vector_array_right_angle
    } ->
    let ar = get_doc vector_array_keyword in
    let la = get_doc vector_array_left_angle in
    let ty = get_doc vector_array_type in
    let ra = get_doc vector_array_right_angle in
    ar ^^^ la ^^^ ty ^^^ ra
  | VectorTypeSpecifier {
      vector_type_keyword;
      vector_type_left_angle;
      vector_type_type;
      vector_type_trailing_comma;
      vector_type_right_angle;
    } ->
    let ar = get_doc vector_type_keyword in
    let la = get_doc vector_type_left_angle in
    let ty = get_doc vector_type_type in
    let tr = get_doc vector_type_trailing_comma in
    let ra = get_doc vector_type_right_angle in
    ar ^^^ la ^^^ ty ^^^ tr ^^^ ra
  | KeysetTypeSpecifier {
      keyset_type_keyword;
      keyset_type_left_angle;
      keyset_type_type;
      keyset_type_trailing_comma;
      keyset_type_right_angle
    } ->
    let ar = get_doc keyset_type_keyword in
    let la = get_doc keyset_type_left_angle in
    let ty = get_doc keyset_type_type in
    let tr = get_doc keyset_type_trailing_comma in
    let ra = get_doc keyset_type_right_angle in
    ar ^^^ la ^^^ ty ^^^ tr ^^^ ra
  | TupleTypeExplicitSpecifier {
      tuple_type_keyword;
      tuple_type_left_angle;
      tuple_type_types;
      tuple_type_right_angle
    } ->
    let tu = get_doc tuple_type_keyword in
    let la = get_doc tuple_type_left_angle in
    let ts = get_doc tuple_type_types in
    let ra = get_doc tuple_type_right_angle in
    tu ^^^ la ^^^ ts ^^^ ra
  | DictionaryTypeSpecifier {
      dictionary_type_keyword;
      dictionary_type_left_angle;
      dictionary_type_members;
      dictionary_type_right_angle
    } ->
    let ar = get_doc dictionary_type_keyword in
    let la = get_doc dictionary_type_left_angle in
    let ms = get_doc dictionary_type_members in
    let ra = get_doc dictionary_type_right_angle in
    ar ^^^ la ^^^ ms ^^^ ra
  | DarrayTypeSpecifier {
      darray_keyword;
      darray_left_angle;
      darray_key;
      darray_comma;
      darray_value;
      darray_trailing_comma;
      darray_right_angle
    } ->
    let ar = get_doc darray_keyword in
    let la = get_doc darray_left_angle in
    let kt = get_doc darray_key in
    let co = get_doc darray_comma in
    let vt = get_doc darray_value in
    let oc = get_doc darray_trailing_comma in
    let ra = get_doc darray_right_angle in
    ar ^^^ la ^^^ kt ^^^ co ^| vt ^^^ oc ^^^ ra
  | MapArrayTypeSpecifier {
      map_array_keyword;
      map_array_left_angle;
      map_array_key;
      map_array_comma;
      map_array_value;
      map_array_right_angle
    } ->
    let ar = get_doc map_array_keyword in
    let la = get_doc map_array_left_angle in
    let kt = get_doc map_array_key in
    let co = get_doc map_array_comma in
    let vt = get_doc map_array_value in
    let ra = get_doc map_array_right_angle in
    ar ^^^ la ^^^ kt ^^^ co ^| vt ^^^ ra
  | ClosureTypeSpecifier
  { closure_outer_left_paren;
    closure_coroutine;
    closure_function_keyword;
    closure_inner_left_paren;
    closure_parameter_types;
    closure_inner_right_paren;
    closure_colon;
    closure_return_type;
    closure_outer_right_paren } ->
    let olp = get_doc closure_outer_left_paren in
    let cor = get_doc closure_coroutine in
    let fnc = get_doc closure_function_keyword in
    let ilp = get_doc closure_inner_left_paren in
    let pts = get_doc closure_parameter_types in
    let irp = get_doc closure_inner_right_paren in
    let col = get_doc closure_colon in
    let ret = get_doc closure_return_type in
    let orp = get_doc closure_outer_right_paren in
    olp ^^^ cor ^| fnc ^^| ilp ^^^ pts ^^^ irp ^^^ col ^^^ ret ^^^ orp
  | ClassnameTypeSpecifier x ->
    let cn = get_doc x.classname_keyword in
    let la = get_doc x.classname_left_angle in
    let ty = get_doc x.classname_type in
    let ra = get_doc x.classname_right_angle in
    cn ^^^ la ^^^ ty ^^^ ra
  | FieldSpecifier x ->
    let q = get_doc x.field_question in
    let n = get_doc x.field_name in
    let a = get_doc x.field_arrow in
    let t = get_doc x.field_type in
    q ^| n ^| a ^| t
  | ShapeTypeSpecifier
    { shape_type_keyword; shape_type_left_paren;
      shape_type_fields; shape_type_ellipsis; shape_type_right_paren } ->
    let sh = get_doc shape_type_keyword in
    let lp = get_doc shape_type_left_paren in
    let fs = get_doc shape_type_fields in
    let ellipsis = get_doc shape_type_ellipsis in
    let rp = get_doc shape_type_right_paren in
    sh ^| lp ^^^ fs ^| ellipsis ^^^ rp
  | TypeArguments {
    type_arguments_left_angle;
    type_arguments_types;
    type_arguments_right_angle } ->
    let left = get_doc type_arguments_left_angle in
    let args = get_doc type_arguments_types in
    let right = get_doc type_arguments_right_angle in
    indent_block_no_space left args right indt
  | TypeParameters {
    type_parameters_left_angle;
    type_parameters_parameters;
    type_parameters_right_angle } ->
    let left = get_doc type_parameters_left_angle in
    let params = get_doc type_parameters_parameters in
    let right = get_doc type_parameters_right_angle in
    indent_block_no_space left params right indt
  | TupleTypeSpecifier x ->
    let left = get_doc x.tuple_left_paren in
    let types = get_doc x.tuple_types in
    let right = get_doc x.tuple_right_paren in
    indent_block_no_space left types right indt
  (* this ideally should never be called *)
  | CaseLabel {
    case_keyword;
    case_expression;
    case_colon } ->
    let keyword = get_doc case_keyword in
    let expr = get_doc case_expression in
    let colon = get_doc case_colon in
    keyword ^^^ space ^^^ expr ^^^ colon
  | DefaultLabel {
      default_keyword;
      default_colon } ->
    let keyword = get_doc default_keyword in
    let colon = get_doc default_colon in
    keyword ^^^ colon
  | ReturnStatement x ->
    let keyword = get_doc x.return_keyword in
    let expr = get_doc x.return_expression in
    let semicolon = get_doc x.return_semicolon in
    let back_part = expr ^^^ semicolon in
    group_doc (indent_doc keyword back_part indt)
  | GotoLabel {
      goto_label_name;
      goto_label_colon; } ->
    let goto_label_name = get_doc goto_label_name in
    let goto_label_colon = get_doc goto_label_colon in
    goto_label_name ^^^ goto_label_colon
  | GotoStatement {
      goto_statement_keyword;
      goto_statement_label_name;
      goto_statement_semicolon; } ->
    let keyword = get_doc goto_statement_keyword in
    let label_name = get_doc goto_statement_label_name in
    let semicolon = get_doc goto_statement_semicolon in
    keyword ^| label_name ^^^ semicolon
  | ThrowStatement x ->
    let keyword = get_doc x.throw_keyword in
    let expr = get_doc x.throw_expression in
    let semicolon = get_doc x.throw_semicolon in
    let back_part = expr ^^^ semicolon in
    group_doc (indent_doc keyword back_part indt)
  | BreakStatement x ->
    let b = get_doc x.break_keyword in
    let l = get_doc x.break_level in
    let s = get_doc x.break_semicolon in
    if is_missing x.break_level then group_doc (b ^^^ l ^^^ s)
    else group_doc (b ^| l ^^^ s)
  | ContinueStatement x ->
    let c = get_doc x.continue_keyword in
    let l = get_doc x.continue_level in
    let s = get_doc x.continue_semicolon in
    if is_missing x.continue_level then group_doc (c ^^^ l ^^^ s)
    else group_doc (c ^| l ^^^ s)
  | FunctionStaticStatement {
    static_static_keyword;
    static_declarations;
    static_semicolon } ->
    let st = get_doc static_static_keyword in
    let ds = get_doc static_declarations in
    let se = get_doc static_semicolon in
    st ^| ds ^^^ se
  | StaticDeclarator
    { static_name; static_initializer } ->
    let n = get_doc static_name in
    let i = get_doc static_initializer in
    group_doc (n ^| i)
  | EchoStatement x ->
    let echo = get_doc x.echo_keyword in
    let expr_list = get_doc x.echo_expressions in
    let semicolon = get_doc x.echo_semicolon in
    echo ^| expr_list ^^^ semicolon
  | GlobalStatement {
    global_keyword;
    global_variables;
    global_semicolon } ->
    let g = get_doc global_keyword in
    let v = get_doc global_variables in
    let s = get_doc global_semicolon in
    g ^| v ^^^ s
  | SimpleInitializer
    { simple_initializer_equal; simple_initializer_value } ->
    let e = get_doc simple_initializer_equal in
    let v = get_doc simple_initializer_value in
    group_doc (e ^| v)

(* sep is the compulsory separator separating the children in the list *)
and get_from_children_no_space children =
  let fold_fun acc el = acc ^^^ get_doc el in
  group_doc (List.fold_left fold_fun (make_simple nil) children)
and get_from_children_with_sep sep children =
  let fold_fun acc el = (acc ^^^ sep) ^| get_doc el in
  group_doc (List.fold_left fold_fun (make_simple nil) children)
and get_from_children node = get_from_children_with_sep (make_simple nil) node
(* if it is a compound statement, the curly braces do not need indent *)
and peek_and_decide_indent x default =
  match syntax x with
  | CompoundStatement _ -> 0
  | _ -> default
(* puts [prefix] on the same line as a compound brace. If the statement is not
 * compound, put an optional newline and group the result *)
and handle_compound_inline_brace prefix statement postfix =
  match syntax statement with
  | CompoundStatement compound_stmt ->
  let left = get_doc compound_stmt.compound_left_brace in
  let right = get_doc compound_stmt.compound_right_brace in
  let statement = get_doc compound_stmt.compound_statements in
  let prefix = group_doc (prefix ^| left) in
  let postfix = group_doc (right ^| postfix) in
  indent_block prefix statement postfix indt
  | _ -> group_doc (prefix ^| get_doc statement ^| postfix)
(* keep open brace of compound statement on the same line as the prefix.
 * If statement is not a compound statement, then indent it with [indt] *)
and handle_compound_brace_prefix_indent prefix statement indt =
  if is_compound_statement statement then
    handle_compound_inline_brace prefix statement missing
  else
    group_doc (indent_doc prefix (get_doc statement) indt)

let pretty_print node =
  let empty_string = make_simple (text "") in
  let to_print = node |> get_doc |> add_break in
  let to_print = to_print ^| empty_string in
  let to_print = combine to_print in
  pretty 0 to_print

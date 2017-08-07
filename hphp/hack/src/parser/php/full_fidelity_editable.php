<?hh // strict
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 * This module contains the type describing the structure of a syntax tree.
 *
 **
 *
 */

require_once 'full_fidelity_parser.php';

abstract class EditableSyntax implements ArrayAccess {
  private string $_syntax_kind;
  protected ?int $_width;
  public function __construct(string $syntax_kind) {
    $this->_syntax_kind = $syntax_kind;
  }

  public function offsetExists (mixed $offset): bool {
    return $offset === 0;
  }

  public function offsetGet (mixed $offset): mixed {
    return $this;
  }

  public function offsetSet (mixed $offset, mixed $value): void {
  }

  public function offsetUnset (mixed $offset): void {
  }

  public function syntax_kind(): string {
    return $this->_syntax_kind;
  }

  public abstract function children():
    Generator<string, EditableSyntax, void>;

  public function preorder(): Continuation<EditableSyntax> {
    yield $this;
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
  }

  private function _parented_preorder(array<EditableSyntax> $parents):
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    $new_parents = $parents;
    array_push($new_parents, $this);
    yield tuple($this, $parents);
    foreach($this->children() as $name => $child)
      foreach($child->_parented_preorder($new_parents) as $descendant)
        yield $descendant;
  }

  public function parented_preorder():
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    return $this->_parented_preorder([]);
  }

  public function postorder(): Continuation<EditableSyntax> {
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
    yield $this;
  }

  public function is_token(): bool {
    return false;
  }

  public function is_trivia(): bool {
    return false;
  }

  public function is_list(): bool {
    return false;
  }

  public function is_missing(): bool {
    return false;
  }

  public function width(): int {
    if ($this->_width === null) {
      $width = 0;
      /* TODO: Make an accumulation sequence operator */
      foreach ($this->children() as $name => $node) {
        $width += $node->width();
      }
      $this->_width = $width;
      return $width;
    } else {
      return $this->_width;
    }
  }

  public function full_text(): string {
    /* TODO: Make an accumulation sequence operator */
    $s = '';
    foreach ($this->children() as $name => $node) {
      $s .= $node->full_text();
    }
    return $s;
  }

  public static function from_json(mixed $json, int $position, string $source) {
    switch($json->kind) {
    case 'token':
      return EditableToken::from_json($json->token, $position, $source);
    case 'list':
      return EditableList::from_json($json, $position, $source);
    case 'whitespace':
      return WhiteSpace::from_json($json, $position, $source);
    case 'end_of_line':
      return EndOfLine::from_json($json, $position, $source);
    case 'delimited_comment':
      return DelimitedComment::from_json($json, $position, $source);
    case 'single_line_comment':
      return SingleLineComment::from_json($json, $position, $source);
    case 'unsafe':
      return Unsafe::from_json($json, $position, $source);
    case 'unsafe_expression':
      return UnsafeExpression::from_json($json, $position, $source);
    case 'fix_me':
      return FixMe::from_json($json, $position, $source);
    case 'ignore_error':
      return IgnoreError::from_json($json, $position, $source);
    case 'fall_through':
      return FallThrough::from_json($json, $position, $source);
    case 'extra_token_error':
      return ExtraTokenError::from_json($json, $position, $source);

    case 'missing':
      return Missing::missing();
    case 'end_of_file':
      return EndOfFile::from_json($json, $position, $source);
    case 'script':
      return Script::from_json($json, $position, $source);
    case 'simple_type_specifier':
      return SimpleTypeSpecifier::from_json($json, $position, $source);
    case 'literal':
      return LiteralExpression::from_json($json, $position, $source);
    case 'variable':
      return VariableExpression::from_json($json, $position, $source);
    case 'qualified_name':
      return QualifiedNameExpression::from_json($json, $position, $source);
    case 'pipe_variable':
      return PipeVariableExpression::from_json($json, $position, $source);
    case 'enum_declaration':
      return EnumDeclaration::from_json($json, $position, $source);
    case 'enumerator':
      return Enumerator::from_json($json, $position, $source);
    case 'alias_declaration':
      return AliasDeclaration::from_json($json, $position, $source);
    case 'property_declaration':
      return PropertyDeclaration::from_json($json, $position, $source);
    case 'property_declarator':
      return PropertyDeclarator::from_json($json, $position, $source);
    case 'namespace_declaration':
      return NamespaceDeclaration::from_json($json, $position, $source);
    case 'namespace_body':
      return NamespaceBody::from_json($json, $position, $source);
    case 'namespace_empty_body':
      return NamespaceEmptyBody::from_json($json, $position, $source);
    case 'namespace_use_declaration':
      return NamespaceUseDeclaration::from_json($json, $position, $source);
    case 'namespace_group_use_declaration':
      return NamespaceGroupUseDeclaration::from_json($json, $position, $source);
    case 'namespace_use_clause':
      return NamespaceUseClause::from_json($json, $position, $source);
    case 'function_declaration':
      return FunctionDeclaration::from_json($json, $position, $source);
    case 'function_declaration_header':
      return FunctionDeclarationHeader::from_json($json, $position, $source);
    case 'where_clause':
      return WhereClause::from_json($json, $position, $source);
    case 'where_constraint':
      return WhereConstraint::from_json($json, $position, $source);
    case 'methodish_declaration':
      return MethodishDeclaration::from_json($json, $position, $source);
    case 'classish_declaration':
      return ClassishDeclaration::from_json($json, $position, $source);
    case 'classish_body':
      return ClassishBody::from_json($json, $position, $source);
    case 'trait_use_precedence_item':
      return TraitUsePrecedenceItem::from_json($json, $position, $source);
    case 'trait_use_alias_item':
      return TraitUseAliasItem::from_json($json, $position, $source);
    case 'trait_use_conflict_resolution':
      return TraitUseConflictResolution::from_json($json, $position, $source);
    case 'trait_use':
      return TraitUse::from_json($json, $position, $source);
    case 'require_clause':
      return RequireClause::from_json($json, $position, $source);
    case 'const_declaration':
      return ConstDeclaration::from_json($json, $position, $source);
    case 'constant_declarator':
      return ConstantDeclarator::from_json($json, $position, $source);
    case 'type_const_declaration':
      return TypeConstDeclaration::from_json($json, $position, $source);
    case 'decorated_expression':
      return DecoratedExpression::from_json($json, $position, $source);
    case 'parameter_declaration':
      return ParameterDeclaration::from_json($json, $position, $source);
    case 'variadic_parameter':
      return VariadicParameter::from_json($json, $position, $source);
    case 'attribute_specification':
      return AttributeSpecification::from_json($json, $position, $source);
    case 'attribute':
      return Attribute::from_json($json, $position, $source);
    case 'inclusion_expression':
      return InclusionExpression::from_json($json, $position, $source);
    case 'inclusion_directive':
      return InclusionDirective::from_json($json, $position, $source);
    case 'compound_statement':
      return CompoundStatement::from_json($json, $position, $source);
    case 'expression_statement':
      return ExpressionStatement::from_json($json, $position, $source);
    case 'markup_section':
      return MarkupSection::from_json($json, $position, $source);
    case 'markup_suffix':
      return MarkupSuffix::from_json($json, $position, $source);
    case 'unset_statement':
      return UnsetStatement::from_json($json, $position, $source);
    case 'while_statement':
      return WhileStatement::from_json($json, $position, $source);
    case 'if_statement':
      return IfStatement::from_json($json, $position, $source);
    case 'elseif_clause':
      return ElseifClause::from_json($json, $position, $source);
    case 'else_clause':
      return ElseClause::from_json($json, $position, $source);
    case 'try_statement':
      return TryStatement::from_json($json, $position, $source);
    case 'catch_clause':
      return CatchClause::from_json($json, $position, $source);
    case 'finally_clause':
      return FinallyClause::from_json($json, $position, $source);
    case 'do_statement':
      return DoStatement::from_json($json, $position, $source);
    case 'for_statement':
      return ForStatement::from_json($json, $position, $source);
    case 'foreach_statement':
      return ForeachStatement::from_json($json, $position, $source);
    case 'switch_statement':
      return SwitchStatement::from_json($json, $position, $source);
    case 'switch_section':
      return SwitchSection::from_json($json, $position, $source);
    case 'switch_fallthrough':
      return SwitchFallthrough::from_json($json, $position, $source);
    case 'case_label':
      return CaseLabel::from_json($json, $position, $source);
    case 'default_label':
      return DefaultLabel::from_json($json, $position, $source);
    case 'return_statement':
      return ReturnStatement::from_json($json, $position, $source);
    case 'goto_label':
      return GotoLabel::from_json($json, $position, $source);
    case 'goto_statement':
      return GotoStatement::from_json($json, $position, $source);
    case 'throw_statement':
      return ThrowStatement::from_json($json, $position, $source);
    case 'break_statement':
      return BreakStatement::from_json($json, $position, $source);
    case 'continue_statement':
      return ContinueStatement::from_json($json, $position, $source);
    case 'function_static_statement':
      return FunctionStaticStatement::from_json($json, $position, $source);
    case 'static_declarator':
      return StaticDeclarator::from_json($json, $position, $source);
    case 'echo_statement':
      return EchoStatement::from_json($json, $position, $source);
    case 'global_statement':
      return GlobalStatement::from_json($json, $position, $source);
    case 'simple_initializer':
      return SimpleInitializer::from_json($json, $position, $source);
    case 'anonymous_function':
      return AnonymousFunction::from_json($json, $position, $source);
    case 'anonymous_function_use_clause':
      return AnonymousFunctionUseClause::from_json($json, $position, $source);
    case 'lambda_expression':
      return LambdaExpression::from_json($json, $position, $source);
    case 'lambda_signature':
      return LambdaSignature::from_json($json, $position, $source);
    case 'cast_expression':
      return CastExpression::from_json($json, $position, $source);
    case 'scope_resolution_expression':
      return ScopeResolutionExpression::from_json($json, $position, $source);
    case 'member_selection_expression':
      return MemberSelectionExpression::from_json($json, $position, $source);
    case 'safe_member_selection_expression':
      return SafeMemberSelectionExpression::from_json($json, $position, $source);
    case 'embedded_member_selection_expression':
      return EmbeddedMemberSelectionExpression::from_json($json, $position, $source);
    case 'yield_expression':
      return YieldExpression::from_json($json, $position, $source);
    case 'yield_from_expression':
      return YieldFromExpression::from_json($json, $position, $source);
    case 'prefix_unary_expression':
      return PrefixUnaryExpression::from_json($json, $position, $source);
    case 'postfix_unary_expression':
      return PostfixUnaryExpression::from_json($json, $position, $source);
    case 'binary_expression':
      return BinaryExpression::from_json($json, $position, $source);
    case 'instanceof_expression':
      return InstanceofExpression::from_json($json, $position, $source);
    case 'conditional_expression':
      return ConditionalExpression::from_json($json, $position, $source);
    case 'eval_expression':
      return EvalExpression::from_json($json, $position, $source);
    case 'empty_expression':
      return EmptyExpression::from_json($json, $position, $source);
    case 'define_expression':
      return DefineExpression::from_json($json, $position, $source);
    case 'isset_expression':
      return IssetExpression::from_json($json, $position, $source);
    case 'function_call_expression':
      return FunctionCallExpression::from_json($json, $position, $source);
    case 'function_call_with_type_arguments_expression':
      return FunctionCallWithTypeArgumentsExpression::from_json($json, $position, $source);
    case 'parenthesized_expression':
      return ParenthesizedExpression::from_json($json, $position, $source);
    case 'braced_expression':
      return BracedExpression::from_json($json, $position, $source);
    case 'embedded_braced_expression':
      return EmbeddedBracedExpression::from_json($json, $position, $source);
    case 'list_expression':
      return ListExpression::from_json($json, $position, $source);
    case 'collection_literal_expression':
      return CollectionLiteralExpression::from_json($json, $position, $source);
    case 'object_creation_expression':
      return ObjectCreationExpression::from_json($json, $position, $source);
    case 'array_creation_expression':
      return ArrayCreationExpression::from_json($json, $position, $source);
    case 'array_intrinsic_expression':
      return ArrayIntrinsicExpression::from_json($json, $position, $source);
    case 'darray_intrinsic_expression':
      return DarrayIntrinsicExpression::from_json($json, $position, $source);
    case 'dictionary_intrinsic_expression':
      return DictionaryIntrinsicExpression::from_json($json, $position, $source);
    case 'keyset_intrinsic_expression':
      return KeysetIntrinsicExpression::from_json($json, $position, $source);
    case 'varray_intrinsic_expression':
      return VarrayIntrinsicExpression::from_json($json, $position, $source);
    case 'vector_intrinsic_expression':
      return VectorIntrinsicExpression::from_json($json, $position, $source);
    case 'element_initializer':
      return ElementInitializer::from_json($json, $position, $source);
    case 'subscript_expression':
      return SubscriptExpression::from_json($json, $position, $source);
    case 'embedded_subscript_expression':
      return EmbeddedSubscriptExpression::from_json($json, $position, $source);
    case 'awaitable_creation_expression':
      return AwaitableCreationExpression::from_json($json, $position, $source);
    case 'xhp_children_declaration':
      return XHPChildrenDeclaration::from_json($json, $position, $source);
    case 'xhp_children_parenthesized_list':
      return XHPChildrenParenthesizedList::from_json($json, $position, $source);
    case 'xhp_category_declaration':
      return XHPCategoryDeclaration::from_json($json, $position, $source);
    case 'xhp_enum_type':
      return XHPEnumType::from_json($json, $position, $source);
    case 'xhp_required':
      return XHPRequired::from_json($json, $position, $source);
    case 'xhp_class_attribute_declaration':
      return XHPClassAttributeDeclaration::from_json($json, $position, $source);
    case 'xhp_class_attribute':
      return XHPClassAttribute::from_json($json, $position, $source);
    case 'xhp_simple_class_attribute':
      return XHPSimpleClassAttribute::from_json($json, $position, $source);
    case 'xhp_attribute':
      return XHPAttribute::from_json($json, $position, $source);
    case 'xhp_open':
      return XHPOpen::from_json($json, $position, $source);
    case 'xhp_expression':
      return XHPExpression::from_json($json, $position, $source);
    case 'xhp_close':
      return XHPClose::from_json($json, $position, $source);
    case 'type_constant':
      return TypeConstant::from_json($json, $position, $source);
    case 'vector_type_specifier':
      return VectorTypeSpecifier::from_json($json, $position, $source);
    case 'keyset_type_specifier':
      return KeysetTypeSpecifier::from_json($json, $position, $source);
    case 'tuple_type_explicit_specifier':
      return TupleTypeExplicitSpecifier::from_json($json, $position, $source);
    case 'varray_type_specifier':
      return VarrayTypeSpecifier::from_json($json, $position, $source);
    case 'vector_array_type_specifier':
      return VectorArrayTypeSpecifier::from_json($json, $position, $source);
    case 'type_parameter':
      return TypeParameter::from_json($json, $position, $source);
    case 'type_constraint':
      return TypeConstraint::from_json($json, $position, $source);
    case 'darray_type_specifier':
      return DarrayTypeSpecifier::from_json($json, $position, $source);
    case 'map_array_type_specifier':
      return MapArrayTypeSpecifier::from_json($json, $position, $source);
    case 'dictionary_type_specifier':
      return DictionaryTypeSpecifier::from_json($json, $position, $source);
    case 'closure_type_specifier':
      return ClosureTypeSpecifier::from_json($json, $position, $source);
    case 'classname_type_specifier':
      return ClassnameTypeSpecifier::from_json($json, $position, $source);
    case 'field_specifier':
      return FieldSpecifier::from_json($json, $position, $source);
    case 'field_initializer':
      return FieldInitializer::from_json($json, $position, $source);
    case 'shape_type_specifier':
      return ShapeTypeSpecifier::from_json($json, $position, $source);
    case 'shape_expression':
      return ShapeExpression::from_json($json, $position, $source);
    case 'tuple_expression':
      return TupleExpression::from_json($json, $position, $source);
    case 'generic_type_specifier':
      return GenericTypeSpecifier::from_json($json, $position, $source);
    case 'nullable_type_specifier':
      return NullableTypeSpecifier::from_json($json, $position, $source);
    case 'soft_type_specifier':
      return SoftTypeSpecifier::from_json($json, $position, $source);
    case 'type_arguments':
      return TypeArguments::from_json($json, $position, $source);
    case 'type_parameters':
      return TypeParameters::from_json($json, $position, $source);
    case 'tuple_type_specifier':
      return TupleTypeSpecifier::from_json($json, $position, $source);
    case 'error':
      return ErrorSyntax::from_json($json, $position, $source);
    case 'list_item':
      return ListItem::from_json($json, $position, $source);

    default:
      throw new Exception('unexpected json kind: ' . $json->kind);
      // TODO: Better exception
    }
  }

  public function to_array(): array<EditableSyntax> {
    return [$this];
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach($this->children() as $child) {
      $accumulator = $child->reduce($reducer, $accumulator, $new_parents);
    }
    return $reducer($this, $accumulator, $parents ?? []);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  public function find_with_parents(
    (function(EditableSyntax):bool) $predicate,
    ?array<EditableSyntax> $parents = null): array<EditableSyntax> {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    if ($predicate($this))
      return $new_parents;
    foreach($this->children() as $child) {
      $result = $child->find_with_parents($predicate, $new_parents);
      if (count($result) != 0)
        return $result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  public function filter(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    $reducer = ($node, $acc, $parents) ==> {
      if ($predicate($node, $parents))
        array_push($acc, $node);
      return $acc;
    };
    return $this->reduce($reducer, []);
  }

  public function of_syntax_kind(string $kind): Continuation<EditableSyntax> {
    foreach($this->preorder() as $child)
      if ($child->syntax_kind() === $kind)
        yield $child;
  }

  public function remove_where(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    return $this->rewrite(
      ($node, $parents) ==>
        $predicate($node, $parents) ? Missing::missing() : $node);
  }

  public function without(EditableSyntax $target): EditableSyntax {
    return $this->remove_where(($node, $parents) ==> $node === $target);
  }

  public function replace(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    return $this->rewrite(
      ($node, $parents) ==> $node === $target ? $new_node : $node);
  }

  public function leftmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;
    foreach($this->children() as $child)
    {
      if (!$child->is_missing())
        return $child->leftmost_token();
    }
    return null;
  }

  public function rightmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;

    // TODO: Better way to reverse a sequence?
    foreach (array_reverse(iterator_to_array($this->children())) as $child) {
      if (!$child->is_missing())
        return $child->rightmost_token();
    }
    return null;
  }

  public function insert_before(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    // Inserting before missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_before.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->leftmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      $new_leading = EditableList::concatenate_lists(
        $token->leading(), $new_node);
      $new_token = $token->with_leading($new_leading);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableList::concatenate_lists($new_node, $target), $target);
  }

  public function insert_after(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {

    // Inserting after missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_after.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->rightmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      $new_trailing = EditableList::concatenate_lists(
        $new_node, $token->trailing());
      $new_token = $token->with_trailing($new_trailing);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableSyntax::concatenate_lists($target, $new_node), $target);
  }
}

final class EditableList extends EditableSyntax implements ArrayAccess {
  private array<EditableSyntax> $_children;
  public function __construct(array<EditableSyntax> $children) {
    parent::__construct('list');
    $this->_children = $children;
  }

  public function offsetExists(mixed $offset): bool {
    return array_key_exists($offset, $this->_children);
  }

  public function offsetGet(mixed $offset): mixed {
    return $this->_children[$offset];
  }

  public function offsetSet(mixed $offset, mixed $value): void {
  }

  public function offsetUnset(mixed $offset): void {
  }

  public function is_list(): bool {
    return true;
  }

  public function to_array(): array<EditableSyntax> {
    return $this->_children;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    foreach($this->_children as $key => $node)
      yield $key => $node;
  }

  /* TODO: Getter by index? */

  public static function to_list(
    array<EditableSyntax> $syntax_list): EditableSyntax {
    if (count($syntax_list) === 0)
      return Missing::missing();
    else
      return new EditableList($syntax_list);
  }

  public static function concatenate_lists(
    EditableSyntax $left,
    EditableSyntax $right): EditableSyntax {
    if ($left->is_missing())
      return $right;
    if ($right->is_missing())
      return $left;
    return new EditableList(
      array_merge($left->to_array(), $right->to_array()));
  }

  public static function from_json(mixed $json, int $position, string $source) {
    // TODO Implement array map
    $children = [];
    $current_position = $position;
    foreach($json->elements as $element)
    {
      $child = EditableSyntax::from_json($element, $current_position, $source);
      array_push($children, $child);
      $current_position += $child->width();
    }
    return new EditableList($children);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $dirty = false;
    $new_children = [];
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach ($this->children() as $child)
    {
      $new_child = $child->rewrite($rewriter, $new_parents);
      if ($new_child != $child)
        $dirty = true;
      if ($new_child != null)
      {
        if ($new_child->is_list())
        {
          foreach($new_child->children() as $n)
            array_push($new_children, $n);
        }
        else
          array_push($new_children, $new_child);
      }
    }
    $result = $this;
    if ($dirty) {
      if (count($new_children) === 0)
        $result = Missing::missing();
      else if (count($new_children) === 1)
        $result = $new_children[0];
      else
        $result = new EditableList($new_children);
    }
    return $rewriter($result, $parents ?? []);
  }
}

abstract class EditableToken extends EditableSyntax {
  private string $_token_kind;
  private EditableSyntax $_leading;
  private EditableSyntax $_trailing;

  public function __construct(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('token');
    $this->_token_kind = $token_kind;
    $this->_text = $text;
    $this->_leading = $leading;
    $this->_trailing = $trailing;
    $this->_width = strlen($text) +
      $leading->width() + $trailing->width();
  }

  public function token_kind(): string {
    return $this->_token_kind;
  }

  public function text(): string {
    return $this->_text;
  }

  public function leading(): EditableSyntax {
    return $this->_leading;
  }

  public function trailing(): EditableSyntax {
    return $this->_trailing;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield 'leading' => $this->leading();
    yield 'trailing' => $this->trailing();
  }

  public function is_token(): bool {
    return true;
  }

  public function full_text(): string {
    return $this->leading()->full_text() .
      $this->text() .
      $this->trailing()->full_text();
  }

  public abstract function with_leading(
    EditableSyntax $leading): EditableToken;

  public abstract function with_trailing(
    EditableSyntax $trailing): EditableToken;

  private static function factory(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $token_text) {
    switch($token_kind) {
    case 'end_of_file':
       return new EndOfFileToken($leading, $trailing);

    case 'abstract':
       return new AbstractToken($leading, $trailing);
    case 'and':
       return new AndToken($leading, $trailing);
    case 'array':
       return new ArrayToken($leading, $trailing);
    case 'arraykey':
       return new ArraykeyToken($leading, $trailing);
    case 'as':
       return new AsToken($leading, $trailing);
    case 'async':
       return new AsyncToken($leading, $trailing);
    case 'attribute':
       return new AttributeToken($leading, $trailing);
    case 'await':
       return new AwaitToken($leading, $trailing);
    case 'bool':
       return new BoolToken($leading, $trailing);
    case 'break':
       return new BreakToken($leading, $trailing);
    case 'case':
       return new CaseToken($leading, $trailing);
    case 'catch':
       return new CatchToken($leading, $trailing);
    case 'category':
       return new CategoryToken($leading, $trailing);
    case 'children':
       return new ChildrenToken($leading, $trailing);
    case 'class':
       return new ClassToken($leading, $trailing);
    case 'classname':
       return new ClassnameToken($leading, $trailing);
    case 'clone':
       return new CloneToken($leading, $trailing);
    case 'const':
       return new ConstToken($leading, $trailing);
    case '__construct':
       return new ConstructToken($leading, $trailing);
    case 'continue':
       return new ContinueToken($leading, $trailing);
    case 'coroutine':
       return new CoroutineToken($leading, $trailing);
    case 'darray':
       return new DarrayToken($leading, $trailing);
    case 'default':
       return new DefaultToken($leading, $trailing);
    case 'define':
       return new DefineToken($leading, $trailing);
    case '__destruct':
       return new DestructToken($leading, $trailing);
    case 'dict':
       return new DictToken($leading, $trailing);
    case 'do':
       return new DoToken($leading, $trailing);
    case 'double':
       return new DoubleToken($leading, $trailing);
    case 'echo':
       return new EchoToken($leading, $trailing);
    case 'else':
       return new ElseToken($leading, $trailing);
    case 'elseif':
       return new ElseifToken($leading, $trailing);
    case 'empty':
       return new EmptyToken($leading, $trailing);
    case 'enum':
       return new EnumToken($leading, $trailing);
    case 'eval':
       return new EvalToken($leading, $trailing);
    case 'extends':
       return new ExtendsToken($leading, $trailing);
    case 'fallthrough':
       return new FallthroughToken($leading, $trailing);
    case 'float':
       return new FloatToken($leading, $trailing);
    case 'final':
       return new FinalToken($leading, $trailing);
    case 'finally':
       return new FinallyToken($leading, $trailing);
    case 'for':
       return new ForToken($leading, $trailing);
    case 'foreach':
       return new ForeachToken($leading, $trailing);
    case 'from':
       return new FromToken($leading, $trailing);
    case 'function':
       return new FunctionToken($leading, $trailing);
    case 'global':
       return new GlobalToken($leading, $trailing);
    case 'goto':
       return new GotoToken($leading, $trailing);
    case 'if':
       return new IfToken($leading, $trailing);
    case 'implements':
       return new ImplementsToken($leading, $trailing);
    case 'include':
       return new IncludeToken($leading, $trailing);
    case 'include_once':
       return new Include_onceToken($leading, $trailing);
    case 'instanceof':
       return new InstanceofToken($leading, $trailing);
    case 'insteadof':
       return new InsteadofToken($leading, $trailing);
    case 'int':
       return new IntToken($leading, $trailing);
    case 'interface':
       return new InterfaceToken($leading, $trailing);
    case 'isset':
       return new IssetToken($leading, $trailing);
    case 'keyset':
       return new KeysetToken($leading, $trailing);
    case 'list':
       return new ListToken($leading, $trailing);
    case 'mixed':
       return new MixedToken($leading, $trailing);
    case 'namespace':
       return new NamespaceToken($leading, $trailing);
    case 'new':
       return new NewToken($leading, $trailing);
    case 'newtype':
       return new NewtypeToken($leading, $trailing);
    case 'noreturn':
       return new NoreturnToken($leading, $trailing);
    case 'num':
       return new NumToken($leading, $trailing);
    case 'object':
       return new ObjectToken($leading, $trailing);
    case 'or':
       return new OrToken($leading, $trailing);
    case 'parent':
       return new ParentToken($leading, $trailing);
    case 'print':
       return new PrintToken($leading, $trailing);
    case 'private':
       return new PrivateToken($leading, $trailing);
    case 'protected':
       return new ProtectedToken($leading, $trailing);
    case 'public':
       return new PublicToken($leading, $trailing);
    case 'require':
       return new RequireToken($leading, $trailing);
    case 'require_once':
       return new Require_onceToken($leading, $trailing);
    case 'required':
       return new RequiredToken($leading, $trailing);
    case 'resource':
       return new ResourceToken($leading, $trailing);
    case 'return':
       return new ReturnToken($leading, $trailing);
    case 'self':
       return new SelfToken($leading, $trailing);
    case 'shape':
       return new ShapeToken($leading, $trailing);
    case 'static':
       return new StaticToken($leading, $trailing);
    case 'string':
       return new StringToken($leading, $trailing);
    case 'super':
       return new SuperToken($leading, $trailing);
    case 'suspend':
       return new SuspendToken($leading, $trailing);
    case 'switch':
       return new SwitchToken($leading, $trailing);
    case 'this':
       return new ThisToken($leading, $trailing);
    case 'throw':
       return new ThrowToken($leading, $trailing);
    case 'trait':
       return new TraitToken($leading, $trailing);
    case 'try':
       return new TryToken($leading, $trailing);
    case 'tuple':
       return new TupleToken($leading, $trailing);
    case 'type':
       return new TypeToken($leading, $trailing);
    case 'unset':
       return new UnsetToken($leading, $trailing);
    case 'use':
       return new UseToken($leading, $trailing);
    case 'var':
       return new VarToken($leading, $trailing);
    case 'varray':
       return new VarrayToken($leading, $trailing);
    case 'vec':
       return new VecToken($leading, $trailing);
    case 'void':
       return new VoidToken($leading, $trailing);
    case 'where':
       return new WhereToken($leading, $trailing);
    case 'while':
       return new WhileToken($leading, $trailing);
    case 'xor':
       return new XorToken($leading, $trailing);
    case 'yield':
       return new YieldToken($leading, $trailing);
    case '[':
       return new LeftBracketToken($leading, $trailing);
    case ']':
       return new RightBracketToken($leading, $trailing);
    case '(':
       return new LeftParenToken($leading, $trailing);
    case ')':
       return new RightParenToken($leading, $trailing);
    case '{':
       return new LeftBraceToken($leading, $trailing);
    case '}':
       return new RightBraceToken($leading, $trailing);
    case '.':
       return new DotToken($leading, $trailing);
    case '->':
       return new MinusGreaterThanToken($leading, $trailing);
    case '++':
       return new PlusPlusToken($leading, $trailing);
    case '--':
       return new MinusMinusToken($leading, $trailing);
    case '**':
       return new StarStarToken($leading, $trailing);
    case '*':
       return new StarToken($leading, $trailing);
    case '+':
       return new PlusToken($leading, $trailing);
    case '-':
       return new MinusToken($leading, $trailing);
    case '~':
       return new TildeToken($leading, $trailing);
    case '!':
       return new ExclamationToken($leading, $trailing);
    case '$':
       return new DollarToken($leading, $trailing);
    case '/':
       return new SlashToken($leading, $trailing);
    case '%':
       return new PercentToken($leading, $trailing);
    case '<>':
       return new LessThanGreaterThanToken($leading, $trailing);
    case '<=>':
       return new LessThanEqualGreaterThanToken($leading, $trailing);
    case '<<':
       return new LessThanLessThanToken($leading, $trailing);
    case '>>':
       return new GreaterThanGreaterThanToken($leading, $trailing);
    case '<':
       return new LessThanToken($leading, $trailing);
    case '>':
       return new GreaterThanToken($leading, $trailing);
    case '<=':
       return new LessThanEqualToken($leading, $trailing);
    case '>=':
       return new GreaterThanEqualToken($leading, $trailing);
    case '==':
       return new EqualEqualToken($leading, $trailing);
    case '===':
       return new EqualEqualEqualToken($leading, $trailing);
    case '!=':
       return new ExclamationEqualToken($leading, $trailing);
    case '!==':
       return new ExclamationEqualEqualToken($leading, $trailing);
    case '^':
       return new CaratToken($leading, $trailing);
    case '|':
       return new BarToken($leading, $trailing);
    case '&':
       return new AmpersandToken($leading, $trailing);
    case '&&':
       return new AmpersandAmpersandToken($leading, $trailing);
    case '||':
       return new BarBarToken($leading, $trailing);
    case '?':
       return new QuestionToken($leading, $trailing);
    case '??':
       return new QuestionQuestionToken($leading, $trailing);
    case ':':
       return new ColonToken($leading, $trailing);
    case ';':
       return new SemicolonToken($leading, $trailing);
    case '=':
       return new EqualToken($leading, $trailing);
    case '**=':
       return new StarStarEqualToken($leading, $trailing);
    case '*=':
       return new StarEqualToken($leading, $trailing);
    case '/=':
       return new SlashEqualToken($leading, $trailing);
    case '%=':
       return new PercentEqualToken($leading, $trailing);
    case '+=':
       return new PlusEqualToken($leading, $trailing);
    case '-=':
       return new MinusEqualToken($leading, $trailing);
    case '.=':
       return new DotEqualToken($leading, $trailing);
    case '<<=':
       return new LessThanLessThanEqualToken($leading, $trailing);
    case '>>=':
       return new GreaterThanGreaterThanEqualToken($leading, $trailing);
    case '&=':
       return new AmpersandEqualToken($leading, $trailing);
    case '^=':
       return new CaratEqualToken($leading, $trailing);
    case '|=':
       return new BarEqualToken($leading, $trailing);
    case ',':
       return new CommaToken($leading, $trailing);
    case '@':
       return new AtToken($leading, $trailing);
    case '::':
       return new ColonColonToken($leading, $trailing);
    case '=>':
       return new EqualGreaterThanToken($leading, $trailing);
    case '==>':
       return new EqualEqualGreaterThanToken($leading, $trailing);
    case '?->':
       return new QuestionMinusGreaterThanToken($leading, $trailing);
    case '...':
       return new DotDotDotToken($leading, $trailing);
    case '$$':
       return new DollarDollarToken($leading, $trailing);
    case '|>':
       return new BarGreaterThanToken($leading, $trailing);
    case 'null':
       return new NullLiteralToken($leading, $trailing);
    case '/>':
       return new SlashGreaterThanToken($leading, $trailing);
    case '</':
       return new LessThanSlashToken($leading, $trailing);
    case '<?':
       return new LessThanQuestionToken($leading, $trailing);
    case '?>':
       return new QuestionGreaterThanToken($leading, $trailing);

    case 'error_token':
       return new ErrorTokenToken($leading, $trailing, $token_text);
    case 'name':
       return new NameToken($leading, $trailing, $token_text);
    case 'qualified_name':
       return new QualifiedNameToken($leading, $trailing, $token_text);
    case 'variable':
       return new VariableToken($leading, $trailing, $token_text);
    case 'namespace_prefix':
       return new NamespacePrefixToken($leading, $trailing, $token_text);
    case 'decimal_literal':
       return new DecimalLiteralToken($leading, $trailing, $token_text);
    case 'octal_literal':
       return new OctalLiteralToken($leading, $trailing, $token_text);
    case 'hexadecimal_literal':
       return new HexadecimalLiteralToken($leading, $trailing, $token_text);
    case 'binary_literal':
       return new BinaryLiteralToken($leading, $trailing, $token_text);
    case 'floating_literal':
       return new FloatingLiteralToken($leading, $trailing, $token_text);
    case 'execution_string':
       return new ExecutionStringToken($leading, $trailing, $token_text);
    case 'single_quoted_string_literal':
       return new SingleQuotedStringLiteralToken($leading, $trailing, $token_text);
    case 'double_quoted_string_literal':
       return new DoubleQuotedStringLiteralToken($leading, $trailing, $token_text);
    case 'double_quoted_string_literal_head':
       return new DoubleQuotedStringLiteralHeadToken($leading, $trailing, $token_text);
    case 'string_literal_body':
       return new StringLiteralBodyToken($leading, $trailing, $token_text);
    case 'double_quoted_string_literal_tail':
       return new DoubleQuotedStringLiteralTailToken($leading, $trailing, $token_text);
    case 'heredoc_string_literal':
       return new HeredocStringLiteralToken($leading, $trailing, $token_text);
    case 'heredoc_string_literal_head':
       return new HeredocStringLiteralHeadToken($leading, $trailing, $token_text);
    case 'heredoc_string_literal_tail':
       return new HeredocStringLiteralTailToken($leading, $trailing, $token_text);
    case 'nowdoc_string_literal':
       return new NowdocStringLiteralToken($leading, $trailing, $token_text);
    case 'boolean_literal':
       return new BooleanLiteralToken($leading, $trailing, $token_text);
    case 'XHP_category_name':
       return new XHPCategoryNameToken($leading, $trailing, $token_text);
    case 'XHP_element_name':
       return new XHPElementNameToken($leading, $trailing, $token_text);
    case 'XHP_class_name':
       return new XHPClassNameToken($leading, $trailing, $token_text);
    case 'XHP_string_literal':
       return new XHPStringLiteralToken($leading, $trailing, $token_text);
    case 'XHP_body':
       return new XHPBodyToken($leading, $trailing, $token_text);
    case 'XHP_comment':
       return new XHPCommentToken($leading, $trailing, $token_text);
    case 'markup':
       return new MarkupToken($leading, $trailing, $token_text);

      default:
        throw new Exception('unexpected token kind: ' . $token_kind);
        // TODO: Better error
    }
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $leading = $this->leading()->rewrite($rewriter, $new_parents);
    $trailing = $this->trailing()->rewrite($rewriter, $new_parents);
    if ($leading === $this->leading() && $trailing === $this->trailing())
      return $rewriter($this, $parents ?? []);
    else
      return $rewriter(EditableToken::factory(
        $this->token_kind(), $leading, $trailing,
        $this->text()), $parents ?? []);
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $accumulator = $this->leading()->reduce($reducer, $accumulator);
    $accumulator = $reducer($this, $accumulator, $parents ?? []);
    $accumulator = $this->trailing()->reduce($reducer, $accumulator);
    return $accumulator;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source): EditableToken {
    $leading_list = fold_map(
      $json->leading,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $position);

    $leading = EditableList::to_list($leading_list);
    $token_position = $position + $leading->width();
    $token_width = $json->width;
    $token_text = substr($source, $token_position, $token_width);
    $trailing_position = $token_position + $token_width;
    $trailing_list = fold_map(
      $json->trailing,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $trailing_position);
    $trailing = EditableList::to_list($trailing_list);
    return EditableToken::factory(
      $json->kind, $leading, $trailing, $token_text);
  }
}

final class EndOfFileToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('end_of_file', $leading, $trailing, '');
  }

  public function with_leading(EditableSyntax $leading): EndOfFileToken {
    return new EndOfFileToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EndOfFileToken {
    return new EndOfFileToken($this->leading(), $trailing);
  }
}

final class AbstractToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('abstract', $leading, $trailing, 'abstract');
  }

  public function with_leading(EditableSyntax $leading): AbstractToken {
    return new AbstractToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AbstractToken {
    return new AbstractToken($this->leading(), $trailing);
  }
}
final class AndToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('and', $leading, $trailing, 'and');
  }

  public function with_leading(EditableSyntax $leading): AndToken {
    return new AndToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AndToken {
    return new AndToken($this->leading(), $trailing);
  }
}
final class ArrayToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('array', $leading, $trailing, 'array');
  }

  public function with_leading(EditableSyntax $leading): ArrayToken {
    return new ArrayToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ArrayToken {
    return new ArrayToken($this->leading(), $trailing);
  }
}
final class ArraykeyToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('arraykey', $leading, $trailing, 'arraykey');
  }

  public function with_leading(EditableSyntax $leading): ArraykeyToken {
    return new ArraykeyToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ArraykeyToken {
    return new ArraykeyToken($this->leading(), $trailing);
  }
}
final class AsToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('as', $leading, $trailing, 'as');
  }

  public function with_leading(EditableSyntax $leading): AsToken {
    return new AsToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AsToken {
    return new AsToken($this->leading(), $trailing);
  }
}
final class AsyncToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('async', $leading, $trailing, 'async');
  }

  public function with_leading(EditableSyntax $leading): AsyncToken {
    return new AsyncToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AsyncToken {
    return new AsyncToken($this->leading(), $trailing);
  }
}
final class AttributeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('attribute', $leading, $trailing, 'attribute');
  }

  public function with_leading(EditableSyntax $leading): AttributeToken {
    return new AttributeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AttributeToken {
    return new AttributeToken($this->leading(), $trailing);
  }
}
final class AwaitToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('await', $leading, $trailing, 'await');
  }

  public function with_leading(EditableSyntax $leading): AwaitToken {
    return new AwaitToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AwaitToken {
    return new AwaitToken($this->leading(), $trailing);
  }
}
final class BoolToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('bool', $leading, $trailing, 'bool');
  }

  public function with_leading(EditableSyntax $leading): BoolToken {
    return new BoolToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BoolToken {
    return new BoolToken($this->leading(), $trailing);
  }
}
final class BreakToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('break', $leading, $trailing, 'break');
  }

  public function with_leading(EditableSyntax $leading): BreakToken {
    return new BreakToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BreakToken {
    return new BreakToken($this->leading(), $trailing);
  }
}
final class CaseToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('case', $leading, $trailing, 'case');
  }

  public function with_leading(EditableSyntax $leading): CaseToken {
    return new CaseToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CaseToken {
    return new CaseToken($this->leading(), $trailing);
  }
}
final class CatchToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('catch', $leading, $trailing, 'catch');
  }

  public function with_leading(EditableSyntax $leading): CatchToken {
    return new CatchToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CatchToken {
    return new CatchToken($this->leading(), $trailing);
  }
}
final class CategoryToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('category', $leading, $trailing, 'category');
  }

  public function with_leading(EditableSyntax $leading): CategoryToken {
    return new CategoryToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CategoryToken {
    return new CategoryToken($this->leading(), $trailing);
  }
}
final class ChildrenToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('children', $leading, $trailing, 'children');
  }

  public function with_leading(EditableSyntax $leading): ChildrenToken {
    return new ChildrenToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ChildrenToken {
    return new ChildrenToken($this->leading(), $trailing);
  }
}
final class ClassToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('class', $leading, $trailing, 'class');
  }

  public function with_leading(EditableSyntax $leading): ClassToken {
    return new ClassToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ClassToken {
    return new ClassToken($this->leading(), $trailing);
  }
}
final class ClassnameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('classname', $leading, $trailing, 'classname');
  }

  public function with_leading(EditableSyntax $leading): ClassnameToken {
    return new ClassnameToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ClassnameToken {
    return new ClassnameToken($this->leading(), $trailing);
  }
}
final class CloneToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('clone', $leading, $trailing, 'clone');
  }

  public function with_leading(EditableSyntax $leading): CloneToken {
    return new CloneToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CloneToken {
    return new CloneToken($this->leading(), $trailing);
  }
}
final class ConstToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('const', $leading, $trailing, 'const');
  }

  public function with_leading(EditableSyntax $leading): ConstToken {
    return new ConstToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ConstToken {
    return new ConstToken($this->leading(), $trailing);
  }
}
final class ConstructToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('__construct', $leading, $trailing, '__construct');
  }

  public function with_leading(EditableSyntax $leading): ConstructToken {
    return new ConstructToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ConstructToken {
    return new ConstructToken($this->leading(), $trailing);
  }
}
final class ContinueToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('continue', $leading, $trailing, 'continue');
  }

  public function with_leading(EditableSyntax $leading): ContinueToken {
    return new ContinueToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ContinueToken {
    return new ContinueToken($this->leading(), $trailing);
  }
}
final class CoroutineToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('coroutine', $leading, $trailing, 'coroutine');
  }

  public function with_leading(EditableSyntax $leading): CoroutineToken {
    return new CoroutineToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CoroutineToken {
    return new CoroutineToken($this->leading(), $trailing);
  }
}
final class DarrayToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('darray', $leading, $trailing, 'darray');
  }

  public function with_leading(EditableSyntax $leading): DarrayToken {
    return new DarrayToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DarrayToken {
    return new DarrayToken($this->leading(), $trailing);
  }
}
final class DefaultToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('default', $leading, $trailing, 'default');
  }

  public function with_leading(EditableSyntax $leading): DefaultToken {
    return new DefaultToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DefaultToken {
    return new DefaultToken($this->leading(), $trailing);
  }
}
final class DefineToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('define', $leading, $trailing, 'define');
  }

  public function with_leading(EditableSyntax $leading): DefineToken {
    return new DefineToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DefineToken {
    return new DefineToken($this->leading(), $trailing);
  }
}
final class DestructToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('__destruct', $leading, $trailing, '__destruct');
  }

  public function with_leading(EditableSyntax $leading): DestructToken {
    return new DestructToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DestructToken {
    return new DestructToken($this->leading(), $trailing);
  }
}
final class DictToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('dict', $leading, $trailing, 'dict');
  }

  public function with_leading(EditableSyntax $leading): DictToken {
    return new DictToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DictToken {
    return new DictToken($this->leading(), $trailing);
  }
}
final class DoToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('do', $leading, $trailing, 'do');
  }

  public function with_leading(EditableSyntax $leading): DoToken {
    return new DoToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DoToken {
    return new DoToken($this->leading(), $trailing);
  }
}
final class DoubleToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('double', $leading, $trailing, 'double');
  }

  public function with_leading(EditableSyntax $leading): DoubleToken {
    return new DoubleToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DoubleToken {
    return new DoubleToken($this->leading(), $trailing);
  }
}
final class EchoToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('echo', $leading, $trailing, 'echo');
  }

  public function with_leading(EditableSyntax $leading): EchoToken {
    return new EchoToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EchoToken {
    return new EchoToken($this->leading(), $trailing);
  }
}
final class ElseToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('else', $leading, $trailing, 'else');
  }

  public function with_leading(EditableSyntax $leading): ElseToken {
    return new ElseToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ElseToken {
    return new ElseToken($this->leading(), $trailing);
  }
}
final class ElseifToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('elseif', $leading, $trailing, 'elseif');
  }

  public function with_leading(EditableSyntax $leading): ElseifToken {
    return new ElseifToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ElseifToken {
    return new ElseifToken($this->leading(), $trailing);
  }
}
final class EmptyToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('empty', $leading, $trailing, 'empty');
  }

  public function with_leading(EditableSyntax $leading): EmptyToken {
    return new EmptyToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EmptyToken {
    return new EmptyToken($this->leading(), $trailing);
  }
}
final class EnumToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('enum', $leading, $trailing, 'enum');
  }

  public function with_leading(EditableSyntax $leading): EnumToken {
    return new EnumToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EnumToken {
    return new EnumToken($this->leading(), $trailing);
  }
}
final class EvalToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('eval', $leading, $trailing, 'eval');
  }

  public function with_leading(EditableSyntax $leading): EvalToken {
    return new EvalToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EvalToken {
    return new EvalToken($this->leading(), $trailing);
  }
}
final class ExtendsToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('extends', $leading, $trailing, 'extends');
  }

  public function with_leading(EditableSyntax $leading): ExtendsToken {
    return new ExtendsToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ExtendsToken {
    return new ExtendsToken($this->leading(), $trailing);
  }
}
final class FallthroughToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('fallthrough', $leading, $trailing, 'fallthrough');
  }

  public function with_leading(EditableSyntax $leading): FallthroughToken {
    return new FallthroughToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FallthroughToken {
    return new FallthroughToken($this->leading(), $trailing);
  }
}
final class FloatToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('float', $leading, $trailing, 'float');
  }

  public function with_leading(EditableSyntax $leading): FloatToken {
    return new FloatToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FloatToken {
    return new FloatToken($this->leading(), $trailing);
  }
}
final class FinalToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('final', $leading, $trailing, 'final');
  }

  public function with_leading(EditableSyntax $leading): FinalToken {
    return new FinalToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FinalToken {
    return new FinalToken($this->leading(), $trailing);
  }
}
final class FinallyToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('finally', $leading, $trailing, 'finally');
  }

  public function with_leading(EditableSyntax $leading): FinallyToken {
    return new FinallyToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FinallyToken {
    return new FinallyToken($this->leading(), $trailing);
  }
}
final class ForToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('for', $leading, $trailing, 'for');
  }

  public function with_leading(EditableSyntax $leading): ForToken {
    return new ForToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ForToken {
    return new ForToken($this->leading(), $trailing);
  }
}
final class ForeachToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('foreach', $leading, $trailing, 'foreach');
  }

  public function with_leading(EditableSyntax $leading): ForeachToken {
    return new ForeachToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ForeachToken {
    return new ForeachToken($this->leading(), $trailing);
  }
}
final class FromToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('from', $leading, $trailing, 'from');
  }

  public function with_leading(EditableSyntax $leading): FromToken {
    return new FromToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FromToken {
    return new FromToken($this->leading(), $trailing);
  }
}
final class FunctionToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('function', $leading, $trailing, 'function');
  }

  public function with_leading(EditableSyntax $leading): FunctionToken {
    return new FunctionToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): FunctionToken {
    return new FunctionToken($this->leading(), $trailing);
  }
}
final class GlobalToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('global', $leading, $trailing, 'global');
  }

  public function with_leading(EditableSyntax $leading): GlobalToken {
    return new GlobalToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GlobalToken {
    return new GlobalToken($this->leading(), $trailing);
  }
}
final class GotoToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('goto', $leading, $trailing, 'goto');
  }

  public function with_leading(EditableSyntax $leading): GotoToken {
    return new GotoToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GotoToken {
    return new GotoToken($this->leading(), $trailing);
  }
}
final class IfToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('if', $leading, $trailing, 'if');
  }

  public function with_leading(EditableSyntax $leading): IfToken {
    return new IfToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): IfToken {
    return new IfToken($this->leading(), $trailing);
  }
}
final class ImplementsToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('implements', $leading, $trailing, 'implements');
  }

  public function with_leading(EditableSyntax $leading): ImplementsToken {
    return new ImplementsToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ImplementsToken {
    return new ImplementsToken($this->leading(), $trailing);
  }
}
final class IncludeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('include', $leading, $trailing, 'include');
  }

  public function with_leading(EditableSyntax $leading): IncludeToken {
    return new IncludeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): IncludeToken {
    return new IncludeToken($this->leading(), $trailing);
  }
}
final class Include_onceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('include_once', $leading, $trailing, 'include_once');
  }

  public function with_leading(EditableSyntax $leading): Include_onceToken {
    return new Include_onceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): Include_onceToken {
    return new Include_onceToken($this->leading(), $trailing);
  }
}
final class InstanceofToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('instanceof', $leading, $trailing, 'instanceof');
  }

  public function with_leading(EditableSyntax $leading): InstanceofToken {
    return new InstanceofToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): InstanceofToken {
    return new InstanceofToken($this->leading(), $trailing);
  }
}
final class InsteadofToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('insteadof', $leading, $trailing, 'insteadof');
  }

  public function with_leading(EditableSyntax $leading): InsteadofToken {
    return new InsteadofToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): InsteadofToken {
    return new InsteadofToken($this->leading(), $trailing);
  }
}
final class IntToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('int', $leading, $trailing, 'int');
  }

  public function with_leading(EditableSyntax $leading): IntToken {
    return new IntToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): IntToken {
    return new IntToken($this->leading(), $trailing);
  }
}
final class InterfaceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('interface', $leading, $trailing, 'interface');
  }

  public function with_leading(EditableSyntax $leading): InterfaceToken {
    return new InterfaceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): InterfaceToken {
    return new InterfaceToken($this->leading(), $trailing);
  }
}
final class IssetToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('isset', $leading, $trailing, 'isset');
  }

  public function with_leading(EditableSyntax $leading): IssetToken {
    return new IssetToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): IssetToken {
    return new IssetToken($this->leading(), $trailing);
  }
}
final class KeysetToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('keyset', $leading, $trailing, 'keyset');
  }

  public function with_leading(EditableSyntax $leading): KeysetToken {
    return new KeysetToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): KeysetToken {
    return new KeysetToken($this->leading(), $trailing);
  }
}
final class ListToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('list', $leading, $trailing, 'list');
  }

  public function with_leading(EditableSyntax $leading): ListToken {
    return new ListToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ListToken {
    return new ListToken($this->leading(), $trailing);
  }
}
final class MixedToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('mixed', $leading, $trailing, 'mixed');
  }

  public function with_leading(EditableSyntax $leading): MixedToken {
    return new MixedToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): MixedToken {
    return new MixedToken($this->leading(), $trailing);
  }
}
final class NamespaceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('namespace', $leading, $trailing, 'namespace');
  }

  public function with_leading(EditableSyntax $leading): NamespaceToken {
    return new NamespaceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NamespaceToken {
    return new NamespaceToken($this->leading(), $trailing);
  }
}
final class NewToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('new', $leading, $trailing, 'new');
  }

  public function with_leading(EditableSyntax $leading): NewToken {
    return new NewToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NewToken {
    return new NewToken($this->leading(), $trailing);
  }
}
final class NewtypeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('newtype', $leading, $trailing, 'newtype');
  }

  public function with_leading(EditableSyntax $leading): NewtypeToken {
    return new NewtypeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NewtypeToken {
    return new NewtypeToken($this->leading(), $trailing);
  }
}
final class NoreturnToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('noreturn', $leading, $trailing, 'noreturn');
  }

  public function with_leading(EditableSyntax $leading): NoreturnToken {
    return new NoreturnToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NoreturnToken {
    return new NoreturnToken($this->leading(), $trailing);
  }
}
final class NumToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('num', $leading, $trailing, 'num');
  }

  public function with_leading(EditableSyntax $leading): NumToken {
    return new NumToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NumToken {
    return new NumToken($this->leading(), $trailing);
  }
}
final class ObjectToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('object', $leading, $trailing, 'object');
  }

  public function with_leading(EditableSyntax $leading): ObjectToken {
    return new ObjectToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ObjectToken {
    return new ObjectToken($this->leading(), $trailing);
  }
}
final class OrToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('or', $leading, $trailing, 'or');
  }

  public function with_leading(EditableSyntax $leading): OrToken {
    return new OrToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): OrToken {
    return new OrToken($this->leading(), $trailing);
  }
}
final class ParentToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('parent', $leading, $trailing, 'parent');
  }

  public function with_leading(EditableSyntax $leading): ParentToken {
    return new ParentToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ParentToken {
    return new ParentToken($this->leading(), $trailing);
  }
}
final class PrintToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('print', $leading, $trailing, 'print');
  }

  public function with_leading(EditableSyntax $leading): PrintToken {
    return new PrintToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PrintToken {
    return new PrintToken($this->leading(), $trailing);
  }
}
final class PrivateToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('private', $leading, $trailing, 'private');
  }

  public function with_leading(EditableSyntax $leading): PrivateToken {
    return new PrivateToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PrivateToken {
    return new PrivateToken($this->leading(), $trailing);
  }
}
final class ProtectedToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('protected', $leading, $trailing, 'protected');
  }

  public function with_leading(EditableSyntax $leading): ProtectedToken {
    return new ProtectedToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ProtectedToken {
    return new ProtectedToken($this->leading(), $trailing);
  }
}
final class PublicToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('public', $leading, $trailing, 'public');
  }

  public function with_leading(EditableSyntax $leading): PublicToken {
    return new PublicToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PublicToken {
    return new PublicToken($this->leading(), $trailing);
  }
}
final class RequireToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('require', $leading, $trailing, 'require');
  }

  public function with_leading(EditableSyntax $leading): RequireToken {
    return new RequireToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): RequireToken {
    return new RequireToken($this->leading(), $trailing);
  }
}
final class Require_onceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('require_once', $leading, $trailing, 'require_once');
  }

  public function with_leading(EditableSyntax $leading): Require_onceToken {
    return new Require_onceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): Require_onceToken {
    return new Require_onceToken($this->leading(), $trailing);
  }
}
final class RequiredToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('required', $leading, $trailing, 'required');
  }

  public function with_leading(EditableSyntax $leading): RequiredToken {
    return new RequiredToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): RequiredToken {
    return new RequiredToken($this->leading(), $trailing);
  }
}
final class ResourceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('resource', $leading, $trailing, 'resource');
  }

  public function with_leading(EditableSyntax $leading): ResourceToken {
    return new ResourceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ResourceToken {
    return new ResourceToken($this->leading(), $trailing);
  }
}
final class ReturnToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('return', $leading, $trailing, 'return');
  }

  public function with_leading(EditableSyntax $leading): ReturnToken {
    return new ReturnToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ReturnToken {
    return new ReturnToken($this->leading(), $trailing);
  }
}
final class SelfToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('self', $leading, $trailing, 'self');
  }

  public function with_leading(EditableSyntax $leading): SelfToken {
    return new SelfToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SelfToken {
    return new SelfToken($this->leading(), $trailing);
  }
}
final class ShapeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('shape', $leading, $trailing, 'shape');
  }

  public function with_leading(EditableSyntax $leading): ShapeToken {
    return new ShapeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ShapeToken {
    return new ShapeToken($this->leading(), $trailing);
  }
}
final class StaticToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('static', $leading, $trailing, 'static');
  }

  public function with_leading(EditableSyntax $leading): StaticToken {
    return new StaticToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StaticToken {
    return new StaticToken($this->leading(), $trailing);
  }
}
final class StringToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('string', $leading, $trailing, 'string');
  }

  public function with_leading(EditableSyntax $leading): StringToken {
    return new StringToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StringToken {
    return new StringToken($this->leading(), $trailing);
  }
}
final class SuperToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('super', $leading, $trailing, 'super');
  }

  public function with_leading(EditableSyntax $leading): SuperToken {
    return new SuperToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SuperToken {
    return new SuperToken($this->leading(), $trailing);
  }
}
final class SuspendToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('suspend', $leading, $trailing, 'suspend');
  }

  public function with_leading(EditableSyntax $leading): SuspendToken {
    return new SuspendToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SuspendToken {
    return new SuspendToken($this->leading(), $trailing);
  }
}
final class SwitchToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('switch', $leading, $trailing, 'switch');
  }

  public function with_leading(EditableSyntax $leading): SwitchToken {
    return new SwitchToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SwitchToken {
    return new SwitchToken($this->leading(), $trailing);
  }
}
final class ThisToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('this', $leading, $trailing, 'this');
  }

  public function with_leading(EditableSyntax $leading): ThisToken {
    return new ThisToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ThisToken {
    return new ThisToken($this->leading(), $trailing);
  }
}
final class ThrowToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('throw', $leading, $trailing, 'throw');
  }

  public function with_leading(EditableSyntax $leading): ThrowToken {
    return new ThrowToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ThrowToken {
    return new ThrowToken($this->leading(), $trailing);
  }
}
final class TraitToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('trait', $leading, $trailing, 'trait');
  }

  public function with_leading(EditableSyntax $leading): TraitToken {
    return new TraitToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): TraitToken {
    return new TraitToken($this->leading(), $trailing);
  }
}
final class TryToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('try', $leading, $trailing, 'try');
  }

  public function with_leading(EditableSyntax $leading): TryToken {
    return new TryToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): TryToken {
    return new TryToken($this->leading(), $trailing);
  }
}
final class TupleToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('tuple', $leading, $trailing, 'tuple');
  }

  public function with_leading(EditableSyntax $leading): TupleToken {
    return new TupleToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): TupleToken {
    return new TupleToken($this->leading(), $trailing);
  }
}
final class TypeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('type', $leading, $trailing, 'type');
  }

  public function with_leading(EditableSyntax $leading): TypeToken {
    return new TypeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): TypeToken {
    return new TypeToken($this->leading(), $trailing);
  }
}
final class UnsetToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('unset', $leading, $trailing, 'unset');
  }

  public function with_leading(EditableSyntax $leading): UnsetToken {
    return new UnsetToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): UnsetToken {
    return new UnsetToken($this->leading(), $trailing);
  }
}
final class UseToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('use', $leading, $trailing, 'use');
  }

  public function with_leading(EditableSyntax $leading): UseToken {
    return new UseToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): UseToken {
    return new UseToken($this->leading(), $trailing);
  }
}
final class VarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('var', $leading, $trailing, 'var');
  }

  public function with_leading(EditableSyntax $leading): VarToken {
    return new VarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): VarToken {
    return new VarToken($this->leading(), $trailing);
  }
}
final class VarrayToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('varray', $leading, $trailing, 'varray');
  }

  public function with_leading(EditableSyntax $leading): VarrayToken {
    return new VarrayToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): VarrayToken {
    return new VarrayToken($this->leading(), $trailing);
  }
}
final class VecToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('vec', $leading, $trailing, 'vec');
  }

  public function with_leading(EditableSyntax $leading): VecToken {
    return new VecToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): VecToken {
    return new VecToken($this->leading(), $trailing);
  }
}
final class VoidToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('void', $leading, $trailing, 'void');
  }

  public function with_leading(EditableSyntax $leading): VoidToken {
    return new VoidToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): VoidToken {
    return new VoidToken($this->leading(), $trailing);
  }
}
final class WhereToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('where', $leading, $trailing, 'where');
  }

  public function with_leading(EditableSyntax $leading): WhereToken {
    return new WhereToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): WhereToken {
    return new WhereToken($this->leading(), $trailing);
  }
}
final class WhileToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('while', $leading, $trailing, 'while');
  }

  public function with_leading(EditableSyntax $leading): WhileToken {
    return new WhileToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): WhileToken {
    return new WhileToken($this->leading(), $trailing);
  }
}
final class XorToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('xor', $leading, $trailing, 'xor');
  }

  public function with_leading(EditableSyntax $leading): XorToken {
    return new XorToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): XorToken {
    return new XorToken($this->leading(), $trailing);
  }
}
final class YieldToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('yield', $leading, $trailing, 'yield');
  }

  public function with_leading(EditableSyntax $leading): YieldToken {
    return new YieldToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): YieldToken {
    return new YieldToken($this->leading(), $trailing);
  }
}
final class LeftBracketToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('[', $leading, $trailing, '[');
  }

  public function with_leading(EditableSyntax $leading): LeftBracketToken {
    return new LeftBracketToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LeftBracketToken {
    return new LeftBracketToken($this->leading(), $trailing);
  }
}
final class RightBracketToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct(']', $leading, $trailing, ']');
  }

  public function with_leading(EditableSyntax $leading): RightBracketToken {
    return new RightBracketToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): RightBracketToken {
    return new RightBracketToken($this->leading(), $trailing);
  }
}
final class LeftParenToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('(', $leading, $trailing, '(');
  }

  public function with_leading(EditableSyntax $leading): LeftParenToken {
    return new LeftParenToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LeftParenToken {
    return new LeftParenToken($this->leading(), $trailing);
  }
}
final class RightParenToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct(')', $leading, $trailing, ')');
  }

  public function with_leading(EditableSyntax $leading): RightParenToken {
    return new RightParenToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): RightParenToken {
    return new RightParenToken($this->leading(), $trailing);
  }
}
final class LeftBraceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('{', $leading, $trailing, '{');
  }

  public function with_leading(EditableSyntax $leading): LeftBraceToken {
    return new LeftBraceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LeftBraceToken {
    return new LeftBraceToken($this->leading(), $trailing);
  }
}
final class RightBraceToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('}', $leading, $trailing, '}');
  }

  public function with_leading(EditableSyntax $leading): RightBraceToken {
    return new RightBraceToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): RightBraceToken {
    return new RightBraceToken($this->leading(), $trailing);
  }
}
final class DotToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('.', $leading, $trailing, '.');
  }

  public function with_leading(EditableSyntax $leading): DotToken {
    return new DotToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DotToken {
    return new DotToken($this->leading(), $trailing);
  }
}
final class MinusGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('->', $leading, $trailing, '->');
  }

  public function with_leading(EditableSyntax $leading): MinusGreaterThanToken {
    return new MinusGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): MinusGreaterThanToken {
    return new MinusGreaterThanToken($this->leading(), $trailing);
  }
}
final class PlusPlusToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('++', $leading, $trailing, '++');
  }

  public function with_leading(EditableSyntax $leading): PlusPlusToken {
    return new PlusPlusToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PlusPlusToken {
    return new PlusPlusToken($this->leading(), $trailing);
  }
}
final class MinusMinusToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('--', $leading, $trailing, '--');
  }

  public function with_leading(EditableSyntax $leading): MinusMinusToken {
    return new MinusMinusToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): MinusMinusToken {
    return new MinusMinusToken($this->leading(), $trailing);
  }
}
final class StarStarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('**', $leading, $trailing, '**');
  }

  public function with_leading(EditableSyntax $leading): StarStarToken {
    return new StarStarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StarStarToken {
    return new StarStarToken($this->leading(), $trailing);
  }
}
final class StarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('*', $leading, $trailing, '*');
  }

  public function with_leading(EditableSyntax $leading): StarToken {
    return new StarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StarToken {
    return new StarToken($this->leading(), $trailing);
  }
}
final class PlusToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('+', $leading, $trailing, '+');
  }

  public function with_leading(EditableSyntax $leading): PlusToken {
    return new PlusToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PlusToken {
    return new PlusToken($this->leading(), $trailing);
  }
}
final class MinusToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('-', $leading, $trailing, '-');
  }

  public function with_leading(EditableSyntax $leading): MinusToken {
    return new MinusToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): MinusToken {
    return new MinusToken($this->leading(), $trailing);
  }
}
final class TildeToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('~', $leading, $trailing, '~');
  }

  public function with_leading(EditableSyntax $leading): TildeToken {
    return new TildeToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): TildeToken {
    return new TildeToken($this->leading(), $trailing);
  }
}
final class ExclamationToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('!', $leading, $trailing, '!');
  }

  public function with_leading(EditableSyntax $leading): ExclamationToken {
    return new ExclamationToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ExclamationToken {
    return new ExclamationToken($this->leading(), $trailing);
  }
}
final class DollarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('$', $leading, $trailing, '$');
  }

  public function with_leading(EditableSyntax $leading): DollarToken {
    return new DollarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DollarToken {
    return new DollarToken($this->leading(), $trailing);
  }
}
final class SlashToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('/', $leading, $trailing, '/');
  }

  public function with_leading(EditableSyntax $leading): SlashToken {
    return new SlashToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SlashToken {
    return new SlashToken($this->leading(), $trailing);
  }
}
final class PercentToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%', $leading, $trailing, '%');
  }

  public function with_leading(EditableSyntax $leading): PercentToken {
    return new PercentToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PercentToken {
    return new PercentToken($this->leading(), $trailing);
  }
}
final class LessThanGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<>', $leading, $trailing, '<>');
  }

  public function with_leading(EditableSyntax $leading): LessThanGreaterThanToken {
    return new LessThanGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanGreaterThanToken {
    return new LessThanGreaterThanToken($this->leading(), $trailing);
  }
}
final class LessThanEqualGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<=>', $leading, $trailing, '<=>');
  }

  public function with_leading(EditableSyntax $leading): LessThanEqualGreaterThanToken {
    return new LessThanEqualGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanEqualGreaterThanToken {
    return new LessThanEqualGreaterThanToken($this->leading(), $trailing);
  }
}
final class LessThanLessThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<<', $leading, $trailing, '<<');
  }

  public function with_leading(EditableSyntax $leading): LessThanLessThanToken {
    return new LessThanLessThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanLessThanToken {
    return new LessThanLessThanToken($this->leading(), $trailing);
  }
}
final class GreaterThanGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('>>', $leading, $trailing, '>>');
  }

  public function with_leading(EditableSyntax $leading): GreaterThanGreaterThanToken {
    return new GreaterThanGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GreaterThanGreaterThanToken {
    return new GreaterThanGreaterThanToken($this->leading(), $trailing);
  }
}
final class LessThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<', $leading, $trailing, '<');
  }

  public function with_leading(EditableSyntax $leading): LessThanToken {
    return new LessThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanToken {
    return new LessThanToken($this->leading(), $trailing);
  }
}
final class GreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('>', $leading, $trailing, '>');
  }

  public function with_leading(EditableSyntax $leading): GreaterThanToken {
    return new GreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GreaterThanToken {
    return new GreaterThanToken($this->leading(), $trailing);
  }
}
final class LessThanEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<=', $leading, $trailing, '<=');
  }

  public function with_leading(EditableSyntax $leading): LessThanEqualToken {
    return new LessThanEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanEqualToken {
    return new LessThanEqualToken($this->leading(), $trailing);
  }
}
final class GreaterThanEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('>=', $leading, $trailing, '>=');
  }

  public function with_leading(EditableSyntax $leading): GreaterThanEqualToken {
    return new GreaterThanEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GreaterThanEqualToken {
    return new GreaterThanEqualToken($this->leading(), $trailing);
  }
}
final class EqualEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('==', $leading, $trailing, '==');
  }

  public function with_leading(EditableSyntax $leading): EqualEqualToken {
    return new EqualEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EqualEqualToken {
    return new EqualEqualToken($this->leading(), $trailing);
  }
}
final class EqualEqualEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('===', $leading, $trailing, '===');
  }

  public function with_leading(EditableSyntax $leading): EqualEqualEqualToken {
    return new EqualEqualEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EqualEqualEqualToken {
    return new EqualEqualEqualToken($this->leading(), $trailing);
  }
}
final class ExclamationEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('!=', $leading, $trailing, '!=');
  }

  public function with_leading(EditableSyntax $leading): ExclamationEqualToken {
    return new ExclamationEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ExclamationEqualToken {
    return new ExclamationEqualToken($this->leading(), $trailing);
  }
}
final class ExclamationEqualEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('!==', $leading, $trailing, '!==');
  }

  public function with_leading(EditableSyntax $leading): ExclamationEqualEqualToken {
    return new ExclamationEqualEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ExclamationEqualEqualToken {
    return new ExclamationEqualEqualToken($this->leading(), $trailing);
  }
}
final class CaratToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('^', $leading, $trailing, '^');
  }

  public function with_leading(EditableSyntax $leading): CaratToken {
    return new CaratToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CaratToken {
    return new CaratToken($this->leading(), $trailing);
  }
}
final class BarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('|', $leading, $trailing, '|');
  }

  public function with_leading(EditableSyntax $leading): BarToken {
    return new BarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BarToken {
    return new BarToken($this->leading(), $trailing);
  }
}
final class AmpersandToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('&', $leading, $trailing, '&');
  }

  public function with_leading(EditableSyntax $leading): AmpersandToken {
    return new AmpersandToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AmpersandToken {
    return new AmpersandToken($this->leading(), $trailing);
  }
}
final class AmpersandAmpersandToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('&&', $leading, $trailing, '&&');
  }

  public function with_leading(EditableSyntax $leading): AmpersandAmpersandToken {
    return new AmpersandAmpersandToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AmpersandAmpersandToken {
    return new AmpersandAmpersandToken($this->leading(), $trailing);
  }
}
final class BarBarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('||', $leading, $trailing, '||');
  }

  public function with_leading(EditableSyntax $leading): BarBarToken {
    return new BarBarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BarBarToken {
    return new BarBarToken($this->leading(), $trailing);
  }
}
final class QuestionToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('?', $leading, $trailing, '?');
  }

  public function with_leading(EditableSyntax $leading): QuestionToken {
    return new QuestionToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): QuestionToken {
    return new QuestionToken($this->leading(), $trailing);
  }
}
final class QuestionQuestionToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('??', $leading, $trailing, '??');
  }

  public function with_leading(EditableSyntax $leading): QuestionQuestionToken {
    return new QuestionQuestionToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): QuestionQuestionToken {
    return new QuestionQuestionToken($this->leading(), $trailing);
  }
}
final class ColonToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct(':', $leading, $trailing, ':');
  }

  public function with_leading(EditableSyntax $leading): ColonToken {
    return new ColonToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ColonToken {
    return new ColonToken($this->leading(), $trailing);
  }
}
final class SemicolonToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct(';', $leading, $trailing, ';');
  }

  public function with_leading(EditableSyntax $leading): SemicolonToken {
    return new SemicolonToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SemicolonToken {
    return new SemicolonToken($this->leading(), $trailing);
  }
}
final class EqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('=', $leading, $trailing, '=');
  }

  public function with_leading(EditableSyntax $leading): EqualToken {
    return new EqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EqualToken {
    return new EqualToken($this->leading(), $trailing);
  }
}
final class StarStarEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('**=', $leading, $trailing, '**=');
  }

  public function with_leading(EditableSyntax $leading): StarStarEqualToken {
    return new StarStarEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StarStarEqualToken {
    return new StarStarEqualToken($this->leading(), $trailing);
  }
}
final class StarEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('*=', $leading, $trailing, '*=');
  }

  public function with_leading(EditableSyntax $leading): StarEqualToken {
    return new StarEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): StarEqualToken {
    return new StarEqualToken($this->leading(), $trailing);
  }
}
final class SlashEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('/=', $leading, $trailing, '/=');
  }

  public function with_leading(EditableSyntax $leading): SlashEqualToken {
    return new SlashEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SlashEqualToken {
    return new SlashEqualToken($this->leading(), $trailing);
  }
}
final class PercentEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%=', $leading, $trailing, '%=');
  }

  public function with_leading(EditableSyntax $leading): PercentEqualToken {
    return new PercentEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PercentEqualToken {
    return new PercentEqualToken($this->leading(), $trailing);
  }
}
final class PlusEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('+=', $leading, $trailing, '+=');
  }

  public function with_leading(EditableSyntax $leading): PlusEqualToken {
    return new PlusEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): PlusEqualToken {
    return new PlusEqualToken($this->leading(), $trailing);
  }
}
final class MinusEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('-=', $leading, $trailing, '-=');
  }

  public function with_leading(EditableSyntax $leading): MinusEqualToken {
    return new MinusEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): MinusEqualToken {
    return new MinusEqualToken($this->leading(), $trailing);
  }
}
final class DotEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('.=', $leading, $trailing, '.=');
  }

  public function with_leading(EditableSyntax $leading): DotEqualToken {
    return new DotEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DotEqualToken {
    return new DotEqualToken($this->leading(), $trailing);
  }
}
final class LessThanLessThanEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<<=', $leading, $trailing, '<<=');
  }

  public function with_leading(EditableSyntax $leading): LessThanLessThanEqualToken {
    return new LessThanLessThanEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanLessThanEqualToken {
    return new LessThanLessThanEqualToken($this->leading(), $trailing);
  }
}
final class GreaterThanGreaterThanEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('>>=', $leading, $trailing, '>>=');
  }

  public function with_leading(EditableSyntax $leading): GreaterThanGreaterThanEqualToken {
    return new GreaterThanGreaterThanEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): GreaterThanGreaterThanEqualToken {
    return new GreaterThanGreaterThanEqualToken($this->leading(), $trailing);
  }
}
final class AmpersandEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('&=', $leading, $trailing, '&=');
  }

  public function with_leading(EditableSyntax $leading): AmpersandEqualToken {
    return new AmpersandEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AmpersandEqualToken {
    return new AmpersandEqualToken($this->leading(), $trailing);
  }
}
final class CaratEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('^=', $leading, $trailing, '^=');
  }

  public function with_leading(EditableSyntax $leading): CaratEqualToken {
    return new CaratEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CaratEqualToken {
    return new CaratEqualToken($this->leading(), $trailing);
  }
}
final class BarEqualToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('|=', $leading, $trailing, '|=');
  }

  public function with_leading(EditableSyntax $leading): BarEqualToken {
    return new BarEqualToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BarEqualToken {
    return new BarEqualToken($this->leading(), $trailing);
  }
}
final class CommaToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct(',', $leading, $trailing, ',');
  }

  public function with_leading(EditableSyntax $leading): CommaToken {
    return new CommaToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): CommaToken {
    return new CommaToken($this->leading(), $trailing);
  }
}
final class AtToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('@', $leading, $trailing, '@');
  }

  public function with_leading(EditableSyntax $leading): AtToken {
    return new AtToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): AtToken {
    return new AtToken($this->leading(), $trailing);
  }
}
final class ColonColonToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('::', $leading, $trailing, '::');
  }

  public function with_leading(EditableSyntax $leading): ColonColonToken {
    return new ColonColonToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): ColonColonToken {
    return new ColonColonToken($this->leading(), $trailing);
  }
}
final class EqualGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('=>', $leading, $trailing, '=>');
  }

  public function with_leading(EditableSyntax $leading): EqualGreaterThanToken {
    return new EqualGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EqualGreaterThanToken {
    return new EqualGreaterThanToken($this->leading(), $trailing);
  }
}
final class EqualEqualGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('==>', $leading, $trailing, '==>');
  }

  public function with_leading(EditableSyntax $leading): EqualEqualGreaterThanToken {
    return new EqualEqualGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): EqualEqualGreaterThanToken {
    return new EqualEqualGreaterThanToken($this->leading(), $trailing);
  }
}
final class QuestionMinusGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('?->', $leading, $trailing, '?->');
  }

  public function with_leading(EditableSyntax $leading): QuestionMinusGreaterThanToken {
    return new QuestionMinusGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): QuestionMinusGreaterThanToken {
    return new QuestionMinusGreaterThanToken($this->leading(), $trailing);
  }
}
final class DotDotDotToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('...', $leading, $trailing, '...');
  }

  public function with_leading(EditableSyntax $leading): DotDotDotToken {
    return new DotDotDotToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DotDotDotToken {
    return new DotDotDotToken($this->leading(), $trailing);
  }
}
final class DollarDollarToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('$$', $leading, $trailing, '$$');
  }

  public function with_leading(EditableSyntax $leading): DollarDollarToken {
    return new DollarDollarToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): DollarDollarToken {
    return new DollarDollarToken($this->leading(), $trailing);
  }
}
final class BarGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('|>', $leading, $trailing, '|>');
  }

  public function with_leading(EditableSyntax $leading): BarGreaterThanToken {
    return new BarGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): BarGreaterThanToken {
    return new BarGreaterThanToken($this->leading(), $trailing);
  }
}
final class NullLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('null', $leading, $trailing, 'null');
  }

  public function with_leading(EditableSyntax $leading): NullLiteralToken {
    return new NullLiteralToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): NullLiteralToken {
    return new NullLiteralToken($this->leading(), $trailing);
  }
}
final class SlashGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('/>', $leading, $trailing, '/>');
  }

  public function with_leading(EditableSyntax $leading): SlashGreaterThanToken {
    return new SlashGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): SlashGreaterThanToken {
    return new SlashGreaterThanToken($this->leading(), $trailing);
  }
}
final class LessThanSlashToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('</', $leading, $trailing, '</');
  }

  public function with_leading(EditableSyntax $leading): LessThanSlashToken {
    return new LessThanSlashToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanSlashToken {
    return new LessThanSlashToken($this->leading(), $trailing);
  }
}
final class LessThanQuestionToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('<?', $leading, $trailing, '<?');
  }

  public function with_leading(EditableSyntax $leading): LessThanQuestionToken {
    return new LessThanQuestionToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): LessThanQuestionToken {
    return new LessThanQuestionToken($this->leading(), $trailing);
  }
}
final class QuestionGreaterThanToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('?>', $leading, $trailing, '?>');
  }

  public function with_leading(EditableSyntax $leading): QuestionGreaterThanToken {
    return new QuestionGreaterThanToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): QuestionGreaterThanToken {
    return new QuestionGreaterThanToken($this->leading(), $trailing);
  }
}

final class ErrorTokenToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('error_token', $leading, $trailing, $text);
  }

  public function with_text(string $text): ErrorTokenToken {
    return new ErrorTokenToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): ErrorTokenToken {
    return new ErrorTokenToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): ErrorTokenToken {
    return new ErrorTokenToken($this->leading(), $trailing, $this->text());
  }
}
final class NameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('name', $leading, $trailing, $text);
  }

  public function with_text(string $text): NameToken {
    return new NameToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): NameToken {
    return new NameToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): NameToken {
    return new NameToken($this->leading(), $trailing, $this->text());
  }
}
final class QualifiedNameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('qualified_name', $leading, $trailing, $text);
  }

  public function with_text(string $text): QualifiedNameToken {
    return new QualifiedNameToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): QualifiedNameToken {
    return new QualifiedNameToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): QualifiedNameToken {
    return new QualifiedNameToken($this->leading(), $trailing, $this->text());
  }
}
final class VariableToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('variable', $leading, $trailing, $text);
  }

  public function with_text(string $text): VariableToken {
    return new VariableToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): VariableToken {
    return new VariableToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): VariableToken {
    return new VariableToken($this->leading(), $trailing, $this->text());
  }
}
final class NamespacePrefixToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('namespace_prefix', $leading, $trailing, $text);
  }

  public function with_text(string $text): NamespacePrefixToken {
    return new NamespacePrefixToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): NamespacePrefixToken {
    return new NamespacePrefixToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): NamespacePrefixToken {
    return new NamespacePrefixToken($this->leading(), $trailing, $this->text());
  }
}
final class DecimalLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('decimal_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): DecimalLiteralToken {
    return new DecimalLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): DecimalLiteralToken {
    return new DecimalLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): DecimalLiteralToken {
    return new DecimalLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class OctalLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('octal_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): OctalLiteralToken {
    return new OctalLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): OctalLiteralToken {
    return new OctalLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): OctalLiteralToken {
    return new OctalLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class HexadecimalLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('hexadecimal_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): HexadecimalLiteralToken {
    return new HexadecimalLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): HexadecimalLiteralToken {
    return new HexadecimalLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): HexadecimalLiteralToken {
    return new HexadecimalLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class BinaryLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('binary_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): BinaryLiteralToken {
    return new BinaryLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): BinaryLiteralToken {
    return new BinaryLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): BinaryLiteralToken {
    return new BinaryLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class FloatingLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('floating_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): FloatingLiteralToken {
    return new FloatingLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): FloatingLiteralToken {
    return new FloatingLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): FloatingLiteralToken {
    return new FloatingLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class ExecutionStringToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('execution_string', $leading, $trailing, $text);
  }

  public function with_text(string $text): ExecutionStringToken {
    return new ExecutionStringToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): ExecutionStringToken {
    return new ExecutionStringToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): ExecutionStringToken {
    return new ExecutionStringToken($this->leading(), $trailing, $this->text());
  }
}
final class SingleQuotedStringLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('single_quoted_string_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): SingleQuotedStringLiteralToken {
    return new SingleQuotedStringLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): SingleQuotedStringLiteralToken {
    return new SingleQuotedStringLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): SingleQuotedStringLiteralToken {
    return new SingleQuotedStringLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class DoubleQuotedStringLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('double_quoted_string_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): DoubleQuotedStringLiteralToken {
    return new DoubleQuotedStringLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): DoubleQuotedStringLiteralToken {
    return new DoubleQuotedStringLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): DoubleQuotedStringLiteralToken {
    return new DoubleQuotedStringLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class DoubleQuotedStringLiteralHeadToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('double_quoted_string_literal_head', $leading, $trailing, $text);
  }

  public function with_text(string $text): DoubleQuotedStringLiteralHeadToken {
    return new DoubleQuotedStringLiteralHeadToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): DoubleQuotedStringLiteralHeadToken {
    return new DoubleQuotedStringLiteralHeadToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): DoubleQuotedStringLiteralHeadToken {
    return new DoubleQuotedStringLiteralHeadToken($this->leading(), $trailing, $this->text());
  }
}
final class StringLiteralBodyToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('string_literal_body', $leading, $trailing, $text);
  }

  public function with_text(string $text): StringLiteralBodyToken {
    return new StringLiteralBodyToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): StringLiteralBodyToken {
    return new StringLiteralBodyToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): StringLiteralBodyToken {
    return new StringLiteralBodyToken($this->leading(), $trailing, $this->text());
  }
}
final class DoubleQuotedStringLiteralTailToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('double_quoted_string_literal_tail', $leading, $trailing, $text);
  }

  public function with_text(string $text): DoubleQuotedStringLiteralTailToken {
    return new DoubleQuotedStringLiteralTailToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): DoubleQuotedStringLiteralTailToken {
    return new DoubleQuotedStringLiteralTailToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): DoubleQuotedStringLiteralTailToken {
    return new DoubleQuotedStringLiteralTailToken($this->leading(), $trailing, $this->text());
  }
}
final class HeredocStringLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('heredoc_string_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): HeredocStringLiteralToken {
    return new HeredocStringLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): HeredocStringLiteralToken {
    return new HeredocStringLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): HeredocStringLiteralToken {
    return new HeredocStringLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class HeredocStringLiteralHeadToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('heredoc_string_literal_head', $leading, $trailing, $text);
  }

  public function with_text(string $text): HeredocStringLiteralHeadToken {
    return new HeredocStringLiteralHeadToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): HeredocStringLiteralHeadToken {
    return new HeredocStringLiteralHeadToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): HeredocStringLiteralHeadToken {
    return new HeredocStringLiteralHeadToken($this->leading(), $trailing, $this->text());
  }
}
final class HeredocStringLiteralTailToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('heredoc_string_literal_tail', $leading, $trailing, $text);
  }

  public function with_text(string $text): HeredocStringLiteralTailToken {
    return new HeredocStringLiteralTailToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): HeredocStringLiteralTailToken {
    return new HeredocStringLiteralTailToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): HeredocStringLiteralTailToken {
    return new HeredocStringLiteralTailToken($this->leading(), $trailing, $this->text());
  }
}
final class NowdocStringLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('nowdoc_string_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): NowdocStringLiteralToken {
    return new NowdocStringLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): NowdocStringLiteralToken {
    return new NowdocStringLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): NowdocStringLiteralToken {
    return new NowdocStringLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class BooleanLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('boolean_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): BooleanLiteralToken {
    return new BooleanLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): BooleanLiteralToken {
    return new BooleanLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): BooleanLiteralToken {
    return new BooleanLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPCategoryNameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_category_name', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPCategoryNameToken {
    return new XHPCategoryNameToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPCategoryNameToken {
    return new XHPCategoryNameToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPCategoryNameToken {
    return new XHPCategoryNameToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPElementNameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_element_name', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPElementNameToken {
    return new XHPElementNameToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPElementNameToken {
    return new XHPElementNameToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPElementNameToken {
    return new XHPElementNameToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPClassNameToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_class_name', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPClassNameToken {
    return new XHPClassNameToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPClassNameToken {
    return new XHPClassNameToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPClassNameToken {
    return new XHPClassNameToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPStringLiteralToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_string_literal', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPStringLiteralToken {
    return new XHPStringLiteralToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPStringLiteralToken {
    return new XHPStringLiteralToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPStringLiteralToken {
    return new XHPStringLiteralToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPBodyToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_body', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPBodyToken {
    return new XHPBodyToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPBodyToken {
    return new XHPBodyToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPBodyToken {
    return new XHPBodyToken($this->leading(), $trailing, $this->text());
  }
}
final class XHPCommentToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('XHP_comment', $leading, $trailing, $text);
  }

  public function with_text(string $text): XHPCommentToken {
    return new XHPCommentToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): XHPCommentToken {
    return new XHPCommentToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): XHPCommentToken {
    return new XHPCommentToken($this->leading(), $trailing, $this->text());
  }
}
final class MarkupToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('markup', $leading, $trailing, $text);
  }

  public function with_text(string $text): MarkupToken {
    return new MarkupToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): MarkupToken {
    return new MarkupToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): MarkupToken {
    return new MarkupToken($this->leading(), $trailing, $this->text());
  }
}


abstract class EditableTrivia extends EditableSyntax {
  private string $_text;
  public function __construct(string $trivia_kind , string $text) {
    parent::__construct($trivia_kind);
    $this->_text = $text;
  }

  public function text(): string {
    return $this->_text;
  }

  public function full_text() {
    return $this->_text;
  }

  public function width() {
    return strlen($this->_text);
  }

  public function is_trivia() {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    $trivia_text = substr($source, $position, $json->width);
    switch($json->kind) {
      case 'whitespace':
        return new WhiteSpace($trivia_text);
      case 'end_of_line':
        return new EndOfLine($trivia_text);
      case 'delimited_comment':
        return new DelimitedComment($trivia_text);
      case 'single_line_comment':
        return new SingleLineComment($trivia_text);
      case 'unsafe':
        return new Unsafe($trivia_text);
      case 'unsafe_expression':
        return new UnsafeExpression($trivia_text);
      case 'fix_me':
        return new FixMe($trivia_text);
      case 'ignore_error':
        return new IgnoreError($trivia_text);
      case 'fall_through':
        return new FallThrough($trivia_text);
      case 'extra_token_error':
        return new ExtraTokenError($trivia_text);

      default:
        throw new Exception('unexpected json kind: ' . $json->kind);
        // TODO: Better error
    }
  }

public function rewrite(
  ( function
    (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
  ?array<EditableSyntax> $parents = null): EditableSyntax {
    return $rewriter($this, $parents ?? []);
  }
}

class WhiteSpace extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('whitespace', $text);
  }
  public function with_text(string $text): WhiteSpace {
    return new WhiteSpace($text);
  }
}

class EndOfLine extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('end_of_line', $text);
  }
  public function with_text(string $text): EndOfLine {
    return new EndOfLine($text);
  }
}

class DelimitedComment extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('delimited_comment', $text);
  }
  public function with_text(string $text): DelimitedComment {
    return new DelimitedComment($text);
  }
}

class SingleLineComment extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('single_line_comment', $text);
  }
  public function with_text(string $text): SingleLineComment {
    return new SingleLineComment($text);
  }
}

class Unsafe extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('unsafe', $text);
  }
  public function with_text(string $text): Unsafe {
    return new Unsafe($text);
  }
}

class UnsafeExpression extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('unsafe_expression', $text);
  }
  public function with_text(string $text): UnsafeExpression {
    return new UnsafeExpression($text);
  }
}

class FixMe extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('fix_me', $text);
  }
  public function with_text(string $text): FixMe {
    return new FixMe($text);
  }
}

class IgnoreError extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('ignore_error', $text);
  }
  public function with_text(string $text): IgnoreError {
    return new IgnoreError($text);
  }
}

class FallThrough extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('fall_through', $text);
  }
  public function with_text(string $text): FallThrough {
    return new FallThrough($text);
  }
}

class ExtraTokenError extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('extra_token_error', $text);
  }
  public function with_text(string $text): ExtraTokenError {
    return new ExtraTokenError($text);
  }
}



final class Missing extends EditableSyntax {
  private static ?Missing $_missing = null;

  public function __construct() {
    parent::__construct('missing');
  }

  public function is_missing(): bool {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function missing(): Missing {
    if (Missing::$_missing === null) {
      $m = new Missing();
      Missing::$_missing = $m;
      return $m;
    } else {
      return Missing::$_missing;
    }
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    return Missing::missing();
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
      return $rewriter($this, $parents ?? []);
  }

  public function to_array(): array<EditableSyntax> {
    return [];
  }
}

final class EndOfFile extends EditableSyntax {
  private EditableSyntax $_token;
  public function __construct(
    EditableSyntax $token) {
    parent::__construct('end_of_file');
    $this->_token = $token;
  }
  public function token(): EditableSyntax {
    return $this->_token;
  }
  public function with_token(EditableSyntax $token): EndOfFile {
    return new EndOfFile(
      $token);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $token = $this->token()->rewrite($rewriter, $new_parents);
    if (
      $token === $this->token()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EndOfFile(
        $token), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $token = EditableSyntax::from_json(
      $json->end_of_file_token, $position, $source);
    $position += $token->width();
    return new EndOfFile(
        $token);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_token;
    yield break;
  }
}
final class Script extends EditableSyntax {
  private EditableSyntax $_declarations;
  public function __construct(
    EditableSyntax $declarations) {
    parent::__construct('script');
    $this->_declarations = $declarations;
  }
  public function declarations(): EditableSyntax {
    return $this->_declarations;
  }
  public function with_declarations(EditableSyntax $declarations): Script {
    return new Script(
      $declarations);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $declarations = $this->declarations()->rewrite($rewriter, $new_parents);
    if (
      $declarations === $this->declarations()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new Script(
        $declarations), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $declarations = EditableSyntax::from_json(
      $json->script_declarations, $position, $source);
    $position += $declarations->width();
    return new Script(
        $declarations);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_declarations;
    yield break;
  }
}
final class SimpleTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_specifier;
  public function __construct(
    EditableSyntax $specifier) {
    parent::__construct('simple_type_specifier');
    $this->_specifier = $specifier;
  }
  public function specifier(): EditableSyntax {
    return $this->_specifier;
  }
  public function with_specifier(EditableSyntax $specifier): SimpleTypeSpecifier {
    return new SimpleTypeSpecifier(
      $specifier);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $specifier = $this->specifier()->rewrite($rewriter, $new_parents);
    if (
      $specifier === $this->specifier()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SimpleTypeSpecifier(
        $specifier), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $specifier = EditableSyntax::from_json(
      $json->simple_type_specifier, $position, $source);
    $position += $specifier->width();
    return new SimpleTypeSpecifier(
        $specifier);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_specifier;
    yield break;
  }
}
final class LiteralExpression extends EditableSyntax {
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $expression) {
    parent::__construct('literal');
    $this->_expression = $expression;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_expression(EditableSyntax $expression): LiteralExpression {
    return new LiteralExpression(
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new LiteralExpression(
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->literal_expression, $position, $source);
    $position += $expression->width();
    return new LiteralExpression(
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield break;
  }
}
final class VariableExpression extends EditableSyntax {
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $expression) {
    parent::__construct('variable');
    $this->_expression = $expression;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_expression(EditableSyntax $expression): VariableExpression {
    return new VariableExpression(
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VariableExpression(
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->variable_expression, $position, $source);
    $position += $expression->width();
    return new VariableExpression(
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield break;
  }
}
final class QualifiedNameExpression extends EditableSyntax {
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $expression) {
    parent::__construct('qualified_name');
    $this->_expression = $expression;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_expression(EditableSyntax $expression): QualifiedNameExpression {
    return new QualifiedNameExpression(
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new QualifiedNameExpression(
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->qualified_name_expression, $position, $source);
    $position += $expression->width();
    return new QualifiedNameExpression(
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield break;
  }
}
final class PipeVariableExpression extends EditableSyntax {
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $expression) {
    parent::__construct('pipe_variable');
    $this->_expression = $expression;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_expression(EditableSyntax $expression): PipeVariableExpression {
    return new PipeVariableExpression(
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new PipeVariableExpression(
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->pipe_variable_expression, $position, $source);
    $position += $expression->width();
    return new PipeVariableExpression(
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield break;
  }
}
final class EnumDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute_spec;
  private EditableSyntax $_keyword;
  private EditableSyntax $_name;
  private EditableSyntax $_colon;
  private EditableSyntax $_base;
  private EditableSyntax $_type;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_enumerators;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $attribute_spec,
    EditableSyntax $keyword,
    EditableSyntax $name,
    EditableSyntax $colon,
    EditableSyntax $base,
    EditableSyntax $type,
    EditableSyntax $left_brace,
    EditableSyntax $enumerators,
    EditableSyntax $right_brace) {
    parent::__construct('enum_declaration');
    $this->_attribute_spec = $attribute_spec;
    $this->_keyword = $keyword;
    $this->_name = $name;
    $this->_colon = $colon;
    $this->_base = $base;
    $this->_type = $type;
    $this->_left_brace = $left_brace;
    $this->_enumerators = $enumerators;
    $this->_right_brace = $right_brace;
  }
  public function attribute_spec(): EditableSyntax {
    return $this->_attribute_spec;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function base(): EditableSyntax {
    return $this->_base;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function enumerators(): EditableSyntax {
    return $this->_enumerators;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_attribute_spec(EditableSyntax $attribute_spec): EnumDeclaration {
    return new EnumDeclaration(
      $attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_keyword(EditableSyntax $keyword): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_name(EditableSyntax $name): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_colon(EditableSyntax $colon): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_base(EditableSyntax $base): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_type(EditableSyntax $type): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $type,
      $this->_left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_left_brace(EditableSyntax $left_brace): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $left_brace,
      $this->_enumerators,
      $this->_right_brace);
  }
  public function with_enumerators(EditableSyntax $enumerators): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $enumerators,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): EnumDeclaration {
    return new EnumDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_colon,
      $this->_base,
      $this->_type,
      $this->_left_brace,
      $this->_enumerators,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute_spec = $this->attribute_spec()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $base = $this->base()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $enumerators = $this->enumerators()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $attribute_spec === $this->attribute_spec() &&
      $keyword === $this->keyword() &&
      $name === $this->name() &&
      $colon === $this->colon() &&
      $base === $this->base() &&
      $type === $this->type() &&
      $left_brace === $this->left_brace() &&
      $enumerators === $this->enumerators() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EnumDeclaration(
        $attribute_spec,
        $keyword,
        $name,
        $colon,
        $base,
        $type,
        $left_brace,
        $enumerators,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute_spec = EditableSyntax::from_json(
      $json->enum_attribute_spec, $position, $source);
    $position += $attribute_spec->width();
    $keyword = EditableSyntax::from_json(
      $json->enum_keyword, $position, $source);
    $position += $keyword->width();
    $name = EditableSyntax::from_json(
      $json->enum_name, $position, $source);
    $position += $name->width();
    $colon = EditableSyntax::from_json(
      $json->enum_colon, $position, $source);
    $position += $colon->width();
    $base = EditableSyntax::from_json(
      $json->enum_base, $position, $source);
    $position += $base->width();
    $type = EditableSyntax::from_json(
      $json->enum_type, $position, $source);
    $position += $type->width();
    $left_brace = EditableSyntax::from_json(
      $json->enum_left_brace, $position, $source);
    $position += $left_brace->width();
    $enumerators = EditableSyntax::from_json(
      $json->enum_enumerators, $position, $source);
    $position += $enumerators->width();
    $right_brace = EditableSyntax::from_json(
      $json->enum_right_brace, $position, $source);
    $position += $right_brace->width();
    return new EnumDeclaration(
        $attribute_spec,
        $keyword,
        $name,
        $colon,
        $base,
        $type,
        $left_brace,
        $enumerators,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute_spec;
    yield $this->_keyword;
    yield $this->_name;
    yield $this->_colon;
    yield $this->_base;
    yield $this->_type;
    yield $this->_left_brace;
    yield $this->_enumerators;
    yield $this->_right_brace;
    yield break;
  }
}
final class Enumerator extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_equal;
  private EditableSyntax $_value;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $equal,
    EditableSyntax $value,
    EditableSyntax $semicolon) {
    parent::__construct('enumerator');
    $this->_name = $name;
    $this->_equal = $equal;
    $this->_value = $value;
    $this->_semicolon = $semicolon;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function equal(): EditableSyntax {
    return $this->_equal;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_name(EditableSyntax $name): Enumerator {
    return new Enumerator(
      $name,
      $this->_equal,
      $this->_value,
      $this->_semicolon);
  }
  public function with_equal(EditableSyntax $equal): Enumerator {
    return new Enumerator(
      $this->_name,
      $equal,
      $this->_value,
      $this->_semicolon);
  }
  public function with_value(EditableSyntax $value): Enumerator {
    return new Enumerator(
      $this->_name,
      $this->_equal,
      $value,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): Enumerator {
    return new Enumerator(
      $this->_name,
      $this->_equal,
      $this->_value,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $equal = $this->equal()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $equal === $this->equal() &&
      $value === $this->value() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new Enumerator(
        $name,
        $equal,
        $value,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->enumerator_name, $position, $source);
    $position += $name->width();
    $equal = EditableSyntax::from_json(
      $json->enumerator_equal, $position, $source);
    $position += $equal->width();
    $value = EditableSyntax::from_json(
      $json->enumerator_value, $position, $source);
    $position += $value->width();
    $semicolon = EditableSyntax::from_json(
      $json->enumerator_semicolon, $position, $source);
    $position += $semicolon->width();
    return new Enumerator(
        $name,
        $equal,
        $value,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_equal;
    yield $this->_value;
    yield $this->_semicolon;
    yield break;
  }
}
final class AliasDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute_spec;
  private EditableSyntax $_keyword;
  private EditableSyntax $_name;
  private EditableSyntax $_generic_parameter;
  private EditableSyntax $_constraint;
  private EditableSyntax $_equal;
  private EditableSyntax $_type;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $attribute_spec,
    EditableSyntax $keyword,
    EditableSyntax $name,
    EditableSyntax $generic_parameter,
    EditableSyntax $constraint,
    EditableSyntax $equal,
    EditableSyntax $type,
    EditableSyntax $semicolon) {
    parent::__construct('alias_declaration');
    $this->_attribute_spec = $attribute_spec;
    $this->_keyword = $keyword;
    $this->_name = $name;
    $this->_generic_parameter = $generic_parameter;
    $this->_constraint = $constraint;
    $this->_equal = $equal;
    $this->_type = $type;
    $this->_semicolon = $semicolon;
  }
  public function attribute_spec(): EditableSyntax {
    return $this->_attribute_spec;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function generic_parameter(): EditableSyntax {
    return $this->_generic_parameter;
  }
  public function constraint(): EditableSyntax {
    return $this->_constraint;
  }
  public function equal(): EditableSyntax {
    return $this->_equal;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_attribute_spec(EditableSyntax $attribute_spec): AliasDeclaration {
    return new AliasDeclaration(
      $attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_generic_parameter,
      $this->_constraint,
      $this->_equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_keyword(EditableSyntax $keyword): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $keyword,
      $this->_name,
      $this->_generic_parameter,
      $this->_constraint,
      $this->_equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_name(EditableSyntax $name): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $name,
      $this->_generic_parameter,
      $this->_constraint,
      $this->_equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_generic_parameter(EditableSyntax $generic_parameter): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $generic_parameter,
      $this->_constraint,
      $this->_equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_constraint(EditableSyntax $constraint): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_generic_parameter,
      $constraint,
      $this->_equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_equal(EditableSyntax $equal): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_generic_parameter,
      $this->_constraint,
      $equal,
      $this->_type,
      $this->_semicolon);
  }
  public function with_type(EditableSyntax $type): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_generic_parameter,
      $this->_constraint,
      $this->_equal,
      $type,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): AliasDeclaration {
    return new AliasDeclaration(
      $this->_attribute_spec,
      $this->_keyword,
      $this->_name,
      $this->_generic_parameter,
      $this->_constraint,
      $this->_equal,
      $this->_type,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute_spec = $this->attribute_spec()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $generic_parameter = $this->generic_parameter()->rewrite($rewriter, $new_parents);
    $constraint = $this->constraint()->rewrite($rewriter, $new_parents);
    $equal = $this->equal()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $attribute_spec === $this->attribute_spec() &&
      $keyword === $this->keyword() &&
      $name === $this->name() &&
      $generic_parameter === $this->generic_parameter() &&
      $constraint === $this->constraint() &&
      $equal === $this->equal() &&
      $type === $this->type() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new AliasDeclaration(
        $attribute_spec,
        $keyword,
        $name,
        $generic_parameter,
        $constraint,
        $equal,
        $type,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute_spec = EditableSyntax::from_json(
      $json->alias_attribute_spec, $position, $source);
    $position += $attribute_spec->width();
    $keyword = EditableSyntax::from_json(
      $json->alias_keyword, $position, $source);
    $position += $keyword->width();
    $name = EditableSyntax::from_json(
      $json->alias_name, $position, $source);
    $position += $name->width();
    $generic_parameter = EditableSyntax::from_json(
      $json->alias_generic_parameter, $position, $source);
    $position += $generic_parameter->width();
    $constraint = EditableSyntax::from_json(
      $json->alias_constraint, $position, $source);
    $position += $constraint->width();
    $equal = EditableSyntax::from_json(
      $json->alias_equal, $position, $source);
    $position += $equal->width();
    $type = EditableSyntax::from_json(
      $json->alias_type, $position, $source);
    $position += $type->width();
    $semicolon = EditableSyntax::from_json(
      $json->alias_semicolon, $position, $source);
    $position += $semicolon->width();
    return new AliasDeclaration(
        $attribute_spec,
        $keyword,
        $name,
        $generic_parameter,
        $constraint,
        $equal,
        $type,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute_spec;
    yield $this->_keyword;
    yield $this->_name;
    yield $this->_generic_parameter;
    yield $this->_constraint;
    yield $this->_equal;
    yield $this->_type;
    yield $this->_semicolon;
    yield break;
  }
}
final class PropertyDeclaration extends EditableSyntax {
  private EditableSyntax $_modifiers;
  private EditableSyntax $_type;
  private EditableSyntax $_declarators;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $modifiers,
    EditableSyntax $type,
    EditableSyntax $declarators,
    EditableSyntax $semicolon) {
    parent::__construct('property_declaration');
    $this->_modifiers = $modifiers;
    $this->_type = $type;
    $this->_declarators = $declarators;
    $this->_semicolon = $semicolon;
  }
  public function modifiers(): EditableSyntax {
    return $this->_modifiers;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function declarators(): EditableSyntax {
    return $this->_declarators;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_modifiers(EditableSyntax $modifiers): PropertyDeclaration {
    return new PropertyDeclaration(
      $modifiers,
      $this->_type,
      $this->_declarators,
      $this->_semicolon);
  }
  public function with_type(EditableSyntax $type): PropertyDeclaration {
    return new PropertyDeclaration(
      $this->_modifiers,
      $type,
      $this->_declarators,
      $this->_semicolon);
  }
  public function with_declarators(EditableSyntax $declarators): PropertyDeclaration {
    return new PropertyDeclaration(
      $this->_modifiers,
      $this->_type,
      $declarators,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): PropertyDeclaration {
    return new PropertyDeclaration(
      $this->_modifiers,
      $this->_type,
      $this->_declarators,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $modifiers = $this->modifiers()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $declarators = $this->declarators()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $modifiers === $this->modifiers() &&
      $type === $this->type() &&
      $declarators === $this->declarators() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new PropertyDeclaration(
        $modifiers,
        $type,
        $declarators,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $modifiers = EditableSyntax::from_json(
      $json->property_modifiers, $position, $source);
    $position += $modifiers->width();
    $type = EditableSyntax::from_json(
      $json->property_type, $position, $source);
    $position += $type->width();
    $declarators = EditableSyntax::from_json(
      $json->property_declarators, $position, $source);
    $position += $declarators->width();
    $semicolon = EditableSyntax::from_json(
      $json->property_semicolon, $position, $source);
    $position += $semicolon->width();
    return new PropertyDeclaration(
        $modifiers,
        $type,
        $declarators,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_modifiers;
    yield $this->_type;
    yield $this->_declarators;
    yield $this->_semicolon;
    yield break;
  }
}
final class PropertyDeclarator extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_initializer;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $initializer) {
    parent::__construct('property_declarator');
    $this->_name = $name;
    $this->_initializer = $initializer;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function initializer(): EditableSyntax {
    return $this->_initializer;
  }
  public function with_name(EditableSyntax $name): PropertyDeclarator {
    return new PropertyDeclarator(
      $name,
      $this->_initializer);
  }
  public function with_initializer(EditableSyntax $initializer): PropertyDeclarator {
    return new PropertyDeclarator(
      $this->_name,
      $initializer);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $initializer = $this->initializer()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $initializer === $this->initializer()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new PropertyDeclarator(
        $name,
        $initializer), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->property_name, $position, $source);
    $position += $name->width();
    $initializer = EditableSyntax::from_json(
      $json->property_initializer, $position, $source);
    $position += $initializer->width();
    return new PropertyDeclarator(
        $name,
        $initializer);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_initializer;
    yield break;
  }
}
final class NamespaceDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_name;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $name,
    EditableSyntax $body) {
    parent::__construct('namespace_declaration');
    $this->_keyword = $keyword;
    $this->_name = $name;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): NamespaceDeclaration {
    return new NamespaceDeclaration(
      $keyword,
      $this->_name,
      $this->_body);
  }
  public function with_name(EditableSyntax $name): NamespaceDeclaration {
    return new NamespaceDeclaration(
      $this->_keyword,
      $name,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): NamespaceDeclaration {
    return new NamespaceDeclaration(
      $this->_keyword,
      $this->_name,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $name === $this->name() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceDeclaration(
        $keyword,
        $name,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->namespace_keyword, $position, $source);
    $position += $keyword->width();
    $name = EditableSyntax::from_json(
      $json->namespace_name, $position, $source);
    $position += $name->width();
    $body = EditableSyntax::from_json(
      $json->namespace_body, $position, $source);
    $position += $body->width();
    return new NamespaceDeclaration(
        $keyword,
        $name,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_name;
    yield $this->_body;
    yield break;
  }
}
final class NamespaceBody extends EditableSyntax {
  private EditableSyntax $_left_brace;
  private EditableSyntax $_declarations;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $left_brace,
    EditableSyntax $declarations,
    EditableSyntax $right_brace) {
    parent::__construct('namespace_body');
    $this->_left_brace = $left_brace;
    $this->_declarations = $declarations;
    $this->_right_brace = $right_brace;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function declarations(): EditableSyntax {
    return $this->_declarations;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_left_brace(EditableSyntax $left_brace): NamespaceBody {
    return new NamespaceBody(
      $left_brace,
      $this->_declarations,
      $this->_right_brace);
  }
  public function with_declarations(EditableSyntax $declarations): NamespaceBody {
    return new NamespaceBody(
      $this->_left_brace,
      $declarations,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): NamespaceBody {
    return new NamespaceBody(
      $this->_left_brace,
      $this->_declarations,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $declarations = $this->declarations()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $left_brace === $this->left_brace() &&
      $declarations === $this->declarations() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceBody(
        $left_brace,
        $declarations,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_brace = EditableSyntax::from_json(
      $json->namespace_left_brace, $position, $source);
    $position += $left_brace->width();
    $declarations = EditableSyntax::from_json(
      $json->namespace_declarations, $position, $source);
    $position += $declarations->width();
    $right_brace = EditableSyntax::from_json(
      $json->namespace_right_brace, $position, $source);
    $position += $right_brace->width();
    return new NamespaceBody(
        $left_brace,
        $declarations,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_brace;
    yield $this->_declarations;
    yield $this->_right_brace;
    yield break;
  }
}
final class NamespaceEmptyBody extends EditableSyntax {
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $semicolon) {
    parent::__construct('namespace_empty_body');
    $this->_semicolon = $semicolon;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_semicolon(EditableSyntax $semicolon): NamespaceEmptyBody {
    return new NamespaceEmptyBody(
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceEmptyBody(
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $semicolon = EditableSyntax::from_json(
      $json->namespace_semicolon, $position, $source);
    $position += $semicolon->width();
    return new NamespaceEmptyBody(
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_semicolon;
    yield break;
  }
}
final class NamespaceUseDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_kind;
  private EditableSyntax $_clauses;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $kind,
    EditableSyntax $clauses,
    EditableSyntax $semicolon) {
    parent::__construct('namespace_use_declaration');
    $this->_keyword = $keyword;
    $this->_kind = $kind;
    $this->_clauses = $clauses;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function kind(): EditableSyntax {
    return $this->_kind;
  }
  public function clauses(): EditableSyntax {
    return $this->_clauses;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): NamespaceUseDeclaration {
    return new NamespaceUseDeclaration(
      $keyword,
      $this->_kind,
      $this->_clauses,
      $this->_semicolon);
  }
  public function with_kind(EditableSyntax $kind): NamespaceUseDeclaration {
    return new NamespaceUseDeclaration(
      $this->_keyword,
      $kind,
      $this->_clauses,
      $this->_semicolon);
  }
  public function with_clauses(EditableSyntax $clauses): NamespaceUseDeclaration {
    return new NamespaceUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $clauses,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): NamespaceUseDeclaration {
    return new NamespaceUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $this->_clauses,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $kind = $this->kind()->rewrite($rewriter, $new_parents);
    $clauses = $this->clauses()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $kind === $this->kind() &&
      $clauses === $this->clauses() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceUseDeclaration(
        $keyword,
        $kind,
        $clauses,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->namespace_use_keyword, $position, $source);
    $position += $keyword->width();
    $kind = EditableSyntax::from_json(
      $json->namespace_use_kind, $position, $source);
    $position += $kind->width();
    $clauses = EditableSyntax::from_json(
      $json->namespace_use_clauses, $position, $source);
    $position += $clauses->width();
    $semicolon = EditableSyntax::from_json(
      $json->namespace_use_semicolon, $position, $source);
    $position += $semicolon->width();
    return new NamespaceUseDeclaration(
        $keyword,
        $kind,
        $clauses,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_kind;
    yield $this->_clauses;
    yield $this->_semicolon;
    yield break;
  }
}
final class NamespaceGroupUseDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_kind;
  private EditableSyntax $_prefix;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_clauses;
  private EditableSyntax $_right_brace;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $kind,
    EditableSyntax $prefix,
    EditableSyntax $left_brace,
    EditableSyntax $clauses,
    EditableSyntax $right_brace,
    EditableSyntax $semicolon) {
    parent::__construct('namespace_group_use_declaration');
    $this->_keyword = $keyword;
    $this->_kind = $kind;
    $this->_prefix = $prefix;
    $this->_left_brace = $left_brace;
    $this->_clauses = $clauses;
    $this->_right_brace = $right_brace;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function kind(): EditableSyntax {
    return $this->_kind;
  }
  public function prefix(): EditableSyntax {
    return $this->_prefix;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function clauses(): EditableSyntax {
    return $this->_clauses;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $keyword,
      $this->_kind,
      $this->_prefix,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace,
      $this->_semicolon);
  }
  public function with_kind(EditableSyntax $kind): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $kind,
      $this->_prefix,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace,
      $this->_semicolon);
  }
  public function with_prefix(EditableSyntax $prefix): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $prefix,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace,
      $this->_semicolon);
  }
  public function with_left_brace(EditableSyntax $left_brace): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $this->_prefix,
      $left_brace,
      $this->_clauses,
      $this->_right_brace,
      $this->_semicolon);
  }
  public function with_clauses(EditableSyntax $clauses): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $this->_prefix,
      $this->_left_brace,
      $clauses,
      $this->_right_brace,
      $this->_semicolon);
  }
  public function with_right_brace(EditableSyntax $right_brace): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $this->_prefix,
      $this->_left_brace,
      $this->_clauses,
      $right_brace,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): NamespaceGroupUseDeclaration {
    return new NamespaceGroupUseDeclaration(
      $this->_keyword,
      $this->_kind,
      $this->_prefix,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $kind = $this->kind()->rewrite($rewriter, $new_parents);
    $prefix = $this->prefix()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $clauses = $this->clauses()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $kind === $this->kind() &&
      $prefix === $this->prefix() &&
      $left_brace === $this->left_brace() &&
      $clauses === $this->clauses() &&
      $right_brace === $this->right_brace() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceGroupUseDeclaration(
        $keyword,
        $kind,
        $prefix,
        $left_brace,
        $clauses,
        $right_brace,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->namespace_group_use_keyword, $position, $source);
    $position += $keyword->width();
    $kind = EditableSyntax::from_json(
      $json->namespace_group_use_kind, $position, $source);
    $position += $kind->width();
    $prefix = EditableSyntax::from_json(
      $json->namespace_group_use_prefix, $position, $source);
    $position += $prefix->width();
    $left_brace = EditableSyntax::from_json(
      $json->namespace_group_use_left_brace, $position, $source);
    $position += $left_brace->width();
    $clauses = EditableSyntax::from_json(
      $json->namespace_group_use_clauses, $position, $source);
    $position += $clauses->width();
    $right_brace = EditableSyntax::from_json(
      $json->namespace_group_use_right_brace, $position, $source);
    $position += $right_brace->width();
    $semicolon = EditableSyntax::from_json(
      $json->namespace_group_use_semicolon, $position, $source);
    $position += $semicolon->width();
    return new NamespaceGroupUseDeclaration(
        $keyword,
        $kind,
        $prefix,
        $left_brace,
        $clauses,
        $right_brace,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_kind;
    yield $this->_prefix;
    yield $this->_left_brace;
    yield $this->_clauses;
    yield $this->_right_brace;
    yield $this->_semicolon;
    yield break;
  }
}
final class NamespaceUseClause extends EditableSyntax {
  private EditableSyntax $_clause_kind;
  private EditableSyntax $_name;
  private EditableSyntax $_as;
  private EditableSyntax $_alias;
  public function __construct(
    EditableSyntax $clause_kind,
    EditableSyntax $name,
    EditableSyntax $as,
    EditableSyntax $alias) {
    parent::__construct('namespace_use_clause');
    $this->_clause_kind = $clause_kind;
    $this->_name = $name;
    $this->_as = $as;
    $this->_alias = $alias;
  }
  public function clause_kind(): EditableSyntax {
    return $this->_clause_kind;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function as(): EditableSyntax {
    return $this->_as;
  }
  public function alias(): EditableSyntax {
    return $this->_alias;
  }
  public function with_clause_kind(EditableSyntax $clause_kind): NamespaceUseClause {
    return new NamespaceUseClause(
      $clause_kind,
      $this->_name,
      $this->_as,
      $this->_alias);
  }
  public function with_name(EditableSyntax $name): NamespaceUseClause {
    return new NamespaceUseClause(
      $this->_clause_kind,
      $name,
      $this->_as,
      $this->_alias);
  }
  public function with_as(EditableSyntax $as): NamespaceUseClause {
    return new NamespaceUseClause(
      $this->_clause_kind,
      $this->_name,
      $as,
      $this->_alias);
  }
  public function with_alias(EditableSyntax $alias): NamespaceUseClause {
    return new NamespaceUseClause(
      $this->_clause_kind,
      $this->_name,
      $this->_as,
      $alias);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $clause_kind = $this->clause_kind()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $as = $this->as()->rewrite($rewriter, $new_parents);
    $alias = $this->alias()->rewrite($rewriter, $new_parents);
    if (
      $clause_kind === $this->clause_kind() &&
      $name === $this->name() &&
      $as === $this->as() &&
      $alias === $this->alias()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NamespaceUseClause(
        $clause_kind,
        $name,
        $as,
        $alias), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $clause_kind = EditableSyntax::from_json(
      $json->namespace_use_clause_kind, $position, $source);
    $position += $clause_kind->width();
    $name = EditableSyntax::from_json(
      $json->namespace_use_name, $position, $source);
    $position += $name->width();
    $as = EditableSyntax::from_json(
      $json->namespace_use_as, $position, $source);
    $position += $as->width();
    $alias = EditableSyntax::from_json(
      $json->namespace_use_alias, $position, $source);
    $position += $alias->width();
    return new NamespaceUseClause(
        $clause_kind,
        $name,
        $as,
        $alias);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_clause_kind;
    yield $this->_name;
    yield $this->_as;
    yield $this->_alias;
    yield break;
  }
}
final class FunctionDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute_spec;
  private EditableSyntax $_declaration_header;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $attribute_spec,
    EditableSyntax $declaration_header,
    EditableSyntax $body) {
    parent::__construct('function_declaration');
    $this->_attribute_spec = $attribute_spec;
    $this->_declaration_header = $declaration_header;
    $this->_body = $body;
  }
  public function attribute_spec(): EditableSyntax {
    return $this->_attribute_spec;
  }
  public function declaration_header(): EditableSyntax {
    return $this->_declaration_header;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_attribute_spec(EditableSyntax $attribute_spec): FunctionDeclaration {
    return new FunctionDeclaration(
      $attribute_spec,
      $this->_declaration_header,
      $this->_body);
  }
  public function with_declaration_header(EditableSyntax $declaration_header): FunctionDeclaration {
    return new FunctionDeclaration(
      $this->_attribute_spec,
      $declaration_header,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): FunctionDeclaration {
    return new FunctionDeclaration(
      $this->_attribute_spec,
      $this->_declaration_header,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute_spec = $this->attribute_spec()->rewrite($rewriter, $new_parents);
    $declaration_header = $this->declaration_header()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $attribute_spec === $this->attribute_spec() &&
      $declaration_header === $this->declaration_header() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FunctionDeclaration(
        $attribute_spec,
        $declaration_header,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute_spec = EditableSyntax::from_json(
      $json->function_attribute_spec, $position, $source);
    $position += $attribute_spec->width();
    $declaration_header = EditableSyntax::from_json(
      $json->function_declaration_header, $position, $source);
    $position += $declaration_header->width();
    $body = EditableSyntax::from_json(
      $json->function_body, $position, $source);
    $position += $body->width();
    return new FunctionDeclaration(
        $attribute_spec,
        $declaration_header,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute_spec;
    yield $this->_declaration_header;
    yield $this->_body;
    yield break;
  }
}
final class FunctionDeclarationHeader extends EditableSyntax {
  private EditableSyntax $_async;
  private EditableSyntax $_coroutine;
  private EditableSyntax $_keyword;
  private EditableSyntax $_ampersand;
  private EditableSyntax $_name;
  private EditableSyntax $_type_parameter_list;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_parameter_list;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_colon;
  private EditableSyntax $_type;
  private EditableSyntax $_where_clause;
  public function __construct(
    EditableSyntax $async,
    EditableSyntax $coroutine,
    EditableSyntax $keyword,
    EditableSyntax $ampersand,
    EditableSyntax $name,
    EditableSyntax $type_parameter_list,
    EditableSyntax $left_paren,
    EditableSyntax $parameter_list,
    EditableSyntax $right_paren,
    EditableSyntax $colon,
    EditableSyntax $type,
    EditableSyntax $where_clause) {
    parent::__construct('function_declaration_header');
    $this->_async = $async;
    $this->_coroutine = $coroutine;
    $this->_keyword = $keyword;
    $this->_ampersand = $ampersand;
    $this->_name = $name;
    $this->_type_parameter_list = $type_parameter_list;
    $this->_left_paren = $left_paren;
    $this->_parameter_list = $parameter_list;
    $this->_right_paren = $right_paren;
    $this->_colon = $colon;
    $this->_type = $type;
    $this->_where_clause = $where_clause;
  }
  public function async(): EditableSyntax {
    return $this->_async;
  }
  public function coroutine(): EditableSyntax {
    return $this->_coroutine;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function ampersand(): EditableSyntax {
    return $this->_ampersand;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function type_parameter_list(): EditableSyntax {
    return $this->_type_parameter_list;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function parameter_list(): EditableSyntax {
    return $this->_parameter_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function where_clause(): EditableSyntax {
    return $this->_where_clause;
  }
  public function with_async(EditableSyntax $async): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_coroutine(EditableSyntax $coroutine): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_keyword(EditableSyntax $keyword): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_ampersand(EditableSyntax $ampersand): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_name(EditableSyntax $name): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_type_parameter_list(EditableSyntax $type_parameter_list): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_left_paren(EditableSyntax $left_paren): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_parameter_list(EditableSyntax $parameter_list): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_right_paren(EditableSyntax $right_paren): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $right_paren,
      $this->_colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_colon(EditableSyntax $colon): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $colon,
      $this->_type,
      $this->_where_clause);
  }
  public function with_type(EditableSyntax $type): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $type,
      $this->_where_clause);
  }
  public function with_where_clause(EditableSyntax $where_clause): FunctionDeclarationHeader {
    return new FunctionDeclarationHeader(
      $this->_async,
      $this->_coroutine,
      $this->_keyword,
      $this->_ampersand,
      $this->_name,
      $this->_type_parameter_list,
      $this->_left_paren,
      $this->_parameter_list,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $where_clause);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $async = $this->async()->rewrite($rewriter, $new_parents);
    $coroutine = $this->coroutine()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $ampersand = $this->ampersand()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $type_parameter_list = $this->type_parameter_list()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $parameter_list = $this->parameter_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $where_clause = $this->where_clause()->rewrite($rewriter, $new_parents);
    if (
      $async === $this->async() &&
      $coroutine === $this->coroutine() &&
      $keyword === $this->keyword() &&
      $ampersand === $this->ampersand() &&
      $name === $this->name() &&
      $type_parameter_list === $this->type_parameter_list() &&
      $left_paren === $this->left_paren() &&
      $parameter_list === $this->parameter_list() &&
      $right_paren === $this->right_paren() &&
      $colon === $this->colon() &&
      $type === $this->type() &&
      $where_clause === $this->where_clause()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FunctionDeclarationHeader(
        $async,
        $coroutine,
        $keyword,
        $ampersand,
        $name,
        $type_parameter_list,
        $left_paren,
        $parameter_list,
        $right_paren,
        $colon,
        $type,
        $where_clause), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $async = EditableSyntax::from_json(
      $json->function_async, $position, $source);
    $position += $async->width();
    $coroutine = EditableSyntax::from_json(
      $json->function_coroutine, $position, $source);
    $position += $coroutine->width();
    $keyword = EditableSyntax::from_json(
      $json->function_keyword, $position, $source);
    $position += $keyword->width();
    $ampersand = EditableSyntax::from_json(
      $json->function_ampersand, $position, $source);
    $position += $ampersand->width();
    $name = EditableSyntax::from_json(
      $json->function_name, $position, $source);
    $position += $name->width();
    $type_parameter_list = EditableSyntax::from_json(
      $json->function_type_parameter_list, $position, $source);
    $position += $type_parameter_list->width();
    $left_paren = EditableSyntax::from_json(
      $json->function_left_paren, $position, $source);
    $position += $left_paren->width();
    $parameter_list = EditableSyntax::from_json(
      $json->function_parameter_list, $position, $source);
    $position += $parameter_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->function_right_paren, $position, $source);
    $position += $right_paren->width();
    $colon = EditableSyntax::from_json(
      $json->function_colon, $position, $source);
    $position += $colon->width();
    $type = EditableSyntax::from_json(
      $json->function_type, $position, $source);
    $position += $type->width();
    $where_clause = EditableSyntax::from_json(
      $json->function_where_clause, $position, $source);
    $position += $where_clause->width();
    return new FunctionDeclarationHeader(
        $async,
        $coroutine,
        $keyword,
        $ampersand,
        $name,
        $type_parameter_list,
        $left_paren,
        $parameter_list,
        $right_paren,
        $colon,
        $type,
        $where_clause);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_async;
    yield $this->_coroutine;
    yield $this->_keyword;
    yield $this->_ampersand;
    yield $this->_name;
    yield $this->_type_parameter_list;
    yield $this->_left_paren;
    yield $this->_parameter_list;
    yield $this->_right_paren;
    yield $this->_colon;
    yield $this->_type;
    yield $this->_where_clause;
    yield break;
  }
}
final class WhereClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_constraints;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $constraints) {
    parent::__construct('where_clause');
    $this->_keyword = $keyword;
    $this->_constraints = $constraints;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function constraints(): EditableSyntax {
    return $this->_constraints;
  }
  public function with_keyword(EditableSyntax $keyword): WhereClause {
    return new WhereClause(
      $keyword,
      $this->_constraints);
  }
  public function with_constraints(EditableSyntax $constraints): WhereClause {
    return new WhereClause(
      $this->_keyword,
      $constraints);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $constraints = $this->constraints()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $constraints === $this->constraints()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new WhereClause(
        $keyword,
        $constraints), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->where_clause_keyword, $position, $source);
    $position += $keyword->width();
    $constraints = EditableSyntax::from_json(
      $json->where_clause_constraints, $position, $source);
    $position += $constraints->width();
    return new WhereClause(
        $keyword,
        $constraints);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_constraints;
    yield break;
  }
}
final class WhereConstraint extends EditableSyntax {
  private EditableSyntax $_left_type;
  private EditableSyntax $_operator;
  private EditableSyntax $_right_type;
  public function __construct(
    EditableSyntax $left_type,
    EditableSyntax $operator,
    EditableSyntax $right_type) {
    parent::__construct('where_constraint');
    $this->_left_type = $left_type;
    $this->_operator = $operator;
    $this->_right_type = $right_type;
  }
  public function left_type(): EditableSyntax {
    return $this->_left_type;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function right_type(): EditableSyntax {
    return $this->_right_type;
  }
  public function with_left_type(EditableSyntax $left_type): WhereConstraint {
    return new WhereConstraint(
      $left_type,
      $this->_operator,
      $this->_right_type);
  }
  public function with_operator(EditableSyntax $operator): WhereConstraint {
    return new WhereConstraint(
      $this->_left_type,
      $operator,
      $this->_right_type);
  }
  public function with_right_type(EditableSyntax $right_type): WhereConstraint {
    return new WhereConstraint(
      $this->_left_type,
      $this->_operator,
      $right_type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_type = $this->left_type()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $right_type = $this->right_type()->rewrite($rewriter, $new_parents);
    if (
      $left_type === $this->left_type() &&
      $operator === $this->operator() &&
      $right_type === $this->right_type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new WhereConstraint(
        $left_type,
        $operator,
        $right_type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_type = EditableSyntax::from_json(
      $json->where_constraint_left_type, $position, $source);
    $position += $left_type->width();
    $operator = EditableSyntax::from_json(
      $json->where_constraint_operator, $position, $source);
    $position += $operator->width();
    $right_type = EditableSyntax::from_json(
      $json->where_constraint_right_type, $position, $source);
    $position += $right_type->width();
    return new WhereConstraint(
        $left_type,
        $operator,
        $right_type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_type;
    yield $this->_operator;
    yield $this->_right_type;
    yield break;
  }
}
final class MethodishDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute;
  private EditableSyntax $_modifiers;
  private EditableSyntax $_function_decl_header;
  private EditableSyntax $_function_body;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $attribute,
    EditableSyntax $modifiers,
    EditableSyntax $function_decl_header,
    EditableSyntax $function_body,
    EditableSyntax $semicolon) {
    parent::__construct('methodish_declaration');
    $this->_attribute = $attribute;
    $this->_modifiers = $modifiers;
    $this->_function_decl_header = $function_decl_header;
    $this->_function_body = $function_body;
    $this->_semicolon = $semicolon;
  }
  public function attribute(): EditableSyntax {
    return $this->_attribute;
  }
  public function modifiers(): EditableSyntax {
    return $this->_modifiers;
  }
  public function function_decl_header(): EditableSyntax {
    return $this->_function_decl_header;
  }
  public function function_body(): EditableSyntax {
    return $this->_function_body;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_attribute(EditableSyntax $attribute): MethodishDeclaration {
    return new MethodishDeclaration(
      $attribute,
      $this->_modifiers,
      $this->_function_decl_header,
      $this->_function_body,
      $this->_semicolon);
  }
  public function with_modifiers(EditableSyntax $modifiers): MethodishDeclaration {
    return new MethodishDeclaration(
      $this->_attribute,
      $modifiers,
      $this->_function_decl_header,
      $this->_function_body,
      $this->_semicolon);
  }
  public function with_function_decl_header(EditableSyntax $function_decl_header): MethodishDeclaration {
    return new MethodishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $function_decl_header,
      $this->_function_body,
      $this->_semicolon);
  }
  public function with_function_body(EditableSyntax $function_body): MethodishDeclaration {
    return new MethodishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_function_decl_header,
      $function_body,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): MethodishDeclaration {
    return new MethodishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_function_decl_header,
      $this->_function_body,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute = $this->attribute()->rewrite($rewriter, $new_parents);
    $modifiers = $this->modifiers()->rewrite($rewriter, $new_parents);
    $function_decl_header = $this->function_decl_header()->rewrite($rewriter, $new_parents);
    $function_body = $this->function_body()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $attribute === $this->attribute() &&
      $modifiers === $this->modifiers() &&
      $function_decl_header === $this->function_decl_header() &&
      $function_body === $this->function_body() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new MethodishDeclaration(
        $attribute,
        $modifiers,
        $function_decl_header,
        $function_body,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute = EditableSyntax::from_json(
      $json->methodish_attribute, $position, $source);
    $position += $attribute->width();
    $modifiers = EditableSyntax::from_json(
      $json->methodish_modifiers, $position, $source);
    $position += $modifiers->width();
    $function_decl_header = EditableSyntax::from_json(
      $json->methodish_function_decl_header, $position, $source);
    $position += $function_decl_header->width();
    $function_body = EditableSyntax::from_json(
      $json->methodish_function_body, $position, $source);
    $position += $function_body->width();
    $semicolon = EditableSyntax::from_json(
      $json->methodish_semicolon, $position, $source);
    $position += $semicolon->width();
    return new MethodishDeclaration(
        $attribute,
        $modifiers,
        $function_decl_header,
        $function_body,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute;
    yield $this->_modifiers;
    yield $this->_function_decl_header;
    yield $this->_function_body;
    yield $this->_semicolon;
    yield break;
  }
}
final class ClassishDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute;
  private EditableSyntax $_modifiers;
  private EditableSyntax $_keyword;
  private EditableSyntax $_name;
  private EditableSyntax $_type_parameters;
  private EditableSyntax $_extends_keyword;
  private EditableSyntax $_extends_list;
  private EditableSyntax $_implements_keyword;
  private EditableSyntax $_implements_list;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $attribute,
    EditableSyntax $modifiers,
    EditableSyntax $keyword,
    EditableSyntax $name,
    EditableSyntax $type_parameters,
    EditableSyntax $extends_keyword,
    EditableSyntax $extends_list,
    EditableSyntax $implements_keyword,
    EditableSyntax $implements_list,
    EditableSyntax $body) {
    parent::__construct('classish_declaration');
    $this->_attribute = $attribute;
    $this->_modifiers = $modifiers;
    $this->_keyword = $keyword;
    $this->_name = $name;
    $this->_type_parameters = $type_parameters;
    $this->_extends_keyword = $extends_keyword;
    $this->_extends_list = $extends_list;
    $this->_implements_keyword = $implements_keyword;
    $this->_implements_list = $implements_list;
    $this->_body = $body;
  }
  public function attribute(): EditableSyntax {
    return $this->_attribute;
  }
  public function modifiers(): EditableSyntax {
    return $this->_modifiers;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function type_parameters(): EditableSyntax {
    return $this->_type_parameters;
  }
  public function extends_keyword(): EditableSyntax {
    return $this->_extends_keyword;
  }
  public function extends_list(): EditableSyntax {
    return $this->_extends_list;
  }
  public function implements_keyword(): EditableSyntax {
    return $this->_implements_keyword;
  }
  public function implements_list(): EditableSyntax {
    return $this->_implements_list;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_attribute(EditableSyntax $attribute): ClassishDeclaration {
    return new ClassishDeclaration(
      $attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_modifiers(EditableSyntax $modifiers): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_keyword(EditableSyntax $keyword): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_name(EditableSyntax $name): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_type_parameters(EditableSyntax $type_parameters): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_extends_keyword(EditableSyntax $extends_keyword): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_extends_list(EditableSyntax $extends_list): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_implements_keyword(EditableSyntax $implements_keyword): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $implements_keyword,
      $this->_implements_list,
      $this->_body);
  }
  public function with_implements_list(EditableSyntax $implements_list): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $implements_list,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): ClassishDeclaration {
    return new ClassishDeclaration(
      $this->_attribute,
      $this->_modifiers,
      $this->_keyword,
      $this->_name,
      $this->_type_parameters,
      $this->_extends_keyword,
      $this->_extends_list,
      $this->_implements_keyword,
      $this->_implements_list,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute = $this->attribute()->rewrite($rewriter, $new_parents);
    $modifiers = $this->modifiers()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $type_parameters = $this->type_parameters()->rewrite($rewriter, $new_parents);
    $extends_keyword = $this->extends_keyword()->rewrite($rewriter, $new_parents);
    $extends_list = $this->extends_list()->rewrite($rewriter, $new_parents);
    $implements_keyword = $this->implements_keyword()->rewrite($rewriter, $new_parents);
    $implements_list = $this->implements_list()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $attribute === $this->attribute() &&
      $modifiers === $this->modifiers() &&
      $keyword === $this->keyword() &&
      $name === $this->name() &&
      $type_parameters === $this->type_parameters() &&
      $extends_keyword === $this->extends_keyword() &&
      $extends_list === $this->extends_list() &&
      $implements_keyword === $this->implements_keyword() &&
      $implements_list === $this->implements_list() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ClassishDeclaration(
        $attribute,
        $modifiers,
        $keyword,
        $name,
        $type_parameters,
        $extends_keyword,
        $extends_list,
        $implements_keyword,
        $implements_list,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute = EditableSyntax::from_json(
      $json->classish_attribute, $position, $source);
    $position += $attribute->width();
    $modifiers = EditableSyntax::from_json(
      $json->classish_modifiers, $position, $source);
    $position += $modifiers->width();
    $keyword = EditableSyntax::from_json(
      $json->classish_keyword, $position, $source);
    $position += $keyword->width();
    $name = EditableSyntax::from_json(
      $json->classish_name, $position, $source);
    $position += $name->width();
    $type_parameters = EditableSyntax::from_json(
      $json->classish_type_parameters, $position, $source);
    $position += $type_parameters->width();
    $extends_keyword = EditableSyntax::from_json(
      $json->classish_extends_keyword, $position, $source);
    $position += $extends_keyword->width();
    $extends_list = EditableSyntax::from_json(
      $json->classish_extends_list, $position, $source);
    $position += $extends_list->width();
    $implements_keyword = EditableSyntax::from_json(
      $json->classish_implements_keyword, $position, $source);
    $position += $implements_keyword->width();
    $implements_list = EditableSyntax::from_json(
      $json->classish_implements_list, $position, $source);
    $position += $implements_list->width();
    $body = EditableSyntax::from_json(
      $json->classish_body, $position, $source);
    $position += $body->width();
    return new ClassishDeclaration(
        $attribute,
        $modifiers,
        $keyword,
        $name,
        $type_parameters,
        $extends_keyword,
        $extends_list,
        $implements_keyword,
        $implements_list,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute;
    yield $this->_modifiers;
    yield $this->_keyword;
    yield $this->_name;
    yield $this->_type_parameters;
    yield $this->_extends_keyword;
    yield $this->_extends_list;
    yield $this->_implements_keyword;
    yield $this->_implements_list;
    yield $this->_body;
    yield break;
  }
}
final class ClassishBody extends EditableSyntax {
  private EditableSyntax $_left_brace;
  private EditableSyntax $_elements;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $left_brace,
    EditableSyntax $elements,
    EditableSyntax $right_brace) {
    parent::__construct('classish_body');
    $this->_left_brace = $left_brace;
    $this->_elements = $elements;
    $this->_right_brace = $right_brace;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function elements(): EditableSyntax {
    return $this->_elements;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_left_brace(EditableSyntax $left_brace): ClassishBody {
    return new ClassishBody(
      $left_brace,
      $this->_elements,
      $this->_right_brace);
  }
  public function with_elements(EditableSyntax $elements): ClassishBody {
    return new ClassishBody(
      $this->_left_brace,
      $elements,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): ClassishBody {
    return new ClassishBody(
      $this->_left_brace,
      $this->_elements,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $elements = $this->elements()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $left_brace === $this->left_brace() &&
      $elements === $this->elements() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ClassishBody(
        $left_brace,
        $elements,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_brace = EditableSyntax::from_json(
      $json->classish_body_left_brace, $position, $source);
    $position += $left_brace->width();
    $elements = EditableSyntax::from_json(
      $json->classish_body_elements, $position, $source);
    $position += $elements->width();
    $right_brace = EditableSyntax::from_json(
      $json->classish_body_right_brace, $position, $source);
    $position += $right_brace->width();
    return new ClassishBody(
        $left_brace,
        $elements,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_brace;
    yield $this->_elements;
    yield $this->_right_brace;
    yield break;
  }
}
final class TraitUsePrecedenceItem extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_keyword;
  private EditableSyntax $_removed_names;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $keyword,
    EditableSyntax $removed_names) {
    parent::__construct('trait_use_precedence_item');
    $this->_name = $name;
    $this->_keyword = $keyword;
    $this->_removed_names = $removed_names;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function removed_names(): EditableSyntax {
    return $this->_removed_names;
  }
  public function with_name(EditableSyntax $name): TraitUsePrecedenceItem {
    return new TraitUsePrecedenceItem(
      $name,
      $this->_keyword,
      $this->_removed_names);
  }
  public function with_keyword(EditableSyntax $keyword): TraitUsePrecedenceItem {
    return new TraitUsePrecedenceItem(
      $this->_name,
      $keyword,
      $this->_removed_names);
  }
  public function with_removed_names(EditableSyntax $removed_names): TraitUsePrecedenceItem {
    return new TraitUsePrecedenceItem(
      $this->_name,
      $this->_keyword,
      $removed_names);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $removed_names = $this->removed_names()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $keyword === $this->keyword() &&
      $removed_names === $this->removed_names()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TraitUsePrecedenceItem(
        $name,
        $keyword,
        $removed_names), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->trait_use_precedence_item_name, $position, $source);
    $position += $name->width();
    $keyword = EditableSyntax::from_json(
      $json->trait_use_precedence_item_keyword, $position, $source);
    $position += $keyword->width();
    $removed_names = EditableSyntax::from_json(
      $json->trait_use_precedence_item_removed_names, $position, $source);
    $position += $removed_names->width();
    return new TraitUsePrecedenceItem(
        $name,
        $keyword,
        $removed_names);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_keyword;
    yield $this->_removed_names;
    yield break;
  }
}
final class TraitUseAliasItem extends EditableSyntax {
  private EditableSyntax $_aliasing_name;
  private EditableSyntax $_keyword;
  private EditableSyntax $_visibility;
  private EditableSyntax $_aliased_name;
  public function __construct(
    EditableSyntax $aliasing_name,
    EditableSyntax $keyword,
    EditableSyntax $visibility,
    EditableSyntax $aliased_name) {
    parent::__construct('trait_use_alias_item');
    $this->_aliasing_name = $aliasing_name;
    $this->_keyword = $keyword;
    $this->_visibility = $visibility;
    $this->_aliased_name = $aliased_name;
  }
  public function aliasing_name(): EditableSyntax {
    return $this->_aliasing_name;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function visibility(): EditableSyntax {
    return $this->_visibility;
  }
  public function aliased_name(): EditableSyntax {
    return $this->_aliased_name;
  }
  public function with_aliasing_name(EditableSyntax $aliasing_name): TraitUseAliasItem {
    return new TraitUseAliasItem(
      $aliasing_name,
      $this->_keyword,
      $this->_visibility,
      $this->_aliased_name);
  }
  public function with_keyword(EditableSyntax $keyword): TraitUseAliasItem {
    return new TraitUseAliasItem(
      $this->_aliasing_name,
      $keyword,
      $this->_visibility,
      $this->_aliased_name);
  }
  public function with_visibility(EditableSyntax $visibility): TraitUseAliasItem {
    return new TraitUseAliasItem(
      $this->_aliasing_name,
      $this->_keyword,
      $visibility,
      $this->_aliased_name);
  }
  public function with_aliased_name(EditableSyntax $aliased_name): TraitUseAliasItem {
    return new TraitUseAliasItem(
      $this->_aliasing_name,
      $this->_keyword,
      $this->_visibility,
      $aliased_name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $aliasing_name = $this->aliasing_name()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $visibility = $this->visibility()->rewrite($rewriter, $new_parents);
    $aliased_name = $this->aliased_name()->rewrite($rewriter, $new_parents);
    if (
      $aliasing_name === $this->aliasing_name() &&
      $keyword === $this->keyword() &&
      $visibility === $this->visibility() &&
      $aliased_name === $this->aliased_name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TraitUseAliasItem(
        $aliasing_name,
        $keyword,
        $visibility,
        $aliased_name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $aliasing_name = EditableSyntax::from_json(
      $json->trait_use_alias_item_aliasing_name, $position, $source);
    $position += $aliasing_name->width();
    $keyword = EditableSyntax::from_json(
      $json->trait_use_alias_item_keyword, $position, $source);
    $position += $keyword->width();
    $visibility = EditableSyntax::from_json(
      $json->trait_use_alias_item_visibility, $position, $source);
    $position += $visibility->width();
    $aliased_name = EditableSyntax::from_json(
      $json->trait_use_alias_item_aliased_name, $position, $source);
    $position += $aliased_name->width();
    return new TraitUseAliasItem(
        $aliasing_name,
        $keyword,
        $visibility,
        $aliased_name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_aliasing_name;
    yield $this->_keyword;
    yield $this->_visibility;
    yield $this->_aliased_name;
    yield break;
  }
}
final class TraitUseConflictResolution extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_names;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_clauses;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $names,
    EditableSyntax $left_brace,
    EditableSyntax $clauses,
    EditableSyntax $right_brace) {
    parent::__construct('trait_use_conflict_resolution');
    $this->_keyword = $keyword;
    $this->_names = $names;
    $this->_left_brace = $left_brace;
    $this->_clauses = $clauses;
    $this->_right_brace = $right_brace;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function names(): EditableSyntax {
    return $this->_names;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function clauses(): EditableSyntax {
    return $this->_clauses;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_keyword(EditableSyntax $keyword): TraitUseConflictResolution {
    return new TraitUseConflictResolution(
      $keyword,
      $this->_names,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace);
  }
  public function with_names(EditableSyntax $names): TraitUseConflictResolution {
    return new TraitUseConflictResolution(
      $this->_keyword,
      $names,
      $this->_left_brace,
      $this->_clauses,
      $this->_right_brace);
  }
  public function with_left_brace(EditableSyntax $left_brace): TraitUseConflictResolution {
    return new TraitUseConflictResolution(
      $this->_keyword,
      $this->_names,
      $left_brace,
      $this->_clauses,
      $this->_right_brace);
  }
  public function with_clauses(EditableSyntax $clauses): TraitUseConflictResolution {
    return new TraitUseConflictResolution(
      $this->_keyword,
      $this->_names,
      $this->_left_brace,
      $clauses,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): TraitUseConflictResolution {
    return new TraitUseConflictResolution(
      $this->_keyword,
      $this->_names,
      $this->_left_brace,
      $this->_clauses,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $names = $this->names()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $clauses = $this->clauses()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $names === $this->names() &&
      $left_brace === $this->left_brace() &&
      $clauses === $this->clauses() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TraitUseConflictResolution(
        $keyword,
        $names,
        $left_brace,
        $clauses,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->trait_use_conflict_resolution_keyword, $position, $source);
    $position += $keyword->width();
    $names = EditableSyntax::from_json(
      $json->trait_use_conflict_resolution_names, $position, $source);
    $position += $names->width();
    $left_brace = EditableSyntax::from_json(
      $json->trait_use_conflict_resolution_left_brace, $position, $source);
    $position += $left_brace->width();
    $clauses = EditableSyntax::from_json(
      $json->trait_use_conflict_resolution_clauses, $position, $source);
    $position += $clauses->width();
    $right_brace = EditableSyntax::from_json(
      $json->trait_use_conflict_resolution_right_brace, $position, $source);
    $position += $right_brace->width();
    return new TraitUseConflictResolution(
        $keyword,
        $names,
        $left_brace,
        $clauses,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_names;
    yield $this->_left_brace;
    yield $this->_clauses;
    yield $this->_right_brace;
    yield break;
  }
}
final class TraitUse extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_names;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $names,
    EditableSyntax $semicolon) {
    parent::__construct('trait_use');
    $this->_keyword = $keyword;
    $this->_names = $names;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function names(): EditableSyntax {
    return $this->_names;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): TraitUse {
    return new TraitUse(
      $keyword,
      $this->_names,
      $this->_semicolon);
  }
  public function with_names(EditableSyntax $names): TraitUse {
    return new TraitUse(
      $this->_keyword,
      $names,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): TraitUse {
    return new TraitUse(
      $this->_keyword,
      $this->_names,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $names = $this->names()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $names === $this->names() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TraitUse(
        $keyword,
        $names,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->trait_use_keyword, $position, $source);
    $position += $keyword->width();
    $names = EditableSyntax::from_json(
      $json->trait_use_names, $position, $source);
    $position += $names->width();
    $semicolon = EditableSyntax::from_json(
      $json->trait_use_semicolon, $position, $source);
    $position += $semicolon->width();
    return new TraitUse(
        $keyword,
        $names,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_names;
    yield $this->_semicolon;
    yield break;
  }
}
final class RequireClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_kind;
  private EditableSyntax $_name;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $kind,
    EditableSyntax $name,
    EditableSyntax $semicolon) {
    parent::__construct('require_clause');
    $this->_keyword = $keyword;
    $this->_kind = $kind;
    $this->_name = $name;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function kind(): EditableSyntax {
    return $this->_kind;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): RequireClause {
    return new RequireClause(
      $keyword,
      $this->_kind,
      $this->_name,
      $this->_semicolon);
  }
  public function with_kind(EditableSyntax $kind): RequireClause {
    return new RequireClause(
      $this->_keyword,
      $kind,
      $this->_name,
      $this->_semicolon);
  }
  public function with_name(EditableSyntax $name): RequireClause {
    return new RequireClause(
      $this->_keyword,
      $this->_kind,
      $name,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): RequireClause {
    return new RequireClause(
      $this->_keyword,
      $this->_kind,
      $this->_name,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $kind = $this->kind()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $kind === $this->kind() &&
      $name === $this->name() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new RequireClause(
        $keyword,
        $kind,
        $name,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->require_keyword, $position, $source);
    $position += $keyword->width();
    $kind = EditableSyntax::from_json(
      $json->require_kind, $position, $source);
    $position += $kind->width();
    $name = EditableSyntax::from_json(
      $json->require_name, $position, $source);
    $position += $name->width();
    $semicolon = EditableSyntax::from_json(
      $json->require_semicolon, $position, $source);
    $position += $semicolon->width();
    return new RequireClause(
        $keyword,
        $kind,
        $name,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_kind;
    yield $this->_name;
    yield $this->_semicolon;
    yield break;
  }
}
final class ConstDeclaration extends EditableSyntax {
  private EditableSyntax $_abstract;
  private EditableSyntax $_keyword;
  private EditableSyntax $_type_specifier;
  private EditableSyntax $_declarators;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $abstract,
    EditableSyntax $keyword,
    EditableSyntax $type_specifier,
    EditableSyntax $declarators,
    EditableSyntax $semicolon) {
    parent::__construct('const_declaration');
    $this->_abstract = $abstract;
    $this->_keyword = $keyword;
    $this->_type_specifier = $type_specifier;
    $this->_declarators = $declarators;
    $this->_semicolon = $semicolon;
  }
  public function abstract(): EditableSyntax {
    return $this->_abstract;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function type_specifier(): EditableSyntax {
    return $this->_type_specifier;
  }
  public function declarators(): EditableSyntax {
    return $this->_declarators;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_abstract(EditableSyntax $abstract): ConstDeclaration {
    return new ConstDeclaration(
      $abstract,
      $this->_keyword,
      $this->_type_specifier,
      $this->_declarators,
      $this->_semicolon);
  }
  public function with_keyword(EditableSyntax $keyword): ConstDeclaration {
    return new ConstDeclaration(
      $this->_abstract,
      $keyword,
      $this->_type_specifier,
      $this->_declarators,
      $this->_semicolon);
  }
  public function with_type_specifier(EditableSyntax $type_specifier): ConstDeclaration {
    return new ConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $type_specifier,
      $this->_declarators,
      $this->_semicolon);
  }
  public function with_declarators(EditableSyntax $declarators): ConstDeclaration {
    return new ConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_specifier,
      $declarators,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): ConstDeclaration {
    return new ConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_specifier,
      $this->_declarators,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $abstract = $this->abstract()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $type_specifier = $this->type_specifier()->rewrite($rewriter, $new_parents);
    $declarators = $this->declarators()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $abstract === $this->abstract() &&
      $keyword === $this->keyword() &&
      $type_specifier === $this->type_specifier() &&
      $declarators === $this->declarators() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ConstDeclaration(
        $abstract,
        $keyword,
        $type_specifier,
        $declarators,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $abstract = EditableSyntax::from_json(
      $json->const_abstract, $position, $source);
    $position += $abstract->width();
    $keyword = EditableSyntax::from_json(
      $json->const_keyword, $position, $source);
    $position += $keyword->width();
    $type_specifier = EditableSyntax::from_json(
      $json->const_type_specifier, $position, $source);
    $position += $type_specifier->width();
    $declarators = EditableSyntax::from_json(
      $json->const_declarators, $position, $source);
    $position += $declarators->width();
    $semicolon = EditableSyntax::from_json(
      $json->const_semicolon, $position, $source);
    $position += $semicolon->width();
    return new ConstDeclaration(
        $abstract,
        $keyword,
        $type_specifier,
        $declarators,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_abstract;
    yield $this->_keyword;
    yield $this->_type_specifier;
    yield $this->_declarators;
    yield $this->_semicolon;
    yield break;
  }
}
final class ConstantDeclarator extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_initializer;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $initializer) {
    parent::__construct('constant_declarator');
    $this->_name = $name;
    $this->_initializer = $initializer;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function initializer(): EditableSyntax {
    return $this->_initializer;
  }
  public function with_name(EditableSyntax $name): ConstantDeclarator {
    return new ConstantDeclarator(
      $name,
      $this->_initializer);
  }
  public function with_initializer(EditableSyntax $initializer): ConstantDeclarator {
    return new ConstantDeclarator(
      $this->_name,
      $initializer);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $initializer = $this->initializer()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $initializer === $this->initializer()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ConstantDeclarator(
        $name,
        $initializer), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->constant_declarator_name, $position, $source);
    $position += $name->width();
    $initializer = EditableSyntax::from_json(
      $json->constant_declarator_initializer, $position, $source);
    $position += $initializer->width();
    return new ConstantDeclarator(
        $name,
        $initializer);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_initializer;
    yield break;
  }
}
final class TypeConstDeclaration extends EditableSyntax {
  private EditableSyntax $_abstract;
  private EditableSyntax $_keyword;
  private EditableSyntax $_type_keyword;
  private EditableSyntax $_name;
  private EditableSyntax $_type_constraint;
  private EditableSyntax $_equal;
  private EditableSyntax $_type_specifier;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $abstract,
    EditableSyntax $keyword,
    EditableSyntax $type_keyword,
    EditableSyntax $name,
    EditableSyntax $type_constraint,
    EditableSyntax $equal,
    EditableSyntax $type_specifier,
    EditableSyntax $semicolon) {
    parent::__construct('type_const_declaration');
    $this->_abstract = $abstract;
    $this->_keyword = $keyword;
    $this->_type_keyword = $type_keyword;
    $this->_name = $name;
    $this->_type_constraint = $type_constraint;
    $this->_equal = $equal;
    $this->_type_specifier = $type_specifier;
    $this->_semicolon = $semicolon;
  }
  public function abstract(): EditableSyntax {
    return $this->_abstract;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function type_keyword(): EditableSyntax {
    return $this->_type_keyword;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function type_constraint(): EditableSyntax {
    return $this->_type_constraint;
  }
  public function equal(): EditableSyntax {
    return $this->_equal;
  }
  public function type_specifier(): EditableSyntax {
    return $this->_type_specifier;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_abstract(EditableSyntax $abstract): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $abstract,
      $this->_keyword,
      $this->_type_keyword,
      $this->_name,
      $this->_type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_keyword(EditableSyntax $keyword): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $keyword,
      $this->_type_keyword,
      $this->_name,
      $this->_type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_type_keyword(EditableSyntax $type_keyword): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $type_keyword,
      $this->_name,
      $this->_type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_name(EditableSyntax $name): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_keyword,
      $name,
      $this->_type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_type_constraint(EditableSyntax $type_constraint): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_keyword,
      $this->_name,
      $type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_equal(EditableSyntax $equal): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_keyword,
      $this->_name,
      $this->_type_constraint,
      $equal,
      $this->_type_specifier,
      $this->_semicolon);
  }
  public function with_type_specifier(EditableSyntax $type_specifier): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_keyword,
      $this->_name,
      $this->_type_constraint,
      $this->_equal,
      $type_specifier,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): TypeConstDeclaration {
    return new TypeConstDeclaration(
      $this->_abstract,
      $this->_keyword,
      $this->_type_keyword,
      $this->_name,
      $this->_type_constraint,
      $this->_equal,
      $this->_type_specifier,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $abstract = $this->abstract()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $type_keyword = $this->type_keyword()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $type_constraint = $this->type_constraint()->rewrite($rewriter, $new_parents);
    $equal = $this->equal()->rewrite($rewriter, $new_parents);
    $type_specifier = $this->type_specifier()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $abstract === $this->abstract() &&
      $keyword === $this->keyword() &&
      $type_keyword === $this->type_keyword() &&
      $name === $this->name() &&
      $type_constraint === $this->type_constraint() &&
      $equal === $this->equal() &&
      $type_specifier === $this->type_specifier() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeConstDeclaration(
        $abstract,
        $keyword,
        $type_keyword,
        $name,
        $type_constraint,
        $equal,
        $type_specifier,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $abstract = EditableSyntax::from_json(
      $json->type_const_abstract, $position, $source);
    $position += $abstract->width();
    $keyword = EditableSyntax::from_json(
      $json->type_const_keyword, $position, $source);
    $position += $keyword->width();
    $type_keyword = EditableSyntax::from_json(
      $json->type_const_type_keyword, $position, $source);
    $position += $type_keyword->width();
    $name = EditableSyntax::from_json(
      $json->type_const_name, $position, $source);
    $position += $name->width();
    $type_constraint = EditableSyntax::from_json(
      $json->type_const_type_constraint, $position, $source);
    $position += $type_constraint->width();
    $equal = EditableSyntax::from_json(
      $json->type_const_equal, $position, $source);
    $position += $equal->width();
    $type_specifier = EditableSyntax::from_json(
      $json->type_const_type_specifier, $position, $source);
    $position += $type_specifier->width();
    $semicolon = EditableSyntax::from_json(
      $json->type_const_semicolon, $position, $source);
    $position += $semicolon->width();
    return new TypeConstDeclaration(
        $abstract,
        $keyword,
        $type_keyword,
        $name,
        $type_constraint,
        $equal,
        $type_specifier,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_abstract;
    yield $this->_keyword;
    yield $this->_type_keyword;
    yield $this->_name;
    yield $this->_type_constraint;
    yield $this->_equal;
    yield $this->_type_specifier;
    yield $this->_semicolon;
    yield break;
  }
}
final class DecoratedExpression extends EditableSyntax {
  private EditableSyntax $_decorator;
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $decorator,
    EditableSyntax $expression) {
    parent::__construct('decorated_expression');
    $this->_decorator = $decorator;
    $this->_expression = $expression;
  }
  public function decorator(): EditableSyntax {
    return $this->_decorator;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_decorator(EditableSyntax $decorator): DecoratedExpression {
    return new DecoratedExpression(
      $decorator,
      $this->_expression);
  }
  public function with_expression(EditableSyntax $expression): DecoratedExpression {
    return new DecoratedExpression(
      $this->_decorator,
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $decorator = $this->decorator()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $decorator === $this->decorator() &&
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DecoratedExpression(
        $decorator,
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $decorator = EditableSyntax::from_json(
      $json->decorated_expression_decorator, $position, $source);
    $position += $decorator->width();
    $expression = EditableSyntax::from_json(
      $json->decorated_expression_expression, $position, $source);
    $position += $expression->width();
    return new DecoratedExpression(
        $decorator,
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_decorator;
    yield $this->_expression;
    yield break;
  }
}
final class ParameterDeclaration extends EditableSyntax {
  private EditableSyntax $_attribute;
  private EditableSyntax $_visibility;
  private EditableSyntax $_type;
  private EditableSyntax $_name;
  private EditableSyntax $_default_value;
  public function __construct(
    EditableSyntax $attribute,
    EditableSyntax $visibility,
    EditableSyntax $type,
    EditableSyntax $name,
    EditableSyntax $default_value) {
    parent::__construct('parameter_declaration');
    $this->_attribute = $attribute;
    $this->_visibility = $visibility;
    $this->_type = $type;
    $this->_name = $name;
    $this->_default_value = $default_value;
  }
  public function attribute(): EditableSyntax {
    return $this->_attribute;
  }
  public function visibility(): EditableSyntax {
    return $this->_visibility;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function default_value(): EditableSyntax {
    return $this->_default_value;
  }
  public function with_attribute(EditableSyntax $attribute): ParameterDeclaration {
    return new ParameterDeclaration(
      $attribute,
      $this->_visibility,
      $this->_type,
      $this->_name,
      $this->_default_value);
  }
  public function with_visibility(EditableSyntax $visibility): ParameterDeclaration {
    return new ParameterDeclaration(
      $this->_attribute,
      $visibility,
      $this->_type,
      $this->_name,
      $this->_default_value);
  }
  public function with_type(EditableSyntax $type): ParameterDeclaration {
    return new ParameterDeclaration(
      $this->_attribute,
      $this->_visibility,
      $type,
      $this->_name,
      $this->_default_value);
  }
  public function with_name(EditableSyntax $name): ParameterDeclaration {
    return new ParameterDeclaration(
      $this->_attribute,
      $this->_visibility,
      $this->_type,
      $name,
      $this->_default_value);
  }
  public function with_default_value(EditableSyntax $default_value): ParameterDeclaration {
    return new ParameterDeclaration(
      $this->_attribute,
      $this->_visibility,
      $this->_type,
      $this->_name,
      $default_value);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $attribute = $this->attribute()->rewrite($rewriter, $new_parents);
    $visibility = $this->visibility()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $default_value = $this->default_value()->rewrite($rewriter, $new_parents);
    if (
      $attribute === $this->attribute() &&
      $visibility === $this->visibility() &&
      $type === $this->type() &&
      $name === $this->name() &&
      $default_value === $this->default_value()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ParameterDeclaration(
        $attribute,
        $visibility,
        $type,
        $name,
        $default_value), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $attribute = EditableSyntax::from_json(
      $json->parameter_attribute, $position, $source);
    $position += $attribute->width();
    $visibility = EditableSyntax::from_json(
      $json->parameter_visibility, $position, $source);
    $position += $visibility->width();
    $type = EditableSyntax::from_json(
      $json->parameter_type, $position, $source);
    $position += $type->width();
    $name = EditableSyntax::from_json(
      $json->parameter_name, $position, $source);
    $position += $name->width();
    $default_value = EditableSyntax::from_json(
      $json->parameter_default_value, $position, $source);
    $position += $default_value->width();
    return new ParameterDeclaration(
        $attribute,
        $visibility,
        $type,
        $name,
        $default_value);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_attribute;
    yield $this->_visibility;
    yield $this->_type;
    yield $this->_name;
    yield $this->_default_value;
    yield break;
  }
}
final class VariadicParameter extends EditableSyntax {
  private EditableSyntax $_ellipsis;
  public function __construct(
    EditableSyntax $ellipsis) {
    parent::__construct('variadic_parameter');
    $this->_ellipsis = $ellipsis;
  }
  public function ellipsis(): EditableSyntax {
    return $this->_ellipsis;
  }
  public function with_ellipsis(EditableSyntax $ellipsis): VariadicParameter {
    return new VariadicParameter(
      $ellipsis);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $ellipsis = $this->ellipsis()->rewrite($rewriter, $new_parents);
    if (
      $ellipsis === $this->ellipsis()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VariadicParameter(
        $ellipsis), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $ellipsis = EditableSyntax::from_json(
      $json->variadic_parameter_ellipsis, $position, $source);
    $position += $ellipsis->width();
    return new VariadicParameter(
        $ellipsis);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_ellipsis;
    yield break;
  }
}
final class AttributeSpecification extends EditableSyntax {
  private EditableSyntax $_left_double_angle;
  private EditableSyntax $_attributes;
  private EditableSyntax $_right_double_angle;
  public function __construct(
    EditableSyntax $left_double_angle,
    EditableSyntax $attributes,
    EditableSyntax $right_double_angle) {
    parent::__construct('attribute_specification');
    $this->_left_double_angle = $left_double_angle;
    $this->_attributes = $attributes;
    $this->_right_double_angle = $right_double_angle;
  }
  public function left_double_angle(): EditableSyntax {
    return $this->_left_double_angle;
  }
  public function attributes(): EditableSyntax {
    return $this->_attributes;
  }
  public function right_double_angle(): EditableSyntax {
    return $this->_right_double_angle;
  }
  public function with_left_double_angle(EditableSyntax $left_double_angle): AttributeSpecification {
    return new AttributeSpecification(
      $left_double_angle,
      $this->_attributes,
      $this->_right_double_angle);
  }
  public function with_attributes(EditableSyntax $attributes): AttributeSpecification {
    return new AttributeSpecification(
      $this->_left_double_angle,
      $attributes,
      $this->_right_double_angle);
  }
  public function with_right_double_angle(EditableSyntax $right_double_angle): AttributeSpecification {
    return new AttributeSpecification(
      $this->_left_double_angle,
      $this->_attributes,
      $right_double_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_double_angle = $this->left_double_angle()->rewrite($rewriter, $new_parents);
    $attributes = $this->attributes()->rewrite($rewriter, $new_parents);
    $right_double_angle = $this->right_double_angle()->rewrite($rewriter, $new_parents);
    if (
      $left_double_angle === $this->left_double_angle() &&
      $attributes === $this->attributes() &&
      $right_double_angle === $this->right_double_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new AttributeSpecification(
        $left_double_angle,
        $attributes,
        $right_double_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_double_angle = EditableSyntax::from_json(
      $json->attribute_specification_left_double_angle, $position, $source);
    $position += $left_double_angle->width();
    $attributes = EditableSyntax::from_json(
      $json->attribute_specification_attributes, $position, $source);
    $position += $attributes->width();
    $right_double_angle = EditableSyntax::from_json(
      $json->attribute_specification_right_double_angle, $position, $source);
    $position += $right_double_angle->width();
    return new AttributeSpecification(
        $left_double_angle,
        $attributes,
        $right_double_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_double_angle;
    yield $this->_attributes;
    yield $this->_right_double_angle;
    yield break;
  }
}
final class Attribute extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_values;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $left_paren,
    EditableSyntax $values,
    EditableSyntax $right_paren) {
    parent::__construct('attribute');
    $this->_name = $name;
    $this->_left_paren = $left_paren;
    $this->_values = $values;
    $this->_right_paren = $right_paren;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function values(): EditableSyntax {
    return $this->_values;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_name(EditableSyntax $name): Attribute {
    return new Attribute(
      $name,
      $this->_left_paren,
      $this->_values,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): Attribute {
    return new Attribute(
      $this->_name,
      $left_paren,
      $this->_values,
      $this->_right_paren);
  }
  public function with_values(EditableSyntax $values): Attribute {
    return new Attribute(
      $this->_name,
      $this->_left_paren,
      $values,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): Attribute {
    return new Attribute(
      $this->_name,
      $this->_left_paren,
      $this->_values,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $values = $this->values()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $left_paren === $this->left_paren() &&
      $values === $this->values() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new Attribute(
        $name,
        $left_paren,
        $values,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->attribute_name, $position, $source);
    $position += $name->width();
    $left_paren = EditableSyntax::from_json(
      $json->attribute_left_paren, $position, $source);
    $position += $left_paren->width();
    $values = EditableSyntax::from_json(
      $json->attribute_values, $position, $source);
    $position += $values->width();
    $right_paren = EditableSyntax::from_json(
      $json->attribute_right_paren, $position, $source);
    $position += $right_paren->width();
    return new Attribute(
        $name,
        $left_paren,
        $values,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_left_paren;
    yield $this->_values;
    yield $this->_right_paren;
    yield break;
  }
}
final class InclusionExpression extends EditableSyntax {
  private EditableSyntax $_require;
  private EditableSyntax $_filename;
  public function __construct(
    EditableSyntax $require,
    EditableSyntax $filename) {
    parent::__construct('inclusion_expression');
    $this->_require = $require;
    $this->_filename = $filename;
  }
  public function require(): EditableSyntax {
    return $this->_require;
  }
  public function filename(): EditableSyntax {
    return $this->_filename;
  }
  public function with_require(EditableSyntax $require): InclusionExpression {
    return new InclusionExpression(
      $require,
      $this->_filename);
  }
  public function with_filename(EditableSyntax $filename): InclusionExpression {
    return new InclusionExpression(
      $this->_require,
      $filename);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $require = $this->require()->rewrite($rewriter, $new_parents);
    $filename = $this->filename()->rewrite($rewriter, $new_parents);
    if (
      $require === $this->require() &&
      $filename === $this->filename()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new InclusionExpression(
        $require,
        $filename), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $require = EditableSyntax::from_json(
      $json->inclusion_require, $position, $source);
    $position += $require->width();
    $filename = EditableSyntax::from_json(
      $json->inclusion_filename, $position, $source);
    $position += $filename->width();
    return new InclusionExpression(
        $require,
        $filename);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_require;
    yield $this->_filename;
    yield break;
  }
}
final class InclusionDirective extends EditableSyntax {
  private EditableSyntax $_expression;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $expression,
    EditableSyntax $semicolon) {
    parent::__construct('inclusion_directive');
    $this->_expression = $expression;
    $this->_semicolon = $semicolon;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_expression(EditableSyntax $expression): InclusionDirective {
    return new InclusionDirective(
      $expression,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): InclusionDirective {
    return new InclusionDirective(
      $this->_expression,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new InclusionDirective(
        $expression,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->inclusion_expression, $position, $source);
    $position += $expression->width();
    $semicolon = EditableSyntax::from_json(
      $json->inclusion_semicolon, $position, $source);
    $position += $semicolon->width();
    return new InclusionDirective(
        $expression,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield $this->_semicolon;
    yield break;
  }
}
final class CompoundStatement extends EditableSyntax {
  private EditableSyntax $_left_brace;
  private EditableSyntax $_statements;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $left_brace,
    EditableSyntax $statements,
    EditableSyntax $right_brace) {
    parent::__construct('compound_statement');
    $this->_left_brace = $left_brace;
    $this->_statements = $statements;
    $this->_right_brace = $right_brace;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function statements(): EditableSyntax {
    return $this->_statements;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_left_brace(EditableSyntax $left_brace): CompoundStatement {
    return new CompoundStatement(
      $left_brace,
      $this->_statements,
      $this->_right_brace);
  }
  public function with_statements(EditableSyntax $statements): CompoundStatement {
    return new CompoundStatement(
      $this->_left_brace,
      $statements,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): CompoundStatement {
    return new CompoundStatement(
      $this->_left_brace,
      $this->_statements,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $statements = $this->statements()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $left_brace === $this->left_brace() &&
      $statements === $this->statements() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new CompoundStatement(
        $left_brace,
        $statements,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_brace = EditableSyntax::from_json(
      $json->compound_left_brace, $position, $source);
    $position += $left_brace->width();
    $statements = EditableSyntax::from_json(
      $json->compound_statements, $position, $source);
    $position += $statements->width();
    $right_brace = EditableSyntax::from_json(
      $json->compound_right_brace, $position, $source);
    $position += $right_brace->width();
    return new CompoundStatement(
        $left_brace,
        $statements,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_brace;
    yield $this->_statements;
    yield $this->_right_brace;
    yield break;
  }
}
final class ExpressionStatement extends EditableSyntax {
  private EditableSyntax $_expression;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $expression,
    EditableSyntax $semicolon) {
    parent::__construct('expression_statement');
    $this->_expression = $expression;
    $this->_semicolon = $semicolon;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_expression(EditableSyntax $expression): ExpressionStatement {
    return new ExpressionStatement(
      $expression,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): ExpressionStatement {
    return new ExpressionStatement(
      $this->_expression,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $expression === $this->expression() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ExpressionStatement(
        $expression,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $expression = EditableSyntax::from_json(
      $json->expression_statement_expression, $position, $source);
    $position += $expression->width();
    $semicolon = EditableSyntax::from_json(
      $json->expression_statement_semicolon, $position, $source);
    $position += $semicolon->width();
    return new ExpressionStatement(
        $expression,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_expression;
    yield $this->_semicolon;
    yield break;
  }
}
final class MarkupSection extends EditableSyntax {
  private EditableSyntax $_prefix;
  private EditableSyntax $_text;
  private EditableSyntax $_suffix;
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $prefix,
    EditableSyntax $text,
    EditableSyntax $suffix,
    EditableSyntax $expression) {
    parent::__construct('markup_section');
    $this->_prefix = $prefix;
    $this->_text = $text;
    $this->_suffix = $suffix;
    $this->_expression = $expression;
  }
  public function prefix(): EditableSyntax {
    return $this->_prefix;
  }
  public function text(): EditableSyntax {
    return $this->_text;
  }
  public function suffix(): EditableSyntax {
    return $this->_suffix;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_prefix(EditableSyntax $prefix): MarkupSection {
    return new MarkupSection(
      $prefix,
      $this->_text,
      $this->_suffix,
      $this->_expression);
  }
  public function with_text(EditableSyntax $text): MarkupSection {
    return new MarkupSection(
      $this->_prefix,
      $text,
      $this->_suffix,
      $this->_expression);
  }
  public function with_suffix(EditableSyntax $suffix): MarkupSection {
    return new MarkupSection(
      $this->_prefix,
      $this->_text,
      $suffix,
      $this->_expression);
  }
  public function with_expression(EditableSyntax $expression): MarkupSection {
    return new MarkupSection(
      $this->_prefix,
      $this->_text,
      $this->_suffix,
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $prefix = $this->prefix()->rewrite($rewriter, $new_parents);
    $text = $this->text()->rewrite($rewriter, $new_parents);
    $suffix = $this->suffix()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $prefix === $this->prefix() &&
      $text === $this->text() &&
      $suffix === $this->suffix() &&
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new MarkupSection(
        $prefix,
        $text,
        $suffix,
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $prefix = EditableSyntax::from_json(
      $json->markup_prefix, $position, $source);
    $position += $prefix->width();
    $text = EditableSyntax::from_json(
      $json->markup_text, $position, $source);
    $position += $text->width();
    $suffix = EditableSyntax::from_json(
      $json->markup_suffix, $position, $source);
    $position += $suffix->width();
    $expression = EditableSyntax::from_json(
      $json->markup_expression, $position, $source);
    $position += $expression->width();
    return new MarkupSection(
        $prefix,
        $text,
        $suffix,
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_prefix;
    yield $this->_text;
    yield $this->_suffix;
    yield $this->_expression;
    yield break;
  }
}
final class MarkupSuffix extends EditableSyntax {
  private EditableSyntax $_less_than_question;
  private EditableSyntax $_name;
  public function __construct(
    EditableSyntax $less_than_question,
    EditableSyntax $name) {
    parent::__construct('markup_suffix');
    $this->_less_than_question = $less_than_question;
    $this->_name = $name;
  }
  public function less_than_question(): EditableSyntax {
    return $this->_less_than_question;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function with_less_than_question(EditableSyntax $less_than_question): MarkupSuffix {
    return new MarkupSuffix(
      $less_than_question,
      $this->_name);
  }
  public function with_name(EditableSyntax $name): MarkupSuffix {
    return new MarkupSuffix(
      $this->_less_than_question,
      $name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $less_than_question = $this->less_than_question()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    if (
      $less_than_question === $this->less_than_question() &&
      $name === $this->name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new MarkupSuffix(
        $less_than_question,
        $name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $less_than_question = EditableSyntax::from_json(
      $json->markup_suffix_less_than_question, $position, $source);
    $position += $less_than_question->width();
    $name = EditableSyntax::from_json(
      $json->markup_suffix_name, $position, $source);
    $position += $name->width();
    return new MarkupSuffix(
        $less_than_question,
        $name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_less_than_question;
    yield $this->_name;
    yield break;
  }
}
final class UnsetStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_variables;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $variables,
    EditableSyntax $right_paren,
    EditableSyntax $semicolon) {
    parent::__construct('unset_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_variables = $variables;
    $this->_right_paren = $right_paren;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function variables(): EditableSyntax {
    return $this->_variables;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): UnsetStatement {
    return new UnsetStatement(
      $keyword,
      $this->_left_paren,
      $this->_variables,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_left_paren(EditableSyntax $left_paren): UnsetStatement {
    return new UnsetStatement(
      $this->_keyword,
      $left_paren,
      $this->_variables,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_variables(EditableSyntax $variables): UnsetStatement {
    return new UnsetStatement(
      $this->_keyword,
      $this->_left_paren,
      $variables,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_right_paren(EditableSyntax $right_paren): UnsetStatement {
    return new UnsetStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_variables,
      $right_paren,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): UnsetStatement {
    return new UnsetStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_variables,
      $this->_right_paren,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $variables = $this->variables()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $variables === $this->variables() &&
      $right_paren === $this->right_paren() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new UnsetStatement(
        $keyword,
        $left_paren,
        $variables,
        $right_paren,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->unset_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->unset_left_paren, $position, $source);
    $position += $left_paren->width();
    $variables = EditableSyntax::from_json(
      $json->unset_variables, $position, $source);
    $position += $variables->width();
    $right_paren = EditableSyntax::from_json(
      $json->unset_right_paren, $position, $source);
    $position += $right_paren->width();
    $semicolon = EditableSyntax::from_json(
      $json->unset_semicolon, $position, $source);
    $position += $semicolon->width();
    return new UnsetStatement(
        $keyword,
        $left_paren,
        $variables,
        $right_paren,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_variables;
    yield $this->_right_paren;
    yield $this->_semicolon;
    yield break;
  }
}
final class WhileStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_condition;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $condition,
    EditableSyntax $right_paren,
    EditableSyntax $body) {
    parent::__construct('while_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_condition = $condition;
    $this->_right_paren = $right_paren;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function condition(): EditableSyntax {
    return $this->_condition;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): WhileStatement {
    return new WhileStatement(
      $keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_body);
  }
  public function with_left_paren(EditableSyntax $left_paren): WhileStatement {
    return new WhileStatement(
      $this->_keyword,
      $left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_body);
  }
  public function with_condition(EditableSyntax $condition): WhileStatement {
    return new WhileStatement(
      $this->_keyword,
      $this->_left_paren,
      $condition,
      $this->_right_paren,
      $this->_body);
  }
  public function with_right_paren(EditableSyntax $right_paren): WhileStatement {
    return new WhileStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $right_paren,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): WhileStatement {
    return new WhileStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $condition = $this->condition()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $condition === $this->condition() &&
      $right_paren === $this->right_paren() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new WhileStatement(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->while_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->while_left_paren, $position, $source);
    $position += $left_paren->width();
    $condition = EditableSyntax::from_json(
      $json->while_condition, $position, $source);
    $position += $condition->width();
    $right_paren = EditableSyntax::from_json(
      $json->while_right_paren, $position, $source);
    $position += $right_paren->width();
    $body = EditableSyntax::from_json(
      $json->while_body, $position, $source);
    $position += $body->width();
    return new WhileStatement(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_condition;
    yield $this->_right_paren;
    yield $this->_body;
    yield break;
  }
}
final class IfStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_condition;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_statement;
  private EditableSyntax $_elseif_clauses;
  private EditableSyntax $_else_clause;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $condition,
    EditableSyntax $right_paren,
    EditableSyntax $statement,
    EditableSyntax $elseif_clauses,
    EditableSyntax $else_clause) {
    parent::__construct('if_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_condition = $condition;
    $this->_right_paren = $right_paren;
    $this->_statement = $statement;
    $this->_elseif_clauses = $elseif_clauses;
    $this->_else_clause = $else_clause;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function condition(): EditableSyntax {
    return $this->_condition;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function statement(): EditableSyntax {
    return $this->_statement;
  }
  public function elseif_clauses(): EditableSyntax {
    return $this->_elseif_clauses;
  }
  public function else_clause(): EditableSyntax {
    return $this->_else_clause;
  }
  public function with_keyword(EditableSyntax $keyword): IfStatement {
    return new IfStatement(
      $keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement,
      $this->_elseif_clauses,
      $this->_else_clause);
  }
  public function with_left_paren(EditableSyntax $left_paren): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement,
      $this->_elseif_clauses,
      $this->_else_clause);
  }
  public function with_condition(EditableSyntax $condition): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $this->_left_paren,
      $condition,
      $this->_right_paren,
      $this->_statement,
      $this->_elseif_clauses,
      $this->_else_clause);
  }
  public function with_right_paren(EditableSyntax $right_paren): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $right_paren,
      $this->_statement,
      $this->_elseif_clauses,
      $this->_else_clause);
  }
  public function with_statement(EditableSyntax $statement): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $statement,
      $this->_elseif_clauses,
      $this->_else_clause);
  }
  public function with_elseif_clauses(EditableSyntax $elseif_clauses): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement,
      $elseif_clauses,
      $this->_else_clause);
  }
  public function with_else_clause(EditableSyntax $else_clause): IfStatement {
    return new IfStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement,
      $this->_elseif_clauses,
      $else_clause);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $condition = $this->condition()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $statement = $this->statement()->rewrite($rewriter, $new_parents);
    $elseif_clauses = $this->elseif_clauses()->rewrite($rewriter, $new_parents);
    $else_clause = $this->else_clause()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $condition === $this->condition() &&
      $right_paren === $this->right_paren() &&
      $statement === $this->statement() &&
      $elseif_clauses === $this->elseif_clauses() &&
      $else_clause === $this->else_clause()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new IfStatement(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $statement,
        $elseif_clauses,
        $else_clause), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->if_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->if_left_paren, $position, $source);
    $position += $left_paren->width();
    $condition = EditableSyntax::from_json(
      $json->if_condition, $position, $source);
    $position += $condition->width();
    $right_paren = EditableSyntax::from_json(
      $json->if_right_paren, $position, $source);
    $position += $right_paren->width();
    $statement = EditableSyntax::from_json(
      $json->if_statement, $position, $source);
    $position += $statement->width();
    $elseif_clauses = EditableSyntax::from_json(
      $json->if_elseif_clauses, $position, $source);
    $position += $elseif_clauses->width();
    $else_clause = EditableSyntax::from_json(
      $json->if_else_clause, $position, $source);
    $position += $else_clause->width();
    return new IfStatement(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $statement,
        $elseif_clauses,
        $else_clause);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_condition;
    yield $this->_right_paren;
    yield $this->_statement;
    yield $this->_elseif_clauses;
    yield $this->_else_clause;
    yield break;
  }
}
final class ElseifClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_condition;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_statement;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $condition,
    EditableSyntax $right_paren,
    EditableSyntax $statement) {
    parent::__construct('elseif_clause');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_condition = $condition;
    $this->_right_paren = $right_paren;
    $this->_statement = $statement;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function condition(): EditableSyntax {
    return $this->_condition;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function statement(): EditableSyntax {
    return $this->_statement;
  }
  public function with_keyword(EditableSyntax $keyword): ElseifClause {
    return new ElseifClause(
      $keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement);
  }
  public function with_left_paren(EditableSyntax $left_paren): ElseifClause {
    return new ElseifClause(
      $this->_keyword,
      $left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_statement);
  }
  public function with_condition(EditableSyntax $condition): ElseifClause {
    return new ElseifClause(
      $this->_keyword,
      $this->_left_paren,
      $condition,
      $this->_right_paren,
      $this->_statement);
  }
  public function with_right_paren(EditableSyntax $right_paren): ElseifClause {
    return new ElseifClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $right_paren,
      $this->_statement);
  }
  public function with_statement(EditableSyntax $statement): ElseifClause {
    return new ElseifClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $statement);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $condition = $this->condition()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $statement = $this->statement()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $condition === $this->condition() &&
      $right_paren === $this->right_paren() &&
      $statement === $this->statement()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ElseifClause(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $statement), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->elseif_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->elseif_left_paren, $position, $source);
    $position += $left_paren->width();
    $condition = EditableSyntax::from_json(
      $json->elseif_condition, $position, $source);
    $position += $condition->width();
    $right_paren = EditableSyntax::from_json(
      $json->elseif_right_paren, $position, $source);
    $position += $right_paren->width();
    $statement = EditableSyntax::from_json(
      $json->elseif_statement, $position, $source);
    $position += $statement->width();
    return new ElseifClause(
        $keyword,
        $left_paren,
        $condition,
        $right_paren,
        $statement);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_condition;
    yield $this->_right_paren;
    yield $this->_statement;
    yield break;
  }
}
final class ElseClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_statement;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $statement) {
    parent::__construct('else_clause');
    $this->_keyword = $keyword;
    $this->_statement = $statement;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function statement(): EditableSyntax {
    return $this->_statement;
  }
  public function with_keyword(EditableSyntax $keyword): ElseClause {
    return new ElseClause(
      $keyword,
      $this->_statement);
  }
  public function with_statement(EditableSyntax $statement): ElseClause {
    return new ElseClause(
      $this->_keyword,
      $statement);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $statement = $this->statement()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $statement === $this->statement()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ElseClause(
        $keyword,
        $statement), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->else_keyword, $position, $source);
    $position += $keyword->width();
    $statement = EditableSyntax::from_json(
      $json->else_statement, $position, $source);
    $position += $statement->width();
    return new ElseClause(
        $keyword,
        $statement);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_statement;
    yield break;
  }
}
final class TryStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_compound_statement;
  private EditableSyntax $_catch_clauses;
  private EditableSyntax $_finally_clause;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $compound_statement,
    EditableSyntax $catch_clauses,
    EditableSyntax $finally_clause) {
    parent::__construct('try_statement');
    $this->_keyword = $keyword;
    $this->_compound_statement = $compound_statement;
    $this->_catch_clauses = $catch_clauses;
    $this->_finally_clause = $finally_clause;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function compound_statement(): EditableSyntax {
    return $this->_compound_statement;
  }
  public function catch_clauses(): EditableSyntax {
    return $this->_catch_clauses;
  }
  public function finally_clause(): EditableSyntax {
    return $this->_finally_clause;
  }
  public function with_keyword(EditableSyntax $keyword): TryStatement {
    return new TryStatement(
      $keyword,
      $this->_compound_statement,
      $this->_catch_clauses,
      $this->_finally_clause);
  }
  public function with_compound_statement(EditableSyntax $compound_statement): TryStatement {
    return new TryStatement(
      $this->_keyword,
      $compound_statement,
      $this->_catch_clauses,
      $this->_finally_clause);
  }
  public function with_catch_clauses(EditableSyntax $catch_clauses): TryStatement {
    return new TryStatement(
      $this->_keyword,
      $this->_compound_statement,
      $catch_clauses,
      $this->_finally_clause);
  }
  public function with_finally_clause(EditableSyntax $finally_clause): TryStatement {
    return new TryStatement(
      $this->_keyword,
      $this->_compound_statement,
      $this->_catch_clauses,
      $finally_clause);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $compound_statement = $this->compound_statement()->rewrite($rewriter, $new_parents);
    $catch_clauses = $this->catch_clauses()->rewrite($rewriter, $new_parents);
    $finally_clause = $this->finally_clause()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $compound_statement === $this->compound_statement() &&
      $catch_clauses === $this->catch_clauses() &&
      $finally_clause === $this->finally_clause()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TryStatement(
        $keyword,
        $compound_statement,
        $catch_clauses,
        $finally_clause), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->try_keyword, $position, $source);
    $position += $keyword->width();
    $compound_statement = EditableSyntax::from_json(
      $json->try_compound_statement, $position, $source);
    $position += $compound_statement->width();
    $catch_clauses = EditableSyntax::from_json(
      $json->try_catch_clauses, $position, $source);
    $position += $catch_clauses->width();
    $finally_clause = EditableSyntax::from_json(
      $json->try_finally_clause, $position, $source);
    $position += $finally_clause->width();
    return new TryStatement(
        $keyword,
        $compound_statement,
        $catch_clauses,
        $finally_clause);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_compound_statement;
    yield $this->_catch_clauses;
    yield $this->_finally_clause;
    yield break;
  }
}
final class CatchClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_type;
  private EditableSyntax $_variable;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $type,
    EditableSyntax $variable,
    EditableSyntax $right_paren,
    EditableSyntax $body) {
    parent::__construct('catch_clause');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_type = $type;
    $this->_variable = $variable;
    $this->_right_paren = $right_paren;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function variable(): EditableSyntax {
    return $this->_variable;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): CatchClause {
    return new CatchClause(
      $keyword,
      $this->_left_paren,
      $this->_type,
      $this->_variable,
      $this->_right_paren,
      $this->_body);
  }
  public function with_left_paren(EditableSyntax $left_paren): CatchClause {
    return new CatchClause(
      $this->_keyword,
      $left_paren,
      $this->_type,
      $this->_variable,
      $this->_right_paren,
      $this->_body);
  }
  public function with_type(EditableSyntax $type): CatchClause {
    return new CatchClause(
      $this->_keyword,
      $this->_left_paren,
      $type,
      $this->_variable,
      $this->_right_paren,
      $this->_body);
  }
  public function with_variable(EditableSyntax $variable): CatchClause {
    return new CatchClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_type,
      $variable,
      $this->_right_paren,
      $this->_body);
  }
  public function with_right_paren(EditableSyntax $right_paren): CatchClause {
    return new CatchClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_type,
      $this->_variable,
      $right_paren,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): CatchClause {
    return new CatchClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_type,
      $this->_variable,
      $this->_right_paren,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $variable = $this->variable()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $type === $this->type() &&
      $variable === $this->variable() &&
      $right_paren === $this->right_paren() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new CatchClause(
        $keyword,
        $left_paren,
        $type,
        $variable,
        $right_paren,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->catch_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->catch_left_paren, $position, $source);
    $position += $left_paren->width();
    $type = EditableSyntax::from_json(
      $json->catch_type, $position, $source);
    $position += $type->width();
    $variable = EditableSyntax::from_json(
      $json->catch_variable, $position, $source);
    $position += $variable->width();
    $right_paren = EditableSyntax::from_json(
      $json->catch_right_paren, $position, $source);
    $position += $right_paren->width();
    $body = EditableSyntax::from_json(
      $json->catch_body, $position, $source);
    $position += $body->width();
    return new CatchClause(
        $keyword,
        $left_paren,
        $type,
        $variable,
        $right_paren,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_type;
    yield $this->_variable;
    yield $this->_right_paren;
    yield $this->_body;
    yield break;
  }
}
final class FinallyClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $body) {
    parent::__construct('finally_clause');
    $this->_keyword = $keyword;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): FinallyClause {
    return new FinallyClause(
      $keyword,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): FinallyClause {
    return new FinallyClause(
      $this->_keyword,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FinallyClause(
        $keyword,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->finally_keyword, $position, $source);
    $position += $keyword->width();
    $body = EditableSyntax::from_json(
      $json->finally_body, $position, $source);
    $position += $body->width();
    return new FinallyClause(
        $keyword,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_body;
    yield break;
  }
}
final class DoStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_body;
  private EditableSyntax $_while_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_condition;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $body,
    EditableSyntax $while_keyword,
    EditableSyntax $left_paren,
    EditableSyntax $condition,
    EditableSyntax $right_paren,
    EditableSyntax $semicolon) {
    parent::__construct('do_statement');
    $this->_keyword = $keyword;
    $this->_body = $body;
    $this->_while_keyword = $while_keyword;
    $this->_left_paren = $left_paren;
    $this->_condition = $condition;
    $this->_right_paren = $right_paren;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function while_keyword(): EditableSyntax {
    return $this->_while_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function condition(): EditableSyntax {
    return $this->_condition;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): DoStatement {
    return new DoStatement(
      $keyword,
      $this->_body,
      $this->_while_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_body(EditableSyntax $body): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $body,
      $this->_while_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_while_keyword(EditableSyntax $while_keyword): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $this->_body,
      $while_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_left_paren(EditableSyntax $left_paren): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $this->_body,
      $this->_while_keyword,
      $left_paren,
      $this->_condition,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_condition(EditableSyntax $condition): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $this->_body,
      $this->_while_keyword,
      $this->_left_paren,
      $condition,
      $this->_right_paren,
      $this->_semicolon);
  }
  public function with_right_paren(EditableSyntax $right_paren): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $this->_body,
      $this->_while_keyword,
      $this->_left_paren,
      $this->_condition,
      $right_paren,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): DoStatement {
    return new DoStatement(
      $this->_keyword,
      $this->_body,
      $this->_while_keyword,
      $this->_left_paren,
      $this->_condition,
      $this->_right_paren,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    $while_keyword = $this->while_keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $condition = $this->condition()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $body === $this->body() &&
      $while_keyword === $this->while_keyword() &&
      $left_paren === $this->left_paren() &&
      $condition === $this->condition() &&
      $right_paren === $this->right_paren() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DoStatement(
        $keyword,
        $body,
        $while_keyword,
        $left_paren,
        $condition,
        $right_paren,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->do_keyword, $position, $source);
    $position += $keyword->width();
    $body = EditableSyntax::from_json(
      $json->do_body, $position, $source);
    $position += $body->width();
    $while_keyword = EditableSyntax::from_json(
      $json->do_while_keyword, $position, $source);
    $position += $while_keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->do_left_paren, $position, $source);
    $position += $left_paren->width();
    $condition = EditableSyntax::from_json(
      $json->do_condition, $position, $source);
    $position += $condition->width();
    $right_paren = EditableSyntax::from_json(
      $json->do_right_paren, $position, $source);
    $position += $right_paren->width();
    $semicolon = EditableSyntax::from_json(
      $json->do_semicolon, $position, $source);
    $position += $semicolon->width();
    return new DoStatement(
        $keyword,
        $body,
        $while_keyword,
        $left_paren,
        $condition,
        $right_paren,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_body;
    yield $this->_while_keyword;
    yield $this->_left_paren;
    yield $this->_condition;
    yield $this->_right_paren;
    yield $this->_semicolon;
    yield break;
  }
}
final class ForStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_initializer;
  private EditableSyntax $_first_semicolon;
  private EditableSyntax $_control;
  private EditableSyntax $_second_semicolon;
  private EditableSyntax $_end_of_loop;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $initializer,
    EditableSyntax $first_semicolon,
    EditableSyntax $control,
    EditableSyntax $second_semicolon,
    EditableSyntax $end_of_loop,
    EditableSyntax $right_paren,
    EditableSyntax $body) {
    parent::__construct('for_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_initializer = $initializer;
    $this->_first_semicolon = $first_semicolon;
    $this->_control = $control;
    $this->_second_semicolon = $second_semicolon;
    $this->_end_of_loop = $end_of_loop;
    $this->_right_paren = $right_paren;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function initializer(): EditableSyntax {
    return $this->_initializer;
  }
  public function first_semicolon(): EditableSyntax {
    return $this->_first_semicolon;
  }
  public function control(): EditableSyntax {
    return $this->_control;
  }
  public function second_semicolon(): EditableSyntax {
    return $this->_second_semicolon;
  }
  public function end_of_loop(): EditableSyntax {
    return $this->_end_of_loop;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): ForStatement {
    return new ForStatement(
      $keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_left_paren(EditableSyntax $left_paren): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_initializer(EditableSyntax $initializer): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_first_semicolon(EditableSyntax $first_semicolon): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_control(EditableSyntax $control): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_second_semicolon(EditableSyntax $second_semicolon): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_end_of_loop(EditableSyntax $end_of_loop): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $end_of_loop,
      $this->_right_paren,
      $this->_body);
  }
  public function with_right_paren(EditableSyntax $right_paren): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $right_paren,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): ForStatement {
    return new ForStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_initializer,
      $this->_first_semicolon,
      $this->_control,
      $this->_second_semicolon,
      $this->_end_of_loop,
      $this->_right_paren,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $initializer = $this->initializer()->rewrite($rewriter, $new_parents);
    $first_semicolon = $this->first_semicolon()->rewrite($rewriter, $new_parents);
    $control = $this->control()->rewrite($rewriter, $new_parents);
    $second_semicolon = $this->second_semicolon()->rewrite($rewriter, $new_parents);
    $end_of_loop = $this->end_of_loop()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $initializer === $this->initializer() &&
      $first_semicolon === $this->first_semicolon() &&
      $control === $this->control() &&
      $second_semicolon === $this->second_semicolon() &&
      $end_of_loop === $this->end_of_loop() &&
      $right_paren === $this->right_paren() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ForStatement(
        $keyword,
        $left_paren,
        $initializer,
        $first_semicolon,
        $control,
        $second_semicolon,
        $end_of_loop,
        $right_paren,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->for_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->for_left_paren, $position, $source);
    $position += $left_paren->width();
    $initializer = EditableSyntax::from_json(
      $json->for_initializer, $position, $source);
    $position += $initializer->width();
    $first_semicolon = EditableSyntax::from_json(
      $json->for_first_semicolon, $position, $source);
    $position += $first_semicolon->width();
    $control = EditableSyntax::from_json(
      $json->for_control, $position, $source);
    $position += $control->width();
    $second_semicolon = EditableSyntax::from_json(
      $json->for_second_semicolon, $position, $source);
    $position += $second_semicolon->width();
    $end_of_loop = EditableSyntax::from_json(
      $json->for_end_of_loop, $position, $source);
    $position += $end_of_loop->width();
    $right_paren = EditableSyntax::from_json(
      $json->for_right_paren, $position, $source);
    $position += $right_paren->width();
    $body = EditableSyntax::from_json(
      $json->for_body, $position, $source);
    $position += $body->width();
    return new ForStatement(
        $keyword,
        $left_paren,
        $initializer,
        $first_semicolon,
        $control,
        $second_semicolon,
        $end_of_loop,
        $right_paren,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_initializer;
    yield $this->_first_semicolon;
    yield $this->_control;
    yield $this->_second_semicolon;
    yield $this->_end_of_loop;
    yield $this->_right_paren;
    yield $this->_body;
    yield break;
  }
}
final class ForeachStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_collection;
  private EditableSyntax $_await_keyword;
  private EditableSyntax $_as;
  private EditableSyntax $_key;
  private EditableSyntax $_arrow;
  private EditableSyntax $_value;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $collection,
    EditableSyntax $await_keyword,
    EditableSyntax $as,
    EditableSyntax $key,
    EditableSyntax $arrow,
    EditableSyntax $value,
    EditableSyntax $right_paren,
    EditableSyntax $body) {
    parent::__construct('foreach_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_collection = $collection;
    $this->_await_keyword = $await_keyword;
    $this->_as = $as;
    $this->_key = $key;
    $this->_arrow = $arrow;
    $this->_value = $value;
    $this->_right_paren = $right_paren;
    $this->_body = $body;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function collection(): EditableSyntax {
    return $this->_collection;
  }
  public function await_keyword(): EditableSyntax {
    return $this->_await_keyword;
  }
  public function as(): EditableSyntax {
    return $this->_as;
  }
  public function key(): EditableSyntax {
    return $this->_key;
  }
  public function arrow(): EditableSyntax {
    return $this->_arrow;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_keyword(EditableSyntax $keyword): ForeachStatement {
    return new ForeachStatement(
      $keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_left_paren(EditableSyntax $left_paren): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_collection(EditableSyntax $collection): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_await_keyword(EditableSyntax $await_keyword): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_as(EditableSyntax $as): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_key(EditableSyntax $key): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_arrow(EditableSyntax $arrow): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $arrow,
      $this->_value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_value(EditableSyntax $value): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $value,
      $this->_right_paren,
      $this->_body);
  }
  public function with_right_paren(EditableSyntax $right_paren): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $right_paren,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): ForeachStatement {
    return new ForeachStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_collection,
      $this->_await_keyword,
      $this->_as,
      $this->_key,
      $this->_arrow,
      $this->_value,
      $this->_right_paren,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $collection = $this->collection()->rewrite($rewriter, $new_parents);
    $await_keyword = $this->await_keyword()->rewrite($rewriter, $new_parents);
    $as = $this->as()->rewrite($rewriter, $new_parents);
    $key = $this->key()->rewrite($rewriter, $new_parents);
    $arrow = $this->arrow()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $collection === $this->collection() &&
      $await_keyword === $this->await_keyword() &&
      $as === $this->as() &&
      $key === $this->key() &&
      $arrow === $this->arrow() &&
      $value === $this->value() &&
      $right_paren === $this->right_paren() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ForeachStatement(
        $keyword,
        $left_paren,
        $collection,
        $await_keyword,
        $as,
        $key,
        $arrow,
        $value,
        $right_paren,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->foreach_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->foreach_left_paren, $position, $source);
    $position += $left_paren->width();
    $collection = EditableSyntax::from_json(
      $json->foreach_collection, $position, $source);
    $position += $collection->width();
    $await_keyword = EditableSyntax::from_json(
      $json->foreach_await_keyword, $position, $source);
    $position += $await_keyword->width();
    $as = EditableSyntax::from_json(
      $json->foreach_as, $position, $source);
    $position += $as->width();
    $key = EditableSyntax::from_json(
      $json->foreach_key, $position, $source);
    $position += $key->width();
    $arrow = EditableSyntax::from_json(
      $json->foreach_arrow, $position, $source);
    $position += $arrow->width();
    $value = EditableSyntax::from_json(
      $json->foreach_value, $position, $source);
    $position += $value->width();
    $right_paren = EditableSyntax::from_json(
      $json->foreach_right_paren, $position, $source);
    $position += $right_paren->width();
    $body = EditableSyntax::from_json(
      $json->foreach_body, $position, $source);
    $position += $body->width();
    return new ForeachStatement(
        $keyword,
        $left_paren,
        $collection,
        $await_keyword,
        $as,
        $key,
        $arrow,
        $value,
        $right_paren,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_collection;
    yield $this->_await_keyword;
    yield $this->_as;
    yield $this->_key;
    yield $this->_arrow;
    yield $this->_value;
    yield $this->_right_paren;
    yield $this->_body;
    yield break;
  }
}
final class SwitchStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_expression;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_sections;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $expression,
    EditableSyntax $right_paren,
    EditableSyntax $left_brace,
    EditableSyntax $sections,
    EditableSyntax $right_brace) {
    parent::__construct('switch_statement');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_expression = $expression;
    $this->_right_paren = $right_paren;
    $this->_left_brace = $left_brace;
    $this->_sections = $sections;
    $this->_right_brace = $right_brace;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function sections(): EditableSyntax {
    return $this->_sections;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_keyword(EditableSyntax $keyword): SwitchStatement {
    return new SwitchStatement(
      $keyword,
      $this->_left_paren,
      $this->_expression,
      $this->_right_paren,
      $this->_left_brace,
      $this->_sections,
      $this->_right_brace);
  }
  public function with_left_paren(EditableSyntax $left_paren): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $left_paren,
      $this->_expression,
      $this->_right_paren,
      $this->_left_brace,
      $this->_sections,
      $this->_right_brace);
  }
  public function with_expression(EditableSyntax $expression): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $this->_left_paren,
      $expression,
      $this->_right_paren,
      $this->_left_brace,
      $this->_sections,
      $this->_right_brace);
  }
  public function with_right_paren(EditableSyntax $right_paren): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_expression,
      $right_paren,
      $this->_left_brace,
      $this->_sections,
      $this->_right_brace);
  }
  public function with_left_brace(EditableSyntax $left_brace): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_expression,
      $this->_right_paren,
      $left_brace,
      $this->_sections,
      $this->_right_brace);
  }
  public function with_sections(EditableSyntax $sections): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_expression,
      $this->_right_paren,
      $this->_left_brace,
      $sections,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): SwitchStatement {
    return new SwitchStatement(
      $this->_keyword,
      $this->_left_paren,
      $this->_expression,
      $this->_right_paren,
      $this->_left_brace,
      $this->_sections,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $sections = $this->sections()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $expression === $this->expression() &&
      $right_paren === $this->right_paren() &&
      $left_brace === $this->left_brace() &&
      $sections === $this->sections() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SwitchStatement(
        $keyword,
        $left_paren,
        $expression,
        $right_paren,
        $left_brace,
        $sections,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->switch_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->switch_left_paren, $position, $source);
    $position += $left_paren->width();
    $expression = EditableSyntax::from_json(
      $json->switch_expression, $position, $source);
    $position += $expression->width();
    $right_paren = EditableSyntax::from_json(
      $json->switch_right_paren, $position, $source);
    $position += $right_paren->width();
    $left_brace = EditableSyntax::from_json(
      $json->switch_left_brace, $position, $source);
    $position += $left_brace->width();
    $sections = EditableSyntax::from_json(
      $json->switch_sections, $position, $source);
    $position += $sections->width();
    $right_brace = EditableSyntax::from_json(
      $json->switch_right_brace, $position, $source);
    $position += $right_brace->width();
    return new SwitchStatement(
        $keyword,
        $left_paren,
        $expression,
        $right_paren,
        $left_brace,
        $sections,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_expression;
    yield $this->_right_paren;
    yield $this->_left_brace;
    yield $this->_sections;
    yield $this->_right_brace;
    yield break;
  }
}
final class SwitchSection extends EditableSyntax {
  private EditableSyntax $_labels;
  private EditableSyntax $_statements;
  private EditableSyntax $_fallthrough;
  public function __construct(
    EditableSyntax $labels,
    EditableSyntax $statements,
    EditableSyntax $fallthrough) {
    parent::__construct('switch_section');
    $this->_labels = $labels;
    $this->_statements = $statements;
    $this->_fallthrough = $fallthrough;
  }
  public function labels(): EditableSyntax {
    return $this->_labels;
  }
  public function statements(): EditableSyntax {
    return $this->_statements;
  }
  public function fallthrough(): EditableSyntax {
    return $this->_fallthrough;
  }
  public function with_labels(EditableSyntax $labels): SwitchSection {
    return new SwitchSection(
      $labels,
      $this->_statements,
      $this->_fallthrough);
  }
  public function with_statements(EditableSyntax $statements): SwitchSection {
    return new SwitchSection(
      $this->_labels,
      $statements,
      $this->_fallthrough);
  }
  public function with_fallthrough(EditableSyntax $fallthrough): SwitchSection {
    return new SwitchSection(
      $this->_labels,
      $this->_statements,
      $fallthrough);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $labels = $this->labels()->rewrite($rewriter, $new_parents);
    $statements = $this->statements()->rewrite($rewriter, $new_parents);
    $fallthrough = $this->fallthrough()->rewrite($rewriter, $new_parents);
    if (
      $labels === $this->labels() &&
      $statements === $this->statements() &&
      $fallthrough === $this->fallthrough()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SwitchSection(
        $labels,
        $statements,
        $fallthrough), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $labels = EditableSyntax::from_json(
      $json->switch_section_labels, $position, $source);
    $position += $labels->width();
    $statements = EditableSyntax::from_json(
      $json->switch_section_statements, $position, $source);
    $position += $statements->width();
    $fallthrough = EditableSyntax::from_json(
      $json->switch_section_fallthrough, $position, $source);
    $position += $fallthrough->width();
    return new SwitchSection(
        $labels,
        $statements,
        $fallthrough);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_labels;
    yield $this->_statements;
    yield $this->_fallthrough;
    yield break;
  }
}
final class SwitchFallthrough extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $semicolon) {
    parent::__construct('switch_fallthrough');
    $this->_keyword = $keyword;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): SwitchFallthrough {
    return new SwitchFallthrough(
      $keyword,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): SwitchFallthrough {
    return new SwitchFallthrough(
      $this->_keyword,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SwitchFallthrough(
        $keyword,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->fallthrough_keyword, $position, $source);
    $position += $keyword->width();
    $semicolon = EditableSyntax::from_json(
      $json->fallthrough_semicolon, $position, $source);
    $position += $semicolon->width();
    return new SwitchFallthrough(
        $keyword,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_semicolon;
    yield break;
  }
}
final class CaseLabel extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_expression;
  private EditableSyntax $_colon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $expression,
    EditableSyntax $colon) {
    parent::__construct('case_label');
    $this->_keyword = $keyword;
    $this->_expression = $expression;
    $this->_colon = $colon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function with_keyword(EditableSyntax $keyword): CaseLabel {
    return new CaseLabel(
      $keyword,
      $this->_expression,
      $this->_colon);
  }
  public function with_expression(EditableSyntax $expression): CaseLabel {
    return new CaseLabel(
      $this->_keyword,
      $expression,
      $this->_colon);
  }
  public function with_colon(EditableSyntax $colon): CaseLabel {
    return new CaseLabel(
      $this->_keyword,
      $this->_expression,
      $colon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $expression === $this->expression() &&
      $colon === $this->colon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new CaseLabel(
        $keyword,
        $expression,
        $colon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->case_keyword, $position, $source);
    $position += $keyword->width();
    $expression = EditableSyntax::from_json(
      $json->case_expression, $position, $source);
    $position += $expression->width();
    $colon = EditableSyntax::from_json(
      $json->case_colon, $position, $source);
    $position += $colon->width();
    return new CaseLabel(
        $keyword,
        $expression,
        $colon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_expression;
    yield $this->_colon;
    yield break;
  }
}
final class DefaultLabel extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_colon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $colon) {
    parent::__construct('default_label');
    $this->_keyword = $keyword;
    $this->_colon = $colon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function with_keyword(EditableSyntax $keyword): DefaultLabel {
    return new DefaultLabel(
      $keyword,
      $this->_colon);
  }
  public function with_colon(EditableSyntax $colon): DefaultLabel {
    return new DefaultLabel(
      $this->_keyword,
      $colon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $colon === $this->colon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DefaultLabel(
        $keyword,
        $colon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->default_keyword, $position, $source);
    $position += $keyword->width();
    $colon = EditableSyntax::from_json(
      $json->default_colon, $position, $source);
    $position += $colon->width();
    return new DefaultLabel(
        $keyword,
        $colon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_colon;
    yield break;
  }
}
final class ReturnStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_expression;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $expression,
    EditableSyntax $semicolon) {
    parent::__construct('return_statement');
    $this->_keyword = $keyword;
    $this->_expression = $expression;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): ReturnStatement {
    return new ReturnStatement(
      $keyword,
      $this->_expression,
      $this->_semicolon);
  }
  public function with_expression(EditableSyntax $expression): ReturnStatement {
    return new ReturnStatement(
      $this->_keyword,
      $expression,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): ReturnStatement {
    return new ReturnStatement(
      $this->_keyword,
      $this->_expression,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $expression === $this->expression() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ReturnStatement(
        $keyword,
        $expression,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->return_keyword, $position, $source);
    $position += $keyword->width();
    $expression = EditableSyntax::from_json(
      $json->return_expression, $position, $source);
    $position += $expression->width();
    $semicolon = EditableSyntax::from_json(
      $json->return_semicolon, $position, $source);
    $position += $semicolon->width();
    return new ReturnStatement(
        $keyword,
        $expression,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_expression;
    yield $this->_semicolon;
    yield break;
  }
}
final class GotoLabel extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_colon;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $colon) {
    parent::__construct('goto_label');
    $this->_name = $name;
    $this->_colon = $colon;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function with_name(EditableSyntax $name): GotoLabel {
    return new GotoLabel(
      $name,
      $this->_colon);
  }
  public function with_colon(EditableSyntax $colon): GotoLabel {
    return new GotoLabel(
      $this->_name,
      $colon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $colon === $this->colon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new GotoLabel(
        $name,
        $colon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->goto_label_name, $position, $source);
    $position += $name->width();
    $colon = EditableSyntax::from_json(
      $json->goto_label_colon, $position, $source);
    $position += $colon->width();
    return new GotoLabel(
        $name,
        $colon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_colon;
    yield break;
  }
}
final class GotoStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_label_name;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $label_name,
    EditableSyntax $semicolon) {
    parent::__construct('goto_statement');
    $this->_keyword = $keyword;
    $this->_label_name = $label_name;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function label_name(): EditableSyntax {
    return $this->_label_name;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): GotoStatement {
    return new GotoStatement(
      $keyword,
      $this->_label_name,
      $this->_semicolon);
  }
  public function with_label_name(EditableSyntax $label_name): GotoStatement {
    return new GotoStatement(
      $this->_keyword,
      $label_name,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): GotoStatement {
    return new GotoStatement(
      $this->_keyword,
      $this->_label_name,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $label_name = $this->label_name()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $label_name === $this->label_name() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new GotoStatement(
        $keyword,
        $label_name,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->goto_statement_keyword, $position, $source);
    $position += $keyword->width();
    $label_name = EditableSyntax::from_json(
      $json->goto_statement_label_name, $position, $source);
    $position += $label_name->width();
    $semicolon = EditableSyntax::from_json(
      $json->goto_statement_semicolon, $position, $source);
    $position += $semicolon->width();
    return new GotoStatement(
        $keyword,
        $label_name,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_label_name;
    yield $this->_semicolon;
    yield break;
  }
}
final class ThrowStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_expression;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $expression,
    EditableSyntax $semicolon) {
    parent::__construct('throw_statement');
    $this->_keyword = $keyword;
    $this->_expression = $expression;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): ThrowStatement {
    return new ThrowStatement(
      $keyword,
      $this->_expression,
      $this->_semicolon);
  }
  public function with_expression(EditableSyntax $expression): ThrowStatement {
    return new ThrowStatement(
      $this->_keyword,
      $expression,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): ThrowStatement {
    return new ThrowStatement(
      $this->_keyword,
      $this->_expression,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $expression === $this->expression() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ThrowStatement(
        $keyword,
        $expression,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->throw_keyword, $position, $source);
    $position += $keyword->width();
    $expression = EditableSyntax::from_json(
      $json->throw_expression, $position, $source);
    $position += $expression->width();
    $semicolon = EditableSyntax::from_json(
      $json->throw_semicolon, $position, $source);
    $position += $semicolon->width();
    return new ThrowStatement(
        $keyword,
        $expression,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_expression;
    yield $this->_semicolon;
    yield break;
  }
}
final class BreakStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_level;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $level,
    EditableSyntax $semicolon) {
    parent::__construct('break_statement');
    $this->_keyword = $keyword;
    $this->_level = $level;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function level(): EditableSyntax {
    return $this->_level;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): BreakStatement {
    return new BreakStatement(
      $keyword,
      $this->_level,
      $this->_semicolon);
  }
  public function with_level(EditableSyntax $level): BreakStatement {
    return new BreakStatement(
      $this->_keyword,
      $level,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): BreakStatement {
    return new BreakStatement(
      $this->_keyword,
      $this->_level,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $level = $this->level()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $level === $this->level() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new BreakStatement(
        $keyword,
        $level,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->break_keyword, $position, $source);
    $position += $keyword->width();
    $level = EditableSyntax::from_json(
      $json->break_level, $position, $source);
    $position += $level->width();
    $semicolon = EditableSyntax::from_json(
      $json->break_semicolon, $position, $source);
    $position += $semicolon->width();
    return new BreakStatement(
        $keyword,
        $level,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_level;
    yield $this->_semicolon;
    yield break;
  }
}
final class ContinueStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_level;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $level,
    EditableSyntax $semicolon) {
    parent::__construct('continue_statement');
    $this->_keyword = $keyword;
    $this->_level = $level;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function level(): EditableSyntax {
    return $this->_level;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): ContinueStatement {
    return new ContinueStatement(
      $keyword,
      $this->_level,
      $this->_semicolon);
  }
  public function with_level(EditableSyntax $level): ContinueStatement {
    return new ContinueStatement(
      $this->_keyword,
      $level,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): ContinueStatement {
    return new ContinueStatement(
      $this->_keyword,
      $this->_level,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $level = $this->level()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $level === $this->level() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ContinueStatement(
        $keyword,
        $level,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->continue_keyword, $position, $source);
    $position += $keyword->width();
    $level = EditableSyntax::from_json(
      $json->continue_level, $position, $source);
    $position += $level->width();
    $semicolon = EditableSyntax::from_json(
      $json->continue_semicolon, $position, $source);
    $position += $semicolon->width();
    return new ContinueStatement(
        $keyword,
        $level,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_level;
    yield $this->_semicolon;
    yield break;
  }
}
final class FunctionStaticStatement extends EditableSyntax {
  private EditableSyntax $_static_keyword;
  private EditableSyntax $_declarations;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $static_keyword,
    EditableSyntax $declarations,
    EditableSyntax $semicolon) {
    parent::__construct('function_static_statement');
    $this->_static_keyword = $static_keyword;
    $this->_declarations = $declarations;
    $this->_semicolon = $semicolon;
  }
  public function static_keyword(): EditableSyntax {
    return $this->_static_keyword;
  }
  public function declarations(): EditableSyntax {
    return $this->_declarations;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_static_keyword(EditableSyntax $static_keyword): FunctionStaticStatement {
    return new FunctionStaticStatement(
      $static_keyword,
      $this->_declarations,
      $this->_semicolon);
  }
  public function with_declarations(EditableSyntax $declarations): FunctionStaticStatement {
    return new FunctionStaticStatement(
      $this->_static_keyword,
      $declarations,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): FunctionStaticStatement {
    return new FunctionStaticStatement(
      $this->_static_keyword,
      $this->_declarations,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $static_keyword = $this->static_keyword()->rewrite($rewriter, $new_parents);
    $declarations = $this->declarations()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $static_keyword === $this->static_keyword() &&
      $declarations === $this->declarations() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FunctionStaticStatement(
        $static_keyword,
        $declarations,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $static_keyword = EditableSyntax::from_json(
      $json->static_static_keyword, $position, $source);
    $position += $static_keyword->width();
    $declarations = EditableSyntax::from_json(
      $json->static_declarations, $position, $source);
    $position += $declarations->width();
    $semicolon = EditableSyntax::from_json(
      $json->static_semicolon, $position, $source);
    $position += $semicolon->width();
    return new FunctionStaticStatement(
        $static_keyword,
        $declarations,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_static_keyword;
    yield $this->_declarations;
    yield $this->_semicolon;
    yield break;
  }
}
final class StaticDeclarator extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_initializer;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $initializer) {
    parent::__construct('static_declarator');
    $this->_name = $name;
    $this->_initializer = $initializer;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function initializer(): EditableSyntax {
    return $this->_initializer;
  }
  public function with_name(EditableSyntax $name): StaticDeclarator {
    return new StaticDeclarator(
      $name,
      $this->_initializer);
  }
  public function with_initializer(EditableSyntax $initializer): StaticDeclarator {
    return new StaticDeclarator(
      $this->_name,
      $initializer);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $initializer = $this->initializer()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $initializer === $this->initializer()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new StaticDeclarator(
        $name,
        $initializer), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->static_name, $position, $source);
    $position += $name->width();
    $initializer = EditableSyntax::from_json(
      $json->static_initializer, $position, $source);
    $position += $initializer->width();
    return new StaticDeclarator(
        $name,
        $initializer);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_initializer;
    yield break;
  }
}
final class EchoStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_expressions;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $expressions,
    EditableSyntax $semicolon) {
    parent::__construct('echo_statement');
    $this->_keyword = $keyword;
    $this->_expressions = $expressions;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function expressions(): EditableSyntax {
    return $this->_expressions;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): EchoStatement {
    return new EchoStatement(
      $keyword,
      $this->_expressions,
      $this->_semicolon);
  }
  public function with_expressions(EditableSyntax $expressions): EchoStatement {
    return new EchoStatement(
      $this->_keyword,
      $expressions,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): EchoStatement {
    return new EchoStatement(
      $this->_keyword,
      $this->_expressions,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $expressions = $this->expressions()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $expressions === $this->expressions() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EchoStatement(
        $keyword,
        $expressions,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->echo_keyword, $position, $source);
    $position += $keyword->width();
    $expressions = EditableSyntax::from_json(
      $json->echo_expressions, $position, $source);
    $position += $expressions->width();
    $semicolon = EditableSyntax::from_json(
      $json->echo_semicolon, $position, $source);
    $position += $semicolon->width();
    return new EchoStatement(
        $keyword,
        $expressions,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_expressions;
    yield $this->_semicolon;
    yield break;
  }
}
final class GlobalStatement extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_variables;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $variables,
    EditableSyntax $semicolon) {
    parent::__construct('global_statement');
    $this->_keyword = $keyword;
    $this->_variables = $variables;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function variables(): EditableSyntax {
    return $this->_variables;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): GlobalStatement {
    return new GlobalStatement(
      $keyword,
      $this->_variables,
      $this->_semicolon);
  }
  public function with_variables(EditableSyntax $variables): GlobalStatement {
    return new GlobalStatement(
      $this->_keyword,
      $variables,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): GlobalStatement {
    return new GlobalStatement(
      $this->_keyword,
      $this->_variables,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $variables = $this->variables()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $variables === $this->variables() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new GlobalStatement(
        $keyword,
        $variables,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->global_keyword, $position, $source);
    $position += $keyword->width();
    $variables = EditableSyntax::from_json(
      $json->global_variables, $position, $source);
    $position += $variables->width();
    $semicolon = EditableSyntax::from_json(
      $json->global_semicolon, $position, $source);
    $position += $semicolon->width();
    return new GlobalStatement(
        $keyword,
        $variables,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_variables;
    yield $this->_semicolon;
    yield break;
  }
}
final class SimpleInitializer extends EditableSyntax {
  private EditableSyntax $_equal;
  private EditableSyntax $_value;
  public function __construct(
    EditableSyntax $equal,
    EditableSyntax $value) {
    parent::__construct('simple_initializer');
    $this->_equal = $equal;
    $this->_value = $value;
  }
  public function equal(): EditableSyntax {
    return $this->_equal;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function with_equal(EditableSyntax $equal): SimpleInitializer {
    return new SimpleInitializer(
      $equal,
      $this->_value);
  }
  public function with_value(EditableSyntax $value): SimpleInitializer {
    return new SimpleInitializer(
      $this->_equal,
      $value);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $equal = $this->equal()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    if (
      $equal === $this->equal() &&
      $value === $this->value()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SimpleInitializer(
        $equal,
        $value), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $equal = EditableSyntax::from_json(
      $json->simple_initializer_equal, $position, $source);
    $position += $equal->width();
    $value = EditableSyntax::from_json(
      $json->simple_initializer_value, $position, $source);
    $position += $value->width();
    return new SimpleInitializer(
        $equal,
        $value);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_equal;
    yield $this->_value;
    yield break;
  }
}
final class AnonymousFunction extends EditableSyntax {
  private EditableSyntax $_static_keyword;
  private EditableSyntax $_async_keyword;
  private EditableSyntax $_coroutine_keyword;
  private EditableSyntax $_function_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_parameters;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_colon;
  private EditableSyntax $_type;
  private EditableSyntax $_use;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $static_keyword,
    EditableSyntax $async_keyword,
    EditableSyntax $coroutine_keyword,
    EditableSyntax $function_keyword,
    EditableSyntax $left_paren,
    EditableSyntax $parameters,
    EditableSyntax $right_paren,
    EditableSyntax $colon,
    EditableSyntax $type,
    EditableSyntax $use,
    EditableSyntax $body) {
    parent::__construct('anonymous_function');
    $this->_static_keyword = $static_keyword;
    $this->_async_keyword = $async_keyword;
    $this->_coroutine_keyword = $coroutine_keyword;
    $this->_function_keyword = $function_keyword;
    $this->_left_paren = $left_paren;
    $this->_parameters = $parameters;
    $this->_right_paren = $right_paren;
    $this->_colon = $colon;
    $this->_type = $type;
    $this->_use = $use;
    $this->_body = $body;
  }
  public function static_keyword(): EditableSyntax {
    return $this->_static_keyword;
  }
  public function async_keyword(): EditableSyntax {
    return $this->_async_keyword;
  }
  public function coroutine_keyword(): EditableSyntax {
    return $this->_coroutine_keyword;
  }
  public function function_keyword(): EditableSyntax {
    return $this->_function_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function parameters(): EditableSyntax {
    return $this->_parameters;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function use(): EditableSyntax {
    return $this->_use;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_static_keyword(EditableSyntax $static_keyword): AnonymousFunction {
    return new AnonymousFunction(
      $static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_async_keyword(EditableSyntax $async_keyword): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_coroutine_keyword(EditableSyntax $coroutine_keyword): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_function_keyword(EditableSyntax $function_keyword): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_left_paren(EditableSyntax $left_paren): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_parameters(EditableSyntax $parameters): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_right_paren(EditableSyntax $right_paren): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_colon(EditableSyntax $colon): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $colon,
      $this->_type,
      $this->_use,
      $this->_body);
  }
  public function with_type(EditableSyntax $type): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $type,
      $this->_use,
      $this->_body);
  }
  public function with_use(EditableSyntax $use): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $use,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): AnonymousFunction {
    return new AnonymousFunction(
      $this->_static_keyword,
      $this->_async_keyword,
      $this->_coroutine_keyword,
      $this->_function_keyword,
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type,
      $this->_use,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $static_keyword = $this->static_keyword()->rewrite($rewriter, $new_parents);
    $async_keyword = $this->async_keyword()->rewrite($rewriter, $new_parents);
    $coroutine_keyword = $this->coroutine_keyword()->rewrite($rewriter, $new_parents);
    $function_keyword = $this->function_keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $parameters = $this->parameters()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $use = $this->use()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $static_keyword === $this->static_keyword() &&
      $async_keyword === $this->async_keyword() &&
      $coroutine_keyword === $this->coroutine_keyword() &&
      $function_keyword === $this->function_keyword() &&
      $left_paren === $this->left_paren() &&
      $parameters === $this->parameters() &&
      $right_paren === $this->right_paren() &&
      $colon === $this->colon() &&
      $type === $this->type() &&
      $use === $this->use() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new AnonymousFunction(
        $static_keyword,
        $async_keyword,
        $coroutine_keyword,
        $function_keyword,
        $left_paren,
        $parameters,
        $right_paren,
        $colon,
        $type,
        $use,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $static_keyword = EditableSyntax::from_json(
      $json->anonymous_static_keyword, $position, $source);
    $position += $static_keyword->width();
    $async_keyword = EditableSyntax::from_json(
      $json->anonymous_async_keyword, $position, $source);
    $position += $async_keyword->width();
    $coroutine_keyword = EditableSyntax::from_json(
      $json->anonymous_coroutine_keyword, $position, $source);
    $position += $coroutine_keyword->width();
    $function_keyword = EditableSyntax::from_json(
      $json->anonymous_function_keyword, $position, $source);
    $position += $function_keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->anonymous_left_paren, $position, $source);
    $position += $left_paren->width();
    $parameters = EditableSyntax::from_json(
      $json->anonymous_parameters, $position, $source);
    $position += $parameters->width();
    $right_paren = EditableSyntax::from_json(
      $json->anonymous_right_paren, $position, $source);
    $position += $right_paren->width();
    $colon = EditableSyntax::from_json(
      $json->anonymous_colon, $position, $source);
    $position += $colon->width();
    $type = EditableSyntax::from_json(
      $json->anonymous_type, $position, $source);
    $position += $type->width();
    $use = EditableSyntax::from_json(
      $json->anonymous_use, $position, $source);
    $position += $use->width();
    $body = EditableSyntax::from_json(
      $json->anonymous_body, $position, $source);
    $position += $body->width();
    return new AnonymousFunction(
        $static_keyword,
        $async_keyword,
        $coroutine_keyword,
        $function_keyword,
        $left_paren,
        $parameters,
        $right_paren,
        $colon,
        $type,
        $use,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_static_keyword;
    yield $this->_async_keyword;
    yield $this->_coroutine_keyword;
    yield $this->_function_keyword;
    yield $this->_left_paren;
    yield $this->_parameters;
    yield $this->_right_paren;
    yield $this->_colon;
    yield $this->_type;
    yield $this->_use;
    yield $this->_body;
    yield break;
  }
}
final class AnonymousFunctionUseClause extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_variables;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $variables,
    EditableSyntax $right_paren) {
    parent::__construct('anonymous_function_use_clause');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_variables = $variables;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function variables(): EditableSyntax {
    return $this->_variables;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): AnonymousFunctionUseClause {
    return new AnonymousFunctionUseClause(
      $keyword,
      $this->_left_paren,
      $this->_variables,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): AnonymousFunctionUseClause {
    return new AnonymousFunctionUseClause(
      $this->_keyword,
      $left_paren,
      $this->_variables,
      $this->_right_paren);
  }
  public function with_variables(EditableSyntax $variables): AnonymousFunctionUseClause {
    return new AnonymousFunctionUseClause(
      $this->_keyword,
      $this->_left_paren,
      $variables,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): AnonymousFunctionUseClause {
    return new AnonymousFunctionUseClause(
      $this->_keyword,
      $this->_left_paren,
      $this->_variables,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $variables = $this->variables()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $variables === $this->variables() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new AnonymousFunctionUseClause(
        $keyword,
        $left_paren,
        $variables,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->anonymous_use_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->anonymous_use_left_paren, $position, $source);
    $position += $left_paren->width();
    $variables = EditableSyntax::from_json(
      $json->anonymous_use_variables, $position, $source);
    $position += $variables->width();
    $right_paren = EditableSyntax::from_json(
      $json->anonymous_use_right_paren, $position, $source);
    $position += $right_paren->width();
    return new AnonymousFunctionUseClause(
        $keyword,
        $left_paren,
        $variables,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_variables;
    yield $this->_right_paren;
    yield break;
  }
}
final class LambdaExpression extends EditableSyntax {
  private EditableSyntax $_async;
  private EditableSyntax $_coroutine;
  private EditableSyntax $_signature;
  private EditableSyntax $_arrow;
  private EditableSyntax $_body;
  public function __construct(
    EditableSyntax $async,
    EditableSyntax $coroutine,
    EditableSyntax $signature,
    EditableSyntax $arrow,
    EditableSyntax $body) {
    parent::__construct('lambda_expression');
    $this->_async = $async;
    $this->_coroutine = $coroutine;
    $this->_signature = $signature;
    $this->_arrow = $arrow;
    $this->_body = $body;
  }
  public function async(): EditableSyntax {
    return $this->_async;
  }
  public function coroutine(): EditableSyntax {
    return $this->_coroutine;
  }
  public function signature(): EditableSyntax {
    return $this->_signature;
  }
  public function arrow(): EditableSyntax {
    return $this->_arrow;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function with_async(EditableSyntax $async): LambdaExpression {
    return new LambdaExpression(
      $async,
      $this->_coroutine,
      $this->_signature,
      $this->_arrow,
      $this->_body);
  }
  public function with_coroutine(EditableSyntax $coroutine): LambdaExpression {
    return new LambdaExpression(
      $this->_async,
      $coroutine,
      $this->_signature,
      $this->_arrow,
      $this->_body);
  }
  public function with_signature(EditableSyntax $signature): LambdaExpression {
    return new LambdaExpression(
      $this->_async,
      $this->_coroutine,
      $signature,
      $this->_arrow,
      $this->_body);
  }
  public function with_arrow(EditableSyntax $arrow): LambdaExpression {
    return new LambdaExpression(
      $this->_async,
      $this->_coroutine,
      $this->_signature,
      $arrow,
      $this->_body);
  }
  public function with_body(EditableSyntax $body): LambdaExpression {
    return new LambdaExpression(
      $this->_async,
      $this->_coroutine,
      $this->_signature,
      $this->_arrow,
      $body);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $async = $this->async()->rewrite($rewriter, $new_parents);
    $coroutine = $this->coroutine()->rewrite($rewriter, $new_parents);
    $signature = $this->signature()->rewrite($rewriter, $new_parents);
    $arrow = $this->arrow()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    if (
      $async === $this->async() &&
      $coroutine === $this->coroutine() &&
      $signature === $this->signature() &&
      $arrow === $this->arrow() &&
      $body === $this->body()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new LambdaExpression(
        $async,
        $coroutine,
        $signature,
        $arrow,
        $body), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $async = EditableSyntax::from_json(
      $json->lambda_async, $position, $source);
    $position += $async->width();
    $coroutine = EditableSyntax::from_json(
      $json->lambda_coroutine, $position, $source);
    $position += $coroutine->width();
    $signature = EditableSyntax::from_json(
      $json->lambda_signature, $position, $source);
    $position += $signature->width();
    $arrow = EditableSyntax::from_json(
      $json->lambda_arrow, $position, $source);
    $position += $arrow->width();
    $body = EditableSyntax::from_json(
      $json->lambda_body, $position, $source);
    $position += $body->width();
    return new LambdaExpression(
        $async,
        $coroutine,
        $signature,
        $arrow,
        $body);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_async;
    yield $this->_coroutine;
    yield $this->_signature;
    yield $this->_arrow;
    yield $this->_body;
    yield break;
  }
}
final class LambdaSignature extends EditableSyntax {
  private EditableSyntax $_left_paren;
  private EditableSyntax $_parameters;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_colon;
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $left_paren,
    EditableSyntax $parameters,
    EditableSyntax $right_paren,
    EditableSyntax $colon,
    EditableSyntax $type) {
    parent::__construct('lambda_signature');
    $this->_left_paren = $left_paren;
    $this->_parameters = $parameters;
    $this->_right_paren = $right_paren;
    $this->_colon = $colon;
    $this->_type = $type;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function parameters(): EditableSyntax {
    return $this->_parameters;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_left_paren(EditableSyntax $left_paren): LambdaSignature {
    return new LambdaSignature(
      $left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type);
  }
  public function with_parameters(EditableSyntax $parameters): LambdaSignature {
    return new LambdaSignature(
      $this->_left_paren,
      $parameters,
      $this->_right_paren,
      $this->_colon,
      $this->_type);
  }
  public function with_right_paren(EditableSyntax $right_paren): LambdaSignature {
    return new LambdaSignature(
      $this->_left_paren,
      $this->_parameters,
      $right_paren,
      $this->_colon,
      $this->_type);
  }
  public function with_colon(EditableSyntax $colon): LambdaSignature {
    return new LambdaSignature(
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $colon,
      $this->_type);
  }
  public function with_type(EditableSyntax $type): LambdaSignature {
    return new LambdaSignature(
      $this->_left_paren,
      $this->_parameters,
      $this->_right_paren,
      $this->_colon,
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $parameters = $this->parameters()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $left_paren === $this->left_paren() &&
      $parameters === $this->parameters() &&
      $right_paren === $this->right_paren() &&
      $colon === $this->colon() &&
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new LambdaSignature(
        $left_paren,
        $parameters,
        $right_paren,
        $colon,
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_paren = EditableSyntax::from_json(
      $json->lambda_left_paren, $position, $source);
    $position += $left_paren->width();
    $parameters = EditableSyntax::from_json(
      $json->lambda_parameters, $position, $source);
    $position += $parameters->width();
    $right_paren = EditableSyntax::from_json(
      $json->lambda_right_paren, $position, $source);
    $position += $right_paren->width();
    $colon = EditableSyntax::from_json(
      $json->lambda_colon, $position, $source);
    $position += $colon->width();
    $type = EditableSyntax::from_json(
      $json->lambda_type, $position, $source);
    $position += $type->width();
    return new LambdaSignature(
        $left_paren,
        $parameters,
        $right_paren,
        $colon,
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_paren;
    yield $this->_parameters;
    yield $this->_right_paren;
    yield $this->_colon;
    yield $this->_type;
    yield break;
  }
}
final class CastExpression extends EditableSyntax {
  private EditableSyntax $_left_paren;
  private EditableSyntax $_type;
  private EditableSyntax $_right_paren;
  private EditableSyntax $_operand;
  public function __construct(
    EditableSyntax $left_paren,
    EditableSyntax $type,
    EditableSyntax $right_paren,
    EditableSyntax $operand) {
    parent::__construct('cast_expression');
    $this->_left_paren = $left_paren;
    $this->_type = $type;
    $this->_right_paren = $right_paren;
    $this->_operand = $operand;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function operand(): EditableSyntax {
    return $this->_operand;
  }
  public function with_left_paren(EditableSyntax $left_paren): CastExpression {
    return new CastExpression(
      $left_paren,
      $this->_type,
      $this->_right_paren,
      $this->_operand);
  }
  public function with_type(EditableSyntax $type): CastExpression {
    return new CastExpression(
      $this->_left_paren,
      $type,
      $this->_right_paren,
      $this->_operand);
  }
  public function with_right_paren(EditableSyntax $right_paren): CastExpression {
    return new CastExpression(
      $this->_left_paren,
      $this->_type,
      $right_paren,
      $this->_operand);
  }
  public function with_operand(EditableSyntax $operand): CastExpression {
    return new CastExpression(
      $this->_left_paren,
      $this->_type,
      $this->_right_paren,
      $operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    $operand = $this->operand()->rewrite($rewriter, $new_parents);
    if (
      $left_paren === $this->left_paren() &&
      $type === $this->type() &&
      $right_paren === $this->right_paren() &&
      $operand === $this->operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new CastExpression(
        $left_paren,
        $type,
        $right_paren,
        $operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_paren = EditableSyntax::from_json(
      $json->cast_left_paren, $position, $source);
    $position += $left_paren->width();
    $type = EditableSyntax::from_json(
      $json->cast_type, $position, $source);
    $position += $type->width();
    $right_paren = EditableSyntax::from_json(
      $json->cast_right_paren, $position, $source);
    $position += $right_paren->width();
    $operand = EditableSyntax::from_json(
      $json->cast_operand, $position, $source);
    $position += $operand->width();
    return new CastExpression(
        $left_paren,
        $type,
        $right_paren,
        $operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_paren;
    yield $this->_type;
    yield $this->_right_paren;
    yield $this->_operand;
    yield break;
  }
}
final class ScopeResolutionExpression extends EditableSyntax {
  private EditableSyntax $_qualifier;
  private EditableSyntax $_operator;
  private EditableSyntax $_name;
  public function __construct(
    EditableSyntax $qualifier,
    EditableSyntax $operator,
    EditableSyntax $name) {
    parent::__construct('scope_resolution_expression');
    $this->_qualifier = $qualifier;
    $this->_operator = $operator;
    $this->_name = $name;
  }
  public function qualifier(): EditableSyntax {
    return $this->_qualifier;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function with_qualifier(EditableSyntax $qualifier): ScopeResolutionExpression {
    return new ScopeResolutionExpression(
      $qualifier,
      $this->_operator,
      $this->_name);
  }
  public function with_operator(EditableSyntax $operator): ScopeResolutionExpression {
    return new ScopeResolutionExpression(
      $this->_qualifier,
      $operator,
      $this->_name);
  }
  public function with_name(EditableSyntax $name): ScopeResolutionExpression {
    return new ScopeResolutionExpression(
      $this->_qualifier,
      $this->_operator,
      $name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $qualifier = $this->qualifier()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    if (
      $qualifier === $this->qualifier() &&
      $operator === $this->operator() &&
      $name === $this->name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ScopeResolutionExpression(
        $qualifier,
        $operator,
        $name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $qualifier = EditableSyntax::from_json(
      $json->scope_resolution_qualifier, $position, $source);
    $position += $qualifier->width();
    $operator = EditableSyntax::from_json(
      $json->scope_resolution_operator, $position, $source);
    $position += $operator->width();
    $name = EditableSyntax::from_json(
      $json->scope_resolution_name, $position, $source);
    $position += $name->width();
    return new ScopeResolutionExpression(
        $qualifier,
        $operator,
        $name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_qualifier;
    yield $this->_operator;
    yield $this->_name;
    yield break;
  }
}
final class MemberSelectionExpression extends EditableSyntax {
  private EditableSyntax $_object;
  private EditableSyntax $_operator;
  private EditableSyntax $_name;
  public function __construct(
    EditableSyntax $object,
    EditableSyntax $operator,
    EditableSyntax $name) {
    parent::__construct('member_selection_expression');
    $this->_object = $object;
    $this->_operator = $operator;
    $this->_name = $name;
  }
  public function object(): EditableSyntax {
    return $this->_object;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function with_object(EditableSyntax $object): MemberSelectionExpression {
    return new MemberSelectionExpression(
      $object,
      $this->_operator,
      $this->_name);
  }
  public function with_operator(EditableSyntax $operator): MemberSelectionExpression {
    return new MemberSelectionExpression(
      $this->_object,
      $operator,
      $this->_name);
  }
  public function with_name(EditableSyntax $name): MemberSelectionExpression {
    return new MemberSelectionExpression(
      $this->_object,
      $this->_operator,
      $name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $object = $this->object()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    if (
      $object === $this->object() &&
      $operator === $this->operator() &&
      $name === $this->name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new MemberSelectionExpression(
        $object,
        $operator,
        $name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $object = EditableSyntax::from_json(
      $json->member_object, $position, $source);
    $position += $object->width();
    $operator = EditableSyntax::from_json(
      $json->member_operator, $position, $source);
    $position += $operator->width();
    $name = EditableSyntax::from_json(
      $json->member_name, $position, $source);
    $position += $name->width();
    return new MemberSelectionExpression(
        $object,
        $operator,
        $name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_object;
    yield $this->_operator;
    yield $this->_name;
    yield break;
  }
}
final class SafeMemberSelectionExpression extends EditableSyntax {
  private EditableSyntax $_object;
  private EditableSyntax $_operator;
  private EditableSyntax $_name;
  public function __construct(
    EditableSyntax $object,
    EditableSyntax $operator,
    EditableSyntax $name) {
    parent::__construct('safe_member_selection_expression');
    $this->_object = $object;
    $this->_operator = $operator;
    $this->_name = $name;
  }
  public function object(): EditableSyntax {
    return $this->_object;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function with_object(EditableSyntax $object): SafeMemberSelectionExpression {
    return new SafeMemberSelectionExpression(
      $object,
      $this->_operator,
      $this->_name);
  }
  public function with_operator(EditableSyntax $operator): SafeMemberSelectionExpression {
    return new SafeMemberSelectionExpression(
      $this->_object,
      $operator,
      $this->_name);
  }
  public function with_name(EditableSyntax $name): SafeMemberSelectionExpression {
    return new SafeMemberSelectionExpression(
      $this->_object,
      $this->_operator,
      $name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $object = $this->object()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    if (
      $object === $this->object() &&
      $operator === $this->operator() &&
      $name === $this->name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SafeMemberSelectionExpression(
        $object,
        $operator,
        $name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $object = EditableSyntax::from_json(
      $json->safe_member_object, $position, $source);
    $position += $object->width();
    $operator = EditableSyntax::from_json(
      $json->safe_member_operator, $position, $source);
    $position += $operator->width();
    $name = EditableSyntax::from_json(
      $json->safe_member_name, $position, $source);
    $position += $name->width();
    return new SafeMemberSelectionExpression(
        $object,
        $operator,
        $name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_object;
    yield $this->_operator;
    yield $this->_name;
    yield break;
  }
}
final class EmbeddedMemberSelectionExpression extends EditableSyntax {
  private EditableSyntax $_object;
  private EditableSyntax $_operator;
  private EditableSyntax $_name;
  public function __construct(
    EditableSyntax $object,
    EditableSyntax $operator,
    EditableSyntax $name) {
    parent::__construct('embedded_member_selection_expression');
    $this->_object = $object;
    $this->_operator = $operator;
    $this->_name = $name;
  }
  public function object(): EditableSyntax {
    return $this->_object;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function with_object(EditableSyntax $object): EmbeddedMemberSelectionExpression {
    return new EmbeddedMemberSelectionExpression(
      $object,
      $this->_operator,
      $this->_name);
  }
  public function with_operator(EditableSyntax $operator): EmbeddedMemberSelectionExpression {
    return new EmbeddedMemberSelectionExpression(
      $this->_object,
      $operator,
      $this->_name);
  }
  public function with_name(EditableSyntax $name): EmbeddedMemberSelectionExpression {
    return new EmbeddedMemberSelectionExpression(
      $this->_object,
      $this->_operator,
      $name);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $object = $this->object()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    if (
      $object === $this->object() &&
      $operator === $this->operator() &&
      $name === $this->name()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EmbeddedMemberSelectionExpression(
        $object,
        $operator,
        $name), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $object = EditableSyntax::from_json(
      $json->embedded_member_object, $position, $source);
    $position += $object->width();
    $operator = EditableSyntax::from_json(
      $json->embedded_member_operator, $position, $source);
    $position += $operator->width();
    $name = EditableSyntax::from_json(
      $json->embedded_member_name, $position, $source);
    $position += $name->width();
    return new EmbeddedMemberSelectionExpression(
        $object,
        $operator,
        $name);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_object;
    yield $this->_operator;
    yield $this->_name;
    yield break;
  }
}
final class YieldExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_operand;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $operand) {
    parent::__construct('yield_expression');
    $this->_keyword = $keyword;
    $this->_operand = $operand;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function operand(): EditableSyntax {
    return $this->_operand;
  }
  public function with_keyword(EditableSyntax $keyword): YieldExpression {
    return new YieldExpression(
      $keyword,
      $this->_operand);
  }
  public function with_operand(EditableSyntax $operand): YieldExpression {
    return new YieldExpression(
      $this->_keyword,
      $operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $operand = $this->operand()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $operand === $this->operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new YieldExpression(
        $keyword,
        $operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->yield_keyword, $position, $source);
    $position += $keyword->width();
    $operand = EditableSyntax::from_json(
      $json->yield_operand, $position, $source);
    $position += $operand->width();
    return new YieldExpression(
        $keyword,
        $operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_operand;
    yield break;
  }
}
final class YieldFromExpression extends EditableSyntax {
  private EditableSyntax $_yield_keyword;
  private EditableSyntax $_from_keyword;
  private EditableSyntax $_operand;
  public function __construct(
    EditableSyntax $yield_keyword,
    EditableSyntax $from_keyword,
    EditableSyntax $operand) {
    parent::__construct('yield_from_expression');
    $this->_yield_keyword = $yield_keyword;
    $this->_from_keyword = $from_keyword;
    $this->_operand = $operand;
  }
  public function yield_keyword(): EditableSyntax {
    return $this->_yield_keyword;
  }
  public function from_keyword(): EditableSyntax {
    return $this->_from_keyword;
  }
  public function operand(): EditableSyntax {
    return $this->_operand;
  }
  public function with_yield_keyword(EditableSyntax $yield_keyword): YieldFromExpression {
    return new YieldFromExpression(
      $yield_keyword,
      $this->_from_keyword,
      $this->_operand);
  }
  public function with_from_keyword(EditableSyntax $from_keyword): YieldFromExpression {
    return new YieldFromExpression(
      $this->_yield_keyword,
      $from_keyword,
      $this->_operand);
  }
  public function with_operand(EditableSyntax $operand): YieldFromExpression {
    return new YieldFromExpression(
      $this->_yield_keyword,
      $this->_from_keyword,
      $operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $yield_keyword = $this->yield_keyword()->rewrite($rewriter, $new_parents);
    $from_keyword = $this->from_keyword()->rewrite($rewriter, $new_parents);
    $operand = $this->operand()->rewrite($rewriter, $new_parents);
    if (
      $yield_keyword === $this->yield_keyword() &&
      $from_keyword === $this->from_keyword() &&
      $operand === $this->operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new YieldFromExpression(
        $yield_keyword,
        $from_keyword,
        $operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $yield_keyword = EditableSyntax::from_json(
      $json->yield_from_yield_keyword, $position, $source);
    $position += $yield_keyword->width();
    $from_keyword = EditableSyntax::from_json(
      $json->yield_from_from_keyword, $position, $source);
    $position += $from_keyword->width();
    $operand = EditableSyntax::from_json(
      $json->yield_from_operand, $position, $source);
    $position += $operand->width();
    return new YieldFromExpression(
        $yield_keyword,
        $from_keyword,
        $operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_yield_keyword;
    yield $this->_from_keyword;
    yield $this->_operand;
    yield break;
  }
}
final class PrefixUnaryExpression extends EditableSyntax {
  private EditableSyntax $_operator;
  private EditableSyntax $_operand;
  public function __construct(
    EditableSyntax $operator,
    EditableSyntax $operand) {
    parent::__construct('prefix_unary_expression');
    $this->_operator = $operator;
    $this->_operand = $operand;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function operand(): EditableSyntax {
    return $this->_operand;
  }
  public function with_operator(EditableSyntax $operator): PrefixUnaryExpression {
    return new PrefixUnaryExpression(
      $operator,
      $this->_operand);
  }
  public function with_operand(EditableSyntax $operand): PrefixUnaryExpression {
    return new PrefixUnaryExpression(
      $this->_operator,
      $operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $operand = $this->operand()->rewrite($rewriter, $new_parents);
    if (
      $operator === $this->operator() &&
      $operand === $this->operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new PrefixUnaryExpression(
        $operator,
        $operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $operator = EditableSyntax::from_json(
      $json->prefix_unary_operator, $position, $source);
    $position += $operator->width();
    $operand = EditableSyntax::from_json(
      $json->prefix_unary_operand, $position, $source);
    $position += $operand->width();
    return new PrefixUnaryExpression(
        $operator,
        $operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_operator;
    yield $this->_operand;
    yield break;
  }
}
final class PostfixUnaryExpression extends EditableSyntax {
  private EditableSyntax $_operand;
  private EditableSyntax $_operator;
  public function __construct(
    EditableSyntax $operand,
    EditableSyntax $operator) {
    parent::__construct('postfix_unary_expression');
    $this->_operand = $operand;
    $this->_operator = $operator;
  }
  public function operand(): EditableSyntax {
    return $this->_operand;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function with_operand(EditableSyntax $operand): PostfixUnaryExpression {
    return new PostfixUnaryExpression(
      $operand,
      $this->_operator);
  }
  public function with_operator(EditableSyntax $operator): PostfixUnaryExpression {
    return new PostfixUnaryExpression(
      $this->_operand,
      $operator);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $operand = $this->operand()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    if (
      $operand === $this->operand() &&
      $operator === $this->operator()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new PostfixUnaryExpression(
        $operand,
        $operator), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $operand = EditableSyntax::from_json(
      $json->postfix_unary_operand, $position, $source);
    $position += $operand->width();
    $operator = EditableSyntax::from_json(
      $json->postfix_unary_operator, $position, $source);
    $position += $operator->width();
    return new PostfixUnaryExpression(
        $operand,
        $operator);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_operand;
    yield $this->_operator;
    yield break;
  }
}
final class BinaryExpression extends EditableSyntax {
  private EditableSyntax $_left_operand;
  private EditableSyntax $_operator;
  private EditableSyntax $_right_operand;
  public function __construct(
    EditableSyntax $left_operand,
    EditableSyntax $operator,
    EditableSyntax $right_operand) {
    parent::__construct('binary_expression');
    $this->_left_operand = $left_operand;
    $this->_operator = $operator;
    $this->_right_operand = $right_operand;
  }
  public function left_operand(): EditableSyntax {
    return $this->_left_operand;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function right_operand(): EditableSyntax {
    return $this->_right_operand;
  }
  public function with_left_operand(EditableSyntax $left_operand): BinaryExpression {
    return new BinaryExpression(
      $left_operand,
      $this->_operator,
      $this->_right_operand);
  }
  public function with_operator(EditableSyntax $operator): BinaryExpression {
    return new BinaryExpression(
      $this->_left_operand,
      $operator,
      $this->_right_operand);
  }
  public function with_right_operand(EditableSyntax $right_operand): BinaryExpression {
    return new BinaryExpression(
      $this->_left_operand,
      $this->_operator,
      $right_operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_operand = $this->left_operand()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $right_operand = $this->right_operand()->rewrite($rewriter, $new_parents);
    if (
      $left_operand === $this->left_operand() &&
      $operator === $this->operator() &&
      $right_operand === $this->right_operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new BinaryExpression(
        $left_operand,
        $operator,
        $right_operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_operand = EditableSyntax::from_json(
      $json->binary_left_operand, $position, $source);
    $position += $left_operand->width();
    $operator = EditableSyntax::from_json(
      $json->binary_operator, $position, $source);
    $position += $operator->width();
    $right_operand = EditableSyntax::from_json(
      $json->binary_right_operand, $position, $source);
    $position += $right_operand->width();
    return new BinaryExpression(
        $left_operand,
        $operator,
        $right_operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_operand;
    yield $this->_operator;
    yield $this->_right_operand;
    yield break;
  }
}
final class InstanceofExpression extends EditableSyntax {
  private EditableSyntax $_left_operand;
  private EditableSyntax $_operator;
  private EditableSyntax $_right_operand;
  public function __construct(
    EditableSyntax $left_operand,
    EditableSyntax $operator,
    EditableSyntax $right_operand) {
    parent::__construct('instanceof_expression');
    $this->_left_operand = $left_operand;
    $this->_operator = $operator;
    $this->_right_operand = $right_operand;
  }
  public function left_operand(): EditableSyntax {
    return $this->_left_operand;
  }
  public function operator(): EditableSyntax {
    return $this->_operator;
  }
  public function right_operand(): EditableSyntax {
    return $this->_right_operand;
  }
  public function with_left_operand(EditableSyntax $left_operand): InstanceofExpression {
    return new InstanceofExpression(
      $left_operand,
      $this->_operator,
      $this->_right_operand);
  }
  public function with_operator(EditableSyntax $operator): InstanceofExpression {
    return new InstanceofExpression(
      $this->_left_operand,
      $operator,
      $this->_right_operand);
  }
  public function with_right_operand(EditableSyntax $right_operand): InstanceofExpression {
    return new InstanceofExpression(
      $this->_left_operand,
      $this->_operator,
      $right_operand);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_operand = $this->left_operand()->rewrite($rewriter, $new_parents);
    $operator = $this->operator()->rewrite($rewriter, $new_parents);
    $right_operand = $this->right_operand()->rewrite($rewriter, $new_parents);
    if (
      $left_operand === $this->left_operand() &&
      $operator === $this->operator() &&
      $right_operand === $this->right_operand()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new InstanceofExpression(
        $left_operand,
        $operator,
        $right_operand), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_operand = EditableSyntax::from_json(
      $json->instanceof_left_operand, $position, $source);
    $position += $left_operand->width();
    $operator = EditableSyntax::from_json(
      $json->instanceof_operator, $position, $source);
    $position += $operator->width();
    $right_operand = EditableSyntax::from_json(
      $json->instanceof_right_operand, $position, $source);
    $position += $right_operand->width();
    return new InstanceofExpression(
        $left_operand,
        $operator,
        $right_operand);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_operand;
    yield $this->_operator;
    yield $this->_right_operand;
    yield break;
  }
}
final class ConditionalExpression extends EditableSyntax {
  private EditableSyntax $_test;
  private EditableSyntax $_question;
  private EditableSyntax $_consequence;
  private EditableSyntax $_colon;
  private EditableSyntax $_alternative;
  public function __construct(
    EditableSyntax $test,
    EditableSyntax $question,
    EditableSyntax $consequence,
    EditableSyntax $colon,
    EditableSyntax $alternative) {
    parent::__construct('conditional_expression');
    $this->_test = $test;
    $this->_question = $question;
    $this->_consequence = $consequence;
    $this->_colon = $colon;
    $this->_alternative = $alternative;
  }
  public function test(): EditableSyntax {
    return $this->_test;
  }
  public function question(): EditableSyntax {
    return $this->_question;
  }
  public function consequence(): EditableSyntax {
    return $this->_consequence;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function alternative(): EditableSyntax {
    return $this->_alternative;
  }
  public function with_test(EditableSyntax $test): ConditionalExpression {
    return new ConditionalExpression(
      $test,
      $this->_question,
      $this->_consequence,
      $this->_colon,
      $this->_alternative);
  }
  public function with_question(EditableSyntax $question): ConditionalExpression {
    return new ConditionalExpression(
      $this->_test,
      $question,
      $this->_consequence,
      $this->_colon,
      $this->_alternative);
  }
  public function with_consequence(EditableSyntax $consequence): ConditionalExpression {
    return new ConditionalExpression(
      $this->_test,
      $this->_question,
      $consequence,
      $this->_colon,
      $this->_alternative);
  }
  public function with_colon(EditableSyntax $colon): ConditionalExpression {
    return new ConditionalExpression(
      $this->_test,
      $this->_question,
      $this->_consequence,
      $colon,
      $this->_alternative);
  }
  public function with_alternative(EditableSyntax $alternative): ConditionalExpression {
    return new ConditionalExpression(
      $this->_test,
      $this->_question,
      $this->_consequence,
      $this->_colon,
      $alternative);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $test = $this->test()->rewrite($rewriter, $new_parents);
    $question = $this->question()->rewrite($rewriter, $new_parents);
    $consequence = $this->consequence()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $alternative = $this->alternative()->rewrite($rewriter, $new_parents);
    if (
      $test === $this->test() &&
      $question === $this->question() &&
      $consequence === $this->consequence() &&
      $colon === $this->colon() &&
      $alternative === $this->alternative()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ConditionalExpression(
        $test,
        $question,
        $consequence,
        $colon,
        $alternative), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $test = EditableSyntax::from_json(
      $json->conditional_test, $position, $source);
    $position += $test->width();
    $question = EditableSyntax::from_json(
      $json->conditional_question, $position, $source);
    $position += $question->width();
    $consequence = EditableSyntax::from_json(
      $json->conditional_consequence, $position, $source);
    $position += $consequence->width();
    $colon = EditableSyntax::from_json(
      $json->conditional_colon, $position, $source);
    $position += $colon->width();
    $alternative = EditableSyntax::from_json(
      $json->conditional_alternative, $position, $source);
    $position += $alternative->width();
    return new ConditionalExpression(
        $test,
        $question,
        $consequence,
        $colon,
        $alternative);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_test;
    yield $this->_question;
    yield $this->_consequence;
    yield $this->_colon;
    yield $this->_alternative;
    yield break;
  }
}
final class EvalExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $argument,
    EditableSyntax $right_paren) {
    parent::__construct('eval_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_argument = $argument;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument(): EditableSyntax {
    return $this->_argument;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): EvalExpression {
    return new EvalExpression(
      $keyword,
      $this->_left_paren,
      $this->_argument,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): EvalExpression {
    return new EvalExpression(
      $this->_keyword,
      $left_paren,
      $this->_argument,
      $this->_right_paren);
  }
  public function with_argument(EditableSyntax $argument): EvalExpression {
    return new EvalExpression(
      $this->_keyword,
      $this->_left_paren,
      $argument,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): EvalExpression {
    return new EvalExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_argument,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument = $this->argument()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $argument === $this->argument() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EvalExpression(
        $keyword,
        $left_paren,
        $argument,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->eval_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->eval_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument = EditableSyntax::from_json(
      $json->eval_argument, $position, $source);
    $position += $argument->width();
    $right_paren = EditableSyntax::from_json(
      $json->eval_right_paren, $position, $source);
    $position += $right_paren->width();
    return new EvalExpression(
        $keyword,
        $left_paren,
        $argument,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_argument;
    yield $this->_right_paren;
    yield break;
  }
}
final class EmptyExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $argument,
    EditableSyntax $right_paren) {
    parent::__construct('empty_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_argument = $argument;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument(): EditableSyntax {
    return $this->_argument;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): EmptyExpression {
    return new EmptyExpression(
      $keyword,
      $this->_left_paren,
      $this->_argument,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): EmptyExpression {
    return new EmptyExpression(
      $this->_keyword,
      $left_paren,
      $this->_argument,
      $this->_right_paren);
  }
  public function with_argument(EditableSyntax $argument): EmptyExpression {
    return new EmptyExpression(
      $this->_keyword,
      $this->_left_paren,
      $argument,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): EmptyExpression {
    return new EmptyExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_argument,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument = $this->argument()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $argument === $this->argument() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EmptyExpression(
        $keyword,
        $left_paren,
        $argument,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->empty_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->empty_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument = EditableSyntax::from_json(
      $json->empty_argument, $position, $source);
    $position += $argument->width();
    $right_paren = EditableSyntax::from_json(
      $json->empty_right_paren, $position, $source);
    $position += $right_paren->width();
    return new EmptyExpression(
        $keyword,
        $left_paren,
        $argument,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_argument;
    yield $this->_right_paren;
    yield break;
  }
}
final class DefineExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument_list;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $argument_list,
    EditableSyntax $right_paren) {
    parent::__construct('define_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_argument_list = $argument_list;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): DefineExpression {
    return new DefineExpression(
      $keyword,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): DefineExpression {
    return new DefineExpression(
      $this->_keyword,
      $left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_argument_list(EditableSyntax $argument_list): DefineExpression {
    return new DefineExpression(
      $this->_keyword,
      $this->_left_paren,
      $argument_list,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): DefineExpression {
    return new DefineExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_argument_list,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $argument_list === $this->argument_list() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DefineExpression(
        $keyword,
        $left_paren,
        $argument_list,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->define_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->define_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument_list = EditableSyntax::from_json(
      $json->define_argument_list, $position, $source);
    $position += $argument_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->define_right_paren, $position, $source);
    $position += $right_paren->width();
    return new DefineExpression(
        $keyword,
        $left_paren,
        $argument_list,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_argument_list;
    yield $this->_right_paren;
    yield break;
  }
}
final class IssetExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument_list;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $argument_list,
    EditableSyntax $right_paren) {
    parent::__construct('isset_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_argument_list = $argument_list;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): IssetExpression {
    return new IssetExpression(
      $keyword,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): IssetExpression {
    return new IssetExpression(
      $this->_keyword,
      $left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_argument_list(EditableSyntax $argument_list): IssetExpression {
    return new IssetExpression(
      $this->_keyword,
      $this->_left_paren,
      $argument_list,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): IssetExpression {
    return new IssetExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_argument_list,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $argument_list === $this->argument_list() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new IssetExpression(
        $keyword,
        $left_paren,
        $argument_list,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->isset_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->isset_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument_list = EditableSyntax::from_json(
      $json->isset_argument_list, $position, $source);
    $position += $argument_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->isset_right_paren, $position, $source);
    $position += $right_paren->width();
    return new IssetExpression(
        $keyword,
        $left_paren,
        $argument_list,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_argument_list;
    yield $this->_right_paren;
    yield break;
  }
}
final class FunctionCallExpression extends EditableSyntax {
  private EditableSyntax $_receiver;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument_list;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $receiver,
    EditableSyntax $left_paren,
    EditableSyntax $argument_list,
    EditableSyntax $right_paren) {
    parent::__construct('function_call_expression');
    $this->_receiver = $receiver;
    $this->_left_paren = $left_paren;
    $this->_argument_list = $argument_list;
    $this->_right_paren = $right_paren;
  }
  public function receiver(): EditableSyntax {
    return $this->_receiver;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_receiver(EditableSyntax $receiver): FunctionCallExpression {
    return new FunctionCallExpression(
      $receiver,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): FunctionCallExpression {
    return new FunctionCallExpression(
      $this->_receiver,
      $left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_argument_list(EditableSyntax $argument_list): FunctionCallExpression {
    return new FunctionCallExpression(
      $this->_receiver,
      $this->_left_paren,
      $argument_list,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): FunctionCallExpression {
    return new FunctionCallExpression(
      $this->_receiver,
      $this->_left_paren,
      $this->_argument_list,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $receiver = $this->receiver()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $receiver === $this->receiver() &&
      $left_paren === $this->left_paren() &&
      $argument_list === $this->argument_list() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FunctionCallExpression(
        $receiver,
        $left_paren,
        $argument_list,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $receiver = EditableSyntax::from_json(
      $json->function_call_receiver, $position, $source);
    $position += $receiver->width();
    $left_paren = EditableSyntax::from_json(
      $json->function_call_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument_list = EditableSyntax::from_json(
      $json->function_call_argument_list, $position, $source);
    $position += $argument_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->function_call_right_paren, $position, $source);
    $position += $right_paren->width();
    return new FunctionCallExpression(
        $receiver,
        $left_paren,
        $argument_list,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_receiver;
    yield $this->_left_paren;
    yield $this->_argument_list;
    yield $this->_right_paren;
    yield break;
  }
}
final class FunctionCallWithTypeArgumentsExpression extends EditableSyntax {
  private EditableSyntax $_receiver;
  private EditableSyntax $_type_args;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument_list;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $receiver,
    EditableSyntax $type_args,
    EditableSyntax $left_paren,
    EditableSyntax $argument_list,
    EditableSyntax $right_paren) {
    parent::__construct('function_call_with_type_arguments_expression');
    $this->_receiver = $receiver;
    $this->_type_args = $type_args;
    $this->_left_paren = $left_paren;
    $this->_argument_list = $argument_list;
    $this->_right_paren = $right_paren;
  }
  public function receiver(): EditableSyntax {
    return $this->_receiver;
  }
  public function type_args(): EditableSyntax {
    return $this->_type_args;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_receiver(EditableSyntax $receiver): FunctionCallWithTypeArgumentsExpression {
    return new FunctionCallWithTypeArgumentsExpression(
      $receiver,
      $this->_type_args,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_type_args(EditableSyntax $type_args): FunctionCallWithTypeArgumentsExpression {
    return new FunctionCallWithTypeArgumentsExpression(
      $this->_receiver,
      $type_args,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): FunctionCallWithTypeArgumentsExpression {
    return new FunctionCallWithTypeArgumentsExpression(
      $this->_receiver,
      $this->_type_args,
      $left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_argument_list(EditableSyntax $argument_list): FunctionCallWithTypeArgumentsExpression {
    return new FunctionCallWithTypeArgumentsExpression(
      $this->_receiver,
      $this->_type_args,
      $this->_left_paren,
      $argument_list,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): FunctionCallWithTypeArgumentsExpression {
    return new FunctionCallWithTypeArgumentsExpression(
      $this->_receiver,
      $this->_type_args,
      $this->_left_paren,
      $this->_argument_list,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $receiver = $this->receiver()->rewrite($rewriter, $new_parents);
    $type_args = $this->type_args()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $receiver === $this->receiver() &&
      $type_args === $this->type_args() &&
      $left_paren === $this->left_paren() &&
      $argument_list === $this->argument_list() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FunctionCallWithTypeArgumentsExpression(
        $receiver,
        $type_args,
        $left_paren,
        $argument_list,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $receiver = EditableSyntax::from_json(
      $json->function_call_with_type_arguments_receiver, $position, $source);
    $position += $receiver->width();
    $type_args = EditableSyntax::from_json(
      $json->function_call_with_type_arguments_type_args, $position, $source);
    $position += $type_args->width();
    $left_paren = EditableSyntax::from_json(
      $json->function_call_with_type_arguments_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument_list = EditableSyntax::from_json(
      $json->function_call_with_type_arguments_argument_list, $position, $source);
    $position += $argument_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->function_call_with_type_arguments_right_paren, $position, $source);
    $position += $right_paren->width();
    return new FunctionCallWithTypeArgumentsExpression(
        $receiver,
        $type_args,
        $left_paren,
        $argument_list,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_receiver;
    yield $this->_type_args;
    yield $this->_left_paren;
    yield $this->_argument_list;
    yield $this->_right_paren;
    yield break;
  }
}
final class ParenthesizedExpression extends EditableSyntax {
  private EditableSyntax $_left_paren;
  private EditableSyntax $_expression;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $left_paren,
    EditableSyntax $expression,
    EditableSyntax $right_paren) {
    parent::__construct('parenthesized_expression');
    $this->_left_paren = $left_paren;
    $this->_expression = $expression;
    $this->_right_paren = $right_paren;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_left_paren(EditableSyntax $left_paren): ParenthesizedExpression {
    return new ParenthesizedExpression(
      $left_paren,
      $this->_expression,
      $this->_right_paren);
  }
  public function with_expression(EditableSyntax $expression): ParenthesizedExpression {
    return new ParenthesizedExpression(
      $this->_left_paren,
      $expression,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ParenthesizedExpression {
    return new ParenthesizedExpression(
      $this->_left_paren,
      $this->_expression,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $left_paren === $this->left_paren() &&
      $expression === $this->expression() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ParenthesizedExpression(
        $left_paren,
        $expression,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_paren = EditableSyntax::from_json(
      $json->parenthesized_expression_left_paren, $position, $source);
    $position += $left_paren->width();
    $expression = EditableSyntax::from_json(
      $json->parenthesized_expression_expression, $position, $source);
    $position += $expression->width();
    $right_paren = EditableSyntax::from_json(
      $json->parenthesized_expression_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ParenthesizedExpression(
        $left_paren,
        $expression,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_paren;
    yield $this->_expression;
    yield $this->_right_paren;
    yield break;
  }
}
final class BracedExpression extends EditableSyntax {
  private EditableSyntax $_left_brace;
  private EditableSyntax $_expression;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $left_brace,
    EditableSyntax $expression,
    EditableSyntax $right_brace) {
    parent::__construct('braced_expression');
    $this->_left_brace = $left_brace;
    $this->_expression = $expression;
    $this->_right_brace = $right_brace;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_left_brace(EditableSyntax $left_brace): BracedExpression {
    return new BracedExpression(
      $left_brace,
      $this->_expression,
      $this->_right_brace);
  }
  public function with_expression(EditableSyntax $expression): BracedExpression {
    return new BracedExpression(
      $this->_left_brace,
      $expression,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): BracedExpression {
    return new BracedExpression(
      $this->_left_brace,
      $this->_expression,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $left_brace === $this->left_brace() &&
      $expression === $this->expression() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new BracedExpression(
        $left_brace,
        $expression,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_brace = EditableSyntax::from_json(
      $json->braced_expression_left_brace, $position, $source);
    $position += $left_brace->width();
    $expression = EditableSyntax::from_json(
      $json->braced_expression_expression, $position, $source);
    $position += $expression->width();
    $right_brace = EditableSyntax::from_json(
      $json->braced_expression_right_brace, $position, $source);
    $position += $right_brace->width();
    return new BracedExpression(
        $left_brace,
        $expression,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_brace;
    yield $this->_expression;
    yield $this->_right_brace;
    yield break;
  }
}
final class EmbeddedBracedExpression extends EditableSyntax {
  private EditableSyntax $_left_brace;
  private EditableSyntax $_expression;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $left_brace,
    EditableSyntax $expression,
    EditableSyntax $right_brace) {
    parent::__construct('embedded_braced_expression');
    $this->_left_brace = $left_brace;
    $this->_expression = $expression;
    $this->_right_brace = $right_brace;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_left_brace(EditableSyntax $left_brace): EmbeddedBracedExpression {
    return new EmbeddedBracedExpression(
      $left_brace,
      $this->_expression,
      $this->_right_brace);
  }
  public function with_expression(EditableSyntax $expression): EmbeddedBracedExpression {
    return new EmbeddedBracedExpression(
      $this->_left_brace,
      $expression,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): EmbeddedBracedExpression {
    return new EmbeddedBracedExpression(
      $this->_left_brace,
      $this->_expression,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $left_brace === $this->left_brace() &&
      $expression === $this->expression() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EmbeddedBracedExpression(
        $left_brace,
        $expression,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_brace = EditableSyntax::from_json(
      $json->embedded_braced_expression_left_brace, $position, $source);
    $position += $left_brace->width();
    $expression = EditableSyntax::from_json(
      $json->embedded_braced_expression_expression, $position, $source);
    $position += $expression->width();
    $right_brace = EditableSyntax::from_json(
      $json->embedded_braced_expression_right_brace, $position, $source);
    $position += $right_brace->width();
    return new EmbeddedBracedExpression(
        $left_brace,
        $expression,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_brace;
    yield $this->_expression;
    yield $this->_right_brace;
    yield break;
  }
}
final class ListExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_members;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $members,
    EditableSyntax $right_paren) {
    parent::__construct('list_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_members = $members;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): ListExpression {
    return new ListExpression(
      $keyword,
      $this->_left_paren,
      $this->_members,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): ListExpression {
    return new ListExpression(
      $this->_keyword,
      $left_paren,
      $this->_members,
      $this->_right_paren);
  }
  public function with_members(EditableSyntax $members): ListExpression {
    return new ListExpression(
      $this->_keyword,
      $this->_left_paren,
      $members,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ListExpression {
    return new ListExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_members,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $members === $this->members() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ListExpression(
        $keyword,
        $left_paren,
        $members,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->list_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->list_left_paren, $position, $source);
    $position += $left_paren->width();
    $members = EditableSyntax::from_json(
      $json->list_members, $position, $source);
    $position += $members->width();
    $right_paren = EditableSyntax::from_json(
      $json->list_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ListExpression(
        $keyword,
        $left_paren,
        $members,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_members;
    yield $this->_right_paren;
    yield break;
  }
}
final class CollectionLiteralExpression extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_initializers;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $left_brace,
    EditableSyntax $initializers,
    EditableSyntax $right_brace) {
    parent::__construct('collection_literal_expression');
    $this->_name = $name;
    $this->_left_brace = $left_brace;
    $this->_initializers = $initializers;
    $this->_right_brace = $right_brace;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function initializers(): EditableSyntax {
    return $this->_initializers;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_name(EditableSyntax $name): CollectionLiteralExpression {
    return new CollectionLiteralExpression(
      $name,
      $this->_left_brace,
      $this->_initializers,
      $this->_right_brace);
  }
  public function with_left_brace(EditableSyntax $left_brace): CollectionLiteralExpression {
    return new CollectionLiteralExpression(
      $this->_name,
      $left_brace,
      $this->_initializers,
      $this->_right_brace);
  }
  public function with_initializers(EditableSyntax $initializers): CollectionLiteralExpression {
    return new CollectionLiteralExpression(
      $this->_name,
      $this->_left_brace,
      $initializers,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): CollectionLiteralExpression {
    return new CollectionLiteralExpression(
      $this->_name,
      $this->_left_brace,
      $this->_initializers,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $initializers = $this->initializers()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $left_brace === $this->left_brace() &&
      $initializers === $this->initializers() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new CollectionLiteralExpression(
        $name,
        $left_brace,
        $initializers,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->collection_literal_name, $position, $source);
    $position += $name->width();
    $left_brace = EditableSyntax::from_json(
      $json->collection_literal_left_brace, $position, $source);
    $position += $left_brace->width();
    $initializers = EditableSyntax::from_json(
      $json->collection_literal_initializers, $position, $source);
    $position += $initializers->width();
    $right_brace = EditableSyntax::from_json(
      $json->collection_literal_right_brace, $position, $source);
    $position += $right_brace->width();
    return new CollectionLiteralExpression(
        $name,
        $left_brace,
        $initializers,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_left_brace;
    yield $this->_initializers;
    yield $this->_right_brace;
    yield break;
  }
}
final class ObjectCreationExpression extends EditableSyntax {
  private EditableSyntax $_new_keyword;
  private EditableSyntax $_type;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_argument_list;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $new_keyword,
    EditableSyntax $type,
    EditableSyntax $left_paren,
    EditableSyntax $argument_list,
    EditableSyntax $right_paren) {
    parent::__construct('object_creation_expression');
    $this->_new_keyword = $new_keyword;
    $this->_type = $type;
    $this->_left_paren = $left_paren;
    $this->_argument_list = $argument_list;
    $this->_right_paren = $right_paren;
  }
  public function new_keyword(): EditableSyntax {
    return $this->_new_keyword;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_new_keyword(EditableSyntax $new_keyword): ObjectCreationExpression {
    return new ObjectCreationExpression(
      $new_keyword,
      $this->_type,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_type(EditableSyntax $type): ObjectCreationExpression {
    return new ObjectCreationExpression(
      $this->_new_keyword,
      $type,
      $this->_left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): ObjectCreationExpression {
    return new ObjectCreationExpression(
      $this->_new_keyword,
      $this->_type,
      $left_paren,
      $this->_argument_list,
      $this->_right_paren);
  }
  public function with_argument_list(EditableSyntax $argument_list): ObjectCreationExpression {
    return new ObjectCreationExpression(
      $this->_new_keyword,
      $this->_type,
      $this->_left_paren,
      $argument_list,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ObjectCreationExpression {
    return new ObjectCreationExpression(
      $this->_new_keyword,
      $this->_type,
      $this->_left_paren,
      $this->_argument_list,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $new_keyword = $this->new_keyword()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $new_keyword === $this->new_keyword() &&
      $type === $this->type() &&
      $left_paren === $this->left_paren() &&
      $argument_list === $this->argument_list() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ObjectCreationExpression(
        $new_keyword,
        $type,
        $left_paren,
        $argument_list,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $new_keyword = EditableSyntax::from_json(
      $json->object_creation_new_keyword, $position, $source);
    $position += $new_keyword->width();
    $type = EditableSyntax::from_json(
      $json->object_creation_type, $position, $source);
    $position += $type->width();
    $left_paren = EditableSyntax::from_json(
      $json->object_creation_left_paren, $position, $source);
    $position += $left_paren->width();
    $argument_list = EditableSyntax::from_json(
      $json->object_creation_argument_list, $position, $source);
    $position += $argument_list->width();
    $right_paren = EditableSyntax::from_json(
      $json->object_creation_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ObjectCreationExpression(
        $new_keyword,
        $type,
        $left_paren,
        $argument_list,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_new_keyword;
    yield $this->_type;
    yield $this->_left_paren;
    yield $this->_argument_list;
    yield $this->_right_paren;
    yield break;
  }
}
final class ArrayCreationExpression extends EditableSyntax {
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('array_creation_expression');
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_left_bracket(EditableSyntax $left_bracket): ArrayCreationExpression {
    return new ArrayCreationExpression(
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): ArrayCreationExpression {
    return new ArrayCreationExpression(
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): ArrayCreationExpression {
    return new ArrayCreationExpression(
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ArrayCreationExpression(
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_bracket = EditableSyntax::from_json(
      $json->array_creation_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->array_creation_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->array_creation_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new ArrayCreationExpression(
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class ArrayIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_members;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $members,
    EditableSyntax $right_paren) {
    parent::__construct('array_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_members = $members;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): ArrayIntrinsicExpression {
    return new ArrayIntrinsicExpression(
      $keyword,
      $this->_left_paren,
      $this->_members,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): ArrayIntrinsicExpression {
    return new ArrayIntrinsicExpression(
      $this->_keyword,
      $left_paren,
      $this->_members,
      $this->_right_paren);
  }
  public function with_members(EditableSyntax $members): ArrayIntrinsicExpression {
    return new ArrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_paren,
      $members,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ArrayIntrinsicExpression {
    return new ArrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_members,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $members === $this->members() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ArrayIntrinsicExpression(
        $keyword,
        $left_paren,
        $members,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->array_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->array_intrinsic_left_paren, $position, $source);
    $position += $left_paren->width();
    $members = EditableSyntax::from_json(
      $json->array_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_paren = EditableSyntax::from_json(
      $json->array_intrinsic_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ArrayIntrinsicExpression(
        $keyword,
        $left_paren,
        $members,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_members;
    yield $this->_right_paren;
    yield break;
  }
}
final class DarrayIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('darray_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_keyword(EditableSyntax $keyword): DarrayIntrinsicExpression {
    return new DarrayIntrinsicExpression(
      $keyword,
      $this->_left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): DarrayIntrinsicExpression {
    return new DarrayIntrinsicExpression(
      $this->_keyword,
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): DarrayIntrinsicExpression {
    return new DarrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): DarrayIntrinsicExpression {
    return new DarrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DarrayIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->darray_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_bracket = EditableSyntax::from_json(
      $json->darray_intrinsic_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->darray_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->darray_intrinsic_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new DarrayIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class DictionaryIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('dictionary_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_keyword(EditableSyntax $keyword): DictionaryIntrinsicExpression {
    return new DictionaryIntrinsicExpression(
      $keyword,
      $this->_left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): DictionaryIntrinsicExpression {
    return new DictionaryIntrinsicExpression(
      $this->_keyword,
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): DictionaryIntrinsicExpression {
    return new DictionaryIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): DictionaryIntrinsicExpression {
    return new DictionaryIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DictionaryIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->dictionary_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_bracket = EditableSyntax::from_json(
      $json->dictionary_intrinsic_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->dictionary_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->dictionary_intrinsic_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new DictionaryIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class KeysetIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('keyset_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_keyword(EditableSyntax $keyword): KeysetIntrinsicExpression {
    return new KeysetIntrinsicExpression(
      $keyword,
      $this->_left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): KeysetIntrinsicExpression {
    return new KeysetIntrinsicExpression(
      $this->_keyword,
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): KeysetIntrinsicExpression {
    return new KeysetIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): KeysetIntrinsicExpression {
    return new KeysetIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new KeysetIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->keyset_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_bracket = EditableSyntax::from_json(
      $json->keyset_intrinsic_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->keyset_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->keyset_intrinsic_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new KeysetIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class VarrayIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('varray_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_keyword(EditableSyntax $keyword): VarrayIntrinsicExpression {
    return new VarrayIntrinsicExpression(
      $keyword,
      $this->_left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): VarrayIntrinsicExpression {
    return new VarrayIntrinsicExpression(
      $this->_keyword,
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): VarrayIntrinsicExpression {
    return new VarrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): VarrayIntrinsicExpression {
    return new VarrayIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VarrayIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->varray_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_bracket = EditableSyntax::from_json(
      $json->varray_intrinsic_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->varray_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->varray_intrinsic_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new VarrayIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class VectorIntrinsicExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_members;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_bracket,
    EditableSyntax $members,
    EditableSyntax $right_bracket) {
    parent::__construct('vector_intrinsic_expression');
    $this->_keyword = $keyword;
    $this->_left_bracket = $left_bracket;
    $this->_members = $members;
    $this->_right_bracket = $right_bracket;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_keyword(EditableSyntax $keyword): VectorIntrinsicExpression {
    return new VectorIntrinsicExpression(
      $keyword,
      $this->_left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): VectorIntrinsicExpression {
    return new VectorIntrinsicExpression(
      $this->_keyword,
      $left_bracket,
      $this->_members,
      $this->_right_bracket);
  }
  public function with_members(EditableSyntax $members): VectorIntrinsicExpression {
    return new VectorIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $members,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): VectorIntrinsicExpression {
    return new VectorIntrinsicExpression(
      $this->_keyword,
      $this->_left_bracket,
      $this->_members,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_bracket === $this->left_bracket() &&
      $members === $this->members() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VectorIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->vector_intrinsic_keyword, $position, $source);
    $position += $keyword->width();
    $left_bracket = EditableSyntax::from_json(
      $json->vector_intrinsic_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $members = EditableSyntax::from_json(
      $json->vector_intrinsic_members, $position, $source);
    $position += $members->width();
    $right_bracket = EditableSyntax::from_json(
      $json->vector_intrinsic_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new VectorIntrinsicExpression(
        $keyword,
        $left_bracket,
        $members,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_bracket;
    yield $this->_members;
    yield $this->_right_bracket;
    yield break;
  }
}
final class ElementInitializer extends EditableSyntax {
  private EditableSyntax $_key;
  private EditableSyntax $_arrow;
  private EditableSyntax $_value;
  public function __construct(
    EditableSyntax $key,
    EditableSyntax $arrow,
    EditableSyntax $value) {
    parent::__construct('element_initializer');
    $this->_key = $key;
    $this->_arrow = $arrow;
    $this->_value = $value;
  }
  public function key(): EditableSyntax {
    return $this->_key;
  }
  public function arrow(): EditableSyntax {
    return $this->_arrow;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function with_key(EditableSyntax $key): ElementInitializer {
    return new ElementInitializer(
      $key,
      $this->_arrow,
      $this->_value);
  }
  public function with_arrow(EditableSyntax $arrow): ElementInitializer {
    return new ElementInitializer(
      $this->_key,
      $arrow,
      $this->_value);
  }
  public function with_value(EditableSyntax $value): ElementInitializer {
    return new ElementInitializer(
      $this->_key,
      $this->_arrow,
      $value);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $key = $this->key()->rewrite($rewriter, $new_parents);
    $arrow = $this->arrow()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    if (
      $key === $this->key() &&
      $arrow === $this->arrow() &&
      $value === $this->value()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ElementInitializer(
        $key,
        $arrow,
        $value), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $key = EditableSyntax::from_json(
      $json->element_key, $position, $source);
    $position += $key->width();
    $arrow = EditableSyntax::from_json(
      $json->element_arrow, $position, $source);
    $position += $arrow->width();
    $value = EditableSyntax::from_json(
      $json->element_value, $position, $source);
    $position += $value->width();
    return new ElementInitializer(
        $key,
        $arrow,
        $value);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_key;
    yield $this->_arrow;
    yield $this->_value;
    yield break;
  }
}
final class SubscriptExpression extends EditableSyntax {
  private EditableSyntax $_receiver;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_index;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $receiver,
    EditableSyntax $left_bracket,
    EditableSyntax $index,
    EditableSyntax $right_bracket) {
    parent::__construct('subscript_expression');
    $this->_receiver = $receiver;
    $this->_left_bracket = $left_bracket;
    $this->_index = $index;
    $this->_right_bracket = $right_bracket;
  }
  public function receiver(): EditableSyntax {
    return $this->_receiver;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function index(): EditableSyntax {
    return $this->_index;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_receiver(EditableSyntax $receiver): SubscriptExpression {
    return new SubscriptExpression(
      $receiver,
      $this->_left_bracket,
      $this->_index,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): SubscriptExpression {
    return new SubscriptExpression(
      $this->_receiver,
      $left_bracket,
      $this->_index,
      $this->_right_bracket);
  }
  public function with_index(EditableSyntax $index): SubscriptExpression {
    return new SubscriptExpression(
      $this->_receiver,
      $this->_left_bracket,
      $index,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): SubscriptExpression {
    return new SubscriptExpression(
      $this->_receiver,
      $this->_left_bracket,
      $this->_index,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $receiver = $this->receiver()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $index = $this->index()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $receiver === $this->receiver() &&
      $left_bracket === $this->left_bracket() &&
      $index === $this->index() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SubscriptExpression(
        $receiver,
        $left_bracket,
        $index,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $receiver = EditableSyntax::from_json(
      $json->subscript_receiver, $position, $source);
    $position += $receiver->width();
    $left_bracket = EditableSyntax::from_json(
      $json->subscript_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $index = EditableSyntax::from_json(
      $json->subscript_index, $position, $source);
    $position += $index->width();
    $right_bracket = EditableSyntax::from_json(
      $json->subscript_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new SubscriptExpression(
        $receiver,
        $left_bracket,
        $index,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_receiver;
    yield $this->_left_bracket;
    yield $this->_index;
    yield $this->_right_bracket;
    yield break;
  }
}
final class EmbeddedSubscriptExpression extends EditableSyntax {
  private EditableSyntax $_receiver;
  private EditableSyntax $_left_bracket;
  private EditableSyntax $_index;
  private EditableSyntax $_right_bracket;
  public function __construct(
    EditableSyntax $receiver,
    EditableSyntax $left_bracket,
    EditableSyntax $index,
    EditableSyntax $right_bracket) {
    parent::__construct('embedded_subscript_expression');
    $this->_receiver = $receiver;
    $this->_left_bracket = $left_bracket;
    $this->_index = $index;
    $this->_right_bracket = $right_bracket;
  }
  public function receiver(): EditableSyntax {
    return $this->_receiver;
  }
  public function left_bracket(): EditableSyntax {
    return $this->_left_bracket;
  }
  public function index(): EditableSyntax {
    return $this->_index;
  }
  public function right_bracket(): EditableSyntax {
    return $this->_right_bracket;
  }
  public function with_receiver(EditableSyntax $receiver): EmbeddedSubscriptExpression {
    return new EmbeddedSubscriptExpression(
      $receiver,
      $this->_left_bracket,
      $this->_index,
      $this->_right_bracket);
  }
  public function with_left_bracket(EditableSyntax $left_bracket): EmbeddedSubscriptExpression {
    return new EmbeddedSubscriptExpression(
      $this->_receiver,
      $left_bracket,
      $this->_index,
      $this->_right_bracket);
  }
  public function with_index(EditableSyntax $index): EmbeddedSubscriptExpression {
    return new EmbeddedSubscriptExpression(
      $this->_receiver,
      $this->_left_bracket,
      $index,
      $this->_right_bracket);
  }
  public function with_right_bracket(EditableSyntax $right_bracket): EmbeddedSubscriptExpression {
    return new EmbeddedSubscriptExpression(
      $this->_receiver,
      $this->_left_bracket,
      $this->_index,
      $right_bracket);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $receiver = $this->receiver()->rewrite($rewriter, $new_parents);
    $left_bracket = $this->left_bracket()->rewrite($rewriter, $new_parents);
    $index = $this->index()->rewrite($rewriter, $new_parents);
    $right_bracket = $this->right_bracket()->rewrite($rewriter, $new_parents);
    if (
      $receiver === $this->receiver() &&
      $left_bracket === $this->left_bracket() &&
      $index === $this->index() &&
      $right_bracket === $this->right_bracket()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new EmbeddedSubscriptExpression(
        $receiver,
        $left_bracket,
        $index,
        $right_bracket), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $receiver = EditableSyntax::from_json(
      $json->embedded_subscript_receiver, $position, $source);
    $position += $receiver->width();
    $left_bracket = EditableSyntax::from_json(
      $json->embedded_subscript_left_bracket, $position, $source);
    $position += $left_bracket->width();
    $index = EditableSyntax::from_json(
      $json->embedded_subscript_index, $position, $source);
    $position += $index->width();
    $right_bracket = EditableSyntax::from_json(
      $json->embedded_subscript_right_bracket, $position, $source);
    $position += $right_bracket->width();
    return new EmbeddedSubscriptExpression(
        $receiver,
        $left_bracket,
        $index,
        $right_bracket);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_receiver;
    yield $this->_left_bracket;
    yield $this->_index;
    yield $this->_right_bracket;
    yield break;
  }
}
final class AwaitableCreationExpression extends EditableSyntax {
  private EditableSyntax $_async;
  private EditableSyntax $_coroutine;
  private EditableSyntax $_compound_statement;
  public function __construct(
    EditableSyntax $async,
    EditableSyntax $coroutine,
    EditableSyntax $compound_statement) {
    parent::__construct('awaitable_creation_expression');
    $this->_async = $async;
    $this->_coroutine = $coroutine;
    $this->_compound_statement = $compound_statement;
  }
  public function async(): EditableSyntax {
    return $this->_async;
  }
  public function coroutine(): EditableSyntax {
    return $this->_coroutine;
  }
  public function compound_statement(): EditableSyntax {
    return $this->_compound_statement;
  }
  public function with_async(EditableSyntax $async): AwaitableCreationExpression {
    return new AwaitableCreationExpression(
      $async,
      $this->_coroutine,
      $this->_compound_statement);
  }
  public function with_coroutine(EditableSyntax $coroutine): AwaitableCreationExpression {
    return new AwaitableCreationExpression(
      $this->_async,
      $coroutine,
      $this->_compound_statement);
  }
  public function with_compound_statement(EditableSyntax $compound_statement): AwaitableCreationExpression {
    return new AwaitableCreationExpression(
      $this->_async,
      $this->_coroutine,
      $compound_statement);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $async = $this->async()->rewrite($rewriter, $new_parents);
    $coroutine = $this->coroutine()->rewrite($rewriter, $new_parents);
    $compound_statement = $this->compound_statement()->rewrite($rewriter, $new_parents);
    if (
      $async === $this->async() &&
      $coroutine === $this->coroutine() &&
      $compound_statement === $this->compound_statement()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new AwaitableCreationExpression(
        $async,
        $coroutine,
        $compound_statement), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $async = EditableSyntax::from_json(
      $json->awaitable_async, $position, $source);
    $position += $async->width();
    $coroutine = EditableSyntax::from_json(
      $json->awaitable_coroutine, $position, $source);
    $position += $coroutine->width();
    $compound_statement = EditableSyntax::from_json(
      $json->awaitable_compound_statement, $position, $source);
    $position += $compound_statement->width();
    return new AwaitableCreationExpression(
        $async,
        $coroutine,
        $compound_statement);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_async;
    yield $this->_coroutine;
    yield $this->_compound_statement;
    yield break;
  }
}
final class XHPChildrenDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_expression;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $expression,
    EditableSyntax $semicolon) {
    parent::__construct('xhp_children_declaration');
    $this->_keyword = $keyword;
    $this->_expression = $expression;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): XHPChildrenDeclaration {
    return new XHPChildrenDeclaration(
      $keyword,
      $this->_expression,
      $this->_semicolon);
  }
  public function with_expression(EditableSyntax $expression): XHPChildrenDeclaration {
    return new XHPChildrenDeclaration(
      $this->_keyword,
      $expression,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): XHPChildrenDeclaration {
    return new XHPChildrenDeclaration(
      $this->_keyword,
      $this->_expression,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $expression === $this->expression() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPChildrenDeclaration(
        $keyword,
        $expression,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->xhp_children_keyword, $position, $source);
    $position += $keyword->width();
    $expression = EditableSyntax::from_json(
      $json->xhp_children_expression, $position, $source);
    $position += $expression->width();
    $semicolon = EditableSyntax::from_json(
      $json->xhp_children_semicolon, $position, $source);
    $position += $semicolon->width();
    return new XHPChildrenDeclaration(
        $keyword,
        $expression,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_expression;
    yield $this->_semicolon;
    yield break;
  }
}
final class XHPChildrenParenthesizedList extends EditableSyntax {
  private EditableSyntax $_left_paren;
  private EditableSyntax $_xhp_children;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $left_paren,
    EditableSyntax $xhp_children,
    EditableSyntax $right_paren) {
    parent::__construct('xhp_children_parenthesized_list');
    $this->_left_paren = $left_paren;
    $this->_xhp_children = $xhp_children;
    $this->_right_paren = $right_paren;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function xhp_children(): EditableSyntax {
    return $this->_xhp_children;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_left_paren(EditableSyntax $left_paren): XHPChildrenParenthesizedList {
    return new XHPChildrenParenthesizedList(
      $left_paren,
      $this->_xhp_children,
      $this->_right_paren);
  }
  public function with_xhp_children(EditableSyntax $xhp_children): XHPChildrenParenthesizedList {
    return new XHPChildrenParenthesizedList(
      $this->_left_paren,
      $xhp_children,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): XHPChildrenParenthesizedList {
    return new XHPChildrenParenthesizedList(
      $this->_left_paren,
      $this->_xhp_children,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $xhp_children = $this->xhp_children()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $left_paren === $this->left_paren() &&
      $xhp_children === $this->xhp_children() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPChildrenParenthesizedList(
        $left_paren,
        $xhp_children,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_paren = EditableSyntax::from_json(
      $json->xhp_children_list_left_paren, $position, $source);
    $position += $left_paren->width();
    $xhp_children = EditableSyntax::from_json(
      $json->xhp_children_list_xhp_children, $position, $source);
    $position += $xhp_children->width();
    $right_paren = EditableSyntax::from_json(
      $json->xhp_children_list_right_paren, $position, $source);
    $position += $right_paren->width();
    return new XHPChildrenParenthesizedList(
        $left_paren,
        $xhp_children,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_paren;
    yield $this->_xhp_children;
    yield $this->_right_paren;
    yield break;
  }
}
final class XHPCategoryDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_categories;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $categories,
    EditableSyntax $semicolon) {
    parent::__construct('xhp_category_declaration');
    $this->_keyword = $keyword;
    $this->_categories = $categories;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function categories(): EditableSyntax {
    return $this->_categories;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): XHPCategoryDeclaration {
    return new XHPCategoryDeclaration(
      $keyword,
      $this->_categories,
      $this->_semicolon);
  }
  public function with_categories(EditableSyntax $categories): XHPCategoryDeclaration {
    return new XHPCategoryDeclaration(
      $this->_keyword,
      $categories,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): XHPCategoryDeclaration {
    return new XHPCategoryDeclaration(
      $this->_keyword,
      $this->_categories,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $categories = $this->categories()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $categories === $this->categories() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPCategoryDeclaration(
        $keyword,
        $categories,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->xhp_category_keyword, $position, $source);
    $position += $keyword->width();
    $categories = EditableSyntax::from_json(
      $json->xhp_category_categories, $position, $source);
    $position += $categories->width();
    $semicolon = EditableSyntax::from_json(
      $json->xhp_category_semicolon, $position, $source);
    $position += $semicolon->width();
    return new XHPCategoryDeclaration(
        $keyword,
        $categories,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_categories;
    yield $this->_semicolon;
    yield break;
  }
}
final class XHPEnumType extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_brace;
  private EditableSyntax $_values;
  private EditableSyntax $_right_brace;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_brace,
    EditableSyntax $values,
    EditableSyntax $right_brace) {
    parent::__construct('xhp_enum_type');
    $this->_keyword = $keyword;
    $this->_left_brace = $left_brace;
    $this->_values = $values;
    $this->_right_brace = $right_brace;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_brace(): EditableSyntax {
    return $this->_left_brace;
  }
  public function values(): EditableSyntax {
    return $this->_values;
  }
  public function right_brace(): EditableSyntax {
    return $this->_right_brace;
  }
  public function with_keyword(EditableSyntax $keyword): XHPEnumType {
    return new XHPEnumType(
      $keyword,
      $this->_left_brace,
      $this->_values,
      $this->_right_brace);
  }
  public function with_left_brace(EditableSyntax $left_brace): XHPEnumType {
    return new XHPEnumType(
      $this->_keyword,
      $left_brace,
      $this->_values,
      $this->_right_brace);
  }
  public function with_values(EditableSyntax $values): XHPEnumType {
    return new XHPEnumType(
      $this->_keyword,
      $this->_left_brace,
      $values,
      $this->_right_brace);
  }
  public function with_right_brace(EditableSyntax $right_brace): XHPEnumType {
    return new XHPEnumType(
      $this->_keyword,
      $this->_left_brace,
      $this->_values,
      $right_brace);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_brace = $this->left_brace()->rewrite($rewriter, $new_parents);
    $values = $this->values()->rewrite($rewriter, $new_parents);
    $right_brace = $this->right_brace()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_brace === $this->left_brace() &&
      $values === $this->values() &&
      $right_brace === $this->right_brace()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPEnumType(
        $keyword,
        $left_brace,
        $values,
        $right_brace), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->xhp_enum_keyword, $position, $source);
    $position += $keyword->width();
    $left_brace = EditableSyntax::from_json(
      $json->xhp_enum_left_brace, $position, $source);
    $position += $left_brace->width();
    $values = EditableSyntax::from_json(
      $json->xhp_enum_values, $position, $source);
    $position += $values->width();
    $right_brace = EditableSyntax::from_json(
      $json->xhp_enum_right_brace, $position, $source);
    $position += $right_brace->width();
    return new XHPEnumType(
        $keyword,
        $left_brace,
        $values,
        $right_brace);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_brace;
    yield $this->_values;
    yield $this->_right_brace;
    yield break;
  }
}
final class XHPRequired extends EditableSyntax {
  private EditableSyntax $_at;
  private EditableSyntax $_keyword;
  public function __construct(
    EditableSyntax $at,
    EditableSyntax $keyword) {
    parent::__construct('xhp_required');
    $this->_at = $at;
    $this->_keyword = $keyword;
  }
  public function at(): EditableSyntax {
    return $this->_at;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function with_at(EditableSyntax $at): XHPRequired {
    return new XHPRequired(
      $at,
      $this->_keyword);
  }
  public function with_keyword(EditableSyntax $keyword): XHPRequired {
    return new XHPRequired(
      $this->_at,
      $keyword);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $at = $this->at()->rewrite($rewriter, $new_parents);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    if (
      $at === $this->at() &&
      $keyword === $this->keyword()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPRequired(
        $at,
        $keyword), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $at = EditableSyntax::from_json(
      $json->xhp_required_at, $position, $source);
    $position += $at->width();
    $keyword = EditableSyntax::from_json(
      $json->xhp_required_keyword, $position, $source);
    $position += $keyword->width();
    return new XHPRequired(
        $at,
        $keyword);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_at;
    yield $this->_keyword;
    yield break;
  }
}
final class XHPClassAttributeDeclaration extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_attributes;
  private EditableSyntax $_semicolon;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $attributes,
    EditableSyntax $semicolon) {
    parent::__construct('xhp_class_attribute_declaration');
    $this->_keyword = $keyword;
    $this->_attributes = $attributes;
    $this->_semicolon = $semicolon;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function attributes(): EditableSyntax {
    return $this->_attributes;
  }
  public function semicolon(): EditableSyntax {
    return $this->_semicolon;
  }
  public function with_keyword(EditableSyntax $keyword): XHPClassAttributeDeclaration {
    return new XHPClassAttributeDeclaration(
      $keyword,
      $this->_attributes,
      $this->_semicolon);
  }
  public function with_attributes(EditableSyntax $attributes): XHPClassAttributeDeclaration {
    return new XHPClassAttributeDeclaration(
      $this->_keyword,
      $attributes,
      $this->_semicolon);
  }
  public function with_semicolon(EditableSyntax $semicolon): XHPClassAttributeDeclaration {
    return new XHPClassAttributeDeclaration(
      $this->_keyword,
      $this->_attributes,
      $semicolon);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $attributes = $this->attributes()->rewrite($rewriter, $new_parents);
    $semicolon = $this->semicolon()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $attributes === $this->attributes() &&
      $semicolon === $this->semicolon()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPClassAttributeDeclaration(
        $keyword,
        $attributes,
        $semicolon), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->xhp_attribute_keyword, $position, $source);
    $position += $keyword->width();
    $attributes = EditableSyntax::from_json(
      $json->xhp_attribute_attributes, $position, $source);
    $position += $attributes->width();
    $semicolon = EditableSyntax::from_json(
      $json->xhp_attribute_semicolon, $position, $source);
    $position += $semicolon->width();
    return new XHPClassAttributeDeclaration(
        $keyword,
        $attributes,
        $semicolon);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_attributes;
    yield $this->_semicolon;
    yield break;
  }
}
final class XHPClassAttribute extends EditableSyntax {
  private EditableSyntax $_type;
  private EditableSyntax $_name;
  private EditableSyntax $_initializer;
  private EditableSyntax $_required;
  public function __construct(
    EditableSyntax $type,
    EditableSyntax $name,
    EditableSyntax $initializer,
    EditableSyntax $required) {
    parent::__construct('xhp_class_attribute');
    $this->_type = $type;
    $this->_name = $name;
    $this->_initializer = $initializer;
    $this->_required = $required;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function initializer(): EditableSyntax {
    return $this->_initializer;
  }
  public function required(): EditableSyntax {
    return $this->_required;
  }
  public function with_type(EditableSyntax $type): XHPClassAttribute {
    return new XHPClassAttribute(
      $type,
      $this->_name,
      $this->_initializer,
      $this->_required);
  }
  public function with_name(EditableSyntax $name): XHPClassAttribute {
    return new XHPClassAttribute(
      $this->_type,
      $name,
      $this->_initializer,
      $this->_required);
  }
  public function with_initializer(EditableSyntax $initializer): XHPClassAttribute {
    return new XHPClassAttribute(
      $this->_type,
      $this->_name,
      $initializer,
      $this->_required);
  }
  public function with_required(EditableSyntax $required): XHPClassAttribute {
    return new XHPClassAttribute(
      $this->_type,
      $this->_name,
      $this->_initializer,
      $required);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $initializer = $this->initializer()->rewrite($rewriter, $new_parents);
    $required = $this->required()->rewrite($rewriter, $new_parents);
    if (
      $type === $this->type() &&
      $name === $this->name() &&
      $initializer === $this->initializer() &&
      $required === $this->required()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPClassAttribute(
        $type,
        $name,
        $initializer,
        $required), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $type = EditableSyntax::from_json(
      $json->xhp_attribute_decl_type, $position, $source);
    $position += $type->width();
    $name = EditableSyntax::from_json(
      $json->xhp_attribute_decl_name, $position, $source);
    $position += $name->width();
    $initializer = EditableSyntax::from_json(
      $json->xhp_attribute_decl_initializer, $position, $source);
    $position += $initializer->width();
    $required = EditableSyntax::from_json(
      $json->xhp_attribute_decl_required, $position, $source);
    $position += $required->width();
    return new XHPClassAttribute(
        $type,
        $name,
        $initializer,
        $required);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_type;
    yield $this->_name;
    yield $this->_initializer;
    yield $this->_required;
    yield break;
  }
}
final class XHPSimpleClassAttribute extends EditableSyntax {
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $type) {
    parent::__construct('xhp_simple_class_attribute');
    $this->_type = $type;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_type(EditableSyntax $type): XHPSimpleClassAttribute {
    return new XHPSimpleClassAttribute(
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPSimpleClassAttribute(
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $type = EditableSyntax::from_json(
      $json->xhp_simple_class_attribute_type, $position, $source);
    $position += $type->width();
    return new XHPSimpleClassAttribute(
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_type;
    yield break;
  }
}
final class XHPAttribute extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_equal;
  private EditableSyntax $_expression;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $equal,
    EditableSyntax $expression) {
    parent::__construct('xhp_attribute');
    $this->_name = $name;
    $this->_equal = $equal;
    $this->_expression = $expression;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function equal(): EditableSyntax {
    return $this->_equal;
  }
  public function expression(): EditableSyntax {
    return $this->_expression;
  }
  public function with_name(EditableSyntax $name): XHPAttribute {
    return new XHPAttribute(
      $name,
      $this->_equal,
      $this->_expression);
  }
  public function with_equal(EditableSyntax $equal): XHPAttribute {
    return new XHPAttribute(
      $this->_name,
      $equal,
      $this->_expression);
  }
  public function with_expression(EditableSyntax $expression): XHPAttribute {
    return new XHPAttribute(
      $this->_name,
      $this->_equal,
      $expression);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $equal = $this->equal()->rewrite($rewriter, $new_parents);
    $expression = $this->expression()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $equal === $this->equal() &&
      $expression === $this->expression()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPAttribute(
        $name,
        $equal,
        $expression), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->xhp_attribute_name, $position, $source);
    $position += $name->width();
    $equal = EditableSyntax::from_json(
      $json->xhp_attribute_equal, $position, $source);
    $position += $equal->width();
    $expression = EditableSyntax::from_json(
      $json->xhp_attribute_expression, $position, $source);
    $position += $expression->width();
    return new XHPAttribute(
        $name,
        $equal,
        $expression);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_equal;
    yield $this->_expression;
    yield break;
  }
}
final class XHPOpen extends EditableSyntax {
  private EditableSyntax $_left_angle;
  private EditableSyntax $_name;
  private EditableSyntax $_attributes;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $left_angle,
    EditableSyntax $name,
    EditableSyntax $attributes,
    EditableSyntax $right_angle) {
    parent::__construct('xhp_open');
    $this->_left_angle = $left_angle;
    $this->_name = $name;
    $this->_attributes = $attributes;
    $this->_right_angle = $right_angle;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function attributes(): EditableSyntax {
    return $this->_attributes;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_left_angle(EditableSyntax $left_angle): XHPOpen {
    return new XHPOpen(
      $left_angle,
      $this->_name,
      $this->_attributes,
      $this->_right_angle);
  }
  public function with_name(EditableSyntax $name): XHPOpen {
    return new XHPOpen(
      $this->_left_angle,
      $name,
      $this->_attributes,
      $this->_right_angle);
  }
  public function with_attributes(EditableSyntax $attributes): XHPOpen {
    return new XHPOpen(
      $this->_left_angle,
      $this->_name,
      $attributes,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): XHPOpen {
    return new XHPOpen(
      $this->_left_angle,
      $this->_name,
      $this->_attributes,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $attributes = $this->attributes()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $left_angle === $this->left_angle() &&
      $name === $this->name() &&
      $attributes === $this->attributes() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPOpen(
        $left_angle,
        $name,
        $attributes,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_angle = EditableSyntax::from_json(
      $json->xhp_open_left_angle, $position, $source);
    $position += $left_angle->width();
    $name = EditableSyntax::from_json(
      $json->xhp_open_name, $position, $source);
    $position += $name->width();
    $attributes = EditableSyntax::from_json(
      $json->xhp_open_attributes, $position, $source);
    $position += $attributes->width();
    $right_angle = EditableSyntax::from_json(
      $json->xhp_open_right_angle, $position, $source);
    $position += $right_angle->width();
    return new XHPOpen(
        $left_angle,
        $name,
        $attributes,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_angle;
    yield $this->_name;
    yield $this->_attributes;
    yield $this->_right_angle;
    yield break;
  }
}
final class XHPExpression extends EditableSyntax {
  private EditableSyntax $_open;
  private EditableSyntax $_body;
  private EditableSyntax $_close;
  public function __construct(
    EditableSyntax $open,
    EditableSyntax $body,
    EditableSyntax $close) {
    parent::__construct('xhp_expression');
    $this->_open = $open;
    $this->_body = $body;
    $this->_close = $close;
  }
  public function open(): EditableSyntax {
    return $this->_open;
  }
  public function body(): EditableSyntax {
    return $this->_body;
  }
  public function close(): EditableSyntax {
    return $this->_close;
  }
  public function with_open(EditableSyntax $open): XHPExpression {
    return new XHPExpression(
      $open,
      $this->_body,
      $this->_close);
  }
  public function with_body(EditableSyntax $body): XHPExpression {
    return new XHPExpression(
      $this->_open,
      $body,
      $this->_close);
  }
  public function with_close(EditableSyntax $close): XHPExpression {
    return new XHPExpression(
      $this->_open,
      $this->_body,
      $close);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $open = $this->open()->rewrite($rewriter, $new_parents);
    $body = $this->body()->rewrite($rewriter, $new_parents);
    $close = $this->close()->rewrite($rewriter, $new_parents);
    if (
      $open === $this->open() &&
      $body === $this->body() &&
      $close === $this->close()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPExpression(
        $open,
        $body,
        $close), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $open = EditableSyntax::from_json(
      $json->xhp_open, $position, $source);
    $position += $open->width();
    $body = EditableSyntax::from_json(
      $json->xhp_body, $position, $source);
    $position += $body->width();
    $close = EditableSyntax::from_json(
      $json->xhp_close, $position, $source);
    $position += $close->width();
    return new XHPExpression(
        $open,
        $body,
        $close);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_open;
    yield $this->_body;
    yield $this->_close;
    yield break;
  }
}
final class XHPClose extends EditableSyntax {
  private EditableSyntax $_left_angle;
  private EditableSyntax $_name;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $left_angle,
    EditableSyntax $name,
    EditableSyntax $right_angle) {
    parent::__construct('xhp_close');
    $this->_left_angle = $left_angle;
    $this->_name = $name;
    $this->_right_angle = $right_angle;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_left_angle(EditableSyntax $left_angle): XHPClose {
    return new XHPClose(
      $left_angle,
      $this->_name,
      $this->_right_angle);
  }
  public function with_name(EditableSyntax $name): XHPClose {
    return new XHPClose(
      $this->_left_angle,
      $name,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): XHPClose {
    return new XHPClose(
      $this->_left_angle,
      $this->_name,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $left_angle === $this->left_angle() &&
      $name === $this->name() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new XHPClose(
        $left_angle,
        $name,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_angle = EditableSyntax::from_json(
      $json->xhp_close_left_angle, $position, $source);
    $position += $left_angle->width();
    $name = EditableSyntax::from_json(
      $json->xhp_close_name, $position, $source);
    $position += $name->width();
    $right_angle = EditableSyntax::from_json(
      $json->xhp_close_right_angle, $position, $source);
    $position += $right_angle->width();
    return new XHPClose(
        $left_angle,
        $name,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_angle;
    yield $this->_name;
    yield $this->_right_angle;
    yield break;
  }
}
final class TypeConstant extends EditableSyntax {
  private EditableSyntax $_left_type;
  private EditableSyntax $_separator;
  private EditableSyntax $_right_type;
  public function __construct(
    EditableSyntax $left_type,
    EditableSyntax $separator,
    EditableSyntax $right_type) {
    parent::__construct('type_constant');
    $this->_left_type = $left_type;
    $this->_separator = $separator;
    $this->_right_type = $right_type;
  }
  public function left_type(): EditableSyntax {
    return $this->_left_type;
  }
  public function separator(): EditableSyntax {
    return $this->_separator;
  }
  public function right_type(): EditableSyntax {
    return $this->_right_type;
  }
  public function with_left_type(EditableSyntax $left_type): TypeConstant {
    return new TypeConstant(
      $left_type,
      $this->_separator,
      $this->_right_type);
  }
  public function with_separator(EditableSyntax $separator): TypeConstant {
    return new TypeConstant(
      $this->_left_type,
      $separator,
      $this->_right_type);
  }
  public function with_right_type(EditableSyntax $right_type): TypeConstant {
    return new TypeConstant(
      $this->_left_type,
      $this->_separator,
      $right_type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_type = $this->left_type()->rewrite($rewriter, $new_parents);
    $separator = $this->separator()->rewrite($rewriter, $new_parents);
    $right_type = $this->right_type()->rewrite($rewriter, $new_parents);
    if (
      $left_type === $this->left_type() &&
      $separator === $this->separator() &&
      $right_type === $this->right_type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeConstant(
        $left_type,
        $separator,
        $right_type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_type = EditableSyntax::from_json(
      $json->type_constant_left_type, $position, $source);
    $position += $left_type->width();
    $separator = EditableSyntax::from_json(
      $json->type_constant_separator, $position, $source);
    $position += $separator->width();
    $right_type = EditableSyntax::from_json(
      $json->type_constant_right_type, $position, $source);
    $position += $right_type->width();
    return new TypeConstant(
        $left_type,
        $separator,
        $right_type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_type;
    yield $this->_separator;
    yield $this->_right_type;
    yield break;
  }
}
final class VectorTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_type;
  private EditableSyntax $_trailing_comma;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $type,
    EditableSyntax $trailing_comma,
    EditableSyntax $right_angle) {
    parent::__construct('vector_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_type = $type;
    $this->_trailing_comma = $trailing_comma;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function trailing_comma(): EditableSyntax {
    return $this->_trailing_comma;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): VectorTypeSpecifier {
    return new VectorTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): VectorTypeSpecifier {
    return new VectorTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_type(EditableSyntax $type): VectorTypeSpecifier {
    return new VectorTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_trailing_comma(EditableSyntax $trailing_comma): VectorTypeSpecifier {
    return new VectorTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $trailing_comma,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): VectorTypeSpecifier {
    return new VectorTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $trailing_comma = $this->trailing_comma()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $type === $this->type() &&
      $trailing_comma === $this->trailing_comma() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VectorTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->vector_type_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->vector_type_left_angle, $position, $source);
    $position += $left_angle->width();
    $type = EditableSyntax::from_json(
      $json->vector_type_type, $position, $source);
    $position += $type->width();
    $trailing_comma = EditableSyntax::from_json(
      $json->vector_type_trailing_comma, $position, $source);
    $position += $trailing_comma->width();
    $right_angle = EditableSyntax::from_json(
      $json->vector_type_right_angle, $position, $source);
    $position += $right_angle->width();
    return new VectorTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_type;
    yield $this->_trailing_comma;
    yield $this->_right_angle;
    yield break;
  }
}
final class KeysetTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_type;
  private EditableSyntax $_trailing_comma;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $type,
    EditableSyntax $trailing_comma,
    EditableSyntax $right_angle) {
    parent::__construct('keyset_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_type = $type;
    $this->_trailing_comma = $trailing_comma;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function trailing_comma(): EditableSyntax {
    return $this->_trailing_comma;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): KeysetTypeSpecifier {
    return new KeysetTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): KeysetTypeSpecifier {
    return new KeysetTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_type(EditableSyntax $type): KeysetTypeSpecifier {
    return new KeysetTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_trailing_comma(EditableSyntax $trailing_comma): KeysetTypeSpecifier {
    return new KeysetTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $trailing_comma,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): KeysetTypeSpecifier {
    return new KeysetTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $trailing_comma = $this->trailing_comma()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $type === $this->type() &&
      $trailing_comma === $this->trailing_comma() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new KeysetTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->keyset_type_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->keyset_type_left_angle, $position, $source);
    $position += $left_angle->width();
    $type = EditableSyntax::from_json(
      $json->keyset_type_type, $position, $source);
    $position += $type->width();
    $trailing_comma = EditableSyntax::from_json(
      $json->keyset_type_trailing_comma, $position, $source);
    $position += $trailing_comma->width();
    $right_angle = EditableSyntax::from_json(
      $json->keyset_type_right_angle, $position, $source);
    $position += $right_angle->width();
    return new KeysetTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_type;
    yield $this->_trailing_comma;
    yield $this->_right_angle;
    yield break;
  }
}
final class TupleTypeExplicitSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_types;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $types,
    EditableSyntax $right_angle) {
    parent::__construct('tuple_type_explicit_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_types = $types;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function types(): EditableSyntax {
    return $this->_types;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): TupleTypeExplicitSpecifier {
    return new TupleTypeExplicitSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_types,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): TupleTypeExplicitSpecifier {
    return new TupleTypeExplicitSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_types,
      $this->_right_angle);
  }
  public function with_types(EditableSyntax $types): TupleTypeExplicitSpecifier {
    return new TupleTypeExplicitSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $types,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): TupleTypeExplicitSpecifier {
    return new TupleTypeExplicitSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_types,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $types = $this->types()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $types === $this->types() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TupleTypeExplicitSpecifier(
        $keyword,
        $left_angle,
        $types,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->tuple_type_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->tuple_type_left_angle, $position, $source);
    $position += $left_angle->width();
    $types = EditableSyntax::from_json(
      $json->tuple_type_types, $position, $source);
    $position += $types->width();
    $right_angle = EditableSyntax::from_json(
      $json->tuple_type_right_angle, $position, $source);
    $position += $right_angle->width();
    return new TupleTypeExplicitSpecifier(
        $keyword,
        $left_angle,
        $types,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_types;
    yield $this->_right_angle;
    yield break;
  }
}
final class VarrayTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_type;
  private EditableSyntax $_trailing_comma;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $type,
    EditableSyntax $trailing_comma,
    EditableSyntax $right_angle) {
    parent::__construct('varray_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_type = $type;
    $this->_trailing_comma = $trailing_comma;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function trailing_comma(): EditableSyntax {
    return $this->_trailing_comma;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): VarrayTypeSpecifier {
    return new VarrayTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): VarrayTypeSpecifier {
    return new VarrayTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_type(EditableSyntax $type): VarrayTypeSpecifier {
    return new VarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_trailing_comma(EditableSyntax $trailing_comma): VarrayTypeSpecifier {
    return new VarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $trailing_comma,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): VarrayTypeSpecifier {
    return new VarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $trailing_comma = $this->trailing_comma()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $type === $this->type() &&
      $trailing_comma === $this->trailing_comma() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VarrayTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->varray_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->varray_left_angle, $position, $source);
    $position += $left_angle->width();
    $type = EditableSyntax::from_json(
      $json->varray_type, $position, $source);
    $position += $type->width();
    $trailing_comma = EditableSyntax::from_json(
      $json->varray_trailing_comma, $position, $source);
    $position += $trailing_comma->width();
    $right_angle = EditableSyntax::from_json(
      $json->varray_right_angle, $position, $source);
    $position += $right_angle->width();
    return new VarrayTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_type;
    yield $this->_trailing_comma;
    yield $this->_right_angle;
    yield break;
  }
}
final class VectorArrayTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_type;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $type,
    EditableSyntax $right_angle) {
    parent::__construct('vector_array_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_type = $type;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): VectorArrayTypeSpecifier {
    return new VectorArrayTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_type,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): VectorArrayTypeSpecifier {
    return new VectorArrayTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_type,
      $this->_right_angle);
  }
  public function with_type(EditableSyntax $type): VectorArrayTypeSpecifier {
    return new VectorArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $type,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): VectorArrayTypeSpecifier {
    return new VectorArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $type === $this->type() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new VectorArrayTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->vector_array_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->vector_array_left_angle, $position, $source);
    $position += $left_angle->width();
    $type = EditableSyntax::from_json(
      $json->vector_array_type, $position, $source);
    $position += $type->width();
    $right_angle = EditableSyntax::from_json(
      $json->vector_array_right_angle, $position, $source);
    $position += $right_angle->width();
    return new VectorArrayTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_type;
    yield $this->_right_angle;
    yield break;
  }
}
final class TypeParameter extends EditableSyntax {
  private EditableSyntax $_variance;
  private EditableSyntax $_name;
  private EditableSyntax $_constraints;
  public function __construct(
    EditableSyntax $variance,
    EditableSyntax $name,
    EditableSyntax $constraints) {
    parent::__construct('type_parameter');
    $this->_variance = $variance;
    $this->_name = $name;
    $this->_constraints = $constraints;
  }
  public function variance(): EditableSyntax {
    return $this->_variance;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function constraints(): EditableSyntax {
    return $this->_constraints;
  }
  public function with_variance(EditableSyntax $variance): TypeParameter {
    return new TypeParameter(
      $variance,
      $this->_name,
      $this->_constraints);
  }
  public function with_name(EditableSyntax $name): TypeParameter {
    return new TypeParameter(
      $this->_variance,
      $name,
      $this->_constraints);
  }
  public function with_constraints(EditableSyntax $constraints): TypeParameter {
    return new TypeParameter(
      $this->_variance,
      $this->_name,
      $constraints);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $variance = $this->variance()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $constraints = $this->constraints()->rewrite($rewriter, $new_parents);
    if (
      $variance === $this->variance() &&
      $name === $this->name() &&
      $constraints === $this->constraints()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeParameter(
        $variance,
        $name,
        $constraints), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $variance = EditableSyntax::from_json(
      $json->type_variance, $position, $source);
    $position += $variance->width();
    $name = EditableSyntax::from_json(
      $json->type_name, $position, $source);
    $position += $name->width();
    $constraints = EditableSyntax::from_json(
      $json->type_constraints, $position, $source);
    $position += $constraints->width();
    return new TypeParameter(
        $variance,
        $name,
        $constraints);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_variance;
    yield $this->_name;
    yield $this->_constraints;
    yield break;
  }
}
final class TypeConstraint extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $type) {
    parent::__construct('type_constraint');
    $this->_keyword = $keyword;
    $this->_type = $type;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_keyword(EditableSyntax $keyword): TypeConstraint {
    return new TypeConstraint(
      $keyword,
      $this->_type);
  }
  public function with_type(EditableSyntax $type): TypeConstraint {
    return new TypeConstraint(
      $this->_keyword,
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeConstraint(
        $keyword,
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->constraint_keyword, $position, $source);
    $position += $keyword->width();
    $type = EditableSyntax::from_json(
      $json->constraint_type, $position, $source);
    $position += $type->width();
    return new TypeConstraint(
        $keyword,
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_type;
    yield break;
  }
}
final class DarrayTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_key;
  private EditableSyntax $_comma;
  private EditableSyntax $_value;
  private EditableSyntax $_trailing_comma;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $key,
    EditableSyntax $comma,
    EditableSyntax $value,
    EditableSyntax $trailing_comma,
    EditableSyntax $right_angle) {
    parent::__construct('darray_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_key = $key;
    $this->_comma = $comma;
    $this->_value = $value;
    $this->_trailing_comma = $trailing_comma;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function key(): EditableSyntax {
    return $this->_key;
  }
  public function comma(): EditableSyntax {
    return $this->_comma;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function trailing_comma(): EditableSyntax {
    return $this->_trailing_comma;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_key(EditableSyntax $key): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $key,
      $this->_comma,
      $this->_value,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_comma(EditableSyntax $comma): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $comma,
      $this->_value,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_value(EditableSyntax $value): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $value,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_trailing_comma(EditableSyntax $trailing_comma): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $trailing_comma,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): DarrayTypeSpecifier {
    return new DarrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $this->_trailing_comma,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $key = $this->key()->rewrite($rewriter, $new_parents);
    $comma = $this->comma()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    $trailing_comma = $this->trailing_comma()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $key === $this->key() &&
      $comma === $this->comma() &&
      $value === $this->value() &&
      $trailing_comma === $this->trailing_comma() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DarrayTypeSpecifier(
        $keyword,
        $left_angle,
        $key,
        $comma,
        $value,
        $trailing_comma,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->darray_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->darray_left_angle, $position, $source);
    $position += $left_angle->width();
    $key = EditableSyntax::from_json(
      $json->darray_key, $position, $source);
    $position += $key->width();
    $comma = EditableSyntax::from_json(
      $json->darray_comma, $position, $source);
    $position += $comma->width();
    $value = EditableSyntax::from_json(
      $json->darray_value, $position, $source);
    $position += $value->width();
    $trailing_comma = EditableSyntax::from_json(
      $json->darray_trailing_comma, $position, $source);
    $position += $trailing_comma->width();
    $right_angle = EditableSyntax::from_json(
      $json->darray_right_angle, $position, $source);
    $position += $right_angle->width();
    return new DarrayTypeSpecifier(
        $keyword,
        $left_angle,
        $key,
        $comma,
        $value,
        $trailing_comma,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_key;
    yield $this->_comma;
    yield $this->_value;
    yield $this->_trailing_comma;
    yield $this->_right_angle;
    yield break;
  }
}
final class MapArrayTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_key;
  private EditableSyntax $_comma;
  private EditableSyntax $_value;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $key,
    EditableSyntax $comma,
    EditableSyntax $value,
    EditableSyntax $right_angle) {
    parent::__construct('map_array_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_key = $key;
    $this->_comma = $comma;
    $this->_value = $value;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function key(): EditableSyntax {
    return $this->_key;
  }
  public function comma(): EditableSyntax {
    return $this->_comma;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $this->_right_angle);
  }
  public function with_key(EditableSyntax $key): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $key,
      $this->_comma,
      $this->_value,
      $this->_right_angle);
  }
  public function with_comma(EditableSyntax $comma): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $comma,
      $this->_value,
      $this->_right_angle);
  }
  public function with_value(EditableSyntax $value): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $value,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): MapArrayTypeSpecifier {
    return new MapArrayTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_key,
      $this->_comma,
      $this->_value,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $key = $this->key()->rewrite($rewriter, $new_parents);
    $comma = $this->comma()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $key === $this->key() &&
      $comma === $this->comma() &&
      $value === $this->value() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new MapArrayTypeSpecifier(
        $keyword,
        $left_angle,
        $key,
        $comma,
        $value,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->map_array_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->map_array_left_angle, $position, $source);
    $position += $left_angle->width();
    $key = EditableSyntax::from_json(
      $json->map_array_key, $position, $source);
    $position += $key->width();
    $comma = EditableSyntax::from_json(
      $json->map_array_comma, $position, $source);
    $position += $comma->width();
    $value = EditableSyntax::from_json(
      $json->map_array_value, $position, $source);
    $position += $value->width();
    $right_angle = EditableSyntax::from_json(
      $json->map_array_right_angle, $position, $source);
    $position += $right_angle->width();
    return new MapArrayTypeSpecifier(
        $keyword,
        $left_angle,
        $key,
        $comma,
        $value,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_key;
    yield $this->_comma;
    yield $this->_value;
    yield $this->_right_angle;
    yield break;
  }
}
final class DictionaryTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_members;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $members,
    EditableSyntax $right_angle) {
    parent::__construct('dictionary_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_members = $members;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function members(): EditableSyntax {
    return $this->_members;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): DictionaryTypeSpecifier {
    return new DictionaryTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_members,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): DictionaryTypeSpecifier {
    return new DictionaryTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_members,
      $this->_right_angle);
  }
  public function with_members(EditableSyntax $members): DictionaryTypeSpecifier {
    return new DictionaryTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $members,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): DictionaryTypeSpecifier {
    return new DictionaryTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_members,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $members = $this->members()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $members === $this->members() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new DictionaryTypeSpecifier(
        $keyword,
        $left_angle,
        $members,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->dictionary_type_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->dictionary_type_left_angle, $position, $source);
    $position += $left_angle->width();
    $members = EditableSyntax::from_json(
      $json->dictionary_type_members, $position, $source);
    $position += $members->width();
    $right_angle = EditableSyntax::from_json(
      $json->dictionary_type_right_angle, $position, $source);
    $position += $right_angle->width();
    return new DictionaryTypeSpecifier(
        $keyword,
        $left_angle,
        $members,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_members;
    yield $this->_right_angle;
    yield break;
  }
}
final class ClosureTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_outer_left_paren;
  private EditableSyntax $_coroutine;
  private EditableSyntax $_function_keyword;
  private EditableSyntax $_inner_left_paren;
  private EditableSyntax $_parameter_types;
  private EditableSyntax $_inner_right_paren;
  private EditableSyntax $_colon;
  private EditableSyntax $_return_type;
  private EditableSyntax $_outer_right_paren;
  public function __construct(
    EditableSyntax $outer_left_paren,
    EditableSyntax $coroutine,
    EditableSyntax $function_keyword,
    EditableSyntax $inner_left_paren,
    EditableSyntax $parameter_types,
    EditableSyntax $inner_right_paren,
    EditableSyntax $colon,
    EditableSyntax $return_type,
    EditableSyntax $outer_right_paren) {
    parent::__construct('closure_type_specifier');
    $this->_outer_left_paren = $outer_left_paren;
    $this->_coroutine = $coroutine;
    $this->_function_keyword = $function_keyword;
    $this->_inner_left_paren = $inner_left_paren;
    $this->_parameter_types = $parameter_types;
    $this->_inner_right_paren = $inner_right_paren;
    $this->_colon = $colon;
    $this->_return_type = $return_type;
    $this->_outer_right_paren = $outer_right_paren;
  }
  public function outer_left_paren(): EditableSyntax {
    return $this->_outer_left_paren;
  }
  public function coroutine(): EditableSyntax {
    return $this->_coroutine;
  }
  public function function_keyword(): EditableSyntax {
    return $this->_function_keyword;
  }
  public function inner_left_paren(): EditableSyntax {
    return $this->_inner_left_paren;
  }
  public function parameter_types(): EditableSyntax {
    return $this->_parameter_types;
  }
  public function inner_right_paren(): EditableSyntax {
    return $this->_inner_right_paren;
  }
  public function colon(): EditableSyntax {
    return $this->_colon;
  }
  public function return_type(): EditableSyntax {
    return $this->_return_type;
  }
  public function outer_right_paren(): EditableSyntax {
    return $this->_outer_right_paren;
  }
  public function with_outer_left_paren(EditableSyntax $outer_left_paren): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_coroutine(EditableSyntax $coroutine): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_function_keyword(EditableSyntax $function_keyword): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_inner_left_paren(EditableSyntax $inner_left_paren): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_parameter_types(EditableSyntax $parameter_types): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_inner_right_paren(EditableSyntax $inner_right_paren): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_colon(EditableSyntax $colon): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $colon,
      $this->_return_type,
      $this->_outer_right_paren);
  }
  public function with_return_type(EditableSyntax $return_type): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $return_type,
      $this->_outer_right_paren);
  }
  public function with_outer_right_paren(EditableSyntax $outer_right_paren): ClosureTypeSpecifier {
    return new ClosureTypeSpecifier(
      $this->_outer_left_paren,
      $this->_coroutine,
      $this->_function_keyword,
      $this->_inner_left_paren,
      $this->_parameter_types,
      $this->_inner_right_paren,
      $this->_colon,
      $this->_return_type,
      $outer_right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $outer_left_paren = $this->outer_left_paren()->rewrite($rewriter, $new_parents);
    $coroutine = $this->coroutine()->rewrite($rewriter, $new_parents);
    $function_keyword = $this->function_keyword()->rewrite($rewriter, $new_parents);
    $inner_left_paren = $this->inner_left_paren()->rewrite($rewriter, $new_parents);
    $parameter_types = $this->parameter_types()->rewrite($rewriter, $new_parents);
    $inner_right_paren = $this->inner_right_paren()->rewrite($rewriter, $new_parents);
    $colon = $this->colon()->rewrite($rewriter, $new_parents);
    $return_type = $this->return_type()->rewrite($rewriter, $new_parents);
    $outer_right_paren = $this->outer_right_paren()->rewrite($rewriter, $new_parents);
    if (
      $outer_left_paren === $this->outer_left_paren() &&
      $coroutine === $this->coroutine() &&
      $function_keyword === $this->function_keyword() &&
      $inner_left_paren === $this->inner_left_paren() &&
      $parameter_types === $this->parameter_types() &&
      $inner_right_paren === $this->inner_right_paren() &&
      $colon === $this->colon() &&
      $return_type === $this->return_type() &&
      $outer_right_paren === $this->outer_right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ClosureTypeSpecifier(
        $outer_left_paren,
        $coroutine,
        $function_keyword,
        $inner_left_paren,
        $parameter_types,
        $inner_right_paren,
        $colon,
        $return_type,
        $outer_right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $outer_left_paren = EditableSyntax::from_json(
      $json->closure_outer_left_paren, $position, $source);
    $position += $outer_left_paren->width();
    $coroutine = EditableSyntax::from_json(
      $json->closure_coroutine, $position, $source);
    $position += $coroutine->width();
    $function_keyword = EditableSyntax::from_json(
      $json->closure_function_keyword, $position, $source);
    $position += $function_keyword->width();
    $inner_left_paren = EditableSyntax::from_json(
      $json->closure_inner_left_paren, $position, $source);
    $position += $inner_left_paren->width();
    $parameter_types = EditableSyntax::from_json(
      $json->closure_parameter_types, $position, $source);
    $position += $parameter_types->width();
    $inner_right_paren = EditableSyntax::from_json(
      $json->closure_inner_right_paren, $position, $source);
    $position += $inner_right_paren->width();
    $colon = EditableSyntax::from_json(
      $json->closure_colon, $position, $source);
    $position += $colon->width();
    $return_type = EditableSyntax::from_json(
      $json->closure_return_type, $position, $source);
    $position += $return_type->width();
    $outer_right_paren = EditableSyntax::from_json(
      $json->closure_outer_right_paren, $position, $source);
    $position += $outer_right_paren->width();
    return new ClosureTypeSpecifier(
        $outer_left_paren,
        $coroutine,
        $function_keyword,
        $inner_left_paren,
        $parameter_types,
        $inner_right_paren,
        $colon,
        $return_type,
        $outer_right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_outer_left_paren;
    yield $this->_coroutine;
    yield $this->_function_keyword;
    yield $this->_inner_left_paren;
    yield $this->_parameter_types;
    yield $this->_inner_right_paren;
    yield $this->_colon;
    yield $this->_return_type;
    yield $this->_outer_right_paren;
    yield break;
  }
}
final class ClassnameTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_angle;
  private EditableSyntax $_type;
  private EditableSyntax $_trailing_comma;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_angle,
    EditableSyntax $type,
    EditableSyntax $trailing_comma,
    EditableSyntax $right_angle) {
    parent::__construct('classname_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_angle = $left_angle;
    $this->_type = $type;
    $this->_trailing_comma = $trailing_comma;
    $this->_right_angle = $right_angle;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function trailing_comma(): EditableSyntax {
    return $this->_trailing_comma;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_keyword(EditableSyntax $keyword): ClassnameTypeSpecifier {
    return new ClassnameTypeSpecifier(
      $keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_left_angle(EditableSyntax $left_angle): ClassnameTypeSpecifier {
    return new ClassnameTypeSpecifier(
      $this->_keyword,
      $left_angle,
      $this->_type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_type(EditableSyntax $type): ClassnameTypeSpecifier {
    return new ClassnameTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $type,
      $this->_trailing_comma,
      $this->_right_angle);
  }
  public function with_trailing_comma(EditableSyntax $trailing_comma): ClassnameTypeSpecifier {
    return new ClassnameTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $trailing_comma,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): ClassnameTypeSpecifier {
    return new ClassnameTypeSpecifier(
      $this->_keyword,
      $this->_left_angle,
      $this->_type,
      $this->_trailing_comma,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    $trailing_comma = $this->trailing_comma()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_angle === $this->left_angle() &&
      $type === $this->type() &&
      $trailing_comma === $this->trailing_comma() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ClassnameTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->classname_keyword, $position, $source);
    $position += $keyword->width();
    $left_angle = EditableSyntax::from_json(
      $json->classname_left_angle, $position, $source);
    $position += $left_angle->width();
    $type = EditableSyntax::from_json(
      $json->classname_type, $position, $source);
    $position += $type->width();
    $trailing_comma = EditableSyntax::from_json(
      $json->classname_trailing_comma, $position, $source);
    $position += $trailing_comma->width();
    $right_angle = EditableSyntax::from_json(
      $json->classname_right_angle, $position, $source);
    $position += $right_angle->width();
    return new ClassnameTypeSpecifier(
        $keyword,
        $left_angle,
        $type,
        $trailing_comma,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_angle;
    yield $this->_type;
    yield $this->_trailing_comma;
    yield $this->_right_angle;
    yield break;
  }
}
final class FieldSpecifier extends EditableSyntax {
  private EditableSyntax $_question;
  private EditableSyntax $_name;
  private EditableSyntax $_arrow;
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $question,
    EditableSyntax $name,
    EditableSyntax $arrow,
    EditableSyntax $type) {
    parent::__construct('field_specifier');
    $this->_question = $question;
    $this->_name = $name;
    $this->_arrow = $arrow;
    $this->_type = $type;
  }
  public function question(): EditableSyntax {
    return $this->_question;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function arrow(): EditableSyntax {
    return $this->_arrow;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_question(EditableSyntax $question): FieldSpecifier {
    return new FieldSpecifier(
      $question,
      $this->_name,
      $this->_arrow,
      $this->_type);
  }
  public function with_name(EditableSyntax $name): FieldSpecifier {
    return new FieldSpecifier(
      $this->_question,
      $name,
      $this->_arrow,
      $this->_type);
  }
  public function with_arrow(EditableSyntax $arrow): FieldSpecifier {
    return new FieldSpecifier(
      $this->_question,
      $this->_name,
      $arrow,
      $this->_type);
  }
  public function with_type(EditableSyntax $type): FieldSpecifier {
    return new FieldSpecifier(
      $this->_question,
      $this->_name,
      $this->_arrow,
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $question = $this->question()->rewrite($rewriter, $new_parents);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $arrow = $this->arrow()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $question === $this->question() &&
      $name === $this->name() &&
      $arrow === $this->arrow() &&
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FieldSpecifier(
        $question,
        $name,
        $arrow,
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $question = EditableSyntax::from_json(
      $json->field_question, $position, $source);
    $position += $question->width();
    $name = EditableSyntax::from_json(
      $json->field_name, $position, $source);
    $position += $name->width();
    $arrow = EditableSyntax::from_json(
      $json->field_arrow, $position, $source);
    $position += $arrow->width();
    $type = EditableSyntax::from_json(
      $json->field_type, $position, $source);
    $position += $type->width();
    return new FieldSpecifier(
        $question,
        $name,
        $arrow,
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_question;
    yield $this->_name;
    yield $this->_arrow;
    yield $this->_type;
    yield break;
  }
}
final class FieldInitializer extends EditableSyntax {
  private EditableSyntax $_name;
  private EditableSyntax $_arrow;
  private EditableSyntax $_value;
  public function __construct(
    EditableSyntax $name,
    EditableSyntax $arrow,
    EditableSyntax $value) {
    parent::__construct('field_initializer');
    $this->_name = $name;
    $this->_arrow = $arrow;
    $this->_value = $value;
  }
  public function name(): EditableSyntax {
    return $this->_name;
  }
  public function arrow(): EditableSyntax {
    return $this->_arrow;
  }
  public function value(): EditableSyntax {
    return $this->_value;
  }
  public function with_name(EditableSyntax $name): FieldInitializer {
    return new FieldInitializer(
      $name,
      $this->_arrow,
      $this->_value);
  }
  public function with_arrow(EditableSyntax $arrow): FieldInitializer {
    return new FieldInitializer(
      $this->_name,
      $arrow,
      $this->_value);
  }
  public function with_value(EditableSyntax $value): FieldInitializer {
    return new FieldInitializer(
      $this->_name,
      $this->_arrow,
      $value);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $name = $this->name()->rewrite($rewriter, $new_parents);
    $arrow = $this->arrow()->rewrite($rewriter, $new_parents);
    $value = $this->value()->rewrite($rewriter, $new_parents);
    if (
      $name === $this->name() &&
      $arrow === $this->arrow() &&
      $value === $this->value()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new FieldInitializer(
        $name,
        $arrow,
        $value), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $name = EditableSyntax::from_json(
      $json->field_initializer_name, $position, $source);
    $position += $name->width();
    $arrow = EditableSyntax::from_json(
      $json->field_initializer_arrow, $position, $source);
    $position += $arrow->width();
    $value = EditableSyntax::from_json(
      $json->field_initializer_value, $position, $source);
    $position += $value->width();
    return new FieldInitializer(
        $name,
        $arrow,
        $value);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_name;
    yield $this->_arrow;
    yield $this->_value;
    yield break;
  }
}
final class ShapeTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_fields;
  private EditableSyntax $_ellipsis;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $fields,
    EditableSyntax $ellipsis,
    EditableSyntax $right_paren) {
    parent::__construct('shape_type_specifier');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_fields = $fields;
    $this->_ellipsis = $ellipsis;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function fields(): EditableSyntax {
    return $this->_fields;
  }
  public function ellipsis(): EditableSyntax {
    return $this->_ellipsis;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): ShapeTypeSpecifier {
    return new ShapeTypeSpecifier(
      $keyword,
      $this->_left_paren,
      $this->_fields,
      $this->_ellipsis,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): ShapeTypeSpecifier {
    return new ShapeTypeSpecifier(
      $this->_keyword,
      $left_paren,
      $this->_fields,
      $this->_ellipsis,
      $this->_right_paren);
  }
  public function with_fields(EditableSyntax $fields): ShapeTypeSpecifier {
    return new ShapeTypeSpecifier(
      $this->_keyword,
      $this->_left_paren,
      $fields,
      $this->_ellipsis,
      $this->_right_paren);
  }
  public function with_ellipsis(EditableSyntax $ellipsis): ShapeTypeSpecifier {
    return new ShapeTypeSpecifier(
      $this->_keyword,
      $this->_left_paren,
      $this->_fields,
      $ellipsis,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ShapeTypeSpecifier {
    return new ShapeTypeSpecifier(
      $this->_keyword,
      $this->_left_paren,
      $this->_fields,
      $this->_ellipsis,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $fields = $this->fields()->rewrite($rewriter, $new_parents);
    $ellipsis = $this->ellipsis()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $fields === $this->fields() &&
      $ellipsis === $this->ellipsis() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ShapeTypeSpecifier(
        $keyword,
        $left_paren,
        $fields,
        $ellipsis,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->shape_type_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->shape_type_left_paren, $position, $source);
    $position += $left_paren->width();
    $fields = EditableSyntax::from_json(
      $json->shape_type_fields, $position, $source);
    $position += $fields->width();
    $ellipsis = EditableSyntax::from_json(
      $json->shape_type_ellipsis, $position, $source);
    $position += $ellipsis->width();
    $right_paren = EditableSyntax::from_json(
      $json->shape_type_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ShapeTypeSpecifier(
        $keyword,
        $left_paren,
        $fields,
        $ellipsis,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_fields;
    yield $this->_ellipsis;
    yield $this->_right_paren;
    yield break;
  }
}
final class ShapeExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_fields;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $fields,
    EditableSyntax $right_paren) {
    parent::__construct('shape_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_fields = $fields;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function fields(): EditableSyntax {
    return $this->_fields;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): ShapeExpression {
    return new ShapeExpression(
      $keyword,
      $this->_left_paren,
      $this->_fields,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): ShapeExpression {
    return new ShapeExpression(
      $this->_keyword,
      $left_paren,
      $this->_fields,
      $this->_right_paren);
  }
  public function with_fields(EditableSyntax $fields): ShapeExpression {
    return new ShapeExpression(
      $this->_keyword,
      $this->_left_paren,
      $fields,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): ShapeExpression {
    return new ShapeExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_fields,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $fields = $this->fields()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $fields === $this->fields() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ShapeExpression(
        $keyword,
        $left_paren,
        $fields,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->shape_expression_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->shape_expression_left_paren, $position, $source);
    $position += $left_paren->width();
    $fields = EditableSyntax::from_json(
      $json->shape_expression_fields, $position, $source);
    $position += $fields->width();
    $right_paren = EditableSyntax::from_json(
      $json->shape_expression_right_paren, $position, $source);
    $position += $right_paren->width();
    return new ShapeExpression(
        $keyword,
        $left_paren,
        $fields,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_fields;
    yield $this->_right_paren;
    yield break;
  }
}
final class TupleExpression extends EditableSyntax {
  private EditableSyntax $_keyword;
  private EditableSyntax $_left_paren;
  private EditableSyntax $_items;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $keyword,
    EditableSyntax $left_paren,
    EditableSyntax $items,
    EditableSyntax $right_paren) {
    parent::__construct('tuple_expression');
    $this->_keyword = $keyword;
    $this->_left_paren = $left_paren;
    $this->_items = $items;
    $this->_right_paren = $right_paren;
  }
  public function keyword(): EditableSyntax {
    return $this->_keyword;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function items(): EditableSyntax {
    return $this->_items;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_keyword(EditableSyntax $keyword): TupleExpression {
    return new TupleExpression(
      $keyword,
      $this->_left_paren,
      $this->_items,
      $this->_right_paren);
  }
  public function with_left_paren(EditableSyntax $left_paren): TupleExpression {
    return new TupleExpression(
      $this->_keyword,
      $left_paren,
      $this->_items,
      $this->_right_paren);
  }
  public function with_items(EditableSyntax $items): TupleExpression {
    return new TupleExpression(
      $this->_keyword,
      $this->_left_paren,
      $items,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): TupleExpression {
    return new TupleExpression(
      $this->_keyword,
      $this->_left_paren,
      $this->_items,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $keyword = $this->keyword()->rewrite($rewriter, $new_parents);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $items = $this->items()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $keyword === $this->keyword() &&
      $left_paren === $this->left_paren() &&
      $items === $this->items() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TupleExpression(
        $keyword,
        $left_paren,
        $items,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $keyword = EditableSyntax::from_json(
      $json->tuple_expression_keyword, $position, $source);
    $position += $keyword->width();
    $left_paren = EditableSyntax::from_json(
      $json->tuple_expression_left_paren, $position, $source);
    $position += $left_paren->width();
    $items = EditableSyntax::from_json(
      $json->tuple_expression_items, $position, $source);
    $position += $items->width();
    $right_paren = EditableSyntax::from_json(
      $json->tuple_expression_right_paren, $position, $source);
    $position += $right_paren->width();
    return new TupleExpression(
        $keyword,
        $left_paren,
        $items,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_keyword;
    yield $this->_left_paren;
    yield $this->_items;
    yield $this->_right_paren;
    yield break;
  }
}
final class GenericTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_class_type;
  private EditableSyntax $_argument_list;
  public function __construct(
    EditableSyntax $class_type,
    EditableSyntax $argument_list) {
    parent::__construct('generic_type_specifier');
    $this->_class_type = $class_type;
    $this->_argument_list = $argument_list;
  }
  public function class_type(): EditableSyntax {
    return $this->_class_type;
  }
  public function argument_list(): EditableSyntax {
    return $this->_argument_list;
  }
  public function with_class_type(EditableSyntax $class_type): GenericTypeSpecifier {
    return new GenericTypeSpecifier(
      $class_type,
      $this->_argument_list);
  }
  public function with_argument_list(EditableSyntax $argument_list): GenericTypeSpecifier {
    return new GenericTypeSpecifier(
      $this->_class_type,
      $argument_list);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $class_type = $this->class_type()->rewrite($rewriter, $new_parents);
    $argument_list = $this->argument_list()->rewrite($rewriter, $new_parents);
    if (
      $class_type === $this->class_type() &&
      $argument_list === $this->argument_list()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new GenericTypeSpecifier(
        $class_type,
        $argument_list), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $class_type = EditableSyntax::from_json(
      $json->generic_class_type, $position, $source);
    $position += $class_type->width();
    $argument_list = EditableSyntax::from_json(
      $json->generic_argument_list, $position, $source);
    $position += $argument_list->width();
    return new GenericTypeSpecifier(
        $class_type,
        $argument_list);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_class_type;
    yield $this->_argument_list;
    yield break;
  }
}
final class NullableTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_question;
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $question,
    EditableSyntax $type) {
    parent::__construct('nullable_type_specifier');
    $this->_question = $question;
    $this->_type = $type;
  }
  public function question(): EditableSyntax {
    return $this->_question;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_question(EditableSyntax $question): NullableTypeSpecifier {
    return new NullableTypeSpecifier(
      $question,
      $this->_type);
  }
  public function with_type(EditableSyntax $type): NullableTypeSpecifier {
    return new NullableTypeSpecifier(
      $this->_question,
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $question = $this->question()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $question === $this->question() &&
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new NullableTypeSpecifier(
        $question,
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $question = EditableSyntax::from_json(
      $json->nullable_question, $position, $source);
    $position += $question->width();
    $type = EditableSyntax::from_json(
      $json->nullable_type, $position, $source);
    $position += $type->width();
    return new NullableTypeSpecifier(
        $question,
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_question;
    yield $this->_type;
    yield break;
  }
}
final class SoftTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_at;
  private EditableSyntax $_type;
  public function __construct(
    EditableSyntax $at,
    EditableSyntax $type) {
    parent::__construct('soft_type_specifier');
    $this->_at = $at;
    $this->_type = $type;
  }
  public function at(): EditableSyntax {
    return $this->_at;
  }
  public function type(): EditableSyntax {
    return $this->_type;
  }
  public function with_at(EditableSyntax $at): SoftTypeSpecifier {
    return new SoftTypeSpecifier(
      $at,
      $this->_type);
  }
  public function with_type(EditableSyntax $type): SoftTypeSpecifier {
    return new SoftTypeSpecifier(
      $this->_at,
      $type);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $at = $this->at()->rewrite($rewriter, $new_parents);
    $type = $this->type()->rewrite($rewriter, $new_parents);
    if (
      $at === $this->at() &&
      $type === $this->type()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new SoftTypeSpecifier(
        $at,
        $type), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $at = EditableSyntax::from_json(
      $json->soft_at, $position, $source);
    $position += $at->width();
    $type = EditableSyntax::from_json(
      $json->soft_type, $position, $source);
    $position += $type->width();
    return new SoftTypeSpecifier(
        $at,
        $type);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_at;
    yield $this->_type;
    yield break;
  }
}
final class TypeArguments extends EditableSyntax {
  private EditableSyntax $_left_angle;
  private EditableSyntax $_types;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $left_angle,
    EditableSyntax $types,
    EditableSyntax $right_angle) {
    parent::__construct('type_arguments');
    $this->_left_angle = $left_angle;
    $this->_types = $types;
    $this->_right_angle = $right_angle;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function types(): EditableSyntax {
    return $this->_types;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_left_angle(EditableSyntax $left_angle): TypeArguments {
    return new TypeArguments(
      $left_angle,
      $this->_types,
      $this->_right_angle);
  }
  public function with_types(EditableSyntax $types): TypeArguments {
    return new TypeArguments(
      $this->_left_angle,
      $types,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): TypeArguments {
    return new TypeArguments(
      $this->_left_angle,
      $this->_types,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $types = $this->types()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $left_angle === $this->left_angle() &&
      $types === $this->types() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeArguments(
        $left_angle,
        $types,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_angle = EditableSyntax::from_json(
      $json->type_arguments_left_angle, $position, $source);
    $position += $left_angle->width();
    $types = EditableSyntax::from_json(
      $json->type_arguments_types, $position, $source);
    $position += $types->width();
    $right_angle = EditableSyntax::from_json(
      $json->type_arguments_right_angle, $position, $source);
    $position += $right_angle->width();
    return new TypeArguments(
        $left_angle,
        $types,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_angle;
    yield $this->_types;
    yield $this->_right_angle;
    yield break;
  }
}
final class TypeParameters extends EditableSyntax {
  private EditableSyntax $_left_angle;
  private EditableSyntax $_parameters;
  private EditableSyntax $_right_angle;
  public function __construct(
    EditableSyntax $left_angle,
    EditableSyntax $parameters,
    EditableSyntax $right_angle) {
    parent::__construct('type_parameters');
    $this->_left_angle = $left_angle;
    $this->_parameters = $parameters;
    $this->_right_angle = $right_angle;
  }
  public function left_angle(): EditableSyntax {
    return $this->_left_angle;
  }
  public function parameters(): EditableSyntax {
    return $this->_parameters;
  }
  public function right_angle(): EditableSyntax {
    return $this->_right_angle;
  }
  public function with_left_angle(EditableSyntax $left_angle): TypeParameters {
    return new TypeParameters(
      $left_angle,
      $this->_parameters,
      $this->_right_angle);
  }
  public function with_parameters(EditableSyntax $parameters): TypeParameters {
    return new TypeParameters(
      $this->_left_angle,
      $parameters,
      $this->_right_angle);
  }
  public function with_right_angle(EditableSyntax $right_angle): TypeParameters {
    return new TypeParameters(
      $this->_left_angle,
      $this->_parameters,
      $right_angle);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_angle = $this->left_angle()->rewrite($rewriter, $new_parents);
    $parameters = $this->parameters()->rewrite($rewriter, $new_parents);
    $right_angle = $this->right_angle()->rewrite($rewriter, $new_parents);
    if (
      $left_angle === $this->left_angle() &&
      $parameters === $this->parameters() &&
      $right_angle === $this->right_angle()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TypeParameters(
        $left_angle,
        $parameters,
        $right_angle), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_angle = EditableSyntax::from_json(
      $json->type_parameters_left_angle, $position, $source);
    $position += $left_angle->width();
    $parameters = EditableSyntax::from_json(
      $json->type_parameters_parameters, $position, $source);
    $position += $parameters->width();
    $right_angle = EditableSyntax::from_json(
      $json->type_parameters_right_angle, $position, $source);
    $position += $right_angle->width();
    return new TypeParameters(
        $left_angle,
        $parameters,
        $right_angle);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_angle;
    yield $this->_parameters;
    yield $this->_right_angle;
    yield break;
  }
}
final class TupleTypeSpecifier extends EditableSyntax {
  private EditableSyntax $_left_paren;
  private EditableSyntax $_types;
  private EditableSyntax $_right_paren;
  public function __construct(
    EditableSyntax $left_paren,
    EditableSyntax $types,
    EditableSyntax $right_paren) {
    parent::__construct('tuple_type_specifier');
    $this->_left_paren = $left_paren;
    $this->_types = $types;
    $this->_right_paren = $right_paren;
  }
  public function left_paren(): EditableSyntax {
    return $this->_left_paren;
  }
  public function types(): EditableSyntax {
    return $this->_types;
  }
  public function right_paren(): EditableSyntax {
    return $this->_right_paren;
  }
  public function with_left_paren(EditableSyntax $left_paren): TupleTypeSpecifier {
    return new TupleTypeSpecifier(
      $left_paren,
      $this->_types,
      $this->_right_paren);
  }
  public function with_types(EditableSyntax $types): TupleTypeSpecifier {
    return new TupleTypeSpecifier(
      $this->_left_paren,
      $types,
      $this->_right_paren);
  }
  public function with_right_paren(EditableSyntax $right_paren): TupleTypeSpecifier {
    return new TupleTypeSpecifier(
      $this->_left_paren,
      $this->_types,
      $right_paren);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $left_paren = $this->left_paren()->rewrite($rewriter, $new_parents);
    $types = $this->types()->rewrite($rewriter, $new_parents);
    $right_paren = $this->right_paren()->rewrite($rewriter, $new_parents);
    if (
      $left_paren === $this->left_paren() &&
      $types === $this->types() &&
      $right_paren === $this->right_paren()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new TupleTypeSpecifier(
        $left_paren,
        $types,
        $right_paren), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $left_paren = EditableSyntax::from_json(
      $json->tuple_left_paren, $position, $source);
    $position += $left_paren->width();
    $types = EditableSyntax::from_json(
      $json->tuple_types, $position, $source);
    $position += $types->width();
    $right_paren = EditableSyntax::from_json(
      $json->tuple_right_paren, $position, $source);
    $position += $right_paren->width();
    return new TupleTypeSpecifier(
        $left_paren,
        $types,
        $right_paren);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_left_paren;
    yield $this->_types;
    yield $this->_right_paren;
    yield break;
  }
}
final class ErrorSyntax extends EditableSyntax {
  private EditableSyntax $_error;
  public function __construct(
    EditableSyntax $error) {
    parent::__construct('error');
    $this->_error = $error;
  }
  public function error(): EditableSyntax {
    return $this->_error;
  }
  public function with_error(EditableSyntax $error): ErrorSyntax {
    return new ErrorSyntax(
      $error);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $error = $this->error()->rewrite($rewriter, $new_parents);
    if (
      $error === $this->error()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ErrorSyntax(
        $error), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $error = EditableSyntax::from_json(
      $json->error_error, $position, $source);
    $position += $error->width();
    return new ErrorSyntax(
        $error);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_error;
    yield break;
  }
}
final class ListItem extends EditableSyntax {
  private EditableSyntax $_item;
  private EditableSyntax $_separator;
  public function __construct(
    EditableSyntax $item,
    EditableSyntax $separator) {
    parent::__construct('list_item');
    $this->_item = $item;
    $this->_separator = $separator;
  }
  public function item(): EditableSyntax {
    return $this->_item;
  }
  public function separator(): EditableSyntax {
    return $this->_separator;
  }
  public function with_item(EditableSyntax $item): ListItem {
    return new ListItem(
      $item,
      $this->_separator);
  }
  public function with_separator(EditableSyntax $separator): ListItem {
    return new ListItem(
      $this->_item,
      $separator);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $item = $this->item()->rewrite($rewriter, $new_parents);
    $separator = $this->separator()->rewrite($rewriter, $new_parents);
    if (
      $item === $this->item() &&
      $separator === $this->separator()) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new ListItem(
        $item,
        $separator), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    $item = EditableSyntax::from_json(
      $json->list_item, $position, $source);
    $position += $item->width();
    $separator = EditableSyntax::from_json(
      $json->list_separator, $position, $source);
    $position += $separator->width();
    return new ListItem(
        $item,
        $separator);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    yield $this->_item;
    yield $this->_separator;
    yield break;
  }
}


function from_json(mixed $json): EditableSyntax {
  return EditableSyntax::from_json($json->parse_tree, 0, $json->program_text);
}
/* End full_fidelity_editable.php */
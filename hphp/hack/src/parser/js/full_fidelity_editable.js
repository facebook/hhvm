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

"use strict";

let utils = require('./full_fidelity_utils.js');
let array_map_reduce = utils.array_map_reduce;
let array_sum = utils.array_sum;

class EditableSyntax
{
  constructor(syntax_kind, children)
  {
    this._syntax_kind = syntax_kind;
    this._children = children;
    let width = 0;
    for(let child in children)
      width += children[child].width;
    this._children_width = width;
  }
  get syntax_kind() { return this._syntax_kind; }
  get children() { return this._children; }
  get is_token() { return false; }
  get is_trivia() { return false; }
  get is_list() { return false; }
  get is_missing() { return false; }

  get width() { return this._children_width; }

  get full_text()
  {
    let s = '';
    for(let key of this.children_keys)
      s += this.children[key].full_text;
    return s;
  }

  static from_json(json, position, source)
  {
    switch(json.kind)
    {
    case 'token':
      return EditableToken.from_json(json.token, position, source);
    case 'list':
      return EditableList.from_json(json, position, source);
    case 'whitespace':
      return WhiteSpace.from_json(json, position, source);
    case 'end_of_line':
      return EndOfLine.from_json(json, position, source);
    case 'delimited_comment':
      return DelimitedComment.from_json(json, position, source);
    case 'single_line_comment':
      return SingleLineComment.from_json(json, position, source);
    case 'unsafe':
      return Unsafe.from_json(json, position, source);
    case 'unsafe_expression':
      return UnsafeExpression.from_json(json, position, source);
    case 'fix_me':
      return FixMe.from_json(json, position, source);
    case 'ignore_error':
      return IgnoreError.from_json(json, position, source);
    case 'fall_through':
      return FallThrough.from_json(json, position, source);
    case 'extra_token_error':
      return ExtraTokenError.from_json(json, position, source);

    case 'missing':
      return Missing.missing;
    case 'end_of_file':
      return EndOfFile.from_json(json, position, source);
    case 'script':
      return Script.from_json(json, position, source);
    case 'simple_type_specifier':
      return SimpleTypeSpecifier.from_json(json, position, source);
    case 'literal':
      return LiteralExpression.from_json(json, position, source);
    case 'variable':
      return VariableExpression.from_json(json, position, source);
    case 'qualified_name':
      return QualifiedNameExpression.from_json(json, position, source);
    case 'pipe_variable':
      return PipeVariableExpression.from_json(json, position, source);
    case 'enum_declaration':
      return EnumDeclaration.from_json(json, position, source);
    case 'enumerator':
      return Enumerator.from_json(json, position, source);
    case 'alias_declaration':
      return AliasDeclaration.from_json(json, position, source);
    case 'property_declaration':
      return PropertyDeclaration.from_json(json, position, source);
    case 'property_declarator':
      return PropertyDeclarator.from_json(json, position, source);
    case 'namespace_declaration':
      return NamespaceDeclaration.from_json(json, position, source);
    case 'namespace_body':
      return NamespaceBody.from_json(json, position, source);
    case 'namespace_empty_body':
      return NamespaceEmptyBody.from_json(json, position, source);
    case 'namespace_use_declaration':
      return NamespaceUseDeclaration.from_json(json, position, source);
    case 'namespace_group_use_declaration':
      return NamespaceGroupUseDeclaration.from_json(json, position, source);
    case 'namespace_use_clause':
      return NamespaceUseClause.from_json(json, position, source);
    case 'function_declaration':
      return FunctionDeclaration.from_json(json, position, source);
    case 'function_declaration_header':
      return FunctionDeclarationHeader.from_json(json, position, source);
    case 'where_clause':
      return WhereClause.from_json(json, position, source);
    case 'where_constraint':
      return WhereConstraint.from_json(json, position, source);
    case 'methodish_declaration':
      return MethodishDeclaration.from_json(json, position, source);
    case 'classish_declaration':
      return ClassishDeclaration.from_json(json, position, source);
    case 'classish_body':
      return ClassishBody.from_json(json, position, source);
    case 'trait_use_precedence_item':
      return TraitUsePrecedenceItem.from_json(json, position, source);
    case 'trait_use_alias_item':
      return TraitUseAliasItem.from_json(json, position, source);
    case 'trait_use_conflict_resolution':
      return TraitUseConflictResolution.from_json(json, position, source);
    case 'trait_use':
      return TraitUse.from_json(json, position, source);
    case 'require_clause':
      return RequireClause.from_json(json, position, source);
    case 'const_declaration':
      return ConstDeclaration.from_json(json, position, source);
    case 'constant_declarator':
      return ConstantDeclarator.from_json(json, position, source);
    case 'type_const_declaration':
      return TypeConstDeclaration.from_json(json, position, source);
    case 'decorated_expression':
      return DecoratedExpression.from_json(json, position, source);
    case 'parameter_declaration':
      return ParameterDeclaration.from_json(json, position, source);
    case 'variadic_parameter':
      return VariadicParameter.from_json(json, position, source);
    case 'attribute_specification':
      return AttributeSpecification.from_json(json, position, source);
    case 'attribute':
      return Attribute.from_json(json, position, source);
    case 'inclusion_expression':
      return InclusionExpression.from_json(json, position, source);
    case 'inclusion_directive':
      return InclusionDirective.from_json(json, position, source);
    case 'compound_statement':
      return CompoundStatement.from_json(json, position, source);
    case 'expression_statement':
      return ExpressionStatement.from_json(json, position, source);
    case 'markup_section':
      return MarkupSection.from_json(json, position, source);
    case 'markup_suffix':
      return MarkupSuffix.from_json(json, position, source);
    case 'unset_statement':
      return UnsetStatement.from_json(json, position, source);
    case 'while_statement':
      return WhileStatement.from_json(json, position, source);
    case 'if_statement':
      return IfStatement.from_json(json, position, source);
    case 'elseif_clause':
      return ElseifClause.from_json(json, position, source);
    case 'else_clause':
      return ElseClause.from_json(json, position, source);
    case 'try_statement':
      return TryStatement.from_json(json, position, source);
    case 'catch_clause':
      return CatchClause.from_json(json, position, source);
    case 'finally_clause':
      return FinallyClause.from_json(json, position, source);
    case 'do_statement':
      return DoStatement.from_json(json, position, source);
    case 'for_statement':
      return ForStatement.from_json(json, position, source);
    case 'foreach_statement':
      return ForeachStatement.from_json(json, position, source);
    case 'switch_statement':
      return SwitchStatement.from_json(json, position, source);
    case 'switch_section':
      return SwitchSection.from_json(json, position, source);
    case 'switch_fallthrough':
      return SwitchFallthrough.from_json(json, position, source);
    case 'case_label':
      return CaseLabel.from_json(json, position, source);
    case 'default_label':
      return DefaultLabel.from_json(json, position, source);
    case 'return_statement':
      return ReturnStatement.from_json(json, position, source);
    case 'goto_label':
      return GotoLabel.from_json(json, position, source);
    case 'goto_statement':
      return GotoStatement.from_json(json, position, source);
    case 'throw_statement':
      return ThrowStatement.from_json(json, position, source);
    case 'break_statement':
      return BreakStatement.from_json(json, position, source);
    case 'continue_statement':
      return ContinueStatement.from_json(json, position, source);
    case 'function_static_statement':
      return FunctionStaticStatement.from_json(json, position, source);
    case 'static_declarator':
      return StaticDeclarator.from_json(json, position, source);
    case 'echo_statement':
      return EchoStatement.from_json(json, position, source);
    case 'global_statement':
      return GlobalStatement.from_json(json, position, source);
    case 'simple_initializer':
      return SimpleInitializer.from_json(json, position, source);
    case 'anonymous_function':
      return AnonymousFunction.from_json(json, position, source);
    case 'anonymous_function_use_clause':
      return AnonymousFunctionUseClause.from_json(json, position, source);
    case 'lambda_expression':
      return LambdaExpression.from_json(json, position, source);
    case 'lambda_signature':
      return LambdaSignature.from_json(json, position, source);
    case 'cast_expression':
      return CastExpression.from_json(json, position, source);
    case 'scope_resolution_expression':
      return ScopeResolutionExpression.from_json(json, position, source);
    case 'member_selection_expression':
      return MemberSelectionExpression.from_json(json, position, source);
    case 'safe_member_selection_expression':
      return SafeMemberSelectionExpression.from_json(json, position, source);
    case 'embedded_member_selection_expression':
      return EmbeddedMemberSelectionExpression.from_json(json, position, source);
    case 'yield_expression':
      return YieldExpression.from_json(json, position, source);
    case 'yield_from_expression':
      return YieldFromExpression.from_json(json, position, source);
    case 'prefix_unary_expression':
      return PrefixUnaryExpression.from_json(json, position, source);
    case 'postfix_unary_expression':
      return PostfixUnaryExpression.from_json(json, position, source);
    case 'binary_expression':
      return BinaryExpression.from_json(json, position, source);
    case 'instanceof_expression':
      return InstanceofExpression.from_json(json, position, source);
    case 'conditional_expression':
      return ConditionalExpression.from_json(json, position, source);
    case 'eval_expression':
      return EvalExpression.from_json(json, position, source);
    case 'empty_expression':
      return EmptyExpression.from_json(json, position, source);
    case 'define_expression':
      return DefineExpression.from_json(json, position, source);
    case 'isset_expression':
      return IssetExpression.from_json(json, position, source);
    case 'function_call_expression':
      return FunctionCallExpression.from_json(json, position, source);
    case 'parenthesized_expression':
      return ParenthesizedExpression.from_json(json, position, source);
    case 'braced_expression':
      return BracedExpression.from_json(json, position, source);
    case 'embedded_braced_expression':
      return EmbeddedBracedExpression.from_json(json, position, source);
    case 'list_expression':
      return ListExpression.from_json(json, position, source);
    case 'collection_literal_expression':
      return CollectionLiteralExpression.from_json(json, position, source);
    case 'object_creation_expression':
      return ObjectCreationExpression.from_json(json, position, source);
    case 'array_creation_expression':
      return ArrayCreationExpression.from_json(json, position, source);
    case 'array_intrinsic_expression':
      return ArrayIntrinsicExpression.from_json(json, position, source);
    case 'darray_intrinsic_expression':
      return DarrayIntrinsicExpression.from_json(json, position, source);
    case 'dictionary_intrinsic_expression':
      return DictionaryIntrinsicExpression.from_json(json, position, source);
    case 'keyset_intrinsic_expression':
      return KeysetIntrinsicExpression.from_json(json, position, source);
    case 'varray_intrinsic_expression':
      return VarrayIntrinsicExpression.from_json(json, position, source);
    case 'vector_intrinsic_expression':
      return VectorIntrinsicExpression.from_json(json, position, source);
    case 'element_initializer':
      return ElementInitializer.from_json(json, position, source);
    case 'subscript_expression':
      return SubscriptExpression.from_json(json, position, source);
    case 'embedded_subscript_expression':
      return EmbeddedSubscriptExpression.from_json(json, position, source);
    case 'awaitable_creation_expression':
      return AwaitableCreationExpression.from_json(json, position, source);
    case 'xhp_children_declaration':
      return XHPChildrenDeclaration.from_json(json, position, source);
    case 'xhp_children_parenthesized_list':
      return XHPChildrenParenthesizedList.from_json(json, position, source);
    case 'xhp_category_declaration':
      return XHPCategoryDeclaration.from_json(json, position, source);
    case 'xhp_enum_type':
      return XHPEnumType.from_json(json, position, source);
    case 'xhp_required':
      return XHPRequired.from_json(json, position, source);
    case 'xhp_class_attribute_declaration':
      return XHPClassAttributeDeclaration.from_json(json, position, source);
    case 'xhp_class_attribute':
      return XHPClassAttribute.from_json(json, position, source);
    case 'xhp_simple_class_attribute':
      return XHPSimpleClassAttribute.from_json(json, position, source);
    case 'xhp_attribute':
      return XHPAttribute.from_json(json, position, source);
    case 'xhp_open':
      return XHPOpen.from_json(json, position, source);
    case 'xhp_expression':
      return XHPExpression.from_json(json, position, source);
    case 'xhp_close':
      return XHPClose.from_json(json, position, source);
    case 'type_constant':
      return TypeConstant.from_json(json, position, source);
    case 'vector_type_specifier':
      return VectorTypeSpecifier.from_json(json, position, source);
    case 'keyset_type_specifier':
      return KeysetTypeSpecifier.from_json(json, position, source);
    case 'tuple_type_explicit_specifier':
      return TupleTypeExplicitSpecifier.from_json(json, position, source);
    case 'varray_type_specifier':
      return VarrayTypeSpecifier.from_json(json, position, source);
    case 'vector_array_type_specifier':
      return VectorArrayTypeSpecifier.from_json(json, position, source);
    case 'type_parameter':
      return TypeParameter.from_json(json, position, source);
    case 'type_constraint':
      return TypeConstraint.from_json(json, position, source);
    case 'darray_type_specifier':
      return DarrayTypeSpecifier.from_json(json, position, source);
    case 'map_array_type_specifier':
      return MapArrayTypeSpecifier.from_json(json, position, source);
    case 'dictionary_type_specifier':
      return DictionaryTypeSpecifier.from_json(json, position, source);
    case 'closure_type_specifier':
      return ClosureTypeSpecifier.from_json(json, position, source);
    case 'classname_type_specifier':
      return ClassnameTypeSpecifier.from_json(json, position, source);
    case 'field_specifier':
      return FieldSpecifier.from_json(json, position, source);
    case 'field_initializer':
      return FieldInitializer.from_json(json, position, source);
    case 'shape_type_specifier':
      return ShapeTypeSpecifier.from_json(json, position, source);
    case 'shape_expression':
      return ShapeExpression.from_json(json, position, source);
    case 'tuple_expression':
      return TupleExpression.from_json(json, position, source);
    case 'generic_type_specifier':
      return GenericTypeSpecifier.from_json(json, position, source);
    case 'nullable_type_specifier':
      return NullableTypeSpecifier.from_json(json, position, source);
    case 'soft_type_specifier':
      return SoftTypeSpecifier.from_json(json, position, source);
    case 'type_arguments':
      return TypeArguments.from_json(json, position, source);
    case 'type_parameters':
      return TypeParameters.from_json(json, position, source);
    case 'tuple_type_specifier':
      return TupleTypeSpecifier.from_json(json, position, source);
    case 'error':
      return ErrorSyntax.from_json(json, position, source);
    case 'list_item':
      return ListItem.from_json(json, position, source);

    default:
      throw 'unexpected json kind: ' + json.kind; // TODO: Better exception
    }
  }

  reduce(reducer, accumulator, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
    {
      accumulator = this.children[key].reduce(
        reducer, accumulator, new_parents);
    }
    return reducer(this, accumulator, parents);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  find(predicate, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    if (predicate(this))
      return new_parents;
    for(let key of this.children_keys)
    {
      let result = this.children[key].find(predicate, new_parents);
      if (result.length != 0)
        return result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  filter(predicate)
  {
    let reducer = (node, acc, parents) => {
      if (predicate(node))
        acc.push(node);
      return acc;
    };
    return this.reduce(reducer, []);
  }

  of_syntax_kind(kind)
  {
    return this.filter((node) => node.syntax_kind == kind);
  }

  remove_where(predicate)
  {
    return this.rewrite(
      (node, parents) => predicate(node) ? Missing.missing : node);
  }

  without(target)
  {
    return this.remove_where((node) => node === target);
  }

  replace(new_node, target)
  {
    return this.rewrite((node, parents) => node === target ? new_node : node);
  }

  pre_order(action, parents)
  {
    if (parents == undefined)
      parents = [];
    action(this, parents);
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
      this.children[key].pre_order(action, new_parents);
  }

  get leftmost_token()
  {
    if (this.is_token)
      return this;

    for(let key of this.children_keys)
    {
      if (!this.children[key].is_missing)
        return this.children[key].leftmost_token;
    }
    return null;
  }

  get rightmost_token()
  {
    if (this.is_token)
      return this;

    for (let i = this.children_keys.length - 1; i >= 0; i--)
    {
      if (!this.children[this.children_keys[i]].is_missing)
        return this.children[key].rightmost_token;
    }
    return null;
  }

  insert_before(new_node, target)
  {
    // Inserting before missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_before.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.leftmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      let new_leading = EditableSyntax.concatenate_lists(
        token.leading, new_node);
      let new_token = token.with_leading(new_leading);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(new_node, target), target);
  }

  insert_after(new_node, target)
  {
    // Inserting after missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_after.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.rightmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      let new_trailing = EditableSyntax.concatenate_lists(
        new_node, token.trailing);
      let new_token = token.with_trailing(new_trailing);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(target, new_node), target);
  }

  static to_list(syntax_list)
  {
    if (syntax_list.length == 0)
      return Missing.missing;
    else
      return new EditableList(syntax_list);
  }

  static concatenate_lists(left, right)
  {
    if (left.is_missing)
      return right;
    if (right.is_missing)
      return left;
    if (left.is_list && right.is_list)
      return new EditableList(left.children.concat(right.children));
    if (left.is_list)
      return new EditableList(left.children.splice().push(right));
    if (right.is_list)
      return new EditableList([right].concat(left.children));
    return new EditableList([left, right]);
  }
}

class EditableList extends EditableSyntax
{
  constructor(children)
  {
    super('list', children);
  }
  get is_list() { return true; }

  static from_json(json, position, source)
  {
    let children = [];
    let current_position = position;
    for(let element of json.elements)
    {
      let child = EditableSyntax.from_json(element, current_position, source);
      children.push(child);
      current_position += child.width;
    }
    return new EditableList(children);
  }

  rewrite(rewriter, parents)
  {
    let dirty = false;
    let new_children = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for (let key of this.children_keys)
    {
      let child = this.children[key];
      let new_child = child.rewrite(rewriter, new_parents);
      if (new_child != child)
        dirty = true;
      if (new_child != null)
      {
        if (new_child.is_list)
        {
          for(let n of new_child.children)
            new_children.push(n);
        }
        else
          new_children.push(new_child);
      }
    }
    let result = this;
    if (dirty)
    {
      if (new_children.length === 0)
        result = Missing.missing;
      else if (new_children.length === 1)
        result = new_children[0];
      else
        result = new EditableList(new_children);
    }
    return rewriter(result, parents);
  }
  get children_keys()
  {
    return Object.keys(this.children);
  }
}

class EditableToken extends EditableSyntax
{
  constructor(token_kind, leading, trailing, text)
  {
    super('token', { leading : leading, trailing : trailing });
    this._token_kind = token_kind;
    this._text = text;
  }

  get token_kind() { return this._token_kind; }
  get text() { return this._text; }
  get leading() { return this.children.leading; }
  get trailing() { return this.children.trailing; }
  get width()
  {
    return this.text.length + this.leading.width + this.trailing.width;
  }
  get is_token() { return true; }
  get full_text()
  {
    return this.leading.full_text + this.text + this.trailing.full_text;
  }
  with_leading(leading)
  {
    return EditableToken.factory(
      this.token_kind, leading, this.trailing, this.text);
  }
  with_trailing(trailing)
  {
    return EditableToken.factory(
      this.token_kind, this.leading, trailing, this.text);
  }
  static factory(token_kind, leading, trailing, token_text)
  {
    switch(token_kind)
    {
    case 'end_of_file':
       return new EndOfFileToken(leading, trailing);

    case 'abstract':
       return new AbstractToken(leading, trailing);
    case 'and':
       return new AndToken(leading, trailing);
    case 'array':
       return new ArrayToken(leading, trailing);
    case 'arraykey':
       return new ArraykeyToken(leading, trailing);
    case 'as':
       return new AsToken(leading, trailing);
    case 'async':
       return new AsyncToken(leading, trailing);
    case 'attribute':
       return new AttributeToken(leading, trailing);
    case 'await':
       return new AwaitToken(leading, trailing);
    case 'bool':
       return new BoolToken(leading, trailing);
    case 'break':
       return new BreakToken(leading, trailing);
    case 'case':
       return new CaseToken(leading, trailing);
    case 'catch':
       return new CatchToken(leading, trailing);
    case 'category':
       return new CategoryToken(leading, trailing);
    case 'children':
       return new ChildrenToken(leading, trailing);
    case 'class':
       return new ClassToken(leading, trailing);
    case 'classname':
       return new ClassnameToken(leading, trailing);
    case 'clone':
       return new CloneToken(leading, trailing);
    case 'const':
       return new ConstToken(leading, trailing);
    case '__construct':
       return new ConstructToken(leading, trailing);
    case 'continue':
       return new ContinueToken(leading, trailing);
    case 'coroutine':
       return new CoroutineToken(leading, trailing);
    case 'darray':
       return new DarrayToken(leading, trailing);
    case 'default':
       return new DefaultToken(leading, trailing);
    case 'define':
       return new DefineToken(leading, trailing);
    case '__destruct':
       return new DestructToken(leading, trailing);
    case 'dict':
       return new DictToken(leading, trailing);
    case 'do':
       return new DoToken(leading, trailing);
    case 'double':
       return new DoubleToken(leading, trailing);
    case 'echo':
       return new EchoToken(leading, trailing);
    case 'else':
       return new ElseToken(leading, trailing);
    case 'elseif':
       return new ElseifToken(leading, trailing);
    case 'empty':
       return new EmptyToken(leading, trailing);
    case 'enum':
       return new EnumToken(leading, trailing);
    case 'eval':
       return new EvalToken(leading, trailing);
    case 'extends':
       return new ExtendsToken(leading, trailing);
    case 'fallthrough':
       return new FallthroughToken(leading, trailing);
    case 'float':
       return new FloatToken(leading, trailing);
    case 'final':
       return new FinalToken(leading, trailing);
    case 'finally':
       return new FinallyToken(leading, trailing);
    case 'for':
       return new ForToken(leading, trailing);
    case 'foreach':
       return new ForeachToken(leading, trailing);
    case 'from':
       return new FromToken(leading, trailing);
    case 'function':
       return new FunctionToken(leading, trailing);
    case 'global':
       return new GlobalToken(leading, trailing);
    case 'goto':
       return new GotoToken(leading, trailing);
    case 'if':
       return new IfToken(leading, trailing);
    case 'implements':
       return new ImplementsToken(leading, trailing);
    case 'include':
       return new IncludeToken(leading, trailing);
    case 'include_once':
       return new Include_onceToken(leading, trailing);
    case 'instanceof':
       return new InstanceofToken(leading, trailing);
    case 'insteadof':
       return new InsteadofToken(leading, trailing);
    case 'int':
       return new IntToken(leading, trailing);
    case 'interface':
       return new InterfaceToken(leading, trailing);
    case 'isset':
       return new IssetToken(leading, trailing);
    case 'keyset':
       return new KeysetToken(leading, trailing);
    case 'list':
       return new ListToken(leading, trailing);
    case 'mixed':
       return new MixedToken(leading, trailing);
    case 'namespace':
       return new NamespaceToken(leading, trailing);
    case 'new':
       return new NewToken(leading, trailing);
    case 'newtype':
       return new NewtypeToken(leading, trailing);
    case 'noreturn':
       return new NoreturnToken(leading, trailing);
    case 'num':
       return new NumToken(leading, trailing);
    case 'object':
       return new ObjectToken(leading, trailing);
    case 'or':
       return new OrToken(leading, trailing);
    case 'parent':
       return new ParentToken(leading, trailing);
    case 'print':
       return new PrintToken(leading, trailing);
    case 'private':
       return new PrivateToken(leading, trailing);
    case 'protected':
       return new ProtectedToken(leading, trailing);
    case 'public':
       return new PublicToken(leading, trailing);
    case 'require':
       return new RequireToken(leading, trailing);
    case 'require_once':
       return new Require_onceToken(leading, trailing);
    case 'required':
       return new RequiredToken(leading, trailing);
    case 'resource':
       return new ResourceToken(leading, trailing);
    case 'return':
       return new ReturnToken(leading, trailing);
    case 'self':
       return new SelfToken(leading, trailing);
    case 'shape':
       return new ShapeToken(leading, trailing);
    case 'static':
       return new StaticToken(leading, trailing);
    case 'string':
       return new StringToken(leading, trailing);
    case 'super':
       return new SuperToken(leading, trailing);
    case 'suspend':
       return new SuspendToken(leading, trailing);
    case 'switch':
       return new SwitchToken(leading, trailing);
    case 'this':
       return new ThisToken(leading, trailing);
    case 'throw':
       return new ThrowToken(leading, trailing);
    case 'trait':
       return new TraitToken(leading, trailing);
    case 'try':
       return new TryToken(leading, trailing);
    case 'tuple':
       return new TupleToken(leading, trailing);
    case 'type':
       return new TypeToken(leading, trailing);
    case 'unset':
       return new UnsetToken(leading, trailing);
    case 'use':
       return new UseToken(leading, trailing);
    case 'var':
       return new VarToken(leading, trailing);
    case 'varray':
       return new VarrayToken(leading, trailing);
    case 'vec':
       return new VecToken(leading, trailing);
    case 'void':
       return new VoidToken(leading, trailing);
    case 'where':
       return new WhereToken(leading, trailing);
    case 'while':
       return new WhileToken(leading, trailing);
    case 'xor':
       return new XorToken(leading, trailing);
    case 'yield':
       return new YieldToken(leading, trailing);
    case '[':
       return new LeftBracketToken(leading, trailing);
    case ']':
       return new RightBracketToken(leading, trailing);
    case '(':
       return new LeftParenToken(leading, trailing);
    case ')':
       return new RightParenToken(leading, trailing);
    case '{':
       return new LeftBraceToken(leading, trailing);
    case '}':
       return new RightBraceToken(leading, trailing);
    case '.':
       return new DotToken(leading, trailing);
    case '->':
       return new MinusGreaterThanToken(leading, trailing);
    case '++':
       return new PlusPlusToken(leading, trailing);
    case '--':
       return new MinusMinusToken(leading, trailing);
    case '**':
       return new StarStarToken(leading, trailing);
    case '*':
       return new StarToken(leading, trailing);
    case '+':
       return new PlusToken(leading, trailing);
    case '-':
       return new MinusToken(leading, trailing);
    case '~':
       return new TildeToken(leading, trailing);
    case '!':
       return new ExclamationToken(leading, trailing);
    case '$':
       return new DollarToken(leading, trailing);
    case '/':
       return new SlashToken(leading, trailing);
    case '%':
       return new PercentToken(leading, trailing);
    case '<>':
       return new LessThanGreaterThanToken(leading, trailing);
    case '<=>':
       return new LessThanEqualGreaterThanToken(leading, trailing);
    case '<<':
       return new LessThanLessThanToken(leading, trailing);
    case '>>':
       return new GreaterThanGreaterThanToken(leading, trailing);
    case '<':
       return new LessThanToken(leading, trailing);
    case '>':
       return new GreaterThanToken(leading, trailing);
    case '<=':
       return new LessThanEqualToken(leading, trailing);
    case '>=':
       return new GreaterThanEqualToken(leading, trailing);
    case '==':
       return new EqualEqualToken(leading, trailing);
    case '===':
       return new EqualEqualEqualToken(leading, trailing);
    case '!=':
       return new ExclamationEqualToken(leading, trailing);
    case '!==':
       return new ExclamationEqualEqualToken(leading, trailing);
    case '^':
       return new CaratToken(leading, trailing);
    case '|':
       return new BarToken(leading, trailing);
    case '&':
       return new AmpersandToken(leading, trailing);
    case '&&':
       return new AmpersandAmpersandToken(leading, trailing);
    case '||':
       return new BarBarToken(leading, trailing);
    case '?':
       return new QuestionToken(leading, trailing);
    case '??':
       return new QuestionQuestionToken(leading, trailing);
    case ':':
       return new ColonToken(leading, trailing);
    case ';':
       return new SemicolonToken(leading, trailing);
    case '=':
       return new EqualToken(leading, trailing);
    case '**=':
       return new StarStarEqualToken(leading, trailing);
    case '*=':
       return new StarEqualToken(leading, trailing);
    case '/=':
       return new SlashEqualToken(leading, trailing);
    case '%=':
       return new PercentEqualToken(leading, trailing);
    case '+=':
       return new PlusEqualToken(leading, trailing);
    case '-=':
       return new MinusEqualToken(leading, trailing);
    case '.=':
       return new DotEqualToken(leading, trailing);
    case '<<=':
       return new LessThanLessThanEqualToken(leading, trailing);
    case '>>=':
       return new GreaterThanGreaterThanEqualToken(leading, trailing);
    case '&=':
       return new AmpersandEqualToken(leading, trailing);
    case '^=':
       return new CaratEqualToken(leading, trailing);
    case '|=':
       return new BarEqualToken(leading, trailing);
    case ',':
       return new CommaToken(leading, trailing);
    case '@':
       return new AtToken(leading, trailing);
    case '::':
       return new ColonColonToken(leading, trailing);
    case '=>':
       return new EqualGreaterThanToken(leading, trailing);
    case '==>':
       return new EqualEqualGreaterThanToken(leading, trailing);
    case '?->':
       return new QuestionMinusGreaterThanToken(leading, trailing);
    case '...':
       return new DotDotDotToken(leading, trailing);
    case '$$':
       return new DollarDollarToken(leading, trailing);
    case '|>':
       return new BarGreaterThanToken(leading, trailing);
    case 'null':
       return new NullLiteralToken(leading, trailing);
    case '/>':
       return new SlashGreaterThanToken(leading, trailing);
    case '</':
       return new LessThanSlashToken(leading, trailing);
    case '<?':
       return new LessThanQuestionToken(leading, trailing);
    case '?>':
       return new QuestionGreaterThanToken(leading, trailing);

    case 'error_token':
       return new ErrorTokenToken(leading, trailing, token_text);
    case 'name':
       return new NameToken(leading, trailing, token_text);
    case 'qualified_name':
       return new QualifiedNameToken(leading, trailing, token_text);
    case 'variable':
       return new VariableToken(leading, trailing, token_text);
    case 'namespace_prefix':
       return new NamespacePrefixToken(leading, trailing, token_text);
    case 'decimal_literal':
       return new DecimalLiteralToken(leading, trailing, token_text);
    case 'octal_literal':
       return new OctalLiteralToken(leading, trailing, token_text);
    case 'hexadecimal_literal':
       return new HexadecimalLiteralToken(leading, trailing, token_text);
    case 'binary_literal':
       return new BinaryLiteralToken(leading, trailing, token_text);
    case 'floating_literal':
       return new FloatingLiteralToken(leading, trailing, token_text);
    case 'execution_string':
       return new ExecutionStringToken(leading, trailing, token_text);
    case 'single_quoted_string_literal':
       return new SingleQuotedStringLiteralToken(leading, trailing, token_text);
    case 'double_quoted_string_literal':
       return new DoubleQuotedStringLiteralToken(leading, trailing, token_text);
    case 'double_quoted_string_literal_head':
       return new DoubleQuotedStringLiteralHeadToken(leading, trailing, token_text);
    case 'string_literal_body':
       return new StringLiteralBodyToken(leading, trailing, token_text);
    case 'double_quoted_string_literal_tail':
       return new DoubleQuotedStringLiteralTailToken(leading, trailing, token_text);
    case 'heredoc_string_literal':
       return new HeredocStringLiteralToken(leading, trailing, token_text);
    case 'heredoc_string_literal_head':
       return new HeredocStringLiteralHeadToken(leading, trailing, token_text);
    case 'heredoc_string_literal_tail':
       return new HeredocStringLiteralTailToken(leading, trailing, token_text);
    case 'nowdoc_string_literal':
       return new NowdocStringLiteralToken(leading, trailing, token_text);
    case 'boolean_literal':
       return new BooleanLiteralToken(leading, trailing, token_text);
    case 'XHP_category_name':
       return new XHPCategoryNameToken(leading, trailing, token_text);
    case 'XHP_element_name':
       return new XHPElementNameToken(leading, trailing, token_text);
    case 'XHP_class_name':
       return new XHPClassNameToken(leading, trailing, token_text);
    case 'XHP_string_literal':
       return new XHPStringLiteralToken(leading, trailing, token_text);
    case 'XHP_body':
       return new XHPBodyToken(leading, trailing, token_text);
    case 'XHP_comment':
       return new XHPCommentToken(leading, trailing, token_text);
    case 'markup':
       return new MarkupToken(leading, trailing, token_text);

      default: throw 'unexpected token kind; ' + token_kind;
      // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    let new_parents = parents.slice();
    new_parents.push(this);
    let leading = this.leading.rewrite(rewriter, new_parents);
    let trailing = this.trailing.rewrite(rewriter, new_parents);
    if (leading === this.leading && trailing === this.trailing)
      return rewriter(this, parents);
    else
      return rewriter(EditableToken.factory(
        this.token_kind, leading, trailing, this.text), parents);
  }

  reduce(reducer, accumulator)
  {
    accumulator = this.leading.reduce(reducer, accumulator);
    accumulator = reducer(this, accumulator);
    accumulator = this.trailing.reduce(reducer, accumulator);
    return accumulator;
  }

  static from_json(json, position, source)
  {
    let leading_list = array_map_reduce(
      json.leading,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      position);
    let leading = EditableSyntax.to_list(leading_list);
    let token_position = position + leading.width;
    let token_text = source.substring(
      token_position, token_position + json.width);
    let trailing_position = token_position + json.width;
    let trailing_list = array_map_reduce(
      json.trailing,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      trailing_position);
    let trailing = EditableSyntax.to_list(trailing_list);
    return EditableToken.factory(json.kind, leading, trailing, token_text);
  }

  get children_keys()
  {
    if (EditableToken._children_keys == null)
      EditableToken._children_keys = ['leading', 'trailing'];
    return EditableToken._children_keys;
  }
}

class EndOfFileToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('end_of_file', leading, trailing, '');
  }
}

class AbstractToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('abstract', leading, trailing, 'abstract');
  }
}
class AndToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('and', leading, trailing, 'and');
  }
}
class ArrayToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('array', leading, trailing, 'array');
  }
}
class ArraykeyToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('arraykey', leading, trailing, 'arraykey');
  }
}
class AsToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('as', leading, trailing, 'as');
  }
}
class AsyncToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('async', leading, trailing, 'async');
  }
}
class AttributeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('attribute', leading, trailing, 'attribute');
  }
}
class AwaitToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('await', leading, trailing, 'await');
  }
}
class BoolToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('bool', leading, trailing, 'bool');
  }
}
class BreakToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('break', leading, trailing, 'break');
  }
}
class CaseToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('case', leading, trailing, 'case');
  }
}
class CatchToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('catch', leading, trailing, 'catch');
  }
}
class CategoryToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('category', leading, trailing, 'category');
  }
}
class ChildrenToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('children', leading, trailing, 'children');
  }
}
class ClassToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('class', leading, trailing, 'class');
  }
}
class ClassnameToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('classname', leading, trailing, 'classname');
  }
}
class CloneToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('clone', leading, trailing, 'clone');
  }
}
class ConstToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('const', leading, trailing, 'const');
  }
}
class ConstructToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('__construct', leading, trailing, '__construct');
  }
}
class ContinueToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('continue', leading, trailing, 'continue');
  }
}
class CoroutineToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('coroutine', leading, trailing, 'coroutine');
  }
}
class DarrayToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('darray', leading, trailing, 'darray');
  }
}
class DefaultToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('default', leading, trailing, 'default');
  }
}
class DefineToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('define', leading, trailing, 'define');
  }
}
class DestructToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('__destruct', leading, trailing, '__destruct');
  }
}
class DictToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('dict', leading, trailing, 'dict');
  }
}
class DoToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('do', leading, trailing, 'do');
  }
}
class DoubleToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('double', leading, trailing, 'double');
  }
}
class EchoToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('echo', leading, trailing, 'echo');
  }
}
class ElseToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('else', leading, trailing, 'else');
  }
}
class ElseifToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('elseif', leading, trailing, 'elseif');
  }
}
class EmptyToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('empty', leading, trailing, 'empty');
  }
}
class EnumToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('enum', leading, trailing, 'enum');
  }
}
class EvalToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('eval', leading, trailing, 'eval');
  }
}
class ExtendsToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('extends', leading, trailing, 'extends');
  }
}
class FallthroughToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('fallthrough', leading, trailing, 'fallthrough');
  }
}
class FloatToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('float', leading, trailing, 'float');
  }
}
class FinalToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('final', leading, trailing, 'final');
  }
}
class FinallyToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('finally', leading, trailing, 'finally');
  }
}
class ForToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('for', leading, trailing, 'for');
  }
}
class ForeachToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('foreach', leading, trailing, 'foreach');
  }
}
class FromToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('from', leading, trailing, 'from');
  }
}
class FunctionToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('function', leading, trailing, 'function');
  }
}
class GlobalToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('global', leading, trailing, 'global');
  }
}
class GotoToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('goto', leading, trailing, 'goto');
  }
}
class IfToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('if', leading, trailing, 'if');
  }
}
class ImplementsToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('implements', leading, trailing, 'implements');
  }
}
class IncludeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('include', leading, trailing, 'include');
  }
}
class Include_onceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('include_once', leading, trailing, 'include_once');
  }
}
class InstanceofToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('instanceof', leading, trailing, 'instanceof');
  }
}
class InsteadofToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('insteadof', leading, trailing, 'insteadof');
  }
}
class IntToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('int', leading, trailing, 'int');
  }
}
class InterfaceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('interface', leading, trailing, 'interface');
  }
}
class IssetToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('isset', leading, trailing, 'isset');
  }
}
class KeysetToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('keyset', leading, trailing, 'keyset');
  }
}
class ListToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('list', leading, trailing, 'list');
  }
}
class MixedToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('mixed', leading, trailing, 'mixed');
  }
}
class NamespaceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('namespace', leading, trailing, 'namespace');
  }
}
class NewToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('new', leading, trailing, 'new');
  }
}
class NewtypeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('newtype', leading, trailing, 'newtype');
  }
}
class NoreturnToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('noreturn', leading, trailing, 'noreturn');
  }
}
class NumToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('num', leading, trailing, 'num');
  }
}
class ObjectToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('object', leading, trailing, 'object');
  }
}
class OrToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('or', leading, trailing, 'or');
  }
}
class ParentToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('parent', leading, trailing, 'parent');
  }
}
class PrintToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('print', leading, trailing, 'print');
  }
}
class PrivateToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('private', leading, trailing, 'private');
  }
}
class ProtectedToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('protected', leading, trailing, 'protected');
  }
}
class PublicToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('public', leading, trailing, 'public');
  }
}
class RequireToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('require', leading, trailing, 'require');
  }
}
class Require_onceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('require_once', leading, trailing, 'require_once');
  }
}
class RequiredToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('required', leading, trailing, 'required');
  }
}
class ResourceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('resource', leading, trailing, 'resource');
  }
}
class ReturnToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('return', leading, trailing, 'return');
  }
}
class SelfToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('self', leading, trailing, 'self');
  }
}
class ShapeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('shape', leading, trailing, 'shape');
  }
}
class StaticToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('static', leading, trailing, 'static');
  }
}
class StringToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('string', leading, trailing, 'string');
  }
}
class SuperToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('super', leading, trailing, 'super');
  }
}
class SuspendToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('suspend', leading, trailing, 'suspend');
  }
}
class SwitchToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('switch', leading, trailing, 'switch');
  }
}
class ThisToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('this', leading, trailing, 'this');
  }
}
class ThrowToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('throw', leading, trailing, 'throw');
  }
}
class TraitToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('trait', leading, trailing, 'trait');
  }
}
class TryToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('try', leading, trailing, 'try');
  }
}
class TupleToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('tuple', leading, trailing, 'tuple');
  }
}
class TypeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('type', leading, trailing, 'type');
  }
}
class UnsetToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('unset', leading, trailing, 'unset');
  }
}
class UseToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('use', leading, trailing, 'use');
  }
}
class VarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('var', leading, trailing, 'var');
  }
}
class VarrayToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('varray', leading, trailing, 'varray');
  }
}
class VecToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('vec', leading, trailing, 'vec');
  }
}
class VoidToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('void', leading, trailing, 'void');
  }
}
class WhereToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('where', leading, trailing, 'where');
  }
}
class WhileToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('while', leading, trailing, 'while');
  }
}
class XorToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('xor', leading, trailing, 'xor');
  }
}
class YieldToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('yield', leading, trailing, 'yield');
  }
}
class LeftBracketToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('[', leading, trailing, '[');
  }
}
class RightBracketToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super(']', leading, trailing, ']');
  }
}
class LeftParenToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('(', leading, trailing, '(');
  }
}
class RightParenToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super(')', leading, trailing, ')');
  }
}
class LeftBraceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('{', leading, trailing, '{');
  }
}
class RightBraceToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('}', leading, trailing, '}');
  }
}
class DotToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('.', leading, trailing, '.');
  }
}
class MinusGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('->', leading, trailing, '->');
  }
}
class PlusPlusToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('++', leading, trailing, '++');
  }
}
class MinusMinusToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('--', leading, trailing, '--');
  }
}
class StarStarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('**', leading, trailing, '**');
  }
}
class StarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('*', leading, trailing, '*');
  }
}
class PlusToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('+', leading, trailing, '+');
  }
}
class MinusToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('-', leading, trailing, '-');
  }
}
class TildeToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('~', leading, trailing, '~');
  }
}
class ExclamationToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('!', leading, trailing, '!');
  }
}
class DollarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('$', leading, trailing, '$');
  }
}
class SlashToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('/', leading, trailing, '/');
  }
}
class PercentToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%', leading, trailing, '%');
  }
}
class LessThanGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<>', leading, trailing, '<>');
  }
}
class LessThanEqualGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<=>', leading, trailing, '<=>');
  }
}
class LessThanLessThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<<', leading, trailing, '<<');
  }
}
class GreaterThanGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('>>', leading, trailing, '>>');
  }
}
class LessThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<', leading, trailing, '<');
  }
}
class GreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('>', leading, trailing, '>');
  }
}
class LessThanEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<=', leading, trailing, '<=');
  }
}
class GreaterThanEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('>=', leading, trailing, '>=');
  }
}
class EqualEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('==', leading, trailing, '==');
  }
}
class EqualEqualEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('===', leading, trailing, '===');
  }
}
class ExclamationEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('!=', leading, trailing, '!=');
  }
}
class ExclamationEqualEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('!==', leading, trailing, '!==');
  }
}
class CaratToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('^', leading, trailing, '^');
  }
}
class BarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('|', leading, trailing, '|');
  }
}
class AmpersandToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('&', leading, trailing, '&');
  }
}
class AmpersandAmpersandToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('&&', leading, trailing, '&&');
  }
}
class BarBarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('||', leading, trailing, '||');
  }
}
class QuestionToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('?', leading, trailing, '?');
  }
}
class QuestionQuestionToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('??', leading, trailing, '??');
  }
}
class ColonToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super(':', leading, trailing, ':');
  }
}
class SemicolonToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super(';', leading, trailing, ';');
  }
}
class EqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('=', leading, trailing, '=');
  }
}
class StarStarEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('**=', leading, trailing, '**=');
  }
}
class StarEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('*=', leading, trailing, '*=');
  }
}
class SlashEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('/=', leading, trailing, '/=');
  }
}
class PercentEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%=', leading, trailing, '%=');
  }
}
class PlusEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('+=', leading, trailing, '+=');
  }
}
class MinusEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('-=', leading, trailing, '-=');
  }
}
class DotEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('.=', leading, trailing, '.=');
  }
}
class LessThanLessThanEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<<=', leading, trailing, '<<=');
  }
}
class GreaterThanGreaterThanEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('>>=', leading, trailing, '>>=');
  }
}
class AmpersandEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('&=', leading, trailing, '&=');
  }
}
class CaratEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('^=', leading, trailing, '^=');
  }
}
class BarEqualToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('|=', leading, trailing, '|=');
  }
}
class CommaToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super(',', leading, trailing, ',');
  }
}
class AtToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('@', leading, trailing, '@');
  }
}
class ColonColonToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('::', leading, trailing, '::');
  }
}
class EqualGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('=>', leading, trailing, '=>');
  }
}
class EqualEqualGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('==>', leading, trailing, '==>');
  }
}
class QuestionMinusGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('?->', leading, trailing, '?->');
  }
}
class DotDotDotToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('...', leading, trailing, '...');
  }
}
class DollarDollarToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('$$', leading, trailing, '$$');
  }
}
class BarGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('|>', leading, trailing, '|>');
  }
}
class NullLiteralToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('null', leading, trailing, 'null');
  }
}
class SlashGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('/>', leading, trailing, '/>');
  }
}
class LessThanSlashToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('</', leading, trailing, '</');
  }
}
class LessThanQuestionToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('<?', leading, trailing, '<?');
  }
}
class QuestionGreaterThanToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('?>', leading, trailing, '?>');
  }
}

class ErrorTokenToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('error_token', leading, trailing, text);
  }
  with_text(text)
  {
    return new ErrorTokenToken(this.leading, this.trailing, text);
  }

}
class NameToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('name', leading, trailing, text);
  }
  with_text(text)
  {
    return new NameToken(this.leading, this.trailing, text);
  }

}
class QualifiedNameToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('qualified_name', leading, trailing, text);
  }
  with_text(text)
  {
    return new QualifiedNameToken(this.leading, this.trailing, text);
  }

}
class VariableToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('variable', leading, trailing, text);
  }
  with_text(text)
  {
    return new VariableToken(this.leading, this.trailing, text);
  }

}
class NamespacePrefixToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('namespace_prefix', leading, trailing, text);
  }
  with_text(text)
  {
    return new NamespacePrefixToken(this.leading, this.trailing, text);
  }

}
class DecimalLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('decimal_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new DecimalLiteralToken(this.leading, this.trailing, text);
  }

}
class OctalLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('octal_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new OctalLiteralToken(this.leading, this.trailing, text);
  }

}
class HexadecimalLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('hexadecimal_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new HexadecimalLiteralToken(this.leading, this.trailing, text);
  }

}
class BinaryLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('binary_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new BinaryLiteralToken(this.leading, this.trailing, text);
  }

}
class FloatingLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('floating_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new FloatingLiteralToken(this.leading, this.trailing, text);
  }

}
class ExecutionStringToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('execution_string', leading, trailing, text);
  }
  with_text(text)
  {
    return new ExecutionStringToken(this.leading, this.trailing, text);
  }

}
class SingleQuotedStringLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('single_quoted_string_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new SingleQuotedStringLiteralToken(this.leading, this.trailing, text);
  }

}
class DoubleQuotedStringLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('double_quoted_string_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new DoubleQuotedStringLiteralToken(this.leading, this.trailing, text);
  }

}
class DoubleQuotedStringLiteralHeadToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('double_quoted_string_literal_head', leading, trailing, text);
  }
  with_text(text)
  {
    return new DoubleQuotedStringLiteralHeadToken(this.leading, this.trailing, text);
  }

}
class StringLiteralBodyToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('string_literal_body', leading, trailing, text);
  }
  with_text(text)
  {
    return new StringLiteralBodyToken(this.leading, this.trailing, text);
  }

}
class DoubleQuotedStringLiteralTailToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('double_quoted_string_literal_tail', leading, trailing, text);
  }
  with_text(text)
  {
    return new DoubleQuotedStringLiteralTailToken(this.leading, this.trailing, text);
  }

}
class HeredocStringLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('heredoc_string_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new HeredocStringLiteralToken(this.leading, this.trailing, text);
  }

}
class HeredocStringLiteralHeadToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('heredoc_string_literal_head', leading, trailing, text);
  }
  with_text(text)
  {
    return new HeredocStringLiteralHeadToken(this.leading, this.trailing, text);
  }

}
class HeredocStringLiteralTailToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('heredoc_string_literal_tail', leading, trailing, text);
  }
  with_text(text)
  {
    return new HeredocStringLiteralTailToken(this.leading, this.trailing, text);
  }

}
class NowdocStringLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('nowdoc_string_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new NowdocStringLiteralToken(this.leading, this.trailing, text);
  }

}
class BooleanLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('boolean_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new BooleanLiteralToken(this.leading, this.trailing, text);
  }

}
class XHPCategoryNameToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_category_name', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPCategoryNameToken(this.leading, this.trailing, text);
  }

}
class XHPElementNameToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_element_name', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPElementNameToken(this.leading, this.trailing, text);
  }

}
class XHPClassNameToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_class_name', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPClassNameToken(this.leading, this.trailing, text);
  }

}
class XHPStringLiteralToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_string_literal', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPStringLiteralToken(this.leading, this.trailing, text);
  }

}
class XHPBodyToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_body', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPBodyToken(this.leading, this.trailing, text);
  }

}
class XHPCommentToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('XHP_comment', leading, trailing, text);
  }
  with_text(text)
  {
    return new XHPCommentToken(this.leading, this.trailing, text);
  }

}
class MarkupToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('markup', leading, trailing, text);
  }
  with_text(text)
  {
    return new MarkupToken(this.leading, this.trailing, text);
  }

}


class EditableTrivia extends EditableSyntax
{
  constructor(trivia_kind, text)
  {
    super(trivia_kind, {});
    this._text = text;
  }
  get text() { return this._text; }
  get full_text() { return this.text; }
  get width() { return this.text.length; }
  get is_trivia() { return true; }

  static from_json(json, position, source)
  {
    let trivia_text = source.substring(position, position + json.width);
    switch(json.kind)
    {
      case 'whitespace':
        return new WhiteSpace(trivia_text);
      case 'end_of_line':
        return new EndOfLine(trivia_text);
      case 'delimited_comment':
        return new DelimitedComment(trivia_text);
      case 'single_line_comment':
        return new SingleLineComment(trivia_text);
      case 'unsafe':
        return new Unsafe(trivia_text);
      case 'unsafe_expression':
        return new UnsafeExpression(trivia_text);
      case 'fix_me':
        return new FixMe(trivia_text);
      case 'ignore_error':
        return new IgnoreError(trivia_text);
      case 'fall_through':
        return new FallThrough(trivia_text);
      case 'extra_token_error':
        return new ExtraTokenError(trivia_text);

      default: throw 'unexpected json kind: ' + json.kind; // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}

class WhiteSpace extends EditableTrivia
{
  constructor(text) { super('whitespace', text); }
  with_text(text)
  {
    return new WhiteSpace(text);
  }
}

class EndOfLine extends EditableTrivia
{
  constructor(text) { super('end_of_line', text); }
  with_text(text)
  {
    return new EndOfLine(text);
  }
}

class DelimitedComment extends EditableTrivia
{
  constructor(text) { super('delimited_comment', text); }
  with_text(text)
  {
    return new DelimitedComment(text);
  }
}

class SingleLineComment extends EditableTrivia
{
  constructor(text) { super('single_line_comment', text); }
  with_text(text)
  {
    return new SingleLineComment(text);
  }
}

class Unsafe extends EditableTrivia
{
  constructor(text) { super('unsafe', text); }
  with_text(text)
  {
    return new Unsafe(text);
  }
}

class UnsafeExpression extends EditableTrivia
{
  constructor(text) { super('unsafe_expression', text); }
  with_text(text)
  {
    return new UnsafeExpression(text);
  }
}

class FixMe extends EditableTrivia
{
  constructor(text) { super('fix_me', text); }
  with_text(text)
  {
    return new FixMe(text);
  }
}

class IgnoreError extends EditableTrivia
{
  constructor(text) { super('ignore_error', text); }
  with_text(text)
  {
    return new IgnoreError(text);
  }
}

class FallThrough extends EditableTrivia
{
  constructor(text) { super('fall_through', text); }
  with_text(text)
  {
    return new FallThrough(text);
  }
}

class ExtraTokenError extends EditableTrivia
{
  constructor(text) { super('extra_token_error', text); }
  with_text(text)
  {
    return new ExtraTokenError(text);
  }
}



class Missing extends EditableSyntax
{
  constructor()
  {
    super('missing', {});
  }
  get is_missing() { return true; }
  static get missing() { return Missing._missing; }
  static from_json(json, position, source)
  {
    return Missing._missing;
  }
  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}
Missing._missing = new Missing();

class EndOfFile extends EditableSyntax
{
  constructor(
    token)
  {
    super('end_of_file', {
      token: token });
  }
  get token() { return this.children.token; }
  with_token(token){
    return new EndOfFile(
      token);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var token = this.token.rewrite(rewriter, new_parents);
    if (
      token === this.token)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EndOfFile(
        token), parents);
    }
  }
  static from_json(json, position, source)
  {
    let token = EditableSyntax.from_json(
      json.end_of_file_token, position, source);
    position += token.width;
    return new EndOfFile(
        token);
  }
  get children_keys()
  {
    if (EndOfFile._children_keys == null)
      EndOfFile._children_keys = [
        'token'];
    return EndOfFile._children_keys;
  }
}
class Script extends EditableSyntax
{
  constructor(
    declarations)
  {
    super('script', {
      declarations: declarations });
  }
  get declarations() { return this.children.declarations; }
  with_declarations(declarations){
    return new Script(
      declarations);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var declarations = this.declarations.rewrite(rewriter, new_parents);
    if (
      declarations === this.declarations)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new Script(
        declarations), parents);
    }
  }
  static from_json(json, position, source)
  {
    let declarations = EditableSyntax.from_json(
      json.script_declarations, position, source);
    position += declarations.width;
    return new Script(
        declarations);
  }
  get children_keys()
  {
    if (Script._children_keys == null)
      Script._children_keys = [
        'declarations'];
    return Script._children_keys;
  }
}
class SimpleTypeSpecifier extends EditableSyntax
{
  constructor(
    specifier)
  {
    super('simple_type_specifier', {
      specifier: specifier });
  }
  get specifier() { return this.children.specifier; }
  with_specifier(specifier){
    return new SimpleTypeSpecifier(
      specifier);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var specifier = this.specifier.rewrite(rewriter, new_parents);
    if (
      specifier === this.specifier)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SimpleTypeSpecifier(
        specifier), parents);
    }
  }
  static from_json(json, position, source)
  {
    let specifier = EditableSyntax.from_json(
      json.simple_type_specifier, position, source);
    position += specifier.width;
    return new SimpleTypeSpecifier(
        specifier);
  }
  get children_keys()
  {
    if (SimpleTypeSpecifier._children_keys == null)
      SimpleTypeSpecifier._children_keys = [
        'specifier'];
    return SimpleTypeSpecifier._children_keys;
  }
}
class LiteralExpression extends EditableSyntax
{
  constructor(
    expression)
  {
    super('literal', {
      expression: expression });
  }
  get expression() { return this.children.expression; }
  with_expression(expression){
    return new LiteralExpression(
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new LiteralExpression(
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.literal_expression, position, source);
    position += expression.width;
    return new LiteralExpression(
        expression);
  }
  get children_keys()
  {
    if (LiteralExpression._children_keys == null)
      LiteralExpression._children_keys = [
        'expression'];
    return LiteralExpression._children_keys;
  }
}
class VariableExpression extends EditableSyntax
{
  constructor(
    expression)
  {
    super('variable', {
      expression: expression });
  }
  get expression() { return this.children.expression; }
  with_expression(expression){
    return new VariableExpression(
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VariableExpression(
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.variable_expression, position, source);
    position += expression.width;
    return new VariableExpression(
        expression);
  }
  get children_keys()
  {
    if (VariableExpression._children_keys == null)
      VariableExpression._children_keys = [
        'expression'];
    return VariableExpression._children_keys;
  }
}
class QualifiedNameExpression extends EditableSyntax
{
  constructor(
    expression)
  {
    super('qualified_name', {
      expression: expression });
  }
  get expression() { return this.children.expression; }
  with_expression(expression){
    return new QualifiedNameExpression(
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new QualifiedNameExpression(
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.qualified_name_expression, position, source);
    position += expression.width;
    return new QualifiedNameExpression(
        expression);
  }
  get children_keys()
  {
    if (QualifiedNameExpression._children_keys == null)
      QualifiedNameExpression._children_keys = [
        'expression'];
    return QualifiedNameExpression._children_keys;
  }
}
class PipeVariableExpression extends EditableSyntax
{
  constructor(
    expression)
  {
    super('pipe_variable', {
      expression: expression });
  }
  get expression() { return this.children.expression; }
  with_expression(expression){
    return new PipeVariableExpression(
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new PipeVariableExpression(
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.pipe_variable_expression, position, source);
    position += expression.width;
    return new PipeVariableExpression(
        expression);
  }
  get children_keys()
  {
    if (PipeVariableExpression._children_keys == null)
      PipeVariableExpression._children_keys = [
        'expression'];
    return PipeVariableExpression._children_keys;
  }
}
class EnumDeclaration extends EditableSyntax
{
  constructor(
    attribute_spec,
    keyword,
    name,
    colon,
    base,
    type,
    left_brace,
    enumerators,
    right_brace)
  {
    super('enum_declaration', {
      attribute_spec: attribute_spec,
      keyword: keyword,
      name: name,
      colon: colon,
      base: base,
      type: type,
      left_brace: left_brace,
      enumerators: enumerators,
      right_brace: right_brace });
  }
  get attribute_spec() { return this.children.attribute_spec; }
  get keyword() { return this.children.keyword; }
  get name() { return this.children.name; }
  get colon() { return this.children.colon; }
  get base() { return this.children.base; }
  get type() { return this.children.type; }
  get left_brace() { return this.children.left_brace; }
  get enumerators() { return this.children.enumerators; }
  get right_brace() { return this.children.right_brace; }
  with_attribute_spec(attribute_spec){
    return new EnumDeclaration(
      attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      this.base,
      this.type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_keyword(keyword){
    return new EnumDeclaration(
      this.attribute_spec,
      keyword,
      this.name,
      this.colon,
      this.base,
      this.type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_name(name){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      name,
      this.colon,
      this.base,
      this.type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_colon(colon){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      colon,
      this.base,
      this.type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_base(base){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      base,
      this.type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_type(type){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      this.base,
      type,
      this.left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_left_brace(left_brace){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      this.base,
      this.type,
      left_brace,
      this.enumerators,
      this.right_brace);
  }
  with_enumerators(enumerators){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      this.base,
      this.type,
      this.left_brace,
      enumerators,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new EnumDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.colon,
      this.base,
      this.type,
      this.left_brace,
      this.enumerators,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute_spec = this.attribute_spec.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var base = this.base.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var enumerators = this.enumerators.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      attribute_spec === this.attribute_spec &&
      keyword === this.keyword &&
      name === this.name &&
      colon === this.colon &&
      base === this.base &&
      type === this.type &&
      left_brace === this.left_brace &&
      enumerators === this.enumerators &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EnumDeclaration(
        attribute_spec,
        keyword,
        name,
        colon,
        base,
        type,
        left_brace,
        enumerators,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute_spec = EditableSyntax.from_json(
      json.enum_attribute_spec, position, source);
    position += attribute_spec.width;
    let keyword = EditableSyntax.from_json(
      json.enum_keyword, position, source);
    position += keyword.width;
    let name = EditableSyntax.from_json(
      json.enum_name, position, source);
    position += name.width;
    let colon = EditableSyntax.from_json(
      json.enum_colon, position, source);
    position += colon.width;
    let base = EditableSyntax.from_json(
      json.enum_base, position, source);
    position += base.width;
    let type = EditableSyntax.from_json(
      json.enum_type, position, source);
    position += type.width;
    let left_brace = EditableSyntax.from_json(
      json.enum_left_brace, position, source);
    position += left_brace.width;
    let enumerators = EditableSyntax.from_json(
      json.enum_enumerators, position, source);
    position += enumerators.width;
    let right_brace = EditableSyntax.from_json(
      json.enum_right_brace, position, source);
    position += right_brace.width;
    return new EnumDeclaration(
        attribute_spec,
        keyword,
        name,
        colon,
        base,
        type,
        left_brace,
        enumerators,
        right_brace);
  }
  get children_keys()
  {
    if (EnumDeclaration._children_keys == null)
      EnumDeclaration._children_keys = [
        'attribute_spec',
        'keyword',
        'name',
        'colon',
        'base',
        'type',
        'left_brace',
        'enumerators',
        'right_brace'];
    return EnumDeclaration._children_keys;
  }
}
class Enumerator extends EditableSyntax
{
  constructor(
    name,
    equal,
    value,
    semicolon)
  {
    super('enumerator', {
      name: name,
      equal: equal,
      value: value,
      semicolon: semicolon });
  }
  get name() { return this.children.name; }
  get equal() { return this.children.equal; }
  get value() { return this.children.value; }
  get semicolon() { return this.children.semicolon; }
  with_name(name){
    return new Enumerator(
      name,
      this.equal,
      this.value,
      this.semicolon);
  }
  with_equal(equal){
    return new Enumerator(
      this.name,
      equal,
      this.value,
      this.semicolon);
  }
  with_value(value){
    return new Enumerator(
      this.name,
      this.equal,
      value,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new Enumerator(
      this.name,
      this.equal,
      this.value,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var equal = this.equal.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      equal === this.equal &&
      value === this.value &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new Enumerator(
        name,
        equal,
        value,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.enumerator_name, position, source);
    position += name.width;
    let equal = EditableSyntax.from_json(
      json.enumerator_equal, position, source);
    position += equal.width;
    let value = EditableSyntax.from_json(
      json.enumerator_value, position, source);
    position += value.width;
    let semicolon = EditableSyntax.from_json(
      json.enumerator_semicolon, position, source);
    position += semicolon.width;
    return new Enumerator(
        name,
        equal,
        value,
        semicolon);
  }
  get children_keys()
  {
    if (Enumerator._children_keys == null)
      Enumerator._children_keys = [
        'name',
        'equal',
        'value',
        'semicolon'];
    return Enumerator._children_keys;
  }
}
class AliasDeclaration extends EditableSyntax
{
  constructor(
    attribute_spec,
    keyword,
    name,
    generic_parameter,
    constraint,
    equal,
    type,
    semicolon)
  {
    super('alias_declaration', {
      attribute_spec: attribute_spec,
      keyword: keyword,
      name: name,
      generic_parameter: generic_parameter,
      constraint: constraint,
      equal: equal,
      type: type,
      semicolon: semicolon });
  }
  get attribute_spec() { return this.children.attribute_spec; }
  get keyword() { return this.children.keyword; }
  get name() { return this.children.name; }
  get generic_parameter() { return this.children.generic_parameter; }
  get constraint() { return this.children.constraint; }
  get equal() { return this.children.equal; }
  get type() { return this.children.type; }
  get semicolon() { return this.children.semicolon; }
  with_attribute_spec(attribute_spec){
    return new AliasDeclaration(
      attribute_spec,
      this.keyword,
      this.name,
      this.generic_parameter,
      this.constraint,
      this.equal,
      this.type,
      this.semicolon);
  }
  with_keyword(keyword){
    return new AliasDeclaration(
      this.attribute_spec,
      keyword,
      this.name,
      this.generic_parameter,
      this.constraint,
      this.equal,
      this.type,
      this.semicolon);
  }
  with_name(name){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      name,
      this.generic_parameter,
      this.constraint,
      this.equal,
      this.type,
      this.semicolon);
  }
  with_generic_parameter(generic_parameter){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      generic_parameter,
      this.constraint,
      this.equal,
      this.type,
      this.semicolon);
  }
  with_constraint(constraint){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.generic_parameter,
      constraint,
      this.equal,
      this.type,
      this.semicolon);
  }
  with_equal(equal){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.generic_parameter,
      this.constraint,
      equal,
      this.type,
      this.semicolon);
  }
  with_type(type){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.generic_parameter,
      this.constraint,
      this.equal,
      type,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new AliasDeclaration(
      this.attribute_spec,
      this.keyword,
      this.name,
      this.generic_parameter,
      this.constraint,
      this.equal,
      this.type,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute_spec = this.attribute_spec.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var generic_parameter = this.generic_parameter.rewrite(rewriter, new_parents);
    var constraint = this.constraint.rewrite(rewriter, new_parents);
    var equal = this.equal.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      attribute_spec === this.attribute_spec &&
      keyword === this.keyword &&
      name === this.name &&
      generic_parameter === this.generic_parameter &&
      constraint === this.constraint &&
      equal === this.equal &&
      type === this.type &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new AliasDeclaration(
        attribute_spec,
        keyword,
        name,
        generic_parameter,
        constraint,
        equal,
        type,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute_spec = EditableSyntax.from_json(
      json.alias_attribute_spec, position, source);
    position += attribute_spec.width;
    let keyword = EditableSyntax.from_json(
      json.alias_keyword, position, source);
    position += keyword.width;
    let name = EditableSyntax.from_json(
      json.alias_name, position, source);
    position += name.width;
    let generic_parameter = EditableSyntax.from_json(
      json.alias_generic_parameter, position, source);
    position += generic_parameter.width;
    let constraint = EditableSyntax.from_json(
      json.alias_constraint, position, source);
    position += constraint.width;
    let equal = EditableSyntax.from_json(
      json.alias_equal, position, source);
    position += equal.width;
    let type = EditableSyntax.from_json(
      json.alias_type, position, source);
    position += type.width;
    let semicolon = EditableSyntax.from_json(
      json.alias_semicolon, position, source);
    position += semicolon.width;
    return new AliasDeclaration(
        attribute_spec,
        keyword,
        name,
        generic_parameter,
        constraint,
        equal,
        type,
        semicolon);
  }
  get children_keys()
  {
    if (AliasDeclaration._children_keys == null)
      AliasDeclaration._children_keys = [
        'attribute_spec',
        'keyword',
        'name',
        'generic_parameter',
        'constraint',
        'equal',
        'type',
        'semicolon'];
    return AliasDeclaration._children_keys;
  }
}
class PropertyDeclaration extends EditableSyntax
{
  constructor(
    modifiers,
    type,
    declarators,
    semicolon)
  {
    super('property_declaration', {
      modifiers: modifiers,
      type: type,
      declarators: declarators,
      semicolon: semicolon });
  }
  get modifiers() { return this.children.modifiers; }
  get type() { return this.children.type; }
  get declarators() { return this.children.declarators; }
  get semicolon() { return this.children.semicolon; }
  with_modifiers(modifiers){
    return new PropertyDeclaration(
      modifiers,
      this.type,
      this.declarators,
      this.semicolon);
  }
  with_type(type){
    return new PropertyDeclaration(
      this.modifiers,
      type,
      this.declarators,
      this.semicolon);
  }
  with_declarators(declarators){
    return new PropertyDeclaration(
      this.modifiers,
      this.type,
      declarators,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new PropertyDeclaration(
      this.modifiers,
      this.type,
      this.declarators,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var modifiers = this.modifiers.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var declarators = this.declarators.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      modifiers === this.modifiers &&
      type === this.type &&
      declarators === this.declarators &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new PropertyDeclaration(
        modifiers,
        type,
        declarators,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let modifiers = EditableSyntax.from_json(
      json.property_modifiers, position, source);
    position += modifiers.width;
    let type = EditableSyntax.from_json(
      json.property_type, position, source);
    position += type.width;
    let declarators = EditableSyntax.from_json(
      json.property_declarators, position, source);
    position += declarators.width;
    let semicolon = EditableSyntax.from_json(
      json.property_semicolon, position, source);
    position += semicolon.width;
    return new PropertyDeclaration(
        modifiers,
        type,
        declarators,
        semicolon);
  }
  get children_keys()
  {
    if (PropertyDeclaration._children_keys == null)
      PropertyDeclaration._children_keys = [
        'modifiers',
        'type',
        'declarators',
        'semicolon'];
    return PropertyDeclaration._children_keys;
  }
}
class PropertyDeclarator extends EditableSyntax
{
  constructor(
    name,
    initializer)
  {
    super('property_declarator', {
      name: name,
      initializer: initializer });
  }
  get name() { return this.children.name; }
  get initializer() { return this.children.initializer; }
  with_name(name){
    return new PropertyDeclarator(
      name,
      this.initializer);
  }
  with_initializer(initializer){
    return new PropertyDeclarator(
      this.name,
      initializer);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var initializer = this.initializer.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      initializer === this.initializer)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new PropertyDeclarator(
        name,
        initializer), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.property_name, position, source);
    position += name.width;
    let initializer = EditableSyntax.from_json(
      json.property_initializer, position, source);
    position += initializer.width;
    return new PropertyDeclarator(
        name,
        initializer);
  }
  get children_keys()
  {
    if (PropertyDeclarator._children_keys == null)
      PropertyDeclarator._children_keys = [
        'name',
        'initializer'];
    return PropertyDeclarator._children_keys;
  }
}
class NamespaceDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    name,
    body)
  {
    super('namespace_declaration', {
      keyword: keyword,
      name: name,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get name() { return this.children.name; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new NamespaceDeclaration(
      keyword,
      this.name,
      this.body);
  }
  with_name(name){
    return new NamespaceDeclaration(
      this.keyword,
      name,
      this.body);
  }
  with_body(body){
    return new NamespaceDeclaration(
      this.keyword,
      this.name,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      name === this.name &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceDeclaration(
        keyword,
        name,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.namespace_keyword, position, source);
    position += keyword.width;
    let name = EditableSyntax.from_json(
      json.namespace_name, position, source);
    position += name.width;
    let body = EditableSyntax.from_json(
      json.namespace_body, position, source);
    position += body.width;
    return new NamespaceDeclaration(
        keyword,
        name,
        body);
  }
  get children_keys()
  {
    if (NamespaceDeclaration._children_keys == null)
      NamespaceDeclaration._children_keys = [
        'keyword',
        'name',
        'body'];
    return NamespaceDeclaration._children_keys;
  }
}
class NamespaceBody extends EditableSyntax
{
  constructor(
    left_brace,
    declarations,
    right_brace)
  {
    super('namespace_body', {
      left_brace: left_brace,
      declarations: declarations,
      right_brace: right_brace });
  }
  get left_brace() { return this.children.left_brace; }
  get declarations() { return this.children.declarations; }
  get right_brace() { return this.children.right_brace; }
  with_left_brace(left_brace){
    return new NamespaceBody(
      left_brace,
      this.declarations,
      this.right_brace);
  }
  with_declarations(declarations){
    return new NamespaceBody(
      this.left_brace,
      declarations,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new NamespaceBody(
      this.left_brace,
      this.declarations,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var declarations = this.declarations.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      left_brace === this.left_brace &&
      declarations === this.declarations &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceBody(
        left_brace,
        declarations,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_brace = EditableSyntax.from_json(
      json.namespace_left_brace, position, source);
    position += left_brace.width;
    let declarations = EditableSyntax.from_json(
      json.namespace_declarations, position, source);
    position += declarations.width;
    let right_brace = EditableSyntax.from_json(
      json.namespace_right_brace, position, source);
    position += right_brace.width;
    return new NamespaceBody(
        left_brace,
        declarations,
        right_brace);
  }
  get children_keys()
  {
    if (NamespaceBody._children_keys == null)
      NamespaceBody._children_keys = [
        'left_brace',
        'declarations',
        'right_brace'];
    return NamespaceBody._children_keys;
  }
}
class NamespaceEmptyBody extends EditableSyntax
{
  constructor(
    semicolon)
  {
    super('namespace_empty_body', {
      semicolon: semicolon });
  }
  get semicolon() { return this.children.semicolon; }
  with_semicolon(semicolon){
    return new NamespaceEmptyBody(
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceEmptyBody(
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let semicolon = EditableSyntax.from_json(
      json.namespace_semicolon, position, source);
    position += semicolon.width;
    return new NamespaceEmptyBody(
        semicolon);
  }
  get children_keys()
  {
    if (NamespaceEmptyBody._children_keys == null)
      NamespaceEmptyBody._children_keys = [
        'semicolon'];
    return NamespaceEmptyBody._children_keys;
  }
}
class NamespaceUseDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    kind,
    clauses,
    semicolon)
  {
    super('namespace_use_declaration', {
      keyword: keyword,
      kind: kind,
      clauses: clauses,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get kind() { return this.children.kind; }
  get clauses() { return this.children.clauses; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new NamespaceUseDeclaration(
      keyword,
      this.kind,
      this.clauses,
      this.semicolon);
  }
  with_kind(kind){
    return new NamespaceUseDeclaration(
      this.keyword,
      kind,
      this.clauses,
      this.semicolon);
  }
  with_clauses(clauses){
    return new NamespaceUseDeclaration(
      this.keyword,
      this.kind,
      clauses,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new NamespaceUseDeclaration(
      this.keyword,
      this.kind,
      this.clauses,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var kind = this.kind.rewrite(rewriter, new_parents);
    var clauses = this.clauses.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      kind === this.kind &&
      clauses === this.clauses &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceUseDeclaration(
        keyword,
        kind,
        clauses,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.namespace_use_keyword, position, source);
    position += keyword.width;
    let kind = EditableSyntax.from_json(
      json.namespace_use_kind, position, source);
    position += kind.width;
    let clauses = EditableSyntax.from_json(
      json.namespace_use_clauses, position, source);
    position += clauses.width;
    let semicolon = EditableSyntax.from_json(
      json.namespace_use_semicolon, position, source);
    position += semicolon.width;
    return new NamespaceUseDeclaration(
        keyword,
        kind,
        clauses,
        semicolon);
  }
  get children_keys()
  {
    if (NamespaceUseDeclaration._children_keys == null)
      NamespaceUseDeclaration._children_keys = [
        'keyword',
        'kind',
        'clauses',
        'semicolon'];
    return NamespaceUseDeclaration._children_keys;
  }
}
class NamespaceGroupUseDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    kind,
    prefix,
    left_brace,
    clauses,
    right_brace,
    semicolon)
  {
    super('namespace_group_use_declaration', {
      keyword: keyword,
      kind: kind,
      prefix: prefix,
      left_brace: left_brace,
      clauses: clauses,
      right_brace: right_brace,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get kind() { return this.children.kind; }
  get prefix() { return this.children.prefix; }
  get left_brace() { return this.children.left_brace; }
  get clauses() { return this.children.clauses; }
  get right_brace() { return this.children.right_brace; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new NamespaceGroupUseDeclaration(
      keyword,
      this.kind,
      this.prefix,
      this.left_brace,
      this.clauses,
      this.right_brace,
      this.semicolon);
  }
  with_kind(kind){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      kind,
      this.prefix,
      this.left_brace,
      this.clauses,
      this.right_brace,
      this.semicolon);
  }
  with_prefix(prefix){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      this.kind,
      prefix,
      this.left_brace,
      this.clauses,
      this.right_brace,
      this.semicolon);
  }
  with_left_brace(left_brace){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      this.kind,
      this.prefix,
      left_brace,
      this.clauses,
      this.right_brace,
      this.semicolon);
  }
  with_clauses(clauses){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      this.kind,
      this.prefix,
      this.left_brace,
      clauses,
      this.right_brace,
      this.semicolon);
  }
  with_right_brace(right_brace){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      this.kind,
      this.prefix,
      this.left_brace,
      this.clauses,
      right_brace,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new NamespaceGroupUseDeclaration(
      this.keyword,
      this.kind,
      this.prefix,
      this.left_brace,
      this.clauses,
      this.right_brace,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var kind = this.kind.rewrite(rewriter, new_parents);
    var prefix = this.prefix.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var clauses = this.clauses.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      kind === this.kind &&
      prefix === this.prefix &&
      left_brace === this.left_brace &&
      clauses === this.clauses &&
      right_brace === this.right_brace &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceGroupUseDeclaration(
        keyword,
        kind,
        prefix,
        left_brace,
        clauses,
        right_brace,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.namespace_group_use_keyword, position, source);
    position += keyword.width;
    let kind = EditableSyntax.from_json(
      json.namespace_group_use_kind, position, source);
    position += kind.width;
    let prefix = EditableSyntax.from_json(
      json.namespace_group_use_prefix, position, source);
    position += prefix.width;
    let left_brace = EditableSyntax.from_json(
      json.namespace_group_use_left_brace, position, source);
    position += left_brace.width;
    let clauses = EditableSyntax.from_json(
      json.namespace_group_use_clauses, position, source);
    position += clauses.width;
    let right_brace = EditableSyntax.from_json(
      json.namespace_group_use_right_brace, position, source);
    position += right_brace.width;
    let semicolon = EditableSyntax.from_json(
      json.namespace_group_use_semicolon, position, source);
    position += semicolon.width;
    return new NamespaceGroupUseDeclaration(
        keyword,
        kind,
        prefix,
        left_brace,
        clauses,
        right_brace,
        semicolon);
  }
  get children_keys()
  {
    if (NamespaceGroupUseDeclaration._children_keys == null)
      NamespaceGroupUseDeclaration._children_keys = [
        'keyword',
        'kind',
        'prefix',
        'left_brace',
        'clauses',
        'right_brace',
        'semicolon'];
    return NamespaceGroupUseDeclaration._children_keys;
  }
}
class NamespaceUseClause extends EditableSyntax
{
  constructor(
    clause_kind,
    name,
    as,
    alias)
  {
    super('namespace_use_clause', {
      clause_kind: clause_kind,
      name: name,
      as: as,
      alias: alias });
  }
  get clause_kind() { return this.children.clause_kind; }
  get name() { return this.children.name; }
  get as() { return this.children.as; }
  get alias() { return this.children.alias; }
  with_clause_kind(clause_kind){
    return new NamespaceUseClause(
      clause_kind,
      this.name,
      this.as,
      this.alias);
  }
  with_name(name){
    return new NamespaceUseClause(
      this.clause_kind,
      name,
      this.as,
      this.alias);
  }
  with_as(as){
    return new NamespaceUseClause(
      this.clause_kind,
      this.name,
      as,
      this.alias);
  }
  with_alias(alias){
    return new NamespaceUseClause(
      this.clause_kind,
      this.name,
      this.as,
      alias);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var clause_kind = this.clause_kind.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var as = this.as.rewrite(rewriter, new_parents);
    var alias = this.alias.rewrite(rewriter, new_parents);
    if (
      clause_kind === this.clause_kind &&
      name === this.name &&
      as === this.as &&
      alias === this.alias)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NamespaceUseClause(
        clause_kind,
        name,
        as,
        alias), parents);
    }
  }
  static from_json(json, position, source)
  {
    let clause_kind = EditableSyntax.from_json(
      json.namespace_use_clause_kind, position, source);
    position += clause_kind.width;
    let name = EditableSyntax.from_json(
      json.namespace_use_name, position, source);
    position += name.width;
    let as = EditableSyntax.from_json(
      json.namespace_use_as, position, source);
    position += as.width;
    let alias = EditableSyntax.from_json(
      json.namespace_use_alias, position, source);
    position += alias.width;
    return new NamespaceUseClause(
        clause_kind,
        name,
        as,
        alias);
  }
  get children_keys()
  {
    if (NamespaceUseClause._children_keys == null)
      NamespaceUseClause._children_keys = [
        'clause_kind',
        'name',
        'as',
        'alias'];
    return NamespaceUseClause._children_keys;
  }
}
class FunctionDeclaration extends EditableSyntax
{
  constructor(
    attribute_spec,
    declaration_header,
    body)
  {
    super('function_declaration', {
      attribute_spec: attribute_spec,
      declaration_header: declaration_header,
      body: body });
  }
  get attribute_spec() { return this.children.attribute_spec; }
  get declaration_header() { return this.children.declaration_header; }
  get body() { return this.children.body; }
  with_attribute_spec(attribute_spec){
    return new FunctionDeclaration(
      attribute_spec,
      this.declaration_header,
      this.body);
  }
  with_declaration_header(declaration_header){
    return new FunctionDeclaration(
      this.attribute_spec,
      declaration_header,
      this.body);
  }
  with_body(body){
    return new FunctionDeclaration(
      this.attribute_spec,
      this.declaration_header,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute_spec = this.attribute_spec.rewrite(rewriter, new_parents);
    var declaration_header = this.declaration_header.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      attribute_spec === this.attribute_spec &&
      declaration_header === this.declaration_header &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FunctionDeclaration(
        attribute_spec,
        declaration_header,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute_spec = EditableSyntax.from_json(
      json.function_attribute_spec, position, source);
    position += attribute_spec.width;
    let declaration_header = EditableSyntax.from_json(
      json.function_declaration_header, position, source);
    position += declaration_header.width;
    let body = EditableSyntax.from_json(
      json.function_body, position, source);
    position += body.width;
    return new FunctionDeclaration(
        attribute_spec,
        declaration_header,
        body);
  }
  get children_keys()
  {
    if (FunctionDeclaration._children_keys == null)
      FunctionDeclaration._children_keys = [
        'attribute_spec',
        'declaration_header',
        'body'];
    return FunctionDeclaration._children_keys;
  }
}
class FunctionDeclarationHeader extends EditableSyntax
{
  constructor(
    async,
    coroutine,
    keyword,
    ampersand,
    name,
    type_parameter_list,
    left_paren,
    parameter_list,
    right_paren,
    colon,
    type,
    where_clause)
  {
    super('function_declaration_header', {
      async: async,
      coroutine: coroutine,
      keyword: keyword,
      ampersand: ampersand,
      name: name,
      type_parameter_list: type_parameter_list,
      left_paren: left_paren,
      parameter_list: parameter_list,
      right_paren: right_paren,
      colon: colon,
      type: type,
      where_clause: where_clause });
  }
  get async() { return this.children.async; }
  get coroutine() { return this.children.coroutine; }
  get keyword() { return this.children.keyword; }
  get ampersand() { return this.children.ampersand; }
  get name() { return this.children.name; }
  get type_parameter_list() { return this.children.type_parameter_list; }
  get left_paren() { return this.children.left_paren; }
  get parameter_list() { return this.children.parameter_list; }
  get right_paren() { return this.children.right_paren; }
  get colon() { return this.children.colon; }
  get type() { return this.children.type; }
  get where_clause() { return this.children.where_clause; }
  with_async(async){
    return new FunctionDeclarationHeader(
      async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_coroutine(coroutine){
    return new FunctionDeclarationHeader(
      this.async,
      coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_keyword(keyword){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_ampersand(ampersand){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_name(name){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_type_parameter_list(type_parameter_list){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_left_paren(left_paren){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_parameter_list(parameter_list){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_right_paren(right_paren){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      right_paren,
      this.colon,
      this.type,
      this.where_clause);
  }
  with_colon(colon){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      colon,
      this.type,
      this.where_clause);
  }
  with_type(type){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      type,
      this.where_clause);
  }
  with_where_clause(where_clause){
    return new FunctionDeclarationHeader(
      this.async,
      this.coroutine,
      this.keyword,
      this.ampersand,
      this.name,
      this.type_parameter_list,
      this.left_paren,
      this.parameter_list,
      this.right_paren,
      this.colon,
      this.type,
      where_clause);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var async = this.async.rewrite(rewriter, new_parents);
    var coroutine = this.coroutine.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var ampersand = this.ampersand.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var type_parameter_list = this.type_parameter_list.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var parameter_list = this.parameter_list.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var where_clause = this.where_clause.rewrite(rewriter, new_parents);
    if (
      async === this.async &&
      coroutine === this.coroutine &&
      keyword === this.keyword &&
      ampersand === this.ampersand &&
      name === this.name &&
      type_parameter_list === this.type_parameter_list &&
      left_paren === this.left_paren &&
      parameter_list === this.parameter_list &&
      right_paren === this.right_paren &&
      colon === this.colon &&
      type === this.type &&
      where_clause === this.where_clause)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FunctionDeclarationHeader(
        async,
        coroutine,
        keyword,
        ampersand,
        name,
        type_parameter_list,
        left_paren,
        parameter_list,
        right_paren,
        colon,
        type,
        where_clause), parents);
    }
  }
  static from_json(json, position, source)
  {
    let async = EditableSyntax.from_json(
      json.function_async, position, source);
    position += async.width;
    let coroutine = EditableSyntax.from_json(
      json.function_coroutine, position, source);
    position += coroutine.width;
    let keyword = EditableSyntax.from_json(
      json.function_keyword, position, source);
    position += keyword.width;
    let ampersand = EditableSyntax.from_json(
      json.function_ampersand, position, source);
    position += ampersand.width;
    let name = EditableSyntax.from_json(
      json.function_name, position, source);
    position += name.width;
    let type_parameter_list = EditableSyntax.from_json(
      json.function_type_parameter_list, position, source);
    position += type_parameter_list.width;
    let left_paren = EditableSyntax.from_json(
      json.function_left_paren, position, source);
    position += left_paren.width;
    let parameter_list = EditableSyntax.from_json(
      json.function_parameter_list, position, source);
    position += parameter_list.width;
    let right_paren = EditableSyntax.from_json(
      json.function_right_paren, position, source);
    position += right_paren.width;
    let colon = EditableSyntax.from_json(
      json.function_colon, position, source);
    position += colon.width;
    let type = EditableSyntax.from_json(
      json.function_type, position, source);
    position += type.width;
    let where_clause = EditableSyntax.from_json(
      json.function_where_clause, position, source);
    position += where_clause.width;
    return new FunctionDeclarationHeader(
        async,
        coroutine,
        keyword,
        ampersand,
        name,
        type_parameter_list,
        left_paren,
        parameter_list,
        right_paren,
        colon,
        type,
        where_clause);
  }
  get children_keys()
  {
    if (FunctionDeclarationHeader._children_keys == null)
      FunctionDeclarationHeader._children_keys = [
        'async',
        'coroutine',
        'keyword',
        'ampersand',
        'name',
        'type_parameter_list',
        'left_paren',
        'parameter_list',
        'right_paren',
        'colon',
        'type',
        'where_clause'];
    return FunctionDeclarationHeader._children_keys;
  }
}
class WhereClause extends EditableSyntax
{
  constructor(
    keyword,
    constraints)
  {
    super('where_clause', {
      keyword: keyword,
      constraints: constraints });
  }
  get keyword() { return this.children.keyword; }
  get constraints() { return this.children.constraints; }
  with_keyword(keyword){
    return new WhereClause(
      keyword,
      this.constraints);
  }
  with_constraints(constraints){
    return new WhereClause(
      this.keyword,
      constraints);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var constraints = this.constraints.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      constraints === this.constraints)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new WhereClause(
        keyword,
        constraints), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.where_clause_keyword, position, source);
    position += keyword.width;
    let constraints = EditableSyntax.from_json(
      json.where_clause_constraints, position, source);
    position += constraints.width;
    return new WhereClause(
        keyword,
        constraints);
  }
  get children_keys()
  {
    if (WhereClause._children_keys == null)
      WhereClause._children_keys = [
        'keyword',
        'constraints'];
    return WhereClause._children_keys;
  }
}
class WhereConstraint extends EditableSyntax
{
  constructor(
    left_type,
    operator,
    right_type)
  {
    super('where_constraint', {
      left_type: left_type,
      operator: operator,
      right_type: right_type });
  }
  get left_type() { return this.children.left_type; }
  get operator() { return this.children.operator; }
  get right_type() { return this.children.right_type; }
  with_left_type(left_type){
    return new WhereConstraint(
      left_type,
      this.operator,
      this.right_type);
  }
  with_operator(operator){
    return new WhereConstraint(
      this.left_type,
      operator,
      this.right_type);
  }
  with_right_type(right_type){
    return new WhereConstraint(
      this.left_type,
      this.operator,
      right_type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_type = this.left_type.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var right_type = this.right_type.rewrite(rewriter, new_parents);
    if (
      left_type === this.left_type &&
      operator === this.operator &&
      right_type === this.right_type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new WhereConstraint(
        left_type,
        operator,
        right_type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_type = EditableSyntax.from_json(
      json.where_constraint_left_type, position, source);
    position += left_type.width;
    let operator = EditableSyntax.from_json(
      json.where_constraint_operator, position, source);
    position += operator.width;
    let right_type = EditableSyntax.from_json(
      json.where_constraint_right_type, position, source);
    position += right_type.width;
    return new WhereConstraint(
        left_type,
        operator,
        right_type);
  }
  get children_keys()
  {
    if (WhereConstraint._children_keys == null)
      WhereConstraint._children_keys = [
        'left_type',
        'operator',
        'right_type'];
    return WhereConstraint._children_keys;
  }
}
class MethodishDeclaration extends EditableSyntax
{
  constructor(
    attribute,
    modifiers,
    function_decl_header,
    function_body,
    semicolon)
  {
    super('methodish_declaration', {
      attribute: attribute,
      modifiers: modifiers,
      function_decl_header: function_decl_header,
      function_body: function_body,
      semicolon: semicolon });
  }
  get attribute() { return this.children.attribute; }
  get modifiers() { return this.children.modifiers; }
  get function_decl_header() { return this.children.function_decl_header; }
  get function_body() { return this.children.function_body; }
  get semicolon() { return this.children.semicolon; }
  with_attribute(attribute){
    return new MethodishDeclaration(
      attribute,
      this.modifiers,
      this.function_decl_header,
      this.function_body,
      this.semicolon);
  }
  with_modifiers(modifiers){
    return new MethodishDeclaration(
      this.attribute,
      modifiers,
      this.function_decl_header,
      this.function_body,
      this.semicolon);
  }
  with_function_decl_header(function_decl_header){
    return new MethodishDeclaration(
      this.attribute,
      this.modifiers,
      function_decl_header,
      this.function_body,
      this.semicolon);
  }
  with_function_body(function_body){
    return new MethodishDeclaration(
      this.attribute,
      this.modifiers,
      this.function_decl_header,
      function_body,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new MethodishDeclaration(
      this.attribute,
      this.modifiers,
      this.function_decl_header,
      this.function_body,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute = this.attribute.rewrite(rewriter, new_parents);
    var modifiers = this.modifiers.rewrite(rewriter, new_parents);
    var function_decl_header = this.function_decl_header.rewrite(rewriter, new_parents);
    var function_body = this.function_body.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      attribute === this.attribute &&
      modifiers === this.modifiers &&
      function_decl_header === this.function_decl_header &&
      function_body === this.function_body &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new MethodishDeclaration(
        attribute,
        modifiers,
        function_decl_header,
        function_body,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute = EditableSyntax.from_json(
      json.methodish_attribute, position, source);
    position += attribute.width;
    let modifiers = EditableSyntax.from_json(
      json.methodish_modifiers, position, source);
    position += modifiers.width;
    let function_decl_header = EditableSyntax.from_json(
      json.methodish_function_decl_header, position, source);
    position += function_decl_header.width;
    let function_body = EditableSyntax.from_json(
      json.methodish_function_body, position, source);
    position += function_body.width;
    let semicolon = EditableSyntax.from_json(
      json.methodish_semicolon, position, source);
    position += semicolon.width;
    return new MethodishDeclaration(
        attribute,
        modifiers,
        function_decl_header,
        function_body,
        semicolon);
  }
  get children_keys()
  {
    if (MethodishDeclaration._children_keys == null)
      MethodishDeclaration._children_keys = [
        'attribute',
        'modifiers',
        'function_decl_header',
        'function_body',
        'semicolon'];
    return MethodishDeclaration._children_keys;
  }
}
class ClassishDeclaration extends EditableSyntax
{
  constructor(
    attribute,
    modifiers,
    keyword,
    name,
    type_parameters,
    extends_keyword,
    extends_list,
    implements_keyword,
    implements_list,
    body)
  {
    super('classish_declaration', {
      attribute: attribute,
      modifiers: modifiers,
      keyword: keyword,
      name: name,
      type_parameters: type_parameters,
      extends_keyword: extends_keyword,
      extends_list: extends_list,
      implements_keyword: implements_keyword,
      implements_list: implements_list,
      body: body });
  }
  get attribute() { return this.children.attribute; }
  get modifiers() { return this.children.modifiers; }
  get keyword() { return this.children.keyword; }
  get name() { return this.children.name; }
  get type_parameters() { return this.children.type_parameters; }
  get extends_keyword() { return this.children.extends_keyword; }
  get extends_list() { return this.children.extends_list; }
  get implements_keyword() { return this.children.implements_keyword; }
  get implements_list() { return this.children.implements_list; }
  get body() { return this.children.body; }
  with_attribute(attribute){
    return new ClassishDeclaration(
      attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_modifiers(modifiers){
    return new ClassishDeclaration(
      this.attribute,
      modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_keyword(keyword){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_name(name){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_type_parameters(type_parameters){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_extends_keyword(extends_keyword){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_extends_list(extends_list){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      extends_list,
      this.implements_keyword,
      this.implements_list,
      this.body);
  }
  with_implements_keyword(implements_keyword){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      implements_keyword,
      this.implements_list,
      this.body);
  }
  with_implements_list(implements_list){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      implements_list,
      this.body);
  }
  with_body(body){
    return new ClassishDeclaration(
      this.attribute,
      this.modifiers,
      this.keyword,
      this.name,
      this.type_parameters,
      this.extends_keyword,
      this.extends_list,
      this.implements_keyword,
      this.implements_list,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute = this.attribute.rewrite(rewriter, new_parents);
    var modifiers = this.modifiers.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var type_parameters = this.type_parameters.rewrite(rewriter, new_parents);
    var extends_keyword = this.extends_keyword.rewrite(rewriter, new_parents);
    var extends_list = this.extends_list.rewrite(rewriter, new_parents);
    var implements_keyword = this.implements_keyword.rewrite(rewriter, new_parents);
    var implements_list = this.implements_list.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      attribute === this.attribute &&
      modifiers === this.modifiers &&
      keyword === this.keyword &&
      name === this.name &&
      type_parameters === this.type_parameters &&
      extends_keyword === this.extends_keyword &&
      extends_list === this.extends_list &&
      implements_keyword === this.implements_keyword &&
      implements_list === this.implements_list &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ClassishDeclaration(
        attribute,
        modifiers,
        keyword,
        name,
        type_parameters,
        extends_keyword,
        extends_list,
        implements_keyword,
        implements_list,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute = EditableSyntax.from_json(
      json.classish_attribute, position, source);
    position += attribute.width;
    let modifiers = EditableSyntax.from_json(
      json.classish_modifiers, position, source);
    position += modifiers.width;
    let keyword = EditableSyntax.from_json(
      json.classish_keyword, position, source);
    position += keyword.width;
    let name = EditableSyntax.from_json(
      json.classish_name, position, source);
    position += name.width;
    let type_parameters = EditableSyntax.from_json(
      json.classish_type_parameters, position, source);
    position += type_parameters.width;
    let extends_keyword = EditableSyntax.from_json(
      json.classish_extends_keyword, position, source);
    position += extends_keyword.width;
    let extends_list = EditableSyntax.from_json(
      json.classish_extends_list, position, source);
    position += extends_list.width;
    let implements_keyword = EditableSyntax.from_json(
      json.classish_implements_keyword, position, source);
    position += implements_keyword.width;
    let implements_list = EditableSyntax.from_json(
      json.classish_implements_list, position, source);
    position += implements_list.width;
    let body = EditableSyntax.from_json(
      json.classish_body, position, source);
    position += body.width;
    return new ClassishDeclaration(
        attribute,
        modifiers,
        keyword,
        name,
        type_parameters,
        extends_keyword,
        extends_list,
        implements_keyword,
        implements_list,
        body);
  }
  get children_keys()
  {
    if (ClassishDeclaration._children_keys == null)
      ClassishDeclaration._children_keys = [
        'attribute',
        'modifiers',
        'keyword',
        'name',
        'type_parameters',
        'extends_keyword',
        'extends_list',
        'implements_keyword',
        'implements_list',
        'body'];
    return ClassishDeclaration._children_keys;
  }
}
class ClassishBody extends EditableSyntax
{
  constructor(
    left_brace,
    elements,
    right_brace)
  {
    super('classish_body', {
      left_brace: left_brace,
      elements: elements,
      right_brace: right_brace });
  }
  get left_brace() { return this.children.left_brace; }
  get elements() { return this.children.elements; }
  get right_brace() { return this.children.right_brace; }
  with_left_brace(left_brace){
    return new ClassishBody(
      left_brace,
      this.elements,
      this.right_brace);
  }
  with_elements(elements){
    return new ClassishBody(
      this.left_brace,
      elements,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new ClassishBody(
      this.left_brace,
      this.elements,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var elements = this.elements.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      left_brace === this.left_brace &&
      elements === this.elements &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ClassishBody(
        left_brace,
        elements,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_brace = EditableSyntax.from_json(
      json.classish_body_left_brace, position, source);
    position += left_brace.width;
    let elements = EditableSyntax.from_json(
      json.classish_body_elements, position, source);
    position += elements.width;
    let right_brace = EditableSyntax.from_json(
      json.classish_body_right_brace, position, source);
    position += right_brace.width;
    return new ClassishBody(
        left_brace,
        elements,
        right_brace);
  }
  get children_keys()
  {
    if (ClassishBody._children_keys == null)
      ClassishBody._children_keys = [
        'left_brace',
        'elements',
        'right_brace'];
    return ClassishBody._children_keys;
  }
}
class TraitUsePrecedenceItem extends EditableSyntax
{
  constructor(
    name,
    keyword,
    removed_names)
  {
    super('trait_use_precedence_item', {
      name: name,
      keyword: keyword,
      removed_names: removed_names });
  }
  get name() { return this.children.name; }
  get keyword() { return this.children.keyword; }
  get removed_names() { return this.children.removed_names; }
  with_name(name){
    return new TraitUsePrecedenceItem(
      name,
      this.keyword,
      this.removed_names);
  }
  with_keyword(keyword){
    return new TraitUsePrecedenceItem(
      this.name,
      keyword,
      this.removed_names);
  }
  with_removed_names(removed_names){
    return new TraitUsePrecedenceItem(
      this.name,
      this.keyword,
      removed_names);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var removed_names = this.removed_names.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      keyword === this.keyword &&
      removed_names === this.removed_names)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TraitUsePrecedenceItem(
        name,
        keyword,
        removed_names), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.trait_use_precedence_item_name, position, source);
    position += name.width;
    let keyword = EditableSyntax.from_json(
      json.trait_use_precedence_item_keyword, position, source);
    position += keyword.width;
    let removed_names = EditableSyntax.from_json(
      json.trait_use_precedence_item_removed_names, position, source);
    position += removed_names.width;
    return new TraitUsePrecedenceItem(
        name,
        keyword,
        removed_names);
  }
  get children_keys()
  {
    if (TraitUsePrecedenceItem._children_keys == null)
      TraitUsePrecedenceItem._children_keys = [
        'name',
        'keyword',
        'removed_names'];
    return TraitUsePrecedenceItem._children_keys;
  }
}
class TraitUseAliasItem extends EditableSyntax
{
  constructor(
    aliasing_name,
    keyword,
    visibility,
    aliased_name)
  {
    super('trait_use_alias_item', {
      aliasing_name: aliasing_name,
      keyword: keyword,
      visibility: visibility,
      aliased_name: aliased_name });
  }
  get aliasing_name() { return this.children.aliasing_name; }
  get keyword() { return this.children.keyword; }
  get visibility() { return this.children.visibility; }
  get aliased_name() { return this.children.aliased_name; }
  with_aliasing_name(aliasing_name){
    return new TraitUseAliasItem(
      aliasing_name,
      this.keyword,
      this.visibility,
      this.aliased_name);
  }
  with_keyword(keyword){
    return new TraitUseAliasItem(
      this.aliasing_name,
      keyword,
      this.visibility,
      this.aliased_name);
  }
  with_visibility(visibility){
    return new TraitUseAliasItem(
      this.aliasing_name,
      this.keyword,
      visibility,
      this.aliased_name);
  }
  with_aliased_name(aliased_name){
    return new TraitUseAliasItem(
      this.aliasing_name,
      this.keyword,
      this.visibility,
      aliased_name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var aliasing_name = this.aliasing_name.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var visibility = this.visibility.rewrite(rewriter, new_parents);
    var aliased_name = this.aliased_name.rewrite(rewriter, new_parents);
    if (
      aliasing_name === this.aliasing_name &&
      keyword === this.keyword &&
      visibility === this.visibility &&
      aliased_name === this.aliased_name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TraitUseAliasItem(
        aliasing_name,
        keyword,
        visibility,
        aliased_name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let aliasing_name = EditableSyntax.from_json(
      json.trait_use_alias_item_aliasing_name, position, source);
    position += aliasing_name.width;
    let keyword = EditableSyntax.from_json(
      json.trait_use_alias_item_keyword, position, source);
    position += keyword.width;
    let visibility = EditableSyntax.from_json(
      json.trait_use_alias_item_visibility, position, source);
    position += visibility.width;
    let aliased_name = EditableSyntax.from_json(
      json.trait_use_alias_item_aliased_name, position, source);
    position += aliased_name.width;
    return new TraitUseAliasItem(
        aliasing_name,
        keyword,
        visibility,
        aliased_name);
  }
  get children_keys()
  {
    if (TraitUseAliasItem._children_keys == null)
      TraitUseAliasItem._children_keys = [
        'aliasing_name',
        'keyword',
        'visibility',
        'aliased_name'];
    return TraitUseAliasItem._children_keys;
  }
}
class TraitUseConflictResolution extends EditableSyntax
{
  constructor(
    keyword,
    names,
    left_brace,
    clauses,
    right_brace)
  {
    super('trait_use_conflict_resolution', {
      keyword: keyword,
      names: names,
      left_brace: left_brace,
      clauses: clauses,
      right_brace: right_brace });
  }
  get keyword() { return this.children.keyword; }
  get names() { return this.children.names; }
  get left_brace() { return this.children.left_brace; }
  get clauses() { return this.children.clauses; }
  get right_brace() { return this.children.right_brace; }
  with_keyword(keyword){
    return new TraitUseConflictResolution(
      keyword,
      this.names,
      this.left_brace,
      this.clauses,
      this.right_brace);
  }
  with_names(names){
    return new TraitUseConflictResolution(
      this.keyword,
      names,
      this.left_brace,
      this.clauses,
      this.right_brace);
  }
  with_left_brace(left_brace){
    return new TraitUseConflictResolution(
      this.keyword,
      this.names,
      left_brace,
      this.clauses,
      this.right_brace);
  }
  with_clauses(clauses){
    return new TraitUseConflictResolution(
      this.keyword,
      this.names,
      this.left_brace,
      clauses,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new TraitUseConflictResolution(
      this.keyword,
      this.names,
      this.left_brace,
      this.clauses,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var names = this.names.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var clauses = this.clauses.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      names === this.names &&
      left_brace === this.left_brace &&
      clauses === this.clauses &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TraitUseConflictResolution(
        keyword,
        names,
        left_brace,
        clauses,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.trait_use_conflict_resolution_keyword, position, source);
    position += keyword.width;
    let names = EditableSyntax.from_json(
      json.trait_use_conflict_resolution_names, position, source);
    position += names.width;
    let left_brace = EditableSyntax.from_json(
      json.trait_use_conflict_resolution_left_brace, position, source);
    position += left_brace.width;
    let clauses = EditableSyntax.from_json(
      json.trait_use_conflict_resolution_clauses, position, source);
    position += clauses.width;
    let right_brace = EditableSyntax.from_json(
      json.trait_use_conflict_resolution_right_brace, position, source);
    position += right_brace.width;
    return new TraitUseConflictResolution(
        keyword,
        names,
        left_brace,
        clauses,
        right_brace);
  }
  get children_keys()
  {
    if (TraitUseConflictResolution._children_keys == null)
      TraitUseConflictResolution._children_keys = [
        'keyword',
        'names',
        'left_brace',
        'clauses',
        'right_brace'];
    return TraitUseConflictResolution._children_keys;
  }
}
class TraitUse extends EditableSyntax
{
  constructor(
    keyword,
    names,
    semicolon)
  {
    super('trait_use', {
      keyword: keyword,
      names: names,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get names() { return this.children.names; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new TraitUse(
      keyword,
      this.names,
      this.semicolon);
  }
  with_names(names){
    return new TraitUse(
      this.keyword,
      names,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new TraitUse(
      this.keyword,
      this.names,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var names = this.names.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      names === this.names &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TraitUse(
        keyword,
        names,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.trait_use_keyword, position, source);
    position += keyword.width;
    let names = EditableSyntax.from_json(
      json.trait_use_names, position, source);
    position += names.width;
    let semicolon = EditableSyntax.from_json(
      json.trait_use_semicolon, position, source);
    position += semicolon.width;
    return new TraitUse(
        keyword,
        names,
        semicolon);
  }
  get children_keys()
  {
    if (TraitUse._children_keys == null)
      TraitUse._children_keys = [
        'keyword',
        'names',
        'semicolon'];
    return TraitUse._children_keys;
  }
}
class RequireClause extends EditableSyntax
{
  constructor(
    keyword,
    kind,
    name,
    semicolon)
  {
    super('require_clause', {
      keyword: keyword,
      kind: kind,
      name: name,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get kind() { return this.children.kind; }
  get name() { return this.children.name; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new RequireClause(
      keyword,
      this.kind,
      this.name,
      this.semicolon);
  }
  with_kind(kind){
    return new RequireClause(
      this.keyword,
      kind,
      this.name,
      this.semicolon);
  }
  with_name(name){
    return new RequireClause(
      this.keyword,
      this.kind,
      name,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new RequireClause(
      this.keyword,
      this.kind,
      this.name,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var kind = this.kind.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      kind === this.kind &&
      name === this.name &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new RequireClause(
        keyword,
        kind,
        name,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.require_keyword, position, source);
    position += keyword.width;
    let kind = EditableSyntax.from_json(
      json.require_kind, position, source);
    position += kind.width;
    let name = EditableSyntax.from_json(
      json.require_name, position, source);
    position += name.width;
    let semicolon = EditableSyntax.from_json(
      json.require_semicolon, position, source);
    position += semicolon.width;
    return new RequireClause(
        keyword,
        kind,
        name,
        semicolon);
  }
  get children_keys()
  {
    if (RequireClause._children_keys == null)
      RequireClause._children_keys = [
        'keyword',
        'kind',
        'name',
        'semicolon'];
    return RequireClause._children_keys;
  }
}
class ConstDeclaration extends EditableSyntax
{
  constructor(
    abstract,
    keyword,
    type_specifier,
    declarators,
    semicolon)
  {
    super('const_declaration', {
      abstract: abstract,
      keyword: keyword,
      type_specifier: type_specifier,
      declarators: declarators,
      semicolon: semicolon });
  }
  get abstract() { return this.children.abstract; }
  get keyword() { return this.children.keyword; }
  get type_specifier() { return this.children.type_specifier; }
  get declarators() { return this.children.declarators; }
  get semicolon() { return this.children.semicolon; }
  with_abstract(abstract){
    return new ConstDeclaration(
      abstract,
      this.keyword,
      this.type_specifier,
      this.declarators,
      this.semicolon);
  }
  with_keyword(keyword){
    return new ConstDeclaration(
      this.abstract,
      keyword,
      this.type_specifier,
      this.declarators,
      this.semicolon);
  }
  with_type_specifier(type_specifier){
    return new ConstDeclaration(
      this.abstract,
      this.keyword,
      type_specifier,
      this.declarators,
      this.semicolon);
  }
  with_declarators(declarators){
    return new ConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_specifier,
      declarators,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new ConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_specifier,
      this.declarators,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var abstract = this.abstract.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var type_specifier = this.type_specifier.rewrite(rewriter, new_parents);
    var declarators = this.declarators.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      abstract === this.abstract &&
      keyword === this.keyword &&
      type_specifier === this.type_specifier &&
      declarators === this.declarators &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ConstDeclaration(
        abstract,
        keyword,
        type_specifier,
        declarators,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let abstract = EditableSyntax.from_json(
      json.const_abstract, position, source);
    position += abstract.width;
    let keyword = EditableSyntax.from_json(
      json.const_keyword, position, source);
    position += keyword.width;
    let type_specifier = EditableSyntax.from_json(
      json.const_type_specifier, position, source);
    position += type_specifier.width;
    let declarators = EditableSyntax.from_json(
      json.const_declarators, position, source);
    position += declarators.width;
    let semicolon = EditableSyntax.from_json(
      json.const_semicolon, position, source);
    position += semicolon.width;
    return new ConstDeclaration(
        abstract,
        keyword,
        type_specifier,
        declarators,
        semicolon);
  }
  get children_keys()
  {
    if (ConstDeclaration._children_keys == null)
      ConstDeclaration._children_keys = [
        'abstract',
        'keyword',
        'type_specifier',
        'declarators',
        'semicolon'];
    return ConstDeclaration._children_keys;
  }
}
class ConstantDeclarator extends EditableSyntax
{
  constructor(
    name,
    initializer)
  {
    super('constant_declarator', {
      name: name,
      initializer: initializer });
  }
  get name() { return this.children.name; }
  get initializer() { return this.children.initializer; }
  with_name(name){
    return new ConstantDeclarator(
      name,
      this.initializer);
  }
  with_initializer(initializer){
    return new ConstantDeclarator(
      this.name,
      initializer);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var initializer = this.initializer.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      initializer === this.initializer)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ConstantDeclarator(
        name,
        initializer), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.constant_declarator_name, position, source);
    position += name.width;
    let initializer = EditableSyntax.from_json(
      json.constant_declarator_initializer, position, source);
    position += initializer.width;
    return new ConstantDeclarator(
        name,
        initializer);
  }
  get children_keys()
  {
    if (ConstantDeclarator._children_keys == null)
      ConstantDeclarator._children_keys = [
        'name',
        'initializer'];
    return ConstantDeclarator._children_keys;
  }
}
class TypeConstDeclaration extends EditableSyntax
{
  constructor(
    abstract,
    keyword,
    type_keyword,
    name,
    type_constraint,
    equal,
    type_specifier,
    semicolon)
  {
    super('type_const_declaration', {
      abstract: abstract,
      keyword: keyword,
      type_keyword: type_keyword,
      name: name,
      type_constraint: type_constraint,
      equal: equal,
      type_specifier: type_specifier,
      semicolon: semicolon });
  }
  get abstract() { return this.children.abstract; }
  get keyword() { return this.children.keyword; }
  get type_keyword() { return this.children.type_keyword; }
  get name() { return this.children.name; }
  get type_constraint() { return this.children.type_constraint; }
  get equal() { return this.children.equal; }
  get type_specifier() { return this.children.type_specifier; }
  get semicolon() { return this.children.semicolon; }
  with_abstract(abstract){
    return new TypeConstDeclaration(
      abstract,
      this.keyword,
      this.type_keyword,
      this.name,
      this.type_constraint,
      this.equal,
      this.type_specifier,
      this.semicolon);
  }
  with_keyword(keyword){
    return new TypeConstDeclaration(
      this.abstract,
      keyword,
      this.type_keyword,
      this.name,
      this.type_constraint,
      this.equal,
      this.type_specifier,
      this.semicolon);
  }
  with_type_keyword(type_keyword){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      type_keyword,
      this.name,
      this.type_constraint,
      this.equal,
      this.type_specifier,
      this.semicolon);
  }
  with_name(name){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_keyword,
      name,
      this.type_constraint,
      this.equal,
      this.type_specifier,
      this.semicolon);
  }
  with_type_constraint(type_constraint){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_keyword,
      this.name,
      type_constraint,
      this.equal,
      this.type_specifier,
      this.semicolon);
  }
  with_equal(equal){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_keyword,
      this.name,
      this.type_constraint,
      equal,
      this.type_specifier,
      this.semicolon);
  }
  with_type_specifier(type_specifier){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_keyword,
      this.name,
      this.type_constraint,
      this.equal,
      type_specifier,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new TypeConstDeclaration(
      this.abstract,
      this.keyword,
      this.type_keyword,
      this.name,
      this.type_constraint,
      this.equal,
      this.type_specifier,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var abstract = this.abstract.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var type_keyword = this.type_keyword.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var type_constraint = this.type_constraint.rewrite(rewriter, new_parents);
    var equal = this.equal.rewrite(rewriter, new_parents);
    var type_specifier = this.type_specifier.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      abstract === this.abstract &&
      keyword === this.keyword &&
      type_keyword === this.type_keyword &&
      name === this.name &&
      type_constraint === this.type_constraint &&
      equal === this.equal &&
      type_specifier === this.type_specifier &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeConstDeclaration(
        abstract,
        keyword,
        type_keyword,
        name,
        type_constraint,
        equal,
        type_specifier,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let abstract = EditableSyntax.from_json(
      json.type_const_abstract, position, source);
    position += abstract.width;
    let keyword = EditableSyntax.from_json(
      json.type_const_keyword, position, source);
    position += keyword.width;
    let type_keyword = EditableSyntax.from_json(
      json.type_const_type_keyword, position, source);
    position += type_keyword.width;
    let name = EditableSyntax.from_json(
      json.type_const_name, position, source);
    position += name.width;
    let type_constraint = EditableSyntax.from_json(
      json.type_const_type_constraint, position, source);
    position += type_constraint.width;
    let equal = EditableSyntax.from_json(
      json.type_const_equal, position, source);
    position += equal.width;
    let type_specifier = EditableSyntax.from_json(
      json.type_const_type_specifier, position, source);
    position += type_specifier.width;
    let semicolon = EditableSyntax.from_json(
      json.type_const_semicolon, position, source);
    position += semicolon.width;
    return new TypeConstDeclaration(
        abstract,
        keyword,
        type_keyword,
        name,
        type_constraint,
        equal,
        type_specifier,
        semicolon);
  }
  get children_keys()
  {
    if (TypeConstDeclaration._children_keys == null)
      TypeConstDeclaration._children_keys = [
        'abstract',
        'keyword',
        'type_keyword',
        'name',
        'type_constraint',
        'equal',
        'type_specifier',
        'semicolon'];
    return TypeConstDeclaration._children_keys;
  }
}
class DecoratedExpression extends EditableSyntax
{
  constructor(
    decorator,
    expression)
  {
    super('decorated_expression', {
      decorator: decorator,
      expression: expression });
  }
  get decorator() { return this.children.decorator; }
  get expression() { return this.children.expression; }
  with_decorator(decorator){
    return new DecoratedExpression(
      decorator,
      this.expression);
  }
  with_expression(expression){
    return new DecoratedExpression(
      this.decorator,
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var decorator = this.decorator.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      decorator === this.decorator &&
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DecoratedExpression(
        decorator,
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let decorator = EditableSyntax.from_json(
      json.decorated_expression_decorator, position, source);
    position += decorator.width;
    let expression = EditableSyntax.from_json(
      json.decorated_expression_expression, position, source);
    position += expression.width;
    return new DecoratedExpression(
        decorator,
        expression);
  }
  get children_keys()
  {
    if (DecoratedExpression._children_keys == null)
      DecoratedExpression._children_keys = [
        'decorator',
        'expression'];
    return DecoratedExpression._children_keys;
  }
}
class ParameterDeclaration extends EditableSyntax
{
  constructor(
    attribute,
    visibility,
    type,
    name,
    default_value)
  {
    super('parameter_declaration', {
      attribute: attribute,
      visibility: visibility,
      type: type,
      name: name,
      default_value: default_value });
  }
  get attribute() { return this.children.attribute; }
  get visibility() { return this.children.visibility; }
  get type() { return this.children.type; }
  get name() { return this.children.name; }
  get default_value() { return this.children.default_value; }
  with_attribute(attribute){
    return new ParameterDeclaration(
      attribute,
      this.visibility,
      this.type,
      this.name,
      this.default_value);
  }
  with_visibility(visibility){
    return new ParameterDeclaration(
      this.attribute,
      visibility,
      this.type,
      this.name,
      this.default_value);
  }
  with_type(type){
    return new ParameterDeclaration(
      this.attribute,
      this.visibility,
      type,
      this.name,
      this.default_value);
  }
  with_name(name){
    return new ParameterDeclaration(
      this.attribute,
      this.visibility,
      this.type,
      name,
      this.default_value);
  }
  with_default_value(default_value){
    return new ParameterDeclaration(
      this.attribute,
      this.visibility,
      this.type,
      this.name,
      default_value);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var attribute = this.attribute.rewrite(rewriter, new_parents);
    var visibility = this.visibility.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var default_value = this.default_value.rewrite(rewriter, new_parents);
    if (
      attribute === this.attribute &&
      visibility === this.visibility &&
      type === this.type &&
      name === this.name &&
      default_value === this.default_value)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ParameterDeclaration(
        attribute,
        visibility,
        type,
        name,
        default_value), parents);
    }
  }
  static from_json(json, position, source)
  {
    let attribute = EditableSyntax.from_json(
      json.parameter_attribute, position, source);
    position += attribute.width;
    let visibility = EditableSyntax.from_json(
      json.parameter_visibility, position, source);
    position += visibility.width;
    let type = EditableSyntax.from_json(
      json.parameter_type, position, source);
    position += type.width;
    let name = EditableSyntax.from_json(
      json.parameter_name, position, source);
    position += name.width;
    let default_value = EditableSyntax.from_json(
      json.parameter_default_value, position, source);
    position += default_value.width;
    return new ParameterDeclaration(
        attribute,
        visibility,
        type,
        name,
        default_value);
  }
  get children_keys()
  {
    if (ParameterDeclaration._children_keys == null)
      ParameterDeclaration._children_keys = [
        'attribute',
        'visibility',
        'type',
        'name',
        'default_value'];
    return ParameterDeclaration._children_keys;
  }
}
class VariadicParameter extends EditableSyntax
{
  constructor(
    ellipsis)
  {
    super('variadic_parameter', {
      ellipsis: ellipsis });
  }
  get ellipsis() { return this.children.ellipsis; }
  with_ellipsis(ellipsis){
    return new VariadicParameter(
      ellipsis);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var ellipsis = this.ellipsis.rewrite(rewriter, new_parents);
    if (
      ellipsis === this.ellipsis)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VariadicParameter(
        ellipsis), parents);
    }
  }
  static from_json(json, position, source)
  {
    let ellipsis = EditableSyntax.from_json(
      json.variadic_parameter_ellipsis, position, source);
    position += ellipsis.width;
    return new VariadicParameter(
        ellipsis);
  }
  get children_keys()
  {
    if (VariadicParameter._children_keys == null)
      VariadicParameter._children_keys = [
        'ellipsis'];
    return VariadicParameter._children_keys;
  }
}
class AttributeSpecification extends EditableSyntax
{
  constructor(
    left_double_angle,
    attributes,
    right_double_angle)
  {
    super('attribute_specification', {
      left_double_angle: left_double_angle,
      attributes: attributes,
      right_double_angle: right_double_angle });
  }
  get left_double_angle() { return this.children.left_double_angle; }
  get attributes() { return this.children.attributes; }
  get right_double_angle() { return this.children.right_double_angle; }
  with_left_double_angle(left_double_angle){
    return new AttributeSpecification(
      left_double_angle,
      this.attributes,
      this.right_double_angle);
  }
  with_attributes(attributes){
    return new AttributeSpecification(
      this.left_double_angle,
      attributes,
      this.right_double_angle);
  }
  with_right_double_angle(right_double_angle){
    return new AttributeSpecification(
      this.left_double_angle,
      this.attributes,
      right_double_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_double_angle = this.left_double_angle.rewrite(rewriter, new_parents);
    var attributes = this.attributes.rewrite(rewriter, new_parents);
    var right_double_angle = this.right_double_angle.rewrite(rewriter, new_parents);
    if (
      left_double_angle === this.left_double_angle &&
      attributes === this.attributes &&
      right_double_angle === this.right_double_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new AttributeSpecification(
        left_double_angle,
        attributes,
        right_double_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_double_angle = EditableSyntax.from_json(
      json.attribute_specification_left_double_angle, position, source);
    position += left_double_angle.width;
    let attributes = EditableSyntax.from_json(
      json.attribute_specification_attributes, position, source);
    position += attributes.width;
    let right_double_angle = EditableSyntax.from_json(
      json.attribute_specification_right_double_angle, position, source);
    position += right_double_angle.width;
    return new AttributeSpecification(
        left_double_angle,
        attributes,
        right_double_angle);
  }
  get children_keys()
  {
    if (AttributeSpecification._children_keys == null)
      AttributeSpecification._children_keys = [
        'left_double_angle',
        'attributes',
        'right_double_angle'];
    return AttributeSpecification._children_keys;
  }
}
class Attribute extends EditableSyntax
{
  constructor(
    name,
    left_paren,
    values,
    right_paren)
  {
    super('attribute', {
      name: name,
      left_paren: left_paren,
      values: values,
      right_paren: right_paren });
  }
  get name() { return this.children.name; }
  get left_paren() { return this.children.left_paren; }
  get values() { return this.children.values; }
  get right_paren() { return this.children.right_paren; }
  with_name(name){
    return new Attribute(
      name,
      this.left_paren,
      this.values,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new Attribute(
      this.name,
      left_paren,
      this.values,
      this.right_paren);
  }
  with_values(values){
    return new Attribute(
      this.name,
      this.left_paren,
      values,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new Attribute(
      this.name,
      this.left_paren,
      this.values,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var values = this.values.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      left_paren === this.left_paren &&
      values === this.values &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new Attribute(
        name,
        left_paren,
        values,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.attribute_name, position, source);
    position += name.width;
    let left_paren = EditableSyntax.from_json(
      json.attribute_left_paren, position, source);
    position += left_paren.width;
    let values = EditableSyntax.from_json(
      json.attribute_values, position, source);
    position += values.width;
    let right_paren = EditableSyntax.from_json(
      json.attribute_right_paren, position, source);
    position += right_paren.width;
    return new Attribute(
        name,
        left_paren,
        values,
        right_paren);
  }
  get children_keys()
  {
    if (Attribute._children_keys == null)
      Attribute._children_keys = [
        'name',
        'left_paren',
        'values',
        'right_paren'];
    return Attribute._children_keys;
  }
}
class InclusionExpression extends EditableSyntax
{
  constructor(
    require,
    filename)
  {
    super('inclusion_expression', {
      require: require,
      filename: filename });
  }
  get require() { return this.children.require; }
  get filename() { return this.children.filename; }
  with_require(require){
    return new InclusionExpression(
      require,
      this.filename);
  }
  with_filename(filename){
    return new InclusionExpression(
      this.require,
      filename);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var require = this.require.rewrite(rewriter, new_parents);
    var filename = this.filename.rewrite(rewriter, new_parents);
    if (
      require === this.require &&
      filename === this.filename)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new InclusionExpression(
        require,
        filename), parents);
    }
  }
  static from_json(json, position, source)
  {
    let require = EditableSyntax.from_json(
      json.inclusion_require, position, source);
    position += require.width;
    let filename = EditableSyntax.from_json(
      json.inclusion_filename, position, source);
    position += filename.width;
    return new InclusionExpression(
        require,
        filename);
  }
  get children_keys()
  {
    if (InclusionExpression._children_keys == null)
      InclusionExpression._children_keys = [
        'require',
        'filename'];
    return InclusionExpression._children_keys;
  }
}
class InclusionDirective extends EditableSyntax
{
  constructor(
    expression,
    semicolon)
  {
    super('inclusion_directive', {
      expression: expression,
      semicolon: semicolon });
  }
  get expression() { return this.children.expression; }
  get semicolon() { return this.children.semicolon; }
  with_expression(expression){
    return new InclusionDirective(
      expression,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new InclusionDirective(
      this.expression,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      expression === this.expression &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new InclusionDirective(
        expression,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.inclusion_expression, position, source);
    position += expression.width;
    let semicolon = EditableSyntax.from_json(
      json.inclusion_semicolon, position, source);
    position += semicolon.width;
    return new InclusionDirective(
        expression,
        semicolon);
  }
  get children_keys()
  {
    if (InclusionDirective._children_keys == null)
      InclusionDirective._children_keys = [
        'expression',
        'semicolon'];
    return InclusionDirective._children_keys;
  }
}
class CompoundStatement extends EditableSyntax
{
  constructor(
    left_brace,
    statements,
    right_brace)
  {
    super('compound_statement', {
      left_brace: left_brace,
      statements: statements,
      right_brace: right_brace });
  }
  get left_brace() { return this.children.left_brace; }
  get statements() { return this.children.statements; }
  get right_brace() { return this.children.right_brace; }
  with_left_brace(left_brace){
    return new CompoundStatement(
      left_brace,
      this.statements,
      this.right_brace);
  }
  with_statements(statements){
    return new CompoundStatement(
      this.left_brace,
      statements,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new CompoundStatement(
      this.left_brace,
      this.statements,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var statements = this.statements.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      left_brace === this.left_brace &&
      statements === this.statements &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new CompoundStatement(
        left_brace,
        statements,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_brace = EditableSyntax.from_json(
      json.compound_left_brace, position, source);
    position += left_brace.width;
    let statements = EditableSyntax.from_json(
      json.compound_statements, position, source);
    position += statements.width;
    let right_brace = EditableSyntax.from_json(
      json.compound_right_brace, position, source);
    position += right_brace.width;
    return new CompoundStatement(
        left_brace,
        statements,
        right_brace);
  }
  get children_keys()
  {
    if (CompoundStatement._children_keys == null)
      CompoundStatement._children_keys = [
        'left_brace',
        'statements',
        'right_brace'];
    return CompoundStatement._children_keys;
  }
}
class ExpressionStatement extends EditableSyntax
{
  constructor(
    expression,
    semicolon)
  {
    super('expression_statement', {
      expression: expression,
      semicolon: semicolon });
  }
  get expression() { return this.children.expression; }
  get semicolon() { return this.children.semicolon; }
  with_expression(expression){
    return new ExpressionStatement(
      expression,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new ExpressionStatement(
      this.expression,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      expression === this.expression &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ExpressionStatement(
        expression,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let expression = EditableSyntax.from_json(
      json.expression_statement_expression, position, source);
    position += expression.width;
    let semicolon = EditableSyntax.from_json(
      json.expression_statement_semicolon, position, source);
    position += semicolon.width;
    return new ExpressionStatement(
        expression,
        semicolon);
  }
  get children_keys()
  {
    if (ExpressionStatement._children_keys == null)
      ExpressionStatement._children_keys = [
        'expression',
        'semicolon'];
    return ExpressionStatement._children_keys;
  }
}
class MarkupSection extends EditableSyntax
{
  constructor(
    prefix,
    text,
    suffix,
    expression)
  {
    super('markup_section', {
      prefix: prefix,
      text: text,
      suffix: suffix,
      expression: expression });
  }
  get prefix() { return this.children.prefix; }
  get text() { return this.children.text; }
  get suffix() { return this.children.suffix; }
  get expression() { return this.children.expression; }
  with_prefix(prefix){
    return new MarkupSection(
      prefix,
      this.text,
      this.suffix,
      this.expression);
  }
  with_text(text){
    return new MarkupSection(
      this.prefix,
      text,
      this.suffix,
      this.expression);
  }
  with_suffix(suffix){
    return new MarkupSection(
      this.prefix,
      this.text,
      suffix,
      this.expression);
  }
  with_expression(expression){
    return new MarkupSection(
      this.prefix,
      this.text,
      this.suffix,
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var prefix = this.prefix.rewrite(rewriter, new_parents);
    var text = this.text.rewrite(rewriter, new_parents);
    var suffix = this.suffix.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      prefix === this.prefix &&
      text === this.text &&
      suffix === this.suffix &&
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new MarkupSection(
        prefix,
        text,
        suffix,
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let prefix = EditableSyntax.from_json(
      json.markup_prefix, position, source);
    position += prefix.width;
    let text = EditableSyntax.from_json(
      json.markup_text, position, source);
    position += text.width;
    let suffix = EditableSyntax.from_json(
      json.markup_suffix, position, source);
    position += suffix.width;
    let expression = EditableSyntax.from_json(
      json.markup_expression, position, source);
    position += expression.width;
    return new MarkupSection(
        prefix,
        text,
        suffix,
        expression);
  }
  get children_keys()
  {
    if (MarkupSection._children_keys == null)
      MarkupSection._children_keys = [
        'prefix',
        'text',
        'suffix',
        'expression'];
    return MarkupSection._children_keys;
  }
}
class MarkupSuffix extends EditableSyntax
{
  constructor(
    less_than_question,
    name)
  {
    super('markup_suffix', {
      less_than_question: less_than_question,
      name: name });
  }
  get less_than_question() { return this.children.less_than_question; }
  get name() { return this.children.name; }
  with_less_than_question(less_than_question){
    return new MarkupSuffix(
      less_than_question,
      this.name);
  }
  with_name(name){
    return new MarkupSuffix(
      this.less_than_question,
      name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var less_than_question = this.less_than_question.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    if (
      less_than_question === this.less_than_question &&
      name === this.name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new MarkupSuffix(
        less_than_question,
        name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let less_than_question = EditableSyntax.from_json(
      json.markup_suffix_less_than_question, position, source);
    position += less_than_question.width;
    let name = EditableSyntax.from_json(
      json.markup_suffix_name, position, source);
    position += name.width;
    return new MarkupSuffix(
        less_than_question,
        name);
  }
  get children_keys()
  {
    if (MarkupSuffix._children_keys == null)
      MarkupSuffix._children_keys = [
        'less_than_question',
        'name'];
    return MarkupSuffix._children_keys;
  }
}
class UnsetStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    variables,
    right_paren,
    semicolon)
  {
    super('unset_statement', {
      keyword: keyword,
      left_paren: left_paren,
      variables: variables,
      right_paren: right_paren,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get variables() { return this.children.variables; }
  get right_paren() { return this.children.right_paren; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new UnsetStatement(
      keyword,
      this.left_paren,
      this.variables,
      this.right_paren,
      this.semicolon);
  }
  with_left_paren(left_paren){
    return new UnsetStatement(
      this.keyword,
      left_paren,
      this.variables,
      this.right_paren,
      this.semicolon);
  }
  with_variables(variables){
    return new UnsetStatement(
      this.keyword,
      this.left_paren,
      variables,
      this.right_paren,
      this.semicolon);
  }
  with_right_paren(right_paren){
    return new UnsetStatement(
      this.keyword,
      this.left_paren,
      this.variables,
      right_paren,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new UnsetStatement(
      this.keyword,
      this.left_paren,
      this.variables,
      this.right_paren,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var variables = this.variables.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      variables === this.variables &&
      right_paren === this.right_paren &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new UnsetStatement(
        keyword,
        left_paren,
        variables,
        right_paren,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.unset_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.unset_left_paren, position, source);
    position += left_paren.width;
    let variables = EditableSyntax.from_json(
      json.unset_variables, position, source);
    position += variables.width;
    let right_paren = EditableSyntax.from_json(
      json.unset_right_paren, position, source);
    position += right_paren.width;
    let semicolon = EditableSyntax.from_json(
      json.unset_semicolon, position, source);
    position += semicolon.width;
    return new UnsetStatement(
        keyword,
        left_paren,
        variables,
        right_paren,
        semicolon);
  }
  get children_keys()
  {
    if (UnsetStatement._children_keys == null)
      UnsetStatement._children_keys = [
        'keyword',
        'left_paren',
        'variables',
        'right_paren',
        'semicolon'];
    return UnsetStatement._children_keys;
  }
}
class WhileStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    condition,
    right_paren,
    body)
  {
    super('while_statement', {
      keyword: keyword,
      left_paren: left_paren,
      condition: condition,
      right_paren: right_paren,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get condition() { return this.children.condition; }
  get right_paren() { return this.children.right_paren; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new WhileStatement(
      keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.body);
  }
  with_left_paren(left_paren){
    return new WhileStatement(
      this.keyword,
      left_paren,
      this.condition,
      this.right_paren,
      this.body);
  }
  with_condition(condition){
    return new WhileStatement(
      this.keyword,
      this.left_paren,
      condition,
      this.right_paren,
      this.body);
  }
  with_right_paren(right_paren){
    return new WhileStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      right_paren,
      this.body);
  }
  with_body(body){
    return new WhileStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var condition = this.condition.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      condition === this.condition &&
      right_paren === this.right_paren &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new WhileStatement(
        keyword,
        left_paren,
        condition,
        right_paren,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.while_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.while_left_paren, position, source);
    position += left_paren.width;
    let condition = EditableSyntax.from_json(
      json.while_condition, position, source);
    position += condition.width;
    let right_paren = EditableSyntax.from_json(
      json.while_right_paren, position, source);
    position += right_paren.width;
    let body = EditableSyntax.from_json(
      json.while_body, position, source);
    position += body.width;
    return new WhileStatement(
        keyword,
        left_paren,
        condition,
        right_paren,
        body);
  }
  get children_keys()
  {
    if (WhileStatement._children_keys == null)
      WhileStatement._children_keys = [
        'keyword',
        'left_paren',
        'condition',
        'right_paren',
        'body'];
    return WhileStatement._children_keys;
  }
}
class IfStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    condition,
    right_paren,
    statement,
    elseif_clauses,
    else_clause)
  {
    super('if_statement', {
      keyword: keyword,
      left_paren: left_paren,
      condition: condition,
      right_paren: right_paren,
      statement: statement,
      elseif_clauses: elseif_clauses,
      else_clause: else_clause });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get condition() { return this.children.condition; }
  get right_paren() { return this.children.right_paren; }
  get statement() { return this.children.statement; }
  get elseif_clauses() { return this.children.elseif_clauses; }
  get else_clause() { return this.children.else_clause; }
  with_keyword(keyword){
    return new IfStatement(
      keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.statement,
      this.elseif_clauses,
      this.else_clause);
  }
  with_left_paren(left_paren){
    return new IfStatement(
      this.keyword,
      left_paren,
      this.condition,
      this.right_paren,
      this.statement,
      this.elseif_clauses,
      this.else_clause);
  }
  with_condition(condition){
    return new IfStatement(
      this.keyword,
      this.left_paren,
      condition,
      this.right_paren,
      this.statement,
      this.elseif_clauses,
      this.else_clause);
  }
  with_right_paren(right_paren){
    return new IfStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      right_paren,
      this.statement,
      this.elseif_clauses,
      this.else_clause);
  }
  with_statement(statement){
    return new IfStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      statement,
      this.elseif_clauses,
      this.else_clause);
  }
  with_elseif_clauses(elseif_clauses){
    return new IfStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.statement,
      elseif_clauses,
      this.else_clause);
  }
  with_else_clause(else_clause){
    return new IfStatement(
      this.keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.statement,
      this.elseif_clauses,
      else_clause);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var condition = this.condition.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var statement = this.statement.rewrite(rewriter, new_parents);
    var elseif_clauses = this.elseif_clauses.rewrite(rewriter, new_parents);
    var else_clause = this.else_clause.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      condition === this.condition &&
      right_paren === this.right_paren &&
      statement === this.statement &&
      elseif_clauses === this.elseif_clauses &&
      else_clause === this.else_clause)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new IfStatement(
        keyword,
        left_paren,
        condition,
        right_paren,
        statement,
        elseif_clauses,
        else_clause), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.if_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.if_left_paren, position, source);
    position += left_paren.width;
    let condition = EditableSyntax.from_json(
      json.if_condition, position, source);
    position += condition.width;
    let right_paren = EditableSyntax.from_json(
      json.if_right_paren, position, source);
    position += right_paren.width;
    let statement = EditableSyntax.from_json(
      json.if_statement, position, source);
    position += statement.width;
    let elseif_clauses = EditableSyntax.from_json(
      json.if_elseif_clauses, position, source);
    position += elseif_clauses.width;
    let else_clause = EditableSyntax.from_json(
      json.if_else_clause, position, source);
    position += else_clause.width;
    return new IfStatement(
        keyword,
        left_paren,
        condition,
        right_paren,
        statement,
        elseif_clauses,
        else_clause);
  }
  get children_keys()
  {
    if (IfStatement._children_keys == null)
      IfStatement._children_keys = [
        'keyword',
        'left_paren',
        'condition',
        'right_paren',
        'statement',
        'elseif_clauses',
        'else_clause'];
    return IfStatement._children_keys;
  }
}
class ElseifClause extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    condition,
    right_paren,
    statement)
  {
    super('elseif_clause', {
      keyword: keyword,
      left_paren: left_paren,
      condition: condition,
      right_paren: right_paren,
      statement: statement });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get condition() { return this.children.condition; }
  get right_paren() { return this.children.right_paren; }
  get statement() { return this.children.statement; }
  with_keyword(keyword){
    return new ElseifClause(
      keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.statement);
  }
  with_left_paren(left_paren){
    return new ElseifClause(
      this.keyword,
      left_paren,
      this.condition,
      this.right_paren,
      this.statement);
  }
  with_condition(condition){
    return new ElseifClause(
      this.keyword,
      this.left_paren,
      condition,
      this.right_paren,
      this.statement);
  }
  with_right_paren(right_paren){
    return new ElseifClause(
      this.keyword,
      this.left_paren,
      this.condition,
      right_paren,
      this.statement);
  }
  with_statement(statement){
    return new ElseifClause(
      this.keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      statement);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var condition = this.condition.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var statement = this.statement.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      condition === this.condition &&
      right_paren === this.right_paren &&
      statement === this.statement)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ElseifClause(
        keyword,
        left_paren,
        condition,
        right_paren,
        statement), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.elseif_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.elseif_left_paren, position, source);
    position += left_paren.width;
    let condition = EditableSyntax.from_json(
      json.elseif_condition, position, source);
    position += condition.width;
    let right_paren = EditableSyntax.from_json(
      json.elseif_right_paren, position, source);
    position += right_paren.width;
    let statement = EditableSyntax.from_json(
      json.elseif_statement, position, source);
    position += statement.width;
    return new ElseifClause(
        keyword,
        left_paren,
        condition,
        right_paren,
        statement);
  }
  get children_keys()
  {
    if (ElseifClause._children_keys == null)
      ElseifClause._children_keys = [
        'keyword',
        'left_paren',
        'condition',
        'right_paren',
        'statement'];
    return ElseifClause._children_keys;
  }
}
class ElseClause extends EditableSyntax
{
  constructor(
    keyword,
    statement)
  {
    super('else_clause', {
      keyword: keyword,
      statement: statement });
  }
  get keyword() { return this.children.keyword; }
  get statement() { return this.children.statement; }
  with_keyword(keyword){
    return new ElseClause(
      keyword,
      this.statement);
  }
  with_statement(statement){
    return new ElseClause(
      this.keyword,
      statement);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var statement = this.statement.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      statement === this.statement)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ElseClause(
        keyword,
        statement), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.else_keyword, position, source);
    position += keyword.width;
    let statement = EditableSyntax.from_json(
      json.else_statement, position, source);
    position += statement.width;
    return new ElseClause(
        keyword,
        statement);
  }
  get children_keys()
  {
    if (ElseClause._children_keys == null)
      ElseClause._children_keys = [
        'keyword',
        'statement'];
    return ElseClause._children_keys;
  }
}
class TryStatement extends EditableSyntax
{
  constructor(
    keyword,
    compound_statement,
    catch_clauses,
    finally_clause)
  {
    super('try_statement', {
      keyword: keyword,
      compound_statement: compound_statement,
      catch_clauses: catch_clauses,
      finally_clause: finally_clause });
  }
  get keyword() { return this.children.keyword; }
  get compound_statement() { return this.children.compound_statement; }
  get catch_clauses() { return this.children.catch_clauses; }
  get finally_clause() { return this.children.finally_clause; }
  with_keyword(keyword){
    return new TryStatement(
      keyword,
      this.compound_statement,
      this.catch_clauses,
      this.finally_clause);
  }
  with_compound_statement(compound_statement){
    return new TryStatement(
      this.keyword,
      compound_statement,
      this.catch_clauses,
      this.finally_clause);
  }
  with_catch_clauses(catch_clauses){
    return new TryStatement(
      this.keyword,
      this.compound_statement,
      catch_clauses,
      this.finally_clause);
  }
  with_finally_clause(finally_clause){
    return new TryStatement(
      this.keyword,
      this.compound_statement,
      this.catch_clauses,
      finally_clause);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var compound_statement = this.compound_statement.rewrite(rewriter, new_parents);
    var catch_clauses = this.catch_clauses.rewrite(rewriter, new_parents);
    var finally_clause = this.finally_clause.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      compound_statement === this.compound_statement &&
      catch_clauses === this.catch_clauses &&
      finally_clause === this.finally_clause)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TryStatement(
        keyword,
        compound_statement,
        catch_clauses,
        finally_clause), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.try_keyword, position, source);
    position += keyword.width;
    let compound_statement = EditableSyntax.from_json(
      json.try_compound_statement, position, source);
    position += compound_statement.width;
    let catch_clauses = EditableSyntax.from_json(
      json.try_catch_clauses, position, source);
    position += catch_clauses.width;
    let finally_clause = EditableSyntax.from_json(
      json.try_finally_clause, position, source);
    position += finally_clause.width;
    return new TryStatement(
        keyword,
        compound_statement,
        catch_clauses,
        finally_clause);
  }
  get children_keys()
  {
    if (TryStatement._children_keys == null)
      TryStatement._children_keys = [
        'keyword',
        'compound_statement',
        'catch_clauses',
        'finally_clause'];
    return TryStatement._children_keys;
  }
}
class CatchClause extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    type,
    variable,
    right_paren,
    body)
  {
    super('catch_clause', {
      keyword: keyword,
      left_paren: left_paren,
      type: type,
      variable: variable,
      right_paren: right_paren,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get type() { return this.children.type; }
  get variable() { return this.children.variable; }
  get right_paren() { return this.children.right_paren; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new CatchClause(
      keyword,
      this.left_paren,
      this.type,
      this.variable,
      this.right_paren,
      this.body);
  }
  with_left_paren(left_paren){
    return new CatchClause(
      this.keyword,
      left_paren,
      this.type,
      this.variable,
      this.right_paren,
      this.body);
  }
  with_type(type){
    return new CatchClause(
      this.keyword,
      this.left_paren,
      type,
      this.variable,
      this.right_paren,
      this.body);
  }
  with_variable(variable){
    return new CatchClause(
      this.keyword,
      this.left_paren,
      this.type,
      variable,
      this.right_paren,
      this.body);
  }
  with_right_paren(right_paren){
    return new CatchClause(
      this.keyword,
      this.left_paren,
      this.type,
      this.variable,
      right_paren,
      this.body);
  }
  with_body(body){
    return new CatchClause(
      this.keyword,
      this.left_paren,
      this.type,
      this.variable,
      this.right_paren,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var variable = this.variable.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      type === this.type &&
      variable === this.variable &&
      right_paren === this.right_paren &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new CatchClause(
        keyword,
        left_paren,
        type,
        variable,
        right_paren,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.catch_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.catch_left_paren, position, source);
    position += left_paren.width;
    let type = EditableSyntax.from_json(
      json.catch_type, position, source);
    position += type.width;
    let variable = EditableSyntax.from_json(
      json.catch_variable, position, source);
    position += variable.width;
    let right_paren = EditableSyntax.from_json(
      json.catch_right_paren, position, source);
    position += right_paren.width;
    let body = EditableSyntax.from_json(
      json.catch_body, position, source);
    position += body.width;
    return new CatchClause(
        keyword,
        left_paren,
        type,
        variable,
        right_paren,
        body);
  }
  get children_keys()
  {
    if (CatchClause._children_keys == null)
      CatchClause._children_keys = [
        'keyword',
        'left_paren',
        'type',
        'variable',
        'right_paren',
        'body'];
    return CatchClause._children_keys;
  }
}
class FinallyClause extends EditableSyntax
{
  constructor(
    keyword,
    body)
  {
    super('finally_clause', {
      keyword: keyword,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new FinallyClause(
      keyword,
      this.body);
  }
  with_body(body){
    return new FinallyClause(
      this.keyword,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FinallyClause(
        keyword,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.finally_keyword, position, source);
    position += keyword.width;
    let body = EditableSyntax.from_json(
      json.finally_body, position, source);
    position += body.width;
    return new FinallyClause(
        keyword,
        body);
  }
  get children_keys()
  {
    if (FinallyClause._children_keys == null)
      FinallyClause._children_keys = [
        'keyword',
        'body'];
    return FinallyClause._children_keys;
  }
}
class DoStatement extends EditableSyntax
{
  constructor(
    keyword,
    body,
    while_keyword,
    left_paren,
    condition,
    right_paren,
    semicolon)
  {
    super('do_statement', {
      keyword: keyword,
      body: body,
      while_keyword: while_keyword,
      left_paren: left_paren,
      condition: condition,
      right_paren: right_paren,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get body() { return this.children.body; }
  get while_keyword() { return this.children.while_keyword; }
  get left_paren() { return this.children.left_paren; }
  get condition() { return this.children.condition; }
  get right_paren() { return this.children.right_paren; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new DoStatement(
      keyword,
      this.body,
      this.while_keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.semicolon);
  }
  with_body(body){
    return new DoStatement(
      this.keyword,
      body,
      this.while_keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.semicolon);
  }
  with_while_keyword(while_keyword){
    return new DoStatement(
      this.keyword,
      this.body,
      while_keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      this.semicolon);
  }
  with_left_paren(left_paren){
    return new DoStatement(
      this.keyword,
      this.body,
      this.while_keyword,
      left_paren,
      this.condition,
      this.right_paren,
      this.semicolon);
  }
  with_condition(condition){
    return new DoStatement(
      this.keyword,
      this.body,
      this.while_keyword,
      this.left_paren,
      condition,
      this.right_paren,
      this.semicolon);
  }
  with_right_paren(right_paren){
    return new DoStatement(
      this.keyword,
      this.body,
      this.while_keyword,
      this.left_paren,
      this.condition,
      right_paren,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new DoStatement(
      this.keyword,
      this.body,
      this.while_keyword,
      this.left_paren,
      this.condition,
      this.right_paren,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    var while_keyword = this.while_keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var condition = this.condition.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      body === this.body &&
      while_keyword === this.while_keyword &&
      left_paren === this.left_paren &&
      condition === this.condition &&
      right_paren === this.right_paren &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DoStatement(
        keyword,
        body,
        while_keyword,
        left_paren,
        condition,
        right_paren,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.do_keyword, position, source);
    position += keyword.width;
    let body = EditableSyntax.from_json(
      json.do_body, position, source);
    position += body.width;
    let while_keyword = EditableSyntax.from_json(
      json.do_while_keyword, position, source);
    position += while_keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.do_left_paren, position, source);
    position += left_paren.width;
    let condition = EditableSyntax.from_json(
      json.do_condition, position, source);
    position += condition.width;
    let right_paren = EditableSyntax.from_json(
      json.do_right_paren, position, source);
    position += right_paren.width;
    let semicolon = EditableSyntax.from_json(
      json.do_semicolon, position, source);
    position += semicolon.width;
    return new DoStatement(
        keyword,
        body,
        while_keyword,
        left_paren,
        condition,
        right_paren,
        semicolon);
  }
  get children_keys()
  {
    if (DoStatement._children_keys == null)
      DoStatement._children_keys = [
        'keyword',
        'body',
        'while_keyword',
        'left_paren',
        'condition',
        'right_paren',
        'semicolon'];
    return DoStatement._children_keys;
  }
}
class ForStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    initializer,
    first_semicolon,
    control,
    second_semicolon,
    end_of_loop,
    right_paren,
    body)
  {
    super('for_statement', {
      keyword: keyword,
      left_paren: left_paren,
      initializer: initializer,
      first_semicolon: first_semicolon,
      control: control,
      second_semicolon: second_semicolon,
      end_of_loop: end_of_loop,
      right_paren: right_paren,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get initializer() { return this.children.initializer; }
  get first_semicolon() { return this.children.first_semicolon; }
  get control() { return this.children.control; }
  get second_semicolon() { return this.children.second_semicolon; }
  get end_of_loop() { return this.children.end_of_loop; }
  get right_paren() { return this.children.right_paren; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new ForStatement(
      keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_left_paren(left_paren){
    return new ForStatement(
      this.keyword,
      left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_initializer(initializer){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_first_semicolon(first_semicolon){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_control(control){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_second_semicolon(second_semicolon){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      second_semicolon,
      this.end_of_loop,
      this.right_paren,
      this.body);
  }
  with_end_of_loop(end_of_loop){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      end_of_loop,
      this.right_paren,
      this.body);
  }
  with_right_paren(right_paren){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      right_paren,
      this.body);
  }
  with_body(body){
    return new ForStatement(
      this.keyword,
      this.left_paren,
      this.initializer,
      this.first_semicolon,
      this.control,
      this.second_semicolon,
      this.end_of_loop,
      this.right_paren,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var initializer = this.initializer.rewrite(rewriter, new_parents);
    var first_semicolon = this.first_semicolon.rewrite(rewriter, new_parents);
    var control = this.control.rewrite(rewriter, new_parents);
    var second_semicolon = this.second_semicolon.rewrite(rewriter, new_parents);
    var end_of_loop = this.end_of_loop.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      initializer === this.initializer &&
      first_semicolon === this.first_semicolon &&
      control === this.control &&
      second_semicolon === this.second_semicolon &&
      end_of_loop === this.end_of_loop &&
      right_paren === this.right_paren &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ForStatement(
        keyword,
        left_paren,
        initializer,
        first_semicolon,
        control,
        second_semicolon,
        end_of_loop,
        right_paren,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.for_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.for_left_paren, position, source);
    position += left_paren.width;
    let initializer = EditableSyntax.from_json(
      json.for_initializer, position, source);
    position += initializer.width;
    let first_semicolon = EditableSyntax.from_json(
      json.for_first_semicolon, position, source);
    position += first_semicolon.width;
    let control = EditableSyntax.from_json(
      json.for_control, position, source);
    position += control.width;
    let second_semicolon = EditableSyntax.from_json(
      json.for_second_semicolon, position, source);
    position += second_semicolon.width;
    let end_of_loop = EditableSyntax.from_json(
      json.for_end_of_loop, position, source);
    position += end_of_loop.width;
    let right_paren = EditableSyntax.from_json(
      json.for_right_paren, position, source);
    position += right_paren.width;
    let body = EditableSyntax.from_json(
      json.for_body, position, source);
    position += body.width;
    return new ForStatement(
        keyword,
        left_paren,
        initializer,
        first_semicolon,
        control,
        second_semicolon,
        end_of_loop,
        right_paren,
        body);
  }
  get children_keys()
  {
    if (ForStatement._children_keys == null)
      ForStatement._children_keys = [
        'keyword',
        'left_paren',
        'initializer',
        'first_semicolon',
        'control',
        'second_semicolon',
        'end_of_loop',
        'right_paren',
        'body'];
    return ForStatement._children_keys;
  }
}
class ForeachStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    collection,
    await_keyword,
    as,
    key,
    arrow,
    value,
    right_paren,
    body)
  {
    super('foreach_statement', {
      keyword: keyword,
      left_paren: left_paren,
      collection: collection,
      await_keyword: await_keyword,
      as: as,
      key: key,
      arrow: arrow,
      value: value,
      right_paren: right_paren,
      body: body });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get collection() { return this.children.collection; }
  get await_keyword() { return this.children.await_keyword; }
  get as() { return this.children.as; }
  get key() { return this.children.key; }
  get arrow() { return this.children.arrow; }
  get value() { return this.children.value; }
  get right_paren() { return this.children.right_paren; }
  get body() { return this.children.body; }
  with_keyword(keyword){
    return new ForeachStatement(
      keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_left_paren(left_paren){
    return new ForeachStatement(
      this.keyword,
      left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_collection(collection){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_await_keyword(await_keyword){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_as(as){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_key(key){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      key,
      this.arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_arrow(arrow){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      arrow,
      this.value,
      this.right_paren,
      this.body);
  }
  with_value(value){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      value,
      this.right_paren,
      this.body);
  }
  with_right_paren(right_paren){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      right_paren,
      this.body);
  }
  with_body(body){
    return new ForeachStatement(
      this.keyword,
      this.left_paren,
      this.collection,
      this.await_keyword,
      this.as,
      this.key,
      this.arrow,
      this.value,
      this.right_paren,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var collection = this.collection.rewrite(rewriter, new_parents);
    var await_keyword = this.await_keyword.rewrite(rewriter, new_parents);
    var as = this.as.rewrite(rewriter, new_parents);
    var key = this.key.rewrite(rewriter, new_parents);
    var arrow = this.arrow.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      collection === this.collection &&
      await_keyword === this.await_keyword &&
      as === this.as &&
      key === this.key &&
      arrow === this.arrow &&
      value === this.value &&
      right_paren === this.right_paren &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ForeachStatement(
        keyword,
        left_paren,
        collection,
        await_keyword,
        as,
        key,
        arrow,
        value,
        right_paren,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.foreach_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.foreach_left_paren, position, source);
    position += left_paren.width;
    let collection = EditableSyntax.from_json(
      json.foreach_collection, position, source);
    position += collection.width;
    let await_keyword = EditableSyntax.from_json(
      json.foreach_await_keyword, position, source);
    position += await_keyword.width;
    let as = EditableSyntax.from_json(
      json.foreach_as, position, source);
    position += as.width;
    let key = EditableSyntax.from_json(
      json.foreach_key, position, source);
    position += key.width;
    let arrow = EditableSyntax.from_json(
      json.foreach_arrow, position, source);
    position += arrow.width;
    let value = EditableSyntax.from_json(
      json.foreach_value, position, source);
    position += value.width;
    let right_paren = EditableSyntax.from_json(
      json.foreach_right_paren, position, source);
    position += right_paren.width;
    let body = EditableSyntax.from_json(
      json.foreach_body, position, source);
    position += body.width;
    return new ForeachStatement(
        keyword,
        left_paren,
        collection,
        await_keyword,
        as,
        key,
        arrow,
        value,
        right_paren,
        body);
  }
  get children_keys()
  {
    if (ForeachStatement._children_keys == null)
      ForeachStatement._children_keys = [
        'keyword',
        'left_paren',
        'collection',
        'await_keyword',
        'as',
        'key',
        'arrow',
        'value',
        'right_paren',
        'body'];
    return ForeachStatement._children_keys;
  }
}
class SwitchStatement extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    expression,
    right_paren,
    left_brace,
    sections,
    right_brace)
  {
    super('switch_statement', {
      keyword: keyword,
      left_paren: left_paren,
      expression: expression,
      right_paren: right_paren,
      left_brace: left_brace,
      sections: sections,
      right_brace: right_brace });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get expression() { return this.children.expression; }
  get right_paren() { return this.children.right_paren; }
  get left_brace() { return this.children.left_brace; }
  get sections() { return this.children.sections; }
  get right_brace() { return this.children.right_brace; }
  with_keyword(keyword){
    return new SwitchStatement(
      keyword,
      this.left_paren,
      this.expression,
      this.right_paren,
      this.left_brace,
      this.sections,
      this.right_brace);
  }
  with_left_paren(left_paren){
    return new SwitchStatement(
      this.keyword,
      left_paren,
      this.expression,
      this.right_paren,
      this.left_brace,
      this.sections,
      this.right_brace);
  }
  with_expression(expression){
    return new SwitchStatement(
      this.keyword,
      this.left_paren,
      expression,
      this.right_paren,
      this.left_brace,
      this.sections,
      this.right_brace);
  }
  with_right_paren(right_paren){
    return new SwitchStatement(
      this.keyword,
      this.left_paren,
      this.expression,
      right_paren,
      this.left_brace,
      this.sections,
      this.right_brace);
  }
  with_left_brace(left_brace){
    return new SwitchStatement(
      this.keyword,
      this.left_paren,
      this.expression,
      this.right_paren,
      left_brace,
      this.sections,
      this.right_brace);
  }
  with_sections(sections){
    return new SwitchStatement(
      this.keyword,
      this.left_paren,
      this.expression,
      this.right_paren,
      this.left_brace,
      sections,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new SwitchStatement(
      this.keyword,
      this.left_paren,
      this.expression,
      this.right_paren,
      this.left_brace,
      this.sections,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var sections = this.sections.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      expression === this.expression &&
      right_paren === this.right_paren &&
      left_brace === this.left_brace &&
      sections === this.sections &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SwitchStatement(
        keyword,
        left_paren,
        expression,
        right_paren,
        left_brace,
        sections,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.switch_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.switch_left_paren, position, source);
    position += left_paren.width;
    let expression = EditableSyntax.from_json(
      json.switch_expression, position, source);
    position += expression.width;
    let right_paren = EditableSyntax.from_json(
      json.switch_right_paren, position, source);
    position += right_paren.width;
    let left_brace = EditableSyntax.from_json(
      json.switch_left_brace, position, source);
    position += left_brace.width;
    let sections = EditableSyntax.from_json(
      json.switch_sections, position, source);
    position += sections.width;
    let right_brace = EditableSyntax.from_json(
      json.switch_right_brace, position, source);
    position += right_brace.width;
    return new SwitchStatement(
        keyword,
        left_paren,
        expression,
        right_paren,
        left_brace,
        sections,
        right_brace);
  }
  get children_keys()
  {
    if (SwitchStatement._children_keys == null)
      SwitchStatement._children_keys = [
        'keyword',
        'left_paren',
        'expression',
        'right_paren',
        'left_brace',
        'sections',
        'right_brace'];
    return SwitchStatement._children_keys;
  }
}
class SwitchSection extends EditableSyntax
{
  constructor(
    labels,
    statements,
    fallthrough)
  {
    super('switch_section', {
      labels: labels,
      statements: statements,
      fallthrough: fallthrough });
  }
  get labels() { return this.children.labels; }
  get statements() { return this.children.statements; }
  get fallthrough() { return this.children.fallthrough; }
  with_labels(labels){
    return new SwitchSection(
      labels,
      this.statements,
      this.fallthrough);
  }
  with_statements(statements){
    return new SwitchSection(
      this.labels,
      statements,
      this.fallthrough);
  }
  with_fallthrough(fallthrough){
    return new SwitchSection(
      this.labels,
      this.statements,
      fallthrough);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var labels = this.labels.rewrite(rewriter, new_parents);
    var statements = this.statements.rewrite(rewriter, new_parents);
    var fallthrough = this.fallthrough.rewrite(rewriter, new_parents);
    if (
      labels === this.labels &&
      statements === this.statements &&
      fallthrough === this.fallthrough)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SwitchSection(
        labels,
        statements,
        fallthrough), parents);
    }
  }
  static from_json(json, position, source)
  {
    let labels = EditableSyntax.from_json(
      json.switch_section_labels, position, source);
    position += labels.width;
    let statements = EditableSyntax.from_json(
      json.switch_section_statements, position, source);
    position += statements.width;
    let fallthrough = EditableSyntax.from_json(
      json.switch_section_fallthrough, position, source);
    position += fallthrough.width;
    return new SwitchSection(
        labels,
        statements,
        fallthrough);
  }
  get children_keys()
  {
    if (SwitchSection._children_keys == null)
      SwitchSection._children_keys = [
        'labels',
        'statements',
        'fallthrough'];
    return SwitchSection._children_keys;
  }
}
class SwitchFallthrough extends EditableSyntax
{
  constructor(
    keyword,
    semicolon)
  {
    super('switch_fallthrough', {
      keyword: keyword,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new SwitchFallthrough(
      keyword,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new SwitchFallthrough(
      this.keyword,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SwitchFallthrough(
        keyword,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.fallthrough_keyword, position, source);
    position += keyword.width;
    let semicolon = EditableSyntax.from_json(
      json.fallthrough_semicolon, position, source);
    position += semicolon.width;
    return new SwitchFallthrough(
        keyword,
        semicolon);
  }
  get children_keys()
  {
    if (SwitchFallthrough._children_keys == null)
      SwitchFallthrough._children_keys = [
        'keyword',
        'semicolon'];
    return SwitchFallthrough._children_keys;
  }
}
class CaseLabel extends EditableSyntax
{
  constructor(
    keyword,
    expression,
    colon)
  {
    super('case_label', {
      keyword: keyword,
      expression: expression,
      colon: colon });
  }
  get keyword() { return this.children.keyword; }
  get expression() { return this.children.expression; }
  get colon() { return this.children.colon; }
  with_keyword(keyword){
    return new CaseLabel(
      keyword,
      this.expression,
      this.colon);
  }
  with_expression(expression){
    return new CaseLabel(
      this.keyword,
      expression,
      this.colon);
  }
  with_colon(colon){
    return new CaseLabel(
      this.keyword,
      this.expression,
      colon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      expression === this.expression &&
      colon === this.colon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new CaseLabel(
        keyword,
        expression,
        colon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.case_keyword, position, source);
    position += keyword.width;
    let expression = EditableSyntax.from_json(
      json.case_expression, position, source);
    position += expression.width;
    let colon = EditableSyntax.from_json(
      json.case_colon, position, source);
    position += colon.width;
    return new CaseLabel(
        keyword,
        expression,
        colon);
  }
  get children_keys()
  {
    if (CaseLabel._children_keys == null)
      CaseLabel._children_keys = [
        'keyword',
        'expression',
        'colon'];
    return CaseLabel._children_keys;
  }
}
class DefaultLabel extends EditableSyntax
{
  constructor(
    keyword,
    colon)
  {
    super('default_label', {
      keyword: keyword,
      colon: colon });
  }
  get keyword() { return this.children.keyword; }
  get colon() { return this.children.colon; }
  with_keyword(keyword){
    return new DefaultLabel(
      keyword,
      this.colon);
  }
  with_colon(colon){
    return new DefaultLabel(
      this.keyword,
      colon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      colon === this.colon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DefaultLabel(
        keyword,
        colon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.default_keyword, position, source);
    position += keyword.width;
    let colon = EditableSyntax.from_json(
      json.default_colon, position, source);
    position += colon.width;
    return new DefaultLabel(
        keyword,
        colon);
  }
  get children_keys()
  {
    if (DefaultLabel._children_keys == null)
      DefaultLabel._children_keys = [
        'keyword',
        'colon'];
    return DefaultLabel._children_keys;
  }
}
class ReturnStatement extends EditableSyntax
{
  constructor(
    keyword,
    expression,
    semicolon)
  {
    super('return_statement', {
      keyword: keyword,
      expression: expression,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get expression() { return this.children.expression; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new ReturnStatement(
      keyword,
      this.expression,
      this.semicolon);
  }
  with_expression(expression){
    return new ReturnStatement(
      this.keyword,
      expression,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new ReturnStatement(
      this.keyword,
      this.expression,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      expression === this.expression &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ReturnStatement(
        keyword,
        expression,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.return_keyword, position, source);
    position += keyword.width;
    let expression = EditableSyntax.from_json(
      json.return_expression, position, source);
    position += expression.width;
    let semicolon = EditableSyntax.from_json(
      json.return_semicolon, position, source);
    position += semicolon.width;
    return new ReturnStatement(
        keyword,
        expression,
        semicolon);
  }
  get children_keys()
  {
    if (ReturnStatement._children_keys == null)
      ReturnStatement._children_keys = [
        'keyword',
        'expression',
        'semicolon'];
    return ReturnStatement._children_keys;
  }
}
class GotoLabel extends EditableSyntax
{
  constructor(
    name,
    colon)
  {
    super('goto_label', {
      name: name,
      colon: colon });
  }
  get name() { return this.children.name; }
  get colon() { return this.children.colon; }
  with_name(name){
    return new GotoLabel(
      name,
      this.colon);
  }
  with_colon(colon){
    return new GotoLabel(
      this.name,
      colon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      colon === this.colon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new GotoLabel(
        name,
        colon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.goto_label_name, position, source);
    position += name.width;
    let colon = EditableSyntax.from_json(
      json.goto_label_colon, position, source);
    position += colon.width;
    return new GotoLabel(
        name,
        colon);
  }
  get children_keys()
  {
    if (GotoLabel._children_keys == null)
      GotoLabel._children_keys = [
        'name',
        'colon'];
    return GotoLabel._children_keys;
  }
}
class GotoStatement extends EditableSyntax
{
  constructor(
    keyword,
    label_name,
    semicolon)
  {
    super('goto_statement', {
      keyword: keyword,
      label_name: label_name,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get label_name() { return this.children.label_name; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new GotoStatement(
      keyword,
      this.label_name,
      this.semicolon);
  }
  with_label_name(label_name){
    return new GotoStatement(
      this.keyword,
      label_name,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new GotoStatement(
      this.keyword,
      this.label_name,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var label_name = this.label_name.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      label_name === this.label_name &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new GotoStatement(
        keyword,
        label_name,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.goto_statement_keyword, position, source);
    position += keyword.width;
    let label_name = EditableSyntax.from_json(
      json.goto_statement_label_name, position, source);
    position += label_name.width;
    let semicolon = EditableSyntax.from_json(
      json.goto_statement_semicolon, position, source);
    position += semicolon.width;
    return new GotoStatement(
        keyword,
        label_name,
        semicolon);
  }
  get children_keys()
  {
    if (GotoStatement._children_keys == null)
      GotoStatement._children_keys = [
        'keyword',
        'label_name',
        'semicolon'];
    return GotoStatement._children_keys;
  }
}
class ThrowStatement extends EditableSyntax
{
  constructor(
    keyword,
    expression,
    semicolon)
  {
    super('throw_statement', {
      keyword: keyword,
      expression: expression,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get expression() { return this.children.expression; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new ThrowStatement(
      keyword,
      this.expression,
      this.semicolon);
  }
  with_expression(expression){
    return new ThrowStatement(
      this.keyword,
      expression,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new ThrowStatement(
      this.keyword,
      this.expression,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      expression === this.expression &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ThrowStatement(
        keyword,
        expression,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.throw_keyword, position, source);
    position += keyword.width;
    let expression = EditableSyntax.from_json(
      json.throw_expression, position, source);
    position += expression.width;
    let semicolon = EditableSyntax.from_json(
      json.throw_semicolon, position, source);
    position += semicolon.width;
    return new ThrowStatement(
        keyword,
        expression,
        semicolon);
  }
  get children_keys()
  {
    if (ThrowStatement._children_keys == null)
      ThrowStatement._children_keys = [
        'keyword',
        'expression',
        'semicolon'];
    return ThrowStatement._children_keys;
  }
}
class BreakStatement extends EditableSyntax
{
  constructor(
    keyword,
    level,
    semicolon)
  {
    super('break_statement', {
      keyword: keyword,
      level: level,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get level() { return this.children.level; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new BreakStatement(
      keyword,
      this.level,
      this.semicolon);
  }
  with_level(level){
    return new BreakStatement(
      this.keyword,
      level,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new BreakStatement(
      this.keyword,
      this.level,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var level = this.level.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      level === this.level &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new BreakStatement(
        keyword,
        level,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.break_keyword, position, source);
    position += keyword.width;
    let level = EditableSyntax.from_json(
      json.break_level, position, source);
    position += level.width;
    let semicolon = EditableSyntax.from_json(
      json.break_semicolon, position, source);
    position += semicolon.width;
    return new BreakStatement(
        keyword,
        level,
        semicolon);
  }
  get children_keys()
  {
    if (BreakStatement._children_keys == null)
      BreakStatement._children_keys = [
        'keyword',
        'level',
        'semicolon'];
    return BreakStatement._children_keys;
  }
}
class ContinueStatement extends EditableSyntax
{
  constructor(
    keyword,
    level,
    semicolon)
  {
    super('continue_statement', {
      keyword: keyword,
      level: level,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get level() { return this.children.level; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new ContinueStatement(
      keyword,
      this.level,
      this.semicolon);
  }
  with_level(level){
    return new ContinueStatement(
      this.keyword,
      level,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new ContinueStatement(
      this.keyword,
      this.level,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var level = this.level.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      level === this.level &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ContinueStatement(
        keyword,
        level,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.continue_keyword, position, source);
    position += keyword.width;
    let level = EditableSyntax.from_json(
      json.continue_level, position, source);
    position += level.width;
    let semicolon = EditableSyntax.from_json(
      json.continue_semicolon, position, source);
    position += semicolon.width;
    return new ContinueStatement(
        keyword,
        level,
        semicolon);
  }
  get children_keys()
  {
    if (ContinueStatement._children_keys == null)
      ContinueStatement._children_keys = [
        'keyword',
        'level',
        'semicolon'];
    return ContinueStatement._children_keys;
  }
}
class FunctionStaticStatement extends EditableSyntax
{
  constructor(
    static_keyword,
    declarations,
    semicolon)
  {
    super('function_static_statement', {
      static_keyword: static_keyword,
      declarations: declarations,
      semicolon: semicolon });
  }
  get static_keyword() { return this.children.static_keyword; }
  get declarations() { return this.children.declarations; }
  get semicolon() { return this.children.semicolon; }
  with_static_keyword(static_keyword){
    return new FunctionStaticStatement(
      static_keyword,
      this.declarations,
      this.semicolon);
  }
  with_declarations(declarations){
    return new FunctionStaticStatement(
      this.static_keyword,
      declarations,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new FunctionStaticStatement(
      this.static_keyword,
      this.declarations,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var static_keyword = this.static_keyword.rewrite(rewriter, new_parents);
    var declarations = this.declarations.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      static_keyword === this.static_keyword &&
      declarations === this.declarations &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FunctionStaticStatement(
        static_keyword,
        declarations,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let static_keyword = EditableSyntax.from_json(
      json.static_static_keyword, position, source);
    position += static_keyword.width;
    let declarations = EditableSyntax.from_json(
      json.static_declarations, position, source);
    position += declarations.width;
    let semicolon = EditableSyntax.from_json(
      json.static_semicolon, position, source);
    position += semicolon.width;
    return new FunctionStaticStatement(
        static_keyword,
        declarations,
        semicolon);
  }
  get children_keys()
  {
    if (FunctionStaticStatement._children_keys == null)
      FunctionStaticStatement._children_keys = [
        'static_keyword',
        'declarations',
        'semicolon'];
    return FunctionStaticStatement._children_keys;
  }
}
class StaticDeclarator extends EditableSyntax
{
  constructor(
    name,
    initializer)
  {
    super('static_declarator', {
      name: name,
      initializer: initializer });
  }
  get name() { return this.children.name; }
  get initializer() { return this.children.initializer; }
  with_name(name){
    return new StaticDeclarator(
      name,
      this.initializer);
  }
  with_initializer(initializer){
    return new StaticDeclarator(
      this.name,
      initializer);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var initializer = this.initializer.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      initializer === this.initializer)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new StaticDeclarator(
        name,
        initializer), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.static_name, position, source);
    position += name.width;
    let initializer = EditableSyntax.from_json(
      json.static_initializer, position, source);
    position += initializer.width;
    return new StaticDeclarator(
        name,
        initializer);
  }
  get children_keys()
  {
    if (StaticDeclarator._children_keys == null)
      StaticDeclarator._children_keys = [
        'name',
        'initializer'];
    return StaticDeclarator._children_keys;
  }
}
class EchoStatement extends EditableSyntax
{
  constructor(
    keyword,
    expressions,
    semicolon)
  {
    super('echo_statement', {
      keyword: keyword,
      expressions: expressions,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get expressions() { return this.children.expressions; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new EchoStatement(
      keyword,
      this.expressions,
      this.semicolon);
  }
  with_expressions(expressions){
    return new EchoStatement(
      this.keyword,
      expressions,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new EchoStatement(
      this.keyword,
      this.expressions,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var expressions = this.expressions.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      expressions === this.expressions &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EchoStatement(
        keyword,
        expressions,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.echo_keyword, position, source);
    position += keyword.width;
    let expressions = EditableSyntax.from_json(
      json.echo_expressions, position, source);
    position += expressions.width;
    let semicolon = EditableSyntax.from_json(
      json.echo_semicolon, position, source);
    position += semicolon.width;
    return new EchoStatement(
        keyword,
        expressions,
        semicolon);
  }
  get children_keys()
  {
    if (EchoStatement._children_keys == null)
      EchoStatement._children_keys = [
        'keyword',
        'expressions',
        'semicolon'];
    return EchoStatement._children_keys;
  }
}
class GlobalStatement extends EditableSyntax
{
  constructor(
    keyword,
    variables,
    semicolon)
  {
    super('global_statement', {
      keyword: keyword,
      variables: variables,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get variables() { return this.children.variables; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new GlobalStatement(
      keyword,
      this.variables,
      this.semicolon);
  }
  with_variables(variables){
    return new GlobalStatement(
      this.keyword,
      variables,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new GlobalStatement(
      this.keyword,
      this.variables,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var variables = this.variables.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      variables === this.variables &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new GlobalStatement(
        keyword,
        variables,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.global_keyword, position, source);
    position += keyword.width;
    let variables = EditableSyntax.from_json(
      json.global_variables, position, source);
    position += variables.width;
    let semicolon = EditableSyntax.from_json(
      json.global_semicolon, position, source);
    position += semicolon.width;
    return new GlobalStatement(
        keyword,
        variables,
        semicolon);
  }
  get children_keys()
  {
    if (GlobalStatement._children_keys == null)
      GlobalStatement._children_keys = [
        'keyword',
        'variables',
        'semicolon'];
    return GlobalStatement._children_keys;
  }
}
class SimpleInitializer extends EditableSyntax
{
  constructor(
    equal,
    value)
  {
    super('simple_initializer', {
      equal: equal,
      value: value });
  }
  get equal() { return this.children.equal; }
  get value() { return this.children.value; }
  with_equal(equal){
    return new SimpleInitializer(
      equal,
      this.value);
  }
  with_value(value){
    return new SimpleInitializer(
      this.equal,
      value);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var equal = this.equal.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    if (
      equal === this.equal &&
      value === this.value)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SimpleInitializer(
        equal,
        value), parents);
    }
  }
  static from_json(json, position, source)
  {
    let equal = EditableSyntax.from_json(
      json.simple_initializer_equal, position, source);
    position += equal.width;
    let value = EditableSyntax.from_json(
      json.simple_initializer_value, position, source);
    position += value.width;
    return new SimpleInitializer(
        equal,
        value);
  }
  get children_keys()
  {
    if (SimpleInitializer._children_keys == null)
      SimpleInitializer._children_keys = [
        'equal',
        'value'];
    return SimpleInitializer._children_keys;
  }
}
class AnonymousFunction extends EditableSyntax
{
  constructor(
    static_keyword,
    async_keyword,
    coroutine_keyword,
    function_keyword,
    left_paren,
    parameters,
    right_paren,
    colon,
    type,
    use,
    body)
  {
    super('anonymous_function', {
      static_keyword: static_keyword,
      async_keyword: async_keyword,
      coroutine_keyword: coroutine_keyword,
      function_keyword: function_keyword,
      left_paren: left_paren,
      parameters: parameters,
      right_paren: right_paren,
      colon: colon,
      type: type,
      use: use,
      body: body });
  }
  get static_keyword() { return this.children.static_keyword; }
  get async_keyword() { return this.children.async_keyword; }
  get coroutine_keyword() { return this.children.coroutine_keyword; }
  get function_keyword() { return this.children.function_keyword; }
  get left_paren() { return this.children.left_paren; }
  get parameters() { return this.children.parameters; }
  get right_paren() { return this.children.right_paren; }
  get colon() { return this.children.colon; }
  get type() { return this.children.type; }
  get use() { return this.children.use; }
  get body() { return this.children.body; }
  with_static_keyword(static_keyword){
    return new AnonymousFunction(
      static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_async_keyword(async_keyword){
    return new AnonymousFunction(
      this.static_keyword,
      async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_coroutine_keyword(coroutine_keyword){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_function_keyword(function_keyword){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_left_paren(left_paren){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_parameters(parameters){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_right_paren(right_paren){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      right_paren,
      this.colon,
      this.type,
      this.use,
      this.body);
  }
  with_colon(colon){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      colon,
      this.type,
      this.use,
      this.body);
  }
  with_type(type){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      type,
      this.use,
      this.body);
  }
  with_use(use){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      use,
      this.body);
  }
  with_body(body){
    return new AnonymousFunction(
      this.static_keyword,
      this.async_keyword,
      this.coroutine_keyword,
      this.function_keyword,
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type,
      this.use,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var static_keyword = this.static_keyword.rewrite(rewriter, new_parents);
    var async_keyword = this.async_keyword.rewrite(rewriter, new_parents);
    var coroutine_keyword = this.coroutine_keyword.rewrite(rewriter, new_parents);
    var function_keyword = this.function_keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var parameters = this.parameters.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var use = this.use.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      static_keyword === this.static_keyword &&
      async_keyword === this.async_keyword &&
      coroutine_keyword === this.coroutine_keyword &&
      function_keyword === this.function_keyword &&
      left_paren === this.left_paren &&
      parameters === this.parameters &&
      right_paren === this.right_paren &&
      colon === this.colon &&
      type === this.type &&
      use === this.use &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new AnonymousFunction(
        static_keyword,
        async_keyword,
        coroutine_keyword,
        function_keyword,
        left_paren,
        parameters,
        right_paren,
        colon,
        type,
        use,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let static_keyword = EditableSyntax.from_json(
      json.anonymous_static_keyword, position, source);
    position += static_keyword.width;
    let async_keyword = EditableSyntax.from_json(
      json.anonymous_async_keyword, position, source);
    position += async_keyword.width;
    let coroutine_keyword = EditableSyntax.from_json(
      json.anonymous_coroutine_keyword, position, source);
    position += coroutine_keyword.width;
    let function_keyword = EditableSyntax.from_json(
      json.anonymous_function_keyword, position, source);
    position += function_keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.anonymous_left_paren, position, source);
    position += left_paren.width;
    let parameters = EditableSyntax.from_json(
      json.anonymous_parameters, position, source);
    position += parameters.width;
    let right_paren = EditableSyntax.from_json(
      json.anonymous_right_paren, position, source);
    position += right_paren.width;
    let colon = EditableSyntax.from_json(
      json.anonymous_colon, position, source);
    position += colon.width;
    let type = EditableSyntax.from_json(
      json.anonymous_type, position, source);
    position += type.width;
    let use = EditableSyntax.from_json(
      json.anonymous_use, position, source);
    position += use.width;
    let body = EditableSyntax.from_json(
      json.anonymous_body, position, source);
    position += body.width;
    return new AnonymousFunction(
        static_keyword,
        async_keyword,
        coroutine_keyword,
        function_keyword,
        left_paren,
        parameters,
        right_paren,
        colon,
        type,
        use,
        body);
  }
  get children_keys()
  {
    if (AnonymousFunction._children_keys == null)
      AnonymousFunction._children_keys = [
        'static_keyword',
        'async_keyword',
        'coroutine_keyword',
        'function_keyword',
        'left_paren',
        'parameters',
        'right_paren',
        'colon',
        'type',
        'use',
        'body'];
    return AnonymousFunction._children_keys;
  }
}
class AnonymousFunctionUseClause extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    variables,
    right_paren)
  {
    super('anonymous_function_use_clause', {
      keyword: keyword,
      left_paren: left_paren,
      variables: variables,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get variables() { return this.children.variables; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new AnonymousFunctionUseClause(
      keyword,
      this.left_paren,
      this.variables,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new AnonymousFunctionUseClause(
      this.keyword,
      left_paren,
      this.variables,
      this.right_paren);
  }
  with_variables(variables){
    return new AnonymousFunctionUseClause(
      this.keyword,
      this.left_paren,
      variables,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new AnonymousFunctionUseClause(
      this.keyword,
      this.left_paren,
      this.variables,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var variables = this.variables.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      variables === this.variables &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new AnonymousFunctionUseClause(
        keyword,
        left_paren,
        variables,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.anonymous_use_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.anonymous_use_left_paren, position, source);
    position += left_paren.width;
    let variables = EditableSyntax.from_json(
      json.anonymous_use_variables, position, source);
    position += variables.width;
    let right_paren = EditableSyntax.from_json(
      json.anonymous_use_right_paren, position, source);
    position += right_paren.width;
    return new AnonymousFunctionUseClause(
        keyword,
        left_paren,
        variables,
        right_paren);
  }
  get children_keys()
  {
    if (AnonymousFunctionUseClause._children_keys == null)
      AnonymousFunctionUseClause._children_keys = [
        'keyword',
        'left_paren',
        'variables',
        'right_paren'];
    return AnonymousFunctionUseClause._children_keys;
  }
}
class LambdaExpression extends EditableSyntax
{
  constructor(
    async,
    coroutine,
    signature,
    arrow,
    body)
  {
    super('lambda_expression', {
      async: async,
      coroutine: coroutine,
      signature: signature,
      arrow: arrow,
      body: body });
  }
  get async() { return this.children.async; }
  get coroutine() { return this.children.coroutine; }
  get signature() { return this.children.signature; }
  get arrow() { return this.children.arrow; }
  get body() { return this.children.body; }
  with_async(async){
    return new LambdaExpression(
      async,
      this.coroutine,
      this.signature,
      this.arrow,
      this.body);
  }
  with_coroutine(coroutine){
    return new LambdaExpression(
      this.async,
      coroutine,
      this.signature,
      this.arrow,
      this.body);
  }
  with_signature(signature){
    return new LambdaExpression(
      this.async,
      this.coroutine,
      signature,
      this.arrow,
      this.body);
  }
  with_arrow(arrow){
    return new LambdaExpression(
      this.async,
      this.coroutine,
      this.signature,
      arrow,
      this.body);
  }
  with_body(body){
    return new LambdaExpression(
      this.async,
      this.coroutine,
      this.signature,
      this.arrow,
      body);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var async = this.async.rewrite(rewriter, new_parents);
    var coroutine = this.coroutine.rewrite(rewriter, new_parents);
    var signature = this.signature.rewrite(rewriter, new_parents);
    var arrow = this.arrow.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    if (
      async === this.async &&
      coroutine === this.coroutine &&
      signature === this.signature &&
      arrow === this.arrow &&
      body === this.body)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new LambdaExpression(
        async,
        coroutine,
        signature,
        arrow,
        body), parents);
    }
  }
  static from_json(json, position, source)
  {
    let async = EditableSyntax.from_json(
      json.lambda_async, position, source);
    position += async.width;
    let coroutine = EditableSyntax.from_json(
      json.lambda_coroutine, position, source);
    position += coroutine.width;
    let signature = EditableSyntax.from_json(
      json.lambda_signature, position, source);
    position += signature.width;
    let arrow = EditableSyntax.from_json(
      json.lambda_arrow, position, source);
    position += arrow.width;
    let body = EditableSyntax.from_json(
      json.lambda_body, position, source);
    position += body.width;
    return new LambdaExpression(
        async,
        coroutine,
        signature,
        arrow,
        body);
  }
  get children_keys()
  {
    if (LambdaExpression._children_keys == null)
      LambdaExpression._children_keys = [
        'async',
        'coroutine',
        'signature',
        'arrow',
        'body'];
    return LambdaExpression._children_keys;
  }
}
class LambdaSignature extends EditableSyntax
{
  constructor(
    left_paren,
    parameters,
    right_paren,
    colon,
    type)
  {
    super('lambda_signature', {
      left_paren: left_paren,
      parameters: parameters,
      right_paren: right_paren,
      colon: colon,
      type: type });
  }
  get left_paren() { return this.children.left_paren; }
  get parameters() { return this.children.parameters; }
  get right_paren() { return this.children.right_paren; }
  get colon() { return this.children.colon; }
  get type() { return this.children.type; }
  with_left_paren(left_paren){
    return new LambdaSignature(
      left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      this.type);
  }
  with_parameters(parameters){
    return new LambdaSignature(
      this.left_paren,
      parameters,
      this.right_paren,
      this.colon,
      this.type);
  }
  with_right_paren(right_paren){
    return new LambdaSignature(
      this.left_paren,
      this.parameters,
      right_paren,
      this.colon,
      this.type);
  }
  with_colon(colon){
    return new LambdaSignature(
      this.left_paren,
      this.parameters,
      this.right_paren,
      colon,
      this.type);
  }
  with_type(type){
    return new LambdaSignature(
      this.left_paren,
      this.parameters,
      this.right_paren,
      this.colon,
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var parameters = this.parameters.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      left_paren === this.left_paren &&
      parameters === this.parameters &&
      right_paren === this.right_paren &&
      colon === this.colon &&
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new LambdaSignature(
        left_paren,
        parameters,
        right_paren,
        colon,
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_paren = EditableSyntax.from_json(
      json.lambda_left_paren, position, source);
    position += left_paren.width;
    let parameters = EditableSyntax.from_json(
      json.lambda_parameters, position, source);
    position += parameters.width;
    let right_paren = EditableSyntax.from_json(
      json.lambda_right_paren, position, source);
    position += right_paren.width;
    let colon = EditableSyntax.from_json(
      json.lambda_colon, position, source);
    position += colon.width;
    let type = EditableSyntax.from_json(
      json.lambda_type, position, source);
    position += type.width;
    return new LambdaSignature(
        left_paren,
        parameters,
        right_paren,
        colon,
        type);
  }
  get children_keys()
  {
    if (LambdaSignature._children_keys == null)
      LambdaSignature._children_keys = [
        'left_paren',
        'parameters',
        'right_paren',
        'colon',
        'type'];
    return LambdaSignature._children_keys;
  }
}
class CastExpression extends EditableSyntax
{
  constructor(
    left_paren,
    type,
    right_paren,
    operand)
  {
    super('cast_expression', {
      left_paren: left_paren,
      type: type,
      right_paren: right_paren,
      operand: operand });
  }
  get left_paren() { return this.children.left_paren; }
  get type() { return this.children.type; }
  get right_paren() { return this.children.right_paren; }
  get operand() { return this.children.operand; }
  with_left_paren(left_paren){
    return new CastExpression(
      left_paren,
      this.type,
      this.right_paren,
      this.operand);
  }
  with_type(type){
    return new CastExpression(
      this.left_paren,
      type,
      this.right_paren,
      this.operand);
  }
  with_right_paren(right_paren){
    return new CastExpression(
      this.left_paren,
      this.type,
      right_paren,
      this.operand);
  }
  with_operand(operand){
    return new CastExpression(
      this.left_paren,
      this.type,
      this.right_paren,
      operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    var operand = this.operand.rewrite(rewriter, new_parents);
    if (
      left_paren === this.left_paren &&
      type === this.type &&
      right_paren === this.right_paren &&
      operand === this.operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new CastExpression(
        left_paren,
        type,
        right_paren,
        operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_paren = EditableSyntax.from_json(
      json.cast_left_paren, position, source);
    position += left_paren.width;
    let type = EditableSyntax.from_json(
      json.cast_type, position, source);
    position += type.width;
    let right_paren = EditableSyntax.from_json(
      json.cast_right_paren, position, source);
    position += right_paren.width;
    let operand = EditableSyntax.from_json(
      json.cast_operand, position, source);
    position += operand.width;
    return new CastExpression(
        left_paren,
        type,
        right_paren,
        operand);
  }
  get children_keys()
  {
    if (CastExpression._children_keys == null)
      CastExpression._children_keys = [
        'left_paren',
        'type',
        'right_paren',
        'operand'];
    return CastExpression._children_keys;
  }
}
class ScopeResolutionExpression extends EditableSyntax
{
  constructor(
    qualifier,
    operator,
    name)
  {
    super('scope_resolution_expression', {
      qualifier: qualifier,
      operator: operator,
      name: name });
  }
  get qualifier() { return this.children.qualifier; }
  get operator() { return this.children.operator; }
  get name() { return this.children.name; }
  with_qualifier(qualifier){
    return new ScopeResolutionExpression(
      qualifier,
      this.operator,
      this.name);
  }
  with_operator(operator){
    return new ScopeResolutionExpression(
      this.qualifier,
      operator,
      this.name);
  }
  with_name(name){
    return new ScopeResolutionExpression(
      this.qualifier,
      this.operator,
      name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var qualifier = this.qualifier.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    if (
      qualifier === this.qualifier &&
      operator === this.operator &&
      name === this.name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ScopeResolutionExpression(
        qualifier,
        operator,
        name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let qualifier = EditableSyntax.from_json(
      json.scope_resolution_qualifier, position, source);
    position += qualifier.width;
    let operator = EditableSyntax.from_json(
      json.scope_resolution_operator, position, source);
    position += operator.width;
    let name = EditableSyntax.from_json(
      json.scope_resolution_name, position, source);
    position += name.width;
    return new ScopeResolutionExpression(
        qualifier,
        operator,
        name);
  }
  get children_keys()
  {
    if (ScopeResolutionExpression._children_keys == null)
      ScopeResolutionExpression._children_keys = [
        'qualifier',
        'operator',
        'name'];
    return ScopeResolutionExpression._children_keys;
  }
}
class MemberSelectionExpression extends EditableSyntax
{
  constructor(
    object,
    operator,
    name)
  {
    super('member_selection_expression', {
      object: object,
      operator: operator,
      name: name });
  }
  get object() { return this.children.object; }
  get operator() { return this.children.operator; }
  get name() { return this.children.name; }
  with_object(object){
    return new MemberSelectionExpression(
      object,
      this.operator,
      this.name);
  }
  with_operator(operator){
    return new MemberSelectionExpression(
      this.object,
      operator,
      this.name);
  }
  with_name(name){
    return new MemberSelectionExpression(
      this.object,
      this.operator,
      name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var object = this.object.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    if (
      object === this.object &&
      operator === this.operator &&
      name === this.name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new MemberSelectionExpression(
        object,
        operator,
        name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let object = EditableSyntax.from_json(
      json.member_object, position, source);
    position += object.width;
    let operator = EditableSyntax.from_json(
      json.member_operator, position, source);
    position += operator.width;
    let name = EditableSyntax.from_json(
      json.member_name, position, source);
    position += name.width;
    return new MemberSelectionExpression(
        object,
        operator,
        name);
  }
  get children_keys()
  {
    if (MemberSelectionExpression._children_keys == null)
      MemberSelectionExpression._children_keys = [
        'object',
        'operator',
        'name'];
    return MemberSelectionExpression._children_keys;
  }
}
class SafeMemberSelectionExpression extends EditableSyntax
{
  constructor(
    object,
    operator,
    name)
  {
    super('safe_member_selection_expression', {
      object: object,
      operator: operator,
      name: name });
  }
  get object() { return this.children.object; }
  get operator() { return this.children.operator; }
  get name() { return this.children.name; }
  with_object(object){
    return new SafeMemberSelectionExpression(
      object,
      this.operator,
      this.name);
  }
  with_operator(operator){
    return new SafeMemberSelectionExpression(
      this.object,
      operator,
      this.name);
  }
  with_name(name){
    return new SafeMemberSelectionExpression(
      this.object,
      this.operator,
      name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var object = this.object.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    if (
      object === this.object &&
      operator === this.operator &&
      name === this.name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SafeMemberSelectionExpression(
        object,
        operator,
        name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let object = EditableSyntax.from_json(
      json.safe_member_object, position, source);
    position += object.width;
    let operator = EditableSyntax.from_json(
      json.safe_member_operator, position, source);
    position += operator.width;
    let name = EditableSyntax.from_json(
      json.safe_member_name, position, source);
    position += name.width;
    return new SafeMemberSelectionExpression(
        object,
        operator,
        name);
  }
  get children_keys()
  {
    if (SafeMemberSelectionExpression._children_keys == null)
      SafeMemberSelectionExpression._children_keys = [
        'object',
        'operator',
        'name'];
    return SafeMemberSelectionExpression._children_keys;
  }
}
class EmbeddedMemberSelectionExpression extends EditableSyntax
{
  constructor(
    object,
    operator,
    name)
  {
    super('embedded_member_selection_expression', {
      object: object,
      operator: operator,
      name: name });
  }
  get object() { return this.children.object; }
  get operator() { return this.children.operator; }
  get name() { return this.children.name; }
  with_object(object){
    return new EmbeddedMemberSelectionExpression(
      object,
      this.operator,
      this.name);
  }
  with_operator(operator){
    return new EmbeddedMemberSelectionExpression(
      this.object,
      operator,
      this.name);
  }
  with_name(name){
    return new EmbeddedMemberSelectionExpression(
      this.object,
      this.operator,
      name);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var object = this.object.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    if (
      object === this.object &&
      operator === this.operator &&
      name === this.name)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EmbeddedMemberSelectionExpression(
        object,
        operator,
        name), parents);
    }
  }
  static from_json(json, position, source)
  {
    let object = EditableSyntax.from_json(
      json.embedded_member_object, position, source);
    position += object.width;
    let operator = EditableSyntax.from_json(
      json.embedded_member_operator, position, source);
    position += operator.width;
    let name = EditableSyntax.from_json(
      json.embedded_member_name, position, source);
    position += name.width;
    return new EmbeddedMemberSelectionExpression(
        object,
        operator,
        name);
  }
  get children_keys()
  {
    if (EmbeddedMemberSelectionExpression._children_keys == null)
      EmbeddedMemberSelectionExpression._children_keys = [
        'object',
        'operator',
        'name'];
    return EmbeddedMemberSelectionExpression._children_keys;
  }
}
class YieldExpression extends EditableSyntax
{
  constructor(
    keyword,
    operand)
  {
    super('yield_expression', {
      keyword: keyword,
      operand: operand });
  }
  get keyword() { return this.children.keyword; }
  get operand() { return this.children.operand; }
  with_keyword(keyword){
    return new YieldExpression(
      keyword,
      this.operand);
  }
  with_operand(operand){
    return new YieldExpression(
      this.keyword,
      operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var operand = this.operand.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      operand === this.operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new YieldExpression(
        keyword,
        operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.yield_keyword, position, source);
    position += keyword.width;
    let operand = EditableSyntax.from_json(
      json.yield_operand, position, source);
    position += operand.width;
    return new YieldExpression(
        keyword,
        operand);
  }
  get children_keys()
  {
    if (YieldExpression._children_keys == null)
      YieldExpression._children_keys = [
        'keyword',
        'operand'];
    return YieldExpression._children_keys;
  }
}
class YieldFromExpression extends EditableSyntax
{
  constructor(
    yield_keyword,
    from_keyword,
    operand)
  {
    super('yield_from_expression', {
      yield_keyword: yield_keyword,
      from_keyword: from_keyword,
      operand: operand });
  }
  get yield_keyword() { return this.children.yield_keyword; }
  get from_keyword() { return this.children.from_keyword; }
  get operand() { return this.children.operand; }
  with_yield_keyword(yield_keyword){
    return new YieldFromExpression(
      yield_keyword,
      this.from_keyword,
      this.operand);
  }
  with_from_keyword(from_keyword){
    return new YieldFromExpression(
      this.yield_keyword,
      from_keyword,
      this.operand);
  }
  with_operand(operand){
    return new YieldFromExpression(
      this.yield_keyword,
      this.from_keyword,
      operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var yield_keyword = this.yield_keyword.rewrite(rewriter, new_parents);
    var from_keyword = this.from_keyword.rewrite(rewriter, new_parents);
    var operand = this.operand.rewrite(rewriter, new_parents);
    if (
      yield_keyword === this.yield_keyword &&
      from_keyword === this.from_keyword &&
      operand === this.operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new YieldFromExpression(
        yield_keyword,
        from_keyword,
        operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let yield_keyword = EditableSyntax.from_json(
      json.yield_from_yield_keyword, position, source);
    position += yield_keyword.width;
    let from_keyword = EditableSyntax.from_json(
      json.yield_from_from_keyword, position, source);
    position += from_keyword.width;
    let operand = EditableSyntax.from_json(
      json.yield_from_operand, position, source);
    position += operand.width;
    return new YieldFromExpression(
        yield_keyword,
        from_keyword,
        operand);
  }
  get children_keys()
  {
    if (YieldFromExpression._children_keys == null)
      YieldFromExpression._children_keys = [
        'yield_keyword',
        'from_keyword',
        'operand'];
    return YieldFromExpression._children_keys;
  }
}
class PrefixUnaryExpression extends EditableSyntax
{
  constructor(
    operator,
    operand)
  {
    super('prefix_unary_expression', {
      operator: operator,
      operand: operand });
  }
  get operator() { return this.children.operator; }
  get operand() { return this.children.operand; }
  with_operator(operator){
    return new PrefixUnaryExpression(
      operator,
      this.operand);
  }
  with_operand(operand){
    return new PrefixUnaryExpression(
      this.operator,
      operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var operand = this.operand.rewrite(rewriter, new_parents);
    if (
      operator === this.operator &&
      operand === this.operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new PrefixUnaryExpression(
        operator,
        operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let operator = EditableSyntax.from_json(
      json.prefix_unary_operator, position, source);
    position += operator.width;
    let operand = EditableSyntax.from_json(
      json.prefix_unary_operand, position, source);
    position += operand.width;
    return new PrefixUnaryExpression(
        operator,
        operand);
  }
  get children_keys()
  {
    if (PrefixUnaryExpression._children_keys == null)
      PrefixUnaryExpression._children_keys = [
        'operator',
        'operand'];
    return PrefixUnaryExpression._children_keys;
  }
}
class PostfixUnaryExpression extends EditableSyntax
{
  constructor(
    operand,
    operator)
  {
    super('postfix_unary_expression', {
      operand: operand,
      operator: operator });
  }
  get operand() { return this.children.operand; }
  get operator() { return this.children.operator; }
  with_operand(operand){
    return new PostfixUnaryExpression(
      operand,
      this.operator);
  }
  with_operator(operator){
    return new PostfixUnaryExpression(
      this.operand,
      operator);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var operand = this.operand.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    if (
      operand === this.operand &&
      operator === this.operator)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new PostfixUnaryExpression(
        operand,
        operator), parents);
    }
  }
  static from_json(json, position, source)
  {
    let operand = EditableSyntax.from_json(
      json.postfix_unary_operand, position, source);
    position += operand.width;
    let operator = EditableSyntax.from_json(
      json.postfix_unary_operator, position, source);
    position += operator.width;
    return new PostfixUnaryExpression(
        operand,
        operator);
  }
  get children_keys()
  {
    if (PostfixUnaryExpression._children_keys == null)
      PostfixUnaryExpression._children_keys = [
        'operand',
        'operator'];
    return PostfixUnaryExpression._children_keys;
  }
}
class BinaryExpression extends EditableSyntax
{
  constructor(
    left_operand,
    operator,
    right_operand)
  {
    super('binary_expression', {
      left_operand: left_operand,
      operator: operator,
      right_operand: right_operand });
  }
  get left_operand() { return this.children.left_operand; }
  get operator() { return this.children.operator; }
  get right_operand() { return this.children.right_operand; }
  with_left_operand(left_operand){
    return new BinaryExpression(
      left_operand,
      this.operator,
      this.right_operand);
  }
  with_operator(operator){
    return new BinaryExpression(
      this.left_operand,
      operator,
      this.right_operand);
  }
  with_right_operand(right_operand){
    return new BinaryExpression(
      this.left_operand,
      this.operator,
      right_operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_operand = this.left_operand.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var right_operand = this.right_operand.rewrite(rewriter, new_parents);
    if (
      left_operand === this.left_operand &&
      operator === this.operator &&
      right_operand === this.right_operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new BinaryExpression(
        left_operand,
        operator,
        right_operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_operand = EditableSyntax.from_json(
      json.binary_left_operand, position, source);
    position += left_operand.width;
    let operator = EditableSyntax.from_json(
      json.binary_operator, position, source);
    position += operator.width;
    let right_operand = EditableSyntax.from_json(
      json.binary_right_operand, position, source);
    position += right_operand.width;
    return new BinaryExpression(
        left_operand,
        operator,
        right_operand);
  }
  get children_keys()
  {
    if (BinaryExpression._children_keys == null)
      BinaryExpression._children_keys = [
        'left_operand',
        'operator',
        'right_operand'];
    return BinaryExpression._children_keys;
  }
}
class InstanceofExpression extends EditableSyntax
{
  constructor(
    left_operand,
    operator,
    right_operand)
  {
    super('instanceof_expression', {
      left_operand: left_operand,
      operator: operator,
      right_operand: right_operand });
  }
  get left_operand() { return this.children.left_operand; }
  get operator() { return this.children.operator; }
  get right_operand() { return this.children.right_operand; }
  with_left_operand(left_operand){
    return new InstanceofExpression(
      left_operand,
      this.operator,
      this.right_operand);
  }
  with_operator(operator){
    return new InstanceofExpression(
      this.left_operand,
      operator,
      this.right_operand);
  }
  with_right_operand(right_operand){
    return new InstanceofExpression(
      this.left_operand,
      this.operator,
      right_operand);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_operand = this.left_operand.rewrite(rewriter, new_parents);
    var operator = this.operator.rewrite(rewriter, new_parents);
    var right_operand = this.right_operand.rewrite(rewriter, new_parents);
    if (
      left_operand === this.left_operand &&
      operator === this.operator &&
      right_operand === this.right_operand)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new InstanceofExpression(
        left_operand,
        operator,
        right_operand), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_operand = EditableSyntax.from_json(
      json.instanceof_left_operand, position, source);
    position += left_operand.width;
    let operator = EditableSyntax.from_json(
      json.instanceof_operator, position, source);
    position += operator.width;
    let right_operand = EditableSyntax.from_json(
      json.instanceof_right_operand, position, source);
    position += right_operand.width;
    return new InstanceofExpression(
        left_operand,
        operator,
        right_operand);
  }
  get children_keys()
  {
    if (InstanceofExpression._children_keys == null)
      InstanceofExpression._children_keys = [
        'left_operand',
        'operator',
        'right_operand'];
    return InstanceofExpression._children_keys;
  }
}
class ConditionalExpression extends EditableSyntax
{
  constructor(
    test,
    question,
    consequence,
    colon,
    alternative)
  {
    super('conditional_expression', {
      test: test,
      question: question,
      consequence: consequence,
      colon: colon,
      alternative: alternative });
  }
  get test() { return this.children.test; }
  get question() { return this.children.question; }
  get consequence() { return this.children.consequence; }
  get colon() { return this.children.colon; }
  get alternative() { return this.children.alternative; }
  with_test(test){
    return new ConditionalExpression(
      test,
      this.question,
      this.consequence,
      this.colon,
      this.alternative);
  }
  with_question(question){
    return new ConditionalExpression(
      this.test,
      question,
      this.consequence,
      this.colon,
      this.alternative);
  }
  with_consequence(consequence){
    return new ConditionalExpression(
      this.test,
      this.question,
      consequence,
      this.colon,
      this.alternative);
  }
  with_colon(colon){
    return new ConditionalExpression(
      this.test,
      this.question,
      this.consequence,
      colon,
      this.alternative);
  }
  with_alternative(alternative){
    return new ConditionalExpression(
      this.test,
      this.question,
      this.consequence,
      this.colon,
      alternative);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var test = this.test.rewrite(rewriter, new_parents);
    var question = this.question.rewrite(rewriter, new_parents);
    var consequence = this.consequence.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var alternative = this.alternative.rewrite(rewriter, new_parents);
    if (
      test === this.test &&
      question === this.question &&
      consequence === this.consequence &&
      colon === this.colon &&
      alternative === this.alternative)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ConditionalExpression(
        test,
        question,
        consequence,
        colon,
        alternative), parents);
    }
  }
  static from_json(json, position, source)
  {
    let test = EditableSyntax.from_json(
      json.conditional_test, position, source);
    position += test.width;
    let question = EditableSyntax.from_json(
      json.conditional_question, position, source);
    position += question.width;
    let consequence = EditableSyntax.from_json(
      json.conditional_consequence, position, source);
    position += consequence.width;
    let colon = EditableSyntax.from_json(
      json.conditional_colon, position, source);
    position += colon.width;
    let alternative = EditableSyntax.from_json(
      json.conditional_alternative, position, source);
    position += alternative.width;
    return new ConditionalExpression(
        test,
        question,
        consequence,
        colon,
        alternative);
  }
  get children_keys()
  {
    if (ConditionalExpression._children_keys == null)
      ConditionalExpression._children_keys = [
        'test',
        'question',
        'consequence',
        'colon',
        'alternative'];
    return ConditionalExpression._children_keys;
  }
}
class EvalExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    argument,
    right_paren)
  {
    super('eval_expression', {
      keyword: keyword,
      left_paren: left_paren,
      argument: argument,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get argument() { return this.children.argument; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new EvalExpression(
      keyword,
      this.left_paren,
      this.argument,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new EvalExpression(
      this.keyword,
      left_paren,
      this.argument,
      this.right_paren);
  }
  with_argument(argument){
    return new EvalExpression(
      this.keyword,
      this.left_paren,
      argument,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new EvalExpression(
      this.keyword,
      this.left_paren,
      this.argument,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument = this.argument.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      argument === this.argument &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EvalExpression(
        keyword,
        left_paren,
        argument,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.eval_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.eval_left_paren, position, source);
    position += left_paren.width;
    let argument = EditableSyntax.from_json(
      json.eval_argument, position, source);
    position += argument.width;
    let right_paren = EditableSyntax.from_json(
      json.eval_right_paren, position, source);
    position += right_paren.width;
    return new EvalExpression(
        keyword,
        left_paren,
        argument,
        right_paren);
  }
  get children_keys()
  {
    if (EvalExpression._children_keys == null)
      EvalExpression._children_keys = [
        'keyword',
        'left_paren',
        'argument',
        'right_paren'];
    return EvalExpression._children_keys;
  }
}
class EmptyExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    argument,
    right_paren)
  {
    super('empty_expression', {
      keyword: keyword,
      left_paren: left_paren,
      argument: argument,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get argument() { return this.children.argument; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new EmptyExpression(
      keyword,
      this.left_paren,
      this.argument,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new EmptyExpression(
      this.keyword,
      left_paren,
      this.argument,
      this.right_paren);
  }
  with_argument(argument){
    return new EmptyExpression(
      this.keyword,
      this.left_paren,
      argument,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new EmptyExpression(
      this.keyword,
      this.left_paren,
      this.argument,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument = this.argument.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      argument === this.argument &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EmptyExpression(
        keyword,
        left_paren,
        argument,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.empty_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.empty_left_paren, position, source);
    position += left_paren.width;
    let argument = EditableSyntax.from_json(
      json.empty_argument, position, source);
    position += argument.width;
    let right_paren = EditableSyntax.from_json(
      json.empty_right_paren, position, source);
    position += right_paren.width;
    return new EmptyExpression(
        keyword,
        left_paren,
        argument,
        right_paren);
  }
  get children_keys()
  {
    if (EmptyExpression._children_keys == null)
      EmptyExpression._children_keys = [
        'keyword',
        'left_paren',
        'argument',
        'right_paren'];
    return EmptyExpression._children_keys;
  }
}
class DefineExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    argument_list,
    right_paren)
  {
    super('define_expression', {
      keyword: keyword,
      left_paren: left_paren,
      argument_list: argument_list,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get argument_list() { return this.children.argument_list; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new DefineExpression(
      keyword,
      this.left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new DefineExpression(
      this.keyword,
      left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_argument_list(argument_list){
    return new DefineExpression(
      this.keyword,
      this.left_paren,
      argument_list,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new DefineExpression(
      this.keyword,
      this.left_paren,
      this.argument_list,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument_list = this.argument_list.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      argument_list === this.argument_list &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DefineExpression(
        keyword,
        left_paren,
        argument_list,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.define_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.define_left_paren, position, source);
    position += left_paren.width;
    let argument_list = EditableSyntax.from_json(
      json.define_argument_list, position, source);
    position += argument_list.width;
    let right_paren = EditableSyntax.from_json(
      json.define_right_paren, position, source);
    position += right_paren.width;
    return new DefineExpression(
        keyword,
        left_paren,
        argument_list,
        right_paren);
  }
  get children_keys()
  {
    if (DefineExpression._children_keys == null)
      DefineExpression._children_keys = [
        'keyword',
        'left_paren',
        'argument_list',
        'right_paren'];
    return DefineExpression._children_keys;
  }
}
class IssetExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    argument_list,
    right_paren)
  {
    super('isset_expression', {
      keyword: keyword,
      left_paren: left_paren,
      argument_list: argument_list,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get argument_list() { return this.children.argument_list; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new IssetExpression(
      keyword,
      this.left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new IssetExpression(
      this.keyword,
      left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_argument_list(argument_list){
    return new IssetExpression(
      this.keyword,
      this.left_paren,
      argument_list,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new IssetExpression(
      this.keyword,
      this.left_paren,
      this.argument_list,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument_list = this.argument_list.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      argument_list === this.argument_list &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new IssetExpression(
        keyword,
        left_paren,
        argument_list,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.isset_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.isset_left_paren, position, source);
    position += left_paren.width;
    let argument_list = EditableSyntax.from_json(
      json.isset_argument_list, position, source);
    position += argument_list.width;
    let right_paren = EditableSyntax.from_json(
      json.isset_right_paren, position, source);
    position += right_paren.width;
    return new IssetExpression(
        keyword,
        left_paren,
        argument_list,
        right_paren);
  }
  get children_keys()
  {
    if (IssetExpression._children_keys == null)
      IssetExpression._children_keys = [
        'keyword',
        'left_paren',
        'argument_list',
        'right_paren'];
    return IssetExpression._children_keys;
  }
}
class FunctionCallExpression extends EditableSyntax
{
  constructor(
    receiver,
    left_paren,
    argument_list,
    right_paren)
  {
    super('function_call_expression', {
      receiver: receiver,
      left_paren: left_paren,
      argument_list: argument_list,
      right_paren: right_paren });
  }
  get receiver() { return this.children.receiver; }
  get left_paren() { return this.children.left_paren; }
  get argument_list() { return this.children.argument_list; }
  get right_paren() { return this.children.right_paren; }
  with_receiver(receiver){
    return new FunctionCallExpression(
      receiver,
      this.left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new FunctionCallExpression(
      this.receiver,
      left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_argument_list(argument_list){
    return new FunctionCallExpression(
      this.receiver,
      this.left_paren,
      argument_list,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new FunctionCallExpression(
      this.receiver,
      this.left_paren,
      this.argument_list,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var receiver = this.receiver.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument_list = this.argument_list.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      receiver === this.receiver &&
      left_paren === this.left_paren &&
      argument_list === this.argument_list &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FunctionCallExpression(
        receiver,
        left_paren,
        argument_list,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let receiver = EditableSyntax.from_json(
      json.function_call_receiver, position, source);
    position += receiver.width;
    let left_paren = EditableSyntax.from_json(
      json.function_call_left_paren, position, source);
    position += left_paren.width;
    let argument_list = EditableSyntax.from_json(
      json.function_call_argument_list, position, source);
    position += argument_list.width;
    let right_paren = EditableSyntax.from_json(
      json.function_call_right_paren, position, source);
    position += right_paren.width;
    return new FunctionCallExpression(
        receiver,
        left_paren,
        argument_list,
        right_paren);
  }
  get children_keys()
  {
    if (FunctionCallExpression._children_keys == null)
      FunctionCallExpression._children_keys = [
        'receiver',
        'left_paren',
        'argument_list',
        'right_paren'];
    return FunctionCallExpression._children_keys;
  }
}
class ParenthesizedExpression extends EditableSyntax
{
  constructor(
    left_paren,
    expression,
    right_paren)
  {
    super('parenthesized_expression', {
      left_paren: left_paren,
      expression: expression,
      right_paren: right_paren });
  }
  get left_paren() { return this.children.left_paren; }
  get expression() { return this.children.expression; }
  get right_paren() { return this.children.right_paren; }
  with_left_paren(left_paren){
    return new ParenthesizedExpression(
      left_paren,
      this.expression,
      this.right_paren);
  }
  with_expression(expression){
    return new ParenthesizedExpression(
      this.left_paren,
      expression,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ParenthesizedExpression(
      this.left_paren,
      this.expression,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      left_paren === this.left_paren &&
      expression === this.expression &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ParenthesizedExpression(
        left_paren,
        expression,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_paren = EditableSyntax.from_json(
      json.parenthesized_expression_left_paren, position, source);
    position += left_paren.width;
    let expression = EditableSyntax.from_json(
      json.parenthesized_expression_expression, position, source);
    position += expression.width;
    let right_paren = EditableSyntax.from_json(
      json.parenthesized_expression_right_paren, position, source);
    position += right_paren.width;
    return new ParenthesizedExpression(
        left_paren,
        expression,
        right_paren);
  }
  get children_keys()
  {
    if (ParenthesizedExpression._children_keys == null)
      ParenthesizedExpression._children_keys = [
        'left_paren',
        'expression',
        'right_paren'];
    return ParenthesizedExpression._children_keys;
  }
}
class BracedExpression extends EditableSyntax
{
  constructor(
    left_brace,
    expression,
    right_brace)
  {
    super('braced_expression', {
      left_brace: left_brace,
      expression: expression,
      right_brace: right_brace });
  }
  get left_brace() { return this.children.left_brace; }
  get expression() { return this.children.expression; }
  get right_brace() { return this.children.right_brace; }
  with_left_brace(left_brace){
    return new BracedExpression(
      left_brace,
      this.expression,
      this.right_brace);
  }
  with_expression(expression){
    return new BracedExpression(
      this.left_brace,
      expression,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new BracedExpression(
      this.left_brace,
      this.expression,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      left_brace === this.left_brace &&
      expression === this.expression &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new BracedExpression(
        left_brace,
        expression,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_brace = EditableSyntax.from_json(
      json.braced_expression_left_brace, position, source);
    position += left_brace.width;
    let expression = EditableSyntax.from_json(
      json.braced_expression_expression, position, source);
    position += expression.width;
    let right_brace = EditableSyntax.from_json(
      json.braced_expression_right_brace, position, source);
    position += right_brace.width;
    return new BracedExpression(
        left_brace,
        expression,
        right_brace);
  }
  get children_keys()
  {
    if (BracedExpression._children_keys == null)
      BracedExpression._children_keys = [
        'left_brace',
        'expression',
        'right_brace'];
    return BracedExpression._children_keys;
  }
}
class EmbeddedBracedExpression extends EditableSyntax
{
  constructor(
    left_brace,
    expression,
    right_brace)
  {
    super('embedded_braced_expression', {
      left_brace: left_brace,
      expression: expression,
      right_brace: right_brace });
  }
  get left_brace() { return this.children.left_brace; }
  get expression() { return this.children.expression; }
  get right_brace() { return this.children.right_brace; }
  with_left_brace(left_brace){
    return new EmbeddedBracedExpression(
      left_brace,
      this.expression,
      this.right_brace);
  }
  with_expression(expression){
    return new EmbeddedBracedExpression(
      this.left_brace,
      expression,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new EmbeddedBracedExpression(
      this.left_brace,
      this.expression,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      left_brace === this.left_brace &&
      expression === this.expression &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EmbeddedBracedExpression(
        left_brace,
        expression,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_brace = EditableSyntax.from_json(
      json.embedded_braced_expression_left_brace, position, source);
    position += left_brace.width;
    let expression = EditableSyntax.from_json(
      json.embedded_braced_expression_expression, position, source);
    position += expression.width;
    let right_brace = EditableSyntax.from_json(
      json.embedded_braced_expression_right_brace, position, source);
    position += right_brace.width;
    return new EmbeddedBracedExpression(
        left_brace,
        expression,
        right_brace);
  }
  get children_keys()
  {
    if (EmbeddedBracedExpression._children_keys == null)
      EmbeddedBracedExpression._children_keys = [
        'left_brace',
        'expression',
        'right_brace'];
    return EmbeddedBracedExpression._children_keys;
  }
}
class ListExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    members,
    right_paren)
  {
    super('list_expression', {
      keyword: keyword,
      left_paren: left_paren,
      members: members,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get members() { return this.children.members; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new ListExpression(
      keyword,
      this.left_paren,
      this.members,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new ListExpression(
      this.keyword,
      left_paren,
      this.members,
      this.right_paren);
  }
  with_members(members){
    return new ListExpression(
      this.keyword,
      this.left_paren,
      members,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ListExpression(
      this.keyword,
      this.left_paren,
      this.members,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      members === this.members &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ListExpression(
        keyword,
        left_paren,
        members,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.list_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.list_left_paren, position, source);
    position += left_paren.width;
    let members = EditableSyntax.from_json(
      json.list_members, position, source);
    position += members.width;
    let right_paren = EditableSyntax.from_json(
      json.list_right_paren, position, source);
    position += right_paren.width;
    return new ListExpression(
        keyword,
        left_paren,
        members,
        right_paren);
  }
  get children_keys()
  {
    if (ListExpression._children_keys == null)
      ListExpression._children_keys = [
        'keyword',
        'left_paren',
        'members',
        'right_paren'];
    return ListExpression._children_keys;
  }
}
class CollectionLiteralExpression extends EditableSyntax
{
  constructor(
    name,
    left_brace,
    initializers,
    right_brace)
  {
    super('collection_literal_expression', {
      name: name,
      left_brace: left_brace,
      initializers: initializers,
      right_brace: right_brace });
  }
  get name() { return this.children.name; }
  get left_brace() { return this.children.left_brace; }
  get initializers() { return this.children.initializers; }
  get right_brace() { return this.children.right_brace; }
  with_name(name){
    return new CollectionLiteralExpression(
      name,
      this.left_brace,
      this.initializers,
      this.right_brace);
  }
  with_left_brace(left_brace){
    return new CollectionLiteralExpression(
      this.name,
      left_brace,
      this.initializers,
      this.right_brace);
  }
  with_initializers(initializers){
    return new CollectionLiteralExpression(
      this.name,
      this.left_brace,
      initializers,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new CollectionLiteralExpression(
      this.name,
      this.left_brace,
      this.initializers,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var initializers = this.initializers.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      left_brace === this.left_brace &&
      initializers === this.initializers &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new CollectionLiteralExpression(
        name,
        left_brace,
        initializers,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.collection_literal_name, position, source);
    position += name.width;
    let left_brace = EditableSyntax.from_json(
      json.collection_literal_left_brace, position, source);
    position += left_brace.width;
    let initializers = EditableSyntax.from_json(
      json.collection_literal_initializers, position, source);
    position += initializers.width;
    let right_brace = EditableSyntax.from_json(
      json.collection_literal_right_brace, position, source);
    position += right_brace.width;
    return new CollectionLiteralExpression(
        name,
        left_brace,
        initializers,
        right_brace);
  }
  get children_keys()
  {
    if (CollectionLiteralExpression._children_keys == null)
      CollectionLiteralExpression._children_keys = [
        'name',
        'left_brace',
        'initializers',
        'right_brace'];
    return CollectionLiteralExpression._children_keys;
  }
}
class ObjectCreationExpression extends EditableSyntax
{
  constructor(
    new_keyword,
    type,
    left_paren,
    argument_list,
    right_paren)
  {
    super('object_creation_expression', {
      new_keyword: new_keyword,
      type: type,
      left_paren: left_paren,
      argument_list: argument_list,
      right_paren: right_paren });
  }
  get new_keyword() { return this.children.new_keyword; }
  get type() { return this.children.type; }
  get left_paren() { return this.children.left_paren; }
  get argument_list() { return this.children.argument_list; }
  get right_paren() { return this.children.right_paren; }
  with_new_keyword(new_keyword){
    return new ObjectCreationExpression(
      new_keyword,
      this.type,
      this.left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_type(type){
    return new ObjectCreationExpression(
      this.new_keyword,
      type,
      this.left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new ObjectCreationExpression(
      this.new_keyword,
      this.type,
      left_paren,
      this.argument_list,
      this.right_paren);
  }
  with_argument_list(argument_list){
    return new ObjectCreationExpression(
      this.new_keyword,
      this.type,
      this.left_paren,
      argument_list,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ObjectCreationExpression(
      this.new_keyword,
      this.type,
      this.left_paren,
      this.argument_list,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var new_keyword = this.new_keyword.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var argument_list = this.argument_list.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      new_keyword === this.new_keyword &&
      type === this.type &&
      left_paren === this.left_paren &&
      argument_list === this.argument_list &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ObjectCreationExpression(
        new_keyword,
        type,
        left_paren,
        argument_list,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let new_keyword = EditableSyntax.from_json(
      json.object_creation_new_keyword, position, source);
    position += new_keyword.width;
    let type = EditableSyntax.from_json(
      json.object_creation_type, position, source);
    position += type.width;
    let left_paren = EditableSyntax.from_json(
      json.object_creation_left_paren, position, source);
    position += left_paren.width;
    let argument_list = EditableSyntax.from_json(
      json.object_creation_argument_list, position, source);
    position += argument_list.width;
    let right_paren = EditableSyntax.from_json(
      json.object_creation_right_paren, position, source);
    position += right_paren.width;
    return new ObjectCreationExpression(
        new_keyword,
        type,
        left_paren,
        argument_list,
        right_paren);
  }
  get children_keys()
  {
    if (ObjectCreationExpression._children_keys == null)
      ObjectCreationExpression._children_keys = [
        'new_keyword',
        'type',
        'left_paren',
        'argument_list',
        'right_paren'];
    return ObjectCreationExpression._children_keys;
  }
}
class ArrayCreationExpression extends EditableSyntax
{
  constructor(
    left_bracket,
    members,
    right_bracket)
  {
    super('array_creation_expression', {
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_left_bracket(left_bracket){
    return new ArrayCreationExpression(
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new ArrayCreationExpression(
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new ArrayCreationExpression(
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ArrayCreationExpression(
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_bracket = EditableSyntax.from_json(
      json.array_creation_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.array_creation_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.array_creation_right_bracket, position, source);
    position += right_bracket.width;
    return new ArrayCreationExpression(
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (ArrayCreationExpression._children_keys == null)
      ArrayCreationExpression._children_keys = [
        'left_bracket',
        'members',
        'right_bracket'];
    return ArrayCreationExpression._children_keys;
  }
}
class ArrayIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    members,
    right_paren)
  {
    super('array_intrinsic_expression', {
      keyword: keyword,
      left_paren: left_paren,
      members: members,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get members() { return this.children.members; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new ArrayIntrinsicExpression(
      keyword,
      this.left_paren,
      this.members,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new ArrayIntrinsicExpression(
      this.keyword,
      left_paren,
      this.members,
      this.right_paren);
  }
  with_members(members){
    return new ArrayIntrinsicExpression(
      this.keyword,
      this.left_paren,
      members,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ArrayIntrinsicExpression(
      this.keyword,
      this.left_paren,
      this.members,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      members === this.members &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ArrayIntrinsicExpression(
        keyword,
        left_paren,
        members,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.array_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.array_intrinsic_left_paren, position, source);
    position += left_paren.width;
    let members = EditableSyntax.from_json(
      json.array_intrinsic_members, position, source);
    position += members.width;
    let right_paren = EditableSyntax.from_json(
      json.array_intrinsic_right_paren, position, source);
    position += right_paren.width;
    return new ArrayIntrinsicExpression(
        keyword,
        left_paren,
        members,
        right_paren);
  }
  get children_keys()
  {
    if (ArrayIntrinsicExpression._children_keys == null)
      ArrayIntrinsicExpression._children_keys = [
        'keyword',
        'left_paren',
        'members',
        'right_paren'];
    return ArrayIntrinsicExpression._children_keys;
  }
}
class DarrayIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_bracket,
    members,
    right_bracket)
  {
    super('darray_intrinsic_expression', {
      keyword: keyword,
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get keyword() { return this.children.keyword; }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_keyword(keyword){
    return new DarrayIntrinsicExpression(
      keyword,
      this.left_bracket,
      this.members,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new DarrayIntrinsicExpression(
      this.keyword,
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new DarrayIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new DarrayIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DarrayIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.darray_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_bracket = EditableSyntax.from_json(
      json.darray_intrinsic_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.darray_intrinsic_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.darray_intrinsic_right_bracket, position, source);
    position += right_bracket.width;
    return new DarrayIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (DarrayIntrinsicExpression._children_keys == null)
      DarrayIntrinsicExpression._children_keys = [
        'keyword',
        'left_bracket',
        'members',
        'right_bracket'];
    return DarrayIntrinsicExpression._children_keys;
  }
}
class DictionaryIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_bracket,
    members,
    right_bracket)
  {
    super('dictionary_intrinsic_expression', {
      keyword: keyword,
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get keyword() { return this.children.keyword; }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_keyword(keyword){
    return new DictionaryIntrinsicExpression(
      keyword,
      this.left_bracket,
      this.members,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new DictionaryIntrinsicExpression(
      this.keyword,
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new DictionaryIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new DictionaryIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DictionaryIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.dictionary_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_bracket = EditableSyntax.from_json(
      json.dictionary_intrinsic_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.dictionary_intrinsic_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.dictionary_intrinsic_right_bracket, position, source);
    position += right_bracket.width;
    return new DictionaryIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (DictionaryIntrinsicExpression._children_keys == null)
      DictionaryIntrinsicExpression._children_keys = [
        'keyword',
        'left_bracket',
        'members',
        'right_bracket'];
    return DictionaryIntrinsicExpression._children_keys;
  }
}
class KeysetIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_bracket,
    members,
    right_bracket)
  {
    super('keyset_intrinsic_expression', {
      keyword: keyword,
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get keyword() { return this.children.keyword; }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_keyword(keyword){
    return new KeysetIntrinsicExpression(
      keyword,
      this.left_bracket,
      this.members,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new KeysetIntrinsicExpression(
      this.keyword,
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new KeysetIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new KeysetIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new KeysetIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.keyset_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_bracket = EditableSyntax.from_json(
      json.keyset_intrinsic_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.keyset_intrinsic_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.keyset_intrinsic_right_bracket, position, source);
    position += right_bracket.width;
    return new KeysetIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (KeysetIntrinsicExpression._children_keys == null)
      KeysetIntrinsicExpression._children_keys = [
        'keyword',
        'left_bracket',
        'members',
        'right_bracket'];
    return KeysetIntrinsicExpression._children_keys;
  }
}
class VarrayIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_bracket,
    members,
    right_bracket)
  {
    super('varray_intrinsic_expression', {
      keyword: keyword,
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get keyword() { return this.children.keyword; }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_keyword(keyword){
    return new VarrayIntrinsicExpression(
      keyword,
      this.left_bracket,
      this.members,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new VarrayIntrinsicExpression(
      this.keyword,
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new VarrayIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new VarrayIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VarrayIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.varray_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_bracket = EditableSyntax.from_json(
      json.varray_intrinsic_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.varray_intrinsic_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.varray_intrinsic_right_bracket, position, source);
    position += right_bracket.width;
    return new VarrayIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (VarrayIntrinsicExpression._children_keys == null)
      VarrayIntrinsicExpression._children_keys = [
        'keyword',
        'left_bracket',
        'members',
        'right_bracket'];
    return VarrayIntrinsicExpression._children_keys;
  }
}
class VectorIntrinsicExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_bracket,
    members,
    right_bracket)
  {
    super('vector_intrinsic_expression', {
      keyword: keyword,
      left_bracket: left_bracket,
      members: members,
      right_bracket: right_bracket });
  }
  get keyword() { return this.children.keyword; }
  get left_bracket() { return this.children.left_bracket; }
  get members() { return this.children.members; }
  get right_bracket() { return this.children.right_bracket; }
  with_keyword(keyword){
    return new VectorIntrinsicExpression(
      keyword,
      this.left_bracket,
      this.members,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new VectorIntrinsicExpression(
      this.keyword,
      left_bracket,
      this.members,
      this.right_bracket);
  }
  with_members(members){
    return new VectorIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      members,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new VectorIntrinsicExpression(
      this.keyword,
      this.left_bracket,
      this.members,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_bracket === this.left_bracket &&
      members === this.members &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VectorIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.vector_intrinsic_keyword, position, source);
    position += keyword.width;
    let left_bracket = EditableSyntax.from_json(
      json.vector_intrinsic_left_bracket, position, source);
    position += left_bracket.width;
    let members = EditableSyntax.from_json(
      json.vector_intrinsic_members, position, source);
    position += members.width;
    let right_bracket = EditableSyntax.from_json(
      json.vector_intrinsic_right_bracket, position, source);
    position += right_bracket.width;
    return new VectorIntrinsicExpression(
        keyword,
        left_bracket,
        members,
        right_bracket);
  }
  get children_keys()
  {
    if (VectorIntrinsicExpression._children_keys == null)
      VectorIntrinsicExpression._children_keys = [
        'keyword',
        'left_bracket',
        'members',
        'right_bracket'];
    return VectorIntrinsicExpression._children_keys;
  }
}
class ElementInitializer extends EditableSyntax
{
  constructor(
    key,
    arrow,
    value)
  {
    super('element_initializer', {
      key: key,
      arrow: arrow,
      value: value });
  }
  get key() { return this.children.key; }
  get arrow() { return this.children.arrow; }
  get value() { return this.children.value; }
  with_key(key){
    return new ElementInitializer(
      key,
      this.arrow,
      this.value);
  }
  with_arrow(arrow){
    return new ElementInitializer(
      this.key,
      arrow,
      this.value);
  }
  with_value(value){
    return new ElementInitializer(
      this.key,
      this.arrow,
      value);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var key = this.key.rewrite(rewriter, new_parents);
    var arrow = this.arrow.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    if (
      key === this.key &&
      arrow === this.arrow &&
      value === this.value)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ElementInitializer(
        key,
        arrow,
        value), parents);
    }
  }
  static from_json(json, position, source)
  {
    let key = EditableSyntax.from_json(
      json.element_key, position, source);
    position += key.width;
    let arrow = EditableSyntax.from_json(
      json.element_arrow, position, source);
    position += arrow.width;
    let value = EditableSyntax.from_json(
      json.element_value, position, source);
    position += value.width;
    return new ElementInitializer(
        key,
        arrow,
        value);
  }
  get children_keys()
  {
    if (ElementInitializer._children_keys == null)
      ElementInitializer._children_keys = [
        'key',
        'arrow',
        'value'];
    return ElementInitializer._children_keys;
  }
}
class SubscriptExpression extends EditableSyntax
{
  constructor(
    receiver,
    left_bracket,
    index,
    right_bracket)
  {
    super('subscript_expression', {
      receiver: receiver,
      left_bracket: left_bracket,
      index: index,
      right_bracket: right_bracket });
  }
  get receiver() { return this.children.receiver; }
  get left_bracket() { return this.children.left_bracket; }
  get index() { return this.children.index; }
  get right_bracket() { return this.children.right_bracket; }
  with_receiver(receiver){
    return new SubscriptExpression(
      receiver,
      this.left_bracket,
      this.index,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new SubscriptExpression(
      this.receiver,
      left_bracket,
      this.index,
      this.right_bracket);
  }
  with_index(index){
    return new SubscriptExpression(
      this.receiver,
      this.left_bracket,
      index,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new SubscriptExpression(
      this.receiver,
      this.left_bracket,
      this.index,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var receiver = this.receiver.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var index = this.index.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      receiver === this.receiver &&
      left_bracket === this.left_bracket &&
      index === this.index &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SubscriptExpression(
        receiver,
        left_bracket,
        index,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let receiver = EditableSyntax.from_json(
      json.subscript_receiver, position, source);
    position += receiver.width;
    let left_bracket = EditableSyntax.from_json(
      json.subscript_left_bracket, position, source);
    position += left_bracket.width;
    let index = EditableSyntax.from_json(
      json.subscript_index, position, source);
    position += index.width;
    let right_bracket = EditableSyntax.from_json(
      json.subscript_right_bracket, position, source);
    position += right_bracket.width;
    return new SubscriptExpression(
        receiver,
        left_bracket,
        index,
        right_bracket);
  }
  get children_keys()
  {
    if (SubscriptExpression._children_keys == null)
      SubscriptExpression._children_keys = [
        'receiver',
        'left_bracket',
        'index',
        'right_bracket'];
    return SubscriptExpression._children_keys;
  }
}
class EmbeddedSubscriptExpression extends EditableSyntax
{
  constructor(
    receiver,
    left_bracket,
    index,
    right_bracket)
  {
    super('embedded_subscript_expression', {
      receiver: receiver,
      left_bracket: left_bracket,
      index: index,
      right_bracket: right_bracket });
  }
  get receiver() { return this.children.receiver; }
  get left_bracket() { return this.children.left_bracket; }
  get index() { return this.children.index; }
  get right_bracket() { return this.children.right_bracket; }
  with_receiver(receiver){
    return new EmbeddedSubscriptExpression(
      receiver,
      this.left_bracket,
      this.index,
      this.right_bracket);
  }
  with_left_bracket(left_bracket){
    return new EmbeddedSubscriptExpression(
      this.receiver,
      left_bracket,
      this.index,
      this.right_bracket);
  }
  with_index(index){
    return new EmbeddedSubscriptExpression(
      this.receiver,
      this.left_bracket,
      index,
      this.right_bracket);
  }
  with_right_bracket(right_bracket){
    return new EmbeddedSubscriptExpression(
      this.receiver,
      this.left_bracket,
      this.index,
      right_bracket);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var receiver = this.receiver.rewrite(rewriter, new_parents);
    var left_bracket = this.left_bracket.rewrite(rewriter, new_parents);
    var index = this.index.rewrite(rewriter, new_parents);
    var right_bracket = this.right_bracket.rewrite(rewriter, new_parents);
    if (
      receiver === this.receiver &&
      left_bracket === this.left_bracket &&
      index === this.index &&
      right_bracket === this.right_bracket)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new EmbeddedSubscriptExpression(
        receiver,
        left_bracket,
        index,
        right_bracket), parents);
    }
  }
  static from_json(json, position, source)
  {
    let receiver = EditableSyntax.from_json(
      json.embedded_subscript_receiver, position, source);
    position += receiver.width;
    let left_bracket = EditableSyntax.from_json(
      json.embedded_subscript_left_bracket, position, source);
    position += left_bracket.width;
    let index = EditableSyntax.from_json(
      json.embedded_subscript_index, position, source);
    position += index.width;
    let right_bracket = EditableSyntax.from_json(
      json.embedded_subscript_right_bracket, position, source);
    position += right_bracket.width;
    return new EmbeddedSubscriptExpression(
        receiver,
        left_bracket,
        index,
        right_bracket);
  }
  get children_keys()
  {
    if (EmbeddedSubscriptExpression._children_keys == null)
      EmbeddedSubscriptExpression._children_keys = [
        'receiver',
        'left_bracket',
        'index',
        'right_bracket'];
    return EmbeddedSubscriptExpression._children_keys;
  }
}
class AwaitableCreationExpression extends EditableSyntax
{
  constructor(
    async,
    coroutine,
    compound_statement)
  {
    super('awaitable_creation_expression', {
      async: async,
      coroutine: coroutine,
      compound_statement: compound_statement });
  }
  get async() { return this.children.async; }
  get coroutine() { return this.children.coroutine; }
  get compound_statement() { return this.children.compound_statement; }
  with_async(async){
    return new AwaitableCreationExpression(
      async,
      this.coroutine,
      this.compound_statement);
  }
  with_coroutine(coroutine){
    return new AwaitableCreationExpression(
      this.async,
      coroutine,
      this.compound_statement);
  }
  with_compound_statement(compound_statement){
    return new AwaitableCreationExpression(
      this.async,
      this.coroutine,
      compound_statement);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var async = this.async.rewrite(rewriter, new_parents);
    var coroutine = this.coroutine.rewrite(rewriter, new_parents);
    var compound_statement = this.compound_statement.rewrite(rewriter, new_parents);
    if (
      async === this.async &&
      coroutine === this.coroutine &&
      compound_statement === this.compound_statement)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new AwaitableCreationExpression(
        async,
        coroutine,
        compound_statement), parents);
    }
  }
  static from_json(json, position, source)
  {
    let async = EditableSyntax.from_json(
      json.awaitable_async, position, source);
    position += async.width;
    let coroutine = EditableSyntax.from_json(
      json.awaitable_coroutine, position, source);
    position += coroutine.width;
    let compound_statement = EditableSyntax.from_json(
      json.awaitable_compound_statement, position, source);
    position += compound_statement.width;
    return new AwaitableCreationExpression(
        async,
        coroutine,
        compound_statement);
  }
  get children_keys()
  {
    if (AwaitableCreationExpression._children_keys == null)
      AwaitableCreationExpression._children_keys = [
        'async',
        'coroutine',
        'compound_statement'];
    return AwaitableCreationExpression._children_keys;
  }
}
class XHPChildrenDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    expression,
    semicolon)
  {
    super('xhp_children_declaration', {
      keyword: keyword,
      expression: expression,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get expression() { return this.children.expression; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new XHPChildrenDeclaration(
      keyword,
      this.expression,
      this.semicolon);
  }
  with_expression(expression){
    return new XHPChildrenDeclaration(
      this.keyword,
      expression,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new XHPChildrenDeclaration(
      this.keyword,
      this.expression,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      expression === this.expression &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPChildrenDeclaration(
        keyword,
        expression,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.xhp_children_keyword, position, source);
    position += keyword.width;
    let expression = EditableSyntax.from_json(
      json.xhp_children_expression, position, source);
    position += expression.width;
    let semicolon = EditableSyntax.from_json(
      json.xhp_children_semicolon, position, source);
    position += semicolon.width;
    return new XHPChildrenDeclaration(
        keyword,
        expression,
        semicolon);
  }
  get children_keys()
  {
    if (XHPChildrenDeclaration._children_keys == null)
      XHPChildrenDeclaration._children_keys = [
        'keyword',
        'expression',
        'semicolon'];
    return XHPChildrenDeclaration._children_keys;
  }
}
class XHPChildrenParenthesizedList extends EditableSyntax
{
  constructor(
    left_paren,
    xhp_children,
    right_paren)
  {
    super('xhp_children_parenthesized_list', {
      left_paren: left_paren,
      xhp_children: xhp_children,
      right_paren: right_paren });
  }
  get left_paren() { return this.children.left_paren; }
  get xhp_children() { return this.children.xhp_children; }
  get right_paren() { return this.children.right_paren; }
  with_left_paren(left_paren){
    return new XHPChildrenParenthesizedList(
      left_paren,
      this.xhp_children,
      this.right_paren);
  }
  with_xhp_children(xhp_children){
    return new XHPChildrenParenthesizedList(
      this.left_paren,
      xhp_children,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new XHPChildrenParenthesizedList(
      this.left_paren,
      this.xhp_children,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var xhp_children = this.xhp_children.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      left_paren === this.left_paren &&
      xhp_children === this.xhp_children &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPChildrenParenthesizedList(
        left_paren,
        xhp_children,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_paren = EditableSyntax.from_json(
      json.xhp_children_list_left_paren, position, source);
    position += left_paren.width;
    let xhp_children = EditableSyntax.from_json(
      json.xhp_children_list_xhp_children, position, source);
    position += xhp_children.width;
    let right_paren = EditableSyntax.from_json(
      json.xhp_children_list_right_paren, position, source);
    position += right_paren.width;
    return new XHPChildrenParenthesizedList(
        left_paren,
        xhp_children,
        right_paren);
  }
  get children_keys()
  {
    if (XHPChildrenParenthesizedList._children_keys == null)
      XHPChildrenParenthesizedList._children_keys = [
        'left_paren',
        'xhp_children',
        'right_paren'];
    return XHPChildrenParenthesizedList._children_keys;
  }
}
class XHPCategoryDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    categories,
    semicolon)
  {
    super('xhp_category_declaration', {
      keyword: keyword,
      categories: categories,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get categories() { return this.children.categories; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new XHPCategoryDeclaration(
      keyword,
      this.categories,
      this.semicolon);
  }
  with_categories(categories){
    return new XHPCategoryDeclaration(
      this.keyword,
      categories,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new XHPCategoryDeclaration(
      this.keyword,
      this.categories,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var categories = this.categories.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      categories === this.categories &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPCategoryDeclaration(
        keyword,
        categories,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.xhp_category_keyword, position, source);
    position += keyword.width;
    let categories = EditableSyntax.from_json(
      json.xhp_category_categories, position, source);
    position += categories.width;
    let semicolon = EditableSyntax.from_json(
      json.xhp_category_semicolon, position, source);
    position += semicolon.width;
    return new XHPCategoryDeclaration(
        keyword,
        categories,
        semicolon);
  }
  get children_keys()
  {
    if (XHPCategoryDeclaration._children_keys == null)
      XHPCategoryDeclaration._children_keys = [
        'keyword',
        'categories',
        'semicolon'];
    return XHPCategoryDeclaration._children_keys;
  }
}
class XHPEnumType extends EditableSyntax
{
  constructor(
    keyword,
    left_brace,
    values,
    right_brace)
  {
    super('xhp_enum_type', {
      keyword: keyword,
      left_brace: left_brace,
      values: values,
      right_brace: right_brace });
  }
  get keyword() { return this.children.keyword; }
  get left_brace() { return this.children.left_brace; }
  get values() { return this.children.values; }
  get right_brace() { return this.children.right_brace; }
  with_keyword(keyword){
    return new XHPEnumType(
      keyword,
      this.left_brace,
      this.values,
      this.right_brace);
  }
  with_left_brace(left_brace){
    return new XHPEnumType(
      this.keyword,
      left_brace,
      this.values,
      this.right_brace);
  }
  with_values(values){
    return new XHPEnumType(
      this.keyword,
      this.left_brace,
      values,
      this.right_brace);
  }
  with_right_brace(right_brace){
    return new XHPEnumType(
      this.keyword,
      this.left_brace,
      this.values,
      right_brace);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_brace = this.left_brace.rewrite(rewriter, new_parents);
    var values = this.values.rewrite(rewriter, new_parents);
    var right_brace = this.right_brace.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_brace === this.left_brace &&
      values === this.values &&
      right_brace === this.right_brace)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPEnumType(
        keyword,
        left_brace,
        values,
        right_brace), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.xhp_enum_keyword, position, source);
    position += keyword.width;
    let left_brace = EditableSyntax.from_json(
      json.xhp_enum_left_brace, position, source);
    position += left_brace.width;
    let values = EditableSyntax.from_json(
      json.xhp_enum_values, position, source);
    position += values.width;
    let right_brace = EditableSyntax.from_json(
      json.xhp_enum_right_brace, position, source);
    position += right_brace.width;
    return new XHPEnumType(
        keyword,
        left_brace,
        values,
        right_brace);
  }
  get children_keys()
  {
    if (XHPEnumType._children_keys == null)
      XHPEnumType._children_keys = [
        'keyword',
        'left_brace',
        'values',
        'right_brace'];
    return XHPEnumType._children_keys;
  }
}
class XHPRequired extends EditableSyntax
{
  constructor(
    at,
    keyword)
  {
    super('xhp_required', {
      at: at,
      keyword: keyword });
  }
  get at() { return this.children.at; }
  get keyword() { return this.children.keyword; }
  with_at(at){
    return new XHPRequired(
      at,
      this.keyword);
  }
  with_keyword(keyword){
    return new XHPRequired(
      this.at,
      keyword);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var at = this.at.rewrite(rewriter, new_parents);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    if (
      at === this.at &&
      keyword === this.keyword)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPRequired(
        at,
        keyword), parents);
    }
  }
  static from_json(json, position, source)
  {
    let at = EditableSyntax.from_json(
      json.xhp_required_at, position, source);
    position += at.width;
    let keyword = EditableSyntax.from_json(
      json.xhp_required_keyword, position, source);
    position += keyword.width;
    return new XHPRequired(
        at,
        keyword);
  }
  get children_keys()
  {
    if (XHPRequired._children_keys == null)
      XHPRequired._children_keys = [
        'at',
        'keyword'];
    return XHPRequired._children_keys;
  }
}
class XHPClassAttributeDeclaration extends EditableSyntax
{
  constructor(
    keyword,
    attributes,
    semicolon)
  {
    super('xhp_class_attribute_declaration', {
      keyword: keyword,
      attributes: attributes,
      semicolon: semicolon });
  }
  get keyword() { return this.children.keyword; }
  get attributes() { return this.children.attributes; }
  get semicolon() { return this.children.semicolon; }
  with_keyword(keyword){
    return new XHPClassAttributeDeclaration(
      keyword,
      this.attributes,
      this.semicolon);
  }
  with_attributes(attributes){
    return new XHPClassAttributeDeclaration(
      this.keyword,
      attributes,
      this.semicolon);
  }
  with_semicolon(semicolon){
    return new XHPClassAttributeDeclaration(
      this.keyword,
      this.attributes,
      semicolon);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var attributes = this.attributes.rewrite(rewriter, new_parents);
    var semicolon = this.semicolon.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      attributes === this.attributes &&
      semicolon === this.semicolon)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPClassAttributeDeclaration(
        keyword,
        attributes,
        semicolon), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.xhp_attribute_keyword, position, source);
    position += keyword.width;
    let attributes = EditableSyntax.from_json(
      json.xhp_attribute_attributes, position, source);
    position += attributes.width;
    let semicolon = EditableSyntax.from_json(
      json.xhp_attribute_semicolon, position, source);
    position += semicolon.width;
    return new XHPClassAttributeDeclaration(
        keyword,
        attributes,
        semicolon);
  }
  get children_keys()
  {
    if (XHPClassAttributeDeclaration._children_keys == null)
      XHPClassAttributeDeclaration._children_keys = [
        'keyword',
        'attributes',
        'semicolon'];
    return XHPClassAttributeDeclaration._children_keys;
  }
}
class XHPClassAttribute extends EditableSyntax
{
  constructor(
    type,
    name,
    initializer,
    required)
  {
    super('xhp_class_attribute', {
      type: type,
      name: name,
      initializer: initializer,
      required: required });
  }
  get type() { return this.children.type; }
  get name() { return this.children.name; }
  get initializer() { return this.children.initializer; }
  get required() { return this.children.required; }
  with_type(type){
    return new XHPClassAttribute(
      type,
      this.name,
      this.initializer,
      this.required);
  }
  with_name(name){
    return new XHPClassAttribute(
      this.type,
      name,
      this.initializer,
      this.required);
  }
  with_initializer(initializer){
    return new XHPClassAttribute(
      this.type,
      this.name,
      initializer,
      this.required);
  }
  with_required(required){
    return new XHPClassAttribute(
      this.type,
      this.name,
      this.initializer,
      required);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var type = this.type.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var initializer = this.initializer.rewrite(rewriter, new_parents);
    var required = this.required.rewrite(rewriter, new_parents);
    if (
      type === this.type &&
      name === this.name &&
      initializer === this.initializer &&
      required === this.required)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPClassAttribute(
        type,
        name,
        initializer,
        required), parents);
    }
  }
  static from_json(json, position, source)
  {
    let type = EditableSyntax.from_json(
      json.xhp_attribute_decl_type, position, source);
    position += type.width;
    let name = EditableSyntax.from_json(
      json.xhp_attribute_decl_name, position, source);
    position += name.width;
    let initializer = EditableSyntax.from_json(
      json.xhp_attribute_decl_initializer, position, source);
    position += initializer.width;
    let required = EditableSyntax.from_json(
      json.xhp_attribute_decl_required, position, source);
    position += required.width;
    return new XHPClassAttribute(
        type,
        name,
        initializer,
        required);
  }
  get children_keys()
  {
    if (XHPClassAttribute._children_keys == null)
      XHPClassAttribute._children_keys = [
        'type',
        'name',
        'initializer',
        'required'];
    return XHPClassAttribute._children_keys;
  }
}
class XHPSimpleClassAttribute extends EditableSyntax
{
  constructor(
    type)
  {
    super('xhp_simple_class_attribute', {
      type: type });
  }
  get type() { return this.children.type; }
  with_type(type){
    return new XHPSimpleClassAttribute(
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPSimpleClassAttribute(
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let type = EditableSyntax.from_json(
      json.xhp_simple_class_attribute_type, position, source);
    position += type.width;
    return new XHPSimpleClassAttribute(
        type);
  }
  get children_keys()
  {
    if (XHPSimpleClassAttribute._children_keys == null)
      XHPSimpleClassAttribute._children_keys = [
        'type'];
    return XHPSimpleClassAttribute._children_keys;
  }
}
class XHPAttribute extends EditableSyntax
{
  constructor(
    name,
    equal,
    expression)
  {
    super('xhp_attribute', {
      name: name,
      equal: equal,
      expression: expression });
  }
  get name() { return this.children.name; }
  get equal() { return this.children.equal; }
  get expression() { return this.children.expression; }
  with_name(name){
    return new XHPAttribute(
      name,
      this.equal,
      this.expression);
  }
  with_equal(equal){
    return new XHPAttribute(
      this.name,
      equal,
      this.expression);
  }
  with_expression(expression){
    return new XHPAttribute(
      this.name,
      this.equal,
      expression);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var equal = this.equal.rewrite(rewriter, new_parents);
    var expression = this.expression.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      equal === this.equal &&
      expression === this.expression)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPAttribute(
        name,
        equal,
        expression), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.xhp_attribute_name, position, source);
    position += name.width;
    let equal = EditableSyntax.from_json(
      json.xhp_attribute_equal, position, source);
    position += equal.width;
    let expression = EditableSyntax.from_json(
      json.xhp_attribute_expression, position, source);
    position += expression.width;
    return new XHPAttribute(
        name,
        equal,
        expression);
  }
  get children_keys()
  {
    if (XHPAttribute._children_keys == null)
      XHPAttribute._children_keys = [
        'name',
        'equal',
        'expression'];
    return XHPAttribute._children_keys;
  }
}
class XHPOpen extends EditableSyntax
{
  constructor(
    left_angle,
    name,
    attributes,
    right_angle)
  {
    super('xhp_open', {
      left_angle: left_angle,
      name: name,
      attributes: attributes,
      right_angle: right_angle });
  }
  get left_angle() { return this.children.left_angle; }
  get name() { return this.children.name; }
  get attributes() { return this.children.attributes; }
  get right_angle() { return this.children.right_angle; }
  with_left_angle(left_angle){
    return new XHPOpen(
      left_angle,
      this.name,
      this.attributes,
      this.right_angle);
  }
  with_name(name){
    return new XHPOpen(
      this.left_angle,
      name,
      this.attributes,
      this.right_angle);
  }
  with_attributes(attributes){
    return new XHPOpen(
      this.left_angle,
      this.name,
      attributes,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new XHPOpen(
      this.left_angle,
      this.name,
      this.attributes,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var attributes = this.attributes.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      left_angle === this.left_angle &&
      name === this.name &&
      attributes === this.attributes &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPOpen(
        left_angle,
        name,
        attributes,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_angle = EditableSyntax.from_json(
      json.xhp_open_left_angle, position, source);
    position += left_angle.width;
    let name = EditableSyntax.from_json(
      json.xhp_open_name, position, source);
    position += name.width;
    let attributes = EditableSyntax.from_json(
      json.xhp_open_attributes, position, source);
    position += attributes.width;
    let right_angle = EditableSyntax.from_json(
      json.xhp_open_right_angle, position, source);
    position += right_angle.width;
    return new XHPOpen(
        left_angle,
        name,
        attributes,
        right_angle);
  }
  get children_keys()
  {
    if (XHPOpen._children_keys == null)
      XHPOpen._children_keys = [
        'left_angle',
        'name',
        'attributes',
        'right_angle'];
    return XHPOpen._children_keys;
  }
}
class XHPExpression extends EditableSyntax
{
  constructor(
    open,
    body,
    close)
  {
    super('xhp_expression', {
      open: open,
      body: body,
      close: close });
  }
  get open() { return this.children.open; }
  get body() { return this.children.body; }
  get close() { return this.children.close; }
  with_open(open){
    return new XHPExpression(
      open,
      this.body,
      this.close);
  }
  with_body(body){
    return new XHPExpression(
      this.open,
      body,
      this.close);
  }
  with_close(close){
    return new XHPExpression(
      this.open,
      this.body,
      close);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var open = this.open.rewrite(rewriter, new_parents);
    var body = this.body.rewrite(rewriter, new_parents);
    var close = this.close.rewrite(rewriter, new_parents);
    if (
      open === this.open &&
      body === this.body &&
      close === this.close)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPExpression(
        open,
        body,
        close), parents);
    }
  }
  static from_json(json, position, source)
  {
    let open = EditableSyntax.from_json(
      json.xhp_open, position, source);
    position += open.width;
    let body = EditableSyntax.from_json(
      json.xhp_body, position, source);
    position += body.width;
    let close = EditableSyntax.from_json(
      json.xhp_close, position, source);
    position += close.width;
    return new XHPExpression(
        open,
        body,
        close);
  }
  get children_keys()
  {
    if (XHPExpression._children_keys == null)
      XHPExpression._children_keys = [
        'open',
        'body',
        'close'];
    return XHPExpression._children_keys;
  }
}
class XHPClose extends EditableSyntax
{
  constructor(
    left_angle,
    name,
    right_angle)
  {
    super('xhp_close', {
      left_angle: left_angle,
      name: name,
      right_angle: right_angle });
  }
  get left_angle() { return this.children.left_angle; }
  get name() { return this.children.name; }
  get right_angle() { return this.children.right_angle; }
  with_left_angle(left_angle){
    return new XHPClose(
      left_angle,
      this.name,
      this.right_angle);
  }
  with_name(name){
    return new XHPClose(
      this.left_angle,
      name,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new XHPClose(
      this.left_angle,
      this.name,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      left_angle === this.left_angle &&
      name === this.name &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new XHPClose(
        left_angle,
        name,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_angle = EditableSyntax.from_json(
      json.xhp_close_left_angle, position, source);
    position += left_angle.width;
    let name = EditableSyntax.from_json(
      json.xhp_close_name, position, source);
    position += name.width;
    let right_angle = EditableSyntax.from_json(
      json.xhp_close_right_angle, position, source);
    position += right_angle.width;
    return new XHPClose(
        left_angle,
        name,
        right_angle);
  }
  get children_keys()
  {
    if (XHPClose._children_keys == null)
      XHPClose._children_keys = [
        'left_angle',
        'name',
        'right_angle'];
    return XHPClose._children_keys;
  }
}
class TypeConstant extends EditableSyntax
{
  constructor(
    left_type,
    separator,
    right_type)
  {
    super('type_constant', {
      left_type: left_type,
      separator: separator,
      right_type: right_type });
  }
  get left_type() { return this.children.left_type; }
  get separator() { return this.children.separator; }
  get right_type() { return this.children.right_type; }
  with_left_type(left_type){
    return new TypeConstant(
      left_type,
      this.separator,
      this.right_type);
  }
  with_separator(separator){
    return new TypeConstant(
      this.left_type,
      separator,
      this.right_type);
  }
  with_right_type(right_type){
    return new TypeConstant(
      this.left_type,
      this.separator,
      right_type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_type = this.left_type.rewrite(rewriter, new_parents);
    var separator = this.separator.rewrite(rewriter, new_parents);
    var right_type = this.right_type.rewrite(rewriter, new_parents);
    if (
      left_type === this.left_type &&
      separator === this.separator &&
      right_type === this.right_type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeConstant(
        left_type,
        separator,
        right_type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_type = EditableSyntax.from_json(
      json.type_constant_left_type, position, source);
    position += left_type.width;
    let separator = EditableSyntax.from_json(
      json.type_constant_separator, position, source);
    position += separator.width;
    let right_type = EditableSyntax.from_json(
      json.type_constant_right_type, position, source);
    position += right_type.width;
    return new TypeConstant(
        left_type,
        separator,
        right_type);
  }
  get children_keys()
  {
    if (TypeConstant._children_keys == null)
      TypeConstant._children_keys = [
        'left_type',
        'separator',
        'right_type'];
    return TypeConstant._children_keys;
  }
}
class VectorTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    type,
    trailing_comma,
    right_angle)
  {
    super('vector_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      type: type,
      trailing_comma: trailing_comma,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get type() { return this.children.type; }
  get trailing_comma() { return this.children.trailing_comma; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new VectorTypeSpecifier(
      keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new VectorTypeSpecifier(
      this.keyword,
      left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_type(type){
    return new VectorTypeSpecifier(
      this.keyword,
      this.left_angle,
      type,
      this.trailing_comma,
      this.right_angle);
  }
  with_trailing_comma(trailing_comma){
    return new VectorTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      trailing_comma,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new VectorTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var trailing_comma = this.trailing_comma.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      type === this.type &&
      trailing_comma === this.trailing_comma &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VectorTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.vector_type_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.vector_type_left_angle, position, source);
    position += left_angle.width;
    let type = EditableSyntax.from_json(
      json.vector_type_type, position, source);
    position += type.width;
    let trailing_comma = EditableSyntax.from_json(
      json.vector_type_trailing_comma, position, source);
    position += trailing_comma.width;
    let right_angle = EditableSyntax.from_json(
      json.vector_type_right_angle, position, source);
    position += right_angle.width;
    return new VectorTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle);
  }
  get children_keys()
  {
    if (VectorTypeSpecifier._children_keys == null)
      VectorTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'type',
        'trailing_comma',
        'right_angle'];
    return VectorTypeSpecifier._children_keys;
  }
}
class KeysetTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    type,
    trailing_comma,
    right_angle)
  {
    super('keyset_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      type: type,
      trailing_comma: trailing_comma,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get type() { return this.children.type; }
  get trailing_comma() { return this.children.trailing_comma; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new KeysetTypeSpecifier(
      keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new KeysetTypeSpecifier(
      this.keyword,
      left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_type(type){
    return new KeysetTypeSpecifier(
      this.keyword,
      this.left_angle,
      type,
      this.trailing_comma,
      this.right_angle);
  }
  with_trailing_comma(trailing_comma){
    return new KeysetTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      trailing_comma,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new KeysetTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var trailing_comma = this.trailing_comma.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      type === this.type &&
      trailing_comma === this.trailing_comma &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new KeysetTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.keyset_type_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.keyset_type_left_angle, position, source);
    position += left_angle.width;
    let type = EditableSyntax.from_json(
      json.keyset_type_type, position, source);
    position += type.width;
    let trailing_comma = EditableSyntax.from_json(
      json.keyset_type_trailing_comma, position, source);
    position += trailing_comma.width;
    let right_angle = EditableSyntax.from_json(
      json.keyset_type_right_angle, position, source);
    position += right_angle.width;
    return new KeysetTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle);
  }
  get children_keys()
  {
    if (KeysetTypeSpecifier._children_keys == null)
      KeysetTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'type',
        'trailing_comma',
        'right_angle'];
    return KeysetTypeSpecifier._children_keys;
  }
}
class TupleTypeExplicitSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    types,
    right_angle)
  {
    super('tuple_type_explicit_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      types: types,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get types() { return this.children.types; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new TupleTypeExplicitSpecifier(
      keyword,
      this.left_angle,
      this.types,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new TupleTypeExplicitSpecifier(
      this.keyword,
      left_angle,
      this.types,
      this.right_angle);
  }
  with_types(types){
    return new TupleTypeExplicitSpecifier(
      this.keyword,
      this.left_angle,
      types,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new TupleTypeExplicitSpecifier(
      this.keyword,
      this.left_angle,
      this.types,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var types = this.types.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      types === this.types &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TupleTypeExplicitSpecifier(
        keyword,
        left_angle,
        types,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.tuple_type_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.tuple_type_left_angle, position, source);
    position += left_angle.width;
    let types = EditableSyntax.from_json(
      json.tuple_type_types, position, source);
    position += types.width;
    let right_angle = EditableSyntax.from_json(
      json.tuple_type_right_angle, position, source);
    position += right_angle.width;
    return new TupleTypeExplicitSpecifier(
        keyword,
        left_angle,
        types,
        right_angle);
  }
  get children_keys()
  {
    if (TupleTypeExplicitSpecifier._children_keys == null)
      TupleTypeExplicitSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'types',
        'right_angle'];
    return TupleTypeExplicitSpecifier._children_keys;
  }
}
class VarrayTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    type,
    trailing_comma,
    right_angle)
  {
    super('varray_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      type: type,
      trailing_comma: trailing_comma,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get type() { return this.children.type; }
  get trailing_comma() { return this.children.trailing_comma; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new VarrayTypeSpecifier(
      keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new VarrayTypeSpecifier(
      this.keyword,
      left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_type(type){
    return new VarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      type,
      this.trailing_comma,
      this.right_angle);
  }
  with_trailing_comma(trailing_comma){
    return new VarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      trailing_comma,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new VarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var trailing_comma = this.trailing_comma.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      type === this.type &&
      trailing_comma === this.trailing_comma &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VarrayTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.varray_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.varray_left_angle, position, source);
    position += left_angle.width;
    let type = EditableSyntax.from_json(
      json.varray_type, position, source);
    position += type.width;
    let trailing_comma = EditableSyntax.from_json(
      json.varray_trailing_comma, position, source);
    position += trailing_comma.width;
    let right_angle = EditableSyntax.from_json(
      json.varray_right_angle, position, source);
    position += right_angle.width;
    return new VarrayTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle);
  }
  get children_keys()
  {
    if (VarrayTypeSpecifier._children_keys == null)
      VarrayTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'type',
        'trailing_comma',
        'right_angle'];
    return VarrayTypeSpecifier._children_keys;
  }
}
class VectorArrayTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    type,
    right_angle)
  {
    super('vector_array_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      type: type,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get type() { return this.children.type; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new VectorArrayTypeSpecifier(
      keyword,
      this.left_angle,
      this.type,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new VectorArrayTypeSpecifier(
      this.keyword,
      left_angle,
      this.type,
      this.right_angle);
  }
  with_type(type){
    return new VectorArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      type,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new VectorArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      type === this.type &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new VectorArrayTypeSpecifier(
        keyword,
        left_angle,
        type,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.vector_array_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.vector_array_left_angle, position, source);
    position += left_angle.width;
    let type = EditableSyntax.from_json(
      json.vector_array_type, position, source);
    position += type.width;
    let right_angle = EditableSyntax.from_json(
      json.vector_array_right_angle, position, source);
    position += right_angle.width;
    return new VectorArrayTypeSpecifier(
        keyword,
        left_angle,
        type,
        right_angle);
  }
  get children_keys()
  {
    if (VectorArrayTypeSpecifier._children_keys == null)
      VectorArrayTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'type',
        'right_angle'];
    return VectorArrayTypeSpecifier._children_keys;
  }
}
class TypeParameter extends EditableSyntax
{
  constructor(
    variance,
    name,
    constraints)
  {
    super('type_parameter', {
      variance: variance,
      name: name,
      constraints: constraints });
  }
  get variance() { return this.children.variance; }
  get name() { return this.children.name; }
  get constraints() { return this.children.constraints; }
  with_variance(variance){
    return new TypeParameter(
      variance,
      this.name,
      this.constraints);
  }
  with_name(name){
    return new TypeParameter(
      this.variance,
      name,
      this.constraints);
  }
  with_constraints(constraints){
    return new TypeParameter(
      this.variance,
      this.name,
      constraints);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var variance = this.variance.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var constraints = this.constraints.rewrite(rewriter, new_parents);
    if (
      variance === this.variance &&
      name === this.name &&
      constraints === this.constraints)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeParameter(
        variance,
        name,
        constraints), parents);
    }
  }
  static from_json(json, position, source)
  {
    let variance = EditableSyntax.from_json(
      json.type_variance, position, source);
    position += variance.width;
    let name = EditableSyntax.from_json(
      json.type_name, position, source);
    position += name.width;
    let constraints = EditableSyntax.from_json(
      json.type_constraints, position, source);
    position += constraints.width;
    return new TypeParameter(
        variance,
        name,
        constraints);
  }
  get children_keys()
  {
    if (TypeParameter._children_keys == null)
      TypeParameter._children_keys = [
        'variance',
        'name',
        'constraints'];
    return TypeParameter._children_keys;
  }
}
class TypeConstraint extends EditableSyntax
{
  constructor(
    keyword,
    type)
  {
    super('type_constraint', {
      keyword: keyword,
      type: type });
  }
  get keyword() { return this.children.keyword; }
  get type() { return this.children.type; }
  with_keyword(keyword){
    return new TypeConstraint(
      keyword,
      this.type);
  }
  with_type(type){
    return new TypeConstraint(
      this.keyword,
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeConstraint(
        keyword,
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.constraint_keyword, position, source);
    position += keyword.width;
    let type = EditableSyntax.from_json(
      json.constraint_type, position, source);
    position += type.width;
    return new TypeConstraint(
        keyword,
        type);
  }
  get children_keys()
  {
    if (TypeConstraint._children_keys == null)
      TypeConstraint._children_keys = [
        'keyword',
        'type'];
    return TypeConstraint._children_keys;
  }
}
class DarrayTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    key,
    comma,
    value,
    trailing_comma,
    right_angle)
  {
    super('darray_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      key: key,
      comma: comma,
      value: value,
      trailing_comma: trailing_comma,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get key() { return this.children.key; }
  get comma() { return this.children.comma; }
  get value() { return this.children.value; }
  get trailing_comma() { return this.children.trailing_comma; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new DarrayTypeSpecifier(
      keyword,
      this.left_angle,
      this.key,
      this.comma,
      this.value,
      this.trailing_comma,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new DarrayTypeSpecifier(
      this.keyword,
      left_angle,
      this.key,
      this.comma,
      this.value,
      this.trailing_comma,
      this.right_angle);
  }
  with_key(key){
    return new DarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      key,
      this.comma,
      this.value,
      this.trailing_comma,
      this.right_angle);
  }
  with_comma(comma){
    return new DarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      comma,
      this.value,
      this.trailing_comma,
      this.right_angle);
  }
  with_value(value){
    return new DarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      this.comma,
      value,
      this.trailing_comma,
      this.right_angle);
  }
  with_trailing_comma(trailing_comma){
    return new DarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      this.comma,
      this.value,
      trailing_comma,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new DarrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      this.comma,
      this.value,
      this.trailing_comma,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var key = this.key.rewrite(rewriter, new_parents);
    var comma = this.comma.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    var trailing_comma = this.trailing_comma.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      key === this.key &&
      comma === this.comma &&
      value === this.value &&
      trailing_comma === this.trailing_comma &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DarrayTypeSpecifier(
        keyword,
        left_angle,
        key,
        comma,
        value,
        trailing_comma,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.darray_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.darray_left_angle, position, source);
    position += left_angle.width;
    let key = EditableSyntax.from_json(
      json.darray_key, position, source);
    position += key.width;
    let comma = EditableSyntax.from_json(
      json.darray_comma, position, source);
    position += comma.width;
    let value = EditableSyntax.from_json(
      json.darray_value, position, source);
    position += value.width;
    let trailing_comma = EditableSyntax.from_json(
      json.darray_trailing_comma, position, source);
    position += trailing_comma.width;
    let right_angle = EditableSyntax.from_json(
      json.darray_right_angle, position, source);
    position += right_angle.width;
    return new DarrayTypeSpecifier(
        keyword,
        left_angle,
        key,
        comma,
        value,
        trailing_comma,
        right_angle);
  }
  get children_keys()
  {
    if (DarrayTypeSpecifier._children_keys == null)
      DarrayTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'key',
        'comma',
        'value',
        'trailing_comma',
        'right_angle'];
    return DarrayTypeSpecifier._children_keys;
  }
}
class MapArrayTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    key,
    comma,
    value,
    right_angle)
  {
    super('map_array_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      key: key,
      comma: comma,
      value: value,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get key() { return this.children.key; }
  get comma() { return this.children.comma; }
  get value() { return this.children.value; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new MapArrayTypeSpecifier(
      keyword,
      this.left_angle,
      this.key,
      this.comma,
      this.value,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new MapArrayTypeSpecifier(
      this.keyword,
      left_angle,
      this.key,
      this.comma,
      this.value,
      this.right_angle);
  }
  with_key(key){
    return new MapArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      key,
      this.comma,
      this.value,
      this.right_angle);
  }
  with_comma(comma){
    return new MapArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      comma,
      this.value,
      this.right_angle);
  }
  with_value(value){
    return new MapArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      this.comma,
      value,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new MapArrayTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.key,
      this.comma,
      this.value,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var key = this.key.rewrite(rewriter, new_parents);
    var comma = this.comma.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      key === this.key &&
      comma === this.comma &&
      value === this.value &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new MapArrayTypeSpecifier(
        keyword,
        left_angle,
        key,
        comma,
        value,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.map_array_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.map_array_left_angle, position, source);
    position += left_angle.width;
    let key = EditableSyntax.from_json(
      json.map_array_key, position, source);
    position += key.width;
    let comma = EditableSyntax.from_json(
      json.map_array_comma, position, source);
    position += comma.width;
    let value = EditableSyntax.from_json(
      json.map_array_value, position, source);
    position += value.width;
    let right_angle = EditableSyntax.from_json(
      json.map_array_right_angle, position, source);
    position += right_angle.width;
    return new MapArrayTypeSpecifier(
        keyword,
        left_angle,
        key,
        comma,
        value,
        right_angle);
  }
  get children_keys()
  {
    if (MapArrayTypeSpecifier._children_keys == null)
      MapArrayTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'key',
        'comma',
        'value',
        'right_angle'];
    return MapArrayTypeSpecifier._children_keys;
  }
}
class DictionaryTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    members,
    right_angle)
  {
    super('dictionary_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      members: members,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get members() { return this.children.members; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new DictionaryTypeSpecifier(
      keyword,
      this.left_angle,
      this.members,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new DictionaryTypeSpecifier(
      this.keyword,
      left_angle,
      this.members,
      this.right_angle);
  }
  with_members(members){
    return new DictionaryTypeSpecifier(
      this.keyword,
      this.left_angle,
      members,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new DictionaryTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.members,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var members = this.members.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      members === this.members &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new DictionaryTypeSpecifier(
        keyword,
        left_angle,
        members,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.dictionary_type_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.dictionary_type_left_angle, position, source);
    position += left_angle.width;
    let members = EditableSyntax.from_json(
      json.dictionary_type_members, position, source);
    position += members.width;
    let right_angle = EditableSyntax.from_json(
      json.dictionary_type_right_angle, position, source);
    position += right_angle.width;
    return new DictionaryTypeSpecifier(
        keyword,
        left_angle,
        members,
        right_angle);
  }
  get children_keys()
  {
    if (DictionaryTypeSpecifier._children_keys == null)
      DictionaryTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'members',
        'right_angle'];
    return DictionaryTypeSpecifier._children_keys;
  }
}
class ClosureTypeSpecifier extends EditableSyntax
{
  constructor(
    outer_left_paren,
    coroutine,
    function_keyword,
    inner_left_paren,
    parameter_types,
    inner_right_paren,
    colon,
    return_type,
    outer_right_paren)
  {
    super('closure_type_specifier', {
      outer_left_paren: outer_left_paren,
      coroutine: coroutine,
      function_keyword: function_keyword,
      inner_left_paren: inner_left_paren,
      parameter_types: parameter_types,
      inner_right_paren: inner_right_paren,
      colon: colon,
      return_type: return_type,
      outer_right_paren: outer_right_paren });
  }
  get outer_left_paren() { return this.children.outer_left_paren; }
  get coroutine() { return this.children.coroutine; }
  get function_keyword() { return this.children.function_keyword; }
  get inner_left_paren() { return this.children.inner_left_paren; }
  get parameter_types() { return this.children.parameter_types; }
  get inner_right_paren() { return this.children.inner_right_paren; }
  get colon() { return this.children.colon; }
  get return_type() { return this.children.return_type; }
  get outer_right_paren() { return this.children.outer_right_paren; }
  with_outer_left_paren(outer_left_paren){
    return new ClosureTypeSpecifier(
      outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_coroutine(coroutine){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_function_keyword(function_keyword){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_inner_left_paren(inner_left_paren){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_parameter_types(parameter_types){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_inner_right_paren(inner_right_paren){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      inner_right_paren,
      this.colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_colon(colon){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      colon,
      this.return_type,
      this.outer_right_paren);
  }
  with_return_type(return_type){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      return_type,
      this.outer_right_paren);
  }
  with_outer_right_paren(outer_right_paren){
    return new ClosureTypeSpecifier(
      this.outer_left_paren,
      this.coroutine,
      this.function_keyword,
      this.inner_left_paren,
      this.parameter_types,
      this.inner_right_paren,
      this.colon,
      this.return_type,
      outer_right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var outer_left_paren = this.outer_left_paren.rewrite(rewriter, new_parents);
    var coroutine = this.coroutine.rewrite(rewriter, new_parents);
    var function_keyword = this.function_keyword.rewrite(rewriter, new_parents);
    var inner_left_paren = this.inner_left_paren.rewrite(rewriter, new_parents);
    var parameter_types = this.parameter_types.rewrite(rewriter, new_parents);
    var inner_right_paren = this.inner_right_paren.rewrite(rewriter, new_parents);
    var colon = this.colon.rewrite(rewriter, new_parents);
    var return_type = this.return_type.rewrite(rewriter, new_parents);
    var outer_right_paren = this.outer_right_paren.rewrite(rewriter, new_parents);
    if (
      outer_left_paren === this.outer_left_paren &&
      coroutine === this.coroutine &&
      function_keyword === this.function_keyword &&
      inner_left_paren === this.inner_left_paren &&
      parameter_types === this.parameter_types &&
      inner_right_paren === this.inner_right_paren &&
      colon === this.colon &&
      return_type === this.return_type &&
      outer_right_paren === this.outer_right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ClosureTypeSpecifier(
        outer_left_paren,
        coroutine,
        function_keyword,
        inner_left_paren,
        parameter_types,
        inner_right_paren,
        colon,
        return_type,
        outer_right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let outer_left_paren = EditableSyntax.from_json(
      json.closure_outer_left_paren, position, source);
    position += outer_left_paren.width;
    let coroutine = EditableSyntax.from_json(
      json.closure_coroutine, position, source);
    position += coroutine.width;
    let function_keyword = EditableSyntax.from_json(
      json.closure_function_keyword, position, source);
    position += function_keyword.width;
    let inner_left_paren = EditableSyntax.from_json(
      json.closure_inner_left_paren, position, source);
    position += inner_left_paren.width;
    let parameter_types = EditableSyntax.from_json(
      json.closure_parameter_types, position, source);
    position += parameter_types.width;
    let inner_right_paren = EditableSyntax.from_json(
      json.closure_inner_right_paren, position, source);
    position += inner_right_paren.width;
    let colon = EditableSyntax.from_json(
      json.closure_colon, position, source);
    position += colon.width;
    let return_type = EditableSyntax.from_json(
      json.closure_return_type, position, source);
    position += return_type.width;
    let outer_right_paren = EditableSyntax.from_json(
      json.closure_outer_right_paren, position, source);
    position += outer_right_paren.width;
    return new ClosureTypeSpecifier(
        outer_left_paren,
        coroutine,
        function_keyword,
        inner_left_paren,
        parameter_types,
        inner_right_paren,
        colon,
        return_type,
        outer_right_paren);
  }
  get children_keys()
  {
    if (ClosureTypeSpecifier._children_keys == null)
      ClosureTypeSpecifier._children_keys = [
        'outer_left_paren',
        'coroutine',
        'function_keyword',
        'inner_left_paren',
        'parameter_types',
        'inner_right_paren',
        'colon',
        'return_type',
        'outer_right_paren'];
    return ClosureTypeSpecifier._children_keys;
  }
}
class ClassnameTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_angle,
    type,
    trailing_comma,
    right_angle)
  {
    super('classname_type_specifier', {
      keyword: keyword,
      left_angle: left_angle,
      type: type,
      trailing_comma: trailing_comma,
      right_angle: right_angle });
  }
  get keyword() { return this.children.keyword; }
  get left_angle() { return this.children.left_angle; }
  get type() { return this.children.type; }
  get trailing_comma() { return this.children.trailing_comma; }
  get right_angle() { return this.children.right_angle; }
  with_keyword(keyword){
    return new ClassnameTypeSpecifier(
      keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_left_angle(left_angle){
    return new ClassnameTypeSpecifier(
      this.keyword,
      left_angle,
      this.type,
      this.trailing_comma,
      this.right_angle);
  }
  with_type(type){
    return new ClassnameTypeSpecifier(
      this.keyword,
      this.left_angle,
      type,
      this.trailing_comma,
      this.right_angle);
  }
  with_trailing_comma(trailing_comma){
    return new ClassnameTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      trailing_comma,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new ClassnameTypeSpecifier(
      this.keyword,
      this.left_angle,
      this.type,
      this.trailing_comma,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    var trailing_comma = this.trailing_comma.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_angle === this.left_angle &&
      type === this.type &&
      trailing_comma === this.trailing_comma &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ClassnameTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.classname_keyword, position, source);
    position += keyword.width;
    let left_angle = EditableSyntax.from_json(
      json.classname_left_angle, position, source);
    position += left_angle.width;
    let type = EditableSyntax.from_json(
      json.classname_type, position, source);
    position += type.width;
    let trailing_comma = EditableSyntax.from_json(
      json.classname_trailing_comma, position, source);
    position += trailing_comma.width;
    let right_angle = EditableSyntax.from_json(
      json.classname_right_angle, position, source);
    position += right_angle.width;
    return new ClassnameTypeSpecifier(
        keyword,
        left_angle,
        type,
        trailing_comma,
        right_angle);
  }
  get children_keys()
  {
    if (ClassnameTypeSpecifier._children_keys == null)
      ClassnameTypeSpecifier._children_keys = [
        'keyword',
        'left_angle',
        'type',
        'trailing_comma',
        'right_angle'];
    return ClassnameTypeSpecifier._children_keys;
  }
}
class FieldSpecifier extends EditableSyntax
{
  constructor(
    question,
    name,
    arrow,
    type)
  {
    super('field_specifier', {
      question: question,
      name: name,
      arrow: arrow,
      type: type });
  }
  get question() { return this.children.question; }
  get name() { return this.children.name; }
  get arrow() { return this.children.arrow; }
  get type() { return this.children.type; }
  with_question(question){
    return new FieldSpecifier(
      question,
      this.name,
      this.arrow,
      this.type);
  }
  with_name(name){
    return new FieldSpecifier(
      this.question,
      name,
      this.arrow,
      this.type);
  }
  with_arrow(arrow){
    return new FieldSpecifier(
      this.question,
      this.name,
      arrow,
      this.type);
  }
  with_type(type){
    return new FieldSpecifier(
      this.question,
      this.name,
      this.arrow,
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var question = this.question.rewrite(rewriter, new_parents);
    var name = this.name.rewrite(rewriter, new_parents);
    var arrow = this.arrow.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      question === this.question &&
      name === this.name &&
      arrow === this.arrow &&
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FieldSpecifier(
        question,
        name,
        arrow,
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let question = EditableSyntax.from_json(
      json.field_question, position, source);
    position += question.width;
    let name = EditableSyntax.from_json(
      json.field_name, position, source);
    position += name.width;
    let arrow = EditableSyntax.from_json(
      json.field_arrow, position, source);
    position += arrow.width;
    let type = EditableSyntax.from_json(
      json.field_type, position, source);
    position += type.width;
    return new FieldSpecifier(
        question,
        name,
        arrow,
        type);
  }
  get children_keys()
  {
    if (FieldSpecifier._children_keys == null)
      FieldSpecifier._children_keys = [
        'question',
        'name',
        'arrow',
        'type'];
    return FieldSpecifier._children_keys;
  }
}
class FieldInitializer extends EditableSyntax
{
  constructor(
    name,
    arrow,
    value)
  {
    super('field_initializer', {
      name: name,
      arrow: arrow,
      value: value });
  }
  get name() { return this.children.name; }
  get arrow() { return this.children.arrow; }
  get value() { return this.children.value; }
  with_name(name){
    return new FieldInitializer(
      name,
      this.arrow,
      this.value);
  }
  with_arrow(arrow){
    return new FieldInitializer(
      this.name,
      arrow,
      this.value);
  }
  with_value(value){
    return new FieldInitializer(
      this.name,
      this.arrow,
      value);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var name = this.name.rewrite(rewriter, new_parents);
    var arrow = this.arrow.rewrite(rewriter, new_parents);
    var value = this.value.rewrite(rewriter, new_parents);
    if (
      name === this.name &&
      arrow === this.arrow &&
      value === this.value)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new FieldInitializer(
        name,
        arrow,
        value), parents);
    }
  }
  static from_json(json, position, source)
  {
    let name = EditableSyntax.from_json(
      json.field_initializer_name, position, source);
    position += name.width;
    let arrow = EditableSyntax.from_json(
      json.field_initializer_arrow, position, source);
    position += arrow.width;
    let value = EditableSyntax.from_json(
      json.field_initializer_value, position, source);
    position += value.width;
    return new FieldInitializer(
        name,
        arrow,
        value);
  }
  get children_keys()
  {
    if (FieldInitializer._children_keys == null)
      FieldInitializer._children_keys = [
        'name',
        'arrow',
        'value'];
    return FieldInitializer._children_keys;
  }
}
class ShapeTypeSpecifier extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    fields,
    ellipsis,
    right_paren)
  {
    super('shape_type_specifier', {
      keyword: keyword,
      left_paren: left_paren,
      fields: fields,
      ellipsis: ellipsis,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get fields() { return this.children.fields; }
  get ellipsis() { return this.children.ellipsis; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new ShapeTypeSpecifier(
      keyword,
      this.left_paren,
      this.fields,
      this.ellipsis,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new ShapeTypeSpecifier(
      this.keyword,
      left_paren,
      this.fields,
      this.ellipsis,
      this.right_paren);
  }
  with_fields(fields){
    return new ShapeTypeSpecifier(
      this.keyword,
      this.left_paren,
      fields,
      this.ellipsis,
      this.right_paren);
  }
  with_ellipsis(ellipsis){
    return new ShapeTypeSpecifier(
      this.keyword,
      this.left_paren,
      this.fields,
      ellipsis,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ShapeTypeSpecifier(
      this.keyword,
      this.left_paren,
      this.fields,
      this.ellipsis,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var fields = this.fields.rewrite(rewriter, new_parents);
    var ellipsis = this.ellipsis.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      fields === this.fields &&
      ellipsis === this.ellipsis &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ShapeTypeSpecifier(
        keyword,
        left_paren,
        fields,
        ellipsis,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.shape_type_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.shape_type_left_paren, position, source);
    position += left_paren.width;
    let fields = EditableSyntax.from_json(
      json.shape_type_fields, position, source);
    position += fields.width;
    let ellipsis = EditableSyntax.from_json(
      json.shape_type_ellipsis, position, source);
    position += ellipsis.width;
    let right_paren = EditableSyntax.from_json(
      json.shape_type_right_paren, position, source);
    position += right_paren.width;
    return new ShapeTypeSpecifier(
        keyword,
        left_paren,
        fields,
        ellipsis,
        right_paren);
  }
  get children_keys()
  {
    if (ShapeTypeSpecifier._children_keys == null)
      ShapeTypeSpecifier._children_keys = [
        'keyword',
        'left_paren',
        'fields',
        'ellipsis',
        'right_paren'];
    return ShapeTypeSpecifier._children_keys;
  }
}
class ShapeExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    fields,
    right_paren)
  {
    super('shape_expression', {
      keyword: keyword,
      left_paren: left_paren,
      fields: fields,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get fields() { return this.children.fields; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new ShapeExpression(
      keyword,
      this.left_paren,
      this.fields,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new ShapeExpression(
      this.keyword,
      left_paren,
      this.fields,
      this.right_paren);
  }
  with_fields(fields){
    return new ShapeExpression(
      this.keyword,
      this.left_paren,
      fields,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new ShapeExpression(
      this.keyword,
      this.left_paren,
      this.fields,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var fields = this.fields.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      fields === this.fields &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ShapeExpression(
        keyword,
        left_paren,
        fields,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.shape_expression_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.shape_expression_left_paren, position, source);
    position += left_paren.width;
    let fields = EditableSyntax.from_json(
      json.shape_expression_fields, position, source);
    position += fields.width;
    let right_paren = EditableSyntax.from_json(
      json.shape_expression_right_paren, position, source);
    position += right_paren.width;
    return new ShapeExpression(
        keyword,
        left_paren,
        fields,
        right_paren);
  }
  get children_keys()
  {
    if (ShapeExpression._children_keys == null)
      ShapeExpression._children_keys = [
        'keyword',
        'left_paren',
        'fields',
        'right_paren'];
    return ShapeExpression._children_keys;
  }
}
class TupleExpression extends EditableSyntax
{
  constructor(
    keyword,
    left_paren,
    items,
    right_paren)
  {
    super('tuple_expression', {
      keyword: keyword,
      left_paren: left_paren,
      items: items,
      right_paren: right_paren });
  }
  get keyword() { return this.children.keyword; }
  get left_paren() { return this.children.left_paren; }
  get items() { return this.children.items; }
  get right_paren() { return this.children.right_paren; }
  with_keyword(keyword){
    return new TupleExpression(
      keyword,
      this.left_paren,
      this.items,
      this.right_paren);
  }
  with_left_paren(left_paren){
    return new TupleExpression(
      this.keyword,
      left_paren,
      this.items,
      this.right_paren);
  }
  with_items(items){
    return new TupleExpression(
      this.keyword,
      this.left_paren,
      items,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new TupleExpression(
      this.keyword,
      this.left_paren,
      this.items,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var keyword = this.keyword.rewrite(rewriter, new_parents);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var items = this.items.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      keyword === this.keyword &&
      left_paren === this.left_paren &&
      items === this.items &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TupleExpression(
        keyword,
        left_paren,
        items,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let keyword = EditableSyntax.from_json(
      json.tuple_expression_keyword, position, source);
    position += keyword.width;
    let left_paren = EditableSyntax.from_json(
      json.tuple_expression_left_paren, position, source);
    position += left_paren.width;
    let items = EditableSyntax.from_json(
      json.tuple_expression_items, position, source);
    position += items.width;
    let right_paren = EditableSyntax.from_json(
      json.tuple_expression_right_paren, position, source);
    position += right_paren.width;
    return new TupleExpression(
        keyword,
        left_paren,
        items,
        right_paren);
  }
  get children_keys()
  {
    if (TupleExpression._children_keys == null)
      TupleExpression._children_keys = [
        'keyword',
        'left_paren',
        'items',
        'right_paren'];
    return TupleExpression._children_keys;
  }
}
class GenericTypeSpecifier extends EditableSyntax
{
  constructor(
    class_type,
    argument_list)
  {
    super('generic_type_specifier', {
      class_type: class_type,
      argument_list: argument_list });
  }
  get class_type() { return this.children.class_type; }
  get argument_list() { return this.children.argument_list; }
  with_class_type(class_type){
    return new GenericTypeSpecifier(
      class_type,
      this.argument_list);
  }
  with_argument_list(argument_list){
    return new GenericTypeSpecifier(
      this.class_type,
      argument_list);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var class_type = this.class_type.rewrite(rewriter, new_parents);
    var argument_list = this.argument_list.rewrite(rewriter, new_parents);
    if (
      class_type === this.class_type &&
      argument_list === this.argument_list)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new GenericTypeSpecifier(
        class_type,
        argument_list), parents);
    }
  }
  static from_json(json, position, source)
  {
    let class_type = EditableSyntax.from_json(
      json.generic_class_type, position, source);
    position += class_type.width;
    let argument_list = EditableSyntax.from_json(
      json.generic_argument_list, position, source);
    position += argument_list.width;
    return new GenericTypeSpecifier(
        class_type,
        argument_list);
  }
  get children_keys()
  {
    if (GenericTypeSpecifier._children_keys == null)
      GenericTypeSpecifier._children_keys = [
        'class_type',
        'argument_list'];
    return GenericTypeSpecifier._children_keys;
  }
}
class NullableTypeSpecifier extends EditableSyntax
{
  constructor(
    question,
    type)
  {
    super('nullable_type_specifier', {
      question: question,
      type: type });
  }
  get question() { return this.children.question; }
  get type() { return this.children.type; }
  with_question(question){
    return new NullableTypeSpecifier(
      question,
      this.type);
  }
  with_type(type){
    return new NullableTypeSpecifier(
      this.question,
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var question = this.question.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      question === this.question &&
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new NullableTypeSpecifier(
        question,
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let question = EditableSyntax.from_json(
      json.nullable_question, position, source);
    position += question.width;
    let type = EditableSyntax.from_json(
      json.nullable_type, position, source);
    position += type.width;
    return new NullableTypeSpecifier(
        question,
        type);
  }
  get children_keys()
  {
    if (NullableTypeSpecifier._children_keys == null)
      NullableTypeSpecifier._children_keys = [
        'question',
        'type'];
    return NullableTypeSpecifier._children_keys;
  }
}
class SoftTypeSpecifier extends EditableSyntax
{
  constructor(
    at,
    type)
  {
    super('soft_type_specifier', {
      at: at,
      type: type });
  }
  get at() { return this.children.at; }
  get type() { return this.children.type; }
  with_at(at){
    return new SoftTypeSpecifier(
      at,
      this.type);
  }
  with_type(type){
    return new SoftTypeSpecifier(
      this.at,
      type);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var at = this.at.rewrite(rewriter, new_parents);
    var type = this.type.rewrite(rewriter, new_parents);
    if (
      at === this.at &&
      type === this.type)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new SoftTypeSpecifier(
        at,
        type), parents);
    }
  }
  static from_json(json, position, source)
  {
    let at = EditableSyntax.from_json(
      json.soft_at, position, source);
    position += at.width;
    let type = EditableSyntax.from_json(
      json.soft_type, position, source);
    position += type.width;
    return new SoftTypeSpecifier(
        at,
        type);
  }
  get children_keys()
  {
    if (SoftTypeSpecifier._children_keys == null)
      SoftTypeSpecifier._children_keys = [
        'at',
        'type'];
    return SoftTypeSpecifier._children_keys;
  }
}
class TypeArguments extends EditableSyntax
{
  constructor(
    left_angle,
    types,
    right_angle)
  {
    super('type_arguments', {
      left_angle: left_angle,
      types: types,
      right_angle: right_angle });
  }
  get left_angle() { return this.children.left_angle; }
  get types() { return this.children.types; }
  get right_angle() { return this.children.right_angle; }
  with_left_angle(left_angle){
    return new TypeArguments(
      left_angle,
      this.types,
      this.right_angle);
  }
  with_types(types){
    return new TypeArguments(
      this.left_angle,
      types,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new TypeArguments(
      this.left_angle,
      this.types,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var types = this.types.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      left_angle === this.left_angle &&
      types === this.types &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeArguments(
        left_angle,
        types,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_angle = EditableSyntax.from_json(
      json.type_arguments_left_angle, position, source);
    position += left_angle.width;
    let types = EditableSyntax.from_json(
      json.type_arguments_types, position, source);
    position += types.width;
    let right_angle = EditableSyntax.from_json(
      json.type_arguments_right_angle, position, source);
    position += right_angle.width;
    return new TypeArguments(
        left_angle,
        types,
        right_angle);
  }
  get children_keys()
  {
    if (TypeArguments._children_keys == null)
      TypeArguments._children_keys = [
        'left_angle',
        'types',
        'right_angle'];
    return TypeArguments._children_keys;
  }
}
class TypeParameters extends EditableSyntax
{
  constructor(
    left_angle,
    parameters,
    right_angle)
  {
    super('type_parameters', {
      left_angle: left_angle,
      parameters: parameters,
      right_angle: right_angle });
  }
  get left_angle() { return this.children.left_angle; }
  get parameters() { return this.children.parameters; }
  get right_angle() { return this.children.right_angle; }
  with_left_angle(left_angle){
    return new TypeParameters(
      left_angle,
      this.parameters,
      this.right_angle);
  }
  with_parameters(parameters){
    return new TypeParameters(
      this.left_angle,
      parameters,
      this.right_angle);
  }
  with_right_angle(right_angle){
    return new TypeParameters(
      this.left_angle,
      this.parameters,
      right_angle);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_angle = this.left_angle.rewrite(rewriter, new_parents);
    var parameters = this.parameters.rewrite(rewriter, new_parents);
    var right_angle = this.right_angle.rewrite(rewriter, new_parents);
    if (
      left_angle === this.left_angle &&
      parameters === this.parameters &&
      right_angle === this.right_angle)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TypeParameters(
        left_angle,
        parameters,
        right_angle), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_angle = EditableSyntax.from_json(
      json.type_parameters_left_angle, position, source);
    position += left_angle.width;
    let parameters = EditableSyntax.from_json(
      json.type_parameters_parameters, position, source);
    position += parameters.width;
    let right_angle = EditableSyntax.from_json(
      json.type_parameters_right_angle, position, source);
    position += right_angle.width;
    return new TypeParameters(
        left_angle,
        parameters,
        right_angle);
  }
  get children_keys()
  {
    if (TypeParameters._children_keys == null)
      TypeParameters._children_keys = [
        'left_angle',
        'parameters',
        'right_angle'];
    return TypeParameters._children_keys;
  }
}
class TupleTypeSpecifier extends EditableSyntax
{
  constructor(
    left_paren,
    types,
    right_paren)
  {
    super('tuple_type_specifier', {
      left_paren: left_paren,
      types: types,
      right_paren: right_paren });
  }
  get left_paren() { return this.children.left_paren; }
  get types() { return this.children.types; }
  get right_paren() { return this.children.right_paren; }
  with_left_paren(left_paren){
    return new TupleTypeSpecifier(
      left_paren,
      this.types,
      this.right_paren);
  }
  with_types(types){
    return new TupleTypeSpecifier(
      this.left_paren,
      types,
      this.right_paren);
  }
  with_right_paren(right_paren){
    return new TupleTypeSpecifier(
      this.left_paren,
      this.types,
      right_paren);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var left_paren = this.left_paren.rewrite(rewriter, new_parents);
    var types = this.types.rewrite(rewriter, new_parents);
    var right_paren = this.right_paren.rewrite(rewriter, new_parents);
    if (
      left_paren === this.left_paren &&
      types === this.types &&
      right_paren === this.right_paren)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new TupleTypeSpecifier(
        left_paren,
        types,
        right_paren), parents);
    }
  }
  static from_json(json, position, source)
  {
    let left_paren = EditableSyntax.from_json(
      json.tuple_left_paren, position, source);
    position += left_paren.width;
    let types = EditableSyntax.from_json(
      json.tuple_types, position, source);
    position += types.width;
    let right_paren = EditableSyntax.from_json(
      json.tuple_right_paren, position, source);
    position += right_paren.width;
    return new TupleTypeSpecifier(
        left_paren,
        types,
        right_paren);
  }
  get children_keys()
  {
    if (TupleTypeSpecifier._children_keys == null)
      TupleTypeSpecifier._children_keys = [
        'left_paren',
        'types',
        'right_paren'];
    return TupleTypeSpecifier._children_keys;
  }
}
class ErrorSyntax extends EditableSyntax
{
  constructor(
    error)
  {
    super('error', {
      error: error });
  }
  get error() { return this.children.error; }
  with_error(error){
    return new ErrorSyntax(
      error);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var error = this.error.rewrite(rewriter, new_parents);
    if (
      error === this.error)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ErrorSyntax(
        error), parents);
    }
  }
  static from_json(json, position, source)
  {
    let error = EditableSyntax.from_json(
      json.error_error, position, source);
    position += error.width;
    return new ErrorSyntax(
        error);
  }
  get children_keys()
  {
    if (ErrorSyntax._children_keys == null)
      ErrorSyntax._children_keys = [
        'error'];
    return ErrorSyntax._children_keys;
  }
}
class ListItem extends EditableSyntax
{
  constructor(
    item,
    separator)
  {
    super('list_item', {
      item: item,
      separator: separator });
  }
  get item() { return this.children.item; }
  get separator() { return this.children.separator; }
  with_item(item){
    return new ListItem(
      item,
      this.separator);
  }
  with_separator(separator){
    return new ListItem(
      this.item,
      separator);
  }
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    var item = this.item.rewrite(rewriter, new_parents);
    var separator = this.separator.rewrite(rewriter, new_parents);
    if (
      item === this.item &&
      separator === this.separator)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new ListItem(
        item,
        separator), parents);
    }
  }
  static from_json(json, position, source)
  {
    let item = EditableSyntax.from_json(
      json.list_item, position, source);
    position += item.width;
    let separator = EditableSyntax.from_json(
      json.list_separator, position, source);
    position += separator.width;
    return new ListItem(
        item,
        separator);
  }
  get children_keys()
  {
    if (ListItem._children_keys == null)
      ListItem._children_keys = [
        'item',
        'separator'];
    return ListItem._children_keys;
  }
}


function from_json(json)
{
  return EditableSyntax.from_json(json.parse_tree, 0, json.program_text);
}

exports.from_json = from_json;
exports.EditableSyntax = EditableSyntax;
exports.EditableList = EditableList;
exports.EditableToken = EditableToken;
exports.EndOfFileToken = EndOfFileToken;

exports.AbstractToken = AbstractToken;
exports.AndToken = AndToken;
exports.ArrayToken = ArrayToken;
exports.ArraykeyToken = ArraykeyToken;
exports.AsToken = AsToken;
exports.AsyncToken = AsyncToken;
exports.AttributeToken = AttributeToken;
exports.AwaitToken = AwaitToken;
exports.BoolToken = BoolToken;
exports.BreakToken = BreakToken;
exports.CaseToken = CaseToken;
exports.CatchToken = CatchToken;
exports.CategoryToken = CategoryToken;
exports.ChildrenToken = ChildrenToken;
exports.ClassToken = ClassToken;
exports.ClassnameToken = ClassnameToken;
exports.CloneToken = CloneToken;
exports.ConstToken = ConstToken;
exports.ConstructToken = ConstructToken;
exports.ContinueToken = ContinueToken;
exports.CoroutineToken = CoroutineToken;
exports.DarrayToken = DarrayToken;
exports.DefaultToken = DefaultToken;
exports.DefineToken = DefineToken;
exports.DestructToken = DestructToken;
exports.DictToken = DictToken;
exports.DoToken = DoToken;
exports.DoubleToken = DoubleToken;
exports.EchoToken = EchoToken;
exports.ElseToken = ElseToken;
exports.ElseifToken = ElseifToken;
exports.EmptyToken = EmptyToken;
exports.EnumToken = EnumToken;
exports.EvalToken = EvalToken;
exports.ExtendsToken = ExtendsToken;
exports.FallthroughToken = FallthroughToken;
exports.FloatToken = FloatToken;
exports.FinalToken = FinalToken;
exports.FinallyToken = FinallyToken;
exports.ForToken = ForToken;
exports.ForeachToken = ForeachToken;
exports.FromToken = FromToken;
exports.FunctionToken = FunctionToken;
exports.GlobalToken = GlobalToken;
exports.GotoToken = GotoToken;
exports.IfToken = IfToken;
exports.ImplementsToken = ImplementsToken;
exports.IncludeToken = IncludeToken;
exports.Include_onceToken = Include_onceToken;
exports.InstanceofToken = InstanceofToken;
exports.InsteadofToken = InsteadofToken;
exports.IntToken = IntToken;
exports.InterfaceToken = InterfaceToken;
exports.IssetToken = IssetToken;
exports.KeysetToken = KeysetToken;
exports.ListToken = ListToken;
exports.MixedToken = MixedToken;
exports.NamespaceToken = NamespaceToken;
exports.NewToken = NewToken;
exports.NewtypeToken = NewtypeToken;
exports.NoreturnToken = NoreturnToken;
exports.NumToken = NumToken;
exports.ObjectToken = ObjectToken;
exports.OrToken = OrToken;
exports.ParentToken = ParentToken;
exports.PrintToken = PrintToken;
exports.PrivateToken = PrivateToken;
exports.ProtectedToken = ProtectedToken;
exports.PublicToken = PublicToken;
exports.RequireToken = RequireToken;
exports.Require_onceToken = Require_onceToken;
exports.RequiredToken = RequiredToken;
exports.ResourceToken = ResourceToken;
exports.ReturnToken = ReturnToken;
exports.SelfToken = SelfToken;
exports.ShapeToken = ShapeToken;
exports.StaticToken = StaticToken;
exports.StringToken = StringToken;
exports.SuperToken = SuperToken;
exports.SuspendToken = SuspendToken;
exports.SwitchToken = SwitchToken;
exports.ThisToken = ThisToken;
exports.ThrowToken = ThrowToken;
exports.TraitToken = TraitToken;
exports.TryToken = TryToken;
exports.TupleToken = TupleToken;
exports.TypeToken = TypeToken;
exports.UnsetToken = UnsetToken;
exports.UseToken = UseToken;
exports.VarToken = VarToken;
exports.VarrayToken = VarrayToken;
exports.VecToken = VecToken;
exports.VoidToken = VoidToken;
exports.WhereToken = WhereToken;
exports.WhileToken = WhileToken;
exports.XorToken = XorToken;
exports.YieldToken = YieldToken;
exports.LeftBracketToken = LeftBracketToken;
exports.RightBracketToken = RightBracketToken;
exports.LeftParenToken = LeftParenToken;
exports.RightParenToken = RightParenToken;
exports.LeftBraceToken = LeftBraceToken;
exports.RightBraceToken = RightBraceToken;
exports.DotToken = DotToken;
exports.MinusGreaterThanToken = MinusGreaterThanToken;
exports.PlusPlusToken = PlusPlusToken;
exports.MinusMinusToken = MinusMinusToken;
exports.StarStarToken = StarStarToken;
exports.StarToken = StarToken;
exports.PlusToken = PlusToken;
exports.MinusToken = MinusToken;
exports.TildeToken = TildeToken;
exports.ExclamationToken = ExclamationToken;
exports.DollarToken = DollarToken;
exports.SlashToken = SlashToken;
exports.PercentToken = PercentToken;
exports.LessThanGreaterThanToken = LessThanGreaterThanToken;
exports.LessThanEqualGreaterThanToken = LessThanEqualGreaterThanToken;
exports.LessThanLessThanToken = LessThanLessThanToken;
exports.GreaterThanGreaterThanToken = GreaterThanGreaterThanToken;
exports.LessThanToken = LessThanToken;
exports.GreaterThanToken = GreaterThanToken;
exports.LessThanEqualToken = LessThanEqualToken;
exports.GreaterThanEqualToken = GreaterThanEqualToken;
exports.EqualEqualToken = EqualEqualToken;
exports.EqualEqualEqualToken = EqualEqualEqualToken;
exports.ExclamationEqualToken = ExclamationEqualToken;
exports.ExclamationEqualEqualToken = ExclamationEqualEqualToken;
exports.CaratToken = CaratToken;
exports.BarToken = BarToken;
exports.AmpersandToken = AmpersandToken;
exports.AmpersandAmpersandToken = AmpersandAmpersandToken;
exports.BarBarToken = BarBarToken;
exports.QuestionToken = QuestionToken;
exports.QuestionQuestionToken = QuestionQuestionToken;
exports.ColonToken = ColonToken;
exports.SemicolonToken = SemicolonToken;
exports.EqualToken = EqualToken;
exports.StarStarEqualToken = StarStarEqualToken;
exports.StarEqualToken = StarEqualToken;
exports.SlashEqualToken = SlashEqualToken;
exports.PercentEqualToken = PercentEqualToken;
exports.PlusEqualToken = PlusEqualToken;
exports.MinusEqualToken = MinusEqualToken;
exports.DotEqualToken = DotEqualToken;
exports.LessThanLessThanEqualToken = LessThanLessThanEqualToken;
exports.GreaterThanGreaterThanEqualToken = GreaterThanGreaterThanEqualToken;
exports.AmpersandEqualToken = AmpersandEqualToken;
exports.CaratEqualToken = CaratEqualToken;
exports.BarEqualToken = BarEqualToken;
exports.CommaToken = CommaToken;
exports.AtToken = AtToken;
exports.ColonColonToken = ColonColonToken;
exports.EqualGreaterThanToken = EqualGreaterThanToken;
exports.EqualEqualGreaterThanToken = EqualEqualGreaterThanToken;
exports.QuestionMinusGreaterThanToken = QuestionMinusGreaterThanToken;
exports.DotDotDotToken = DotDotDotToken;
exports.DollarDollarToken = DollarDollarToken;
exports.BarGreaterThanToken = BarGreaterThanToken;
exports.NullLiteralToken = NullLiteralToken;
exports.SlashGreaterThanToken = SlashGreaterThanToken;
exports.LessThanSlashToken = LessThanSlashToken;
exports.LessThanQuestionToken = LessThanQuestionToken;
exports.QuestionGreaterThanToken = QuestionGreaterThanToken;

exports.ErrorTokenToken = ErrorTokenToken;
exports.NameToken = NameToken;
exports.QualifiedNameToken = QualifiedNameToken;
exports.VariableToken = VariableToken;
exports.NamespacePrefixToken = NamespacePrefixToken;
exports.DecimalLiteralToken = DecimalLiteralToken;
exports.OctalLiteralToken = OctalLiteralToken;
exports.HexadecimalLiteralToken = HexadecimalLiteralToken;
exports.BinaryLiteralToken = BinaryLiteralToken;
exports.FloatingLiteralToken = FloatingLiteralToken;
exports.ExecutionStringToken = ExecutionStringToken;
exports.SingleQuotedStringLiteralToken = SingleQuotedStringLiteralToken;
exports.DoubleQuotedStringLiteralToken = DoubleQuotedStringLiteralToken;
exports.DoubleQuotedStringLiteralHeadToken = DoubleQuotedStringLiteralHeadToken;
exports.StringLiteralBodyToken = StringLiteralBodyToken;
exports.DoubleQuotedStringLiteralTailToken = DoubleQuotedStringLiteralTailToken;
exports.HeredocStringLiteralToken = HeredocStringLiteralToken;
exports.HeredocStringLiteralHeadToken = HeredocStringLiteralHeadToken;
exports.HeredocStringLiteralTailToken = HeredocStringLiteralTailToken;
exports.NowdocStringLiteralToken = NowdocStringLiteralToken;
exports.BooleanLiteralToken = BooleanLiteralToken;
exports.XHPCategoryNameToken = XHPCategoryNameToken;
exports.XHPElementNameToken = XHPElementNameToken;
exports.XHPClassNameToken = XHPClassNameToken;
exports.XHPStringLiteralToken = XHPStringLiteralToken;
exports.XHPBodyToken = XHPBodyToken;
exports.XHPCommentToken = XHPCommentToken;
exports.MarkupToken = MarkupToken;

exports.EditableTrivia = EditableTrivia;
exports.WhiteSpace = WhiteSpace;
exports.EndOfLine = EndOfLine;
exports.DelimitedComment = DelimitedComment;
exports.SingleLineComment = SingleLineComment;
exports.Unsafe = Unsafe;
exports.UnsafeExpression = UnsafeExpression;
exports.FixMe = FixMe;
exports.IgnoreError = IgnoreError;
exports.FallThrough = FallThrough;
exports.ExtraTokenError = ExtraTokenError;

exports.EndOfFile = EndOfFile;
exports.Script = Script;
exports.SimpleTypeSpecifier = SimpleTypeSpecifier;
exports.LiteralExpression = LiteralExpression;
exports.VariableExpression = VariableExpression;
exports.QualifiedNameExpression = QualifiedNameExpression;
exports.PipeVariableExpression = PipeVariableExpression;
exports.EnumDeclaration = EnumDeclaration;
exports.Enumerator = Enumerator;
exports.AliasDeclaration = AliasDeclaration;
exports.PropertyDeclaration = PropertyDeclaration;
exports.PropertyDeclarator = PropertyDeclarator;
exports.NamespaceDeclaration = NamespaceDeclaration;
exports.NamespaceBody = NamespaceBody;
exports.NamespaceEmptyBody = NamespaceEmptyBody;
exports.NamespaceUseDeclaration = NamespaceUseDeclaration;
exports.NamespaceGroupUseDeclaration = NamespaceGroupUseDeclaration;
exports.NamespaceUseClause = NamespaceUseClause;
exports.FunctionDeclaration = FunctionDeclaration;
exports.FunctionDeclarationHeader = FunctionDeclarationHeader;
exports.WhereClause = WhereClause;
exports.WhereConstraint = WhereConstraint;
exports.MethodishDeclaration = MethodishDeclaration;
exports.ClassishDeclaration = ClassishDeclaration;
exports.ClassishBody = ClassishBody;
exports.TraitUsePrecedenceItem = TraitUsePrecedenceItem;
exports.TraitUseAliasItem = TraitUseAliasItem;
exports.TraitUseConflictResolution = TraitUseConflictResolution;
exports.TraitUse = TraitUse;
exports.RequireClause = RequireClause;
exports.ConstDeclaration = ConstDeclaration;
exports.ConstantDeclarator = ConstantDeclarator;
exports.TypeConstDeclaration = TypeConstDeclaration;
exports.DecoratedExpression = DecoratedExpression;
exports.ParameterDeclaration = ParameterDeclaration;
exports.VariadicParameter = VariadicParameter;
exports.AttributeSpecification = AttributeSpecification;
exports.Attribute = Attribute;
exports.InclusionExpression = InclusionExpression;
exports.InclusionDirective = InclusionDirective;
exports.CompoundStatement = CompoundStatement;
exports.ExpressionStatement = ExpressionStatement;
exports.MarkupSection = MarkupSection;
exports.MarkupSuffix = MarkupSuffix;
exports.UnsetStatement = UnsetStatement;
exports.WhileStatement = WhileStatement;
exports.IfStatement = IfStatement;
exports.ElseifClause = ElseifClause;
exports.ElseClause = ElseClause;
exports.TryStatement = TryStatement;
exports.CatchClause = CatchClause;
exports.FinallyClause = FinallyClause;
exports.DoStatement = DoStatement;
exports.ForStatement = ForStatement;
exports.ForeachStatement = ForeachStatement;
exports.SwitchStatement = SwitchStatement;
exports.SwitchSection = SwitchSection;
exports.SwitchFallthrough = SwitchFallthrough;
exports.CaseLabel = CaseLabel;
exports.DefaultLabel = DefaultLabel;
exports.ReturnStatement = ReturnStatement;
exports.GotoLabel = GotoLabel;
exports.GotoStatement = GotoStatement;
exports.ThrowStatement = ThrowStatement;
exports.BreakStatement = BreakStatement;
exports.ContinueStatement = ContinueStatement;
exports.FunctionStaticStatement = FunctionStaticStatement;
exports.StaticDeclarator = StaticDeclarator;
exports.EchoStatement = EchoStatement;
exports.GlobalStatement = GlobalStatement;
exports.SimpleInitializer = SimpleInitializer;
exports.AnonymousFunction = AnonymousFunction;
exports.AnonymousFunctionUseClause = AnonymousFunctionUseClause;
exports.LambdaExpression = LambdaExpression;
exports.LambdaSignature = LambdaSignature;
exports.CastExpression = CastExpression;
exports.ScopeResolutionExpression = ScopeResolutionExpression;
exports.MemberSelectionExpression = MemberSelectionExpression;
exports.SafeMemberSelectionExpression = SafeMemberSelectionExpression;
exports.EmbeddedMemberSelectionExpression = EmbeddedMemberSelectionExpression;
exports.YieldExpression = YieldExpression;
exports.YieldFromExpression = YieldFromExpression;
exports.PrefixUnaryExpression = PrefixUnaryExpression;
exports.PostfixUnaryExpression = PostfixUnaryExpression;
exports.BinaryExpression = BinaryExpression;
exports.InstanceofExpression = InstanceofExpression;
exports.ConditionalExpression = ConditionalExpression;
exports.EvalExpression = EvalExpression;
exports.EmptyExpression = EmptyExpression;
exports.DefineExpression = DefineExpression;
exports.IssetExpression = IssetExpression;
exports.FunctionCallExpression = FunctionCallExpression;
exports.ParenthesizedExpression = ParenthesizedExpression;
exports.BracedExpression = BracedExpression;
exports.EmbeddedBracedExpression = EmbeddedBracedExpression;
exports.ListExpression = ListExpression;
exports.CollectionLiteralExpression = CollectionLiteralExpression;
exports.ObjectCreationExpression = ObjectCreationExpression;
exports.ArrayCreationExpression = ArrayCreationExpression;
exports.ArrayIntrinsicExpression = ArrayIntrinsicExpression;
exports.DarrayIntrinsicExpression = DarrayIntrinsicExpression;
exports.DictionaryIntrinsicExpression = DictionaryIntrinsicExpression;
exports.KeysetIntrinsicExpression = KeysetIntrinsicExpression;
exports.VarrayIntrinsicExpression = VarrayIntrinsicExpression;
exports.VectorIntrinsicExpression = VectorIntrinsicExpression;
exports.ElementInitializer = ElementInitializer;
exports.SubscriptExpression = SubscriptExpression;
exports.EmbeddedSubscriptExpression = EmbeddedSubscriptExpression;
exports.AwaitableCreationExpression = AwaitableCreationExpression;
exports.XHPChildrenDeclaration = XHPChildrenDeclaration;
exports.XHPChildrenParenthesizedList = XHPChildrenParenthesizedList;
exports.XHPCategoryDeclaration = XHPCategoryDeclaration;
exports.XHPEnumType = XHPEnumType;
exports.XHPRequired = XHPRequired;
exports.XHPClassAttributeDeclaration = XHPClassAttributeDeclaration;
exports.XHPClassAttribute = XHPClassAttribute;
exports.XHPSimpleClassAttribute = XHPSimpleClassAttribute;
exports.XHPAttribute = XHPAttribute;
exports.XHPOpen = XHPOpen;
exports.XHPExpression = XHPExpression;
exports.XHPClose = XHPClose;
exports.TypeConstant = TypeConstant;
exports.VectorTypeSpecifier = VectorTypeSpecifier;
exports.KeysetTypeSpecifier = KeysetTypeSpecifier;
exports.TupleTypeExplicitSpecifier = TupleTypeExplicitSpecifier;
exports.VarrayTypeSpecifier = VarrayTypeSpecifier;
exports.VectorArrayTypeSpecifier = VectorArrayTypeSpecifier;
exports.TypeParameter = TypeParameter;
exports.TypeConstraint = TypeConstraint;
exports.DarrayTypeSpecifier = DarrayTypeSpecifier;
exports.MapArrayTypeSpecifier = MapArrayTypeSpecifier;
exports.DictionaryTypeSpecifier = DictionaryTypeSpecifier;
exports.ClosureTypeSpecifier = ClosureTypeSpecifier;
exports.ClassnameTypeSpecifier = ClassnameTypeSpecifier;
exports.FieldSpecifier = FieldSpecifier;
exports.FieldInitializer = FieldInitializer;
exports.ShapeTypeSpecifier = ShapeTypeSpecifier;
exports.ShapeExpression = ShapeExpression;
exports.TupleExpression = TupleExpression;
exports.GenericTypeSpecifier = GenericTypeSpecifier;
exports.NullableTypeSpecifier = NullableTypeSpecifier;
exports.SoftTypeSpecifier = SoftTypeSpecifier;
exports.TypeArguments = TypeArguments;
exports.TypeParameters = TypeParameters;
exports.TupleTypeSpecifier = TupleTypeSpecifier;
exports.ErrorSyntax = ErrorSyntax;
exports.ListItem = ListItem;

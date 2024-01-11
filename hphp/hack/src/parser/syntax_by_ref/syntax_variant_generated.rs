/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */
use super::{
    syntax::Syntax,
    syntax_children_iterator::SyntaxChildrenIterator,
};

#[derive(Debug, Clone)]
pub enum SyntaxVariant<'a, T, V> {
    Token(T),
    Missing,
    SyntaxList(&'a [Syntax<'a, T, V>]),
    EndOfFile(&'a EndOfFileChildren<'a, T, V>),
    Script(&'a ScriptChildren<'a, T, V>),
    QualifiedName(&'a QualifiedNameChildren<'a, T, V>),
    ModuleName(&'a ModuleNameChildren<'a, T, V>),
    SimpleTypeSpecifier(&'a SimpleTypeSpecifierChildren<'a, T, V>),
    LiteralExpression(&'a LiteralExpressionChildren<'a, T, V>),
    PrefixedStringExpression(&'a PrefixedStringExpressionChildren<'a, T, V>),
    PrefixedCodeExpression(&'a PrefixedCodeExpressionChildren<'a, T, V>),
    VariableExpression(&'a VariableExpressionChildren<'a, T, V>),
    PipeVariableExpression(&'a PipeVariableExpressionChildren<'a, T, V>),
    FileAttributeSpecification(&'a FileAttributeSpecificationChildren<'a, T, V>),
    EnumDeclaration(&'a EnumDeclarationChildren<'a, T, V>),
    EnumUse(&'a EnumUseChildren<'a, T, V>),
    Enumerator(&'a EnumeratorChildren<'a, T, V>),
    EnumClassDeclaration(&'a EnumClassDeclarationChildren<'a, T, V>),
    EnumClassEnumerator(&'a EnumClassEnumeratorChildren<'a, T, V>),
    AliasDeclaration(&'a AliasDeclarationChildren<'a, T, V>),
    ContextAliasDeclaration(&'a ContextAliasDeclarationChildren<'a, T, V>),
    CaseTypeDeclaration(&'a CaseTypeDeclarationChildren<'a, T, V>),
    CaseTypeVariant(&'a CaseTypeVariantChildren<'a, T, V>),
    PropertyDeclaration(&'a PropertyDeclarationChildren<'a, T, V>),
    PropertyDeclarator(&'a PropertyDeclaratorChildren<'a, T, V>),
    NamespaceDeclaration(&'a NamespaceDeclarationChildren<'a, T, V>),
    NamespaceDeclarationHeader(&'a NamespaceDeclarationHeaderChildren<'a, T, V>),
    NamespaceBody(&'a NamespaceBodyChildren<'a, T, V>),
    NamespaceEmptyBody(&'a NamespaceEmptyBodyChildren<'a, T, V>),
    NamespaceUseDeclaration(&'a NamespaceUseDeclarationChildren<'a, T, V>),
    NamespaceGroupUseDeclaration(&'a NamespaceGroupUseDeclarationChildren<'a, T, V>),
    NamespaceUseClause(&'a NamespaceUseClauseChildren<'a, T, V>),
    FunctionDeclaration(&'a FunctionDeclarationChildren<'a, T, V>),
    FunctionDeclarationHeader(&'a FunctionDeclarationHeaderChildren<'a, T, V>),
    Contexts(&'a ContextsChildren<'a, T, V>),
    WhereClause(&'a WhereClauseChildren<'a, T, V>),
    WhereConstraint(&'a WhereConstraintChildren<'a, T, V>),
    MethodishDeclaration(&'a MethodishDeclarationChildren<'a, T, V>),
    MethodishTraitResolution(&'a MethodishTraitResolutionChildren<'a, T, V>),
    ClassishDeclaration(&'a ClassishDeclarationChildren<'a, T, V>),
    ClassishBody(&'a ClassishBodyChildren<'a, T, V>),
    TraitUse(&'a TraitUseChildren<'a, T, V>),
    RequireClause(&'a RequireClauseChildren<'a, T, V>),
    ConstDeclaration(&'a ConstDeclarationChildren<'a, T, V>),
    ConstantDeclarator(&'a ConstantDeclaratorChildren<'a, T, V>),
    TypeConstDeclaration(&'a TypeConstDeclarationChildren<'a, T, V>),
    ContextConstDeclaration(&'a ContextConstDeclarationChildren<'a, T, V>),
    DecoratedExpression(&'a DecoratedExpressionChildren<'a, T, V>),
    ParameterDeclaration(&'a ParameterDeclarationChildren<'a, T, V>),
    VariadicParameter(&'a VariadicParameterChildren<'a, T, V>),
    OldAttributeSpecification(&'a OldAttributeSpecificationChildren<'a, T, V>),
    AttributeSpecification(&'a AttributeSpecificationChildren<'a, T, V>),
    Attribute(&'a AttributeChildren<'a, T, V>),
    InclusionExpression(&'a InclusionExpressionChildren<'a, T, V>),
    InclusionDirective(&'a InclusionDirectiveChildren<'a, T, V>),
    CompoundStatement(&'a CompoundStatementChildren<'a, T, V>),
    ExpressionStatement(&'a ExpressionStatementChildren<'a, T, V>),
    MarkupSection(&'a MarkupSectionChildren<'a, T, V>),
    MarkupSuffix(&'a MarkupSuffixChildren<'a, T, V>),
    UnsetStatement(&'a UnsetStatementChildren<'a, T, V>),
    DeclareLocalStatement(&'a DeclareLocalStatementChildren<'a, T, V>),
    UsingStatementBlockScoped(&'a UsingStatementBlockScopedChildren<'a, T, V>),
    UsingStatementFunctionScoped(&'a UsingStatementFunctionScopedChildren<'a, T, V>),
    WhileStatement(&'a WhileStatementChildren<'a, T, V>),
    IfStatement(&'a IfStatementChildren<'a, T, V>),
    ElseClause(&'a ElseClauseChildren<'a, T, V>),
    TryStatement(&'a TryStatementChildren<'a, T, V>),
    CatchClause(&'a CatchClauseChildren<'a, T, V>),
    FinallyClause(&'a FinallyClauseChildren<'a, T, V>),
    DoStatement(&'a DoStatementChildren<'a, T, V>),
    ForStatement(&'a ForStatementChildren<'a, T, V>),
    ForeachStatement(&'a ForeachStatementChildren<'a, T, V>),
    SwitchStatement(&'a SwitchStatementChildren<'a, T, V>),
    SwitchSection(&'a SwitchSectionChildren<'a, T, V>),
    SwitchFallthrough(&'a SwitchFallthroughChildren<'a, T, V>),
    CaseLabel(&'a CaseLabelChildren<'a, T, V>),
    DefaultLabel(&'a DefaultLabelChildren<'a, T, V>),
    MatchStatement(&'a MatchStatementChildren<'a, T, V>),
    MatchStatementArm(&'a MatchStatementArmChildren<'a, T, V>),
    ReturnStatement(&'a ReturnStatementChildren<'a, T, V>),
    YieldBreakStatement(&'a YieldBreakStatementChildren<'a, T, V>),
    ThrowStatement(&'a ThrowStatementChildren<'a, T, V>),
    BreakStatement(&'a BreakStatementChildren<'a, T, V>),
    ContinueStatement(&'a ContinueStatementChildren<'a, T, V>),
    EchoStatement(&'a EchoStatementChildren<'a, T, V>),
    ConcurrentStatement(&'a ConcurrentStatementChildren<'a, T, V>),
    SimpleInitializer(&'a SimpleInitializerChildren<'a, T, V>),
    AnonymousClass(&'a AnonymousClassChildren<'a, T, V>),
    AnonymousFunction(&'a AnonymousFunctionChildren<'a, T, V>),
    AnonymousFunctionUseClause(&'a AnonymousFunctionUseClauseChildren<'a, T, V>),
    VariablePattern(&'a VariablePatternChildren<'a, T, V>),
    ConstructorPattern(&'a ConstructorPatternChildren<'a, T, V>),
    RefinementPattern(&'a RefinementPatternChildren<'a, T, V>),
    LambdaExpression(&'a LambdaExpressionChildren<'a, T, V>),
    LambdaSignature(&'a LambdaSignatureChildren<'a, T, V>),
    CastExpression(&'a CastExpressionChildren<'a, T, V>),
    ScopeResolutionExpression(&'a ScopeResolutionExpressionChildren<'a, T, V>),
    MemberSelectionExpression(&'a MemberSelectionExpressionChildren<'a, T, V>),
    SafeMemberSelectionExpression(&'a SafeMemberSelectionExpressionChildren<'a, T, V>),
    EmbeddedMemberSelectionExpression(&'a EmbeddedMemberSelectionExpressionChildren<'a, T, V>),
    YieldExpression(&'a YieldExpressionChildren<'a, T, V>),
    PrefixUnaryExpression(&'a PrefixUnaryExpressionChildren<'a, T, V>),
    PostfixUnaryExpression(&'a PostfixUnaryExpressionChildren<'a, T, V>),
    BinaryExpression(&'a BinaryExpressionChildren<'a, T, V>),
    IsExpression(&'a IsExpressionChildren<'a, T, V>),
    AsExpression(&'a AsExpressionChildren<'a, T, V>),
    NullableAsExpression(&'a NullableAsExpressionChildren<'a, T, V>),
    UpcastExpression(&'a UpcastExpressionChildren<'a, T, V>),
    ConditionalExpression(&'a ConditionalExpressionChildren<'a, T, V>),
    EvalExpression(&'a EvalExpressionChildren<'a, T, V>),
    IssetExpression(&'a IssetExpressionChildren<'a, T, V>),
    NameofExpression(&'a NameofExpressionChildren<'a, T, V>),
    FunctionCallExpression(&'a FunctionCallExpressionChildren<'a, T, V>),
    FunctionPointerExpression(&'a FunctionPointerExpressionChildren<'a, T, V>),
    ParenthesizedExpression(&'a ParenthesizedExpressionChildren<'a, T, V>),
    BracedExpression(&'a BracedExpressionChildren<'a, T, V>),
    ETSpliceExpression(&'a ETSpliceExpressionChildren<'a, T, V>),
    EmbeddedBracedExpression(&'a EmbeddedBracedExpressionChildren<'a, T, V>),
    ListExpression(&'a ListExpressionChildren<'a, T, V>),
    CollectionLiteralExpression(&'a CollectionLiteralExpressionChildren<'a, T, V>),
    ObjectCreationExpression(&'a ObjectCreationExpressionChildren<'a, T, V>),
    ConstructorCall(&'a ConstructorCallChildren<'a, T, V>),
    DarrayIntrinsicExpression(&'a DarrayIntrinsicExpressionChildren<'a, T, V>),
    DictionaryIntrinsicExpression(&'a DictionaryIntrinsicExpressionChildren<'a, T, V>),
    KeysetIntrinsicExpression(&'a KeysetIntrinsicExpressionChildren<'a, T, V>),
    VarrayIntrinsicExpression(&'a VarrayIntrinsicExpressionChildren<'a, T, V>),
    VectorIntrinsicExpression(&'a VectorIntrinsicExpressionChildren<'a, T, V>),
    ElementInitializer(&'a ElementInitializerChildren<'a, T, V>),
    SubscriptExpression(&'a SubscriptExpressionChildren<'a, T, V>),
    EmbeddedSubscriptExpression(&'a EmbeddedSubscriptExpressionChildren<'a, T, V>),
    AwaitableCreationExpression(&'a AwaitableCreationExpressionChildren<'a, T, V>),
    XHPChildrenDeclaration(&'a XHPChildrenDeclarationChildren<'a, T, V>),
    XHPChildrenParenthesizedList(&'a XHPChildrenParenthesizedListChildren<'a, T, V>),
    XHPCategoryDeclaration(&'a XHPCategoryDeclarationChildren<'a, T, V>),
    XHPEnumType(&'a XHPEnumTypeChildren<'a, T, V>),
    XHPLateinit(&'a XHPLateinitChildren<'a, T, V>),
    XHPRequired(&'a XHPRequiredChildren<'a, T, V>),
    XHPClassAttributeDeclaration(&'a XHPClassAttributeDeclarationChildren<'a, T, V>),
    XHPClassAttribute(&'a XHPClassAttributeChildren<'a, T, V>),
    XHPSimpleClassAttribute(&'a XHPSimpleClassAttributeChildren<'a, T, V>),
    XHPSimpleAttribute(&'a XHPSimpleAttributeChildren<'a, T, V>),
    XHPSpreadAttribute(&'a XHPSpreadAttributeChildren<'a, T, V>),
    XHPOpen(&'a XHPOpenChildren<'a, T, V>),
    XHPExpression(&'a XHPExpressionChildren<'a, T, V>),
    XHPClose(&'a XHPCloseChildren<'a, T, V>),
    TypeConstant(&'a TypeConstantChildren<'a, T, V>),
    VectorTypeSpecifier(&'a VectorTypeSpecifierChildren<'a, T, V>),
    KeysetTypeSpecifier(&'a KeysetTypeSpecifierChildren<'a, T, V>),
    TupleTypeExplicitSpecifier(&'a TupleTypeExplicitSpecifierChildren<'a, T, V>),
    VarrayTypeSpecifier(&'a VarrayTypeSpecifierChildren<'a, T, V>),
    FunctionCtxTypeSpecifier(&'a FunctionCtxTypeSpecifierChildren<'a, T, V>),
    TypeParameter(&'a TypeParameterChildren<'a, T, V>),
    TypeConstraint(&'a TypeConstraintChildren<'a, T, V>),
    ContextConstraint(&'a ContextConstraintChildren<'a, T, V>),
    DarrayTypeSpecifier(&'a DarrayTypeSpecifierChildren<'a, T, V>),
    DictionaryTypeSpecifier(&'a DictionaryTypeSpecifierChildren<'a, T, V>),
    ClosureTypeSpecifier(&'a ClosureTypeSpecifierChildren<'a, T, V>),
    ClosureParameterTypeSpecifier(&'a ClosureParameterTypeSpecifierChildren<'a, T, V>),
    TypeRefinement(&'a TypeRefinementChildren<'a, T, V>),
    TypeInRefinement(&'a TypeInRefinementChildren<'a, T, V>),
    CtxInRefinement(&'a CtxInRefinementChildren<'a, T, V>),
    ClassnameTypeSpecifier(&'a ClassnameTypeSpecifierChildren<'a, T, V>),
    ClassArgsTypeSpecifier(&'a ClassArgsTypeSpecifierChildren<'a, T, V>),
    FieldSpecifier(&'a FieldSpecifierChildren<'a, T, V>),
    FieldInitializer(&'a FieldInitializerChildren<'a, T, V>),
    ShapeTypeSpecifier(&'a ShapeTypeSpecifierChildren<'a, T, V>),
    ShapeExpression(&'a ShapeExpressionChildren<'a, T, V>),
    TupleExpression(&'a TupleExpressionChildren<'a, T, V>),
    GenericTypeSpecifier(&'a GenericTypeSpecifierChildren<'a, T, V>),
    NullableTypeSpecifier(&'a NullableTypeSpecifierChildren<'a, T, V>),
    LikeTypeSpecifier(&'a LikeTypeSpecifierChildren<'a, T, V>),
    SoftTypeSpecifier(&'a SoftTypeSpecifierChildren<'a, T, V>),
    AttributizedSpecifier(&'a AttributizedSpecifierChildren<'a, T, V>),
    ReifiedTypeArgument(&'a ReifiedTypeArgumentChildren<'a, T, V>),
    TypeArguments(&'a TypeArgumentsChildren<'a, T, V>),
    TypeParameters(&'a TypeParametersChildren<'a, T, V>),
    TupleTypeSpecifier(&'a TupleTypeSpecifierChildren<'a, T, V>),
    UnionTypeSpecifier(&'a UnionTypeSpecifierChildren<'a, T, V>),
    IntersectionTypeSpecifier(&'a IntersectionTypeSpecifierChildren<'a, T, V>),
    ErrorSyntax(&'a ErrorSyntaxChildren<'a, T, V>),
    ListItem(&'a ListItemChildren<'a, T, V>),
    EnumClassLabelExpression(&'a EnumClassLabelExpressionChildren<'a, T, V>),
    ModuleDeclaration(&'a ModuleDeclarationChildren<'a, T, V>),
    ModuleExports(&'a ModuleExportsChildren<'a, T, V>),
    ModuleImports(&'a ModuleImportsChildren<'a, T, V>),
    ModuleMembershipDeclaration(&'a ModuleMembershipDeclarationChildren<'a, T, V>),
    PackageExpression(&'a PackageExpressionChildren<'a, T, V>),
}

#[derive(Debug, Clone)]
pub struct EndOfFileChildren<'a, T, V> {
    pub token: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ScriptChildren<'a, T, V> {
    pub declarations: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct QualifiedNameChildren<'a, T, V> {
    pub parts: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ModuleNameChildren<'a, T, V> {
    pub parts: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SimpleTypeSpecifierChildren<'a, T, V> {
    pub specifier: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct LiteralExpressionChildren<'a, T, V> {
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixedStringExpressionChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub str: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixedCodeExpressionChildren<'a, T, V> {
    pub prefix: Syntax<'a, T, V>,
    pub left_backtick: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
    pub right_backtick: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VariableExpressionChildren<'a, T, V> {
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PipeVariableExpressionChildren<'a, T, V> {
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FileAttributeSpecificationChildren<'a, T, V> {
    pub left_double_angle: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub attributes: Syntax<'a, T, V>,
    pub right_double_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub base: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub use_clauses: Syntax<'a, T, V>,
    pub enumerators: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumUseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub names: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumeratorChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumClassDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub enum_keyword: Syntax<'a, T, V>,
    pub class_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub base: Syntax<'a, T, V>,
    pub extends: Syntax<'a, T, V>,
    pub extends_list: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub elements: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumClassEnumeratorChildren<'a, T, V> {
    pub modifiers: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AliasDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub module_kw_opt: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub generic_parameter: Syntax<'a, T, V>,
    pub constraint: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ContextAliasDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub generic_parameter: Syntax<'a, T, V>,
    pub as_constraint: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub context: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CaseTypeDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub case_keyword: Syntax<'a, T, V>,
    pub type_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub generic_parameter: Syntax<'a, T, V>,
    pub as_: Syntax<'a, T, V>,
    pub bounds: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub variants: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CaseTypeVariantChildren<'a, T, V> {
    pub bar: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PropertyDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub declarators: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PropertyDeclaratorChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceDeclarationChildren<'a, T, V> {
    pub header: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceDeclarationHeaderChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceBodyChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub declarations: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceEmptyBodyChildren<'a, T, V> {
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceUseDeclarationChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub kind: Syntax<'a, T, V>,
    pub clauses: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceGroupUseDeclarationChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub kind: Syntax<'a, T, V>,
    pub prefix: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub clauses: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceUseClauseChildren<'a, T, V> {
    pub clause_kind: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub as_: Syntax<'a, T, V>,
    pub alias: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub declaration_header: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionDeclarationHeaderChildren<'a, T, V> {
    pub modifiers: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameter_list: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub parameter_list: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub contexts: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub readonly_return: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub where_clause: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ContextsChildren<'a, T, V> {
    pub left_bracket: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct WhereClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub constraints: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct WhereConstraintChildren<'a, T, V> {
    pub left_type: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_type: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MethodishDeclarationChildren<'a, T, V> {
    pub attribute: Syntax<'a, T, V>,
    pub function_decl_header: Syntax<'a, T, V>,
    pub function_body: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MethodishTraitResolutionChildren<'a, T, V> {
    pub attribute: Syntax<'a, T, V>,
    pub function_decl_header: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassishDeclarationChildren<'a, T, V> {
    pub attribute: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub xhp: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameters: Syntax<'a, T, V>,
    pub extends_keyword: Syntax<'a, T, V>,
    pub extends_list: Syntax<'a, T, V>,
    pub implements_keyword: Syntax<'a, T, V>,
    pub implements_list: Syntax<'a, T, V>,
    pub where_clause: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassishBodyChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub elements: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TraitUseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub names: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct RequireClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub kind: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub type_specifier: Syntax<'a, T, V>,
    pub declarators: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstantDeclaratorChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub modifiers: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub type_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameters: Syntax<'a, T, V>,
    pub type_constraints: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub type_specifier: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ContextConstDeclarationChildren<'a, T, V> {
    pub modifiers: Syntax<'a, T, V>,
    pub const_keyword: Syntax<'a, T, V>,
    pub ctx_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameters: Syntax<'a, T, V>,
    pub constraint: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub ctx_list: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DecoratedExpressionChildren<'a, T, V> {
    pub decorator: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ParameterDeclarationChildren<'a, T, V> {
    pub attribute: Syntax<'a, T, V>,
    pub visibility: Syntax<'a, T, V>,
    pub call_convention: Syntax<'a, T, V>,
    pub readonly: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub default_value: Syntax<'a, T, V>,
    pub parameter_end: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VariadicParameterChildren<'a, T, V> {
    pub call_convention: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub ellipsis: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct OldAttributeSpecificationChildren<'a, T, V> {
    pub left_double_angle: Syntax<'a, T, V>,
    pub attributes: Syntax<'a, T, V>,
    pub right_double_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributeSpecificationChildren<'a, T, V> {
    pub attributes: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributeChildren<'a, T, V> {
    pub at: Syntax<'a, T, V>,
    pub attribute_name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct InclusionExpressionChildren<'a, T, V> {
    pub require: Syntax<'a, T, V>,
    pub filename: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct InclusionDirectiveChildren<'a, T, V> {
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CompoundStatementChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub statements: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ExpressionStatementChildren<'a, T, V> {
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MarkupSectionChildren<'a, T, V> {
    pub hashbang: Syntax<'a, T, V>,
    pub suffix: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MarkupSuffixChildren<'a, T, V> {
    pub less_than_question: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct UnsetStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub variables: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DeclareLocalStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub variable: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct UsingStatementBlockScopedChildren<'a, T, V> {
    pub await_keyword: Syntax<'a, T, V>,
    pub using_keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub expressions: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct UsingStatementFunctionScopedChildren<'a, T, V> {
    pub await_keyword: Syntax<'a, T, V>,
    pub using_keyword: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct WhileStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub condition: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct IfStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub condition: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub statement: Syntax<'a, T, V>,
    pub else_clause: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ElseClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub statement: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TryStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub compound_statement: Syntax<'a, T, V>,
    pub catch_clauses: Syntax<'a, T, V>,
    pub finally_clause: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CatchClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub variable: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FinallyClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DoStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
    pub while_keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub condition: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ForStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
    pub first_semicolon: Syntax<'a, T, V>,
    pub control: Syntax<'a, T, V>,
    pub second_semicolon: Syntax<'a, T, V>,
    pub end_of_loop: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ForeachStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub collection: Syntax<'a, T, V>,
    pub await_keyword: Syntax<'a, T, V>,
    pub as_: Syntax<'a, T, V>,
    pub key: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub sections: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchSectionChildren<'a, T, V> {
    pub labels: Syntax<'a, T, V>,
    pub statements: Syntax<'a, T, V>,
    pub fallthrough: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchFallthroughChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CaseLabelChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DefaultLabelChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MatchStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub arms: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MatchStatementArmChildren<'a, T, V> {
    pub pattern: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ReturnStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct YieldBreakStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub break_: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ThrowStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct BreakStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ContinueStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EchoStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub expressions: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConcurrentStatementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub statement: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SimpleInitializerChildren<'a, T, V> {
    pub equal: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousClassChildren<'a, T, V> {
    pub class_keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub argument_list: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub extends_keyword: Syntax<'a, T, V>,
    pub extends_list: Syntax<'a, T, V>,
    pub implements_keyword: Syntax<'a, T, V>,
    pub implements_list: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousFunctionChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub async_keyword: Syntax<'a, T, V>,
    pub function_keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub parameters: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub ctx_list: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub readonly_return: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub use_: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousFunctionUseClauseChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub variables: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VariablePatternChildren<'a, T, V> {
    pub variable: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstructorPatternChildren<'a, T, V> {
    pub constructor: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct RefinementPatternChildren<'a, T, V> {
    pub variable: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub specifier: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct LambdaExpressionChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub async_: Syntax<'a, T, V>,
    pub signature: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct LambdaSignatureChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub parameters: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub contexts: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub readonly_return: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CastExpressionChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
    pub operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ScopeResolutionExpressionChildren<'a, T, V> {
    pub qualifier: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct MemberSelectionExpressionChildren<'a, T, V> {
    pub object: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SafeMemberSelectionExpressionChildren<'a, T, V> {
    pub object: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedMemberSelectionExpressionChildren<'a, T, V> {
    pub object: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct YieldExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixUnaryExpressionChildren<'a, T, V> {
    pub operator: Syntax<'a, T, V>,
    pub operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PostfixUnaryExpressionChildren<'a, T, V> {
    pub operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct BinaryExpressionChildren<'a, T, V> {
    pub left_operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct IsExpressionChildren<'a, T, V> {
    pub left_operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AsExpressionChildren<'a, T, V> {
    pub left_operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NullableAsExpressionChildren<'a, T, V> {
    pub left_operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct UpcastExpressionChildren<'a, T, V> {
    pub left_operand: Syntax<'a, T, V>,
    pub operator: Syntax<'a, T, V>,
    pub right_operand: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConditionalExpressionChildren<'a, T, V> {
    pub test: Syntax<'a, T, V>,
    pub question: Syntax<'a, T, V>,
    pub consequence: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub alternative: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EvalExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub argument: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct IssetExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub argument_list: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NameofExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub target: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionCallExpressionChildren<'a, T, V> {
    pub receiver: Syntax<'a, T, V>,
    pub type_args: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub argument_list: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionPointerExpressionChildren<'a, T, V> {
    pub receiver: Syntax<'a, T, V>,
    pub type_args: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ParenthesizedExpressionChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct BracedExpressionChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ETSpliceExpressionChildren<'a, T, V> {
    pub dollar: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedBracedExpressionChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ListExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CollectionLiteralExpressionChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub initializers: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ObjectCreationExpressionChildren<'a, T, V> {
    pub new_keyword: Syntax<'a, T, V>,
    pub object: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstructorCallChildren<'a, T, V> {
    pub type_: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub argument_list: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DarrayIntrinsicExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub explicit_type: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DictionaryIntrinsicExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub explicit_type: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct KeysetIntrinsicExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub explicit_type: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VarrayIntrinsicExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub explicit_type: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VectorIntrinsicExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub explicit_type: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ElementInitializerChildren<'a, T, V> {
    pub key: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SubscriptExpressionChildren<'a, T, V> {
    pub receiver: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub index: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedSubscriptExpressionChildren<'a, T, V> {
    pub receiver: Syntax<'a, T, V>,
    pub left_bracket: Syntax<'a, T, V>,
    pub index: Syntax<'a, T, V>,
    pub right_bracket: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AwaitableCreationExpressionChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub async_: Syntax<'a, T, V>,
    pub compound_statement: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPChildrenDeclarationChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPChildrenParenthesizedListChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub xhp_children: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPCategoryDeclarationChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub categories: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPEnumTypeChildren<'a, T, V> {
    pub like: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub values: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPLateinitChildren<'a, T, V> {
    pub at: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPRequiredChildren<'a, T, V> {
    pub at: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPClassAttributeDeclarationChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub attributes: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPClassAttributeChildren<'a, T, V> {
    pub type_: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub initializer: Syntax<'a, T, V>,
    pub required: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSimpleClassAttributeChildren<'a, T, V> {
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSimpleAttributeChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSpreadAttributeChildren<'a, T, V> {
    pub left_brace: Syntax<'a, T, V>,
    pub spread_operator: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPOpenChildren<'a, T, V> {
    pub left_angle: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub attributes: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPExpressionChildren<'a, T, V> {
    pub open: Syntax<'a, T, V>,
    pub body: Syntax<'a, T, V>,
    pub close: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPCloseChildren<'a, T, V> {
    pub left_angle: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstantChildren<'a, T, V> {
    pub left_type: Syntax<'a, T, V>,
    pub separator: Syntax<'a, T, V>,
    pub right_type: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VectorTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct KeysetTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleTypeExplicitSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct VarrayTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionCtxTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub variable: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeParameterChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub reified: Syntax<'a, T, V>,
    pub variance: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub param_params: Syntax<'a, T, V>,
    pub constraints: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstraintChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ContextConstraintChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub ctx_list: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DarrayTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub key: Syntax<'a, T, V>,
    pub comma: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct DictionaryTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClosureTypeSpecifierChildren<'a, T, V> {
    pub outer_left_paren: Syntax<'a, T, V>,
    pub readonly_keyword: Syntax<'a, T, V>,
    pub function_keyword: Syntax<'a, T, V>,
    pub inner_left_paren: Syntax<'a, T, V>,
    pub parameter_list: Syntax<'a, T, V>,
    pub inner_right_paren: Syntax<'a, T, V>,
    pub contexts: Syntax<'a, T, V>,
    pub colon: Syntax<'a, T, V>,
    pub readonly_return: Syntax<'a, T, V>,
    pub return_type: Syntax<'a, T, V>,
    pub outer_right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClosureParameterTypeSpecifierChildren<'a, T, V> {
    pub call_convention: Syntax<'a, T, V>,
    pub readonly: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeRefinementChildren<'a, T, V> {
    pub type_: Syntax<'a, T, V>,
    pub keyword: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub members: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeInRefinementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameters: Syntax<'a, T, V>,
    pub constraints: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct CtxInRefinementChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub type_parameters: Syntax<'a, T, V>,
    pub constraints: Syntax<'a, T, V>,
    pub equal: Syntax<'a, T, V>,
    pub ctx_list: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassnameTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassArgsTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_angle: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
    pub trailing_comma: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FieldSpecifierChildren<'a, T, V> {
    pub question: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct FieldInitializerChildren<'a, T, V> {
    pub name: Syntax<'a, T, V>,
    pub arrow: Syntax<'a, T, V>,
    pub value: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ShapeTypeSpecifierChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub fields: Syntax<'a, T, V>,
    pub ellipsis: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ShapeExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub fields: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub left_paren: Syntax<'a, T, V>,
    pub items: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct GenericTypeSpecifierChildren<'a, T, V> {
    pub class_type: Syntax<'a, T, V>,
    pub argument_list: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct NullableTypeSpecifierChildren<'a, T, V> {
    pub question: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct LikeTypeSpecifierChildren<'a, T, V> {
    pub tilde: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct SoftTypeSpecifierChildren<'a, T, V> {
    pub at: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributizedSpecifierChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ReifiedTypeArgumentChildren<'a, T, V> {
    pub reified: Syntax<'a, T, V>,
    pub type_: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeArgumentsChildren<'a, T, V> {
    pub left_angle: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeParametersChildren<'a, T, V> {
    pub left_angle: Syntax<'a, T, V>,
    pub parameters: Syntax<'a, T, V>,
    pub right_angle: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleTypeSpecifierChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct UnionTypeSpecifierChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct IntersectionTypeSpecifierChildren<'a, T, V> {
    pub left_paren: Syntax<'a, T, V>,
    pub types: Syntax<'a, T, V>,
    pub right_paren: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ErrorSyntaxChildren<'a, T, V> {
    pub error: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ListItemChildren<'a, T, V> {
    pub item: Syntax<'a, T, V>,
    pub separator: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumClassLabelExpressionChildren<'a, T, V> {
    pub qualifier: Syntax<'a, T, V>,
    pub hash: Syntax<'a, T, V>,
    pub expression: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ModuleDeclarationChildren<'a, T, V> {
    pub attribute_spec: Syntax<'a, T, V>,
    pub new_keyword: Syntax<'a, T, V>,
    pub module_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub exports: Syntax<'a, T, V>,
    pub imports: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ModuleExportsChildren<'a, T, V> {
    pub exports_keyword: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub exports: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ModuleImportsChildren<'a, T, V> {
    pub imports_keyword: Syntax<'a, T, V>,
    pub left_brace: Syntax<'a, T, V>,
    pub imports: Syntax<'a, T, V>,
    pub right_brace: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct ModuleMembershipDeclarationChildren<'a, T, V> {
    pub module_keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
    pub semicolon: Syntax<'a, T, V>,
}

#[derive(Debug, Clone)]
pub struct PackageExpressionChildren<'a, T, V> {
    pub keyword: Syntax<'a, T, V>,
    pub name: Syntax<'a, T, V>,
}



impl<'a, T, V> SyntaxVariant<'a, T, V> {
    pub fn iter_children(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        SyntaxChildrenIterator {
            syntax: &self,
            index: 0,
            index_back: 0,
        }
    }
}

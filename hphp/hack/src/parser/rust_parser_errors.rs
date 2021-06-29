// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use itertools::Itertools;
use lazy_static::lazy_static;
use regex::Regex;
use std::collections::BTreeMap;
use std::{matches, str::FromStr};
use strum;
use strum::IntoEnumIterator;
use strum_macros::{Display, EnumIter, EnumString, IntoStaticStr};

use naming_special_names_rust as sn;

use hash::{HashMap, HashSet};
use oxidized::parser_options::ParserOptions;
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    lexable_token::LexableToken,
    syntax::SyntaxValueType,
    syntax_by_ref::{
        positioned_syntax::PositionedSyntax,
        positioned_token::PositionedToken,
        positioned_value::PositionedValue,
        syntax::Syntax,
        syntax_variant_generated::{ListItemChildren, SyntaxVariant, SyntaxVariant::*},
    },
    syntax_error::{self as errors, Error, ErrorType, SyntaxError},
    syntax_trait::SyntaxTrait,
    syntax_tree::SyntaxTree,
    token_kind::TokenKind,
};

use hh_autoimport_rust as hh_autoimport;

#[derive(Clone, PartialEq, Debug)]
struct Location {
    start_offset: usize,
    end_offset: usize,
}

#[derive(Clone, PartialEq, Debug)]
enum NameKind {
    NameUse,         // `use` construct
    NameDef,         // definition e.g. `class` or `trait`
    NameImplicitUse, // implicit `use` e.g. HH type in type hint
}

enum LvalType {
    LvalTypeNone,
    LvalTypeNonFinal,
    LvalTypeFinal,
    LvalTypeNonFinalInout,
}

use LvalType::*;

enum BinopAllowsAwaitInPositions {
    BinopAllowAwaitBoth,
    BinopAllowAwaitLeft,
    BinopAllowAwaitRight,
    BinopAllowAwaitNone,
}

#[allow(dead_code)] // Deprecated is currently unused
#[derive(Eq, PartialEq)]
enum FeatureStatus {
    Unstable,
    Preview,
    Migration,
    Deprecated,
    // TODO: add other modes like "Advanced" or "Deprecated" if necessary.
    // Those are just variants of "Preview" for the runtime's sake, though,
    // and likely only need to be distinguished in the lint rule rather than here
}

#[derive(
    Clone,
    Copy,
    EnumIter,
    EnumString,
    Eq,
    Display,
    Hash,
    IntoStaticStr,
    PartialEq
)]
#[strum(serialize_all = "snake_case")]
enum UnstableFeatures {
    // TODO: rename this from unstable to something else
    UnionIntersectionTypeHints,
    ClassLevelWhere,
    ExpressionTrees,
    EnumClassLabel,
    IFC,
    Readonly,
    Modules,
    ContextAliasDeclaration,
    ClassConstDefault,
}
impl UnstableFeatures {
    // Preview features are allowed to run in prod. This function decides
    // whether the feature is considered Unstable or Previw.
    fn get_feature_status(&self) -> FeatureStatus {
        use FeatureStatus::*;
        match self {
            UnstableFeatures::UnionIntersectionTypeHints => Unstable,
            UnstableFeatures::ClassLevelWhere => Unstable,
            UnstableFeatures::ExpressionTrees => Unstable,
            UnstableFeatures::EnumClassLabel => Preview,
            UnstableFeatures::IFC => Unstable,
            UnstableFeatures::Readonly => Preview,
            UnstableFeatures::Modules => Unstable,
            UnstableFeatures::ContextAliasDeclaration => Unstable,
            UnstableFeatures::ClassConstDefault => Unstable,
        }
    }
}

use BinopAllowsAwaitInPositions::*;
use NameKind::*;

#[derive(Clone, Debug)]
struct FirstUseOrDef {
    location: Location,
    kind: NameKind,
    name: String,
    global: bool,
}

#[derive(PartialEq)]
enum NamespaceType {
    Unspecified,
    Bracketed(Location),
    Unbracketed(Location),
}

use NamespaceType::*;

// TODO: is there a more Rust idiomatic way to write this?
#[derive(Clone, Debug)]
enum Strmap<X> {
    YesCase(HashMap<String, X>),
    NoCase(HashMap<String, X>),
}

impl<X> Strmap<X> {
    fn mem(&self, k: &str) -> bool {
        match &self {
            NoCase(m) => m.contains_key(&k.to_ascii_lowercase()),
            YesCase(m) => m.contains_key(k),
        }
    }

    fn add(&mut self, k: &str, v: X) {
        match self {
            NoCase(m) => m.insert(k.to_ascii_lowercase(), v),
            YesCase(m) => m.insert(k.to_string(), v),
        };
    }

    fn get(&self, k: &str) -> Option<&X> {
        match &self {
            NoCase(m) => m.get(&k.to_ascii_lowercase()),
            YesCase(m) => m.get(k),
        }
    }

    fn filter<F>(self, f: F) -> Self
    where
        F: Fn(&X) -> bool,
    {
        match self {
            NoCase(m) => NoCase(m.into_iter().filter(|(_, x)| f(x)).collect()),
            YesCase(m) => YesCase(m.into_iter().filter(|(_, x)| f(x)).collect()),
        }
    }
}

use crate::Strmap::*;

fn empty_trait_require_clauses() -> Strmap<TokenKind> {
    NoCase(HashMap::default())
}

#[derive(Clone, Debug)]
struct UsedNames {
    classes: Strmap<FirstUseOrDef>,    // NoCase
    namespaces: Strmap<FirstUseOrDef>, // NoCase
    functions: Strmap<FirstUseOrDef>,  // NoCase
    constants: Strmap<FirstUseOrDef>,  // YesCase
    attributes: Strmap<FirstUseOrDef>, // YesCase
}

impl UsedNames {
    fn empty() -> Self {
        Self {
            classes: NoCase(HashMap::default()),
            namespaces: NoCase(HashMap::default()),
            functions: NoCase(HashMap::default()),
            constants: YesCase(HashMap::default()),
            attributes: YesCase(HashMap::default()),
        }
    }
}

struct Context<'a, Syntax> {
    pub active_classish: Option<&'a Syntax>,
    pub active_methodish: Option<&'a Syntax>,
    pub active_callable: Option<&'a Syntax>,
    pub active_callable_attr_spec: Option<&'a Syntax>,
    pub active_const: Option<&'a Syntax>,
    pub active_unstable_features: HashSet<UnstableFeatures>,
    pub active_expression_tree: bool,
}

// TODO: why can't this be auto-derived?
impl<'a, Syntax> std::clone::Clone for Context<'a, Syntax> {
    fn clone(&self) -> Self {
        Self {
            active_classish: self.active_classish,
            active_methodish: self.active_methodish,
            active_callable: self.active_callable,
            active_callable_attr_spec: self.active_callable_attr_spec,
            active_const: self.active_const,
            active_unstable_features: self.active_unstable_features.clone(),
            active_expression_tree: self.active_expression_tree,
        }
    }
}

struct Env<'a, Syntax, SyntaxTree> {
    parser_options: ParserOptions,
    syntax_tree: &'a SyntaxTree,
    text: IndexedSourceText<'a>,
    context: Context<'a, Syntax>,
    hhvm_compat_mode: bool,
    hhi_mode: bool,
    codegen: bool,
}

impl<'a, Syntax, SyntaxTree> Env<'a, Syntax, SyntaxTree> {
    fn is_hhvm_compat(&self) -> bool {
        self.hhvm_compat_mode
    }

    fn is_typechecker(&self) -> bool {
        !self.codegen
    }

    fn is_hhi_mode(&self) -> bool {
        self.hhi_mode
    }
}

const GLOBAL_NAMESPACE_NAME: &str = "\\";

fn combine_names(n1: &str, n2: &str) -> String {
    let has_leading_slash = n2.starts_with('\\');
    let has_trailing_slash = n1.ends_with('\\');
    match (has_leading_slash, has_trailing_slash) {
        (true, true) => n1.to_string() + &n2[1..],
        (false, false) => n1.to_string() + "\\" + n2,
        _ => n1.to_string() + n2,
    }
}

fn make_first_use_or_def(
    is_method: bool,
    kind: NameKind,
    location: Location,
    namespace_name: &str,
    name: &str,
) -> FirstUseOrDef {
    let res = FirstUseOrDef {
        location,
        kind,
        name: combine_names(namespace_name, name),
        global: !is_method && namespace_name == GLOBAL_NAMESPACE_NAME,
    };

    res
}
struct ParserErrors<'a, Token, Value, State> {
    phantom: std::marker::PhantomData<(*const Token, *const Value, *const State)>,

    env: Env<'a, Syntax<'a, Token, Value>, SyntaxTree<'a, Syntax<'a, Token, Value>, State>>,
    errors: Vec<SyntaxError>,
    parents: Vec<S<'a, Token, Value>>,

    trait_require_clauses: Strmap<TokenKind>,
    is_in_concurrent_block: bool,
    names: UsedNames,
    // Named (not anonymous) namespaces that the current expression is enclosed within.
    nested_namespaces: Vec<S<'a, Token, Value>>,
    namespace_type: NamespaceType,
    namespace_name: String,
}

type S<'a, T, V> = &'a Syntax<'a, T, V>;

// TODO: why do we need :'a everywhere?
impl<'a, Token: 'a, Value: 'a, State: 'a> ParserErrors<'a, Token, Value, State>
where
    Syntax<'a, Token, Value>: SyntaxTrait,
    Token: LexableToken + std::fmt::Debug,
    Value: SyntaxValueType<Token> + std::fmt::Debug,
    State: Clone,
{
    fn new(
        env: Env<'a, Syntax<'a, Token, Value>, SyntaxTree<'a, Syntax<'a, Token, Value>, State>>,
    ) -> Self {
        Self {
            env,
            errors: vec![],
            parents: vec![],
            names: UsedNames::empty(),
            trait_require_clauses: empty_trait_require_clauses(),
            namespace_name: GLOBAL_NAMESPACE_NAME.to_string(),
            namespace_type: Unspecified,
            is_in_concurrent_block: false,
            nested_namespaces: vec![],
            phantom: std::marker::PhantomData,
        }
    }

    fn text(&self, node: S<'a, Token, Value>) -> &'a str {
        node.extract_text(self.env.syntax_tree.text())
            .expect("<text_extraction_failure>")
    }

    fn make_location(s: S<'a, Token, Value>, e: S<'a, Token, Value>) -> Location {
        let start_offset = Self::start_offset(s);
        let end_offset = Self::end_offset(e);
        Location {
            start_offset,
            end_offset,
        }
    }

    fn make_location_of_node(n: S<'a, Token, Value>) -> Location {
        Self::make_location(n, n)
    }

    fn start_offset(n: S<'a, Token, Value>) -> usize {
        // TODO: this logic should be moved to SyntaxTrait::position, when implemented
        n.leading_start_offset() + n.leading_width()
    }

    fn end_offset(n: S<'a, Token, Value>) -> usize {
        // TODO: this logic should be moved to SyntaxTrait::position, when implemented
        let w = n.width();
        n.leading_start_offset() + n.leading_width() + w
    }

    fn get_short_name_from_qualified_name<'b>(name: &'b str, alias: &'b str) -> &'b str {
        if !alias.is_empty() {
            return alias;
        }
        match name.rfind('\\') {
            Some(i) => &name[i + 1..],
            None => name,
        }
    }

    // Turns a syntax node into a list of nodes; if it is a separated syntax
    // list then the separators are filtered from the resulting list.
    fn syntax_to_list(
        include_separators: bool,
        node: S<'a, Token, Value>,
    ) -> impl DoubleEndedIterator<Item = S<'a, Token, Value>> {
        use itertools::Either::{Left, Right};
        use std::iter::{empty, once};
        let on_list_item = move |x: &'a ListItemChildren<Token, Value>| {
            if include_separators {
                vec![&x.item, &x.separator].into_iter()
            } else {
                vec![&x.item].into_iter()
            }
        };
        match &node.children {
            Missing => Left(Left(empty())),
            SyntaxList(s) => Left(Right(
                s.iter()
                    .map(move |x| {
                        match &x.children {
                            ListItem(x) => Left(on_list_item(x)),
                            _ => Right(once(x)),
                        }
                    })
                    .flatten(),
            )),
            ListItem(x) => Right(Left(on_list_item(x))),
            _ => Right(Right(once(node))),
        }
    }

    fn syntax_to_list_no_separators(
        node: S<'a, Token, Value>,
    ) -> impl DoubleEndedIterator<Item = S<'a, Token, Value>> {
        Self::syntax_to_list(false, node)
    }

    fn syntax_to_list_with_separators(
        node: S<'a, Token, Value>,
    ) -> impl DoubleEndedIterator<Item = S<'a, Token, Value>> {
        Self::syntax_to_list(true, node)
    }

    fn assert_last_in_list<F>(
        assert_fun: F,
        node: S<'a, Token, Value>,
    ) -> Option<S<'a, Token, Value>>
    where
        F: Fn(S<'a, Token, Value>) -> bool,
    {
        let mut iter = Self::syntax_to_list_no_separators(node);
        iter.next_back();
        iter.find(|x| assert_fun(*x))
    }

    fn enable_unstable_feature(&mut self, node: S<'a, Token, Value>, arg: S<'a, Token, Value>) {
        let error_invalid_argument = |self_: &mut Self, message| {
            self_.errors.push(Self::make_error_from_node(
                arg,
                errors::invalid_use_of_enable_unstable_feature(message),
            ))
        };

        match &arg.children {
            LiteralExpression(x) => {
                let text = self.text(&x.expression);
                match UnstableFeatures::from_str(escaper::unquote_str(text)) {
                    Ok(feature) => {
                        if !self.env.parser_options.po_allow_unstable_features
                            && !self.env.is_hhi_mode()
                            && feature.get_feature_status() == FeatureStatus::Unstable
                        {
                            self.errors.push(Self::make_error_from_node(
                                node,
                                errors::cannot_enable_unstable_feature(
                                    format!(
                                        "{} is unstable and unstable features are disabled",
                                        text
                                    )
                                    .as_str(),
                                ),
                            ))
                        } else {
                            self.env.context.active_unstable_features.insert(feature);
                        }
                    }
                    Err(_) => error_invalid_argument(
                        self,
                        format!(
                            "there is no feature named {}.\nAvailable features are:\n\t{}",
                            text,
                            UnstableFeatures::iter().join("\n\t")
                        )
                        .as_str(),
                    ),
                }
            }
            _ => error_invalid_argument(self, "this is not a literal string expression"),
        };
    }

    fn check_can_use_feature(&mut self, node: S<'a, Token, Value>, feature: &UnstableFeatures) {
        let parser_options = &self.env.parser_options;
        let enabled = match feature {
            UnstableFeatures::UnionIntersectionTypeHints => {
                parser_options.tco_union_intersection_type_hints
            }
            UnstableFeatures::ClassLevelWhere => parser_options.po_enable_class_level_where_clauses,
            UnstableFeatures::IFC => {
                let file_path = format!("/{}", self.env.text.source_text().file_path().path_str());
                parser_options
                    .tco_ifc_enabled
                    .iter()
                    .any(|prefix| file_path.find(prefix) == Some(0))
            }
            _ => false,
        } || self.env.context.active_unstable_features.contains(feature);
        if !enabled {
            self.errors.push(Self::make_error_from_node(
                node,
                errors::cannot_use_feature(feature.into()),
            ))
        }
    }

    fn attr_spec_to_node_list(
        node: S<'a, Token, Value>,
    ) -> impl DoubleEndedIterator<Item = S<'a, Token, Value>> {
        use itertools::Either::{Left, Right};
        let f = |attrs| Left(Self::syntax_to_list_no_separators(attrs));
        match &node.children {
            AttributeSpecification(x) => f(&x.attributes),
            OldAttributeSpecification(x) => f(&x.attributes),
            FileAttributeSpecification(x) => f(&x.attributes),
            _ => Right(std::iter::empty()),
        }
    }

    fn attr_constructor_call(node: S<'a, Token, Value>) -> &'a SyntaxVariant<Token, Value> {
        match &node.children {
            ConstructorCall(_) => &node.children,
            Attribute(x) => &x.attribute_name.children,
            _ => &Missing,
        }
    }

    fn attr_name(&self, node: S<'a, Token, Value>) -> Option<&'a str> {
        if let ConstructorCall(x) = Self::attr_constructor_call(node) {
            Some(self.text(&x.type_))
        } else {
            None
        }
    }

    fn attr_args(
        &self,
        node: S<'a, Token, Value>,
    ) -> Option<impl DoubleEndedIterator<Item = S<'a, Token, Value>>> {
        if let ConstructorCall(x) = Self::attr_constructor_call(node) {
            Some(Self::syntax_to_list_no_separators(&x.argument_list))
        } else {
            None
        }
    }

    fn attribute_specification_contains(&self, node: S<'a, Token, Value>, name: &str) -> bool {
        match &node.children {
            AttributeSpecification(_)
            | OldAttributeSpecification(_)
            | FileAttributeSpecification(_) => {
                Self::attr_spec_to_node_list(node).any(|node| self.attr_name(node) == Some(name))
            }
            _ => false,
        }
    }

    fn methodish_contains_attribute(&self, node: S<'a, Token, Value>, attribute: &str) -> bool {
        match &node.children {
            MethodishDeclaration(x) => {
                self.attribute_specification_contains(&x.attribute, attribute)
            }
            _ => false,
        }
    }

    fn is_decorated_expression<F>(node: S<'a, Token, Value>, f: F) -> bool
    where
        F: Fn(S<'a, Token, Value>) -> bool,
    {
        match &node.children {
            DecoratedExpression(x) => f(&x.decorator),
            _ => false,
        }
    }

    fn test_decorated_expression_child<F>(node: S<'a, Token, Value>, f: F) -> bool
    where
        F: Fn(S<'a, Token, Value>) -> bool,
    {
        match &node.children {
            DecoratedExpression(x) => f(&x.expression),
            _ => false,
        }
    }

    fn is_variadic_expression(node: S<'a, Token, Value>) -> bool {
        Self::is_decorated_expression(node, |x| x.is_ellipsis())
            || Self::test_decorated_expression_child(node, &Self::is_variadic_expression)
    }

    fn is_double_variadic(node: S<'a, Token, Value>) -> bool {
        Self::is_decorated_expression(node, |x| x.is_ellipsis())
            && Self::test_decorated_expression_child(node, &Self::is_variadic_expression)
    }

    fn is_variadic_parameter_variable(node: S<'a, Token, Value>) -> bool {
        // TODO: This shouldn't be a decorated *expression* because we are not
        // expecting an expression at all. We're expecting a declaration.
        Self::is_variadic_expression(node)
    }

    fn is_variadic_parameter_declaration(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            VariadicParameter(_) => true,
            ParameterDeclaration(x) => Self::is_variadic_parameter_variable(&x.name),
            _ => false,
        }
    }
    fn misplaced_variadic_param(param: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        Self::assert_last_in_list(&Self::is_variadic_parameter_declaration, param)
    }
    fn misplaced_variadic_arg(args: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        Self::assert_last_in_list(&Self::is_variadic_expression, args)
    }
    // If a list ends with a variadic parameter followed by a comma, return it
    fn ends_with_variadic_comma(params: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        let mut iter = Self::syntax_to_list_with_separators(params).rev();
        let y = iter.next();
        let x = iter.next();
        match (x, y) {
            (Some(x), Some(y)) if Self::is_variadic_parameter_declaration(x) && y.is_comma() => {
                Some(y)
            }
            _ => None,
        }
    }

    // Extract variadic parameter from a parameter list
    fn variadic_param(params: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        Self::syntax_to_list_with_separators(params)
            .find(|&x| Self::is_variadic_parameter_declaration(x))
    }

    fn is_parameter_with_default_value(param: S<'a, Token, Value>) -> bool {
        match &param.children {
            ParameterDeclaration(x) => !x.default_value.is_missing(),
            _ => false,
        }
    }

    // test a node is a syntaxlist and that the list contains an element
    // satisfying a given predicate
    fn list_contains_predicate<P>(p: P, node: S<'a, Token, Value>) -> bool
    where
        P: Fn(S<'a, Token, Value>) -> bool,
    {
        if let SyntaxList(lst) = &node.children {
            lst.iter().any(p)
        } else {
            false
        }
    }

    fn list_first_duplicate_token(node: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        if let SyntaxList(lst) = &node.children {
            let mut seen = BTreeMap::new();
            for node in lst.iter().rev() {
                if let Token(t) = &node.children {
                    if let Some(dup) = seen.insert(t.kind(), node) {
                        return Some(dup);
                    }
                }
            }
        }
        None
    }

    fn is_empty_list_or_missing(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            SyntaxList(x) if x.is_empty() => true,
            Missing => true,
            _ => false,
        }
    }

    fn token_kind(node: S<'a, Token, Value>) -> Option<TokenKind> {
        if let Token(t) = &node.children {
            return Some(t.kind());
        }
        None
    }

    // Helper function for common code pattern
    fn is_token_kind(node: S<'a, Token, Value>, kind: TokenKind) -> bool {
        Self::token_kind(node) == Some(kind)
    }

    fn active_classish_kind(&self) -> Option<TokenKind> {
        self.env
            .context
            .active_classish
            .and_then(|x| match &x.children {
                ClassishDeclaration(cd) => Self::token_kind(&cd.keyword),
                _ => None,
            })
    }

    fn modifiers_of_function_decl_header_exn(node: S<'a, Token, Value>) -> S<'a, Token, Value> {
        match &node.children {
            FunctionDeclarationHeader(x) => &x.modifiers,
            _ => panic!("expected to get FunctionDeclarationHeader"),
        }
    }

    fn get_modifiers_of_declaration(node: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        match &node.children {
            MethodishDeclaration(x) => Some(Self::modifiers_of_function_decl_header_exn(
                &x.function_decl_header,
            )),
            FunctionDeclaration(x) => Some(Self::modifiers_of_function_decl_header_exn(
                &x.declaration_header,
            )),
            PropertyDeclaration(x) => Some(&x.modifiers),
            ConstDeclaration(x) => Some(&x.modifiers),
            TypeConstDeclaration(x) => Some(&x.modifiers),
            ClassishDeclaration(x) => Some(&x.modifiers),
            TraitUseAliasItem(x) => Some(&x.modifiers),
            _ => None,
        }
    }

    // tests whether the node's modifiers contain one that satisfies [p]
    fn has_modifier_helper<P>(p: P, node: S<'a, Token, Value>) -> bool
    where
        P: Fn(S<'a, Token, Value>) -> bool,
    {
        match Self::get_modifiers_of_declaration(node) {
            Some(x) => Self::list_contains_predicate(p, x),
            None => false,
        }
    }

    // does the node contain the Abstract keyword in its modifiers
    fn has_modifier_abstract(node: S<'a, Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_abstract(), node)
    }

    // does the node contain the Static keyword in its modifiers
    fn has_modifier_static(node: S<'a, Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_static(), node)
    }

    // does the node contain the Private keyword in its modifiers
    fn has_modifier_private(node: S<'a, Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_private(), node)
    }

    fn get_modifier_final(modifiers: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        Self::syntax_to_list_no_separators(modifiers).find(|x| x.is_final())
    }

    fn is_visibility(x: S<'a, Token, Value>) -> bool {
        x.is_public() || x.is_private() || x.is_protected()
    }

    fn contains_async_not_last(mods: S<'a, Token, Value>) -> bool {
        let mut mod_list = Self::syntax_to_list_no_separators(mods);
        match mod_list.next_back() {
            Some(x) if x.is_async() => false,
            _ => mod_list.any(|x| x.is_async()),
        }
    }

    fn has_static<F>(&self, node: S<'a, Token, Value>, f: F) -> bool
    where
        F: Fn(S<'a, Token, Value>) -> bool,
    {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                let label = &node.name;
                f(label)
                    && self
                        .env
                        .context
                        .active_methodish
                        .iter()
                        .any(|&x| Self::has_modifier_static(x))
            }
            _ => false,
        }
    }

    fn has_this(&self) -> bool {
        if !self.is_in_active_class_scope() {
            return false;
        }
        match self.env.context.active_methodish {
            Some(x) if Self::has_modifier_static(x) => false,
            _ => true,
        }
    }

    fn is_clone(&self, label: S<'a, Token, Value>) -> bool {
        self.text(label).eq_ignore_ascii_case(sn::members::__CLONE)
    }

    fn class_constructor_has_static(&self, node: S<'a, Token, Value>) -> bool {
        self.has_static(node, |x| x.is_construct())
    }

    fn clone_cannot_be_static(&self, node: S<'a, Token, Value>) -> bool {
        self.has_static(node, |x| self.is_clone(x))
    }

    fn promoted_params(
        params: impl DoubleEndedIterator<Item = S<'a, Token, Value>>,
    ) -> impl DoubleEndedIterator<Item = S<'a, Token, Value>> {
        params.filter(|node| match &node.children {
            ParameterDeclaration(x) => !x.visibility.is_missing(),
            _ => false,
        })
    }

    // Given a function declaration header, confirm that it is NOT a constructor
    // and that the header containing it has visibility modifiers in parameters
    fn class_non_constructor_has_visibility_param(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                let params = Self::syntax_to_list_no_separators(&node.parameter_list);
                (!&node.name.is_construct()) && Self::promoted_params(params).next().is_some()
            }
            _ => false,
        }
    }

    fn class_constructor_has_tparams(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                node.name.is_construct() && !node.type_parameter_list.is_missing()
            }
            _ => false,
        }
    }

    // Don't allow a promoted parameter in a constructor if the class
    // already has a property with the same name. Return the clashing name found.
    fn class_constructor_param_promotion_clash(&self, node: S<'a, Token, Value>) -> Option<&str> {
        use itertools::Either::{Left, Right};
        let class_elts = |node: Option<S<'a, Token, Value>>| match node.map(|x| &x.children) {
            Some(ClassishDeclaration(cd)) => match &cd.body.children {
                ClassishBody(cb) => Left(Self::syntax_to_list_no_separators(&cb.elements)),
                _ => Right(std::iter::empty()),
            },
            _ => Right(std::iter::empty()),
        };

        // A property declaration may include multiple property names:
        // public int $x, $y;
        let prop_names = |elt: S<'a, Token, Value>| match &elt.children {
            PropertyDeclaration(x) => Left(
                Self::syntax_to_list_no_separators(&x.declarators).filter_map(|decl| {
                    match &decl.children {
                        PropertyDeclarator(x) => Some(self.text(&x.name)),
                        _ => None,
                    }
                }),
            ),
            _ => Right(std::iter::empty()),
        };

        let param_name = |p: S<'a, Token, Value>| match &p.children {
            ParameterDeclaration(x) => Some(self.text(&x.name)),
            _ => None,
        };

        match &node.children {
            FunctionDeclarationHeader(node) if node.name.is_construct() => {
                let class_var_names: Vec<_> = class_elts(self.env.context.active_classish)
                    .map(|x| prop_names(x))
                    .flatten()
                    .collect();
                let params = Self::syntax_to_list_no_separators(&node.parameter_list);
                let mut promoted_param_names = Self::promoted_params(params).filter_map(param_name);
                promoted_param_names.find(|name| class_var_names.contains(name))
            }
            _ => None,
        }
    }

    // Ban parameter promotion in abstract constructors.
    fn abstract_class_constructor_has_visibility_param(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                let label = &node.name;
                let params = Self::syntax_to_list_no_separators(&node.parameter_list);
                label.is_construct()
                    && Self::list_contains_predicate(|x| x.is_abstract(), &node.modifiers)
                    && Self::promoted_params(params).next().is_some()
            }
            _ => false,
        }
    }

    // Ban parameter promotion in interfaces and traits.
    fn interface_or_trait_has_visibility_param(&self, node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                let is_interface_or_trait =
                    self.env
                        .context
                        .active_classish
                        .map_or(false, |parent_classish| match &parent_classish.children {
                            ClassishDeclaration(cd) => {
                                let kind = Self::token_kind(&cd.keyword);
                                kind == Some(TokenKind::Interface) || kind == Some(TokenKind::Trait)
                            }
                            _ => false,
                        });

                let params = Self::syntax_to_list_no_separators(&node.parameter_list);
                Self::promoted_params(params).next().is_some() && is_interface_or_trait
            }
            _ => false,
        }
    }

    // check that a constructor is type annotated
    fn class_constructor_has_non_void_type(&self, node: S<'a, Token, Value>) -> bool {
        if !self.env.is_typechecker() {
            false
        } else {
            match &node.children {
                FunctionDeclarationHeader(node) => {
                    let label = &node.name;
                    let type_ano = &node.type_;
                    let function_colon = &node.colon;
                    let is_missing = type_ano.is_missing() && function_colon.is_missing();
                    let is_void = match &type_ano.children {
                        SimpleTypeSpecifier(spec) => spec.specifier.is_void(),
                        _ => false,
                    };
                    label.is_construct() && !(is_missing || is_void)
                }
                _ => false,
            }
        }
    }

    fn unsupported_magic_method_errors(&mut self, node: S<'a, Token, Value>) {
        if let FunctionDeclarationHeader(x) = &node.children {
            let name = self.text(&x.name).to_ascii_lowercase();
            let unsupported = sn::members::UNSUPPORTED_MAP.get(&name);

            if let Some(unsupported) = unsupported {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::unsupported_magic_method(unsupported),
                ));
            }
        }
    }

    fn async_magic_method(&self, node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(node) => {
                let name = self.text(&node.name).to_ascii_lowercase();
                match name {
                    _ if name.eq_ignore_ascii_case(sn::members::__DISPOSE_ASYNC) => false,
                    _ if sn::members::AS_LOWERCASE_SET.contains(&name) => {
                        Self::list_contains_predicate(|x| x.is_async(), &node.modifiers)
                    }
                    _ => false,
                }
            }
            _ => false,
        }
    }

    fn clone_takes_no_arguments(&self, node: S<'a, Token, Value>) -> bool {
        match &node.children {
            FunctionDeclarationHeader(x) => {
                let mut params = Self::syntax_to_list_no_separators(&x.parameter_list);
                self.is_clone(&x.name) && params.next().is_some()
            }
            _ => false,
        }
    }

    // whether a methodish decl has body
    fn methodish_has_body(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            MethodishDeclaration(syntax) => !syntax.function_body.is_missing(),
            _ => false,
        }
    }

    // whether a methodish decl is native
    fn methodish_is_native(&self, node: S<'a, Token, Value>) -> bool {
        self.methodish_contains_attribute(node, "__Native")
    }

    // By checking the third parent of a methodish node, tests whether the methodish
    // node is inside an interface.
    fn methodish_inside_interface(&self) -> bool {
        self.env
            .context
            .active_classish
            .iter()
            .any(|parent_classish| match &parent_classish.children {
                ClassishDeclaration(cd) => {
                    Self::token_kind(&cd.keyword) == Some(TokenKind::Interface)
                }
                _ => false,
            })
    }

    // Test whether node is a non-abstract method without a body and not native.
    // Here node is the methodish node
    // And methods inside interfaces are inherently considered abstract *)
    fn methodish_non_abstract_without_body_not_native(&self, node: S<'a, Token, Value>) -> bool {
        let non_abstract =
            !(Self::has_modifier_abstract(node) || self.methodish_inside_interface());
        let not_has_body = !Self::methodish_has_body(node);
        let not_native = !self.methodish_is_native(node);
        let not_hhi = !self.env.is_hhi_mode();

        not_hhi && non_abstract && not_has_body && not_native
    }

    // Test whether node is a method that is both abstract and private
    fn methodish_abstract_conflict_with_private(node: S<'a, Token, Value>) -> bool {
        let is_abstract = Self::has_modifier_abstract(node);
        let has_private = Self::has_modifier_private(node);
        is_abstract && has_private
    }

    fn using_statement_function_scoped_is_legal(&self) -> bool {
        // using is allowed in the toplevel, and also in toplevel async blocks
        let len = self.parents.len();
        if len < 3 {
            return false;
        }
        match (self.parents.get(len - 2), self.parents.get(len - 3)) {
            (
                Some(Syntax {
                    children: CompoundStatement(_),
                    ..
                }),
                Some(x),
            ) => match &x.children {
                FunctionDeclaration(_)
                | MethodishDeclaration(_)
                | AnonymousFunction(_)
                | LambdaExpression(_)
                | AwaitableCreationExpression(_) => true,
                _ => false,
            },
            _ => false,
        }
    }

    fn make_error_from_nodes(
        child: Option<SyntaxError>,
        start_node: S<'a, Token, Value>,
        end_node: S<'a, Token, Value>,
        error_type: ErrorType,
        error: errors::Error,
    ) -> SyntaxError {
        let s = Self::start_offset(start_node);
        let e = Self::end_offset(end_node);
        SyntaxError::make_with_child_and_type(child, s, e, error_type, error)
    }

    fn make_error_from_node(node: S<'a, Token, Value>, error: errors::Error) -> SyntaxError {
        Self::make_error_from_nodes(None, node, node, ErrorType::ParseError, error)
    }

    fn make_error_from_node_with_type(
        node: S<'a, Token, Value>,
        error: errors::Error,
        error_type: ErrorType,
    ) -> SyntaxError {
        Self::make_error_from_nodes(None, node, node, error_type, error)
    }

    fn is_invalid_xhp_attr_enum_item_literal(literal_expression: S<'a, Token, Value>) -> bool {
        if let Token(t) = &literal_expression.children {
            match t.kind() {
                TokenKind::DecimalLiteral
                | TokenKind::SingleQuotedStringLiteral
                | TokenKind::DoubleQuotedStringLiteral => false,
                _ => true,
            }
        } else {
            true
        }
    }

    fn is_invalid_xhp_attr_enum_item(node: S<'a, Token, Value>) -> bool {
        if let LiteralExpression(x) = &node.children {
            Self::is_invalid_xhp_attr_enum_item_literal(&x.expression)
        } else {
            true
        }
    }

    fn xhp_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            XHPEnumType(enum_type) => {
                if self.env.is_typechecker() && enum_type.values.is_missing() {
                    self.errors.push(Self::make_error_from_node(
                        &enum_type.values,
                        errors::error2055,
                    ))
                } else if self.env.is_typechecker() {
                    Self::syntax_to_list_no_separators(&enum_type.values)
                        .filter(|&x| Self::is_invalid_xhp_attr_enum_item(x))
                        .for_each(|item| {
                            self.errors
                                .push(Self::make_error_from_node(item, errors::error2063))
                        })
                }
            }
            XHPExpression(x) => {
                if let XHPOpen(xhp_open) = &x.open.children {
                    if let XHPClose(xhp_close) = &x.close.children {
                        let open_tag = self.text(&xhp_open.name);
                        let close_tag = self.text(&xhp_close.name);
                        if open_tag != close_tag {
                            self.errors.push(Self::make_error_from_node(
                                node,
                                errors::error2070(&open_tag, &close_tag),
                            ))
                        }
                    }
                }
            }
            _ => {}
        }
    }

    fn invalid_modifier_errors<F>(&mut self, decl_name: &str, node: S<'a, Token, Value>, ok: F)
    where
        F: Fn(TokenKind) -> bool,
    {
        if let Some(modifiers) = Self::get_modifiers_of_declaration(node) {
            for modifier in Self::syntax_to_list_no_separators(modifiers) {
                if let Some(kind) = Self::token_kind(modifier) {
                    if kind == TokenKind::Readonly {
                        self.check_can_use_feature(modifier, &UnstableFeatures::Readonly)
                    }
                    if !ok(kind) {
                        self.errors.push(Self::make_error_from_node(
                            modifier,
                            errors::invalid_modifier_for_declaration(
                                decl_name,
                                self.text(modifier),
                            ),
                        ));
                    }
                }
            }
            if let Some(duplicate) = Self::list_first_duplicate_token(modifiers) {
                self.errors.push(Self::make_error_from_node(
                    duplicate,
                    errors::duplicate_modifiers_for_declaration(decl_name),
                ))
            }
            if let SyntaxList(modifiers) = &modifiers.children {
                let modifiers: Vec<S<'a, Token, Value>> = modifiers
                    .iter()
                    .filter(|x: &S<'a, Token, Value>| Self::is_visibility(*x))
                    .collect();
                if modifiers.len() > 1 {
                    self.errors.push(Self::make_error_from_node(
                        modifiers.last().unwrap(),
                        errors::multiple_visibility_modifiers_for_declaration(decl_name),
                    ));
                }
            }
        }
    }

    // helper since there are so many kinds of errors
    fn produce_error<'b, F, E, X>(
        &mut self,
        check: F,
        node: &'b X,
        error: E, // closure to avoid constant premature concatenation of error strings
        error_node: S<'a, Token, Value>,
    ) where
        F: Fn(&mut Self, &'b X) -> bool,
        E: Fn() -> Error,
    {
        if check(self, node) {
            self.errors
                .push(Self::make_error_from_node(error_node, error()))
        }
    }

    // helper since there are so many kinds of errors
    fn produce_error_from_check<'b, F, E, X>(&mut self, check: F, node: &'b X, error: E)
    where
        F: Fn(&'b X) -> Option<S<'a, Token, Value>>,
        E: Fn() -> Error,
    {
        if let Some(error_node) = check(node) {
            self.errors
                .push(Self::make_error_from_node(error_node, error()))
        }
    }

    fn cant_be_classish_name(name: &str) -> bool {
        match name.to_ascii_lowercase().as_ref() {
            "callable" | "classname" | "darray" | "false" | "null" | "parent" | "self" | "this"
            | "true" | "varray" => true,
            _ => false,
        }
    }

    // Given a function_declaration_header node, returns its function_name
    // as a string opt.
    fn extract_function_name(&self, header_node: S<'a, Token, Value>) -> Option<&'a str> {
        // The '_' arm of this match will never be reached, but the type checker
        // doesn't allow a direct extraction of function_name from
        // function_declaration_header. *)
        match &header_node.children {
            FunctionDeclarationHeader(fdh) => Some(self.text(&fdh.name)),
            _ => None,
        }
    }

    // Return, as a string opt, the name of the function or method given the context *)
    fn first_parent_function_name(&self) -> Option<&str> {
        // Note: matching on either is sound because functions and/or methods cannot be nested

        match self.env.context.active_methodish {
            Some(Syntax {
                children: FunctionDeclaration(x),
                ..
            }) => self.extract_function_name(&x.declaration_header),
            Some(Syntax {
                children: MethodishDeclaration(x),
                ..
            }) => self.extract_function_name(&x.function_decl_header),
            _ => None,
        }
    }

    // Given a particular TokenKind::(Trait/Interface), tests if a given
    // classish_declaration node is both of that kind and declared abstract.
    fn is_classish_kind_declared_abstract(&self, cd_node: S<'a, Token, Value>) -> bool {
        match &cd_node.children {
            ClassishDeclaration(x)
                if Self::is_token_kind(&x.keyword, TokenKind::Trait)
                    || Self::is_token_kind(&x.keyword, TokenKind::Interface) =>
            {
                Self::list_contains_predicate(|x| x.is_abstract(), &x.modifiers)
            }
            _ => false,
        }
    }

    fn is_immediately_in_lambda(&self) -> bool {
        self.env
            .context
            .active_callable
            .map_or(false, |node| match &node.children {
                AnonymousFunction(_) | LambdaExpression(_) | AwaitableCreationExpression(_) => true,
                _ => false,
            })
    }

    // Returns the whether the current context is in an active class scope
    fn is_in_active_class_scope(&self) -> bool {
        self.env.context.active_classish.is_some()
    }

    // Returns the first ClassishDeclaration node or
    // None if there isn't one or classish_kind does not match. *)
    fn first_parent_classish_node(&self, classish_kind: TokenKind) -> Option<S<'a, Token, Value>> {
        self.env
            .context
            .active_classish
            .and_then(|node| match &node.children {
                ClassishDeclaration(cd) if Self::is_token_kind(&cd.keyword, classish_kind) => {
                    Some(node)
                }
                _ => None,
            })
    }

    // Return, as a string opt, the name of the closest enclosing classish entity in
    // the given context (not just Classes )
    fn active_classish_name(&self) -> Option<&'a str> {
        self.env.context.active_classish.and_then(|node| {
            if let ClassishDeclaration(cd) = &node.children {
                cd.name.extract_text(self.env.syntax_tree.text())
            } else {
                None
            }
        })
    }

    // Return, as a string opt, the name of the Class in the given context
    fn first_parent_class_name(&self) -> Option<&'a str> {
        self.env
            .context
            .active_classish
            .and_then(|parent_classish| {
                if let ClassishDeclaration(cd) = &parent_classish.children {
                    if Self::token_kind(&cd.keyword) == Some(TokenKind::Class) {
                        return self.active_classish_name();
                    } else {
                        return None; // This arm is never reached
                    }
                }
                return None;
            })
    }

    // Given a declaration node, returns the modifier node matching the given
    // predicate from its list of modifiers, or None if there isn't one.
    fn extract_keyword<F>(
        modifier: F,
        declaration_node: S<'a, Token, Value>,
    ) -> Option<S<'a, Token, Value>>
    where
        F: Fn(S<'a, Token, Value>) -> bool,
    {
        Self::get_modifiers_of_declaration(declaration_node).and_then(|modifiers_list| {
            Self::syntax_to_list_no_separators(modifiers_list)
                .find(|x: &S<'a, Token, Value>| modifier(*x))
        })
    }

    // Wrapper function that uses above extract_keyword function to test if node
    // contains is_abstract keyword
    fn is_abstract_declaration(declaration_node: S<'a, Token, Value>) -> bool {
        Self::extract_keyword(|x| x.is_abstract(), declaration_node).is_some()
    }

    // Tests if the immediate classish parent is an interface.
    fn is_inside_interface(&self) -> bool {
        self.first_parent_classish_node(TokenKind::Interface)
            .is_some()
    }

    fn is_abstract_and_async_method(md_node: S<'a, Token, Value>) -> bool {
        Self::is_abstract_declaration(md_node)
            && Self::extract_keyword(|x| x.is_async(), md_node).is_some()
    }

    fn is_interface_and_async_method(&self, md_node: S<'a, Token, Value>) -> bool {
        self.is_inside_interface() && Self::extract_keyword(|x| x.is_async(), md_node).is_some()
    }

    fn get_params_for_enclosing_callable(&self) -> Option<S<'a, Token, Value>> {
        let from_header = |header: S<'a, Token, Value>| match &header.children {
            FunctionDeclarationHeader(fdh) => Some(&fdh.parameter_list),
            _ => None,
        };
        self.env
            .context
            .active_callable
            .and_then(|callable| match &callable.children {
                FunctionDeclaration(x) => from_header(&x.declaration_header),
                MethodishDeclaration(x) => from_header(&x.function_decl_header),
                LambdaExpression(x) => match &x.signature.children {
                    LambdaSignature(x) => Some(&x.parameters),
                    _ => None,
                },
                _ => None,
            })
    }

    fn first_parent_function_attributes_contains(&self, name: &str) -> bool {
        let from_attr_spec = |attr_spec| {
            Self::attr_spec_to_node_list(attr_spec).any(|node| self.attr_name(node) == Some(name))
        };
        match self.env.context.active_methodish {
            Some(Syntax {
                children: FunctionDeclaration(x),
                ..
            }) => from_attr_spec(&x.attribute_spec),
            Some(Syntax {
                children: MethodishDeclaration(x),
                ..
            }) => from_attr_spec(&x.attribute),
            _ => false,
        }
    }

    fn parameter_callconv(param: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        (match &param.children {
            ParameterDeclaration(x) => Some(&x.call_convention),
            ClosureParameterTypeSpecifier(x) => Some(&x.call_convention),
            VariadicParameter(x) => Some(&x.call_convention),
            _ => None,
        })
        .filter(|node| !node.is_missing())
    }

    fn is_parameter_with_callconv(param: S<'a, Token, Value>) -> bool {
        Self::parameter_callconv(param).is_some()
    }

    fn has_inout_params(&self) -> bool {
        self.get_params_for_enclosing_callable()
            .map_or(false, |function_parameter_list| {
                Self::syntax_to_list_no_separators(function_parameter_list)
                    .any(|x| Self::is_parameter_with_callconv(x))
            })
    }

    fn is_inside_async_method(&self) -> bool {
        let from_header = |header: S<'a, Token, Value>| match &header.children {
            FunctionDeclarationHeader(fdh) => {
                Self::syntax_to_list_no_separators(&fdh.modifiers).any(|x| x.is_async())
            }
            _ => false,
        };
        self.env
            .context
            .active_callable
            .map_or(false, |node| match &node.children {
                FunctionDeclaration(x) => from_header(&x.declaration_header),
                MethodishDeclaration(x) => from_header(&x.function_decl_header),
                AnonymousFunction(x) => !x.async_keyword.is_missing(),
                LambdaExpression(x) => !x.async_.is_missing(),
                AwaitableCreationExpression(_) => true,
                _ => false,
            })
    }

    fn make_name_already_used_error(
        node: S<'a, Token, Value>,
        name: &str,
        short_name: &str,
        original_location: &Location,
        report_error: &dyn Fn(&str, &str) -> Error,
    ) -> SyntaxError {
        let name = Self::strip_ns(name);
        let original_location_error = SyntaxError::make(
            original_location.start_offset,
            original_location.end_offset,
            errors::original_definition,
        );

        let s = Self::start_offset(node);
        let e = Self::end_offset(node);
        SyntaxError::make_with_child_and_type(
            Some(original_location_error),
            s,
            e,
            ErrorType::ParseError,
            report_error(&name, short_name),
        )
    }

    fn check_type_name_reference(&mut self, name_text: &str, location: Location) {
        if hh_autoimport::is_hh_autoimport(name_text) && !self.names.classes.mem(name_text) {
            let def = make_first_use_or_def(false, NameImplicitUse, location, "HH", name_text);
            self.names.classes.add(name_text, def)
        }
    }

    fn check_type_hint(&mut self, node: S<'a, Token, Value>) {
        for x in node.iter_children() {
            self.check_type_hint(x)
        }
        let check_type_name = |self_: &mut Self, s| {
            self_.check_type_name_reference(self_.text(s), Self::make_location_of_node(node))
        };
        match &node.children {
            SimpleTypeSpecifier(x) => check_type_name(self, &x.specifier),
            GenericTypeSpecifier(x) => check_type_name(self, &x.class_type),
            _ => {}
        }
    }

    fn extract_callconv_node(node: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        match &node.children {
            ParameterDeclaration(x) => Some(&x.call_convention),
            ClosureParameterTypeSpecifier(x) => Some(&x.call_convention),
            VariadicParameter(x) => Some(&x.call_convention),
            _ => None,
        }
    }

    // Given a node, checks if it is a abstract ConstDeclaration
    fn is_abstract_const(declaration: S<'a, Token, Value>) -> bool {
        match &declaration.children {
            ConstDeclaration(_) => Self::has_modifier_abstract(declaration),
            _ => false,
        }
    }

    // Given a ConstDeclarator node, test whether it is abstract, but has an
    // initializer.
    fn constant_abstract_with_initializer(&self, init: S<'a, Token, Value>) -> bool {
        let is_abstract = match self.env.context.active_const {
            Some(p_const_declaration) if Self::is_abstract_const(p_const_declaration) => true,
            _ => false,
        };
        let has_initializer = !init.is_missing();
        is_abstract && has_initializer
    }

    // Given a node, checks if it is a concrete ConstDeclaration *)
    fn is_concrete_const(declaration: S<'a, Token, Value>) -> bool {
        match &declaration.children {
            ConstDeclaration(_) => !Self::has_modifier_abstract(declaration),
            _ => false,
        }
    }

    // Given a ConstDeclarator node, test whether it is concrete, but has no
    // initializer.
    fn constant_concrete_without_initializer(&self, init: S<'a, Token, Value>) -> bool {
        let is_concrete = match self.env.context.active_const {
            Some(p_const_declaration) if Self::is_concrete_const(p_const_declaration) => true,
            _ => false,
        };
        is_concrete && init.is_missing()
    }

    fn methodish_memoize_lsb_on_non_static(&mut self, node: S<'a, Token, Value>) {
        if self.methodish_contains_attribute(node, sn::user_attributes::MEMOIZE_LSB)
            && !Self::has_modifier_static(node)
        {
            self.errors.push(Self::make_error_from_node(
                node,
                errors::memoize_lsb_on_non_static,
            ));
        }
    }

    fn function_declaration_contains_attribute(
        &self,
        node: S<'a, Token, Value>,
        attribute: &str,
    ) -> bool {
        match &node.children {
            FunctionDeclaration(x) => {
                self.attribute_specification_contains(&x.attribute_spec, attribute)
            }
            _ => false,
        }
    }

    fn methodish_contains_memoize(&self, node: S<'a, Token, Value>) -> bool {
        self.env.is_typechecker()
            && self.is_inside_interface()
            && self.methodish_contains_attribute(node, sn::user_attributes::MEMOIZE)
    }

    fn is_ifc_attribute(&self, name: &str) -> bool {
        name == sn::user_attributes::POLICIED
            || name == sn::user_attributes::INFERFLOWS
            || name == sn::user_attributes::EXTERNAL
    }

    fn is_module_attribute(&self, name: &str) -> bool {
        name == sn::user_attributes::MODULE || name == sn::user_attributes::INTERNAL
    }

    fn check_attr_enabled(&mut self, attrs: S<'a, Token, Value>) {
        for node in Self::attr_spec_to_node_list(attrs) {
            match self.attr_name(node) {
                Some(n) => {
                    if self.is_ifc_attribute(n) {
                        self.check_can_use_feature(node, &UnstableFeatures::IFC)
                    }
                    if self.is_module_attribute(n) {
                        self.check_can_use_feature(node, &UnstableFeatures::Modules)
                    }
                }
                None => {}
            }
        }
    }

    fn function_declaration_header_memoize_lsb(&mut self) {
        if let (Some(node), None) = (
            self.env.context.active_methodish,
            self.env.context.active_classish,
        ) {
            // a function, not a method
            if self.function_declaration_contains_attribute(node, sn::user_attributes::MEMOIZE_LSB)
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::memoize_lsb_on_non_method,
                ))
            }
        }
    }

    fn is_in_enum_class(&self) -> bool {
        let active_classish = match self.env.context.active_classish {
            Some(x) => x,
            _ => return false,
        };
        if let ClassishDeclaration(cd) = &active_classish.children {
            return self.attr_spec_contains_enum_class(&cd.attribute);
        }
        return false;
    }

    fn is_in_reified_class(&self) -> bool {
        let active_classish = match self.env.context.active_classish {
            Some(x) => x,
            _ => return false,
        };
        if let ClassishDeclaration(x) = &active_classish.children {
            if let TypeParameters(x) = &x.type_parameters.children {
                return Self::syntax_to_list_no_separators(&x.parameters).any(|p| {
                    match &p.children {
                        TypeParameter(x) => !x.reified.is_missing(),
                        _ => false,
                    }
                });
            }
        };
        false
    }

    fn methodish_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            FunctionDeclarationHeader(x) => {
                let function_parameter_list = &x.parameter_list;
                let function_type = &x.type_;
                if x.readonly_return.is_readonly() {
                    self.check_can_use_feature(&x.readonly_return, &UnstableFeatures::Readonly)
                }

                self.produce_error(
                    |self_, x| Self::class_constructor_has_non_void_type(self_, x),
                    node,
                    || errors::error2018,
                    function_type,
                );

                self.produce_error(
                    |_, x| Self::class_non_constructor_has_visibility_param(x),
                    node,
                    || errors::error2010,
                    function_parameter_list,
                );

                self.produce_error(
                    |_, x| Self::class_constructor_has_tparams(x),
                    node,
                    || errors::no_generics_on_constructors,
                    &x.type_parameter_list,
                );

                if let Some(clashing_name) = self.class_constructor_param_promotion_clash(node) {
                    let class_name = self.active_classish_name().unwrap_or("");
                    let error_msg = errors::error2025(class_name, &clashing_name);
                    self.errors.push(Self::make_error_from_node(
                        function_parameter_list,
                        error_msg,
                    ))
                }

                self.produce_error(
                    |_, x| Self::abstract_class_constructor_has_visibility_param(x),
                    node,
                    || errors::error2023,
                    function_parameter_list,
                );

                self.produce_error(
                    |self_, x| Self::interface_or_trait_has_visibility_param(self_, x),
                    node,
                    || errors::error2024,
                    function_parameter_list,
                );
                self.function_declaration_header_memoize_lsb();
            }
            FunctionDeclaration(fd) => {
                let function_attrs = &fd.attribute_spec;
                self.check_attr_enabled(function_attrs);
                self.invalid_modifier_errors("Top-level functions", node, |kind| {
                    kind == TokenKind::Async
                });
            }
            MethodishDeclaration(md) => {
                let header_node = &md.function_decl_header;
                let modifiers = Self::modifiers_of_function_decl_header_exn(header_node);
                let class_name = self.active_classish_name().unwrap_or("");
                let method_name = self
                    .extract_function_name(&md.function_decl_header)
                    .unwrap_or("");
                let method_attrs = &md.attribute;
                self.check_attr_enabled(method_attrs);
                self.produce_error(
                    |self_, x| self_.methodish_contains_memoize(x),
                    node,
                    || errors::interface_with_memoize,
                    header_node,
                );
                self.produce_error(
                    |self_, x| self_.class_constructor_has_static(x),
                    header_node,
                    || errors::error2009(class_name, method_name),
                    modifiers,
                );
                self.unsupported_magic_method_errors(header_node);
                self.produce_error(
                    |self_, x| self_.async_magic_method(x),
                    header_node,
                    || errors::async_magic_method(method_name),
                    modifiers,
                );
                self.produce_error(
                    |self_, x| self_.clone_takes_no_arguments(x),
                    header_node,
                    || errors::clone_takes_no_arguments(class_name, method_name),
                    modifiers,
                );
                self.produce_error(
                    |self_, x| self_.clone_cannot_be_static(x),
                    header_node,
                    || errors::clone_cannot_be_static(class_name, method_name),
                    modifiers,
                );
                self.invalid_modifier_errors("Methods", node, |kind| {
                    kind == TokenKind::Abstract
                        || kind == TokenKind::Final
                        || kind == TokenKind::Static
                        || kind == TokenKind::Private
                        || kind == TokenKind::Protected
                        || kind == TokenKind::Public
                        || kind == TokenKind::Async
                        || kind == TokenKind::Readonly
                });

                if self.is_inside_interface() {
                    self.invalid_modifier_errors("Interface methods", node, |kind| {
                        kind != TokenKind::Final && kind != TokenKind::Abstract
                    });
                };

                let fun_semicolon = &md.semicolon;

                self.produce_error(
                    |self_, x| self_.methodish_non_abstract_without_body_not_native(x),
                    node,
                    || errors::error2015,
                    fun_semicolon,
                );
                self.produce_error(
                    |_, x| Self::methodish_abstract_conflict_with_private(x),
                    node,
                    || errors::error2016(class_name, method_name),
                    modifiers,
                );

                if let Some(modifier) = Self::get_modifier_final(modifiers) {
                    self.produce_error(
                        |_, x| Self::has_modifier_abstract(x),
                        node,
                        || errors::error2019(class_name, method_name),
                        modifier,
                    );
                }

                self.methodish_memoize_lsb_on_non_static(node);
                let async_annotation =
                    Self::extract_keyword(|x| x.is_async(), node).unwrap_or(node);

                self.produce_error(
                    |self_, x| self_.is_interface_and_async_method(x),
                    node,
                    || errors::error2046("a method in an interface"),
                    async_annotation,
                );

                self.produce_error(
                    |_, x| Self::is_abstract_and_async_method(x),
                    node,
                    || errors::error2046("an `abstract` method"),
                    async_annotation,
                );

                if self.env.is_typechecker() {
                    self.produce_error(
                        |_, x| Self::contains_async_not_last(x),
                        modifiers,
                        || errors::async_not_last,
                        modifiers,
                    );
                }
            }
            _ => {}
        }
    }

    fn is_hashbang(&self, node: S<'a, Token, Value>) -> bool {
        let text = self.text(node);
        lazy_static! {
            static ref RE: Regex = Regex::new("^#!.*\n").unwrap();
        }
        text.lines().nth(1).is_none() && // only one line
        RE.is_match(text)
    }

    fn is_in_construct_method(&self) -> bool {
        if self.is_immediately_in_lambda() {
            false
        } else {
            self.first_parent_function_name()
                .map_or(false, |s| s.eq_ignore_ascii_case(sn::members::__CONSTRUCT))
        }
    }

    // If a variadic parameter has a default value, return it
    fn variadic_param_with_default_value(
        params: S<'a, Token, Value>,
    ) -> Option<S<'a, Token, Value>> {
        Self::variadic_param(params).filter(|x| Self::is_parameter_with_default_value(x))
    }

    // If a variadic parameter is marked inout, return it
    fn variadic_param_with_callconv(params: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        Self::variadic_param(params).filter(|x| Self::is_parameter_with_callconv(x))
    }

    // If an inout parameter has a default, return the default
    fn param_with_callconv_has_default(node: S<'a, Token, Value>) -> Option<S<'a, Token, Value>> {
        match &node.children {
            ParameterDeclaration(x)
                if Self::is_parameter_with_callconv(node)
                    && Self::is_parameter_with_default_value(&node) =>
            {
                Some(&x.default_value)
            }
            _ => None,
        }
    }

    fn params_errors(&mut self, params: S<'a, Token, Value>) {
        self.produce_error_from_check(&Self::ends_with_variadic_comma, params, || {
            errors::error2022
        });
        self.produce_error_from_check(&Self::misplaced_variadic_param, params, || {
            errors::error2021
        });

        self.produce_error_from_check(&Self::variadic_param_with_default_value, params, || {
            errors::error2065
        });

        self.produce_error_from_check(&Self::variadic_param_with_callconv, params, || {
            errors::error2073
        });
    }

    fn decoration_errors(&mut self, node: S<'a, Token, Value>) {
        self.produce_error(
            |_, x| Self::is_double_variadic(x),
            node,
            || errors::double_variadic,
            node,
        );
    }

    fn check_parameter_ifc(&mut self, node: S<'a, Token, Value>) {
        if let ParameterDeclaration(x) = &node.children {
            let attr = &x.attribute;
            if self.attribute_specification_contains(attr, sn::user_attributes::EXTERNAL) {
                self.check_can_use_feature(attr, &UnstableFeatures::IFC);
            }
        }
    }

    fn check_parameter_readonly(&mut self, node: S<'a, Token, Value>) {
        if let ParameterDeclaration(x) = &node.children {
            if x.readonly.is_readonly() {
                self.check_can_use_feature(&x.readonly, &UnstableFeatures::Readonly);
            }
        }
    }

    fn does_unop_create_write(token_kind: Option<TokenKind>) -> bool {
        token_kind.map_or(false, |x| {
            matches!(x, TokenKind::PlusPlus | TokenKind::MinusMinus)
        })
    }

    fn does_decorator_create_write(token_kind: Option<TokenKind>) -> bool {
        matches!(token_kind, Some(TokenKind::Inout))
    }

    fn node_lval_type<'b>(
        node: S<'a, Token, Value>,
        parents: &'b [S<'a, Token, Value>],
    ) -> LvalType {
        let is_in_final_lval_position = |mut node, parents: &'b [S<'a, Token, Value>]| {
            for &parent in parents.iter().rev() {
                match &parent.children {
                    SyntaxList(_) | ListItem(_) => {
                        node = parent;
                        continue;
                    }
                    ExpressionStatement(_) => return true,
                    ForStatement(x)
                        if node as *const _ == &x.initializer as *const _
                            || node as *const _ == &x.end_of_loop as *const _ =>
                    {
                        return true;
                    }
                    UsingStatementFunctionScoped(x)
                        if node as *const _ == &x.expression as *const _ =>
                    {
                        return true;
                    }
                    UsingStatementBlockScoped(x)
                        if node as *const _ == &x.expressions as *const _ =>
                    {
                        return true;
                    }
                    _ => return false,
                }
            }
            false
        };
        let get_arg_call_node_with_parents = |mut node, parents: &'b [S<'a, Token, Value>]| {
            for i in (0..parents.len()).rev() {
                let parent = parents[i];
                match &parent.children {
                    SyntaxList(_) | ListItem(_) => {
                        node = parent;
                        continue;
                    }
                    FunctionCallExpression(x)
                        if node as *const _ == &x.argument_list as *const _ =>
                    {
                        if i == 0 {
                            // probably unreachable, but just in case to avoid crashing on 0-1
                            return Some((parent, &parents[0..0]));
                        }
                        let grandparent = parents.get(i - 1).unwrap();
                        return match &grandparent.children {
                            PrefixUnaryExpression(x)
                                if Self::token_kind(&x.operator) == Some(TokenKind::At) =>
                            {
                                Some((grandparent, &parents[..i - 1]))
                            }
                            _ => Some((parent, &parents[..i])),
                        };
                    }
                    _ => return None,
                }
            }
            None
        };

        let lval_ness_of_function_arg_for_inout =
            |node, parents| match get_arg_call_node_with_parents(node, parents) {
                None => LvalTypeNone,
                Some((call_node, parents)) => {
                    if is_in_final_lval_position(call_node, parents) {
                        return LvalTypeFinal;
                    }
                    match parents.last() {
                        None => LvalTypeNonFinalInout,
                        Some(parent) => match &parent.children {
                            BinaryExpression(x)
                                if call_node as *const _ == &x.right_operand as *const _
                                    && Self::does_binop_create_write_on_left(Self::token_kind(
                                        &x.operator,
                                    )) =>
                            {
                                if is_in_final_lval_position(parent, &parents[..parents.len() - 1])
                                {
                                    LvalTypeFinal
                                } else {
                                    LvalTypeNonFinalInout
                                }
                            }
                            _ => LvalTypeNonFinalInout,
                        },
                    }
                }
            };

        let unary_expression_operator = |x: S<'a, Token, Value>| match &x.children {
            PrefixUnaryExpression(x) => &x.operator,
            PostfixUnaryExpression(x) => &x.operator,
            _ => panic!("expected expression operator"),
        };

        let unary_expression_operand = |x: S<'a, Token, Value>| match &x.children {
            PrefixUnaryExpression(x) => &x.operand,
            PostfixUnaryExpression(x) => &x.operand,
            _ => panic!("expected expression operator"),
        };

        if let Some(next_node) = parents.last() {
            let parents = &parents[..parents.len() - 1];
            match &next_node.children {
                DecoratedExpression(x)
                    if node as *const _ == &x.expression as *const _
                        && Self::does_decorator_create_write(Self::token_kind(&x.decorator)) =>
                {
                    lval_ness_of_function_arg_for_inout(next_node, parents)
                }
                PrefixUnaryExpression(_) | PostfixUnaryExpression(_)
                    if node as *const _ == unary_expression_operand(next_node) as *const _
                        && Self::does_unop_create_write(Self::token_kind(
                            unary_expression_operator(next_node),
                        )) =>
                {
                    if is_in_final_lval_position(next_node, parents) {
                        LvalTypeFinal
                    } else {
                        LvalTypeNonFinal
                    }
                }
                BinaryExpression(x)
                    if node as *const _ == &x.left_operand as *const _
                        && Self::does_binop_create_write_on_left(Self::token_kind(&x.operator)) =>
                {
                    if is_in_final_lval_position(next_node, parents) {
                        LvalTypeFinal
                    } else {
                        LvalTypeNonFinal
                    }
                }
                ForeachStatement(x)
                    if node as *const _ == &x.key as *const _
                        || node as *const _ == &x.value as *const _ =>
                {
                    LvalTypeFinal
                }
                _ => LvalTypeNone,
            }
        } else {
            LvalTypeNone
        }
    }

    fn lval_errors(&mut self, syntax_node: S<'a, Token, Value>) {
        if self.env.parser_options.po_disable_lval_as_an_expression {
            if let LvalTypeNonFinal = Self::node_lval_type(syntax_node, &self.parents) {
                self.errors.push(Self::make_error_from_node(
                    syntax_node,
                    errors::lval_as_expression,
                ))
            }
        }
    }

    fn parameter_errors(&mut self, node: S<'a, Token, Value>) {
        let param_errors = |self_: &mut Self, params| {
            for x in Self::syntax_to_list_no_separators(params) {
                self_.check_parameter_ifc(x);
                self_.check_parameter_readonly(x);
            }
            self_.params_errors(params)
        };
        match &node.children {
            ParameterDeclaration(p) => {
                let callconv_text = self.text(Self::extract_callconv_node(node).unwrap_or(node));
                self.produce_error_from_check(&Self::param_with_callconv_has_default, node, || {
                    errors::error2074(callconv_text)
                });

                self.check_type_hint(&p.type_);
                self.check_parameter_ifc(node);
                self.check_parameter_readonly(node);

                if let Some(inout_modifier) = Self::parameter_callconv(node) {
                    if self.is_inside_async_method() {
                        self.errors.push(Self::make_error_from_node_with_type(
                            inout_modifier,
                            errors::inout_param_in_async,
                            ErrorType::RuntimeError,
                        ))
                    }
                    if self.is_in_construct_method() {
                        self.errors.push(Self::make_error_from_node(
                            inout_modifier,
                            errors::inout_param_in_construct,
                        ))
                    }
                    let in_memoize = self
                        .first_parent_function_attributes_contains(sn::user_attributes::MEMOIZE);
                    let in_memoize_lsb = self.first_parent_function_attributes_contains(
                        sn::user_attributes::MEMOIZE_LSB,
                    );

                    if (in_memoize || in_memoize_lsb) && !self.is_immediately_in_lambda() {
                        self.errors.push(Self::make_error_from_node_with_type(
                            inout_modifier,
                            errors::memoize_with_inout,
                            ErrorType::RuntimeError,
                        ))
                    }
                }
            }
            FunctionDeclarationHeader(x) => param_errors(self, &x.parameter_list),
            AnonymousFunction(x) => param_errors(self, &x.parameters),
            ClosureTypeSpecifier(x) => param_errors(self, &x.parameter_list),
            LambdaExpression(x) => {
                if let LambdaSignature(x) = &x.signature.children {
                    param_errors(self, &x.parameters)
                }
            }
            DecoratedExpression(_) => self.decoration_errors(node),
            _ => {}
        }
    }

    // Only check the functions; invalid attributes on methods (like <<__EntryPoint>>) are caught elsewhere
    fn multiple_entrypoint_attribute_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            FunctionDeclaration(f)
                if self.attribute_specification_contains(
                    &f.attribute_spec,
                    sn::user_attributes::ENTRY_POINT,
                ) =>
            {
                // Get the location of the <<...>> annotation
                let location = match &f.attribute_spec.children {
                    AttributeSpecification(x) => Self::make_location_of_node(&x.attributes),
                    OldAttributeSpecification(x) => Self::make_location_of_node(&x.attributes),
                    _ => panic!("Expected attribute specification node"),
                };
                let def = make_first_use_or_def(
                    false,
                    NameDef,
                    location,
                    &self.namespace_name,
                    sn::user_attributes::ENTRY_POINT,
                );
                match self.names.attributes.get(sn::user_attributes::ENTRY_POINT) {
                    Some(prev_def) => {
                        let (line_num, _) = self
                            .env
                            .text
                            .offset_to_position(prev_def.location.start_offset as isize);

                        let path = self.env.text.source_text().file_path().path_str();
                        let loc = String::from(path) + ":" + &line_num.to_string();
                        let err = errors::multiple_entrypoints(&loc);
                        let err_type = ErrorType::ParseError;
                        self.errors
                            .push(Self::make_error_from_node_with_type(node, err, err_type))
                    }
                    _ => {}
                };
                self.names
                    .attributes
                    .add(sn::user_attributes::ENTRY_POINT, def)
            }
            _ => {}
        }
    }

    fn redeclaration_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            FunctionDeclarationHeader(f) if !f.name.is_missing() => {
                let mut it = self.parents.iter().rev();
                let p1 = it.next();
                let _ = it.next();
                let p3 = it.next();
                let p4 = it.next();
                match (p1, p3, p4) {
                    (
                        Some(Syntax {
                            children: FunctionDeclaration(_),
                            ..
                        }),
                        Some(Syntax {
                            children: NamespaceBody(_),
                            ..
                        }),
                        _,
                    )
                    | (
                        Some(Syntax {
                            children: FunctionDeclaration(_),
                            ..
                        }),
                        _,
                        None,
                    )
                    | (
                        Some(Syntax {
                            children: MethodishDeclaration(_),
                            ..
                        }),
                        _,
                        _,
                    )
                    | (
                        Some(Syntax {
                            children: MethodishTraitResolution(_),
                            ..
                        }),
                        _,
                        _,
                    ) => {
                        let function_name: &str = self.text(&f.name);
                        let location = Self::make_location_of_node(&f.name);
                        let is_method = match p1 {
                            Some(Syntax {
                                children: MethodishDeclaration(_),
                                ..
                            }) => true,
                            _ => false,
                        };
                        let def = make_first_use_or_def(
                            is_method,
                            NameDef,
                            location,
                            &self.namespace_name,
                            &function_name,
                        );
                        match self.names.functions.get(function_name) {
                            Some(prev_def)
                                if prev_def.global == def.global && prev_def.kind == NameDef =>
                            {
                                let (line_num, _) = self
                                    .env
                                    .text
                                    .offset_to_position(prev_def.location.start_offset as isize);

                                let path = self.env.text.source_text().file_path().path_str();
                                let loc = String::from(path) + ":" + &line_num.to_string();
                                let (err, err_type) = match self.first_parent_class_name() {
                                    None => (
                                        errors::redeclaration_of_function(function_name, &loc),
                                        ErrorType::RuntimeError,
                                    ),
                                    Some(class_name) => {
                                        let full_name =
                                            String::from(class_name) + "::" + function_name;
                                        (
                                            errors::redeclaration_of_method(&full_name),
                                            ErrorType::ParseError,
                                        )
                                    }
                                };
                                self.errors
                                    .push(Self::make_error_from_node_with_type(node, err, err_type))
                            }
                            Some(prev_def) if (prev_def.kind != NameDef) => {
                                let (line_num, _) = self
                                    .env
                                    .text
                                    .offset_to_position(prev_def.location.start_offset as isize);
                                let line_num = line_num as usize;

                                self.errors.push(Self::make_name_already_used_error(
                                    &f.name,
                                    &combine_names(&self.namespace_name, &function_name),
                                    &function_name,
                                    &def.location,
                                    &|x, y| errors::declared_name_is_already_in_use(line_num, x, y),
                                ))
                            }
                            _ => {}
                        };
                        self.names.functions.add(function_name, def)
                    }
                    _ if self.env.is_typechecker() => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::decl_outside_global_scope,
                    )),
                    _ => {}
                }
            }
            _ => {}
        }
    }

    fn is_foreach_in_for(for_initializer: S<'a, Token, Value>) -> bool {
        if let Some(Syntax {
            children: ListItem(x),
            ..
        }) = for_initializer.syntax_node_to_list().next()
        {
            x.item.is_as_expression()
        } else {
            false
        }
    }

    fn statement_errors(&mut self, node: S<'a, Token, Value>) {
        let expect_colon = |colon: S<'a, Token, Value>| match &colon.children {
            Token(m) if self.env.is_typechecker() && m.kind() != TokenKind::Colon => {
                Some((colon, errors::error1020))
            }
            _ => None,
        };
        (match &node.children {
            TryStatement(x) if x.catch_clauses.is_missing() && x.finally_clause.is_missing() => {
                Some((node, errors::error2007))
            }
            UsingStatementFunctionScoped(_) if !self.using_statement_function_scoped_is_legal() => {
                Some((node, errors::using_st_function_scoped_top_level))
            }
            ForStatement(x) if Self::is_foreach_in_for(&x.initializer) => {
                Some((node, errors::for_with_as_expression))
            }
            CaseLabel(x) => expect_colon(&x.colon),

            DefaultLabel(x) => expect_colon(&x.colon),
            _ => None,
        })
        .into_iter()
        .for_each(|(error_node, error_message)| {
            self.errors
                .push(Self::make_error_from_node(error_node, error_message))
        })
    }

    fn invalid_shape_initializer_name(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            LiteralExpression(x) => {
                let is_str = match Self::token_kind(&x.expression) {
                    Some(TokenKind::SingleQuotedStringLiteral) => true,

                    // TODO: Double quoted string are only legal
                    // if they contain no encapsulated expressions.
                    Some(TokenKind::DoubleQuotedStringLiteral) => true,
                    _ => false,
                };
                if !is_str {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_shape_field_name,
                    ))
                }
            }
            ScopeResolutionExpression(_) => {}
            QualifiedName(_) => {
                if self.env.is_typechecker() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_shape_field_name,
                    ))
                }
            }
            Token(_) if node.is_name() => {
                if self.env.is_typechecker() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_shape_field_name,
                    ))
                }
            }
            _ => self.errors.push(Self::make_error_from_node(
                node,
                errors::invalid_shape_field_name,
            )),
        }
    }

    fn invalid_shape_field_check(&mut self, node: S<'a, Token, Value>) {
        if let FieldInitializer(x) = &node.children {
            self.invalid_shape_initializer_name(&x.name)
        } else {
            self.errors.push(Self::make_error_from_node(
                node,
                errors::invalid_shape_field_name,
            ))
        }
    }

    fn is_in_unyieldable_magic_method(&self) -> bool {
        self.first_parent_function_name().map_or(false, |s| {
            let s = s.to_ascii_lowercase();
            match s {
                _ if s == sn::members::__INVOKE => false,
                _ => sn::members::AS_LOWERCASE_SET.contains(&s),
            }
        })
    }

    fn check_disallowed_variables(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            VariableExpression(x) => {
                // TODO(T75820862): Allow $GLOBALS to be used as a variable name
                if self.text(&x.expression) == sn::superglobals::GLOBALS {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::globals_disallowed))
                } else if self.text(&x.expression) == sn::special_idents::THIS && !self.has_this() {
                    // If we are in the special top level debugger function, lets not check for $this since
                    // it will be properly lifted in closure convert
                    if self
                        .first_parent_function_name()
                        .map_or(true, |s| s == "include")
                    {
                        return {};
                    }
                    self.errors
                        .push(Self::make_error_from_node(node, errors::invalid_this))
                }
            }
            _ => {}
        }
    }

    fn unset_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            UnsetStatement(x) => {
                for expr in Self::syntax_to_list_no_separators(&x.variables) {
                    match &expr.children {
                        VariableExpression(x)
                            if self.text(&x.expression) == sn::special_idents::THIS =>
                        {
                            self.errors
                                .push(Self::make_error_from_node(node, errors::cannot_unset_this))
                        }
                        _ => {}
                    }
                }
                {}
            }
            _ => {}
        }
    }

    fn function_call_argument_errors(
        &mut self,
        in_constructor_call: bool,
        node: S<'a, Token, Value>,
    ) {
        if let Some(e) = match &node.children {
            DecoratedExpression(x) => {
                if let Token(token) = &x.decorator.children {
                    if token.kind() == TokenKind::Inout {
                        let expression = &x.expression;
                        match &expression.children {
                            _ if in_constructor_call => Some(errors::inout_param_in_construct),
                            VariableExpression(x)
                                if sn::superglobals::is_any_global(self.text(&x.expression))
                                    || self.text(&x.expression) == sn::special_idents::THIS =>
                            {
                                Some(errors::fun_arg_invalid_arg)
                            }
                            PipeVariableExpression(_) => Some(errors::fun_arg_invalid_arg),
                            BinaryExpression(_) => Some(errors::fun_arg_inout_set),
                            QualifiedName(_) => Some(errors::fun_arg_inout_const),
                            Token(_) if expression.is_name() => Some(errors::fun_arg_inout_const),
                            // TODO: Maybe be more descriptive in error messages
                            ScopeResolutionExpression(_)
                            | FunctionCallExpression(_)
                            | MemberSelectionExpression(_)
                            | SafeMemberSelectionExpression(_) => Some(errors::fun_arg_invalid_arg),
                            SubscriptExpression(x) => match &x.receiver.children {
                                MemberSelectionExpression(_) | ScopeResolutionExpression(_) => {
                                    Some(errors::fun_arg_invalid_arg)
                                }
                                _ => {
                                    let text = self.text(&x.receiver);
                                    if sn::superglobals::is_any_global(text) {
                                        Some(errors::fun_arg_inout_containers)
                                    } else {
                                        None
                                    }
                                }
                            },
                            _ => None,
                        }
                    } else {
                        None
                    }
                } else {
                    None
                }
            }
            _ => None,
        } {
            self.errors.push(Self::make_error_from_node(node, e))
        }
    }

    fn function_call_on_xhp_name_errors(&mut self, node: S<'a, Token, Value>) {
        let check =
            |self_: &mut Self, member_object: S<'a, Token, Value>, name: S<'a, Token, Value>| {
                if let XHPExpression(_) = &member_object.children {
                    if self_.env.is_typechecker() {
                        self_.errors.push(Self::make_error_from_node(
                            node,
                            errors::method_calls_on_xhp_expression,
                        ))
                    }
                }

                if let Token(token) = &name.children {
                    if token.kind() == TokenKind::XHPClassName {
                        self_.errors.push(Self::make_error_from_node(
                            node,
                            errors::method_calls_on_xhp_attributes,
                        ))
                    }
                }
            };
        match &node.children {
            MemberSelectionExpression(x) => check(self, &x.object, &x.name),
            SafeMemberSelectionExpression(x) => check(self, &x.object, &x.name),
            _ => {}
        }
    }

    fn no_async_before_lambda_body(&mut self, body_node: S<'a, Token, Value>) {
        if let AwaitableCreationExpression(_) = &body_node.children {
            if self.env.is_typechecker() {
                self.errors.push(Self::make_error_from_node(
                    body_node,
                    errors::no_async_before_lambda_body,
                ))
            }
        }
    }

    fn no_memoize_attribute_on_lambda(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            OldAttributeSpecification(_) | AttributeSpecification(_) => {
                for node in Self::attr_spec_to_node_list(node) {
                    match self.attr_name(node) {
                        Some(n)
                            if n == sn::user_attributes::MEMOIZE
                                || n == sn::user_attributes::MEMOIZE_LSB =>
                        {
                            self.errors
                                .push(Self::make_error_from_node(node, errors::memoize_on_lambda))
                        }

                        _ => {}
                    }
                }
            }

            _ => {}
        }
    }

    fn is_good_scope_resolution_qualifier(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            QualifiedName(_) => true,
            Token(token) => match token.kind() {
                TokenKind::XHPClassName
                | TokenKind::Name
                | TokenKind::SelfToken
                | TokenKind::Parent
                | TokenKind::Static => true,
                _ => false,
            },
            _ => false,
        }
    }

    fn new_variable_errors_(&mut self, node: S<'a, Token, Value>, inside_scope_resolution: bool) {
        match &node.children {
            SimpleTypeSpecifier(_)
            | VariableExpression(_)
            | GenericTypeSpecifier(_)
            | PipeVariableExpression(_) => {}
            SubscriptExpression(x) if x.index.is_missing() => self.errors.push(
                Self::make_error_from_node(node, errors::instanceof_missing_subscript_index),
            ),
            SubscriptExpression(x) => {
                self.new_variable_errors_(&x.receiver, inside_scope_resolution)
            }
            MemberSelectionExpression(x) => {
                if inside_scope_resolution {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::instanceof_memberselection_inside_scoperesolution,
                    ))
                } else {
                    self.new_variable_errors_(&x.object, inside_scope_resolution)
                }
            }
            ScopeResolutionExpression(x) => {
                if let Token(name) = &x.name.children {
                    if Self::is_good_scope_resolution_qualifier(&x.qualifier)
                        && name.kind() == TokenKind::Variable
                    {
                        // OK
                    } else if name.kind() == TokenKind::Variable {
                        self.new_variable_errors_(&x.qualifier, true)
                    } else {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::instanceof_invalid_scope_resolution,
                        ))
                    }
                } else {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::instanceof_invalid_scope_resolution,
                    ))
                }
            }
            _ => {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::new_unknown_node(node.kind().to_string()),
                ));
            }
        }
    }

    fn new_variable_errors(&mut self, node: S<'a, Token, Value>) {
        self.new_variable_errors_(node, false)
    }

    fn class_type_designator_errors(&mut self, node: S<'a, Token, Value>) {
        if !Self::is_good_scope_resolution_qualifier(node) {
            match &node.children {
                ParenthesizedExpression(_) => {}
                _ => self.new_variable_errors(node),
            }
        }
    }

    fn rec_walk_impl<F, X>(
        &self,
        parents: &mut Vec<S<'a, Token, Value>>,
        f: &F,
        node: S<'a, Token, Value>,
        mut acc: X,
    ) -> X
    where
        F: Fn(S<'a, Token, Value>, &Vec<S<'a, Token, Value>>, X) -> (bool, X),
    {
        let (continue_walk, new_acc) = f(node, parents, acc);
        acc = new_acc;
        if continue_walk {
            parents.push(node);
            for child in node.iter_children() {
                acc = self.rec_walk_impl(parents, f, child, acc);
            }
            parents.pop();
        }
        acc
    }

    fn rec_walk<F, X>(&self, f: F, node: S<'a, Token, Value>, acc: X) -> X
    where
        F: Fn(S<'a, Token, Value>, &Vec<S<'a, Token, Value>>, X) -> (bool, X),
    {
        self.rec_walk_impl(&mut vec![], &f, node, acc)
    }

    fn find_invalid_lval_usage(&self, node: S<'a, Token, Value>) -> Vec<SyntaxError> {
        self.rec_walk(
            |node, parents, mut acc| match &node.children {
                AnonymousFunction(_) | LambdaExpression(_) | AwaitableCreationExpression(_) => {
                    (false, acc)
                }
                _ => {
                    match Self::node_lval_type(node, parents) {
                        LvalTypeFinal | LvalTypeNone => {}
                        LvalTypeNonFinalInout | LvalTypeNonFinal => {
                            acc.push(Self::make_error_from_node(node, errors::lval_as_expression))
                        }
                    };
                    (true, acc)
                }
            },
            node,
            vec![],
        )
    }

    fn does_binop_create_write_on_left(token_kind: Option<TokenKind>) -> bool {
        token_kind.map_or(false, |x| match x {
            TokenKind::Equal
            | TokenKind::BarEqual
            | TokenKind::PlusEqual
            | TokenKind::StarEqual
            | TokenKind::StarStarEqual
            | TokenKind::SlashEqual
            | TokenKind::DotEqual
            | TokenKind::MinusEqual
            | TokenKind::PercentEqual
            | TokenKind::CaratEqual
            | TokenKind::AmpersandEqual
            | TokenKind::LessThanLessThanEqual
            | TokenKind::GreaterThanGreaterThanEqual
            | TokenKind::QuestionQuestionEqual => true,
            _ => false,
        })
    }

    fn get_positions_binop_allows_await(t: S<'a, Token, Value>) -> BinopAllowsAwaitInPositions {
        use TokenKind::*;
        match Self::token_kind(t) {
            None => BinopAllowAwaitNone,
            Some(t) => match t {
                BarBar | AmpersandAmpersand | QuestionColon | QuestionQuestion | BarGreaterThan => {
                    BinopAllowAwaitLeft
                }
                Equal
                | BarEqual
                | PlusEqual
                | StarEqual
                | StarStarEqual
                | SlashEqual
                | DotEqual
                | MinusEqual
                | PercentEqual
                | CaratEqual
                | AmpersandEqual
                | LessThanLessThanEqual
                | GreaterThanGreaterThanEqual => BinopAllowAwaitRight,
                Plus
                | Minus
                | Star
                | Slash
                | StarStar
                | EqualEqualEqual
                | LessThan
                | GreaterThan
                | Percent
                | Dot
                | EqualEqual
                | ExclamationEqual
                | ExclamationEqualEqual
                | LessThanEqual
                | LessThanEqualGreaterThan
                | GreaterThanEqual
                | Ampersand
                | Bar
                | LessThanLessThan
                | GreaterThanGreaterThan
                | Carat => BinopAllowAwaitBoth,
                QuestionQuestionEqual | _ => BinopAllowAwaitNone,
            },
        }
    }

    fn unop_allows_await(t: S<'a, Token, Value>) -> bool {
        use TokenKind::*;
        Self::token_kind(t).map_or(false, |t| match t {
            Exclamation | Tilde | Plus | Minus | At | Clone | Print | Readonly => true,
            _ => false,
        })
    }

    fn await_as_an_expression_errors(&mut self, await_node: S<'a, Token, Value>) {
        let mut prev = None;
        let mut node = await_node;
        for n in self.parents.iter().rev() {
            if let Some(prev) = prev {
                node = prev;
            }
            prev = Some(n);
            match &n.children {
                // statements that root for the concurrently executed await expressions
                ExpressionStatement(_)
                | ReturnStatement(_)
                | UnsetStatement(_)
                | EchoStatement(_)
                | ThrowStatement(_) => break,
                IfStatement(x) if node as *const _ == &x.condition as *const _ => break,
                ForStatement(x) if node as *const _ == &x.initializer as *const _ => break,
                SwitchStatement(x) if node as *const _ == &x.expression as *const _ => break,
                ForeachStatement(x) if node as *const _ == &x.collection as *const _ => {
                    break;
                }
                UsingStatementBlockScoped(x) if node as *const _ == &x.expressions as *const _ => {
                    break;
                }
                UsingStatementFunctionScoped(x)
                    if node as *const _ == &x.expression as *const _ =>
                {
                    break;
                }
                LambdaExpression(x) if node as *const _ == &x.body as *const _ => break,
                // Dependent awaits are not allowed currently
                PrefixUnaryExpression(x)
                    if Self::token_kind(&x.operator) == Some(TokenKind::Await) =>
                {
                    self.errors.push(Self::make_error_from_node(
                        await_node,
                        errors::invalid_await_position_dependent,
                    ));
                    break;
                }
                // Unary based expressions have their own custom fanout
                PrefixUnaryExpression(x) if Self::unop_allows_await(&x.operator) => {
                    continue;
                }
                PostfixUnaryExpression(x) if Self::unop_allows_await(&x.operator) => {
                    continue;
                }
                DecoratedExpression(x) if Self::unop_allows_await(&x.decorator) => {
                    continue;
                }
                // Special case the pipe operator error message
                BinaryExpression(x)
                    if node as *const _ == &x.right_operand as *const _
                        && Self::token_kind(&x.operator) == Some(TokenKind::BarGreaterThan) =>
                {
                    self.errors.push(Self::make_error_from_node(
                        await_node,
                        errors::invalid_await_position_pipe,
                    ));
                    break;
                }
                // left or right operand of binary expressions are considered legal locations
                // if operator is not short-circuiting and containing expression
                // is in legal location
                BinaryExpression(x)
                    if (match Self::get_positions_binop_allows_await(&x.operator) {
                        BinopAllowAwaitBoth => true,
                        BinopAllowAwaitLeft => node as *const _ == &x.left_operand as *const _,
                        BinopAllowAwaitRight => node as *const _ == &x.right_operand as *const _,
                        BinopAllowAwaitNone => false,
                    }) =>
                {
                    continue;
                }
                // test part of conditional expression is considered legal location if
                //  onditional expression itself is in legal location
                ConditionalExpression(x) if node as *const _ == &x.test as *const _ => {
                    continue;
                }
                FunctionCallExpression(x)
                    if node as *const _ == &x.receiver as *const _
                        || node as *const _ == &x.argument_list as *const _
                            && !x.receiver.is_safe_member_selection_expression() =>
                {
                    continue;
                }

                // object of member selection expression or safe member selection expression
                // is in legal position if member selection expression itself is in legal position
                SafeMemberSelectionExpression(x) if node as *const _ == &x.object as *const _ => {
                    continue;
                }

                // These are nodes where any position is valid
                CastExpression(_)
                | MemberSelectionExpression(_)
                | ScopeResolutionExpression(_)
                | IsExpression(_)
                | AsExpression(_)
                | NullableAsExpression(_)
                | IssetExpression(_)
                | ParenthesizedExpression(_)
                | BracedExpression(_)
                | EmbeddedBracedExpression(_)
                | CollectionLiteralExpression(_)
                | ObjectCreationExpression(_)
                | ConstructorCall(_)
                | ShapeExpression(_)
                | TupleExpression(_)
                | DarrayIntrinsicExpression(_)
                | DictionaryIntrinsicExpression(_)
                | KeysetIntrinsicExpression(_)
                | VarrayIntrinsicExpression(_)
                | VectorIntrinsicExpression(_)
                | ElementInitializer(_)
                | FieldInitializer(_)
                | SimpleInitializer(_)
                | SubscriptExpression(_)
                | EmbeddedSubscriptExpression(_)
                | YieldExpression(_)
                | XHPExpression(_)
                | XHPOpen(_)
                | XHPSimpleAttribute(_)
                | XHPSpreadAttribute(_)
                | SyntaxList(_)
                | ListItem(_) => continue,
                // otherwise report error and bail out
                _ => {
                    self.errors.push(Self::make_error_from_node(
                        await_node,
                        errors::invalid_await_position,
                    ));
                    break;
                }
            }
        }
        let is_in_concurrent = self
            .parents
            .iter()
            .rev()
            .any(|parent| match &parent.children {
                ConcurrentStatement(_) => true,
                _ => false,
            });
        if !is_in_concurrent {
            let await_node_statement_parent =
                self.parents
                    .iter()
                    .rev()
                    .find(|parent| match &parent.children {
                        ExpressionStatement(_)
                        | ReturnStatement(_)
                        | UnsetStatement(_)
                        | EchoStatement(_)
                        | ThrowStatement(_)
                        | IfStatement(_)
                        | ForStatement(_)
                        | SwitchStatement(_)
                        | ForeachStatement(_) => true,
                        _ => false,
                    });
            if let Some(x) = await_node_statement_parent {
                for error in self.find_invalid_lval_usage(x) {
                    self.errors.push(error)
                }
            } else {
                // We must have already errored in for loop
            }
        }
    }

    fn check_prefix_unary_dollar(node: S<'a, Token, Value>) -> bool {
        match &node.children {
            PrefixUnaryExpression(x)
                if Self::token_kind(&x.operator) == Some(TokenKind::Dollar) =>
            {
                Self::check_prefix_unary_dollar(&x.operand)
            }
            BracedExpression(_) | SubscriptExpression(_) | VariableExpression(_) => false, // these ones are valid
            LiteralExpression(_) | PipeVariableExpression(_) => false, // these ones get caught later
            _ => true,
        }
    }

    fn node_has_await_child(&mut self, node: S<'a, Token, Value>) -> bool {
        self.rec_walk(
            |node, _parents, acc| {
                let is_new_scope = match &node.children {
                    AnonymousFunction(_) | LambdaExpression(_) | AwaitableCreationExpression(_) => {
                        true
                    }
                    _ => false,
                };
                if is_new_scope {
                    (false, false)
                } else {
                    let is_await = |n: S<'a, Token, Value>| match &n.children {
                        PrefixUnaryExpression(x)
                            if Self::token_kind(&x.operator) == Some(TokenKind::Await) =>
                        {
                            true
                        }
                        _ => false,
                    };
                    let found_await = acc || is_await(node);
                    (!found_await, found_await)
                }
            },
            node,
            false,
        )
    }

    fn expression_errors(&mut self, node: S<'a, Token, Value>) {
        let check_is_as_expression = |self_: &mut Self, hint: S<'a, Token, Value>| {
            let n = match &node.children {
                IsExpression(_) => "is",
                _ => "as",
            };
            match &hint.children {
                ClosureTypeSpecifier(_) if self_.env.is_hhvm_compat() => {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "__Callable"),
                    ));
                }
                SoftTypeSpecifier(_) => {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "__Soft"),
                    ));
                }
                AttributizedSpecifier(x)
                    if self_.attribute_specification_contains(&x.attribute_spec, "__Soft") =>
                {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "__Soft"),
                    ));
                }
                _ => {}
            }
        };
        match &node.children {
            // We parse the right hand side of `new` as a generic expression, but PHP
            // (and therefore Hack) only allow a certain subset of expressions, so we
            // should verify here that the expression we parsed is in that subset.
            // Refer: https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#instanceof-operator*)
            ConstructorCall(ctr_call) => {
                for p in Self::syntax_to_list_no_separators(&ctr_call.argument_list) {
                    self.function_call_argument_errors(true, p);
                }
                self.class_type_designator_errors(&ctr_call.type_);
                if self.env.is_typechecker() {
                    // attr or list item -> syntax list -> attribute
                    match self.parents.iter().rev().nth(2) {
                        Some(a)
                            if a.is_attribute_specification()
                                || a.is_old_attribute_specification()
                                || a.is_file_attribute_specification() =>
                        {
                            ()
                        }
                        _ => {
                            if ctr_call.left_paren.is_missing() || ctr_call.right_paren.is_missing()
                            {
                                let node = &ctr_call.type_;
                                let constructor_name = self.text(&ctr_call.type_);
                                self.errors.push(Self::make_error_from_node(
                                    node,
                                    errors::error2038(constructor_name),
                                ));
                            }
                        }
                    }
                };
            }
            LiteralExpression(x) => {
                if let Token(token) = &x.expression.children {
                    if token.kind() == TokenKind::DecimalLiteral
                        || token.kind() == TokenKind::DecimalLiteral
                    {
                        let text = self.text(&x.expression);
                        if text.parse::<i64>().is_err() {
                            let error_text = if token.kind() == TokenKind::DecimalLiteral {
                                errors::error2071(text)
                            } else {
                                errors::error2072(text)
                            };
                            self.errors
                                .push(Self::make_error_from_node(node, error_text))
                        }
                    }
                }
            }

            SubscriptExpression(x)
                if self.env.is_typechecker() && x.left_bracket.is_left_brace() =>
            {
                self.errors
                    .push(Self::make_error_from_node(node, errors::error2020))
            }

            FunctionCallExpression(x) => {
                let arg_list = &x.argument_list;
                if let Some(h) = Self::misplaced_variadic_arg(arg_list) {
                    self.errors
                        .push(Self::make_error_from_node(h, errors::error2033))
                }

                for p in Self::syntax_to_list_no_separators(arg_list) {
                    self.function_call_argument_errors(false, p)
                }

                let recv = &x.receiver;

                self.function_call_on_xhp_name_errors(recv);

                let fun_and_clsmeth_disabled = self
                    .env
                    .parser_options
                    .po_disallow_fun_and_cls_meth_pseudo_funcs;

                if Self::strip_ns(self.text(recv)) == Self::strip_ns(sn::readonly::AS_MUT) {
                    self.check_can_use_feature(recv, &UnstableFeatures::Readonly);
                }

                if self.text(recv) == Self::strip_hh_ns(sn::autoimported_functions::FUN_)
                    && fun_and_clsmeth_disabled
                {
                    let mut arg_node_list = Self::syntax_to_list_no_separators(arg_list);
                    match arg_node_list.next() {
                        Some(name) if arg_node_list.count() == 0 => self.errors.push(
                            Self::make_error_from_node(recv, errors::fun_disabled(self.text(name))),
                        ),
                        _ => self.errors.push(Self::make_error_from_node(
                            recv,
                            errors::fun_requires_const_string,
                        )),
                    }
                }

                if self.text(recv) == Self::strip_hh_ns(sn::autoimported_functions::CLASS_METH)
                    && fun_and_clsmeth_disabled
                {
                    self.errors.push(Self::make_error_from_node(
                        recv,
                        errors::class_meth_disabled,
                    ))
                }

                if self.text(recv) == Self::strip_hh_ns(sn::autoimported_functions::INST_METH)
                    && self.env.parser_options.po_disallow_inst_meth
                {
                    self.errors
                        .push(Self::make_error_from_node(recv, errors::inst_meth_disabled))
                }
            }

            ETSpliceExpression(_) => {
                if !self.env.context.active_expression_tree {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::splice_outside_et))
                }
            }

            ListExpression(x) if x.members.is_missing() && self.env.is_hhvm_compat() => {
                if let Some(Syntax {
                    children: ForeachStatement(x),
                    ..
                }) = self.parents.last()
                {
                    if node as *const _ == &x.value as *const _ {
                        self.errors.push(Self::make_error_from_node_with_type(
                            node,
                            errors::error2077,
                            ErrorType::RuntimeError,
                        ))
                    }
                }
            }

            ListExpression(_) => {
                if self
                    .parents
                    .last()
                    .map_or(false, |e| e.is_return_statement())
                {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::list_must_be_lvar))
                }
            }
            ShapeExpression(x) => {
                for f in Self::syntax_to_list_no_separators(&x.fields).rev() {
                    self.invalid_shape_field_check(f)
                }
            }
            DecoratedExpression(x) => {
                let decorator = &x.decorator;
                if Self::token_kind(decorator) == Some(TokenKind::Await) {
                    self.await_as_an_expression_errors(node)
                }
            }
            YieldExpression(_) => {
                if self.is_in_unyieldable_magic_method() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::yield_in_magic_methods,
                    ))
                }
                if self.env.context.active_callable.is_none() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::yield_outside_function,
                    ))
                }

                if self.has_inout_params() {
                    let e = if self.is_inside_async_method() {
                        errors::inout_param_in_async_generator
                    } else {
                        errors::inout_param_in_generator
                    };
                    self.errors.push(Self::make_error_from_node_with_type(
                        node,
                        e,
                        ErrorType::RuntimeError,
                    ))
                }
            }
            ScopeResolutionExpression(x) => {
                let qualifier = &x.qualifier;
                let name = &x.name;

                let (is_dynamic_name, is_self_or_parent, is_valid) =
                    // PHP langspec allows string literals, variables
                    // qualified names, static, self and parent as valid qualifiers
                    // We do not allow string literals in hack
                    match (&qualifier.children, Self::token_kind(qualifier)) {
                            (LiteralExpression(_), _) => (false, false, false),
                            (QualifiedName(_), _) => (false, false, true),
                            (_, Some(TokenKind::Name))
                            | (_, Some(TokenKind::XHPClassName))
                            | (_, Some(TokenKind::Static)) => (false, false, true),
                            (_, Some(TokenKind::SelfToken)) | (_, Some(TokenKind::Parent)) => {
                                (false, true, true)
                            }
                            // ${}::class
                            (PrefixUnaryExpression(x), _)
                                if Self::token_kind(&x.operator) == Some(TokenKind::Dollar) =>
                            {
                                (true, false, true)
                            }
                            (PipeVariableExpression(_), _)
                            | (VariableExpression(_), _)
                            | (SimpleTypeSpecifier(_), _)
                            | (GenericTypeSpecifier(_), _) => (true, false, true),
                            _ => (true, false, false),
                        };
                if !is_valid && self.env.is_typechecker() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_scope_resolution_qualifier,
                    ))
                }
                let is_name_class = self.text(name).eq_ignore_ascii_case("class");
                if (is_dynamic_name || !is_valid) && is_name_class {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::coloncolonclass_on_dynamic,
                    ))
                }
                let text_name = self.text(qualifier);
                let is_name_namespace = text_name.eq_ignore_ascii_case("namespace");
                if is_name_namespace {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::namespace_not_a_classname,
                    ))
                }
                if is_self_or_parent && is_name_class && !self.is_in_active_class_scope() {
                    self.errors.push(Self::make_error_from_node_with_type(
                        node,
                        errors::self_or_parent_colon_colon_class_outside_of_class(text_name),
                        ErrorType::RuntimeError,
                    ))
                }
            }

            PrefixUnaryExpression(x)
                if Self::token_kind(&x.operator) == Some(TokenKind::Dollar) =>
            {
                if Self::check_prefix_unary_dollar(node) {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::dollar_unary))
                }
            }

            // TODO(T21285960): Remove this bug-port, stemming from T22184312
            LambdaExpression(x)
                if self.env.is_hhvm_compat()
                    && !x.async_.is_missing()
                    && x.async_.trailing_width() == 0
                    && x.signature.leading_width() == 0 =>
            {
                self.errors
                    .push(Self::make_error_from_node(node, errors::error1057("==>")))
            }
            // End of bug-port
            IsExpression(x) => check_is_as_expression(self, &x.right_operand),
            AsExpression(x) => check_is_as_expression(self, &x.right_operand),

            ConditionalExpression(x) => {
                if x.consequence.is_missing() && self.env.is_typechecker() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::elvis_operator_space,
                    ))
                }
                if x.test.is_conditional_expression() && self.env.is_typechecker() {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::nested_ternary))
                }
                match &x.alternative.children {
                    LambdaExpression(x)
                        if x.body.is_conditional_expression() && self.env.is_typechecker() =>
                    {
                        self.errors
                            .push(Self::make_error_from_node(node, errors::nested_ternary))
                    }
                    _ => {}
                }
            }
            LambdaExpression(x) => {
                self.no_memoize_attribute_on_lambda(&x.attribute_spec);
                self.no_async_before_lambda_body(&x.body);
            }
            AnonymousFunction(x) => self.no_memoize_attribute_on_lambda(&x.attribute_spec),
            AwaitableCreationExpression(x) => {
                self.no_memoize_attribute_on_lambda(&x.attribute_spec)
            }

            CollectionLiteralExpression(x) => {
                enum Status {
                    ValidClass(String),
                    InvalidClass,
                    InvalidBraceKind,
                }
                use Status::*;

                let n = &x.name;
                let initializers = &x.initializers;

                let is_standard_collection = |lc_name: &str| {
                    lc_name.eq_ignore_ascii_case("pair")
                        || lc_name.eq_ignore_ascii_case("vector")
                        || lc_name.eq_ignore_ascii_case("map")
                        || lc_name.eq_ignore_ascii_case("set")
                        || lc_name.eq_ignore_ascii_case("immvector")
                        || lc_name.eq_ignore_ascii_case("immmap")
                        || lc_name.eq_ignore_ascii_case("immset")
                };
                let use_key_value_initializers = |lc_name: &str| {
                    lc_name.eq_ignore_ascii_case("map") || lc_name.eq_ignore_ascii_case("immmap")
                };
                let is_qualified_std_collection = |l, r| {
                    Self::token_kind(l) == Some(TokenKind::Name)
                        && Self::token_kind(r) == Some(TokenKind::Name)
                        && self.text(l).eq_ignore_ascii_case("hh")
                        && is_standard_collection(&self.text(r))
                };

                let check_type_specifier = |n, t: &Token| {
                    if t.kind() == TokenKind::Name {
                        match self.text(n).to_ascii_lowercase().as_ref() {
                            "dict" | "vec" | "keyset" => InvalidBraceKind,
                            n => {
                                if is_standard_collection(n) {
                                    ValidClass(n.to_string())
                                } else {
                                    InvalidClass
                                }
                            }
                        }
                    } else {
                        InvalidClass
                    }
                };

                let check_qualified_name = |parts| {
                    let mut parts = Self::syntax_to_list(false, parts);
                    let p1 = parts.next();
                    let p2 = parts.next();
                    let p3 = parts.next();
                    let p4 = parts.next();
                    match (p1, p2, p3, p4) {
                        (Some(l), Some(r), None, None)
                            if self.namespace_name == GLOBAL_NAMESPACE_NAME
                                && is_qualified_std_collection(l, r) =>
                        {
                            // HH\Vector in global namespace
                            ValidClass(self.text(r).to_ascii_lowercase())
                        }
                        (Some(missing), Some(l), Some(r), None)
                            if missing.is_missing() && is_qualified_std_collection(l, r) =>
                        {
                            // \HH\Vector
                            ValidClass(self.text(r).to_ascii_lowercase())
                        }
                        _ => InvalidClass,
                    }
                };
                let status = match &n.children {
                    // non-qualified name
                    SimpleTypeSpecifier(x) => match &x.specifier.children {
                        Token(t) => check_type_specifier(&x.specifier, t),
                        QualifiedName(x) => check_qualified_name(&x.parts),
                        _ => InvalidClass,
                    },
                    GenericTypeSpecifier(x) => match &x.class_type.children {
                        Token(t) => check_type_specifier(&x.class_type, t),
                        QualifiedName(x) => check_qualified_name(&x.parts),
                        _ => InvalidClass,
                    },
                    _ => InvalidClass,
                };

                let is_key_value = |s: &Syntax<'a, Token, Value>| {
                    if let ElementInitializer(_) = s.children {
                        true
                    } else {
                        false
                    }
                };
                let initializer_list = || Self::syntax_to_list_no_separators(initializers);
                let num_initializers = initializer_list().count();
                match &status {
                    ValidClass(name)
                        if use_key_value_initializers(name)
                            && initializer_list().any(|i| !is_key_value(i)) =>
                    {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::invalid_value_initializer(self.text(n)),
                        ));
                    }

                    ValidClass(name)
                        if !use_key_value_initializers(name)
                            && initializer_list().any(|i| is_key_value(i)) =>
                    {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::invalid_key_value_initializer(self.text(n)),
                        ));
                    }

                    ValidClass(pair) if pair == "pair" && num_initializers != 2 => {
                        let msg = if num_initializers == 0 {
                            errors::pair_initializer_needed
                        } else {
                            errors::pair_initializer_arity
                        };
                        self.errors.push(Self::make_error_from_node_with_type(
                            node,
                            msg,
                            ErrorType::RuntimeError,
                        ));
                    }

                    ValidClass(_) => {}
                    InvalidBraceKind => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_brace_kind_in_collection_initializer,
                    )),
                    InvalidClass => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_class_in_collection_initializer,
                    )),
                }
            }
            PrefixUnaryExpression(x) if Self::token_kind(&x.operator) == Some(TokenKind::Await) => {
                self.await_as_an_expression_errors(node)
            }
            PrefixUnaryExpression(x)
                if Self::token_kind(&x.operator) == Some(TokenKind::Readonly) =>
            {
                self.check_can_use_feature(&x.operator, &UnstableFeatures::Readonly)
            }

            // Other kinds of expressions currently produce no expr errors.
            _ => {}
        }
    }

    fn check_repeated_properties_tconst_const(
        &mut self,
        full_name: &str,
        prop: S<'a, Token, Value>,
        p_names: &mut HashSet<String>,
        c_names: &mut HashSet<String>,
        xhp_names: &mut HashSet<String>,
    ) {
        let mut check = |sname, names: &mut HashSet<String>| {
            let name = self.text(sname);
            // If the name is empty, then there was an earlier
            // parsing error that should supercede this one.
            if name == "" {
            } else if names.contains(name) {
                self.errors.push(Self::make_error_from_node(
                    prop,
                    errors::redeclaration_error(
                        &(Self::strip_ns(&full_name).to_string() + "::" + name),
                    ),
                ))
            } else {
                names.insert(name.to_owned());
            }
        };

        match &prop.children {
            PropertyDeclaration(x) => {
                for prop in Self::syntax_to_list_no_separators(&x.declarators) {
                    if let PropertyDeclarator(x) = &prop.children {
                        check(&x.name, p_names)
                    }
                }
            }
            ConstDeclaration(x) => {
                for prop in Self::syntax_to_list_no_separators(&x.declarators) {
                    if let ConstantDeclarator(x) = &prop.children {
                        check(&x.name, c_names)
                    }
                }
            }
            TypeConstDeclaration(x) => check(&x.name, c_names),
            ContextConstDeclaration(x) => check(&x.name, c_names),
            XHPClassAttributeDeclaration(x) => {
                for attr in Self::syntax_to_list_no_separators(&x.attributes) {
                    if let XHPClassAttribute(x) = &attr.children {
                        check(&x.name, xhp_names)
                    }
                }
            }
            _ => {}
        }
    }

    fn require_errors(&mut self, node: S<'a, Token, Value>) {
        if let RequireClause(p) = &node.children {
            let name = self.text(&p.name);
            let req_kind = Self::token_kind(&p.kind);
            match (self.trait_require_clauses.get(name), req_kind) {
                (None, Some(tk)) => self.trait_require_clauses.add(name, tk),
                (Some(tk1), Some(tk2)) if *tk1 == tk2 =>
                    // duplicate, it is okay
                    {}
                _ => {
                    // Conflicting entry
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::conflicting_trait_require_clauses(name),
                    ))
                }
            };
            match (self.active_classish_kind(), req_kind) {
                (Some(TokenKind::Interface), Some(TokenKind::Implements))
                | (Some(TokenKind::Class), Some(TokenKind::Implements)) => self
                    .errors
                    .push(Self::make_error_from_node(node, errors::error2030)),
                _ => {}
            }
        }
    }

    fn check_type_name(&mut self, name: S<'a, Token, Value>, name_text: &str, location: Location) {
        match self.names.classes.get(name_text) {
            Some(FirstUseOrDef {
                location,
                kind,
                name: def_name,
                ..
            }) if &combine_names(&self.namespace_name, name_text) != def_name
                && *kind != NameDef =>
            {
                let (line_num, _) = self
                    .env
                    .text
                    .offset_to_position(location.start_offset as isize);
                let line_num = line_num as usize;
                let long_name_text = combine_names(&self.namespace_name, name_text);
                self.errors.push(Self::make_name_already_used_error(
                    &name,
                    &long_name_text,
                    name_text,
                    location,
                    &|x, y| match kind {
                        NameImplicitUse => {
                            errors::declared_name_is_already_in_use_implicit_hh(line_num, x, y)
                        }
                        NameUse => errors::declared_name_is_already_in_use(line_num, x, y),
                        NameDef => errors::type_name_is_already_in_use(x, y),
                    },
                ))
            }
            _ => {
                let def = make_first_use_or_def(
                    false,
                    NameDef,
                    location,
                    &self.namespace_name,
                    name_text,
                );
                self.names.classes.add(&name_text, def)
            }
        }
    }

    fn get_type_params_and_emit_shadowing_errors(
        &mut self,
        l: S<'a, Token, Value>,
    ) -> (HashSet<&'a str>, HashSet<&'a str>) {
        let mut res: HashSet<&'a str> = HashSet::default();
        let mut notreified: HashSet<&'a str> = HashSet::default();
        for p in Self::syntax_to_list_no_separators(l).rev() {
            match &p.children {
                TypeParameter(x) => {
                    let name = self.text(&x.name);
                    if !x.reified.is_missing() {
                        if res.contains(&name) {
                            self.errors
                                .push(Self::make_error_from_node(p, errors::shadowing_reified))
                        } else {
                            res.insert(name);
                        }
                    } else {
                        notreified.insert(name);
                    }
                }
                _ => {}
            }
        }
        (res, notreified)
    }

    fn reified_parameter_errors(&mut self, node: S<'a, Token, Value>) {
        if let FunctionDeclarationHeader(x) = &node.children {
            if let TypeParameters(x) = &x.type_parameter_list.children {
                self.get_type_params_and_emit_shadowing_errors(&x.parameters);
            }
        }
    }

    fn is_method_declaration(node: S<'a, Token, Value>) -> bool {
        if let MethodishDeclaration(_) = &node.children {
            true
        } else {
            false
        }
    }

    fn class_reified_param_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            ClassishDeclaration(cd) => {
                let (reified, non_reified) = match &cd.type_parameters.children {
                    TypeParameters(x) => {
                        self.get_type_params_and_emit_shadowing_errors(&x.parameters)
                    }
                    _ => (HashSet::default(), HashSet::default()),
                };

                let tparams: HashSet<&'a str> = reified
                    .union(&non_reified)
                    .cloned()
                    .collect::<HashSet<&'a str>>();

                let add_error = |self_: &mut Self, e: S<'a, Token, Value>| {
                    if let TypeParameter(x) = &e.children {
                        if !x.reified.is_missing() && tparams.contains(&self_.text(&x.name)) {
                            self_
                                .errors
                                .push(Self::make_error_from_node(e, errors::shadowing_reified))
                        }
                    }
                };
                let check_method = |e: S<'a, Token, Value>| {
                    if let MethodishDeclaration(x) = &e.children {
                        if let FunctionDeclarationHeader(x) = &x.function_decl_header.children {
                            if let TypeParameters(x) = &x.type_parameter_list.children {
                                Self::syntax_to_list_no_separators(&x.parameters)
                                    .rev()
                                    .for_each(|x| add_error(self, x))
                            }
                        }
                    }
                };
                if let ClassishBody(x) = &cd.body.children {
                    Self::syntax_to_list_no_separators(&x.elements)
                        .rev()
                        .for_each(check_method)
                }

                if !reified.is_empty() {
                    if Self::is_token_kind(&cd.keyword, TokenKind::Interface) {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::reified_in_invalid_classish("an interface"),
                        ))
                    } else if Self::is_token_kind(&cd.keyword, TokenKind::Trait) {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::reified_in_invalid_classish("a trait"),
                        ))
                    }
                }
            }
            PropertyDeclaration(_) => {
                if Self::has_modifier_static(node) && self.is_in_reified_class() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::static_property_in_reified_class,
                    ));
                }
            }
            _ => {}
        }
    }

    fn attr_spec_contains_sealed(&self, node: S<'a, Token, Value>) -> bool {
        self.attribute_specification_contains(node, sn::user_attributes::SEALED)
    }

    fn attr_spec_contains_enum_class(&self, node: S<'a, Token, Value>) -> bool {
        self.attribute_specification_contains(node, sn::user_attributes::ENUM_CLASS)
    }

    fn attr_spec_contains_const(&self, node: S<'a, Token, Value>) -> bool {
        self.attribute_specification_contains(node, sn::user_attributes::CONST)
    }

    // If there's more than one XHP category, report an error on the last one.
    fn duplicate_xhp_category_errors<I>(&mut self, elts: I)
    where
        I: Iterator<Item = S<'a, Token, Value>>,
    {
        let mut iter = elts.filter(|x| matches!(&(*x).children, XHPCategoryDeclaration(_)));
        iter.next();
        if let Some(node) = iter.last() {
            self.errors.push(Self::make_error_from_node(
                node,
                errors::xhp_class_multiple_category_decls,
            ))
        }
    }

    // If there's more than one XHP children declaration, report an error
    // on the last one.
    fn duplicate_xhp_children_errors<I>(&mut self, elts: I)
    where
        I: Iterator<Item = S<'a, Token, Value>>,
    {
        let mut iter = elts.filter(|x| matches!(&(*x).children, XHPChildrenDeclaration(_)));
        iter.next();
        if let Some(node) = iter.last() {
            self.errors.push(Self::make_error_from_node(
                node,
                errors::xhp_class_multiple_children_decls,
            ))
        }
    }

    fn interface_private_method_errors<I>(&mut self, elts: I)
    where
        I: Iterator<Item = S<'a, Token, Value>>,
    {
        for elt in elts {
            if let Some(modifiers) = Self::get_modifiers_of_declaration(elt) {
                for modifier in Self::syntax_to_list_no_separators(modifiers) {
                    if modifier.is_private() {
                        self.errors.push(Self::make_error_from_node(
                            modifier,
                            errors::interface_has_private_method,
                        ))
                    }
                }
            }
        }
    }

    fn classish_errors(&mut self, node: S<'a, Token, Value>) {
        if let ClassishDeclaration(cd) = &node.children {
            // Given a ClassishDeclaration node, test whether or not it's a trait
            // invoking the 'extends' keyword.
            let classish_invalid_extends_keyword = |_| {
                // Invalid if uses 'extends' and is a trait.
                Self::token_kind(&cd.extends_keyword) == Some(TokenKind::Extends)
                    && Self::token_kind(&cd.keyword) == Some(TokenKind::Trait)
            };

            let abstract_keyword = Self::extract_keyword(|x| x.is_abstract(), node).unwrap_or(node);

            self.produce_error(
                |self_, x| self_.is_classish_kind_declared_abstract(x),
                node,
                || errors::error2042,
                abstract_keyword,
            );

            // Given a sealed ClassishDeclaration node, test whether all the params
            // are classnames.
            let classish_sealed_arg_not_classname = |self_: &mut Self| {
                Self::attr_spec_to_node_list(&cd.attribute).any(|node| {
                    self_.attr_name(node) == Some(sn::user_attributes::SEALED)
                        && self_.attr_args(node).map_or(false, |mut args| {
                            args.any(|arg_node| match &arg_node.children {
                                ScopeResolutionExpression(x) => self_.text(&x.name) != "class",
                                _ => true,
                            })
                        })
                })
            };

            // Only "regular" class names are allowed in `__Sealed()`
            // attributes.
            for node in Self::attr_spec_to_node_list(&cd.attribute) {
                if (self.attr_name(node)) == Some(sn::user_attributes::SEALED) {
                    match self.attr_args(node) {
                        Some(args) => {
                            for arg in args {
                                match &arg.children {
                                    ScopeResolutionExpression(x) => {
                                        let txt = self.text(&x.qualifier);
                                        let excludes = vec![
                                            sn::classes::SELF,
                                            sn::classes::PARENT,
                                            sn::classes::STATIC,
                                        ];
                                        if excludes.iter().any(|&e| txt == e) {
                                            self.errors.push(Self::make_error_from_node(
                                                &x.qualifier,
                                                errors::sealed_qualifier_invalid,
                                            ));
                                        }
                                    }
                                    _ => {}
                                }
                            }
                        }
                        None => {}
                    }
                }
            }

            self.check_attr_enabled(&cd.attribute);

            let classish_is_sealed = self.attr_spec_contains_sealed(&cd.attribute);

            // Given a ClassishDeclaration node, test whether or not length of
            // extends_list is appropriate for the classish_keyword. *)
            let classish_invalid_extends_list = |self_: &mut Self| {
                // Invalid if is a class and has list of length greater than one.
                self_.env.is_typechecker()
                    && Self::token_kind(&cd.keyword) == Some(TokenKind::Class)
                    && Self::token_kind(&cd.extends_keyword) == Some(TokenKind::Extends)
                    && Self::syntax_to_list_no_separators(&cd.extends_list).count() != 1
            };

            // Given a ClassishDeclaration node, test whether it is sealed and final.
            let classish_sealed_final = |_| {
                Self::list_contains_predicate(|x| x.is_final(), &cd.modifiers) && classish_is_sealed
            };

            self.produce_error(
                |self_, _| classish_invalid_extends_list(self_),
                &(),
                || errors::error2037,
                &cd.extends_list,
            );

            self.invalid_modifier_errors("Classes, interfaces, and traits", node, |kind| {
                kind == TokenKind::Abstract || kind == TokenKind::Final || kind == TokenKind::XHP
            });

            self.produce_error(
                |self_, _| classish_sealed_arg_not_classname(self_),
                &(),
                || errors::sealed_val_not_classname,
                &cd.attribute,
            );

            self.produce_error(
                |_, x| classish_invalid_extends_keyword(x),
                &(),
                || errors::error2036,
                &cd.extends_keyword,
            );

            self.produce_error(
                |_, x| classish_sealed_final(x),
                &(),
                || errors::sealed_final,
                &cd.attribute,
            );

            let classish_name = self.text(&cd.name);
            self.produce_error(
                |_, x| Self::cant_be_classish_name(x),
                &classish_name,
                || errors::reserved_keyword_as_class_name(&classish_name),
                &cd.name,
            );
            if Self::is_token_kind(&cd.keyword, TokenKind::Interface)
                && !cd.implements_keyword.is_missing()
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::interface_implements,
                ))
            };
            if self.attr_spec_contains_const(&cd.attribute)
                && (Self::is_token_kind(&cd.keyword, TokenKind::Interface)
                    || Self::is_token_kind(&cd.keyword, TokenKind::Trait))
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_interfaces_traits_enums,
                ))
            }

            if self.attr_spec_contains_const(&cd.attribute)
                && Self::is_token_kind(&cd.keyword, TokenKind::Class)
                && Self::list_contains_predicate(|x| x.is_abstract(), &cd.modifiers)
                && Self::list_contains_predicate(|x| x.is_final(), &cd.modifiers)
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_abstract_final_class,
                ))
            }

            if Self::list_contains_predicate(|x| x.is_final(), &cd.modifiers) {
                match Self::token_kind(&cd.keyword) {
                    Some(TokenKind::Interface) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::declared_final("Interfaces"),
                    )),
                    Some(TokenKind::Trait) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::declared_final("Traits"),
                    )),
                    _ => {}
                }
            }

            if Self::token_kind(&cd.xhp) == Some(TokenKind::XHP) {
                match Self::token_kind(&cd.keyword) {
                    Some(TokenKind::Interface) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_xhp_classish("Interfaces"),
                    )),
                    Some(TokenKind::Trait) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_xhp_classish("Traits"),
                    )),
                    Some(TokenKind::Enum) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_xhp_classish("Enums"),
                    )),
                    _ => {}
                }
            }

            let name = self.text(&cd.name);
            if let ClassishBody(cb) = &cd.body.children {
                let declared_name_str = self.text(&cd.name);
                let full_name = combine_names(&self.namespace_name, declared_name_str);

                let class_body_elts = || Self::syntax_to_list_no_separators(&cb.elements);
                let class_body_methods =
                    || class_body_elts().filter(|x| Self::is_method_declaration(x));

                let mut p_names = HashSet::<String>::default();
                let mut c_names = HashSet::<String>::default();
                let mut xhp_names = HashSet::<String>::default();
                for elt in class_body_elts() {
                    self.check_repeated_properties_tconst_const(
                        &full_name,
                        elt,
                        &mut p_names,
                        &mut c_names,
                        &mut xhp_names,
                    );
                }
                let has_abstract_fn = class_body_methods().any(&Self::has_modifier_abstract);
                if has_abstract_fn
                    && Self::is_token_kind(&cd.keyword, TokenKind::Class)
                    && !Self::list_contains_predicate(|x| x.is_abstract(), &cd.modifiers)
                {
                    self.errors.push(Self::make_error_from_node(
                        &cd.name,
                        errors::class_with_abstract_method(name),
                    ))
                }

                if Self::is_token_kind(&cd.keyword, TokenKind::Interface) {
                    self.interface_private_method_errors(class_body_elts());
                }

                self.duplicate_xhp_category_errors(class_body_elts());
                self.duplicate_xhp_children_errors(class_body_elts());
            }

            match Self::token_kind(&cd.keyword) {
                Some(TokenKind::Class) | Some(TokenKind::Trait) if !cd.name.is_missing() => {
                    let location = Self::make_location_of_node(&cd.name);
                    self.check_type_name(&cd.name, name, location)
                }
                _ => {}
            }
        }
    }

    // Checks for modifiers on class constants
    fn class_constant_modifier_errors(&mut self, node: S<'a, Token, Value>) {
        self.invalid_modifier_errors("Constants", node, |kind| kind == TokenKind::Abstract);
    }

    fn type_const_modifier_errors(&mut self, node: S<'a, Token, Value>) {
        self.invalid_modifier_errors("Type constants", node, |kind| kind == TokenKind::Abstract);
    }

    fn alias_errors(&mut self, node: S<'a, Token, Value>) {
        if let AliasDeclaration(ad) = &node.children {
            let attrs = &ad.attribute_spec;
            self.check_attr_enabled(&attrs);
            if Self::token_kind(&ad.keyword) == Some(TokenKind::Type) && !ad.constraint.is_missing()
            {
                self.errors
                    .push(Self::make_error_from_node(&ad.keyword, errors::error2034))
            }
            if !ad.name.is_missing() {
                let name = self.text(&ad.name);
                let location = Self::make_location_of_node(&ad.name);
                if let TypeConstant(_) = &ad.type_.children {
                    if self.env.is_typechecker() {
                        self.errors.push(Self::make_error_from_node(
                            &ad.type_,
                            errors::type_alias_to_type_constant,
                        ))
                    }
                }

                self.check_type_name(&ad.name, name, location)
            }
        } else if let ContextAliasDeclaration(cad) = &node.children {
            self.check_can_use_feature(node, &UnstableFeatures::ContextAliasDeclaration);
            let attrs = &cad.attribute_spec;
            self.check_attr_enabled(&attrs);
            if Self::token_kind(&cad.keyword) == Some(TokenKind::Type)
                && !cad.as_constraint.is_missing()
            {
                self.errors
                    .push(Self::make_error_from_node(&cad.keyword, errors::error2034))
            }
            if !cad.name.is_missing() {
                let name = self.text(&cad.name);
                let location = Self::make_location_of_node(&cad.name);
                if let TypeConstant(_) = &cad.context.children {
                    if self.env.is_typechecker() {
                        self.errors.push(Self::make_error_from_node(
                            &cad.context,
                            errors::type_alias_to_type_constant,
                        ))
                    }
                }

                self.check_type_name(&cad.name, name, location)
            }
        }
    }

    fn is_invalid_group_use_clause(kind: S<'a, Token, Value>, clause: S<'a, Token, Value>) -> bool {
        if let NamespaceUseClause(x) = &clause.children {
            let clause_kind = &x.clause_kind;
            if kind.is_missing() {
                match &clause_kind.children {
                    Missing => false,
                    Token(token)
                        if token.kind() == TokenKind::Function
                            || token.kind() == TokenKind::Const =>
                    {
                        false
                    }
                    _ => true,
                }
            } else {
                !clause_kind.is_missing()
            }
        } else {
            false
        }
    }

    fn is_invalid_group_use_prefix(prefix: S<'a, Token, Value>) -> bool {
        !prefix.is_namespace_prefix()
    }

    fn group_use_errors(&mut self, node: S<'a, Token, Value>) {
        if let NamespaceGroupUseDeclaration(x) = &node.children {
            let prefix = &x.prefix;
            let clauses = &x.clauses;
            let kind = &x.kind;
            Self::syntax_to_list_no_separators(clauses)
                .filter(|x| Self::is_invalid_group_use_clause(kind, x))
                .for_each(|clause| {
                    self.errors
                        .push(Self::make_error_from_node(clause, errors::error2049))
                });
            self.produce_error(
                |_, x| Self::is_invalid_group_use_prefix(&x),
                prefix,
                || errors::error2048,
                prefix,
            )
        }
    }

    fn use_class_or_namespace_clause_errors(
        &mut self,
        namespace_prefix: Option<&str>,

        kind: S<'a, Token, Value>,
        cl: S<'a, Token, Value>,
    ) {
        match &cl.children {
            NamespaceUseClause(x) if !&x.name.is_missing() => {
                let name = &x.name;

                let kind = if kind.is_missing() {
                    &x.clause_kind
                } else {
                    kind
                };

                let name_text = self.text(name);
                let qualified_name = match namespace_prefix {
                    None => combine_names(GLOBAL_NAMESPACE_NAME, name_text),
                    Some(p) => combine_names(p, name_text),
                };
                let short_name =
                    Self::get_short_name_from_qualified_name(name_text, self.text(&x.alias));

                let do_check = |
                    self_: &mut Self,
                    error_on_global_redefinition,
                    get_names: &dyn Fn(&mut UsedNames) -> &mut Strmap<FirstUseOrDef>,
                    report_error,
                | {
                    let is_global_namespace = self_.is_global_namespace();
                    let names = get_names(&mut self_.names);
                    match names.get(&short_name) {
                        Some(FirstUseOrDef {
                            location,
                            kind,
                            global,
                            ..
                        }) => {
                            if *kind != NameDef
                                || error_on_global_redefinition && (is_global_namespace || *global)
                            {
                                self_.errors.push(Self::make_name_already_used_error(
                                    name,
                                    name_text,
                                    &short_name,
                                    location,
                                    report_error,
                                ))
                            }
                        }
                        None => {
                            let new_use = make_first_use_or_def(
                                false,
                                NameUse,
                                Self::make_location_of_node(name),
                                GLOBAL_NAMESPACE_NAME,
                                &qualified_name,
                            );
                            names.add(&short_name, new_use)
                        }
                    }
                };

                match &kind.children {
                    Token(token) => match token.kind() {
                        TokenKind::Namespace => do_check(
                            self,
                            false,
                            &|x: &mut UsedNames| &mut x.namespaces,
                            &errors::namespace_name_is_already_in_use,
                        ),

                        TokenKind::Type => do_check(
                            self,
                            false,
                            &|x: &mut UsedNames| &mut x.classes,
                            &errors::type_name_is_already_in_use,
                        ),

                        TokenKind::Function => do_check(
                            self,
                            true,
                            &|x: &mut UsedNames| &mut x.functions,
                            &errors::function_name_is_already_in_use,
                        ),

                        TokenKind::Const => do_check(
                            self,
                            true,
                            &|x: &mut UsedNames| &mut x.constants,
                            &errors::const_name_is_already_in_use,
                        ),
                        _ => {}
                    },
                    Missing => {
                        if name_text == "strict" {
                            self.errors.push(Self::make_error_from_node(
                                name,
                                errors::strict_namespace_hh,
                            ))
                        }
                        let location = Self::make_location_of_node(name);

                        match self.names.classes.get(&short_name) {
                            Some(FirstUseOrDef {
                                location: loc,
                                name: def_name,
                                kind,
                                ..
                            }) => {
                                if &qualified_name != def_name || kind != &NameDef {
                                    let (line_num, _) =
                                        self.env.text.offset_to_position(loc.start_offset as isize);
                                    let err_msg = |x: &str, y: &str| -> Error {
                                        if kind != &NameDef {
                                            if kind == &NameImplicitUse {
                                                errors::name_is_already_in_use_implicit_hh(
                                                    line_num, x, y,
                                                )
                                            } else {
                                                errors::name_is_already_in_use_hh(line_num, x, y)
                                            }
                                        } else {
                                            errors::name_is_already_in_use_php(x, y)
                                        }
                                    };

                                    self.errors.push(Self::make_name_already_used_error(
                                        name,
                                        name_text,
                                        &short_name,
                                        loc,
                                        &err_msg,
                                    ))
                                }
                            }
                            None => {
                                let new_use = make_first_use_or_def(
                                    false,
                                    NameUse,
                                    location,
                                    GLOBAL_NAMESPACE_NAME,
                                    &qualified_name,
                                );

                                if !self.names.namespaces.mem(&short_name) {
                                    self.names.namespaces.add(&short_name, new_use.clone());
                                    self.names.classes.add(&short_name, new_use);
                                } else {
                                    self.names.classes.add(&short_name, new_use);
                                }
                            }
                        }
                    }
                    _ => {}
                }
            }

            _ => {}
        }
    }

    fn is_global_in_const_decl(&self, init: S<'a, Token, Value>) -> bool {
        if let SimpleInitializer(x) = &init.children {
            if let VariableExpression(x) = &x.value.children {
                return sn::superglobals::is_any_global(self.text(&x.expression));
            }
        }
        false
    }

    fn namespace_use_declaration_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            NamespaceUseDeclaration(x) => {
                Self::syntax_to_list_no_separators(&x.clauses).for_each(|clause| {
                    self.use_class_or_namespace_clause_errors(None, &x.kind, clause)
                })
            }
            NamespaceGroupUseDeclaration(x) => Self::syntax_to_list_no_separators(&x.clauses)
                .for_each(|clause| {
                    match &clause.children {
                        NamespaceUseClause(x) if !x.name.is_missing() => {
                            self.check_preceding_backslashes_qualified_name(&x.name)
                        }
                        _ => {}
                    }
                    self.use_class_or_namespace_clause_errors(
                        Some(self.text(&x.prefix)),
                        &x.kind,
                        clause,
                    )
                }),
            _ => {}
        }
    }

    fn token_text<'b>(&'b self, token: &Token) -> &'b str {
        self.env.text.source_text().sub_as_str(
            token.leading_start_offset().unwrap() + token.leading_width(),
            token.width(),
        )
    }

    fn check_constant_expression(&mut self, node: S<'a, Token, Value>) {
        // __FUNCTION_CREDENTIAL__ emits an object,
        // so it cannot be used in a constant expression
        let not_function_credential = |self_: &Self, token: &Token| {
            !self_
                .token_text(token)
                .eq_ignore_ascii_case("__FUNCTION_CREDENTIAL__")
        };

        let is_whitelisted_function = |self_: &Self, receiver_token| {
            let text = self_.text(receiver_token);

            (!self_.env.parser_options.po_disallow_func_ptrs_in_constants
                && (text == Self::strip_hh_ns(sn::autoimported_functions::FUN_)
                    || text == Self::strip_hh_ns(sn::autoimported_functions::CLASS_METH)))
                || (text == sn::std_lib_functions::ARRAY_MARK_LEGACY)
                || (text == Self::strip_ns(sn::std_lib_functions::ARRAY_MARK_LEGACY))
                || (text == sn::std_lib_functions::ARRAY_UNMARK_LEGACY)
                || (text == Self::strip_ns(sn::std_lib_functions::ARRAY_UNMARK_LEGACY))
        };

        let is_namey = |self_: &Self, token: &Token| -> bool {
            token.kind() == TokenKind::Name && not_function_credential(self_, token)
        };

        let is_good_scope_resolution_name = |node: S<'a, Token, Value>| match &node.children {
            QualifiedName(_) => true,
            Token(token) => {
                use TokenKind::*;
                match token.kind() {
                    Name | Trait | Extends | Implements | Static | Abstract | Final | Private
                    | Protected | Public | Global | Instanceof | Insteadof | Interface
                    | Namespace | New | Try | Use | Var | List | Clone | Include | Include_once
                    | Throw | Tuple | Print | Echo | Require | Require_once | Return | Else
                    | Elseif | Default | Break | Continue | Switch | Yield | Function | If
                    | Finally | For | Foreach | Case | Do | While | As | Catch | Empty | Using
                    | Class | NullLiteral | Super | Where => true,
                    _ => false,
                }
            }
            _ => false,
        };

        let default = |self_: &mut Self| {
            self_.errors.push(Self::make_error_from_node(
                node,
                errors::invalid_constant_initializer,
            ))
        };

        let check_type_specifier = |self_: &mut Self, x: S<'a, Token, Value>, initializer| {
            if let Token(token) = &x.children {
                if is_namey(self_, &token) {
                    return Self::syntax_to_list_no_separators(initializer)
                        .for_each(|x| self_.check_constant_expression(x));
                }
            };
            default(self_)
        };

        let check_collection_members = |self_: &mut Self, x| {
            Self::syntax_to_list_no_separators(x).for_each(|x| self_.check_constant_expression(&x))
        };
        match &node.children {
            Missing | QualifiedName(_) | LiteralExpression(_) => {}
            Token(token) => {
                if !is_namey(self, token) {
                    default(self)
                }
            }
            PrefixUnaryExpression(x) => {
                if let Token(token) = &x.operator.children {
                    use TokenKind::*;
                    match token.kind() {
                        Exclamation | Plus | Minus | Tilde => {
                            self.check_constant_expression(&x.operand)
                        }
                        _ => default(self),
                    }
                } else {
                    default(self)
                }
            }
            BinaryExpression(x) => {
                if let Token(token) = &x.operator.children {
                    use TokenKind::*;
                    match token.kind() {
                        BarBar
                        | AmpersandAmpersand
                        | Carat
                        | Bar
                        | Ampersand
                        | Dot
                        | Plus
                        | Minus
                        | Star
                        | Slash
                        | Percent
                        | LessThanLessThan
                        | GreaterThanGreaterThan
                        | StarStar
                        | EqualEqual
                        | EqualEqualEqual
                        | ExclamationEqual
                        | ExclamationEqualEqual
                        | GreaterThan
                        | GreaterThanEqual
                        | LessThan
                        | LessThanEqual
                        | LessThanEqualGreaterThan
                        | QuestionColon => {
                            self.check_constant_expression(&x.left_operand);
                            self.check_constant_expression(&x.right_operand);
                        }
                        _ => default(self),
                    }
                } else {
                    default(self)
                }
            }
            ConditionalExpression(x) => {
                self.check_constant_expression(&x.test);
                self.check_constant_expression(&x.consequence);
                self.check_constant_expression(&x.alternative);
            }
            SimpleInitializer(x) => {
                if let LiteralExpression(y) = &x.value.children {
                    if let SyntaxList(_) = &y.expression.children {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::invalid_constant_initializer,
                        ))
                    }
                    self.check_constant_expression(&x.value)
                } else {
                    self.check_constant_expression(&x.value)
                }
            }

            ParenthesizedExpression(x) => self.check_constant_expression(&x.expression),
            CollectionLiteralExpression(x) => {
                if let SimpleTypeSpecifier(y) = &x.name.children {
                    check_type_specifier(self, &y.specifier, &x.initializers)
                } else if let GenericTypeSpecifier(y) = &x.name.children {
                    check_type_specifier(self, &y.class_type, &x.initializers)
                } else {
                    default(self)
                };
            }

            TupleExpression(x) => check_collection_members(self, &x.items),
            KeysetIntrinsicExpression(x) => check_collection_members(self, &x.members),
            VarrayIntrinsicExpression(x) => check_collection_members(self, &x.members),
            DarrayIntrinsicExpression(x) => check_collection_members(self, &x.members),
            VectorIntrinsicExpression(x) => check_collection_members(self, &x.members),
            DictionaryIntrinsicExpression(x) => check_collection_members(self, &x.members),
            ShapeExpression(x) => check_collection_members(self, &x.fields),
            ElementInitializer(x) => {
                self.check_constant_expression(&x.key);
                self.check_constant_expression(&x.value);
            }
            FieldInitializer(x) => {
                self.check_constant_expression(&x.name);
                self.check_constant_expression(&x.value);
            }
            ScopeResolutionExpression(x)
                if Self::is_good_scope_resolution_qualifier(&x.qualifier)
                    && is_good_scope_resolution_name(&x.name) => {}
            AsExpression(x) => match &x.right_operand.children {
                LikeTypeSpecifier(_) => self.check_constant_expression(&x.left_operand),
                GenericTypeSpecifier(y)
                    if self.text(&y.class_type) == sn::fb::INCORRECT_TYPE
                        || self.text(&y.class_type) == Self::strip_ns(sn::fb::INCORRECT_TYPE) =>
                {
                    self.check_constant_expression(&x.left_operand)
                }
                _ => default(self),
            },
            FunctionCallExpression(x) => {
                let mut check_receiver_and_arguments = |receiver| {
                    if is_whitelisted_function(self, receiver) {
                        for node in Self::syntax_to_list_no_separators(&x.argument_list) {
                            self.check_constant_expression(node)
                        }
                    } else {
                        default(self)
                    }
                };

                match &x.receiver.children {
                    Token(tok) if tok.kind() == TokenKind::Name => {
                        check_receiver_and_arguments(&x.receiver)
                    }
                    QualifiedName(_) => check_receiver_and_arguments(&x.receiver),
                    _ => default(self),
                }
            }
            FunctionPointerExpression(_) => {
                // Bans the equivalent of inst_meth as well as class_meth and fun
                if self.env.parser_options.po_disallow_func_ptrs_in_constants {
                    default(self)
                }
            }
            ObjectCreationExpression(_) => {
                // We allow "enum class" constants to be initialized via new.
                if !(self.env.parser_options.po_enable_enum_classes && self.is_in_enum_class()) {
                    default(self);
                }
            }
            _ => default(self),
        }
    }

    fn check_static_in_initializer(&mut self, initializer: S<'a, Token, Value>) -> bool {
        if let SimpleInitializer(x) = &initializer.children {
            if let ScopeResolutionExpression(x) = &x.value.children {
                if let Token(t) = &x.qualifier.children {
                    match t.kind() {
                        TokenKind::Static => return true,
                        TokenKind::Parent => {
                            return self.text(&x.name).eq_ignore_ascii_case("class");
                        }
                        _ => return false,
                    }
                }
            }
        };
        false
    }

    fn const_decl_errors(&mut self, node: S<'a, Token, Value>) {
        if let ConstantDeclarator(cd) = &node.children {
            if self.constant_abstract_with_initializer(&cd.initializer) {
                self.check_can_use_feature(node, &UnstableFeatures::ClassConstDefault);
            }

            self.produce_error(
                |self_, x| self_.constant_concrete_without_initializer(x),
                &cd.initializer,
                || errors::error2050,
                &cd.initializer,
            );

            self.produce_error(
                |self_, x| self_.is_global_in_const_decl(x),
                &cd.initializer,
                || errors::global_in_const_decl,
                &cd.initializer,
            );
            self.check_constant_expression(&cd.initializer);

            self.produce_error(
                |self_, x| self_.check_static_in_initializer(x),
                &cd.initializer,
                || errors::parent_static_const_decl,
                &cd.initializer,
            );

            if !cd.name.is_missing() {
                let constant_name = self.text(&cd.name);
                let location = Self::make_location_of_node(&cd.name);
                let def = make_first_use_or_def(
                    false,
                    NameDef,
                    location,
                    &self.namespace_name,
                    constant_name,
                );

                match (
                    self.names.constants.get(constant_name),
                    self.first_parent_class_name(),
                ) {
                    // Only error if this is inside a class
                    (Some(_), Some(class_name)) => {
                        let full_name = class_name.to_string() + "::" + constant_name;
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::redeclaration_error(&full_name),
                        ))
                    }
                    (Some(prev_def), None) if prev_def.kind != NameDef => {
                        let (line_num, _) = self
                            .env
                            .text
                            .offset_to_position(prev_def.location.start_offset as isize);
                        let line_num = line_num as usize;

                        self.errors.push(Self::make_name_already_used_error(
                            &cd.name,
                            &combine_names(&self.namespace_name, &constant_name),
                            &constant_name,
                            &def.location,
                            &|x, y| errors::declared_name_is_already_in_use(line_num, x, y),
                        ))
                    }
                    _ => {}
                }
                self.names.constants.add(constant_name, def)
            }
        }
    }

    fn class_property_modifiers_errors(&mut self, node: S<'a, Token, Value>) {
        if let PropertyDeclaration(x) = &node.children {
            let property_modifiers = &x.modifiers;

            let abstract_static_props = self.env.parser_options.po_abstract_static_props;
            self.invalid_modifier_errors("Properties", node, |kind| {
                if kind == TokenKind::Abstract {
                    return abstract_static_props;
                }
                kind == TokenKind::Static
                    || kind == TokenKind::Private
                    || kind == TokenKind::Protected
                    || kind == TokenKind::Public
                    || kind == TokenKind::Readonly
            });

            self.produce_error(
                |_, x| Self::is_empty_list_or_missing(x),
                property_modifiers,
                || errors::property_requires_visibility,
                node,
            );

            if self.env.parser_options.po_abstract_static_props {
                self.produce_error(
                    |_, n| Self::has_modifier_abstract(n) && !Self::has_modifier_static(n),
                    node,
                    || errors::abstract_instance_property,
                    node,
                );
            }

            if Self::has_modifier_abstract(node) && Self::has_modifier_private(node) {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::elt_abstract_private("properties"),
                ));
            }
        }
    }

    fn class_property_const_errors(&mut self, node: S<'a, Token, Value>) {
        if let PropertyDeclaration(x) = &node.children {
            if self.attr_spec_contains_const(&x.attribute_spec)
                && self.attribute_specification_contains(
                    &x.attribute_spec,
                    sn::user_attributes::LATE_INIT,
                )
            {
                // __LateInit together with const just doesn't make sense.
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_late_init_props,
                ))
            }
        }
    }

    fn class_property_declarator_errors(&mut self, node: S<'a, Token, Value>) {
        let check_decls = |
            self_: &mut Self,
            f: &dyn Fn(S<'a, Token, Value>) -> bool,
            error: errors::Error,
            property_declarators,
        | {
            Self::syntax_to_list_no_separators(property_declarators).for_each(|decl| {
                if let PropertyDeclarator(x) = &decl.children {
                    if f(&x.initializer) {
                        self_
                            .errors
                            .push(Self::make_error_from_node(node, error.clone()))
                    }
                }
            })
        };
        if let PropertyDeclaration(x) = &node.children {
            if self.env.parser_options.tco_const_static_props && Self::has_modifier_static(node) {
                if self.env.parser_options.po_abstract_static_props
                    && Self::has_modifier_abstract(node)
                {
                    check_decls(
                        self,
                        &|n| !n.is_missing(),
                        errors::abstract_prop_init,
                        &x.declarators,
                    )
                } else if self.attr_spec_contains_const(&x.attribute_spec) {
                    check_decls(
                        self,
                        &|n| n.is_missing(),
                        errors::const_static_prop_init,
                        &x.declarators,
                    )
                }
            }
        }
    }

    fn trait_use_alias_item_modifier_errors(&mut self, node: S<'a, Token, Value>) {
        self.invalid_modifier_errors("Trait use aliases", node, |kind| {
            kind == TokenKind::Final
                || kind == TokenKind::Private
                || kind == TokenKind::Protected
                || kind == TokenKind::Public
        });
    }

    fn mixed_namespace_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            NamespaceBody(x) => {
                let s = Self::start_offset(&x.left_brace);
                let e = Self::end_offset(&x.right_brace);
                if let NamespaceType::Unbracketed(Location {
                    start_offset,
                    end_offset,
                }) = self.namespace_type
                {
                    let child = Some(SyntaxError::make(
                        start_offset,
                        end_offset,
                        errors::error2057,
                    ));
                    self.errors.push(SyntaxError::make_with_child_and_type(
                        child,
                        s,
                        e,
                        ErrorType::ParseError,
                        errors::error2052,
                    ))
                }
            }
            NamespaceEmptyBody(x) => {
                let s = Self::start_offset(&x.semicolon);
                let e = Self::end_offset(&x.semicolon);
                if let NamespaceType::Bracketed(Location {
                    start_offset,
                    end_offset,
                }) = self.namespace_type
                {
                    let child = Some(SyntaxError::make(
                        start_offset,
                        end_offset,
                        errors::error2056,
                    ));
                    self.errors.push(SyntaxError::make_with_child_and_type(
                        child,
                        s,
                        e,
                        ErrorType::ParseError,
                        errors::error2052,
                    ))
                }
            }
            NamespaceDeclaration(x) => {
                let mut is_first_decl = true;
                let mut has_code_outside_namespace = false;

                if let [Syntax {
                    children: Script(_),
                    ..
                }, syntax_list] = self.parents.as_slice()
                {
                    if let SyntaxList(_) = syntax_list.children {
                        is_first_decl = false;
                        for decl in Self::syntax_to_list_no_separators(syntax_list) {
                            match &decl.children {
                                MarkupSection(x) => {
                                    if x.hashbang.width() == 0 || self.is_hashbang(&x.hashbang) {
                                        continue;
                                    } else {
                                        break;
                                    }
                                }
                                NamespaceUseDeclaration(_) | FileAttributeSpecification(_) => {}
                                NamespaceDeclaration(_) => {
                                    is_first_decl = true;
                                    break;
                                }
                                _ => break,
                            }
                        }

                        has_code_outside_namespace = !(x.body.is_namespace_empty_body())
                            && Self::syntax_to_list_no_separators(syntax_list).any(|decl| {
                                match &decl.children {
                                    MarkupSection(x) => {
                                        !(x.hashbang.width() == 0 || self.is_hashbang(&x.hashbang))
                                    }
                                    NamespaceDeclaration(_)
                                    | FileAttributeSpecification(_)
                                    | EndOfFile(_)
                                    | NamespaceUseDeclaration(_) => false,
                                    _ => true,
                                }
                            })
                    }
                }

                if !is_first_decl {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::namespace_decl_first_statement,
                    ))
                }
                if has_code_outside_namespace {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::code_outside_namespace,
                    ))
                }
            }
            _ => {}
        }
    }

    fn enumerator_errors(&mut self, node: S<'a, Token, Value>) {
        if let Enumerator(x) = &node.children {
            if self.text(&x.name).eq_ignore_ascii_case("class") {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::enum_elem_name_is_class,
                ))
            }
            self.check_constant_expression(&x.value)
        }
    }

    fn enum_decl_errors(&mut self, node: S<'a, Token, Value>) {
        if let EnumDeclaration(x) = &node.children {
            let attrs = &x.attribute_spec;
            self.check_attr_enabled(&attrs);
            if self.attr_spec_contains_const(attrs) {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_interfaces_traits_enums,
                ))
            }

            if !x.name.is_missing() {
                let name = self.text(&x.name);
                let location = Self::make_location_of_node(&x.name);
                self.check_type_name(&x.name, name, location)
            }
        }
    }

    fn check_lvalue(&mut self, loperand: S<'a, Token, Value>) {
        let append_errors = |self_: &mut Self, node, error| {
            self_.errors.push(Self::make_error_from_node(node, error))
        };

        let err = |self_: &mut Self, error| append_errors(self_, loperand, error);

        let check_unary_expression = |self_: &mut Self, op| match Self::token_kind(op) {
            Some(TokenKind::At) | Some(TokenKind::Dollar) => {}
            _ => err(self_, errors::not_allowed_in_write("Unary expression")),
        };

        let check_variable = |self_: &mut Self, text| {
            if text == sn::special_idents::THIS {
                err(self_, errors::reassign_this)
            }
        };

        match &loperand.children {
            ListExpression(x) => {
                Self::syntax_to_list_no_separators(&x.members).for_each(|n| self.check_lvalue(n))
            }
            SafeMemberSelectionExpression(_) => {
                err(self, errors::not_allowed_in_write("`?->` operator"))
            }
            MemberSelectionExpression(x) => {
                if Self::token_kind(&x.name) == Some(TokenKind::XHPClassName) {
                    err(self, errors::not_allowed_in_write("`->:` operator"))
                }
            }
            CatchClause(x) => check_variable(self, self.text(&x.variable)),
            VariableExpression(x) => check_variable(self, self.text(&x.expression)),
            DecoratedExpression(x) => match Self::token_kind(&x.decorator) {
                Some(TokenKind::Clone) => err(self, errors::not_allowed_in_write("`clone`")),
                Some(TokenKind::Await) => err(self, errors::not_allowed_in_write("`await`")),
                Some(TokenKind::QuestionQuestion) => {
                    err(self, errors::not_allowed_in_write("`??` operator"))
                }
                Some(TokenKind::BarGreaterThan) => {
                    err(self, errors::not_allowed_in_write("`|>` operator"))
                }
                Some(TokenKind::Inout) => err(self, errors::not_allowed_in_write("`inout`")),
                _ => {}
            },
            ParenthesizedExpression(x) => self.check_lvalue(&x.expression),
            SubscriptExpression(x) => self.check_lvalue(&x.receiver),
            LambdaExpression(_)
            | AnonymousFunction(_)
            | AwaitableCreationExpression(_)
            | DarrayIntrinsicExpression(_)
            | VarrayIntrinsicExpression(_)
            | ShapeExpression(_)
            | CollectionLiteralExpression(_)
            | GenericTypeSpecifier(_)
            | YieldExpression(_)
            | CastExpression(_)
            | BinaryExpression(_)
            | ConditionalExpression(_)
            | IsExpression(_)
            | AsExpression(_)
            | NullableAsExpression(_)
            | ConstructorCall(_)
            | AnonymousClass(_)
            | XHPExpression(_)
            | InclusionExpression(_)
            | TupleExpression(_)
            | LiteralExpression(_) => err(
                self,
                errors::not_allowed_in_write(loperand.kind().to_string()),
            ),
            PrefixUnaryExpression(x) => check_unary_expression(self, &x.operator),
            PostfixUnaryExpression(x) => check_unary_expression(self, &x.operator),

            // FIXME: Array_get ((_, Class_const _), _) is not a valid lvalue. *)
            _ => {} // Ideally we should put all the rest of the syntax here so everytime
            // a new syntax is added people need to consider whether the syntax
            // can be a valid lvalue or not. However, there are too many of them.
        }
    }

    fn assignment_errors(&mut self, node: S<'a, Token, Value>) {
        let check_unary_expression = |self_: &mut Self, op, loperand: S<'a, Token, Value>| {
            if Self::does_unop_create_write(Self::token_kind(op)) {
                self_.check_lvalue(loperand)
            }
        };
        match &node.children {
            PrefixUnaryExpression(x) => check_unary_expression(self, &x.operator, &x.operand),
            PostfixUnaryExpression(x) => check_unary_expression(self, &x.operator, &x.operand),
            DecoratedExpression(x) => {
                let loperand = &x.expression;
                if Self::does_decorator_create_write(Self::token_kind(&x.decorator)) {
                    self.check_lvalue(&loperand)
                }
            }
            BinaryExpression(x) => {
                let loperand = &x.left_operand;
                if Self::does_binop_create_write_on_left(Self::token_kind(&x.operator)) {
                    self.check_lvalue(&loperand);
                }
            }
            ForeachStatement(x) => {
                self.check_lvalue(&x.value);
                self.check_lvalue(&x.key);
            }
            CatchClause(_) => {
                self.check_lvalue(node);
            }
            _ => {}
        }
    }

    fn dynamic_method_call_errors(&mut self, node: S<'a, Token, Value>) {
        match &node.children {
            FunctionCallExpression(x) if !x.type_args.is_missing() => {
                let is_variable = |x| Self::is_token_kind(x, TokenKind::Variable);
                let is_dynamic = match &x.receiver.children {
                    ScopeResolutionExpression(x) => is_variable(&x.name),
                    MemberSelectionExpression(x) => is_variable(&x.name),
                    SafeMemberSelectionExpression(x) => is_variable(&x.name),
                    _ => false,
                };
                if is_dynamic {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::no_type_parameters_on_dynamic_method_calls,
                    ))
                }
            }
            _ => {}
        }
    }

    fn get_namespace_name(&self) -> String {
        if let Some(node) = self.nested_namespaces.last() {
            if let NamespaceDeclaration(x) = &node.children {
                if let NamespaceDeclarationHeader(x) = &x.header.children {
                    let ns = &x.name;
                    if !ns.is_missing() {
                        return combine_names(&self.namespace_name, self.text(ns));
                    }
                }
            }
        }
        return self.namespace_name.clone();
    }

    fn is_invalid_hack_mode(&mut self) {
        if self.env.syntax_tree.mode().is_none() {
            let root = self.env.syntax_tree.root();
            let e = Self::make_error_from_node(root, errors::invalid_hack_mode);
            self.errors.push(e);
        }
    }

    fn disabled_legacy_soft_typehint_errors(&mut self, node: S<'a, Token, Value>) {
        if let SoftTypeSpecifier(_) = node.children {
            if self.env.parser_options.po_disable_legacy_soft_typehints {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_legacy_soft_typehints,
                ))
            }
        }
    }

    fn disabled_legacy_attribute_syntax_errors(&mut self, node: S<'a, Token, Value>) {
        match node.children {
            OldAttributeSpecification(_)
                if self.env.parser_options.po_disable_legacy_attribute_syntax =>
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_legacy_attribute_syntax,
                ))
            }
            _ => {}
        }
    }

    fn param_default_decl_errors(&mut self, node: S<'a, Token, Value>) {
        if let ParameterDeclaration(x) = &node.children {
            if self.env.parser_options.po_const_default_lambda_args {
                match self.env.context.active_callable {
                    Some(node) => match node.children {
                        AnonymousFunction(_) | LambdaExpression(_) => {
                            self.check_constant_expression(&x.default_value);
                        }
                        _ => {}
                    },
                    _ => {}
                }
            }
            if self.env.parser_options.po_const_default_func_args {
                self.check_constant_expression(&x.default_value)
            }
        }
    }

    fn concurrent_statement_errors(&mut self, node: S<'a, Token, Value>) {
        if let ConcurrentStatement(x) = &node.children {
            // issue error if concurrent blocks are nested
            if self.is_in_concurrent_block {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::nested_concurrent_blocks,
                ))
            };
            if let CompoundStatement(x) = &x.statement.children {
                let statement_list = || Self::syntax_to_list_no_separators(&x.statements);
                if statement_list().nth(1).is_none() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::fewer_than_two_statements_in_concurrent_block,
                    ))
                }
                for ref n in statement_list() {
                    if let ExpressionStatement(x) = &n.children {
                        if !self.node_has_await_child(&x.expression) {
                            self.errors.push(Self::make_error_from_node(
                                n,
                                errors::statement_without_await_in_concurrent_block,
                            ))
                        }
                    } else {
                        self.errors.push(Self::make_error_from_node(
                            n,
                            errors::invalid_syntax_concurrent_block,
                        ))
                    }
                }
                for ref n in statement_list() {
                    for error in self.find_invalid_lval_usage(n) {
                        self.errors.push(error)
                    }
                }
            } else {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::invalid_syntax_concurrent_block,
                ))
            }
        }
    }

    fn check_qualified_name(&mut self, node: S<'a, Token, Value>) {
        // The last segment in a qualified name should not have a trailing backslash
        // i.e. `Foospace\Bar\` except as the prefix of a GroupUseClause
        if let Some(Syntax {
            children: NamespaceGroupUseDeclaration(_),
            ..
        }) = self.parents.last()
        {
        } else {
            if let QualifiedName(x) = &node.children {
                let name_parts = &x.parts;
                let mut parts = Self::syntax_to_list_with_separators(name_parts);
                let last_part = parts.nth_back(0);
                match last_part {
                    Some(t) if Self::token_kind(t) == Some(TokenKind::Backslash) => self
                        .errors
                        .push(Self::make_error_from_node(t, errors::error0008)),
                    _ => {}
                }
            }
        }
    }

    fn check_preceding_backslashes_qualified_name(&mut self, node: S<'a, Token, Value>) {
        // Qualified names as part of file level declarations
        // (group use, namespace use, namespace declarations) should not have preceding backslashes
        // `use namespace A\{\B}` will throw this error.
        if let QualifiedName(x) = &node.children {
            let name_parts = &x.parts;
            let mut parts = Self::syntax_to_list_with_separators(name_parts);
            let first_part = parts.find(|x| !x.is_missing());

            match first_part {
                Some(t) if Self::token_kind(t) == Some(TokenKind::Backslash) => self.errors.push(
                    Self::make_error_from_node(node, errors::preceding_backslash),
                ),
                _ => {}
            }
        }
    }

    fn strip_ns(name: &str) -> &str {
        match name.chars().next() {
            Some('\\') => &name[1..],
            _ => name,
        }
    }

    fn strip_hh_ns(name: &str) -> &str {
        name.trim_start_matches("\\HH\\")
    }

    fn is_global_namespace(&self) -> bool {
        self.namespace_name == GLOBAL_NAMESPACE_NAME
    }

    fn folder(&mut self, node: S<'a, Token, Value>) {
        let mut prev_context = None;
        let mut pushed_nested_namespace = false;

        let named_function_context =
            |self_: &mut Self, node, s, prev_context: &mut Option<Context<'a, _>>| {
                *prev_context = Some(self_.env.context.clone());
                // a _single_ variable suffices as they cannot be nested
                self_.env.context.active_methodish = Some(node);
                self_.env.context.active_callable = Some(node);
                self_.env.context.active_callable_attr_spec = Some(s);
            };

        let lambda_context =
            |self_: &mut Self, node, s, prev_context: &mut Option<Context<'a, _>>| {
                *prev_context = Some(self_.env.context.clone());
                //preserve context when entering lambdas (and anonymous functions)
                self_.env.context.active_callable = Some(node);
                self_.env.context.active_callable_attr_spec = Some(s);
            };

        match &node.children {
            ConstDeclaration(_) => {
                prev_context = Some(self.env.context.clone());
                self.env.context.active_const = Some(node)
            }
            FunctionDeclaration(x) => {
                named_function_context(self, node, &x.attribute_spec, &mut prev_context)
            }
            MethodishDeclaration(x) => {
                named_function_context(self, node, &x.attribute, &mut prev_context)
            }
            NamespaceDeclaration(x) => {
                if let NamespaceDeclarationHeader(x) = &x.header.children {
                    let namespace_name = &x.name;
                    if !namespace_name.is_missing() && !self.text(namespace_name).is_empty() {
                        pushed_nested_namespace = true;
                        self.nested_namespaces.push(node)
                    }
                }
            }
            AnonymousFunction(x) => {
                lambda_context(self, node, &x.attribute_spec, &mut prev_context)
            }
            LambdaExpression(x) => {
                lambda_context(self, node, &x.attribute_spec, &mut prev_context)
            }
            AwaitableCreationExpression(x) => {
                lambda_context(self, node, &x.attribute_spec, &mut prev_context)
            }
            ClassishDeclaration(_) => {
                prev_context = Some(self.env.context.clone());
                self.env.context.active_classish = Some(node)
            }
            FileAttributeSpecification(_) => Self::attr_spec_to_node_list(node).for_each(|ref node| {
                if self.attr_name(node).as_deref()
                    == Some(sn::user_attributes::ENABLE_UNSTABLE_FEATURES)
                {
                    if let Some(args) = self.attr_args(node) {
                        let mut args = args.peekable();
                        if args.peek().is_none() {
                            self.errors.push(Self::make_error_from_node(
                                node,
                                errors::invalid_use_of_enable_unstable_feature(
                                    format!(
                                        "you didn't select a feature. Available features are:\n\t{}",
                                        UnstableFeatures::iter().join("\n\t")
                                    )
                                    .as_str(),
                                ),
                            ))
                        } else {
                            args.for_each(|ref arg| self.enable_unstable_feature(node, arg))
                        }
                    }
                }
            }),
            _ => (),
        };

        self.parameter_errors(node);

        match &node.children {
            TryStatement(_)
            | UsingStatementFunctionScoped(_)
            | ForStatement(_)
            | CaseLabel(_)
            | DefaultLabel(_) => self.statement_errors(node),
            MethodishDeclaration(_) | FunctionDeclaration(_) | FunctionDeclarationHeader(_) => {
                self.reified_parameter_errors(node);
                self.redeclaration_errors(node);
                self.multiple_entrypoint_attribute_errors(node);
                self.methodish_errors(node);
            }

            LiteralExpression(_)
            | SafeMemberSelectionExpression(_)
            | FunctionCallExpression(_)
            | ListExpression(_)
            | ShapeExpression(_)
            | DecoratedExpression(_)
            | VectorIntrinsicExpression(_)
            | DictionaryIntrinsicExpression(_)
            | KeysetIntrinsicExpression(_)
            | VarrayIntrinsicExpression(_)
            | DarrayIntrinsicExpression(_)
            | YieldExpression(_)
            | ScopeResolutionExpression(_)
            | PrefixUnaryExpression(_)
            | LambdaExpression(_)
            | IsExpression(_)
            | AsExpression(_)
            | AnonymousFunction(_)
            | SubscriptExpression(_)
            | ConstructorCall(_)
            | AwaitableCreationExpression(_)
            | PipeVariableExpression(_)
            | ConditionalExpression(_)
            | CollectionLiteralExpression(_)
            | VariableExpression(_) => {
                self.check_disallowed_variables(node);
                self.dynamic_method_call_errors(node);
                self.expression_errors(node);
                self.assignment_errors(node);
            }

            ParameterDeclaration(_) => self.param_default_decl_errors(node),
            RequireClause(_) => self.require_errors(node),
            ClassishDeclaration(_) => {
                self.classish_errors(node);
                self.class_reified_param_errors(node);
            }

            ConstDeclaration(_) => self.class_constant_modifier_errors(node),

            TypeConstDeclaration(_) => self.type_const_modifier_errors(node),

            AliasDeclaration(_) | ContextAliasDeclaration(_) => self.alias_errors(node),
            ConstantDeclarator(_) => self.const_decl_errors(node),
            NamespaceBody(_) | NamespaceEmptyBody(_) | NamespaceDeclaration(_) => {
                self.mixed_namespace_errors(node)
            }
            NamespaceUseDeclaration(_) | NamespaceGroupUseDeclaration(_) => {
                self.group_use_errors(node);
                self.namespace_use_declaration_errors(node);
            }
            PropertyDeclaration(_) => {
                self.class_property_modifiers_errors(node);
                self.class_reified_param_errors(node);
                self.class_property_const_errors(node);
                self.class_property_declarator_errors(node);
            }
            TraitUseAliasItem(_) => self.trait_use_alias_item_modifier_errors(node),
            EnumDeclaration(_) => self.enum_decl_errors(node),
            Enumerator(_) => self.enumerator_errors(node),
            PostfixUnaryExpression(_)
            | BinaryExpression(_)
            | ForeachStatement(_)
            | CatchClause(_) => self.assignment_errors(node),
            XHPEnumType(_) | XHPExpression(_) => self.xhp_errors(node),
            PropertyDeclarator(x) => {
                let init = &x.initializer;

                self.produce_error(
                    |self_, x| self_.check_static_in_initializer(x),
                    &init,
                    || errors::parent_static_prop_decl,
                    &init,
                );
                self.check_constant_expression(&init)
            }
            RecordField(x) => self.check_constant_expression(&x.init),
            XHPClassAttribute(x) => self.check_constant_expression(&x.initializer),
            OldAttributeSpecification(_) => self.disabled_legacy_attribute_syntax_errors(node),
            SoftTypeSpecifier(_) => self.disabled_legacy_soft_typehint_errors(node),
            QualifiedName(_) => self.check_qualified_name(node),
            UnsetStatement(_) => self.unset_errors(node),
            _ => {}
        }
        self.lval_errors(node);

        match &node.children {
            // todo: lambda
            LambdaExpression(_) | AwaitableCreationExpression(_) | AnonymousFunction(_) => {
                let prev_is_in_concurrent_block = self.is_in_concurrent_block;
                // reset is_in_concurrent_block for functions
                self.is_in_concurrent_block = false;
                // analyze the body of lambda block
                self.fold_child_nodes(node);
                // adjust is_in_concurrent_block in final result
                self.is_in_concurrent_block = prev_is_in_concurrent_block;
            }

            ConcurrentStatement(_) => {
                self.concurrent_statement_errors(node);
                // adjust is_in_concurrent_block in accumulator to dive into the
                let prev_is_in_concurrent_block = self.is_in_concurrent_block;
                self.is_in_concurrent_block = true;
                // analyze the body of concurrent block
                self.fold_child_nodes(node);
                // adjust is_in_concurrent_block in final result
                self.is_in_concurrent_block = prev_is_in_concurrent_block;
            }

            NamespaceBody(x) => {
                if self.namespace_type == Unspecified {
                    self.namespace_type =
                        Bracketed(Self::make_location(&x.left_brace, &x.right_brace))
                }

                let old_namespace_name = self.namespace_name.clone();
                let old_names = self.names.clone();
                // reset names before diving into namespace body,
                // keeping global function names
                self.namespace_name = self.get_namespace_name();
                let names_copy = std::mem::replace(&mut self.names, UsedNames::empty());
                self.names.functions = names_copy.functions.filter(|x| x.global);
                self.fold_child_nodes(node);

                // resume with old set of names and pull back
                // accumulated errors/last seen namespace type
                self.names = old_names;
                self.namespace_name = old_namespace_name;
            }
            NamespaceEmptyBody(x) => {
                if self.namespace_type == Unspecified {
                    self.namespace_type = Unbracketed(Self::make_location_of_node(&x.semicolon))
                }
                self.namespace_name = self.get_namespace_name();
                self.names = UsedNames::empty();
                self.fold_child_nodes(node);
            }
            ClassishDeclaration(_) | AnonymousClass(_) => {
                // Reset the trait require clauses
                // Reset the const declarations
                // Reset the function declarations

                let constants =
                    std::mem::replace(&mut self.names.constants, YesCase(HashMap::default()));
                let functions =
                    std::mem::replace(&mut self.names.functions, NoCase(HashMap::default()));
                let trait_require_clauses = std::mem::replace(
                    &mut self.trait_require_clauses,
                    empty_trait_require_clauses(),
                );

                self.fold_child_nodes(node);

                self.trait_require_clauses = trait_require_clauses;
                self.names.functions = functions;
                self.names.constants = constants;
            }
            PrefixedCodeExpression(_) => {
                prev_context = Some(self.env.context.clone());
                self.env.context.active_expression_tree = true;

                self.fold_child_nodes(node)
            }
            ETSpliceExpression(_) => {
                let previous_state = self.env.context.active_expression_tree;
                self.env.context.active_expression_tree = false;
                self.fold_child_nodes(node);
                self.env.context.active_expression_tree = previous_state;
            }
            _ => self.fold_child_nodes(node),
        }

        match &node.children {
            UnionTypeSpecifier(_) | IntersectionTypeSpecifier(_) => {
                self.check_can_use_feature(node, &UnstableFeatures::UnionIntersectionTypeHints)
            }
            ClassishDeclaration(x) => match &x.where_clause.children {
                WhereClause(_) => {
                    self.check_can_use_feature(&x.where_clause, &UnstableFeatures::ClassLevelWhere)
                }
                _ => {}
            },
            EnumClassLabelExpression(_) => {
                self.check_can_use_feature(node, &UnstableFeatures::EnumClassLabel)
            }
            OldAttributeSpecification(x) => {
                let attributes_string = self.text(&x.attributes);
                let has_via_label = attributes_string
                    .split(',')
                    .any(|attr| attr.trim() == sn::user_attributes::VIA_LABEL);
                if has_via_label {
                    self.check_can_use_feature(node, &UnstableFeatures::EnumClassLabel)
                }
            }
            _ => {}
        }

        if let Some(prev_context) = prev_context {
            self.env.context = prev_context;
        }
        if pushed_nested_namespace {
            assert_eq!(
                self.nested_namespaces.pop().map(|x| x as *const _),
                Some(node as *const _)
            );
        }
    }

    fn fold_child_nodes(&mut self, node: S<'a, Token, Value>) {
        self.parents.push(node);
        for c in node.iter_children() {
            self.folder(c)
        }
        assert_eq!(
            self.parents.pop().map(|x| x as *const _),
            Some(node as *const _)
        );
    }

    fn parse_errors_impl(mut self) -> Vec<SyntaxError> {
        if self.env.is_typechecker() {
            self.is_invalid_hack_mode();
        }
        self.fold_child_nodes(self.env.syntax_tree.root());
        self.errors.reverse();
        self.errors
    }

    fn parse_errors(
        tree: &'a SyntaxTree<'a, Syntax<'a, Token, Value>, State>,
        text: IndexedSourceText<'a>,
        parser_options: ParserOptions,
        hhvm_compat_mode: bool,
        hhi_mode: bool,
        codegen: bool,
    ) -> Vec<SyntaxError> {
        let env = Env {
            parser_options,
            syntax_tree: tree,
            text,
            context: Context {
                active_classish: None,
                active_methodish: None,
                active_callable: None,
                active_callable_attr_spec: None,
                active_const: None,
                active_unstable_features: HashSet::default(),
                active_expression_tree: false,
            },
            hhvm_compat_mode,
            hhi_mode,
            codegen,
        };

        match tree.required_stack_size() {
            None => Self::new(env).parse_errors_impl(),
            Some(stack_size) => {
                // We want to use new thread ONLY for it's capability of adjustable stack size.
                // Rust is against it because SyntaxTree is not a thread safe structure. Moreover,
                // spawned thread could outlive the 'a lifetime.
                // Since the only thing the main thread will do after spawning the child is to wait
                // for it to finish, there will be no actual concurrency, and we can "safely" unsafe
                // it. The usize cast is to fool the borrow checker about the thread lifetime and 'a.
                let raw_pointer = Box::leak(Box::new(Self::new(env))) as *mut Self as usize;
                std::thread::Builder::new()
                    .stack_size(stack_size)
                    .spawn(move || {
                        let self_ = unsafe { Box::from_raw(raw_pointer as *mut Self) };
                        self_.parse_errors_impl()
                    })
                    .expect("ERROR: thread::spawn")
                    .join()
                    .expect("ERROR: failed to wait on new thread")
            }
        }
    }
}

pub fn parse_errors<'a, State: Clone>(
    tree: &'a SyntaxTree<'a, PositionedSyntax<'a>, State>,
    parser_options: ParserOptions,
    hhvm_compat_mode: bool,
    hhi_mode: bool,
    codegen: bool,
) -> Vec<SyntaxError> {
    <ParserErrors<'a, PositionedToken<'a>, PositionedValue<'a>, State>>::parse_errors(
        tree,
        IndexedSourceText::new(tree.text().clone()),
        parser_options,
        hhvm_compat_mode,
        hhi_mode,
        codegen,
    )
}

pub fn parse_errors_with_text<'a, State: Clone>(
    tree: &'a SyntaxTree<'a, PositionedSyntax<'a>, State>,
    text: IndexedSourceText<'a>,
    parser_options: ParserOptions,
    hhvm_compat_mode: bool,
    hhi_mode: bool,
    codegen: bool,
) -> Vec<SyntaxError> {
    <ParserErrors<'a, PositionedToken<'a>, PositionedValue<'a>, State>>::parse_errors(
        tree,
        text,
        parser_options,
        hhvm_compat_mode,
        hhi_mode,
        codegen,
    )
}

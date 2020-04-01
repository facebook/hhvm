// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use lazy_static::lazy_static;
use regex::Regex;
use std::collections::{BTreeMap, HashMap, HashSet};

use naming_special_names_rust as sn;

use oxidized::parser_options::ParserOptions;
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    lexable_token::LexableToken,
    positioned_syntax::PositionedSyntax,
    syntax::{ListItemChildren, Syntax, SyntaxValueType, SyntaxVariant, SyntaxVariant::*},
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

    fn into_iter(self) -> std::collections::hash_map::IntoIter<String, X> {
        match self {
            NoCase(m) => m.into_iter(),
            YesCase(m) => m.into_iter(),
        }
    }
}

use crate::Strmap::*;

fn empty_trait_require_clauses() -> Strmap<TokenKind> {
    NoCase(HashMap::new())
}

#[derive(Clone, Debug)]
struct UsedNames {
    classes: Strmap<FirstUseOrDef>,    // NoCase
    namespaces: Strmap<FirstUseOrDef>, // NoCase
    functions: Strmap<FirstUseOrDef>,  // NoCase
    constants: Strmap<FirstUseOrDef>,  // YesCase
}

impl UsedNames {
    fn empty() -> Self {
        Self {
            classes: NoCase(HashMap::new()),
            namespaces: NoCase(HashMap::new()),
            functions: NoCase(HashMap::new()),
            constants: YesCase(HashMap::new()),
        }
    }
}

struct Context<'a, Syntax> {
    pub active_classish: Option<&'a Syntax>,
    pub active_methodish: Option<&'a Syntax>,
    pub active_callable: Option<&'a Syntax>,
    pub active_callable_attr_spec: Option<&'a Syntax>,
    // true if active callable is reactive if it is a function or method, or there is a reactive
    // proper ancestor (including lambdas) but not beyond the enclosing function or method
    pub active_is_rx_or_enclosing_for_lambdas: bool,
    pub active_const: Option<&'a Syntax>,
}

// TODO: why can't this be auto-derived?
impl<'a, Syntax> std::clone::Clone for Context<'a, Syntax> {
    fn clone(&self) -> Self {
        Self {
            active_classish: self.active_classish,
            active_methodish: self.active_methodish,
            active_callable: self.active_callable,
            active_callable_attr_spec: self.active_callable_attr_spec,
            active_is_rx_or_enclosing_for_lambdas: self.active_is_rx_or_enclosing_for_lambdas,
            active_const: self.active_const,
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

    env: Env<'a, Syntax<Token, Value>, SyntaxTree<'a, Syntax<Token, Value>, State>>,
    errors: Vec<SyntaxError>,
    parents: Vec<&'a Syntax<Token, Value>>,

    trait_require_clauses: Strmap<TokenKind>,
    is_in_concurrent_block: bool,
    names: UsedNames,
    // Named (not anonymous) namespaces that the current expression is enclosed within.
    nested_namespaces: Vec<&'a Syntax<Token, Value>>,
    namespace_type: NamespaceType,
    namespace_name: String,
}

// TODO: why do we need :'a everywhere?
impl<'a, Token: 'a, Value: 'a, State: 'a> ParserErrors<'a, Token, Value, State>
where
    Syntax<Token, Value>: SyntaxTrait,
    Token: LexableToken<'a> + std::fmt::Debug,
    Value: SyntaxValueType<Token> + std::fmt::Debug,
    State: Clone,
{
    fn new(
        env: Env<'a, Syntax<Token, Value>, SyntaxTree<'a, Syntax<Token, Value>, State>>,
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

    fn text(&self, node: &'a Syntax<Token, Value>) -> &'a str {
        node.extract_text(self.env.syntax_tree.text())
            .expect("<text_extraction_failure>")
    }

    fn make_location(s: &'a Syntax<Token, Value>, e: &'a Syntax<Token, Value>) -> Location {
        let start_offset = Self::start_offset(s);
        let end_offset = Self::end_offset(e);
        Location {
            start_offset,
            end_offset,
        }
    }

    fn make_location_of_node(n: &'a Syntax<Token, Value>) -> Location {
        Self::make_location(n, n)
    }

    fn start_offset(n: &'a Syntax<Token, Value>) -> usize {
        // TODO: this logic should be moved to SyntaxTrait::position, when implemented
        n.leading_start_offset() + n.leading_width()
    }

    fn end_offset(n: &'a Syntax<Token, Value>) -> usize {
        // TODO: this logic should be moved to SyntaxTrait::position, when implemented
        let w = n.width();
        let w = if w > 0 { w - 1 } else { w };
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
        node: &'a Syntax<Token, Value>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        use itertools::Either::{Left, Right};
        use std::iter::{empty, once};
        let on_list_item = move |x: &'a ListItemChildren<Token, Value>| {
            if include_separators {
                vec![&x.list_item, &x.list_separator].into_iter()
            } else {
                vec![&x.list_item].into_iter()
            }
        };
        match &node.syntax {
            Missing => Left(Left(empty())),
            SyntaxList(s) => Left(Right(
                s.iter()
                    .map(move |x| match &x.syntax {
                        ListItem(x) => Left(on_list_item(x)),
                        _ => Right(once(x)),
                    })
                    .flatten(),
            )),
            ListItem(x) => Right(Left(on_list_item(x))),
            _ => Right(Right(once(node))),
        }
    }

    fn syntax_to_list_no_separators(
        node: &'a Syntax<Token, Value>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        Self::syntax_to_list(false, node)
    }

    fn syntax_to_list_with_separators(
        node: &'a Syntax<Token, Value>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        Self::syntax_to_list(true, node)
    }

    fn assert_last_in_list<F>(
        assert_fun: F,
        node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>>
    where
        F: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        let mut iter = Self::syntax_to_list_no_separators(node);
        iter.next_back();
        iter.find(|x| assert_fun(*x))
    }

    fn attr_spec_to_node_list(
        node: &'a Syntax<Token, Value>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        use itertools::Either::{Left, Right};
        let f = |attrs| Left(Self::syntax_to_list_no_separators(attrs));
        match &node.syntax {
            AttributeSpecification(x) => f(&x.attribute_specification_attributes),
            OldAttributeSpecification(x) => f(&x.old_attribute_specification_attributes),
            _ => Right(std::iter::empty()),
        }
    }

    fn attr_constructor_call(node: &'a Syntax<Token, Value>) -> &'a SyntaxVariant<Token, Value> {
        match &node.syntax {
            ConstructorCall(_) => &node.syntax,
            Attribute(x) => &x.attribute_attribute_name.syntax,
            _ => &Missing,
        }
    }

    fn attr_name(&self, node: &'a Syntax<Token, Value>) -> Option<&'a str> {
        if let ConstructorCall(x) = Self::attr_constructor_call(node) {
            Some(self.text(&x.constructor_call_type))
        } else {
            None
        }
    }

    fn attr_args(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> Option<impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>>> {
        if let ConstructorCall(x) = Self::attr_constructor_call(node) {
            Some(Self::syntax_to_list_no_separators(
                &x.constructor_call_argument_list,
            ))
        } else {
            None
        }
    }

    fn attribute_specification_contains(&self, node: &'a Syntax<Token, Value>, name: &str) -> bool {
        match &node.syntax {
            AttributeSpecification(_) | OldAttributeSpecification(_) => {
                Self::attr_spec_to_node_list(node).any(|node| self.attr_name(node) == Some(name))
            }
            _ => false,
        }
    }

    fn methodish_contains_attribute(
        &self,
        node: &'a Syntax<Token, Value>,
        attribute: &str,
    ) -> bool {
        match &node.syntax {
            MethodishDeclaration(x) => {
                self.attribute_specification_contains(&x.methodish_attribute, attribute)
            }
            _ => false,
        }
    }

    fn is_decorated_expression<F>(node: &'a Syntax<Token, Value>, f: F) -> bool
    where
        F: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        match &node.syntax {
            DecoratedExpression(x) => f(&x.decorated_expression_decorator),
            _ => false,
        }
    }

    fn test_decorated_expression_child<F>(node: &'a Syntax<Token, Value>, f: F) -> bool
    where
        F: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        match &node.syntax {
            DecoratedExpression(x) => f(&x.decorated_expression_expression),
            _ => false,
        }
    }

    fn is_variadic_expression(node: &'a Syntax<Token, Value>) -> bool {
        Self::is_decorated_expression(node, |x| x.is_ellipsis())
            || Self::test_decorated_expression_child(node, &Self::is_variadic_expression)
    }

    fn is_double_variadic(node: &'a Syntax<Token, Value>) -> bool {
        Self::is_decorated_expression(node, |x| x.is_ellipsis())
            && Self::test_decorated_expression_child(node, &Self::is_variadic_expression)
    }

    fn is_variadic_parameter_variable(node: &'a Syntax<Token, Value>) -> bool {
        // TODO: This shouldn't be a decorated *expression* because we are not
        // expecting an expression at all. We're expecting a declaration.
        Self::is_variadic_expression(node)
    }

    fn is_variadic_parameter_declaration(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            VariadicParameter(_) => true,
            ParameterDeclaration(x) => Self::is_variadic_parameter_variable(&x.parameter_name),
            _ => false,
        }
    }
    fn misplaced_variadic_param(
        param: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        Self::assert_last_in_list(&Self::is_variadic_parameter_declaration, param)
    }
    fn misplaced_variadic_arg(args: &'a Syntax<Token, Value>) -> Option<&'a Syntax<Token, Value>> {
        Self::assert_last_in_list(&Self::is_variadic_expression, args)
    }
    // If a list ends with a variadic parameter followed by a comma, return it
    fn ends_with_variadic_comma(
        params: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
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
    fn variadic_param(params: &'a Syntax<Token, Value>) -> Option<&'a Syntax<Token, Value>> {
        Self::syntax_to_list_with_separators(params)
            .find(|&x| Self::is_variadic_parameter_declaration(x))
    }

    fn is_parameter_with_default_value(param: &'a Syntax<Token, Value>) -> bool {
        match &param.syntax {
            ParameterDeclaration(x) => !x.parameter_default_value.is_missing(),
            _ => false,
        }
    }

    // test a node is a syntaxlist and that the list contains an element
    // satisfying a given predicate
    fn list_contains_predicate<P>(p: P, node: &'a Syntax<Token, Value>) -> bool
    where
        P: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        if let SyntaxList(lst) = &node.syntax {
            lst.iter().any(p)
        } else {
            false
        }
    }

    fn list_first_duplicate_token(
        node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        if let SyntaxList(lst) = &node.syntax {
            let mut seen = BTreeMap::new();
            for node in lst.iter().rev() {
                if let Token(t) = &node.syntax {
                    if let Some(dup) = seen.insert(t.kind(), node) {
                        return Some(dup);
                    }
                }
            }
        }
        None
    }

    fn is_empty_list_or_missing(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            SyntaxList(x) if x.is_empty() => true,
            Missing => true,
            _ => false,
        }
    }

    fn token_kind(node: &'a Syntax<Token, Value>) -> Option<TokenKind> {
        if let Token(t) = &node.syntax {
            return Some(t.kind());
        }
        None
    }

    // Helper function for common code pattern
    fn is_token_kind(node: &'a Syntax<Token, Value>, kind: TokenKind) -> bool {
        Self::token_kind(node) == Some(kind)
    }

    fn active_classish_kind(&self) -> Option<TokenKind> {
        self.env
            .context
            .active_classish
            .and_then(|x| match &x.syntax {
                ClassishDeclaration(cd) => Self::token_kind(&cd.classish_keyword),
                _ => None,
            })
    }

    fn modifiers_of_function_decl_header_exn(
        node: &'a Syntax<Token, Value>,
    ) -> &'a Syntax<Token, Value> {
        match &node.syntax {
            FunctionDeclarationHeader(x) => &x.function_modifiers,
            _ => panic!("expected to get FunctionDeclarationHeader"),
        }
    }

    fn get_modifiers_of_declaration(
        node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        match &node.syntax {
            MethodishDeclaration(x) => Some(Self::modifiers_of_function_decl_header_exn(
                &x.methodish_function_decl_header,
            )),
            FunctionDeclaration(x) => Some(Self::modifiers_of_function_decl_header_exn(
                &x.function_declaration_header,
            )),
            PropertyDeclaration(x) => Some(&x.property_modifiers),
            ConstDeclaration(x) => Some(&x.const_modifiers),
            TypeConstDeclaration(x) => Some(&x.type_const_modifiers),
            ClassishDeclaration(x) => Some(&x.classish_modifiers),
            TraitUseAliasItem(x) => Some(&x.trait_use_alias_item_modifiers),
            _ => None,
        }
    }

    // tests whether the node's modifiers contain one that satisfies [p]
    fn has_modifier_helper<P>(p: P, node: &'a Syntax<Token, Value>) -> bool
    where
        P: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        match Self::get_modifiers_of_declaration(node) {
            Some(x) => Self::list_contains_predicate(p, x),
            None => false,
        }
    }

    // does the node contain the Final keyword in its modifiers
    fn has_modifier_final(node: &'a Syntax<Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_final(), node)
    }

    // does the node contain the Abstract keyword in its modifiers
    fn has_modifier_abstract(node: &'a Syntax<Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_abstract(), node)
    }

    // does the node contain the Static keyword in its modifiers
    fn has_modifier_static(node: &'a Syntax<Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_static(), node)
    }

    // does the node contain the Private keyword in its modifiers
    fn has_modifier_private(node: &'a Syntax<Token, Value>) -> bool {
        Self::has_modifier_helper(|x| x.is_private(), node)
    }

    fn is_visibility(x: &'a Syntax<Token, Value>) -> bool {
        x.is_public() || x.is_private() || x.is_protected()
    }

    fn contains_async_not_last(mods: &'a Syntax<Token, Value>) -> bool {
        let mut mod_list = Self::syntax_to_list_no_separators(mods);
        match mod_list.next_back() {
            Some(x) if x.is_async() => false,
            _ => mod_list.any(|x| x.is_async()),
        }
    }

    fn has_static<F>(&self, node: &'a Syntax<Token, Value>, f: F) -> bool
    where
        F: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        match &node.syntax {
            FunctionDeclarationHeader(node) => {
                let label = &node.function_name;
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

    fn is_clone(&self, label: &'a Syntax<Token, Value>) -> bool {
        self.text(label).eq_ignore_ascii_case(sn::members::__CLONE)
    }

    fn class_constructor_has_static(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.has_static(node, |x| x.is_construct())
    }

    fn clone_cannot_be_static(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.has_static(node, |x| self.is_clone(x))
    }

    fn promoted_params(
        params: impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        params.filter(|node| match &node.syntax {
            ParameterDeclaration(x) => !x.parameter_visibility.is_missing(),
            _ => false,
        })
    }

    // Given a function declaration header, confirm that it is NOT a constructor
    // and that the header containing it has visibility modifiers in parameters
    fn class_non_constructor_has_visibility_param(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(node) => {
                let params = Self::syntax_to_list_no_separators(&node.function_parameter_list);
                (!&node.function_name.is_construct())
                    && Self::promoted_params(params).next().is_some()
            }
            _ => false,
        }
    }

    // Don't allow a promoted parameter in a constructor if the class
    // already has a property with the same name. Return the clashing name found.
    fn class_constructor_param_promotion_clash(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> Option<&str> {
        use itertools::Either::{Left, Right};
        let class_elts = |node: Option<&'a Syntax<Token, Value>>| match node.map(|x| &x.syntax) {
            Some(ClassishDeclaration(cd)) => match &cd.classish_body.syntax {
                ClassishBody(cb) => Left(Self::syntax_to_list_no_separators(
                    &cb.classish_body_elements,
                )),
                _ => Right(std::iter::empty()),
            },
            _ => Right(std::iter::empty()),
        };

        // A property declaration may include multiple property names:
        // public int $x, $y;
        let prop_names = |elt: &'a Syntax<Token, Value>| match &elt.syntax {
            PropertyDeclaration(x) => Left(
                Self::syntax_to_list_no_separators(&x.property_declarators).filter_map(|decl| {
                    match &decl.syntax {
                        PropertyDeclarator(x) => Some(self.text(&x.property_name)),
                        _ => None,
                    }
                }),
            ),
            _ => Right(std::iter::empty()),
        };

        let param_name = |p: &'a Syntax<Token, Value>| match &p.syntax {
            ParameterDeclaration(x) => Some(self.text(&x.parameter_name)),
            _ => None,
        };

        match &node.syntax {
            FunctionDeclarationHeader(node) if node.function_name.is_construct() => {
                let class_var_names: Vec<_> = class_elts(self.env.context.active_classish)
                    .map(|x| prop_names(x))
                    .flatten()
                    .collect();
                let params = Self::syntax_to_list_no_separators(&node.function_parameter_list);
                let mut promoted_param_names = Self::promoted_params(params).filter_map(param_name);
                promoted_param_names.find(|name| class_var_names.contains(name))
            }
            _ => None,
        }
    }

    // Ban parameter promotion in abstract constructors.
    fn abstract_class_constructor_has_visibility_param(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(node) => {
                let label = &node.function_name;
                let params = Self::syntax_to_list_no_separators(&node.function_parameter_list);
                label.is_construct()
                    && Self::list_contains_predicate(|x| x.is_abstract(), &node.function_modifiers)
                    && Self::promoted_params(params).next().is_some()
            }
            _ => false,
        }
    }

    // Ban parameter promotion in interfaces and traits.
    fn interface_or_trait_has_visibility_param(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(node) => {
                let is_interface_or_trait =
                    self.env
                        .context
                        .active_classish
                        .map_or(false, |parent_classish| match &parent_classish.syntax {
                            ClassishDeclaration(cd) => {
                                let kind = Self::token_kind(&cd.classish_keyword);
                                kind == Some(TokenKind::Interface) || kind == Some(TokenKind::Trait)
                            }
                            _ => false,
                        });

                let params = Self::syntax_to_list_no_separators(&node.function_parameter_list);
                Self::promoted_params(params).next().is_some() && is_interface_or_trait
            }
            _ => false,
        }
    }

    // check that a constructor is type annotated
    fn class_constructor_has_non_void_type(&self, node: &'a Syntax<Token, Value>) -> bool {
        if !self.env.is_typechecker() {
            false
        } else {
            match &node.syntax {
                FunctionDeclarationHeader(node) => {
                    let label = &node.function_name;
                    let type_ano = &node.function_type;
                    let function_colon = &node.function_colon;
                    let is_missing = type_ano.is_missing() && function_colon.is_missing();
                    let is_void = match &type_ano.syntax {
                        SimpleTypeSpecifier(spec) => spec.simple_type_specifier.is_void(),
                        _ => false,
                    };
                    label.is_construct() && !(is_missing || is_void)
                }
                _ => false,
            }
        }
    }

    fn async_magic_method(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(node) => {
                let name = self.text(&node.function_name).to_ascii_lowercase();
                match name {
                    _ if name.eq_ignore_ascii_case(sn::members::__DISPOSE_ASYNC) => false,
                    _ if sn::members::AS_LOWERCASE_SET.contains(&name) => {
                        Self::list_contains_predicate(|x| x.is_async(), &node.function_modifiers)
                    }
                    _ => false,
                }
            }
            _ => false,
        }
    }

    fn call_static_method(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(node) => self
                .text(&node.function_name)
                .eq_ignore_ascii_case(sn::members::__CALL_STATIC),

            _ => false,
        }
    }

    fn clone_takes_no_arguments(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclarationHeader(x) => {
                let mut params = Self::syntax_to_list_no_separators(&x.function_parameter_list);
                self.is_clone(&x.function_name) && params.next().is_some()
            }
            _ => false,
        }
    }

    // whether a methodish decl has body
    fn methodish_has_body(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            MethodishDeclaration(syntax) => !syntax.methodish_function_body.is_missing(),
            _ => false,
        }
    }

    // whether a methodish decl is native
    fn methodish_is_native(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.methodish_contains_attribute(node, "__Native")
    }

    // By checking the third parent of a methodish node, tests whether the methodish
    // node is inside an interface.
    fn methodish_inside_interface(&self) -> bool {
        self.env
            .context
            .active_classish
            .iter()
            .any(|parent_classish| match &parent_classish.syntax {
                ClassishDeclaration(cd) => {
                    Self::token_kind(&cd.classish_keyword) == Some(TokenKind::Interface)
                }
                _ => false,
            })
    }

    // Test whether node is a non-abstract method without a body and not native.
    // Here node is the methodish node
    // And methods inside interfaces are inherently considered abstract *)
    fn methodish_non_abstract_without_body_not_native(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> bool {
        let non_abstract =
            !(Self::has_modifier_abstract(node) || self.methodish_inside_interface());
        let not_has_body = !Self::methodish_has_body(node);
        let not_native = !self.methodish_is_native(node);
        let not_hhi = !self.env.is_hhi_mode();

        not_hhi && non_abstract && not_has_body && not_native
    }

    // Test whether node is a method that is both abstract and private
    fn methodish_abstract_conflict_with_private(node: &'a Syntax<Token, Value>) -> bool {
        let is_abstract = Self::has_modifier_abstract(node);
        let has_private = Self::has_modifier_private(node);
        is_abstract && has_private
    }

    // Test whether node is a method that is both abstract and final
    fn methodish_abstract_conflict_with_final(node: &'a Syntax<Token, Value>) -> bool {
        let is_abstract = Self::has_modifier_abstract(node);
        let has_final = Self::has_modifier_final(node);
        is_abstract && has_final
    }

    fn methodish_abstract_inside_interface(&self, node: &'a Syntax<Token, Value>) -> bool {
        let is_abstract = Self::has_modifier_abstract(node);
        let is_in_interface = self.methodish_inside_interface();
        is_abstract && is_in_interface
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
                    syntax: CompoundStatement(_),
                    ..
                }),
                Some(x),
            ) => match &x.syntax {
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
        start_node: &'a Syntax<Token, Value>,
        end_node: &'a Syntax<Token, Value>,
        error_type: ErrorType,
        error: errors::Error,
    ) -> SyntaxError {
        let s = Self::start_offset(start_node);
        let e = Self::end_offset(end_node);
        SyntaxError::make_with_child_and_type(child, s, e, error_type, error)
    }

    fn make_error_from_node(node: &'a Syntax<Token, Value>, error: errors::Error) -> SyntaxError {
        Self::make_error_from_nodes(None, node, node, ErrorType::ParseError, error)
    }

    fn make_error_from_node_with_type(
        node: &'a Syntax<Token, Value>,
        error: errors::Error,
        error_type: ErrorType,
    ) -> SyntaxError {
        Self::make_error_from_nodes(None, node, node, error_type, error)
    }

    fn is_invalid_xhp_attr_enum_item_literal(literal_expression: &'a Syntax<Token, Value>) -> bool {
        if let Token(t) = &literal_expression.syntax {
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

    fn is_invalid_xhp_attr_enum_item(node: &'a Syntax<Token, Value>) -> bool {
        if let LiteralExpression(x) = &node.syntax {
            Self::is_invalid_xhp_attr_enum_item_literal(&x.literal_expression)
        } else {
            true
        }
    }

    fn xhp_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            XHPEnumType(enum_type) => {
                if self.env.is_typechecker() && enum_type.xhp_enum_values.is_missing() {
                    self.errors.push(Self::make_error_from_node(
                        &enum_type.xhp_enum_values,
                        errors::error2055,
                    ))
                } else if self.env.is_typechecker() {
                    Self::syntax_to_list_no_separators(&enum_type.xhp_enum_values)
                        .filter(|&x| Self::is_invalid_xhp_attr_enum_item(x))
                        .for_each(|item| {
                            self.errors
                                .push(Self::make_error_from_node(item, errors::error2063))
                        })
                }
            }
            XHPExpression(x) => {
                if let XHPOpen(xhp_open) = &x.xhp_open.syntax {
                    if let XHPClose(xhp_close) = &x.xhp_close.syntax {
                        let open_tag = self.text(&xhp_open.xhp_open_name);
                        let close_tag = self.text(&xhp_close.xhp_close_name);
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

    fn invalid_modifier_errors<F>(&mut self, decl_name: &str, node: &'a Syntax<Token, Value>, ok: F)
    where
        F: Fn(TokenKind) -> bool,
    {
        if let Some(modifiers) = Self::get_modifiers_of_declaration(node) {
            for modifier in Self::syntax_to_list_no_separators(modifiers) {
                if let Some(kind) = Self::token_kind(modifier) {
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
            if let SyntaxList(modifiers) = &modifiers.syntax {
                let modifiers: Vec<&'a Syntax<Token, Value>> = modifiers
                    .iter()
                    .filter(|x: &&'a Syntax<Token, Value>| Self::is_visibility(*x))
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
        error_node: &'a Syntax<Token, Value>,
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
        F: Fn(&'b X) -> Option<&'a Syntax<Token, Value>>,
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
    fn extract_function_name(&self, header_node: &'a Syntax<Token, Value>) -> Option<&'a str> {
        // The '_' arm of this match will never be reached, but the type checker
        // doesn't allow a direct extraction of function_name from
        // function_declaration_header. *)
        match &header_node.syntax {
            FunctionDeclarationHeader(fdh) => Some(self.text(&fdh.function_name)),
            _ => None,
        }
    }

    // Return, as a string opt, the name of the function or method given the context *)
    fn first_parent_function_name(&self) -> Option<&str> {
        // Note: matching on either is sound because functions and/or methods cannot be nested

        match self.env.context.active_methodish {
            Some(Syntax {
                syntax: FunctionDeclaration(x),
                ..
            }) => self.extract_function_name(&x.function_declaration_header),
            Some(Syntax {
                syntax: MethodishDeclaration(x),
                ..
            }) => self.extract_function_name(&x.methodish_function_decl_header),
            _ => None,
        }
    }

    // Given a particular TokenKind::(Trait/Interface), tests if a given
    // classish_declaration node is both of that kind and declared abstract.
    fn is_classish_kind_declared_abstract(&self, cd_node: &'a Syntax<Token, Value>) -> bool {
        match &cd_node.syntax {
            ClassishDeclaration(x)
                if Self::is_token_kind(&x.classish_keyword, TokenKind::Trait)
                    || Self::is_token_kind(&x.classish_keyword, TokenKind::Interface) =>
            {
                Self::list_contains_predicate(|x| x.is_abstract(), &x.classish_modifiers)
            }
            _ => false,
        }
    }

    fn is_immediately_in_lambda(&self) -> bool {
        self.env
            .context
            .active_callable
            .map_or(false, |node| match &node.syntax {
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
    fn first_parent_classish_node(
        &self,
        classish_kind: TokenKind,
    ) -> Option<&'a Syntax<Token, Value>> {
        self.env
            .context
            .active_classish
            .and_then(|node| match &node.syntax {
                ClassishDeclaration(cd)
                    if Self::is_token_kind(&cd.classish_keyword, classish_kind) =>
                {
                    Some(node)
                }
                _ => None,
            })
    }

    // Return, as a string opt, the name of the closest enclosing classish entity in
    // the given context (not just Classes )
    fn active_classish_name(&self) -> Option<&'a str> {
        self.env.context.active_classish.and_then(|node| {
            if let ClassishDeclaration(cd) = &node.syntax {
                cd.classish_name.extract_text(self.env.syntax_tree.text())
            } else {
                None
            }
        })
    }

    // Return, as a string opt, the name of the Class in the given context
    fn first_parent_class_name(&self) -> Option<&'a str> {
        return self
            .env
            .context
            .active_classish
            .and_then(|parent_classish| {
                if let ClassishDeclaration(cd) = &parent_classish.syntax {
                    if Self::token_kind(&cd.classish_keyword) == Some(TokenKind::Class) {
                        return self.active_classish_name();
                    } else {
                        return None; // This arm is never reached
                    }
                }
                return None;
            });
    }

    // Given a declaration node, returns the modifier node matching the given
    // predicate from its list of modifiers, or None if there isn't one.
    fn extract_keyword<F>(
        modifier: F,
        declaration_node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>>
    where
        F: Fn(&'a Syntax<Token, Value>) -> bool,
    {
        Self::get_modifiers_of_declaration(declaration_node).and_then(|modifiers_list| {
            Self::syntax_to_list_no_separators(modifiers_list)
                .find(|x: &&'a Syntax<Token, Value>| modifier(*x))
        })
    }

    // Wrapper function that uses above extract_keyword function to test if node
    // contains is_abstract keyword
    fn is_abstract_declaration(declaration_node: &'a Syntax<Token, Value>) -> bool {
        Self::extract_keyword(|x| x.is_abstract(), declaration_node).is_some()
    }

    // Tests if the immediate classish parent is an interface.
    fn is_inside_interface(&self) -> bool {
        self.first_parent_classish_node(TokenKind::Interface)
            .is_some()
    }

    // Tests if the immediate classish parent is a trait.
    fn is_inside_trait(&self) -> bool {
        self.first_parent_classish_node(TokenKind::Trait).is_some()
    }

    fn is_abstract_and_async_method(md_node: &'a Syntax<Token, Value>) -> bool {
        Self::is_abstract_declaration(md_node)
            && Self::extract_keyword(|x| x.is_async(), md_node).is_some()
    }

    fn is_interface_and_async_method(&self, md_node: &'a Syntax<Token, Value>) -> bool {
        self.is_inside_interface() && Self::extract_keyword(|x| x.is_async(), md_node).is_some()
    }

    fn get_params_for_enclosing_callable(&self) -> Option<&'a Syntax<Token, Value>> {
        let from_header = |header: &'a Syntax<Token, Value>| match &header.syntax {
            FunctionDeclarationHeader(fdh) => Some(&fdh.function_parameter_list),
            _ => None,
        };
        self.env
            .context
            .active_callable
            .and_then(|callable| match &callable.syntax {
                FunctionDeclaration(x) => from_header(&x.function_declaration_header),
                MethodishDeclaration(x) => from_header(&x.methodish_function_decl_header),
                LambdaExpression(x) => match &x.lambda_signature.syntax {
                    LambdaSignature(x) => Some(&x.lambda_parameters),
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
                syntax: FunctionDeclaration(x),
                ..
            }) => from_attr_spec(&x.function_attribute_spec),
            Some(Syntax {
                syntax: MethodishDeclaration(x),
                ..
            }) => from_attr_spec(&x.methodish_attribute),
            _ => false,
        }
    }

    fn is_parameter_with_callconv(param: &'a Syntax<Token, Value>) -> bool {
        match &param.syntax {
            ParameterDeclaration(x) => !x.parameter_call_convention.is_missing(),
            ClosureParameterTypeSpecifier(x) => !x.closure_parameter_call_convention.is_missing(),
            VariadicParameter(x) => !x.variadic_parameter_call_convention.is_missing(),
            _ => false,
        }
    }

    fn has_inout_params(&self) -> bool {
        self.get_params_for_enclosing_callable()
            .map_or(false, |function_parameter_list| {
                Self::syntax_to_list_no_separators(function_parameter_list)
                    .any(|x| Self::is_parameter_with_callconv(x))
            })
    }

    fn is_inside_async_method(&self) -> bool {
        let from_header = |header: &'a Syntax<Token, Value>| match &header.syntax {
            FunctionDeclarationHeader(fdh) => {
                Self::syntax_to_list_no_separators(&fdh.function_modifiers).any(|x| x.is_async())
            }
            _ => false,
        };
        self.env
            .context
            .active_callable
            .map_or(false, |node| match &node.syntax {
                FunctionDeclaration(x) => from_header(&x.function_declaration_header),
                MethodishDeclaration(x) => from_header(&x.methodish_function_decl_header),
                AnonymousFunction(x) => !x.anonymous_async_keyword.is_missing(),
                LambdaExpression(x) => !x.lambda_async.is_missing(),
                AwaitableCreationExpression(_) => true,
                _ => false,
            })
    }

    fn make_name_already_used_error(
        node: &'a Syntax<Token, Value>,
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

    fn check_type_hint(&mut self, node: &'a Syntax<Token, Value>) {
        for x in node.iter_children() {
            self.check_type_hint(x)
        }
        let check_type_name = |self_: &mut Self, s| {
            self_.check_type_name_reference(self_.text(s), Self::make_location_of_node(node))
        };
        match &node.syntax {
            SimpleTypeSpecifier(x) => check_type_name(self, &x.simple_type_specifier),
            GenericTypeSpecifier(x) => check_type_name(self, &x.generic_class_type),
            _ => (),
        }
    }

    fn extract_callconv_node(node: &'a Syntax<Token, Value>) -> Option<&'a Syntax<Token, Value>> {
        match &node.syntax {
            ParameterDeclaration(x) => Some(&x.parameter_call_convention),
            ClosureParameterTypeSpecifier(x) => Some(&x.closure_parameter_call_convention),
            VariadicParameter(x) => Some(&x.variadic_parameter_call_convention),
            _ => None,
        }
    }

    // Given a node, checks if it is a abstract ConstDeclaration
    fn is_abstract_const(declaration: &'a Syntax<Token, Value>) -> bool {
        match &declaration.syntax {
            ConstDeclaration(_) => Self::has_modifier_abstract(declaration),
            _ => false,
        }
    }

    // Given a ConstDeclarator node, test whether it is abstract, but has an
    // initializer.
    fn constant_abstract_with_initializer(&self, init: &'a Syntax<Token, Value>) -> bool {
        let is_abstract = match self.env.context.active_const {
            Some(p_const_declaration) if Self::is_abstract_const(p_const_declaration) => true,
            _ => false,
        };
        let has_initializer = !init.is_missing();
        is_abstract && has_initializer
    }

    // Given a node, checks if it is a concrete ConstDeclaration *)
    fn is_concrete_const(declaration: &'a Syntax<Token, Value>) -> bool {
        match &declaration.syntax {
            ConstDeclaration(_) => !Self::has_modifier_abstract(declaration),
            _ => false,
        }
    }

    // Given a ConstDeclarator node, test whether it is concrete, but has no
    // initializer.
    fn constant_concrete_without_initializer(&self, init: &'a Syntax<Token, Value>) -> bool {
        let is_concrete = match self.env.context.active_const {
            Some(p_const_declaration) if Self::is_concrete_const(p_const_declaration) => true,
            _ => false,
        };
        is_concrete && init.is_missing()
    }

    fn methodish_memoize_lsb_on_non_static(&mut self, node: &'a Syntax<Token, Value>) {
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
        node: &'a Syntax<Token, Value>,
        attribute: &str,
    ) -> bool {
        match &node.syntax {
            FunctionDeclaration(x) => {
                self.attribute_specification_contains(&x.function_attribute_spec, attribute)
            }
            _ => false,
        }
    }

    fn methodish_contains_memoize(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.env.is_typechecker()
            && self.is_inside_interface()
            && self.methodish_contains_attribute(node, sn::user_attributes::MEMOIZE)
    }

    fn is_some_reactivity_attribute_name(name: &str) -> bool {
        name == sn::user_attributes::REACTIVE
            || name == sn::user_attributes::SHALLOW_REACTIVE
            || name == sn::user_attributes::LOCAL_REACTIVE
            || name == sn::user_attributes::NON_RX
            || name == sn::user_attributes::PURE
    }

    fn is_some_reactivity_attribute(&self, node: &'a Syntax<Token, Value>) -> bool {
        match self.attr_name(node) {
            None => false,
            Some(name) => Self::is_some_reactivity_attribute_name(name),
        }
    }

    fn attribute_first_reactivity_annotation(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        match &node.syntax {
            AttributeSpecification(_) | OldAttributeSpecification(_) => {
                Self::attr_spec_to_node_list(node).find(|x| self.is_some_reactivity_attribute(x))
            }
            _ => None,
        }
    }

    fn attribute_has_reactivity_annotation(&self, attr_spec: &'a Syntax<Token, Value>) -> bool {
        self.attribute_first_reactivity_annotation(attr_spec)
            .is_some()
    }

    fn attribute_missing_reactivity_for_condition(
        &self,
        attr_spec: &'a Syntax<Token, Value>,
    ) -> bool {
        let has_attr = |attr| self.attribute_specification_contains(attr_spec, attr);
        !(self.attribute_has_reactivity_annotation(attr_spec))
            && (has_attr(sn::user_attributes::ONLY_RX_IF_IMPL)
                || has_attr(sn::user_attributes::AT_MOST_RX_AS_ARGS))
    }

    fn error_if_memoize_function_returns_mutable(&mut self, attrs: &'a Syntax<Token, Value>) {
        let mut has_memoize = false;
        let mut mutable_node = None;
        let mut mut_return_node = None;
        for node in Self::attr_spec_to_node_list(attrs) {
            match self.attr_name(node) {
                Some(n) if n == sn::user_attributes::MUTABLE_RETURN => mut_return_node = Some(node),
                Some(n)
                    if n == sn::user_attributes::MEMOIZE
                        || n == sn::user_attributes::MEMOIZE_LSB =>
                {
                    has_memoize = true
                }
                Some(n) if n == sn::user_attributes::MUTABLE => mutable_node = Some(node),
                _ => (),
            }
        }

        if has_memoize {
            if let Some(n) = mutable_node {
                self.errors.push(Self::make_error_from_node(
                    n,
                    errors::mutable_parameter_in_memoize_function(true),
                ))
            }
            if let Some(n) = mut_return_node {
                self.errors.push(Self::make_error_from_node(
                    n,
                    errors::mutable_return_in_memoize_function,
                ))
            }
        }
    }

    fn methodish_missing_reactivity_for_condition(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            MethodishDeclaration(x) => {
                self.attribute_missing_reactivity_for_condition(&x.methodish_attribute)
            }
            _ => false,
        }
    }

    fn methodish_contains_owned_mutable_attribute(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.methodish_contains_attribute(node, sn::user_attributes::OWNED_MUTABLE)
    }

    fn check_nonrx_annotation(&mut self, node: &'a Syntax<Token, Value>) {
        let err_decl = |self_: &mut Self| {
            self_.errors.push(Self::make_error_from_node(
                node,
                errors::invalid_non_rx_argument_for_declaration,
            ))
        };
        let err_lambda = |self_: &mut Self| {
            self_.errors.push(Self::make_error_from_node(
                node,
                errors::invalid_non_rx_argument_for_lambda,
            ))
        };
        let attr_spec = match &node.syntax {
            MethodishDeclaration(x) => Some((&x.methodish_attribute, true)),
            FunctionDeclaration(x) => Some((&x.function_attribute_spec, true)),
            AnonymousFunction(x) => Some((&x.anonymous_attribute_spec, false)),
            LambdaExpression(x) => Some((&x.lambda_attribute_spec, false)),
            AwaitableCreationExpression(x) => Some((&x.awaitable_attribute_spec, false)),
            _ => None,
        };

        if let Some((node, is_decl)) = attr_spec {
            // try find argument list
            let args_opt = Self::attr_spec_to_node_list(node)
                .find(|node| self.attr_name(node) == Some(sn::user_attributes::NON_RX))
                .and_then(|x| self.attr_args(x));

            let is_string_argument = |x: &'a Syntax<Token, Value>| match &x.syntax {
                LiteralExpression(x) => match &x.literal_expression.syntax {
                    Token(token) => {
                        token.kind() == TokenKind::DoubleQuotedStringLiteral
                            || token.kind() == TokenKind::SingleQuotedStringLiteral
                    }
                    _ => false,
                },
                _ => false,
            };

            if let Some(mut args_opt) = args_opt {
                let first_arg = args_opt.next();
                let second_arg = args_opt.next();
                match (first_arg, second_arg) {
                    // __NonRx attribute is found and argument list is empty.
                    // This is ok for lambdas but error for declarations
                    (None, _) => {
                        if is_decl {
                            err_decl(self)
                        }
                    }
                    // __NonRx attribute is found with single string argument.
                    // This is ok for declarations for not allowed for lambdas *)
                    (Some(arg), None) if is_string_argument(arg) => {
                        if !is_decl {
                            err_lambda(self)
                        }
                    }
                    // __NonRx attribute is found but argument list is not suitable
                    // nor for declarations, neither for lambdas
                    _ => {
                        if is_decl {
                            err_decl(self)
                        } else {
                            err_lambda(self)
                        }
                    }
                }
            }
        } else {
            // __NonRx attribute not found
        }
    }

    fn function_missing_reactivity_for_condition(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclaration(x) => {
                self.attribute_missing_reactivity_for_condition(&x.function_attribute_spec)
            }
            _ => false,
        }
    }

    fn function_declaration_contains_only_rx_if_impl_attribute(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> bool {
        self.function_declaration_contains_attribute(node, sn::user_attributes::ONLY_RX_IF_IMPL)
    }

    fn function_declaration_contains_owned_mutable_attribute(
        &self,
        node: &'a Syntax<Token, Value>,
    ) -> bool {
        self.function_declaration_contains_attribute(node, sn::user_attributes::OWNED_MUTABLE)
    }

    fn attribute_multiple_reactivity_annotations(
        &self,
        attr_spec: &'a Syntax<Token, Value>,
    ) -> bool {
        match &attr_spec.syntax {
            OldAttributeSpecification(_) | AttributeSpecification(_) => {
                Self::attr_spec_to_node_list(attr_spec)
                    .filter(|x| self.is_some_reactivity_attribute(x))
                    .take(2)
                    .count()
                    > 1
            }
            _ => false,
        }
    }

    fn methodish_multiple_reactivity_annotations(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            MethodishDeclaration(x) => {
                self.attribute_multiple_reactivity_annotations(&x.methodish_attribute)
            }
            _ => false,
        }
    }

    fn function_multiple_reactivity_annotations(&self, node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            FunctionDeclaration(x) => {
                self.attribute_multiple_reactivity_annotations(&x.function_attribute_spec)
            }
            _ => false,
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

    fn special_method_param_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let FunctionDeclarationHeader(x) = &node.syntax {
            let function_name = &x.function_name;
            let function_parameter_list = &x.function_parameter_list;
            let name = self.text(function_name).to_ascii_lowercase();

            if !sn::members::AS_LOWERCASE_SET.contains(&name) {
                return;
            }
            let params = || Self::syntax_to_list_no_separators(function_parameter_list);
            let len = params().count();

            let full_name = self
                .first_parent_class_name()
                .map(|c_name| c_name.to_string() + "::" + &name + "()")
                .unwrap_or(name.to_string());

            let s = name;
            let num_args_opt = match s {
                _ if s == sn::members::__CALL && len != 2 => Some(2),
                _ if s == sn::members::__GET && len != 1 => Some(1),
                _ if s == sn::members::__SET && len != 2 => Some(2),
                _ if s == sn::members::__ISSET && len != 1 => Some(1),
                _ if s == sn::members::__UNSET && len != 1 => Some(1),
                _ => None,
            };

            if let Some(n) = num_args_opt {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::invalid_number_of_args(&full_name, n),
                ))
            }
            if s == sn::members::__CALL
                || s == sn::members::__GET
                || s == sn::members::__SET
                || s == sn::members::__ISSET
                || s == sn::members::__UNSET
            {
                // disallow inout parameters on magic methods
                if params().any(&Self::is_parameter_with_callconv) {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_inout_args(&full_name),
                    ))
                }
            }
        }
    }

    fn is_in_reified_class(&self) -> bool {
        let active_classish = match self.env.context.active_classish {
            Some(x) => x,
            _ => return false,
        };
        if let ClassishDeclaration(x) = &active_classish.syntax {
            if let TypeParameters(x) = &x.classish_type_parameters.syntax {
                return Self::syntax_to_list_no_separators(&x.type_parameters_parameters).any(
                    |p| match &p.syntax {
                        TypeParameter(x) => !x.type_reified.is_missing(),
                        _ => false,
                    },
                );
            }
        };
        false
    }

    fn methodish_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            FunctionDeclarationHeader(x) => {
                let function_parameter_list = &x.function_parameter_list;
                let function_type = &x.function_type;

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
                let function_attrs = &fd.function_attribute_spec;
                self.produce_error(
                    |self_, x| Self::function_multiple_reactivity_annotations(self_, x),
                    node,
                    || errors::multiple_reactivity_annotations,
                    function_attrs,
                );
                self.error_if_memoize_function_returns_mutable(function_attrs);

                self.produce_error(
                    |self_, x| {
                        Self::function_declaration_contains_only_rx_if_impl_attribute(self_, x)
                    },
                    node,
                    || errors::functions_cannot_implement_reactive,
                    function_attrs,
                );
                self.check_nonrx_annotation(node);

                self.produce_error(
                    |self_, x| Self::function_missing_reactivity_for_condition(self_, x),
                    node,
                    || errors::missing_reactivity_for_condition,
                    function_attrs,
                );

                self.produce_error(
                    |self_, x| {
                        Self::function_declaration_contains_owned_mutable_attribute(self_, x)
                    },
                    node,
                    || errors::misplaced_owned_mutable,
                    function_attrs,
                );

                self.invalid_modifier_errors("Top-level functions", node, |kind| {
                    kind == TokenKind::Async || kind == TokenKind::Coroutine
                });
            }
            MethodishDeclaration(md) => {
                let header_node = &md.methodish_function_decl_header;
                let modifiers = Self::modifiers_of_function_decl_header_exn(header_node);
                let class_name = self.active_classish_name().unwrap_or("");
                let method_name = self
                    .extract_function_name(&md.methodish_function_decl_header)
                    .unwrap_or("");
                let method_attrs = &md.methodish_attribute;
                self.error_if_memoize_function_returns_mutable(method_attrs);
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
                self.produce_error(
                    |self_, x| self_.async_magic_method(x),
                    header_node,
                    || errors::async_magic_method(method_name),
                    modifiers,
                );
                self.produce_error(
                    |self_, x| self_.call_static_method(x),
                    header_node,
                    || errors::call_static_method,
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
                        || kind == TokenKind::Coroutine
                });

                if Self::has_modifier_static(node)
                    && (self.attribute_specification_contains(
                        method_attrs,
                        sn::user_attributes::MUTABLE,
                    ) || self.attribute_specification_contains(
                        method_attrs,
                        sn::user_attributes::MAYBE_MUTABLE,
                    ))
                {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::mutability_annotation_on_static_method,
                    ))
                };

                if method_name.eq_ignore_ascii_case(sn::members::__CONSTRUCT)
                    && (self.attribute_specification_contains(
                        method_attrs,
                        sn::user_attributes::MUTABLE,
                    ) || self.attribute_specification_contains(
                        method_attrs,
                        sn::user_attributes::MAYBE_MUTABLE,
                    ) || self.attribute_specification_contains(
                        method_attrs,
                        sn::user_attributes::MUTABLE_RETURN,
                    ))
                {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::mutability_annotation_on_constructor,
                    ))
                };

                let fun_semicolon = &md.methodish_semicolon;

                self.produce_error(
                    |self_, x| self_.methodish_non_abstract_without_body_not_native(x),
                    node,
                    || errors::error2015(class_name, method_name),
                    fun_semicolon,
                );
                self.produce_error(
                    |_, x| Self::methodish_abstract_conflict_with_private(x),
                    node,
                    || errors::error2016(class_name, method_name),
                    modifiers,
                );

                self.produce_error(
                    |_, x| Self::methodish_abstract_conflict_with_final(x),
                    node,
                    || errors::error2019(class_name, method_name),
                    modifiers,
                );
                self.produce_error(
                    |self_, x| self_.methodish_abstract_inside_interface(x),
                    node,
                    || errors::error2045,
                    modifiers,
                );
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
                    || errors::error2046("an abstract method"),
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
                self.special_method_param_errors(&md.methodish_function_decl_header);
                self.produce_error(
                    |self_, x| self_.methodish_multiple_reactivity_annotations(x),
                    node,
                    || errors::multiple_reactivity_annotations,
                    method_attrs,
                );
                self.check_nonrx_annotation(node);
                self.produce_error(
                    |self_, x| self_.methodish_missing_reactivity_for_condition(x),
                    node,
                    || errors::missing_reactivity_for_condition,
                    method_attrs,
                );
                self.produce_error(
                    |self_, x| self_.methodish_contains_owned_mutable_attribute(x),
                    node,
                    || errors::misplaced_owned_mutable,
                    method_attrs,
                );
            }
            _ => (),
        }
    }

    fn is_hashbang(&self, node: &'a Syntax<Token, Value>) -> bool {
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
        params: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        Self::variadic_param(params).filter(|x| Self::is_parameter_with_default_value(x))
    }

    // If a variadic parameter is marked inout, return it
    fn variadic_param_with_callconv(
        params: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        Self::variadic_param(params).filter(|x| Self::is_parameter_with_callconv(x))
    }

    // If an inout parameter has a default, return the default
    fn param_with_callconv_has_default(
        node: &'a Syntax<Token, Value>,
    ) -> Option<&'a Syntax<Token, Value>> {
        match &node.syntax {
            ParameterDeclaration(x)
                if Self::is_parameter_with_callconv(node)
                    && Self::is_parameter_with_default_value(&node) =>
            {
                Some(&x.parameter_default_value)
            }
            _ => None,
        }
    }

    fn params_errors(&mut self, params: &'a Syntax<Token, Value>) {
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

    fn decoration_errors(&mut self, node: &'a Syntax<Token, Value>) {
        self.produce_error(
            |_, x| Self::is_double_variadic(x),
            node,
            || errors::double_variadic,
            node,
        );
    }

    fn parameter_rx_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let ParameterDeclaration(x) = &node.syntax {
            let spec = &x.parameter_attribute;
            let name = &x.parameter_name;
            let has_owned_mutable =
                self.attribute_specification_contains(spec, sn::user_attributes::OWNED_MUTABLE);

            let has_mutable =
                self.attribute_specification_contains(spec, sn::user_attributes::MUTABLE);

            let has_maybemutable =
                self.attribute_specification_contains(spec, sn::user_attributes::MAYBE_MUTABLE);

            match (has_mutable, has_owned_mutable, has_maybemutable) {
                (true, true, _) => self.errors.push(Self::make_error_from_node(
                    node,
                    errors::conflicting_mutable_and_owned_mutable_attributes,
                )),
                (true, _, true) => self.errors.push(Self::make_error_from_node(
                    node,
                    errors::conflicting_mutable_and_maybe_mutable_attributes,
                )),
                (_, true, true) => self.errors.push(Self::make_error_from_node(
                    node,
                    errors::conflicting_owned_mutable_and_maybe_mutable_attributes,
                )),
                _ => (),
            }
            if (has_mutable || has_owned_mutable || has_maybemutable)
                && Self::is_variadic_expression(name)
            {
                self.errors
                    .push(Self::make_error_from_node(name, errors::vararg_and_mutable))
            }
            let is_inout = Self::is_parameter_with_callconv(node);
            if is_inout && (has_mutable || has_maybemutable || has_owned_mutable) {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::mutability_annotation_on_inout_parameter,
                ))
            }
            if has_owned_mutable || has_mutable {
                let attrs = self.env.context.active_callable_attr_spec;
                let active_is_rx = self.env.context.active_is_rx_or_enclosing_for_lambdas;

                let parent_func_is_memoize = attrs
                    .map(|spec| {
                        self.attribute_specification_contains(spec, sn::user_attributes::MEMOIZE)
                            || self.attribute_specification_contains(
                                spec,
                                sn::user_attributes::MEMOIZE,
                            )
                    })
                    .unwrap_or(false);

                if has_owned_mutable && !active_is_rx {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::mutably_owned_attribute_on_non_rx_function,
                    ))
                }
                if has_mutable && parent_func_is_memoize {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::mutable_parameter_in_memoize_function(false),
                    ))
                }
            }
        }
    }

    fn does_unop_create_write(token_kind: Option<TokenKind>) -> bool {
        token_kind.map_or(false, |x| match x {
            TokenKind::PlusPlus | TokenKind::MinusMinus => true,
            _ => false,
        })
    }

    fn does_decorator_create_write(token_kind: Option<TokenKind>) -> bool {
        match token_kind {
            Some(TokenKind::Inout) => true,
            _ => false,
        }
    }

    fn node_lval_type<'b>(
        node: &'a Syntax<Token, Value>,
        parents: &'b [&'a Syntax<Token, Value>],
    ) -> LvalType {
        let is_in_final_lval_position = |mut node, parents: &'b [&'a Syntax<Token, Value>]| {
            for &parent in parents.iter().rev() {
                match &parent.syntax {
                    SyntaxList(_) | ListItem(_) => {
                        node = parent;
                        continue;
                    }
                    ExpressionStatement(_) => return true,
                    ForStatement(x)
                        if node as *const _ == &x.for_initializer as *const _
                            || node as *const _ == &x.for_end_of_loop as *const _ =>
                    {
                        return true
                    }
                    UsingStatementFunctionScoped(x)
                        if node as *const _ == &x.using_function_expression as *const _ =>
                    {
                        return true
                    }
                    UsingStatementBlockScoped(x)
                        if node as *const _ == &x.using_block_expressions as *const _ =>
                    {
                        return true
                    }
                    _ => return false,
                }
            }
            false
        };
        let get_arg_call_node_with_parents = |mut node, parents: &'b [&'a Syntax<Token, Value>]| {
            for i in (0..parents.len()).rev() {
                let parent = parents[i];
                match &parent.syntax {
                    SyntaxList(_) | ListItem(_) => {
                        node = parent;
                        continue;
                    }
                    FunctionCallExpression(x)
                        if node as *const _ == &x.function_call_argument_list as *const _ =>
                    {
                        if i == 0 {
                            // probably unreachable, but just in case to avoid crashing on 0-1
                            return Some((parent, &parents[0..0]));
                        }
                        let grandparent = parents.get(i - 1).unwrap();
                        return match &grandparent.syntax {
                            PrefixUnaryExpression(x)
                                if Self::token_kind(&x.prefix_unary_operator)
                                    == Some(TokenKind::At) =>
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
                        Some(parent) => match &parent.syntax {
                            BinaryExpression(x)
                                if call_node as *const _ == &x.binary_right_operand as *const _
                                    && Self::does_binop_create_write_on_left(Self::token_kind(
                                        &x.binary_operator,
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

        let unary_expression_operator = |x: &'a Syntax<Token, Value>| match &x.syntax {
            PrefixUnaryExpression(x) => &x.prefix_unary_operator,
            PostfixUnaryExpression(x) => &x.postfix_unary_operator,
            _ => panic!("expected expression operator"),
        };

        let unary_expression_operand = |x: &'a Syntax<Token, Value>| match &x.syntax {
            PrefixUnaryExpression(x) => &x.prefix_unary_operand,
            PostfixUnaryExpression(x) => &x.postfix_unary_operand,
            _ => panic!("expected expression operator"),
        };

        if let Some(next_node) = parents.last() {
            let parents = &parents[..parents.len() - 1];
            match &next_node.syntax {
                DecoratedExpression(x)
                    if node as *const _ == &x.decorated_expression_expression as *const _
                        && Self::does_decorator_create_write(Self::token_kind(
                            &x.decorated_expression_decorator,
                        )) =>
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
                    if node as *const _ == &x.binary_left_operand as *const _
                        && Self::does_binop_create_write_on_left(Self::token_kind(
                            &x.binary_operator,
                        )) =>
                {
                    if is_in_final_lval_position(next_node, parents) {
                        LvalTypeFinal
                    } else {
                        LvalTypeNonFinal
                    }
                }
                ForeachStatement(x)
                    if node as *const _ == &x.foreach_key as *const _
                        || node as *const _ == &x.foreach_value as *const _ =>
                {
                    LvalTypeFinal
                }
                _ => LvalTypeNone,
            }
        } else {
            LvalTypeNone
        }
    }

    fn lval_errors(&mut self, syntax_node: &'a Syntax<Token, Value>) {
        if self.env.parser_options.po_disable_lval_as_an_expression {
            if let LvalTypeNonFinal = Self::node_lval_type(syntax_node, &self.parents) {
                self.errors.push(Self::make_error_from_node(
                    syntax_node,
                    errors::lval_as_expression,
                ))
            }
        }
    }

    fn parameter_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let param_errors = |self_: &mut Self, params| {
            for x in Self::syntax_to_list_no_separators(params) {
                self_.parameter_rx_errors(x)
            }
            self_.params_errors(params)
        };
        match &node.syntax {
            ParameterDeclaration(p) => {
                let callconv_text = self.text(Self::extract_callconv_node(node).unwrap_or(node));
                self.produce_error_from_check(&Self::param_with_callconv_has_default, node, || {
                    errors::error2074(callconv_text)
                });
                self.parameter_rx_errors(node);
                self.check_type_hint(&p.parameter_type);

                if Self::is_parameter_with_callconv(node) {
                    if self.is_inside_async_method() {
                        self.errors.push(Self::make_error_from_node_with_type(
                            node,
                            errors::inout_param_in_async,
                            ErrorType::RuntimeError,
                        ))
                    }
                    if self.is_in_construct_method() {
                        self.errors.push(Self::make_error_from_node(
                            node,
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
                            node,
                            errors::memoize_with_inout,
                            ErrorType::RuntimeError,
                        ))
                    }
                }
            }
            FunctionDeclarationHeader(x) => param_errors(self, &x.function_parameter_list),
            AnonymousFunction(x) => param_errors(self, &x.anonymous_parameters),
            ClosureTypeSpecifier(x) => param_errors(self, &x.closure_parameter_list),
            LambdaExpression(x) => {
                if let LambdaSignature(x) = &x.lambda_signature.syntax {
                    param_errors(self, &x.lambda_parameters)
                }
            }
            DecoratedExpression(_) => self.decoration_errors(node),
            _ => (),
        }
    }

    fn redeclaration_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            FunctionDeclarationHeader(f) if !f.function_name.is_missing() => {
                let mut it = self.parents.iter().rev();
                let p1 = it.next();
                let _ = it.next();
                let p3 = it.next();
                let p4 = it.next();
                match (p1, p3, p4) {
                    (
                        Some(Syntax {
                            syntax: FunctionDeclaration(_),
                            ..
                        }),
                        Some(Syntax {
                            syntax: NamespaceBody(_),
                            ..
                        }),
                        _,
                    )
                    | (
                        Some(Syntax {
                            syntax: FunctionDeclaration(_),
                            ..
                        }),
                        _,
                        None,
                    )
                    | (
                        Some(Syntax {
                            syntax: MethodishDeclaration(_),
                            ..
                        }),
                        _,
                        _,
                    )
                    | (
                        Some(Syntax {
                            syntax: MethodishTraitResolution(_),
                            ..
                        }),
                        _,
                        _,
                    ) => {
                        let function_name = self.text(&f.function_name);
                        let location = Self::make_location_of_node(&f.function_name);
                        let is_method = match p1 {
                            Some(Syntax {
                                syntax: MethodishDeclaration(_),
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
                                    &f.function_name,
                                    &combine_names(&self.namespace_name, &function_name),
                                    &function_name,
                                    &def.location,
                                    &|x, y| errors::declared_name_is_already_in_use(line_num, x, y),
                                ))
                            }
                            _ => (),
                        };
                        self.names.functions.add(function_name, def)
                    }
                    _ if self.env.is_typechecker() => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::decl_outside_global_scope,
                    )),
                    _ => (),
                }
            }
            _ => (),
        }
    }

    fn is_foreach_in_for(for_initializer: &'a Syntax<Token, Value>) -> bool {
        if let Some(Syntax {
            syntax: ListItem(x),
            ..
        }) = for_initializer.syntax_node_to_list().next()
        {
            x.list_item.is_as_expression()
        } else {
            false
        }
    }

    fn statement_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let expect_colon = |colon: &'a Syntax<Token, Value>| match &colon.syntax {
            Token(m) if self.env.is_typechecker() && m.kind() != TokenKind::Colon => {
                Some((colon, errors::error1020))
            }
            _ => None,
        };
        (match &node.syntax {
            TryStatement(x)
                if x.try_catch_clauses.is_missing() && x.try_finally_clause.is_missing() =>
            {
                Some((node, errors::error2007))
            }
            UsingStatementFunctionScoped(_) if !self.using_statement_function_scoped_is_legal() => {
                Some((node, errors::using_st_function_scoped_top_level))
            }
            ForStatement(x) if Self::is_foreach_in_for(&x.for_initializer) => {
                Some((node, errors::for_with_as_expression))
            }
            CaseLabel(x) => expect_colon(&x.case_colon),

            DefaultLabel(x) => expect_colon(&x.default_colon),
            _ => None,
        })
        .into_iter()
        .for_each(|(error_node, error_message)| {
            self.errors
                .push(Self::make_error_from_node(error_node, error_message))
        })
    }

    fn invalid_shape_initializer_name(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            LiteralExpression(x) => {
                let is_str = match Self::token_kind(&x.literal_expression) {
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
            ScopeResolutionExpression(_) => (),
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

    fn invalid_shape_field_check(&mut self, node: &'a Syntax<Token, Value>) {
        if let FieldInitializer(x) = &node.syntax {
            self.invalid_shape_initializer_name(&x.field_initializer_name)
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
                _ if s == sn::members::__CALL => false,
                _ if s == sn::members::__INVOKE => false,
                _ => sn::members::AS_LOWERCASE_SET.contains(&s),
            }
        })
    }

    fn function_call_argument_errors(
        &mut self,
        in_constructor_call: bool,
        node: &'a Syntax<Token, Value>,
    ) {
        if let Some(e) = match &node.syntax {
            VariableExpression(x)
                if self.text(&x.variable_expression) == sn::superglobals::GLOBALS =>
            {
                Some(errors::globals_without_subscript)
            }
            DecoratedExpression(x) => {
                if let Token(token) = &x.decorated_expression_decorator.syntax {
                    if token.kind() == TokenKind::Inout {
                        let expression = &x.decorated_expression_expression;
                        match &expression.syntax {
                            _ if in_constructor_call => Some(errors::inout_param_in_construct),
                            VariableExpression(x)
                                if sn::superglobals::is_any_global(
                                    self.text(&x.variable_expression),
                                ) =>
                            {
                                Some(errors::fun_arg_invalid_arg)
                            }
                            BinaryExpression(_) => Some(errors::fun_arg_inout_set),
                            QualifiedName(_) => Some(errors::fun_arg_inout_const),
                            Token(_) if expression.is_name() => Some(errors::fun_arg_inout_const),
                            // TODO: Maybe be more descriptive in error messages
                            ScopeResolutionExpression(_)
                            | FunctionCallExpression(_)
                            | MemberSelectionExpression(_)
                            | SafeMemberSelectionExpression(_) => Some(errors::fun_arg_invalid_arg),
                            SubscriptExpression(x) => match &x.subscript_receiver.syntax {
                                MemberSelectionExpression(_) | ScopeResolutionExpression(_) => {
                                    Some(errors::fun_arg_invalid_arg)
                                }
                                _ => {
                                    let text = self.text(&x.subscript_receiver);
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

    fn function_call_on_xhp_name_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let check = |self_: &mut Self,
                     member_object: &'a Syntax<Token, Value>,
                     name: &'a Syntax<Token, Value>| {
            if let XHPExpression(_) = &member_object.syntax {
                if self_.env.is_typechecker() {
                    self_.errors.push(Self::make_error_from_node(
                        node,
                        errors::method_calls_on_xhp_expression,
                    ))
                }
            }

            if let Token(token) = &name.syntax {
                if token.kind() == TokenKind::XHPClassName {
                    self_.errors.push(Self::make_error_from_node(
                        node,
                        errors::method_calls_on_xhp_attributes,
                    ))
                }
            }
        };
        match &node.syntax {
            MemberSelectionExpression(x) => check(self, &x.member_object, &x.member_name),
            SafeMemberSelectionExpression(x) => {
                check(self, &x.safe_member_object, &x.safe_member_name)
            }
            _ => (),
        }
    }

    fn no_async_before_lambda_body(&mut self, body_node: &'a Syntax<Token, Value>) {
        if let AwaitableCreationExpression(_) = &body_node.syntax {
            if self.env.is_typechecker() {
                self.errors.push(Self::make_error_from_node(
                    body_node,
                    errors::no_async_before_lambda_body,
                ))
            }
        }
    }

    fn no_memoize_attribute_on_lambda(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
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

                        _ => (),
                    }
                }
            }

            _ => (),
        }
    }

    fn is_good_scope_resolution_qualifier(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
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

    fn new_variable_errors_(
        &mut self,
        node: &'a Syntax<Token, Value>,
        inside_scope_resolution: bool,
    ) {
        match &node.syntax {
            SimpleTypeSpecifier(_)
            | VariableExpression(_)
            | GenericTypeSpecifier(_)
            | PipeVariableExpression(_) => (),
            SubscriptExpression(x) if x.subscript_index.is_missing() => self.errors.push(
                Self::make_error_from_node(node, errors::instanceof_missing_subscript_index),
            ),
            SubscriptExpression(x) => {
                self.new_variable_errors_(&x.subscript_receiver, inside_scope_resolution)
            }
            MemberSelectionExpression(x) => {
                if inside_scope_resolution {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::instanceof_memberselection_inside_scoperesolution,
                    ))
                } else {
                    self.new_variable_errors_(&x.member_object, inside_scope_resolution)
                }
            }
            ScopeResolutionExpression(x) => {
                if let Token(name) = &x.scope_resolution_name.syntax {
                    if Self::is_good_scope_resolution_qualifier(&x.scope_resolution_qualifier)
                        && name.kind() == TokenKind::Variable
                    {
                        // OK
                    } else if name.kind() == TokenKind::Variable {
                        self.new_variable_errors_(&x.scope_resolution_qualifier, true)
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
                    errors::instanceof_new_unknown_node(node.kind().to_string()),
                ));
            }
        }
    }

    fn new_variable_errors(&mut self, node: &'a Syntax<Token, Value>) {
        self.new_variable_errors_(node, false)
    }

    fn class_type_designator_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if !Self::is_good_scope_resolution_qualifier(node) {
            match &node.syntax {
                ParenthesizedExpression(_) => (),
                _ => self.new_variable_errors(node),
            }
        }
    }

    fn rec_walk_impl<F, X>(
        &self,
        parents: &mut Vec<&'a Syntax<Token, Value>>,
        f: &F,
        node: &'a Syntax<Token, Value>,
        mut acc: X,
    ) -> X
    where
        F: Fn(&'a Syntax<Token, Value>, &Vec<&'a Syntax<Token, Value>>, X) -> (bool, X),
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

    fn rec_walk<F, X>(&self, f: F, node: &'a Syntax<Token, Value>, acc: X) -> X
    where
        F: Fn(&'a Syntax<Token, Value>, &Vec<&'a Syntax<Token, Value>>, X) -> (bool, X),
    {
        self.rec_walk_impl(&mut vec![], &f, node, acc)
    }

    fn find_invalid_lval_usage(&self, node: &'a Syntax<Token, Value>) -> Vec<SyntaxError> {
        self.rec_walk(
            |node, parents, mut acc| match &node.syntax {
                AnonymousFunction(_) | LambdaExpression(_) | AwaitableCreationExpression(_) => {
                    (false, acc)
                }
                _ => {
                    match Self::node_lval_type(node, parents) {
                        LvalTypeFinal | LvalTypeNone => (),
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

    fn get_positions_binop_allows_await(
        t: &'a Syntax<Token, Value>,
    ) -> BinopAllowsAwaitInPositions {
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

    fn unop_allows_await(t: &'a Syntax<Token, Value>) -> bool {
        use TokenKind::*;
        Self::token_kind(t).map_or(false, |t| match t {
            Exclamation | Tilde | Plus | Minus | At | Clone | Print => true,
            _ => false,
        })
    }

    fn await_as_an_expression_errors(&mut self, await_node: &'a Syntax<Token, Value>) {
        let mut prev = None;
        let mut node = await_node;
        for n in self.parents.iter().rev() {
            if let Some(prev) = prev {
                node = prev;
            }
            prev = Some(n);
            match &n.syntax {
                // statements that root for the concurrently executed await expressions
                ExpressionStatement(_)
                | ReturnStatement(_)
                | UnsetStatement(_)
                | EchoStatement(_)
                | ThrowStatement(_) => break,
                IfStatement(x) if node as *const _ == &x.if_condition as *const _ => break,
                ForStatement(x) if node as *const _ == &x.for_initializer as *const _ => break,
                SwitchStatement(x) if node as *const _ == &x.switch_expression as *const _ => break,
                ForeachStatement(x) if node as *const _ == &x.foreach_collection as *const _ => {
                    break
                }
                UsingStatementBlockScoped(x)
                    if node as *const _ == &x.using_block_expressions as *const _ =>
                {
                    break
                }
                UsingStatementFunctionScoped(x)
                    if node as *const _ == &x.using_function_expression as *const _ =>
                {
                    break
                }
                LambdaExpression(x) if node as *const _ == &x.lambda_body as *const _ => break,
                // Dependent awaits are not allowed currently
                PrefixUnaryExpression(x)
                    if Self::token_kind(&x.prefix_unary_operator) == Some(TokenKind::Await) =>
                {
                    self.errors.push(Self::make_error_from_node(
                        await_node,
                        errors::invalid_await_position_dependent,
                    ));
                    break;
                }
                // Unary based expressions have their own custom fanout
                PrefixUnaryExpression(x) if Self::unop_allows_await(&x.prefix_unary_operator) => {
                    continue
                }
                PostfixUnaryExpression(x) if Self::unop_allows_await(&x.postfix_unary_operator) => {
                    continue
                }
                DecoratedExpression(x)
                    if Self::unop_allows_await(&x.decorated_expression_decorator) =>
                {
                    continue
                }
                // Special case the pipe operator error message
                BinaryExpression(x)
                    if node as *const _ == &x.binary_right_operand as *const _
                        && Self::token_kind(&x.binary_operator)
                            == Some(TokenKind::BarGreaterThan) =>
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
                    if (match Self::get_positions_binop_allows_await(&x.binary_operator) {
                        BinopAllowAwaitBoth => true,
                        BinopAllowAwaitLeft => {
                            node as *const _ == &x.binary_left_operand as *const _
                        }
                        BinopAllowAwaitRight => {
                            node as *const _ == &x.binary_right_operand as *const _
                        }
                        BinopAllowAwaitNone => false,
                    }) =>
                {
                    continue
                }
                // test part of conditional expression is considered legal location if
                //  onditional expression itself is in legal location
                ConditionalExpression(x) if node as *const _ == &x.conditional_test as *const _ => {
                    continue
                }
                FunctionCallExpression(x)
                    if node as *const _ == &x.function_call_receiver as *const _
                        || node as *const _ == &x.function_call_argument_list as *const _
                            && !x
                                .function_call_receiver
                                .is_safe_member_selection_expression() =>
                {
                    continue
                }

                // object of member selection expression or safe member selection expression
                // is in legal position if member selection expression itself is in legal position
                SafeMemberSelectionExpression(x)
                    if node as *const _ == &x.safe_member_object as *const _ =>
                {
                    continue
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
                | ArrayIntrinsicExpression(_)
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
            .any(|parent| match &parent.syntax {
                ConcurrentStatement(_) => true,
                _ => false,
            });
        if !is_in_concurrent {
            let await_node_statement_parent =
                self.parents
                    .iter()
                    .rev()
                    .find(|parent| match &parent.syntax {
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

    fn check_prefix_unary_dollar(node: &'a Syntax<Token, Value>) -> bool {
        match &node.syntax {
            PrefixUnaryExpression(x)
                if Self::token_kind(&x.prefix_unary_operator) == Some(TokenKind::Dollar) =>
            {
                Self::check_prefix_unary_dollar(&x.prefix_unary_operand)
            }
            BracedExpression(_) | SubscriptExpression(_) | VariableExpression(_) => false, // these ones are valid
            LiteralExpression(_) | PipeVariableExpression(_) => false, // these ones get caught later
            _ => true,
        }
    }

    fn node_has_await_child(&mut self, node: &'a Syntax<Token, Value>) -> bool {
        self.rec_walk(
            |node, _parents, acc| {
                let is_new_scope = match &node.syntax {
                    AnonymousFunction(_) | LambdaExpression(_) | AwaitableCreationExpression(_) => {
                        true
                    }
                    _ => false,
                };
                if is_new_scope {
                    (false, false)
                } else {
                    let is_await = |n: &'a Syntax<Token, Value>| match &n.syntax {
                        PrefixUnaryExpression(x)
                            if Self::token_kind(&x.prefix_unary_operator)
                                == Some(TokenKind::Await) =>
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

    fn expression_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let check_is_as_expression = |self_: &mut Self, hint: &'a Syntax<Token, Value>| {
            let n = match &node.syntax {
                IsExpression(_) => "is",
                _ => "as",
            };
            match &hint.syntax {
                ClosureTypeSpecifier(_) if self_.env.is_hhvm_compat() => {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "Callable"),
                    ));
                }
                SoftTypeSpecifier(_) => {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "Soft"),
                    ));
                }
                AttributizedSpecifier(x)
                    if self_.attribute_specification_contains(
                        &x.attributized_specifier_attribute_spec,
                        "__Soft",
                    ) =>
                {
                    self_.errors.push(Self::make_error_from_node(
                        hint,
                        errors::invalid_is_as_expression_hint(n, "Soft"),
                    ));
                }
                _ => (),
            }
        };
        match &node.syntax {
            // We parse the right hand side of `new` as a generic expression, but PHP
            // (and therefore Hack) only allow a certain subset of expressions, so we
            // should verify here that the expression we parsed is in that subset.
            // Refer: https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#instanceof-operator*)
            ConstructorCall(ctr_call) => {
                for p in
                    Self::syntax_to_list_no_separators(&ctr_call.constructor_call_argument_list)
                {
                    self.function_call_argument_errors(true, p);
                }
                self.class_type_designator_errors(&ctr_call.constructor_call_type);
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
                            if ctr_call.constructor_call_left_paren.is_missing()
                                || ctr_call.constructor_call_right_paren.is_missing()
                            {
                                let node = &ctr_call.constructor_call_type;
                                let constructor_name = self.text(&ctr_call.constructor_call_type);
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
                if let Token(token) = &x.literal_expression.syntax {
                    if token.kind() == TokenKind::DecimalLiteral
                        || token.kind() == TokenKind::DecimalLiteral
                    {
                        let text = self.text(&x.literal_expression);
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
                if self.env.is_typechecker() && x.subscript_left_bracket.is_left_brace() =>
            {
                self.errors
                    .push(Self::make_error_from_node(node, errors::error2020))
            }

            FunctionCallExpression(x) => {
                let arg_list = &x.function_call_argument_list;
                if let Some(h) = Self::misplaced_variadic_arg(arg_list) {
                    self.errors
                        .push(Self::make_error_from_node(h, errors::error2033))
                }

                for p in Self::syntax_to_list_no_separators(arg_list) {
                    self.function_call_argument_errors(false, p)
                }
                self.function_call_on_xhp_name_errors(&x.function_call_receiver);
            }
            ListExpression(x) if x.list_members.is_missing() && self.env.is_hhvm_compat() => {
                if let Some(Syntax {
                    syntax: ForeachStatement(x),
                    ..
                }) = self.parents.last()
                {
                    if node as *const _ == &x.foreach_value as *const _ {
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
                for f in Self::syntax_to_list_no_separators(&x.shape_expression_fields).rev() {
                    self.invalid_shape_field_check(f)
                }
            }
            DecoratedExpression(x) => {
                let decorator = &x.decorated_expression_decorator;
                if Self::token_kind(decorator) == Some(TokenKind::Await) {
                    self.await_as_an_expression_errors(node)
                }
            }
            YieldFromExpression(_) | YieldExpression(_) => {
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
                let qualifier = &x.scope_resolution_qualifier;
                let name = &x.scope_resolution_name;

                let (is_dynamic_name, is_self_or_parent, is_valid) =
                // PHP langspec allows string literals, variables
                // qualified names, static, self and parent as valid qualifiers
                // We do not allow string literals in hack
                match (&qualifier.syntax , Self::token_kind(qualifier)) {
                | (LiteralExpression (_), _) => (false, false, !(self.env.is_typechecker())),
                | (QualifiedName (_), _) => (false, false, true),
                | (_, Some (TokenKind::Name))
                | (_, Some (TokenKind::XHPClassName))
                | (_, Some (TokenKind::Static)) =>
                  (false, false, true),
                | (_, Some (TokenKind::SelfToken))
                | (_, Some (TokenKind::Parent)) =>
                  (false, true, true),
                // ${}::class
                | (PrefixUnaryExpression (x), _) if Self::token_kind(&x.prefix_unary_operator) == Some (TokenKind::Dollar) =>
                  (true, false, true),
                | (PipeVariableExpression (_), _)
                | (VariableExpression (_), _)
                | (SimpleTypeSpecifier (_), _)
                | (GenericTypeSpecifier (_), _) =>
                  (true, false, true),
                | _ => (true, false, !self.env.is_typechecker()),
            };
                if !is_valid {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::invalid_scope_resolution_qualifier,
                    ))
                }
                let is_name_class = self.text(name).eq_ignore_ascii_case("class");
                if is_dynamic_name && is_name_class {
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
                if Self::token_kind(&x.prefix_unary_operator) == Some(TokenKind::Dollar) =>
            {
                if Self::check_prefix_unary_dollar(node) {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::dollar_unary))
                }
            }

            // TODO(T21285960): Remove this bug-port, stemming from T22184312
            LambdaExpression(x)
                if self.env.is_hhvm_compat()
                    && !x.lambda_async.is_missing()
                    && x.lambda_async.trailing_width() == 0
                    && x.lambda_coroutine.full_width() == 0
                    && x.lambda_signature.leading_width() == 0 =>
            {
                self.errors
                    .push(Self::make_error_from_node(node, errors::error1057("==>")))
            }
            // End of bug-port
            IsExpression(x) => check_is_as_expression(self, &x.is_right_operand),
            AsExpression(x) => check_is_as_expression(self, &x.as_right_operand),

            ConditionalExpression(x) => {
                if x.conditional_consequence.is_missing() && self.env.is_typechecker() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::elvis_operator_space,
                    ))
                }
                if x.conditional_test.is_conditional_expression() && self.env.is_typechecker() {
                    self.errors
                        .push(Self::make_error_from_node(node, errors::nested_ternary))
                }
                match &x.conditional_alternative.syntax {
                    LambdaExpression(x)
                        if x.lambda_body.is_conditional_expression()
                            && self.env.is_typechecker() =>
                    {
                        self.errors
                            .push(Self::make_error_from_node(node, errors::nested_ternary))
                    }
                    _ => (),
                }
            }
            LambdaExpression(x) => {
                self.no_memoize_attribute_on_lambda(&x.lambda_attribute_spec);
                self.no_async_before_lambda_body(&x.lambda_body);
            }
            AnonymousFunction(x) => {
                self.no_memoize_attribute_on_lambda(&x.anonymous_attribute_spec)
            }
            AwaitableCreationExpression(x) => {
                self.no_memoize_attribute_on_lambda(&x.awaitable_attribute_spec)
            }

            CollectionLiteralExpression(x) => {
                enum Status {
                    ValidClass(String),
                    InvalidClass,
                    InvalidBraceKind,
                }
                use Status::*;

                let n = &x.collection_literal_name;
                let initializers = &x.collection_literal_initializers;

                let is_standard_collection = |lc_name: &str| {
                    lc_name.eq_ignore_ascii_case("pair")
                        || lc_name.eq_ignore_ascii_case("vector")
                        || lc_name.eq_ignore_ascii_case("map")
                        || lc_name.eq_ignore_ascii_case("set")
                        || lc_name.eq_ignore_ascii_case("immvector")
                        || lc_name.eq_ignore_ascii_case("immmap")
                        || lc_name.eq_ignore_ascii_case("immset")
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
                let status = match &n.syntax {
                    // non-qualified name
                    SimpleTypeSpecifier(x) => match &x.simple_type_specifier.syntax {
                        Token(t) => check_type_specifier(&x.simple_type_specifier, t),
                        QualifiedName(x) => check_qualified_name(&x.qualified_name_parts),
                        _ => InvalidClass,
                    },
                    GenericTypeSpecifier(x) => match &x.generic_class_type.syntax {
                        Token(t) => check_type_specifier(&x.generic_class_type, t),
                        QualifiedName(x) => check_qualified_name(&x.qualified_name_parts),
                        _ => InvalidClass,
                    },
                    _ => InvalidClass,
                };
                let num_initializers = Self::syntax_to_list_no_separators(initializers).count();
                match &status {
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
                    ValidClass(_) => (),
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
            PrefixUnaryExpression(x)
                if Self::token_kind(&x.prefix_unary_operator) == Some(TokenKind::Await) =>
            {
                self.await_as_an_expression_errors(node)
            }
            // Other kinds of expressions currently produce no expr errors.
            _ => (),
        }
    }

    fn check_repeated_properties_tconst_const(
        &mut self,
        full_name: &str,
        prop: &'a Syntax<Token, Value>,
        p_names: &mut HashSet<String>,
        c_names: &mut HashSet<String>,
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

        match &prop.syntax {
            PropertyDeclaration(x) => {
                for prop in Self::syntax_to_list_no_separators(&x.property_declarators) {
                    if let PropertyDeclarator(x) = &prop.syntax {
                        check(&x.property_name, p_names)
                    }
                }
            }
            ConstDeclaration(x) => {
                for prop in Self::syntax_to_list_no_separators(&x.const_declarators) {
                    if let ConstantDeclarator(x) = &prop.syntax {
                        check(&x.constant_declarator_name, c_names)
                    }
                }
            }
            TypeConstDeclaration(x) => check(&x.type_const_name, c_names),
            _ => (),
        }
    }

    fn require_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let RequireClause(p) = &node.syntax {
            let name = self.text(&p.require_name);
            let req_kind = Self::token_kind(&p.require_kind);
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
                _ => (),
            }
        }
    }

    fn check_type_name(
        &mut self,
        name: &'a Syntax<Token, Value>,
        name_text: &str,
        location: Location,
    ) {
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
        l: &'a Syntax<Token, Value>,
    ) -> (HashSet<&'a str>, HashSet<&'a str>) {
        let mut res: HashSet<&'a str> = HashSet::new();
        let mut notreified: HashSet<&'a str> = HashSet::new();
        for p in Self::syntax_to_list_no_separators(l).rev() {
            match &p.syntax {
                TypeParameter(x) => {
                    let name = self.text(&x.type_name);
                    if !x.type_reified.is_missing() {
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
                _ => (),
            }
        }
        (res, notreified)
    }

    fn reified_parameter_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let FunctionDeclarationHeader(x) = &node.syntax {
            if let TypeParameters(x) = &x.function_type_parameter_list.syntax {
                self.get_type_params_and_emit_shadowing_errors(&x.type_parameters_parameters)
                    .0;
            }
        }
    }

    fn is_method_declaration(node: &'a Syntax<Token, Value>) -> bool {
        if let MethodishDeclaration(_) = &node.syntax {
            true
        } else {
            false
        }
    }

    fn class_reified_param_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            ClassishDeclaration(cd) => {
                let (reified, non_reified) = match &cd.classish_type_parameters.syntax {
                    TypeParameters(x) => self
                        .get_type_params_and_emit_shadowing_errors(&x.type_parameters_parameters),
                    _ => (HashSet::new(), HashSet::new()),
                };

                let tparams: HashSet<&'a str> = reified
                    .union(&non_reified)
                    .cloned()
                    .collect::<HashSet<&'a str>>();

                let add_error = |self_: &mut Self, e: &'a Syntax<Token, Value>| {
                    if let TypeParameter(x) = &e.syntax {
                        if !x.type_reified.is_missing()
                            && tparams.contains(&self_.text(&x.type_name))
                        {
                            self_
                                .errors
                                .push(Self::make_error_from_node(e, errors::shadowing_reified))
                        }
                    }
                };
                let check_method = |e: &'a Syntax<Token, Value>| {
                    if let MethodishDeclaration(x) = &e.syntax {
                        if let FunctionDeclarationHeader(x) =
                            &x.methodish_function_decl_header.syntax
                        {
                            if let TypeParameters(x) = &x.function_type_parameter_list.syntax {
                                Self::syntax_to_list_no_separators(&x.type_parameters_parameters)
                                    .rev()
                                    .for_each(|x| add_error(self, x))
                            }
                        }
                    }
                };
                if let ClassishBody(x) = &cd.classish_body.syntax {
                    Self::syntax_to_list_no_separators(&x.classish_body_elements)
                        .rev()
                        .for_each(check_method)
                }

                if !reified.is_empty() {
                    if Self::is_token_kind(&cd.classish_keyword, TokenKind::Interface) {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::reified_in_invalid_classish("an interface"),
                        ))
                    } else if Self::is_token_kind(&cd.classish_keyword, TokenKind::Trait) {
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

    fn attr_spec_contains_sealed(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.attribute_specification_contains(node, sn::user_attributes::SEALED)
    }

    fn attr_spec_contains_const(&self, node: &'a Syntax<Token, Value>) -> bool {
        self.attribute_specification_contains(node, sn::user_attributes::CONST)
    }

    // If there's more than one XHP category, report an error on the last one.
    fn duplicate_xhp_category_errors<I>(&mut self, elts: I)
    where
        I: Iterator<Item = &'a Syntax<Token, Value>>,
    {
        let mut iter = elts.filter(|x| match &x.syntax {
            XHPCategoryDeclaration(_) => true,
            _ => false,
        });
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
        I: Iterator<Item = &'a Syntax<Token, Value>>,
    {
        let mut iter = elts.filter(|x| match &x.syntax {
            XHPChildrenDeclaration(_) => true,
            _ => false,
        });
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
        I: Iterator<Item = &'a Syntax<Token, Value>>,
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

    fn classish_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let ClassishDeclaration(cd) = &node.syntax {
            // Given a ClassishDeclaration node, test whether or not it's a trait
            // invoking the 'extends' keyword.
            let classish_invalid_extends_keyword = |_|
              // Invalid if uses 'extends' and is a trait.
              Self::token_kind(&cd.classish_extends_keyword) == Some(TokenKind::Extends)
              && Self::token_kind(&cd.classish_keyword) == Some(TokenKind::Trait);

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
                Self::attr_spec_to_node_list(&cd.classish_attribute).any(|node| {
                    self_.attr_name(node) == Some(sn::user_attributes::SEALED)
                        && self_.attr_args(node).map_or(false, |mut args| {
                            args.any(|arg_node| match &arg_node.syntax {
                                ScopeResolutionExpression(x) => {
                                    self_.text(&x.scope_resolution_name) != "class"
                                }
                                _ => true,
                            })
                        })
                })
            };

            let classish_is_sealed = self.attr_spec_contains_sealed(&cd.classish_attribute);

            // Given a ClassishDeclaration node, test whether or not length of
            // extends_list is appropriate for the classish_keyword. *)
            let classish_invalid_extends_list = |self_: &mut Self| {
                // Invalid if is a class and has list of length greater than one.
                self_.env.is_typechecker()
                    && Self::token_kind(&cd.classish_keyword) == Some(TokenKind::Class)
                    && Self::token_kind(&cd.classish_extends_keyword) == Some(TokenKind::Extends)
                    && Self::syntax_to_list_no_separators(&cd.classish_extends_list).count() != 1
            };

            // Given a ClassishDeclaration node, test whether it is sealed and final.
            let classish_sealed_final = |_| {
                Self::list_contains_predicate(|x| x.is_final(), &cd.classish_modifiers)
                    && classish_is_sealed
            };

            self.produce_error(
                |self_, _| classish_invalid_extends_list(self_),
                &(),
                || errors::error2037,
                &cd.classish_extends_list,
            );

            if let Some(n) = self.attribute_first_reactivity_annotation(&cd.classish_attribute) {
                self.errors.push(Self::make_error_from_node(
                    n,
                    errors::misplaced_reactivity_annotation,
                ))
            };

            self.invalid_modifier_errors("Classes, interfaces, and traits", node, |kind| {
                kind == TokenKind::Abstract || kind == TokenKind::Final || kind == TokenKind::XHP
            });

            self.produce_error(
                |self_, _| classish_sealed_arg_not_classname(self_),
                &(),
                || errors::sealed_val_not_classname,
                &cd.classish_attribute,
            );

            self.produce_error(
                |_, x| classish_invalid_extends_keyword(x),
                &(),
                || errors::error2036,
                &cd.classish_extends_keyword,
            );

            self.produce_error(
                |_, x| classish_sealed_final(x),
                &(),
                || errors::sealed_final,
                &cd.classish_attribute,
            );

            let classish_name = self.text(&cd.classish_name);
            self.produce_error(
                |_, x| Self::cant_be_classish_name(x),
                &classish_name,
                || errors::reserved_keyword_as_class_name(&classish_name),
                &cd.classish_name,
            );
            if Self::is_token_kind(&cd.classish_keyword, TokenKind::Interface)
                && !cd.classish_implements_keyword.is_missing()
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::interface_implements,
                ))
            };
            if self.attr_spec_contains_const(&cd.classish_attribute)
                && (Self::is_token_kind(&cd.classish_keyword, TokenKind::Interface)
                    || Self::is_token_kind(&cd.classish_keyword, TokenKind::Trait))
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_interfaces_traits_enums,
                ))
            }
            if self.attr_spec_contains_const(&cd.classish_attribute)
                && Self::is_token_kind(&cd.classish_keyword, TokenKind::Class)
                && Self::list_contains_predicate(|x| x.is_abstract(), &cd.classish_modifiers)
                && Self::list_contains_predicate(|x| x.is_final(), &cd.classish_modifiers)
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_abstract_final_class,
                ))
            }

            if Self::list_contains_predicate(|x| x.is_final(), &cd.classish_modifiers) {
                match Self::token_kind(&cd.classish_keyword) {
                    Some(TokenKind::Interface) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::declared_final("Interfaces"),
                    )),
                    Some(TokenKind::Trait) => self.errors.push(Self::make_error_from_node(
                        node,
                        errors::declared_final("Traits"),
                    )),
                    _ => (),
                }
            }

            if Self::token_kind(&cd.classish_xhp) == Some(TokenKind::XHP) {
                match Self::token_kind(&cd.classish_keyword) {
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
                    _ => (),
                }
            }

            let name = self.text(&cd.classish_name);
            if let ClassishBody(cb) = &cd.classish_body.syntax {
                let declared_name_str = self.text(&cd.classish_name);
                let full_name = combine_names(&self.namespace_name, declared_name_str);

                let class_body_elts =
                    || Self::syntax_to_list_no_separators(&cb.classish_body_elements);
                let class_body_methods =
                    || class_body_elts().filter(|x| Self::is_method_declaration(x));

                let mut p_names = HashSet::<String>::new();
                let mut c_names = HashSet::<String>::new();
                for elt in class_body_elts() {
                    self.check_repeated_properties_tconst_const(
                        &full_name,
                        elt,
                        &mut p_names,
                        &mut c_names,
                    );
                }
                let has_abstract_fn = class_body_methods().any(&Self::has_modifier_abstract);
                if has_abstract_fn
                    && Self::is_token_kind(&cd.classish_keyword, TokenKind::Class)
                    && !Self::list_contains_predicate(|x| x.is_abstract(), &cd.classish_modifiers)
                {
                    self.errors.push(Self::make_error_from_node(
                        &cd.classish_name,
                        errors::class_with_abstract_method(name),
                    ))
                }

                if Self::is_token_kind(&cd.classish_keyword, TokenKind::Interface) {
                    self.interface_private_method_errors(class_body_elts());
                }

                self.duplicate_xhp_category_errors(class_body_elts());
                self.duplicate_xhp_children_errors(class_body_elts());
            }

            match Self::token_kind(&cd.classish_keyword) {
                Some(TokenKind::Class) | Some(TokenKind::Trait)
                    if !cd.classish_name.is_missing() =>
                {
                    let location = Self::make_location_of_node(&cd.classish_name);
                    self.check_type_name(&cd.classish_name, name, location)
                }
                _ => (),
            }
        }
    }

    // Checks for modifiers on class constants
    fn class_constant_modifier_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if self.is_inside_trait() {
            self.errors
                .push(Self::make_error_from_node(node, errors::const_in_trait))
        }
        self.invalid_modifier_errors("Constants", node, |kind| kind == TokenKind::Abstract);
    }

    fn type_const_modifier_errors(&mut self, node: &'a Syntax<Token, Value>) {
        self.invalid_modifier_errors("Type constants", node, |kind| kind == TokenKind::Abstract);
    }

    fn alias_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let AliasDeclaration(ad) = &node.syntax {
            if Self::token_kind(&ad.alias_keyword) == Some(TokenKind::Type)
                && !ad.alias_constraint.is_missing()
            {
                self.errors.push(Self::make_error_from_node(
                    &ad.alias_keyword,
                    errors::error2034,
                ))
            }
            if !ad.alias_name.is_missing() {
                let name = self.text(&ad.alias_name);
                let location = Self::make_location_of_node(&ad.alias_name);
                if let TypeConstant(_) = &ad.alias_type.syntax {
                    if self.env.is_typechecker() {
                        self.errors.push(Self::make_error_from_node(
                            &ad.alias_type,
                            errors::type_alias_to_type_constant,
                        ))
                    }
                }

                self.check_type_name(&ad.alias_name, name, location)
            }
        }
    }

    fn is_invalid_group_use_clause(
        kind: &'a Syntax<Token, Value>,
        clause: &'a Syntax<Token, Value>,
    ) -> bool {
        if let NamespaceUseClause(x) = &clause.syntax {
            let clause_kind = &x.namespace_use_clause_kind;
            if kind.is_missing() {
                match &clause_kind.syntax {
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

    fn is_invalid_group_use_prefix(prefix: &'a Syntax<Token, Value>) -> bool {
        !prefix.is_namespace_prefix()
    }

    fn group_use_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let NamespaceGroupUseDeclaration(x) = &node.syntax {
            let prefix = &x.namespace_group_use_prefix;
            let clauses = &x.namespace_group_use_clauses;
            let kind = &x.namespace_group_use_kind;
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

        kind: &'a Syntax<Token, Value>,
        cl: &'a Syntax<Token, Value>,
    ) {
        match &cl.syntax {
            NamespaceUseClause(x) if !&x.namespace_use_name.is_missing() => {
                let name = &x.namespace_use_name;

                let kind = if kind.is_missing() {
                    &x.namespace_use_clause_kind
                } else {
                    kind
                };

                let name_text = self.text(name);
                let qualified_name = match namespace_prefix {
                    None => combine_names(GLOBAL_NAMESPACE_NAME, name_text),
                    Some(p) => combine_names(p, name_text),
                };
                let short_name = Self::get_short_name_from_qualified_name(
                    name_text,
                    self.text(&x.namespace_use_alias),
                );

                let do_check =
                    |self_: &mut Self,
                     error_on_global_redefinition,
                     get_names: &dyn Fn(&mut UsedNames) -> &mut Strmap<FirstUseOrDef>,
                     report_error| {
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
                                    || error_on_global_redefinition
                                        && (is_global_namespace || *global)
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

                match &kind.syntax {
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

    fn is_global_in_const_decl(&self, init: &'a Syntax<Token, Value>) -> bool {
        if let SimpleInitializer(x) = &init.syntax {
            if let VariableExpression(x) = &x.simple_initializer_value.syntax {
                return sn::superglobals::is_any_global(self.text(&x.variable_expression));
            }
        }
        false
    }

    fn namespace_use_declaration_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            NamespaceUseDeclaration(x) => {
                Self::syntax_to_list_no_separators(&x.namespace_use_clauses).for_each(|clause| {
                    self.use_class_or_namespace_clause_errors(None, &x.namespace_use_kind, clause)
                })
            }
            NamespaceGroupUseDeclaration(x) => {
                Self::syntax_to_list_no_separators(&x.namespace_group_use_clauses).for_each(
                    |clause| {
                        match &clause.syntax {
                            NamespaceUseClause(x) if !x.namespace_use_name.is_missing() => self
                                .check_preceding_backslashes_qualified_name(&x.namespace_use_name),
                            _ => (),
                        }
                        self.use_class_or_namespace_clause_errors(
                            Some(self.text(&x.namespace_group_use_prefix)),
                            &x.namespace_group_use_kind,
                            clause,
                        )
                    },
                )
            }
            _ => {}
        }
    }

    fn token_text<'b>(&'b self, token: &Token) -> &'b str {
        self.env.text.source_text().sub_as_str(
            token.leading_start_offset().unwrap() + token.leading_width(),
            token.width(),
        )
    }

    fn check_constant_expression(&mut self, node: &'a Syntax<Token, Value>) {
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
                || (text == sn::std_lib_functions::MARK_LEGACY_HACK_ARRAY)
                || (text == sn::std_lib_functions::ARRAY_MARK_LEGACY)
        };

        let is_namey = |self_: &Self, token: &Token| -> bool {
            token.kind() == TokenKind::Name && not_function_credential(self_, token)
        };

        let is_good_scope_resolution_name = |node: &'a Syntax<Token, Value>| match &node.syntax {
            QualifiedName(_) => true,
            Token(token) => {
                use TokenKind::*;
                match token.kind() {
                    Name | Trait | Extends | Implements | Static | Abstract | Final | Private
                    | Protected | Public | Global | Goto | Instanceof | Insteadof | Interface
                    | Namespace | New | Try | Use | Var | List | Clone | Include | Include_once
                    | Throw | Array | Tuple | Print | Echo | Require | Require_once | Return
                    | Else | Elseif | Default | Break | Continue | Switch | Yield | Function
                    | If | Finally | For | Foreach | Case | Do | While | As | Catch | Empty
                    | Using | Class | NullLiteral | Super | Where => true,
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

        let check_type_specifier = |self_: &mut Self, x: &'a Syntax<Token, Value>, initializer| {
            if let Token(token) = &x.syntax {
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
        match &node.syntax {
            Missing | QualifiedName(_) | LiteralExpression(_) => {}
            Token(token) => {
                if !is_namey(self, token) {
                    default(self)
                }
            }
            PrefixUnaryExpression(x) => {
                if let Token(token) = &x.prefix_unary_operator.syntax {
                    use TokenKind::*;
                    match token.kind() {
                        Exclamation | Plus | Minus | Tilde => {
                            self.check_constant_expression(&x.prefix_unary_operand)
                        }
                        _ => default(self),
                    }
                } else {
                    default(self)
                }
            }
            BinaryExpression(x) => {
                if let Token(token) = &x.binary_operator.syntax {
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
                            self.check_constant_expression(&x.binary_left_operand);
                            self.check_constant_expression(&x.binary_right_operand);
                        }
                        _ => default(self),
                    }
                } else {
                    default(self)
                }
            }
            ConditionalExpression(x) => {
                self.check_constant_expression(&x.conditional_test);
                self.check_constant_expression(&x.conditional_consequence);
                self.check_constant_expression(&x.conditional_alternative);
            }
            SimpleInitializer(x) => {
                if let LiteralExpression(y) = &x.simple_initializer_value.syntax {
                    if let SyntaxList(_) = &y.literal_expression.syntax {
                        self.errors.push(Self::make_error_from_node(
                            node,
                            errors::invalid_constant_initializer,
                        ))
                    }
                    self.check_constant_expression(&x.simple_initializer_value)
                } else {
                    self.check_constant_expression(&x.simple_initializer_value)
                }
            }

            ParenthesizedExpression(x) => {
                self.check_constant_expression(&x.parenthesized_expression_expression)
            }
            CollectionLiteralExpression(x) => {
                if let SimpleTypeSpecifier(y) = &x.collection_literal_name.syntax {
                    check_type_specifier(
                        self,
                        &y.simple_type_specifier,
                        &x.collection_literal_initializers,
                    )
                } else if let GenericTypeSpecifier(y) = &x.collection_literal_name.syntax {
                    check_type_specifier(
                        self,
                        &y.generic_class_type,
                        &x.collection_literal_initializers,
                    )
                } else {
                    default(self)
                };
            }

            TupleExpression(x) => check_collection_members(self, &x.tuple_expression_items),
            KeysetIntrinsicExpression(x) => {
                check_collection_members(self, &x.keyset_intrinsic_members)
            }
            VarrayIntrinsicExpression(x) => {
                check_collection_members(self, &x.varray_intrinsic_members)
            }
            DarrayIntrinsicExpression(x) => {
                check_collection_members(self, &x.darray_intrinsic_members)
            }
            VectorIntrinsicExpression(x) => {
                check_collection_members(self, &x.vector_intrinsic_members)
            }
            DictionaryIntrinsicExpression(x) => {
                check_collection_members(self, &x.dictionary_intrinsic_members)
            }
            ArrayIntrinsicExpression(x) => {
                check_collection_members(self, &x.array_intrinsic_members)
            }
            ShapeExpression(x) => check_collection_members(self, &x.shape_expression_fields),
            ElementInitializer(x) => {
                self.check_constant_expression(&x.element_key);
                self.check_constant_expression(&x.element_value);
            }
            FieldInitializer(x) => {
                self.check_constant_expression(&x.field_initializer_name);
                self.check_constant_expression(&x.field_initializer_value);
            }
            ScopeResolutionExpression(x)
                if Self::is_good_scope_resolution_qualifier(&x.scope_resolution_qualifier)
                    && is_good_scope_resolution_name(&x.scope_resolution_name) => {}
            AsExpression(x) => match &x.as_right_operand.syntax {
                LikeTypeSpecifier(_) => self.check_constant_expression(&x.as_left_operand),
                GenericTypeSpecifier(y)
                    if self.text(&y.generic_class_type) == sn::fb::INCORRECT_TYPE
                        || self.text(&y.generic_class_type)
                            == Self::strip_ns(sn::fb::INCORRECT_TYPE) =>
                {
                    self.check_constant_expression(&x.as_left_operand)
                }
                _ => default(self),
            },
            FunctionCallExpression(x) => {
                let mut check_receiver_and_arguments = |receiver| {
                    if is_whitelisted_function(self, receiver) {
                        for node in
                            Self::syntax_to_list_no_separators(&x.function_call_argument_list)
                        {
                            self.check_constant_expression(node)
                        }
                    } else {
                        default(self)
                    }
                };

                match &x.function_call_receiver.syntax {
                    Token(tok) if tok.kind() == TokenKind::Name => {
                        check_receiver_and_arguments(&x.function_call_receiver)
                    }
                    QualifiedName(_) => check_receiver_and_arguments(&x.function_call_receiver),
                    _ => default(self),
                }
            }
            FunctionPointerExpression(_) => {
                // Bans the equivalent of inst_meth as well as class_meth and fun
                if self.env.parser_options.po_disallow_func_ptrs_in_constants {
                    default(self)
                }
            }
            _ => default(self),
        }
    }

    fn check_static_in_initializer(&mut self, initializer: &'a Syntax<Token, Value>) -> bool {
        if let SimpleInitializer(x) = &initializer.syntax {
            if let ScopeResolutionExpression(x) = &x.simple_initializer_value.syntax {
                if let Token(t) = &x.scope_resolution_qualifier.syntax {
                    match t.kind() {
                        TokenKind::Static => return true,
                        TokenKind::Parent => {
                            return self
                                .text(&x.scope_resolution_name)
                                .eq_ignore_ascii_case("class")
                        }
                        _ => return false,
                    }
                }
            }
        };
        false
    }

    fn const_decl_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let ConstantDeclarator(cd) = &node.syntax {
            self.produce_error(
                |self_, x| self_.constant_abstract_with_initializer(x),
                &cd.constant_declarator_initializer,
                || errors::error2051,
                &cd.constant_declarator_initializer,
            );

            self.produce_error(
                |self_, x| self_.constant_concrete_without_initializer(x),
                &cd.constant_declarator_initializer,
                || errors::error2050,
                &cd.constant_declarator_initializer,
            );

            self.produce_error(
                |self_, x| self_.is_global_in_const_decl(x),
                &cd.constant_declarator_initializer,
                || errors::global_in_const_decl,
                &cd.constant_declarator_initializer,
            );
            self.check_constant_expression(&cd.constant_declarator_initializer);

            self.produce_error(
                |self_, x| self_.check_static_in_initializer(x),
                &cd.constant_declarator_initializer,
                || errors::parent_static_const_decl,
                &cd.constant_declarator_initializer,
            );

            if !cd.constant_declarator_name.is_missing() {
                let constant_name = self.text(&cd.constant_declarator_name);
                let location = Self::make_location_of_node(&cd.constant_declarator_name);
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
                            &cd.constant_declarator_name,
                            &combine_names(&self.namespace_name, &constant_name),
                            &constant_name,
                            &def.location,
                            &|x, y| errors::declared_name_is_already_in_use(line_num, x, y),
                        ))
                    }
                    _ => (),
                }
                self.names.constants.add(constant_name, def)
            }
        }
    }

    fn class_property_modifiers_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let PropertyDeclaration(x) = &node.syntax {
            let property_modifiers = &x.property_modifiers;

            let abstract_static_props = self.env.parser_options.po_abstract_static_props;
            self.invalid_modifier_errors("Properties", node, |kind| {
                if kind == TokenKind::Abstract {
                    return abstract_static_props;
                }
                kind == TokenKind::Static
                    || kind == TokenKind::Private
                    || kind == TokenKind::Protected
                    || kind == TokenKind::Public
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

    fn class_property_const_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let PropertyDeclaration(x) = &node.syntax {
            if self.attr_spec_contains_const(&x.property_attribute_spec)
                && self.attribute_specification_contains(
                    &x.property_attribute_spec,
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

    fn class_property_declarator_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let check_decls = |self_: &mut Self,
                           f: &dyn Fn(&'a Syntax<Token, Value>) -> bool,
                           error: errors::Error,
                           property_declarators| {
            Self::syntax_to_list_no_separators(property_declarators).for_each(|decl| {
                if let PropertyDeclarator(x) = &decl.syntax {
                    if f(&x.property_initializer) {
                        self_
                            .errors
                            .push(Self::make_error_from_node(node, error.clone()))
                    }
                }
            })
        };
        if let PropertyDeclaration(x) = &node.syntax {
            if self.env.parser_options.tco_const_static_props && Self::has_modifier_static(node) {
                if self.env.parser_options.po_abstract_static_props
                    && Self::has_modifier_abstract(node)
                {
                    check_decls(
                        self,
                        &|n| !n.is_missing(),
                        errors::abstract_prop_init,
                        &x.property_declarators,
                    )
                } else if self.attr_spec_contains_const(&x.property_attribute_spec) {
                    check_decls(
                        self,
                        &|n| n.is_missing(),
                        errors::const_static_prop_init,
                        &x.property_declarators,
                    )
                }
            }
        }
    }

    fn trait_use_alias_item_modifier_errors(&mut self, node: &'a Syntax<Token, Value>) {
        self.invalid_modifier_errors("Trait use aliases", node, |kind| {
            kind == TokenKind::Final
                || kind == TokenKind::Private
                || kind == TokenKind::Protected
                || kind == TokenKind::Public
        });
    }

    fn mixed_namespace_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            NamespaceBody(x) => {
                let s = Self::start_offset(&x.namespace_left_brace);
                let e = Self::end_offset(&x.namespace_right_brace);
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
                let s = Self::start_offset(&x.namespace_semicolon);
                let e = Self::end_offset(&x.namespace_semicolon);
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
                    syntax: Script(_), ..
                }, syntax_list] = self.parents.as_slice()
                {
                    if let SyntaxList(_) = syntax_list.syntax {
                        is_first_decl = false;
                        for decl in Self::syntax_to_list_no_separators(syntax_list) {
                            match &decl.syntax {
                                MarkupSection(x) => {
                                    if x.markup_text.width() == 0
                                        || self.is_hashbang(&x.markup_text)
                                    {
                                        continue;
                                    } else {
                                        break;
                                    }
                                }
                                NamespaceUseDeclaration(_) | FileAttributeSpecification(_) => (),
                                NamespaceDeclaration(_) => {
                                    is_first_decl = true;
                                    break;
                                }
                                _ => break,
                            }
                        }

                        has_code_outside_namespace = !(x.namespace_body.is_namespace_empty_body())
                            && Self::syntax_to_list_no_separators(syntax_list).any(|decl| {
                                match &decl.syntax {
                                    MarkupSection(x) => {
                                        !(x.markup_text.width() == 0
                                            || self.is_hashbang(&x.markup_text))
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
            _ => (),
        }
    }

    fn enumerator_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let Enumerator(x) = &node.syntax {
            if self.text(&x.enumerator_name).eq_ignore_ascii_case("class") {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::enum_elem_name_is_class,
                ))
            }
            self.check_constant_expression(&x.enumerator_value)
        }
    }

    fn enum_decl_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let EnumDeclaration(x) = &node.syntax {
            let attrs = &x.enum_attribute_spec;
            if self.attr_spec_contains_sealed(attrs) {
                self.errors
                    .push(Self::make_error_from_node(node, errors::sealed_enum))
            } else if self.attr_spec_contains_const(attrs) {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_const_interfaces_traits_enums,
                ))
            }

            if !x.enum_name.is_missing() {
                let name = self.text(&x.enum_name);
                let location = Self::make_location_of_node(&x.enum_name);
                self.check_type_name(&x.enum_name, name, location)
            }
        }
    }

    fn check_lvalue(&mut self, allow_reassign: bool, loperand: &'a Syntax<Token, Value>) {
        let append_errors = |self_: &mut Self, node, error| {
            self_.errors.push(Self::make_error_from_node(node, error))
        };

        let err = |self_: &mut Self, error| append_errors(self_, loperand, error);

        let check_unary_expression = |self_: &mut Self, op| match Self::token_kind(op) {
            Some(TokenKind::At) | Some(TokenKind::Dollar) => {}
            _ => err(self_, errors::not_allowed_in_write("Unary expression")),
        };

        match &loperand.syntax {
            ListExpression(x) => Self::syntax_to_list_no_separators(&x.list_members)
                .for_each(|n| self.check_lvalue(false, n)),
            SafeMemberSelectionExpression(_) => {
                err(self, errors::not_allowed_in_write("?-> operator"))
            }
            MemberSelectionExpression(x) => {
                if Self::token_kind(&x.member_name) == Some(TokenKind::XHPClassName) {
                    err(self, errors::not_allowed_in_write("->: operator"))
                }
            }
            VariableExpression(x) => {
                if !allow_reassign {
                    let text = self.text(&x.variable_expression);
                    if text == sn::special_idents::THIS {
                        err(self, errors::reassign_this)
                    } else if text == sn::superglobals::GLOBALS {
                        err(self, errors::not_allowed_in_write("$GLOBALS"))
                    }
                }
            }
            DecoratedExpression(x) => match Self::token_kind(&x.decorated_expression_decorator) {
                Some(TokenKind::Clone) => err(self, errors::not_allowed_in_write("Clone")),
                Some(TokenKind::Await) => err(self, errors::not_allowed_in_write("Await")),
                Some(TokenKind::Suspend) => err(self, errors::not_allowed_in_write("Suspend")),
                Some(TokenKind::QuestionQuestion) => {
                    err(self, errors::not_allowed_in_write("?? operator"))
                }
                Some(TokenKind::BarGreaterThan) => {
                    err(self, errors::not_allowed_in_write("|> operator"))
                }
                Some(TokenKind::Inout) => err(self, errors::not_allowed_in_write("Inout")),
                _ => {}
            },
            ParenthesizedExpression(x) => {
                self.check_lvalue(allow_reassign, &x.parenthesized_expression_expression)
            }
            SubscriptExpression(x) => self.check_lvalue(true, &x.subscript_receiver),
            LambdaExpression(_)
            | AnonymousFunction(_)
            | AwaitableCreationExpression(_)
            | ArrayIntrinsicExpression(_)
            | DarrayIntrinsicExpression(_)
            | VarrayIntrinsicExpression(_)
            | ShapeExpression(_)
            | CollectionLiteralExpression(_)
            | GenericTypeSpecifier(_)
            | YieldExpression(_)
            | YieldFromExpression(_)
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
            PrefixUnaryExpression(x) => check_unary_expression(self, &x.prefix_unary_operator),
            PostfixUnaryExpression(x) => check_unary_expression(self, &x.postfix_unary_operator),

            // FIXME: Array_get ((_, Class_const _), _) is not a valid lvalue. *)
            _ => {} // Ideally we should put all the rest of the syntax here so everytime
                    // a new syntax is added people need to consider whether the syntax
                    // can be a valid lvalue or not. However, there are too many of them.
        }
    }

    fn assignment_errors(&mut self, node: &'a Syntax<Token, Value>) {
        let check_rvalue = |self_: &mut Self, roperand: &'a Syntax<Token, Value>| {
            let append_errors = |self_: &mut Self, node, error| {
                self_.errors.push(Self::make_error_from_node(node, error))
            };
            match &roperand.syntax {
                VariableExpression(x)
                    if self_.text(&x.variable_expression) == sn::superglobals::GLOBALS =>
                {
                    append_errors(self_, roperand, errors::globals_without_subscript)
                }

                _ => (),
            }
        };

        let check_unary_expression = |self_: &mut Self, op, loperand: &'a Syntax<Token, Value>| {
            if Self::does_unop_create_write(Self::token_kind(op)) {
                self_.check_lvalue(true, loperand)
            }
        };
        match &node.syntax {
            PrefixUnaryExpression(x) => {
                check_unary_expression(self, &x.prefix_unary_operator, &x.prefix_unary_operand)
            }
            PostfixUnaryExpression(x) => {
                check_unary_expression(self, &x.postfix_unary_operator, &x.postfix_unary_operand)
            }
            DecoratedExpression(x) => {
                let loperand = &x.decorated_expression_expression;
                if Self::does_decorator_create_write(Self::token_kind(
                    &x.decorated_expression_decorator,
                )) {
                    self.check_lvalue(true, &loperand)
                }
            }
            BinaryExpression(x) => {
                let loperand = &x.binary_left_operand;
                let roperand = &x.binary_right_operand;
                if Self::does_binop_create_write_on_left(Self::token_kind(&x.binary_operator)) {
                    self.check_lvalue(false, &loperand);
                    check_rvalue(self, &roperand);
                }
            }
            ForeachStatement(x) => {
                self.check_lvalue(false, &x.foreach_value);
                self.check_lvalue(false, &x.foreach_key);
                check_rvalue(self, &x.foreach_collection);
            }
            _ => {}
        }
    }

    fn dynamic_method_call_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match &node.syntax {
            FunctionCallExpression(x) if !x.function_call_type_args.is_missing() => {
                let is_variable = |x| Self::is_token_kind(x, TokenKind::Variable);
                let is_dynamic = match &x.function_call_receiver.syntax {
                    ScopeResolutionExpression(x) => is_variable(&x.scope_resolution_name),
                    MemberSelectionExpression(x) => is_variable(&x.member_name),
                    SafeMemberSelectionExpression(x) => is_variable(&x.safe_member_name),
                    _ => false,
                };
                if is_dynamic {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::no_type_parameters_on_dynamic_method_calls,
                    ))
                }
            }
            _ => (),
        }
    }

    fn get_namespace_name(&self) -> String {
        if let Some(node) = self.nested_namespaces.last() {
            if let NamespaceDeclaration(x) = &node.syntax {
                let ns = &x.namespace_name;
                if !ns.is_missing() {
                    return combine_names(&self.namespace_name, self.text(ns));
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

    fn disabled_legacy_soft_typehint_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let SoftTypeSpecifier(_) = node.syntax {
            if self.env.parser_options.po_disable_legacy_soft_typehints {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::no_legacy_soft_typehints,
                ))
            }
        }
    }

    fn disabled_legacy_attribute_syntax_errors(&mut self, node: &'a Syntax<Token, Value>) {
        match node.syntax {
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

    fn param_default_decl_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let ParameterDeclaration(x) = &node.syntax {
            if self.env.parser_options.po_const_default_func_args {
                self.check_constant_expression(&x.parameter_default_value)
            }
        }
    }

    fn concurrent_statement_errors(&mut self, node: &'a Syntax<Token, Value>) {
        if let ConcurrentStatement(x) = &node.syntax {
            // issue error if concurrent blocks are nested
            if self.is_in_concurrent_block {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::nested_concurrent_blocks,
                ))
            };
            if let CompoundStatement(x) = &x.concurrent_statement.syntax {
                let statement_list = || Self::syntax_to_list_no_separators(&x.compound_statements);
                if statement_list().nth(1).is_none() {
                    self.errors.push(Self::make_error_from_node(
                        node,
                        errors::fewer_than_two_statements_in_concurrent_block,
                    ))
                }
                for n in statement_list() {
                    if let ExpressionStatement(x) = &n.syntax {
                        if !self.node_has_await_child(&x.expression_statement_expression) {
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
                for n in statement_list() {
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

    fn disabled_function_pointer_expression_error(&mut self, node: &'a Syntax<Token, Value>) {
        if let FunctionPointerExpression(_) = &node.syntax {
            if !self
                .env
                .parser_options
                .po_enable_first_class_function_pointers
            {
                self.errors.push(Self::make_error_from_node(
                    node,
                    errors::function_pointers_disabled,
                ))
            }
        }
    }

    fn check_qualified_name(&mut self, node: &'a Syntax<Token, Value>) {
        // The last segment in a qualified name should not have a trailing backslash
        // i.e. `Foospace\Bar\` except as the prefix of a GroupUseClause
        if let Some(Syntax {
            syntax: NamespaceGroupUseDeclaration(_),
            ..
        }) = self.parents.last()
        {
        } else {
            if let QualifiedName(x) = &node.syntax {
                let name_parts = &x.qualified_name_parts;
                let mut parts = Self::syntax_to_list_with_separators(name_parts);
                let last_part = parts.nth_back(0);
                match last_part {
                    Some(t) if Self::token_kind(t) == Some(TokenKind::Backslash) => self
                        .errors
                        .push(Self::make_error_from_node(t, errors::error0008)),
                    _ => (),
                }
            }
        }
    }

    fn check_preceding_backslashes_qualified_name(&mut self, node: &'a Syntax<Token, Value>) {
        // Qualified names as part of file level declarations
        // (group use, namespace use, namespace declarations) should not have preceding backslashes
        // `use namespace A\{\B}` will throw this error.
        if let QualifiedName(x) = &node.syntax {
            let name_parts = &x.qualified_name_parts;
            let mut parts = Self::syntax_to_list_with_separators(name_parts);
            let first_part = parts.find(|x| !x.is_missing());

            match first_part {
                Some(t) if Self::token_kind(t) == Some(TokenKind::Backslash) => self.errors.push(
                    Self::make_error_from_node(node, errors::preceding_backslash),
                ),
                _ => (),
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

    fn folder(&mut self, node: &'a Syntax<Token, Value>) {
        let has_rx_attr_mutable_hack = |self_: &mut Self, attrs| {
            self_
                .attribute_first_reactivity_annotation(attrs)
                .map_or(false, |node| {
                    self_.attr_name(node) != Some(sn::user_attributes::NON_RX)
                })
        };
        let mut prev_context = None;
        let mut pushed_nested_namespace = false;

        let named_function_context =
            |self_: &mut Self, node, s, prev_context: &mut Option<Context<'a, _>>| {
                *prev_context = Some(self_.env.context.clone());
                // a _single_ variable suffices as they cannot be nested
                self_.env.context.active_methodish = Some(node);
                // inspect the rx attribute directly.
                self_.env.context.active_is_rx_or_enclosing_for_lambdas =
                    has_rx_attr_mutable_hack(self_, s);
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

        match &node.syntax {
            ConstDeclaration(_) => {
                prev_context = Some(self.env.context.clone());
                self.env.context.active_const = Some(node)
            }
            FunctionDeclaration(x) => {
                named_function_context(self, node, &x.function_attribute_spec, &mut prev_context)
            }
            MethodishDeclaration(x) => {
                named_function_context(self, node, &x.methodish_attribute, &mut prev_context)
            }
            NamespaceDeclaration(x) => {
                let namespace_name = &x.namespace_name;
                if !namespace_name.is_missing() && !self.text(namespace_name).is_empty() {
                    pushed_nested_namespace = true;
                    self.nested_namespaces.push(node)
                }
            }
            AnonymousFunction(x) => {
                lambda_context(self, node, &x.anonymous_attribute_spec, &mut prev_context)
            }
            LambdaExpression(x) => {
                lambda_context(self, node, &x.lambda_attribute_spec, &mut prev_context)
            }
            AwaitableCreationExpression(x) => {
                lambda_context(self, node, &x.awaitable_attribute_spec, &mut prev_context)
            }
            ClassishDeclaration(_) => {
                prev_context = Some(self.env.context.clone());
                self.env.context.active_classish = Some(node)
            }
            _ => (),
        };

        self.parameter_errors(node);

        match &node.syntax {
            TryStatement(_)
            | UsingStatementFunctionScoped(_)
            | ForStatement(_)
            | CaseLabel(_)
            | DefaultLabel(_) => self.statement_errors(node),
            MethodishDeclaration(_) | FunctionDeclaration(_) | FunctionDeclarationHeader(_) => {
                self.reified_parameter_errors(node);
                self.redeclaration_errors(node);
                self.methodish_errors(node);
            }

            ArrayIntrinsicExpression(_) => self.expression_errors(node),
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
            | YieldFromExpression(_)
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
                self.dynamic_method_call_errors(node);
                self.expression_errors(node);
                self.check_nonrx_annotation(node);
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

            AliasDeclaration(_) => self.alias_errors(node),
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
            PostfixUnaryExpression(_) | BinaryExpression(_) | ForeachStatement(_) => {
                self.assignment_errors(node)
            }
            XHPEnumType(_) | XHPExpression(_) => self.xhp_errors(node),
            PropertyDeclarator(x) => {
                let init = &x.property_initializer;

                self.produce_error(
                    |self_, x| self_.check_static_in_initializer(x),
                    &init,
                    || errors::parent_static_prop_decl,
                    &init,
                );
                self.check_constant_expression(&init)
            }
            RecordField(x) => self.check_constant_expression(&x.record_field_init),
            XHPClassAttribute(x) => {
                self.check_constant_expression(&x.xhp_attribute_decl_initializer)
            }
            OldAttributeSpecification(_) => self.disabled_legacy_attribute_syntax_errors(node),
            SoftTypeSpecifier(_) => self.disabled_legacy_soft_typehint_errors(node),
            FunctionPointerExpression(_) => self.disabled_function_pointer_expression_error(node),
            QualifiedName(_) => self.check_qualified_name(node),
            _ => {}
        }
        self.lval_errors(node);

        match &node.syntax {
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
                    self.namespace_type = Bracketed(Self::make_location(
                        &x.namespace_left_brace,
                        &x.namespace_right_brace,
                    ))
                }

                let old_namespace_name = self.namespace_name.clone();
                let mut old_names = self.names.clone();
                // reset names before diving into namespace body,
                // keeping global function names
                self.namespace_name = self.get_namespace_name();
                let names_copy = std::mem::replace(&mut self.names, UsedNames::empty());
                self.names.functions = names_copy.functions.filter(|x| x.global);
                self.fold_child_nodes(node);

                // add newly declared global functions to the old set of names
                let names_copy = std::mem::replace(&mut self.names, UsedNames::empty());
                for (k, v) in names_copy.functions.into_iter().filter(|(_, x)| x.global) {
                    old_names.functions.add(&k, v)
                }
                // resume with old set of names and pull back
                // accumulated errors/last seen namespace type
                self.names = old_names;
                self.namespace_name = old_namespace_name;
            }
            NamespaceEmptyBody(x) => {
                if self.namespace_type == Unspecified {
                    self.namespace_type =
                        Unbracketed(Self::make_location_of_node(&x.namespace_semicolon))
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
                    std::mem::replace(&mut self.names.constants, YesCase(HashMap::new()));
                let functions =
                    std::mem::replace(&mut self.names.functions, NoCase(HashMap::new()));
                let trait_require_clauses = std::mem::replace(
                    &mut self.trait_require_clauses,
                    empty_trait_require_clauses(),
                );

                self.fold_child_nodes(node);

                self.trait_require_clauses = trait_require_clauses;
                self.names.functions = functions;
                self.names.constants = constants;
            }
            _ => self.fold_child_nodes(node),
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

    fn fold_child_nodes(&mut self, node: &'a Syntax<Token, Value>) {
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
        if self.env.is_typechecker() && !self.env.parser_options.po_disable_modes {
            self.is_invalid_hack_mode();
        }
        self.fold_child_nodes(self.env.syntax_tree.root());
        self.errors.reverse();
        self.errors
    }

    pub fn parse_errors(
        tree: &'a SyntaxTree<'a, Syntax<Token, Value>, State>,
        parser_options: ParserOptions,
        hhvm_compat_mode: bool,
        hhi_mode: bool,
        codegen: bool,
    ) -> Vec<SyntaxError> {
        let env = Env {
            parser_options,
            syntax_tree: tree,
            // TODO(kasper): do not repeat work if we have the indexed text already available somwhere)
            text: IndexedSourceText::new(tree.text().clone()),
            context: Context {
                active_classish: None,
                active_methodish: None,
                active_callable: None,
                active_callable_attr_spec: None,
                active_is_rx_or_enclosing_for_lambdas: false,
                active_const: None,
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

pub fn parse_errors<'a>(
    tree: &'a SyntaxTree<'a, PositionedSyntax, ()>,
    parser_options: ParserOptions,
    hhvm_compat_mode: bool,
    hhi_mode: bool,
    codegen: bool,
) -> Vec<SyntaxError> {
    ParserErrors::parse_errors(tree, parser_options, hhvm_compat_mode, hhi_mode, codegen)
}

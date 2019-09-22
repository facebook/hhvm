// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code, unused_imports, unused_variables)]

use lazy_static::lazy_static;
use regex::Regex;
use std::collections::{BTreeMap, HashMap, HashSet};

use naming_special_names_rust as sn;

use oxidized::parser_options::ParserOptions;
use parser_core_types::{
    indexed_source_text::IndexedSourceText,
    lexable_token::LexableToken,
    syntax::{ListItemChildren, Syntax, SyntaxValueType, SyntaxVariant, SyntaxVariant::*},
    syntax_error::{self as errors, Error, ErrorType, SyntaxError},
    syntax_trait::SyntaxTrait,
    token_kind::TokenKind,
};
use syntax_tree::SyntaxTree;

use parser_rust::hh_autoimport;

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
pub struct ParserErrors<'a, Token, Value, State> {
    phanotm: std::marker::PhantomData<(*const Token, *const Value, *const State)>,

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
            phanotm: std::marker::PhantomData,
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

    fn promoted_params(
        params: impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>>,
    ) -> impl DoubleEndedIterator<Item = &'a Syntax<Token, Value>> {
        params.filter(|node| match &node.syntax {
            ParameterDeclaration(x) => !x.parameter_visibility.is_missing(),
            _ => false,
        })
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

    fn token_text<'b>(&'b self, token: &Token) -> &'b str {
        self.env.text.source_text().sub_as_str(
            token.leading_start_offset().unwrap() + token.leading_width(),
            token.width(),
        )
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

    fn strip_ns(name: &str) -> &str {
        match name.chars().next() {
            Some('\\') => &name[1..],
            _ => name,
        }
    }

    fn is_global_namespace(&self) -> bool {
        self.namespace_name == GLOBAL_NAMESPACE_NAME
    }

    fn folder(&mut self, node: &'a Syntax<Token, Value>) {
        let has_rx_attr_mutable_hack = |self_: &mut Self, attrs| {
            // TODO
            false
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

        match &node.syntax {
            // TODO: check errors here
            _ => {}
        }

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
            text: IndexedSourceText::new(tree.text()),
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

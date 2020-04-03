/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
pub mod direct_decl_smart_constructors_generated;

use hh_autoimport_rust as hh_autoimport;
use naming_special_names_rust as naming_special_names;

use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use oxidized::{
    aast, aast_defs,
    ast_defs::{Bop, ClassKind, ConstraintKind, FunKind, Id, ShapeFieldName, Uop, Variance},
    decl_defs::MethodReactivity,
    errors::Errors,
    file_info::Mode,
    i_set::ISet,
    nast,
    pos::Pos,
    shallow_decl_defs,
    shape_map::{ShapeField, ShapeMap},
    tany_sentinel::TanySentinel,
    typing_defs,
    typing_defs::{
        EnumType, FunArity, FunElt, FunParam, FunParams, FunType, ParamMode, ParamMutability,
        PossiblyEnforcedTy, Reactivity, ShapeFieldType, ShapeKind, Tparam, Ty, Ty_, TypedefType,
    },
    typing_defs_flags,
    typing_reason::Reason,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken,
    lexable_trivia::LexablePositionedTrivia, positioned_token::PositionedToken,
    token_kind::TokenKind, trivia_kind::TriviaKind,
};
use std::borrow::Cow;
use std::collections::{BTreeMap, HashMap, HashSet};
use std::rc::Rc;

pub use crate::direct_decl_smart_constructors_generated::*;

type ParseError = String;

#[derive(Clone, Debug)]
pub struct InProgressDecls {
    pub classes: HashMap<String, Rc<shallow_decl_defs::ShallowClass>>,
    pub funs: HashMap<String, Rc<typing_defs::FunElt>>,
    pub typedefs: HashMap<String, Rc<typing_defs::TypedefType>>,
    pub consts: HashMap<String, Rc<typing_defs::Ty>>,
}

pub fn empty_decls() -> InProgressDecls {
    InProgressDecls {
        classes: HashMap::new(),
        funs: HashMap::new(),
        typedefs: HashMap::new(),
        consts: HashMap::new(),
    }
}

pub fn get_name(namespace: &str, name: &Node_) -> Result<Id, ParseError> {
    fn qualified_name_from_parts(
        namespace: &str,
        parts: &Vec<Node_>,
        pos: &Pos,
    ) -> Result<Id, ParseError> {
        let mut qualified_name = String::new();
        for part in parts {
            match part {
                Node_::Name(name, _pos) => qualified_name.push_str(&name),
                Node_::Backslash(_) => qualified_name.push('\\'),
                Node_::ListItem(listitem) => {
                    if let (Node_::Name(name, _), Node_::Backslash(_)) = &**listitem {
                        qualified_name.push_str(&name);
                        qualified_name.push_str("\\");
                    } else {
                        return Err(format!(
                            "Expected a name or backslash, but got {:?}",
                            listitem
                        ));
                    }
                }
                n => {
                    return Err(format!(
                        "Expected a name, backslash, or list item, but got {:?}",
                        n
                    ))
                }
            }
        }
        let name = if namespace.is_empty() {
            qualified_name // globally qualified name
        } else {
            let namespace = namespace.to_owned();
            let namespace = if namespace.ends_with("\\") {
                namespace
            } else {
                namespace + "\\"
            };
            namespace + &qualified_name
        };
        let pos = pos.clone();
        Ok(Id(pos, name))
    }

    match name {
        Node_::Name(name, pos) => {
            // always a simple name
            let name = name.to_string();
            let name = if namespace.is_empty() {
                name
            } else {
                namespace.to_owned() + "\\" + &name
            };
            let pos = pos.clone();
            Ok(Id(pos, name))
        }
        Node_::XhpName(name, pos) => {
            // xhp names are always unqualified
            Ok(Id(pos.clone(), name.to_string()))
        }
        Node_::QualifiedName(parts, pos) => qualified_name_from_parts(namespace, &parts, pos),
        Node_::Construct(pos) => Ok(Id(
            pos.clone(),
            naming_special_names::members::__CONSTRUCT.to_string(),
        )),
        n => {
            return Err(format!(
                "Expected a name, XHP name, or qualified name, but got {:?}",
                n,
            ))
        }
    }
}

fn prefix_slash<'a>(name: Cow<'a, String>) -> Cow<'a, String> {
    if name.starts_with("\\") {
        name
    } else {
        Cow::Owned("\\".to_owned() + &name)
    }
}

fn strip_dollar_prefix<'a>(name: Cow<'a, String>) -> Cow<'a, String> {
    if name.starts_with("$") {
        Cow::Owned(name.trim_start_matches("$").to_owned())
    } else {
        name
    }
}

fn tany() -> Ty {
    Ty(Reason::Rnone, Box::new(Ty_::Tany(TanySentinel)))
}

#[derive(Clone, Debug)]
struct NamespaceInfo {
    name: String,
    imports: BTreeMap<String, String>,
}

#[derive(Clone, Debug)]
enum NamespaceType {
    Simple(NamespaceInfo),
    Delimited(Vec<NamespaceInfo>),
}

#[derive(Clone, Debug)]
struct NamespaceBuilder {
    namespace: NamespaceType,
    in_progress_namespace: String,
    is_building_namespace: bool,
}

impl NamespaceBuilder {
    fn new() -> NamespaceBuilder {
        NamespaceBuilder {
            namespace: NamespaceType::Delimited(vec![NamespaceInfo {
                name: "".to_string(),
                imports: BTreeMap::new(),
            }]),
            in_progress_namespace: String::new(),
            is_building_namespace: false,
        }
    }

    fn set_namespace(&mut self) {
        // This clone isn't a perf mistake because we might keep mutating
        // self.in_progress_namespace and we don't want the current namespace to
        // reflect those changes.
        self.namespace = NamespaceType::Simple(NamespaceInfo {
            name: self.in_progress_namespace.clone(),
            imports: BTreeMap::new(),
        });
    }

    fn push_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            vec.push(NamespaceInfo {
                name: self.in_progress_namespace.clone(),
                imports: BTreeMap::new(),
            });
        }
    }

    fn pop_namespace(&mut self) {
        // If we're currently in a simple namespace, then having a delimited
        // namespace (the kind we'd be in if we're calling this method) is an
        // error. We're not in the business of handling errors in this parser,
        // so just ignore it.
        if let NamespaceType::Delimited(ref mut vec) = self.namespace {
            // This clone isn't a perf mistake because we might keep mutating
            // self.in_progress_namespace and we don't want the namespace stack
            // to reflect those changes.
            // Also, while we normally try to use the Result type to surface
            // errors that we see as part of the file being malformed, the only
            // way we can hit this pop without having hit the push first is
            // through coding error, so we panic instead.
            vec.pop()
                .expect("Attempted to pop from the namespace stack when there were no entries");
            if vec.is_empty() {
                panic!("Popping from the namespace stack has left it empty");
            }
            self.in_progress_namespace = self.current_namespace().to_string();
        }
    }

    fn current_namespace(&self) -> &str {
        match &self.namespace {
            NamespaceType::Simple(ni) => &ni.name,
            NamespaceType::Delimited(stack) => {
                stack.last().map(|ni| ni.name.as_str()).unwrap_or("")
            }
        }
    }

    fn add_import(&mut self, name: String, aliased_name: Option<String>) {
        let imports =
            match self.namespace {
                NamespaceType::Simple(ref mut ni) => &mut ni.imports,
                NamespaceType::Delimited(ref mut nis) => &mut nis
                    .last_mut()
                    .expect(
                        "Attempted to get the current import map, but namespace stack was empty",
                    )
                    .imports,
            };
        match aliased_name {
            Some(aliased_name) => {
                imports.insert(aliased_name, name);
            }
            None => {
                let aliased_name = name
                    .rsplit_terminator('\\')
                    .nth(0)
                    .expect("Expected at least one entry in import name")
                    .to_string();
                imports.insert(aliased_name, name);
            }
        };
    }

    fn rename_import<'a>(&'a self, name: Cow<'a, String>) -> Cow<'a, String> {
        let should_prepend_slash = name.starts_with('\\');
        let trimmed_name = name.trim_start_matches('\\');
        let check_import_map = |import_map: &'a BTreeMap<String, String>| {
            import_map.get(trimmed_name).map(|renamed| {
                if should_prepend_slash {
                    prefix_slash(Cow::Borrowed(renamed))
                } else {
                    Cow::Borrowed(renamed)
                }
            })
        };
        match &self.namespace {
            NamespaceType::Simple(ni) => {
                if let Some(name) = check_import_map(&ni.imports) {
                    return name;
                }
            }
            NamespaceType::Delimited(nis) => {
                for ni in nis.iter().rev() {
                    if let Some(name) = check_import_map(&ni.imports) {
                        return name;
                    }
                }
            }
        }
        check_import_map(&hh_autoimport::TYPES_MAP).unwrap_or(name)
    }
}

#[derive(Clone, Debug)]
enum ClassishNameBuilder {
    /// We are not in a classish declaration.
    NotInClassish,

    /// We saw a classish keyword token followed by a Name, so we make it
    /// available as the name of the containing class declaration.
    InClassish(Rc<(String, Pos)>),
}

impl ClassishNameBuilder {
    fn new() -> Self {
        ClassishNameBuilder::NotInClassish
    }

    fn lexed_name_after_classish_keyword(&mut self, name: &str, pos: &Pos) {
        use ClassishNameBuilder::*;
        match self {
            NotInClassish => {
                let mut class_name = String::with_capacity(1 + name.len());
                class_name.push('\\');
                class_name.push_str(name);
                *self = InClassish(Rc::new((class_name, pos.clone())))
            }
            InClassish(_) => (),
        }
    }

    fn parsed_classish_declaration(&mut self) {
        *self = ClassishNameBuilder::NotInClassish;
    }

    fn get_current_classish_name(&self) -> Option<(String, Pos)> {
        use ClassishNameBuilder::*;
        match self {
            NotInClassish => None,
            InClassish(name_and_pos) => Some((**name_and_pos).clone()),
        }
    }
}

#[derive(Clone, Debug)]
enum FileModeBuilder {
    // We haven't seen any tokens yet.
    None,

    // We've seen <? and we're waiting for the next token, which has the trivia
    // with the mode.
    Pending,

    // We either saw a <?, then `hh`, then a mode, or we didn't see that
    // sequence and we're defaulting to Mstrict.
    Set(Mode),
}

#[derive(Clone, Debug)]
pub struct State<'a> {
    pub source_text: IndexedSourceText<'a>,
    pub decls: Rc<InProgressDecls>,
    namespace_builder: Rc<NamespaceBuilder>,
    classish_name_builder: ClassishNameBuilder,

    // We don't need to wrap this in a Cow because it's very small.
    file_mode_builder: FileModeBuilder,

    previous_token_kind: TokenKind,
}

impl<'a> State<'a> {
    pub fn new(source_text: IndexedSourceText) -> State {
        State {
            source_text,
            decls: Rc::new(empty_decls()),
            namespace_builder: Rc::new(NamespaceBuilder::new()),
            classish_name_builder: ClassishNameBuilder::new(),
            file_mode_builder: FileModeBuilder::None,
            // EndOfFile is used here as a None value (signifying "beginning of
            // file") to save space. There is no legitimate circumstance where
            // we would parse a token and the previous token kind would be
            // EndOfFile.
            previous_token_kind: TokenKind::EndOfFile,
        }
    }
}

#[derive(Clone, Debug)]
pub enum HintValue {
    Void,
    Int,
    Bool,
    Float,
    String,
    Resource,
    Num,
    ArrayKey,
    NoReturn,
    Apply(Box<(Id, Vec<Node_>)>),
    Access(Box<(Node_, Vec<Id>)>),
    Array(Box<(Node_, Node_)>),
    Varray(Box<Node_>),
    Darray(Box<(Node_, Node_)>),
    Mixed,
    Tuple(Vec<Node_>),
    Shape(Box<ShapeDecl>),
    Nullable(Box<Node_>),
    LikeType(Box<Node_>),
    Soft(Box<Node_>),
    Closure(Box<ClosureTypeHint>),
}

#[derive(Clone, Debug)]
pub struct ConstDecl {
    modifiers: Node_,
    id: Id,
    ty: Ty,
    expr: Option<Box<nast::Expr>>,
}

#[derive(Clone, Debug)]
pub struct VariableDecl {
    attributes: Node_,
    visibility: Node_,
    kind: ParamMode,
    hint: Node_,
    id: Id,
    variadic: bool,
    initializer: Node_,
}

#[derive(Clone, Debug)]
pub struct Attribute {
    id: Id,
    args: Vec<Node_>,
}

#[derive(Clone, Debug)]
pub struct FunctionHeader {
    name: Node_,
    modifiers: Node_,
    type_params: Node_,
    param_list: Node_,
    ret_hint: Node_,
}

#[derive(Clone, Debug)]
pub struct FunctionDecl {
    attributes: Node_,
    header: FunctionHeader,
    body: Node_,
}

#[derive(Clone, Debug)]
pub struct PropertyDecl {
    attrs: Node_,
    modifiers: Node_,
    hint: Node_,
    id: Id,
    expr: Option<Box<nast::Expr>>,
}

#[derive(Clone, Debug)]
pub struct RequireClause {
    require_type: Node_,
    name: Node_,
}

#[derive(Clone, Debug)]
pub struct ShapeFieldDecl {
    is_optional: bool,
    name: Node_,
    type_: Node_,
}

#[derive(Clone, Debug)]
pub struct ShapeDecl {
    kind: ShapeKind,
    fields: Vec<ShapeFieldDecl>,
}

#[derive(Clone, Debug)]
pub struct TypeParameterDecl {
    name: Node_,
    reified: aast::ReifyKind,
    variance: Variance,

    // Box the insides of the vector so we don't need to reallocate them when
    // we pull them out of the TypeConstraint variant.
    constraints: Vec<Box<(ConstraintKind, Node_)>>,
}

#[derive(Clone, Debug)]
pub struct ClosureTypeHint {
    args: Node_,
    ret_hint: Node_,
}

#[derive(Clone, Debug)]
pub struct NamespaceUseClause {
    id: Id,
    as_: Option<String>,
}

#[derive(Clone, Debug)]
pub struct TypeConstant {
    id: Id,
    constraints: Vec<Node_>,
    type_: Node_,
    abstract_: bool,
    reified: Option<Pos>,
}

#[derive(Clone, Debug)]
pub enum OperatorType {
    Tilde,
    Not,
    Plus,
    Minus,
    PlusPlus,
    MinusMinus,
    Silence,
    Star,
    Slash,
    Eqeq,
    Eqeqeq,
    Starstar,
    Diff,
    Diff2,
    Ampamp,
    Barbar,
    LogXor,
    Lt,
    Lte,
    Gt,
    Gte,
    Dot,
    Amp,
    Bar,
    Ltlt,
    Gtgt,
    Percent,
    Xor,
    Cmp,
    QuestionQuestion,
}

#[derive(Clone, Debug)]
pub enum Node_ {
    List(Vec<Node_>),
    BracketedList(Box<(Pos, Vec<Node_>, Pos)>),
    Ignored,
    Name(String, Pos),
    XhpName(String, Pos),
    QualifiedName(Vec<Node_>, Pos),
    Array(Pos),
    Darray(Pos),
    Varray(Pos),
    StringLiteral(String, Pos),   // For shape keys and const expressions.
    IntLiteral(String, Pos),      // For const expressions.
    FloatingLiteral(String, Pos), // For const expressions.
    BooleanLiteral(String, Pos),  // For const expressions.
    Null(Pos),                    // For const expressions.
    Hint(HintValue, Pos),
    Backslash(Pos), // This needs a pos since it shows up in names.
    ListItem(Box<(Node_, Node_)>),
    Const(Box<ConstDecl>),
    Variable(Box<VariableDecl>),
    Attribute(Box<Attribute>),
    FunctionHeader(Box<FunctionHeader>),
    Function(Box<FunctionDecl>),
    Property(Box<PropertyDecl>),
    TraitUse(Box<Node_>),
    TypeConstant(Box<TypeConstant>),
    RequireClause(Box<RequireClause>),
    ClassishBody(Vec<Node_>),
    TypeParameter(Box<TypeParameterDecl>),
    TypeConstraint(Box<(ConstraintKind, Node_)>),
    ShapeFieldSpecifier(Box<ShapeFieldDecl>),
    NamespaceUseClause(Box<NamespaceUseClause>),
    Expr(Box<nast::Expr>),
    Operator(Pos, OperatorType),
    Construct(Pos),
    LessThan(Pos),    // This needs a pos since it shows up in generics.
    GreaterThan(Pos), // This needs a pos since it shows up in generics.
    LeftParen(Pos),   // This needs a pos since it shows up in tuples and shapes.
    RightParen(Pos),  // This needs a pos since it shows up in tuples and shapes.
    Shape(Pos),       // This needs a pos since it shows up in shapes.
    Question(Pos),    // This needs a pos since it shows up in nullable types.
    This(Pos),        // This needs a pos since it shows up in Taccess.
    ColonColon(Pos),  // This needs a pos since it shows up in Taccess.

    LessThanLessThan(Pos), // This needs a pos since it shows up in attributized type specifiers.
    GreaterThanGreaterThan(Pos), // This needs a pos since it shows up in attributized type specifiers.

    // Simple keywords and tokens.
    Abstract,
    As,
    Async,
    Coroutine,
    DotDotDot,
    Extends,
    Final,
    Implements,
    Inout,
    Interface,
    Private,
    Protected,
    Public,
    Reify,
    Semicolon,
    Static,
    Super,
    Trait,
    XHP,
    Yield,
}

impl Node_ {
    pub fn get_pos(&self) -> Result<Pos, ParseError> {
        match self {
            Node_::Name(_, pos) => Ok(pos.clone()),
            Node_::Hint(_, pos) => Ok(pos.clone()),
            Node_::XhpName(_, pos) => Ok(pos.clone()),
            Node_::QualifiedName(_, pos) => Ok(pos.clone()),
            Node_::Backslash(pos)
            | Node_::ColonColon(pos)
            | Node_::Construct(pos)
            | Node_::LessThan(pos)
            | Node_::GreaterThan(pos)
            | Node_::LeftParen(pos)
            | Node_::RightParen(pos)
            | Node_::Question(pos)
            | Node_::Shape(pos)
            | Node_::This(pos)
            | Node_::Array(pos)
            | Node_::Darray(pos)
            | Node_::Varray(pos)
            | Node_::LessThanLessThan(pos)
            | Node_::GreaterThanGreaterThan(pos)
            | Node_::IntLiteral(_, pos)
            | Node_::FloatingLiteral(_, pos)
            | Node_::Null(pos)
            | Node_::StringLiteral(_, pos)
            | Node_::BooleanLiteral(_, pos)
            | Node_::Operator(pos, _) => Ok(pos.clone()),
            Node_::ListItem(items) => {
                let fst = &items.0;
                let snd = &items.1;
                match (fst.get_pos(), snd.get_pos()) {
                    (Ok(fst_pos), Ok(snd_pos)) => Pos::merge(&fst_pos, &snd_pos),
                    (Ok(pos), Err(_)) => Ok(pos),
                    (Err(_), Ok(pos)) => Ok(pos),
                    (Err(_), Err(_)) => Err(format!("No pos found for {:?} or {:?}", fst, snd)),
                }
            }
            Node_::List(items) => self.pos_from_vec(&items),
            Node_::BracketedList(innards) => {
                let (first_pos, inner_list, second_pos) = &**innards;
                Pos::merge(
                    &first_pos,
                    &Pos::merge(&self.pos_from_vec(&inner_list)?, &second_pos)?,
                )
            }
            Node_::Expr(innards) => {
                let aast::Expr(pos, _) = &**innards;
                Ok(pos.clone())
            }
            _ => Err(format!("No pos found for node {:?}", self)),
        }
    }

    fn pos_from_vec(&self, nodes: &Vec<Node_>) -> Result<Pos, ParseError> {
        nodes.iter().fold(
            Err(format!("No pos found for any children under {:?}", self)),
            |acc, elem| match (acc, elem.get_pos()) {
                (Ok(acc_pos), Ok(elem_pos)) => Pos::merge(&acc_pos, &elem_pos),
                (Err(_), Ok(elem_pos)) => Ok(elem_pos),
                (acc, Err(_)) => acc,
            },
        )
    }

    fn into_vec(self) -> Vec<Self> {
        match self {
            Node_::List(items) => items,
            Node_::BracketedList(innards) => {
                let (_, items, _) = *innards;
                items
            }
            Node_::Ignored => Vec::new(),
            n => vec![n],
        }
    }

    fn into_iter(self) -> std::vec::IntoIter<Self> {
        self.into_vec().into_iter()
    }

    fn iter(&self) -> NodeIterHelper {
        match self {
            Node_::List(items) => NodeIterHelper::Vec(items.iter()),
            Node_::BracketedList(innards) => {
                let (_, items, _) = &**innards;
                NodeIterHelper::Vec(items.iter())
            }
            Node_::Ignored => NodeIterHelper::Empty,
            n => NodeIterHelper::Single(n),
        }
    }

    fn as_visibility(&self) -> Result<aast::Visibility, ParseError> {
        match self {
            Node_::Private => Ok(aast::Visibility::Private),
            Node_::Protected => Ok(aast::Visibility::Protected),
            Node_::Public => Ok(aast::Visibility::Public),
            n => Err(format!("Expected a visibility modifier, but was {:?}", n)),
        }
    }

    fn as_expr(&self) -> Result<nast::Expr, ParseError> {
        let expr_ = match self {
            Node_::Expr(expr) => return Ok(*expr.clone()),
            Node_::IntLiteral(s, _) => aast::Expr_::Int(s.to_string()),
            Node_::FloatingLiteral(s, _) => aast::Expr_::Float(s.to_string()),
            Node_::StringLiteral(s, _) => aast::Expr_::String(s.to_string()),
            Node_::BooleanLiteral(s, _) => {
                if s.eq_ignore_ascii_case("true") {
                    aast::Expr_::True
                } else {
                    aast::Expr_::False
                }
            }
            Node_::Null(_) => aast::Expr_::Null,
            n => return Err(format!("Could not construct an Expr for {:?}", n)),
        };
        let pos = self.get_pos()?;
        Ok(aast::Expr(pos, expr_))
    }

    fn as_attributes(&self) -> Result<Attributes, ParseError> {
        let mut attributes = Attributes {
            reactivity: Reactivity::Nonreactive,
            param_mutability: None,
            deprecated: None,
            reifiable: None,
            returns_mutable: false,
            late_init: false,
            const_: false,
            lsb: false,
            memoizelsb: false,
            override_: false,
            at_most_rx_as_func: false,
        };

        let mut reactivity_condition_type = None;
        for attribute in self.iter() {
            if let Node_::Attribute(attribute) = attribute {
                // If we see the attribute `__OnlyRxIfImpl(Foo::class)`, set
                // `reactivity_condition_type` to `Foo`.
                match (attribute.id.1.as_ref(), &attribute.args[..]) {
                    ("__OnlyRxIfImpl", [Node_::Expr(expr)]) => {
                        let aast::Expr(pos, expr_) = &**expr;
                        match &*expr_ {
                            aast::Expr_::ClassConst(args) => {
                                let (aast::ClassId(_, class_id_), (_, const_name)) = &**args;
                                match class_id_ {
                                    aast::ClassId_::CI(class_name) if const_name == "class" => {
                                        reactivity_condition_type = Some(Ty(
                                            Reason::Rhint(pos.clone()),
                                            Box::new(Ty_::Tapply(class_name.clone(), vec![])),
                                        ));
                                    }
                                    _ => (),
                                }
                            }
                            _ => (),
                        }
                    }
                    _ => (),
                }
            }
        }

        for attribute in self.iter() {
            if let Node_::Attribute(attribute) = attribute {
                match attribute.id.1.as_ref() {
                    // NB: It is an error to specify more than one of __Rx,
                    // __RxShallow, and __RxLocal, so to avoid cloning the
                    // condition type, we use Option::take here.
                    "__Rx" => {
                        attributes.reactivity =
                            Reactivity::Reactive(reactivity_condition_type.take())
                    }
                    "__RxShallow" => {
                        attributes.reactivity =
                            Reactivity::Shallow(reactivity_condition_type.take())
                    }
                    "__RxLocal" => {
                        attributes.reactivity = Reactivity::Local(reactivity_condition_type.take())
                    }
                    "__Mutable" => {
                        attributes.param_mutability = Some(ParamMutability::ParamBorrowedMutable)
                    }
                    "__MaybeMutable" => {
                        attributes.param_mutability = Some(ParamMutability::ParamMaybeMutable)
                    }
                    "__OwnedMutable" => {
                        attributes.param_mutability = Some(ParamMutability::ParamOwnedMutable)
                    }
                    "__MutableReturn" => attributes.returns_mutable = true,
                    "__Deprecated" => {
                        attributes.deprecated = attribute.args.first().and_then(|node| match node {
                            Node_::StringLiteral(val, _) => Some(val.clone()),
                            _ => None,
                        })
                    }
                    "__Reifiable" => attributes.reifiable = Some(attribute.id.0.clone()),
                    "__LateInit" => {
                        attributes.late_init = true;
                    }
                    "__Const" => {
                        attributes.const_ = true;
                    }
                    "__LSB" => {
                        attributes.lsb = true;
                    }
                    "__MemoizeLSB" => {
                        attributes.memoizelsb = true;
                    }
                    "__Override" => {
                        attributes.override_ = true;
                    }
                    "__AtMostRxAsFunc" => {
                        attributes.at_most_rx_as_func = true;
                    }
                    _ => (),
                }
            } else {
                return Err(format!("Expected an attribute, but was {:?}", self));
            }
        }

        Ok(attributes)
    }

    fn is_ignored(&self) -> bool {
        match self {
            Node_::Ignored => true,
            _ => false,
        }
    }
}

pub type Node = Result<Node_, ParseError>;

struct Attributes {
    reactivity: Reactivity,
    param_mutability: Option<ParamMutability>,
    deprecated: Option<String>,
    reifiable: Option<Pos>,
    returns_mutable: bool,
    late_init: bool,
    const_: bool,
    lsb: bool,
    memoizelsb: bool,
    override_: bool,
    at_most_rx_as_func: bool,
}

impl DirectDeclSmartConstructors<'_> {
    fn set_mode(&mut self, token: &PositionedToken) {
        for trivia in &token.trailing {
            if trivia.kind == TriviaKind::SingleLineComment {
                match &*String::from_utf8_lossy(
                    trivia.text_raw(self.state.source_text.source_text()),
                )
                .trim_start_matches('/')
                .trim()
                {
                    "decl" => self.state.file_mode_builder = FileModeBuilder::Set(Mode::Mdecl),
                    "partial" => {
                        self.state.file_mode_builder = FileModeBuilder::Set(Mode::Mpartial)
                    }
                    "strict" => self.state.file_mode_builder = FileModeBuilder::Set(Mode::Mstrict),
                    _ => self.state.file_mode_builder = FileModeBuilder::Set(Mode::Mstrict),
                }
            }
        }
    }

    fn node_to_ty(
        &self,
        node: &Node_,
        type_variables: &HashSet<Rc<String>>,
    ) -> Result<Ty, ParseError> {
        match node {
            Node_::Hint(hv, pos) => {
                let reason = Reason::Rhint(pos.clone());
                let ty_ = match hv {
                    HintValue::Void => Ty_::Tprim(aast::Tprim::Tvoid),
                    HintValue::Int => Ty_::Tprim(aast::Tprim::Tint),
                    HintValue::Bool => Ty_::Tprim(aast::Tprim::Tbool),
                    HintValue::Float => Ty_::Tprim(aast::Tprim::Tfloat),
                    HintValue::String => Ty_::Tprim(aast::Tprim::Tstring),
                    HintValue::Resource => Ty_::Tprim(aast::Tprim::Tresource),
                    HintValue::Num => Ty_::Tprim(aast::Tprim::Tnum),
                    HintValue::ArrayKey => Ty_::Tprim(aast::Tprim::Tarraykey),
                    HintValue::NoReturn => Ty_::Tprim(aast::Tprim::Tnoreturn),
                    HintValue::Apply(innards) => {
                        let (id, inner_types) = &**innards;
                        match id.1.trim_start_matches("\\") {
                            "varray_or_darray" => {
                                match inner_types.as_slice() {
                                    [tk, tv] => {
                                        Ty_::TvarrayOrDarray(Some(self.node_to_ty(tk, type_variables)?), self.node_to_ty(tv, type_variables)?)
                                    }
                                    [tv] => {
                                        Ty_::TvarrayOrDarray(None, self.node_to_ty(tv, type_variables)?)
                                    }
                                    _ => return Err(format!("Expected one or two type arguments on varray_or_darray, got {}", inner_types.len())),
                                }
                            }
                            _ => {
                                let id = Id(
                                    id.0.clone(),
                                    self.state.namespace_builder.rename_import(Cow::Borrowed(&id.1)).into_owned(),
                                );
                                Ty_::Tapply(
                                    id,
                                    inner_types
                                        .iter()
                                        .map(|node| self.node_to_ty(node, type_variables))
                                        .collect::<Result<Vec<_>, ParseError>>()?,
                                )
                            }
                        }
                    }
                    HintValue::Access(innards) => {
                        let (ty, names) = &**innards;
                        let ty = match ty {
                            Node_::Name(name, self_pos) if name == "self" => {
                                match self.state.classish_name_builder.get_current_classish_name() {
                                    Some((name, class_name_pos)) => {
                                        // In classes, we modify the position when rewriting the
                                        // `self` keyword to point to the class name. In traits, we
                                        // don't (for some reason). We indicate that the position
                                        // shouldn't be rewritten with the none Pos.
                                        let id_pos = if class_name_pos.is_none() {
                                            self_pos.clone()
                                        } else {
                                            class_name_pos
                                        };
                                        let reason = Reason::Rhint(self_pos.clone());
                                        let ty_ = Ty_::Tapply(Id(id_pos, name), Vec::new());
                                        Ty(reason, Box::new(ty_))
                                    }
                                    None => self.node_to_ty(ty, type_variables)?,
                                }
                            }
                            _ => self.node_to_ty(ty, type_variables)?,
                        };
                        Ty_::Taccess(typing_defs::TaccessType(ty, names.to_vec()))
                    }
                    HintValue::Array(innards) => {
                        let (key_type, value_type) = &**innards;
                        let key_type = match key_type {
                            Node_::Ignored => None,
                            n => Some(self.node_to_ty(n, type_variables)?),
                        };
                        let value_type = match value_type {
                            Node_::Ignored => None,
                            n => Some(self.node_to_ty(n, type_variables)?),
                        };
                        Ty_::Tarray(key_type, value_type)
                    }
                    HintValue::Darray(innards) => {
                        let (key_type, value_type) = &**innards;
                        Ty_::Tdarray(
                            self.node_to_ty(key_type, type_variables)?,
                            self.node_to_ty(value_type, type_variables)?,
                        )
                    }
                    HintValue::Varray(inner) => {
                        Ty_::Tvarray(self.node_to_ty(inner, type_variables)?)
                    }
                    HintValue::Mixed => Ty_::Tmixed,
                    HintValue::Tuple(items) => Ty_::Ttuple(
                        items
                            .iter()
                            .map(|node| self.node_to_ty(node, type_variables))
                            .collect::<Result<Vec<_>, ParseError>>()?,
                    ),
                    HintValue::Shape(innards) => {
                        let shape_decl = &**innards;
                        Ty_::Tshape(
                        shape_decl.kind,
                        shape_decl.fields
                            .iter()
                            .map(|field_decl| {
                                let name = match &field_decl.name {
                                    Node_::StringLiteral(s, pos) => Ok(ShapeFieldName::SFlitStr((pos.clone(), s.to_string()))),
                                    n => Err(format!("Expected a string literal for shape key name, but was {:?}", n))
                                }?;
                                Ok((
                                    ShapeField(name),
                                    ShapeFieldType {
                                        optional: field_decl.is_optional,
                                        ty: self.node_to_ty(&field_decl.type_, type_variables)?,
                                    },
                                ))
                            })
                            .collect::<Result<ShapeMap<_>, ParseError>>()?,
                    )
                    }
                    HintValue::Nullable(hint) => {
                        Ty_::Toption(self.node_to_ty(hint, type_variables)?)
                    }
                    HintValue::LikeType(hint) => Ty_::Tlike(self.node_to_ty(hint, type_variables)?),
                    HintValue::Soft(hint) => {
                        // Use the pos from the Soft node (above, when we
                        // construct `reason`) so that we include the position
                        // of the attribute list, but throw away the knowledge
                        // that we had a soft type specifier here (the
                        // typechecker does not use it)
                        let Ty(_, ty_) = self.node_to_ty(hint, type_variables)?;
                        *ty_
                    }
                    HintValue::Closure(hint) => {
                        let params = hint
                            .args
                            .iter()
                            .map(|node| {
                                Ok(FunParam {
                                    pos: node.get_pos()?,
                                    name: None,
                                    type_: PossiblyEnforcedTy {
                                        enforced: false,
                                        type_: self.node_to_ty(node, type_variables)?,
                                    },
                                    kind: ParamMode::FPnormal,
                                    accept_disposable: false,
                                    mutability: None,
                                    rx_annotation: None,
                                })
                            })
                            .collect::<Result<Vec<_>, ParseError>>()?;
                        let ret = self.node_to_ty(&hint.ret_hint, type_variables)?;
                        Ty_::Tfun(FunType {
                            arity: FunArity::Fstandard(params.len() as isize),
                            tparams: Vec::new(),
                            where_constraints: Vec::new(),
                            params,
                            ret: PossiblyEnforcedTy {
                                enforced: false,
                                type_: ret,
                            },
                            reactive: Reactivity::Nonreactive,
                            flags: 0,
                        })
                    }
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
            Node_::Array(pos) => Ok(Ty(
                Reason::Rhint(pos.clone()),
                Box::new(Ty_::Tarray(None, None)),
            )),
            Node_::This(pos) => Ok(Ty(Reason::Rhint(pos.clone()), Box::new(Ty_::Tthis))),
            Node_::Expr(expr) => {
                fn expr_to_ty(expr: &nast::Expr) -> Result<Ty_, ParseError> {
                    use aast::Expr_::*;
                    match &expr.1 {
                        Null => Ok(Ty_::Tprim(aast::Tprim::Tnull)),
                        This => Ok(Ty_::Tthis),
                        True | False => Ok(Ty_::Tprim(aast::Tprim::Tbool)),
                        Int(_) => Ok(Ty_::Tprim(aast::Tprim::Tint)),
                        Float(_) => Ok(Ty_::Tprim(aast::Tprim::Tfloat)),
                        String(_) => Ok(Ty_::Tprim(aast::Tprim::Tstring)),
                        String2(_) => Ok(Ty_::Tprim(aast::Tprim::Tstring)),
                        PrefixedString(_) => Ok(Ty_::Tprim(aast::Tprim::Tstring)),
                        Unop(innards) => expr_to_ty(&innards.1),
                        ParenthesizedExpr(expr) => expr_to_ty(&expr),
                        Any => Ok(Ty_::Tany(TanySentinel {})),

                        Array(_) | ArrayGet(_) | As(_) | Assert(_) | Await(_) | Binop(_)
                        | BracedExpr(_) | Call(_) | Callconv(_) | Cast(_) | ClassConst(_)
                        | ClassGet(_) | Clone(_) | Collection(_) | Darray(_) | Dollardollar(_)
                        | Efun(_) | Eif(_) | ExprList(_) | FunctionPointer(_) | FunId(_)
                        | Id(_) | Import(_) | Is(_) | KeyValCollection(_) | Lfun(_) | List(_)
                        | Lplaceholder(_) | Lvar(_) | MethodCaller(_) | MethodId(_) | New(_)
                        | ObjGet(_) | Omitted | Pair(_) | Pipe(_) | PUAtom(_) | PUIdentifier(_)
                        | Record(_) | Shape(_) | SmethodId(_) | Suspend(_) | Typename(_)
                        | ValCollection(_) | Varray(_) | Xml(_) | Yield(_) | YieldBreak
                        | YieldFrom(_) => Err(format!("Cannot convert expr to type: {:?}", expr)),
                    }
                }

                Ok(Ty(
                    Reason::Rwitness(expr.0.clone()),
                    Box::new(expr_to_ty(&expr)?),
                ))
            }
            Node_::IntLiteral(_, pos) => Ok(Ty(
                Reason::Rwitness(pos.clone()),
                Box::new(Ty_::Tprim(aast::Tprim::Tint)),
            )),
            Node_::FloatingLiteral(_, pos) => Ok(Ty(
                Reason::Rwitness(pos.clone()),
                Box::new(Ty_::Tprim(aast::Tprim::Tfloat)),
            )),
            Node_::StringLiteral(_, pos) => Ok(Ty(
                Reason::Rwitness(pos.clone()),
                Box::new(Ty_::Tprim(aast::Tprim::Tstring)),
            )),
            Node_::BooleanLiteral(_, pos) => Ok(Ty(
                Reason::Rwitness(pos.clone()),
                Box::new(Ty_::Tprim(aast::Tprim::Tbool)),
            )),
            Node_::Null(pos) => Ok(Ty(
                Reason::Rhint(pos.clone()),
                Box::new(Ty_::Tprim(aast::Tprim::Tnull)),
            )),
            node => {
                let Id(pos, name) = get_name("", node)?;
                let reason = Reason::Rhint(pos.clone());
                let ty_ = if type_variables.contains(&name) {
                    Ty_::Tgeneric(name)
                } else {
                    match name.as_ref() {
                        "nothing" => Ty_::Tunion(Vec::new()),
                        "nonnull" => Ty_::Tnonnull,
                        "dynamic" => Ty_::Tdynamic,
                        _ => {
                            let name = self
                                .state
                                .namespace_builder
                                .rename_import(self.prefix_ns(Cow::Owned(name)));
                            Ty_::Tapply(Id(pos, name.into_owned()), Vec::new())
                        }
                    }
                };
                Ok(Ty(reason, Box::new(ty_)))
            }
        }
    }

    /// Converts any node that can represent a list of Node_::TypeParameter
    /// into the type parameter list and a list of all type variables. Used for
    /// classes, methods, and functions.
    fn into_type_params(
        &self,
        node: Node_,
    ) -> Result<(Vec<Tparam>, HashSet<Rc<String>>), ParseError> {
        let mut type_variables = HashSet::new();
        let type_params = node
            .into_iter()
            .map(|node| {
                let TypeParameterDecl {
                    name,
                    variance,
                    reified,
                    constraints,
                } = match node {
                    Node_::TypeParameter(innards) => *innards,
                    n => return Err(format!("Expected a type parameter, but got {:?}", n)),
                };
                let id = get_name("", &name)?;
                let constraints = constraints
                    .into_iter()
                    .map(|constraint| {
                        let (kind, value) = *constraint;
                        Ok((kind, self.node_to_ty(&value, &HashSet::new())?))
                    })
                    .collect::<Result<Vec<_>, ParseError>>()?;
                type_variables.insert(Rc::new(id.1.clone()));
                Ok(Tparam {
                    variance,
                    name: id,
                    constraints,
                    reified,
                    user_attributes: Vec::new(),
                })
            })
            .collect::<Result<Vec<_>, ParseError>>()?;
        Ok((type_params, type_variables))
    }

    fn function_into_ty(
        &self,
        namespace: &str,
        attributes: Node_,
        header: FunctionHeader,
        body: Node_,
        outer_type_variables: &HashSet<Rc<String>>,
    ) -> Result<Box<(Id, Ty, Vec<PropertyDecl>)>, ParseError> {
        let id = get_name(namespace, &header.name)?;
        let (type_params, mut type_variables) = self.into_type_params(header.type_params)?;
        type_variables.extend(outer_type_variables.into_iter().map(Rc::clone));
        let (params, properties, arity) =
            self.into_variables_list(header.param_list, &type_variables)?;
        let type_ = match header.name {
            Node_::Construct(pos) => Ty(
                Reason::Rwitness(pos),
                Box::new(Ty_::Tprim(aast::Tprim::Tvoid)),
            ),
            _ => self
                .node_to_ty(&header.ret_hint, &type_variables)
                .unwrap_or_else(|_| tany()),
        };
        let (async_, is_coroutine) = header.modifiers.iter().fold(
            (false, false),
            |(async_, is_coroutine), node| match node {
                Node_::Async => (true, is_coroutine),
                Node_::Coroutine => (async_, true),
                _ => (async_, is_coroutine),
            },
        );
        let fun_kind = if is_coroutine {
            FunKind::FCoroutine
        } else {
            if body.iter().any(|node| match node {
                Node_::Yield => true,
                _ => false,
            }) {
                if async_ {
                    FunKind::FAsyncGenerator
                } else {
                    FunKind::FGenerator
                }
            } else {
                if async_ {
                    FunKind::FAsync
                } else {
                    FunKind::FSync
                }
            }
        };
        let attributes = attributes.as_attributes()?;
        // TODO(hrust) Put this in a helper. Possibly do this for all flags.
        let mut flags = match fun_kind {
            FunKind::FSync => 0,
            FunKind::FAsync => typing_defs_flags::FT_FLAGS_ASYNC,
            FunKind::FGenerator => typing_defs_flags::FT_FLAGS_GENERATOR,
            FunKind::FAsyncGenerator => {
                typing_defs_flags::FT_FLAGS_ASYNC | typing_defs_flags::FT_FLAGS_GENERATOR
            }
            FunKind::FCoroutine => typing_defs_flags::FT_FLAGS_IS_COROUTINE,
        };
        if attributes.returns_mutable {
            flags |= typing_defs_flags::FT_FLAGS_RETURNS_MUTABLE;
        }
        let ft = FunType {
            arity,
            tparams: type_params,
            where_constraints: Vec::new(),
            params,
            ret: PossiblyEnforcedTy {
                enforced: false,
                type_,
            },
            reactive: attributes.reactivity,
            flags,
        };

        let ty = Ty(Reason::Rwitness(id.0.clone()), Box::new(Ty_::Tfun(ft)));
        Ok(Box::new((id, ty, properties)))
    }

    fn prefix_ns<'a>(&self, name: Cow<'a, String>) -> Cow<'a, String> {
        if name.starts_with("\\") {
            name
        } else if self.state.namespace_builder.current_namespace().is_empty() {
            Cow::Owned("\\".to_owned() + &name)
        } else {
            Cow::Owned(
                "\\".to_owned() + self.state.namespace_builder.current_namespace() + "\\" + &name,
            )
        }
    }

    fn into_variables_list(
        &self,
        list: Node_,
        type_variables: &HashSet<Rc<String>>,
    ) -> Result<(FunParams, Vec<PropertyDecl>, FunArity), ParseError> {
        match list {
            Node_::List(nodes) => nodes.into_iter().fold(
                Ok((Vec::new(), Vec::new(), FunArity::Fstandard(0))),
                |acc, variable| match (acc, variable) {
                    (Ok((mut variables, mut properties, arity)), Node_::Variable(innards)) => {
                        let VariableDecl {
                            attributes,
                            visibility,
                            kind,
                            hint,
                            id,
                            variadic,
                            initializer,
                        } = *innards;
                        match visibility.as_visibility() {
                            Ok(_) => properties.push(PropertyDecl {
                                attrs: attributes.clone(),
                                modifiers: visibility.clone(),
                                hint: hint.clone(),
                                id: id.clone(),
                                expr: None,
                            }),
                            Err(_) => (),
                        };

                        let attributes = attributes.as_attributes()?;
                        let type_ = match &hint {
                            Node_::Ignored => tany(),
                            _ => self.node_to_ty(&hint, type_variables).map(|mut ty| {
                                match &mut *ty.1 {
                                    Ty_::Tfun(fun_type) if attributes.at_most_rx_as_func => {
                                        fun_type.reactive = Reactivity::RxVar(None);
                                    }
                                    _ => (),
                                }
                                ty
                            })?,
                        };
                        let param = FunParam {
                            pos: id.0,
                            name: Some(id.1),
                            type_: PossiblyEnforcedTy {
                                enforced: false,
                                type_,
                            },
                            kind,
                            accept_disposable: false,
                            mutability: attributes.param_mutability,
                            rx_annotation: None,
                        };
                        let arity = match (arity, initializer, variadic) {
                            (FunArity::Fstandard(min), Node_::Ignored, false) => {
                                variables.push(param);
                                FunArity::Fstandard(min + 1)
                            }
                            (FunArity::Fstandard(min), Node_::Ignored, true) => {
                                FunArity::Fvariadic(min, param)
                            }
                            (FunArity::Fstandard(min), _, _) => {
                                variables.push(param);
                                FunArity::Fstandard(min)
                            }
                            (arity, _, _) => {
                                variables.push(param);
                                arity
                            }
                        };
                        Ok((variables, properties, arity))
                    }
                    (Ok(_), n) => Err(format!("Expected a variable, but got {:?}", n)),
                    (acc @ Err(_), _) => acc,
                },
            ),
            Node_::Ignored => Ok((Vec::new(), Vec::new(), FunArity::Fstandard(0))),
            n => Err(format!("Expected a list of variables, but got {:?}", n)),
        }
    }

    fn make_apply(
        &self,
        base_ty: Node_,
        type_variables: Node_,
        closing_delimiter: Option<Node_>,
    ) -> Result<Node_, ParseError> {
        let Id(base_ty_pos, base_ty_name) = get_name("", &base_ty)?;
        let base_ty_name = prefix_slash(Cow::Owned(base_ty_name)).into_owned();
        let pos = match closing_delimiter {
            Some(closing_delimiter) => Pos::merge(&base_ty_pos, &closing_delimiter.get_pos()?)?,
            None => base_ty_pos.clone(),
        };
        Ok(Node_::Hint(
            HintValue::Apply(Box::new((
                Id(base_ty_pos, base_ty_name),
                type_variables.into_vec(),
            ))),
            pos,
        ))
    }
}

enum NodeIterHelper<'a> {
    Empty,
    Single(&'a Node_),
    Vec(std::slice::Iter<'a, Node_>),
}

impl<'a> Iterator for NodeIterHelper<'a> {
    type Item = &'a Node_;

    fn next(&mut self) -> Option<Self::Item> {
        match self {
            NodeIterHelper::Empty => None,
            NodeIterHelper::Single(node) => {
                let node = *node;
                *self = NodeIterHelper::Empty;
                Some(node)
            }
            NodeIterHelper::Vec(ref mut iter) => iter.next(),
        }
    }
}

impl<'a> FlattenOp for DirectDeclSmartConstructors<'_> {
    type S = Node;

    fn flatten(lst: Vec<Self::S>) -> Self::S {
        let mut r = lst
            .into_iter()
            .map(|s| match s {
                Ok(Node_::List(children)) => children.into_iter().map(|x| Ok(x)).collect(),
                x => {
                    if Self::is_zero(&x) {
                        vec![]
                    } else {
                        vec![x]
                    }
                }
            })
            .flatten()
            .collect::<Result<Vec<_>, ParseError>>()?;
        Ok(match r.as_slice() {
            [] => Node_::Ignored,
            [_] => r.pop().unwrap(),
            _ => Node_::List(r),
        })
    }

    fn zero() -> Self::S {
        Ok(Node_::Ignored)
    }

    fn is_zero(s: &Self::S) -> bool {
        fn inner_is_zero(n: &Node_) -> bool {
            match n {
                Node_::Yield => false,
                Node_::List(inner) => inner.iter().all(inner_is_zero),
                _ => true,
            }
        }

        match s {
            Err(_) => false,
            Ok(n) => inner_is_zero(n),
        }
    }
}

impl<'a> FlattenSmartConstructors<'a, State<'a>> for DirectDeclSmartConstructors<'a> {
    fn make_token(&mut self, token: Self::Token) -> Self::R {
        let token_text = |this: &Self| {
            String::from_utf8_lossy(this.state.source_text.source_text().sub(
                token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                token.width(),
            ))
            .to_string()
        };
        let token_pos = |this: &Self| {
            this.state
                .source_text
                .relative_pos(token.start_offset(), token.end_offset() + 1)
        };
        let kind = token.kind();

        // We only want to check the mode if <? is the very first token we see.
        match (&self.state.file_mode_builder, &kind) {
            (FileModeBuilder::None, TokenKind::Markup) => (),
            (FileModeBuilder::None, TokenKind::LessThanQuestion) => {
                self.state.file_mode_builder = FileModeBuilder::Pending
            }
            (FileModeBuilder::Pending, TokenKind::Name) if token_text(self) == "hh" => {
                self.set_mode(&token);
            }
            (FileModeBuilder::None, _) | (FileModeBuilder::Pending, _) => {
                self.state.file_mode_builder = FileModeBuilder::Set(Mode::Mstrict)
            }
            (_, _) => (),
        }

        // So... this part gets pretty disgusting. We're setting a lot of parser
        // state in here, and we're unsetting a lot of it down in the actual
        // make_namespace_XXXX methods. I tried a few other ways of handling
        // namespaces, but ultimately rejected them.
        //
        // The reason we even need this code at all is because we insert entries
        // into our state.decls struct as soon as we see them, but because the
        // FFP calls its make_XXXX functions in inside out order (which is the
        // most reasonable thing for a parser to do), we don't actually know
        // which namespace we're in until we leave it, at which point we've
        // already inserted all of the decls from inside that namespace.
        //
        // One idea I attempted was having both the state.decls struct as well
        // as a new state.in_progress_decls struct, and if we hit a
        // make_namespace_XXXX method, we would rename every decl within that
        // struct to have the correct namespace, then merge
        // state.in_progress_decls into state.decls. The problem that I ran into
        // with this approach had to do with the insides of the decls. While
        // it's easy enough to rename every decl's name, it's much harder to dig
        // into each decl and rename every single Tapply, which we have to do if
        // a decl references a type within the same namespace.
        //
        // The facts parser, meanwhile, solves this problem by keeping track of
        // a micro-AST, and only inserting names into the returned facts struct
        // at the very end. This would be convenient for us, but I have concerns
        // about the future-proofing of ensuring that 100% of all decls bubble
        // up properly in the face of changing code and behavior.
        //
        // So we can't allocate sub-trees for each namespace, and we can't just
        // wait until we hit make_namespace_XXXX then rewrite all the names, so
        // what can we do? The answer is that we can encode a really gross
        // namespace-tracking state machine inside of our parser, and take
        // advantage of some mildly unspoken quasi-implementation details. We
        // always call the make_XXXX methods after we finish the full parsing of
        // whatever we're making... but since make_token() only depends on a
        // single token, it gets called *before* we enter the namespace!
        //
        // The general idea, then, is that when we see a namespace token we know
        // that we're about to parse a namespace name, so we enter a state where
        // we know we're building up a namespace name (as opposed to the many,
        // many other kinds of names). While we're in that state, every name and
        // backslash token gets appended to the in-progress namespace name. Once
        // we see either an opening curly brace or a semicolon, we know that
        // we've finished building the namespace name, so we exit the "building
        // a namespace" state and push the newly created namespace onto our
        // namespace stack. Then, in all of the various make_namespace_XXXX
        // methods, we pop the namespace (since we know we've just exited it).
        let result = Ok(match kind {
            TokenKind::Name => {
                let name = token_text(self);
                let pos = token_pos(self);
                if self.state.previous_token_kind == TokenKind::Class
                    || self.state.previous_token_kind == TokenKind::Trait
                    || self.state.previous_token_kind == TokenKind::Interface
                {
                    self.state
                        .classish_name_builder
                        .lexed_name_after_classish_keyword(&name, &pos);
                }
                if self.state.namespace_builder.is_building_namespace {
                    Rc::make_mut(&mut self.state.namespace_builder)
                        .in_progress_namespace
                        .push_str(&name.to_string());
                    Node_::Ignored
                } else {
                    Node_::Name(name, pos)
                }
            }
            TokenKind::Class => Node_::Name(token_text(self), token_pos(self)),
            // There are a few types whose string representations we have to
            // grab anyway, so just go ahead and treat them as generic names.
            TokenKind::Variable
            | TokenKind::Vec
            | TokenKind::Dict
            | TokenKind::Keyset
            | TokenKind::Tuple
            | TokenKind::Classname
            | TokenKind::SelfToken => Node_::Name(token_text(self), token_pos(self)),
            TokenKind::XHPClassName => Node_::XhpName(token_text(self), token_pos(self)),
            TokenKind::SingleQuotedStringLiteral => Node_::StringLiteral(
                token_text(self)
                    .trim_start_matches("'")
                    .trim_end_matches("'")
                    .to_string(),
                token_pos(self),
            ),
            TokenKind::DoubleQuotedStringLiteral => Node_::StringLiteral(
                token_text(self)
                    .trim_start_matches('"')
                    .trim_end_matches('"')
                    .to_string(),
                token_pos(self),
            ),
            TokenKind::DecimalLiteral
            | TokenKind::OctalLiteral
            | TokenKind::HexadecimalLiteral
            | TokenKind::BinaryLiteral => Node_::IntLiteral(token_text(self), token_pos(self)),
            TokenKind::FloatingLiteral => Node_::FloatingLiteral(token_text(self), token_pos(self)),
            TokenKind::NullLiteral => Node_::Null(token_pos(self)),
            TokenKind::BooleanLiteral => Node_::BooleanLiteral(token_text(self), token_pos(self)),
            TokenKind::String => Node_::Hint(HintValue::String, token_pos(self)),
            TokenKind::Int => Node_::Hint(HintValue::Int, token_pos(self)),
            TokenKind::Float => Node_::Hint(HintValue::Float, token_pos(self)),
            TokenKind::Double => Node_::Hint(
                HintValue::Apply(Box::new((
                    Id(token_pos(self), token_text(self)),
                    Vec::new(),
                ))),
                token_pos(self),
            ),
            TokenKind::Num => Node_::Hint(HintValue::Num, token_pos(self)),
            TokenKind::Bool => Node_::Hint(HintValue::Bool, token_pos(self)),
            TokenKind::Boolean => Node_::Hint(
                HintValue::Apply(Box::new((
                    Id(token_pos(self), token_text(self)),
                    Vec::new(),
                ))),
                token_pos(self),
            ),
            TokenKind::Mixed => Node_::Hint(HintValue::Mixed, token_pos(self)),
            TokenKind::Void => Node_::Hint(HintValue::Void, token_pos(self)),
            TokenKind::Arraykey => Node_::Hint(HintValue::ArrayKey, token_pos(self)),
            TokenKind::Noreturn => Node_::Hint(HintValue::NoReturn, token_pos(self)),
            TokenKind::Resource => Node_::Hint(HintValue::Resource, token_pos(self)),
            TokenKind::Array => Node_::Array(token_pos(self)),
            TokenKind::Darray => Node_::Darray(token_pos(self)),
            TokenKind::Varray => Node_::Varray(token_pos(self)),
            TokenKind::Backslash => {
                if self.state.namespace_builder.is_building_namespace {
                    Rc::make_mut(&mut self.state.namespace_builder)
                        .in_progress_namespace
                        .push('\\');
                    Node_::Ignored
                } else {
                    Node_::Backslash(token_pos(self))
                }
            }
            TokenKind::Construct => Node_::Construct(token_pos(self)),
            TokenKind::LeftParen => Node_::LeftParen(token_pos(self)),
            TokenKind::RightParen | TokenKind::RightBracket => {
                // We don't technically need to differentiate these.
                Node_::RightParen(token_pos(self))
            }
            TokenKind::Shape => Node_::Shape(token_pos(self)),
            TokenKind::Question => Node_::Question(token_pos(self)),
            TokenKind::This => Node_::This(token_pos(self)),
            TokenKind::ColonColon => Node_::ColonColon(token_pos(self)),
            TokenKind::Tilde => Node_::Operator(token_pos(self), OperatorType::Tilde),
            TokenKind::Exclamation => Node_::Operator(token_pos(self), OperatorType::Not),
            TokenKind::Plus => Node_::Operator(token_pos(self), OperatorType::Plus),
            TokenKind::Minus => Node_::Operator(token_pos(self), OperatorType::Minus),
            TokenKind::PlusPlus => Node_::Operator(token_pos(self), OperatorType::PlusPlus),
            TokenKind::MinusMinus => Node_::Operator(token_pos(self), OperatorType::MinusMinus),
            TokenKind::At => Node_::Operator(token_pos(self), OperatorType::Silence),
            TokenKind::Star => Node_::Operator(token_pos(self), OperatorType::Star),
            TokenKind::Slash => Node_::Operator(token_pos(self), OperatorType::Slash),
            TokenKind::EqualEqual => Node_::Operator(token_pos(self), OperatorType::Eqeq),
            TokenKind::EqualEqualEqual => Node_::Operator(token_pos(self), OperatorType::Eqeqeq),
            TokenKind::StarStar => Node_::Operator(token_pos(self), OperatorType::Starstar),
            TokenKind::AmpersandAmpersand => Node_::Operator(token_pos(self), OperatorType::Ampamp),
            TokenKind::BarBar => Node_::Operator(token_pos(self), OperatorType::Barbar),
            TokenKind::LessThan => Node_::Operator(token_pos(self), OperatorType::Lt),
            TokenKind::LessThanEqual => Node_::Operator(token_pos(self), OperatorType::Lte),
            TokenKind::GreaterThan => Node_::Operator(token_pos(self), OperatorType::Gt),
            TokenKind::GreaterThanEqual => Node_::Operator(token_pos(self), OperatorType::Gte),
            TokenKind::Dot => Node_::Operator(token_pos(self), OperatorType::Dot),
            TokenKind::Ampersand => Node_::Operator(token_pos(self), OperatorType::Amp),
            TokenKind::Bar => Node_::Operator(token_pos(self), OperatorType::Bar),
            TokenKind::LessThanLessThan => Node_::LessThanLessThan(token_pos(self)),
            TokenKind::GreaterThanGreaterThan => Node_::GreaterThanGreaterThan(token_pos(self)),
            TokenKind::Percent => Node_::Operator(token_pos(self), OperatorType::Percent),
            TokenKind::QuestionQuestion => {
                Node_::Operator(token_pos(self), OperatorType::QuestionQuestion)
            }
            TokenKind::Abstract => Node_::Abstract,
            TokenKind::As => Node_::As,
            TokenKind::Super => Node_::Super,
            TokenKind::Async => Node_::Async,
            TokenKind::Coroutine => Node_::Coroutine,
            TokenKind::DotDotDot => Node_::DotDotDot,
            TokenKind::Extends => Node_::Extends,
            TokenKind::Final => Node_::Final,
            TokenKind::Implements => Node_::Implements,
            TokenKind::Inout => Node_::Inout,
            TokenKind::Interface => Node_::Interface,
            TokenKind::XHP => Node_::XHP,
            TokenKind::Yield => Node_::Yield,
            TokenKind::Namespace => {
                Rc::make_mut(&mut self.state.namespace_builder).is_building_namespace = true;
                if !self.state.namespace_builder.current_namespace().is_empty() {
                    Rc::make_mut(&mut self.state.namespace_builder)
                        .in_progress_namespace
                        .push('\\')
                }
                Node_::Ignored
            }
            TokenKind::LeftBrace | TokenKind::Semicolon => {
                if self.state.namespace_builder.is_building_namespace {
                    if kind == TokenKind::LeftBrace {
                        Rc::make_mut(&mut self.state.namespace_builder).push_namespace();
                    } else {
                        Rc::make_mut(&mut self.state.namespace_builder).set_namespace();
                    }
                    Rc::make_mut(&mut self.state.namespace_builder).is_building_namespace = false;
                }
                // It's not necessary to track left braces, so just always
                // return a semicolon.
                Node_::Semicolon
            }
            TokenKind::Private => Node_::Private,
            TokenKind::Protected => Node_::Protected,
            TokenKind::Public => Node_::Public,
            TokenKind::Reify => Node_::Reify,
            TokenKind::Static => Node_::Static,
            TokenKind::Trait => Node_::Trait,
            _ => Node_::Ignored,
        });
        self.state.previous_token_kind = kind;
        result
    }

    fn make_missing(&mut self, _: usize) -> Self::R {
        Ok(Node_::Ignored)
    }

    fn make_list(&mut self, items: Vec<Self::R>, _: usize) -> Self::R {
        if items.iter().any(|node| matches!(node, Ok(Node_::Yield))) {
            Ok(Node_::Yield)
        } else {
            let items = items
                .into_iter()
                .filter(|node| !(matches!(node, Ok(Node_::Ignored))))
                .collect::<Result<Vec<_>, ParseError>>()?;
            if items.is_empty() {
                Ok(Node_::Ignored)
            } else {
                Ok(Node_::List(items))
            }
        }
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        let arg0 = arg0?;
        let pos = arg0.get_pos();
        Ok(match arg0 {
            Node_::Ignored => Node_::Ignored,
            Node_::List(nodes) => Node_::QualifiedName(nodes, pos?),
            node => Node_::QualifiedName(vec![node], pos?),
        })
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::R) -> Self::R {
        // Return this explicitly because flatten filters out zero nodes, and
        // we treat most non-error nodes as zeroes.
        arg0
    }

    fn make_literal_expression(&mut self, arg0: Self::R) -> Self::R {
        arg0
    }

    fn make_simple_initializer(&mut self, _arg0: Self::R, expr: Self::R) -> Self::R {
        expr
    }

    fn make_array_intrinsic_expression(
        &mut self,
        array: Self::R,
        _arg1: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| match node {
                Node_::ListItem(innards) => Ok(aast::Afield::AFkvalue(
                    innards.0.as_expr()?,
                    innards.1.as_expr()?,
                )),
                node => Ok(aast::Afield::AFvalue(node.as_expr()?)),
            })
            .collect::<Result<Vec<_>, ParseError>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&array?.get_pos()?, &right_paren?.get_pos()?)?,
            nast::Expr_::Array(fields),
        ))))
    }

    fn make_darray_intrinsic_expression(
        &mut self,
        darray: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| match node {
                Node_::ListItem(innards) => {
                    let (key, value) = *innards;
                    let key = key.as_expr()?;
                    let value = value.as_expr()?;
                    Ok((key, value))
                }
                n => Err(format!("Expected a ListItem but was {:?}", n)),
            })
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&darray?.get_pos()?, &right_bracket?.get_pos()?)?,
            nast::Expr_::Darray(Box::new((None, fields))),
        ))))
    }

    fn make_dictionary_intrinsic_expression(
        &mut self,
        dict: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| match node {
                Node_::ListItem(innards) => {
                    let (key, value) = *innards;
                    let key = key.as_expr()?;
                    let value = value.as_expr()?;
                    Ok(aast::Field(key, value))
                }
                n => Err(format!("Expected a ListItem but was {:?}", n)),
            })
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&dict?.get_pos()?, &right_bracket?.get_pos()?)?,
            nast::Expr_::KeyValCollection(Box::new((aast_defs::KvcKind::Dict, None, fields))),
        ))))
    }

    fn make_keyset_intrinsic_expression(
        &mut self,
        keyset: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| node.as_expr())
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&keyset?.get_pos()?, &right_bracket?.get_pos()?)?,
            nast::Expr_::ValCollection(Box::new((aast_defs::VcKind::Keyset, None, fields))),
        ))))
    }

    fn make_varray_intrinsic_expression(
        &mut self,
        varray: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| node.as_expr())
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&varray?.get_pos()?, &right_bracket?.get_pos()?)?,
            nast::Expr_::Varray(Box::new((None, fields))),
        ))))
    }

    fn make_vector_intrinsic_expression(
        &mut self,
        vec: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| node.as_expr())
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&vec?.get_pos()?, &right_bracket?.get_pos()?)?,
            nast::Expr_::ValCollection(Box::new((aast_defs::VcKind::Vec, None, fields))),
        ))))
    }

    fn make_element_initializer(
        &mut self,
        key: Self::R,
        _arg1: Self::R,
        value: Self::R,
    ) -> Self::R {
        Ok(Node_::ListItem(Box::new((key?, value?))))
    }

    fn make_prefix_unary_expression(&mut self, op: Self::R, value: Self::R) -> Self::R {
        let (op, value) = (op?, value?);
        let pos = match (op.get_pos(), value.get_pos()) {
            (Ok(op_pos), Ok(value_pos)) => Pos::merge(&op_pos, &value_pos)?,
            _ => return Ok(Node_::Ignored),
        };
        let op = match &op {
            Node_::Operator(_, op) => match op {
                OperatorType::Tilde => Uop::Utild,
                OperatorType::Not => Uop::Unot,
                OperatorType::Plus => Uop::Uplus,
                OperatorType::Minus => Uop::Uminus,
                OperatorType::PlusPlus => Uop::Uincr,
                OperatorType::MinusMinus => Uop::Udecr,
                OperatorType::Silence => Uop::Usilence,
                op => {
                    return Err(format!(
                        "Operator {:?} cannot be used as a unary operator",
                        op
                    ))
                }
            },
            op => return Err(format!("Did not recognize operator {:?}", op)),
        };
        Ok(Node_::Expr(Box::new(aast::Expr(
            pos,
            aast::Expr_::Unop(Box::new((op, value.as_expr()?))),
        ))))
    }

    fn make_postfix_unary_expression(&mut self, value: Self::R, op: Self::R) -> Self::R {
        let (value, op) = (value?, op?);
        let pos = match (value.get_pos(), op.get_pos()) {
            (Ok(value_pos), Ok(op_pos)) => Pos::merge(&value_pos, &op_pos)?,
            _ => return Ok(Node_::Ignored),
        };
        let op = match &op {
            Node_::Operator(_, op) => match op {
                OperatorType::PlusPlus => Uop::Upincr,
                OperatorType::MinusMinus => Uop::Updecr,
                op => {
                    return Err(format!(
                        "Operator {:?} cannot be used as a postfix unary operator",
                        op
                    ))
                }
            },
            op => return Err(format!("Did not recognize operator {:?}", op)),
        };
        Ok(Node_::Expr(Box::new(aast::Expr(
            pos,
            aast::Expr_::Unop(Box::new((op, value.as_expr()?))),
        ))))
    }

    fn make_binary_expression(&mut self, lhs: Self::R, op: Self::R, rhs: Self::R) -> Self::R {
        let (lhs, op, rhs) = (lhs?, op?, rhs?);
        let pos = match (lhs.get_pos(), op.get_pos(), rhs.get_pos()) {
            (Ok(lhs_pos), Ok(op_pos), Ok(rhs_pos)) => {
                Pos::merge(&Pos::merge(&lhs_pos, &op_pos)?, &rhs_pos)?
            }
            _ => return Ok(Node_::Ignored),
        };

        let op = match &op {
            Node_::Operator(_, op) => match op {
                OperatorType::Plus => Bop::Plus,
                OperatorType::Minus => Bop::Minus,
                OperatorType::Star => Bop::Star,
                OperatorType::Slash => Bop::Slash,
                OperatorType::Eqeq => Bop::Eqeq,
                OperatorType::Eqeqeq => Bop::Eqeqeq,
                OperatorType::Starstar => Bop::Starstar,
                OperatorType::Ampamp => Bop::Ampamp,
                OperatorType::Barbar => Bop::Barbar,
                OperatorType::LogXor => Bop::LogXor,
                OperatorType::Lt => Bop::Lt,
                OperatorType::Lte => Bop::Lte,
                OperatorType::Gt => Bop::Gt,
                OperatorType::Gte => Bop::Gte,
                OperatorType::Dot => Bop::Dot,
                OperatorType::Amp => Bop::Amp,
                OperatorType::Bar => Bop::Bar,
                OperatorType::Percent => Bop::Percent,
                OperatorType::Xor => Bop::Xor,
                OperatorType::Cmp => Bop::Cmp,
                OperatorType::QuestionQuestion => Bop::QuestionQuestion,
                op => {
                    return Err(format!(
                        "Operator {:?} cannot be used as a binary operator",
                        op
                    ))
                }
            },
            Node_::LessThanLessThan(_) => Bop::Ltlt,
            Node_::GreaterThanGreaterThan(_) => Bop::Gtgt,
            op => return Err(format!("Did not recognize operator {:?}", op)),
        };

        Ok(Node_::Expr(Box::new(aast::Expr(
            pos,
            aast::Expr_::Binop(Box::new((op, lhs.as_expr()?, rhs.as_expr()?))),
        ))))
    }

    fn make_list_item(&mut self, item: Self::R, sep: Self::R) -> Self::R {
        Ok(match (item?, sep?) {
            (Node_::Ignored, Node_::Ignored) => Node_::Ignored,
            (x, Node_::Ignored) | (Node_::Ignored, x) => x,
            (x, y) => Node_::ListItem(Box::new((x, y))),
        })
    }

    fn make_type_arguments(
        &mut self,
        less_than: Self::R,
        arguments: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::BracketedList(Box::new((
            less_than?.get_pos()?,
            arguments?.into_vec(),
            greater_than?.get_pos()?,
        ))))
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::R,
        argument_list: Self::R,
    ) -> Self::R {
        let (class_type, argument_list) = (class_type?, argument_list?);
        let Id(pos, class_type) = get_name(
            self.state.namespace_builder.current_namespace(),
            &class_type,
        )?;
        let full_pos = match argument_list.get_pos() {
            Ok(p2) => Pos::merge(&pos, &p2)?,
            Err(_) => pos.clone(),
        };
        Ok(Node_::Hint(
            HintValue::Apply(Box::new((
                Id(pos, "\\".to_string() + &class_type),
                argument_list.into_vec(),
            ))),
            full_pos,
        ))
    }

    fn make_alias_declaration(
        &mut self,
        _attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        generic_params: Self::R,
        _constraint: Self::R,
        _equal: Self::R,
        aliased_type: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let (name, aliased_type) = (name?, aliased_type?);
        match name {
            Node_::Ignored => (),
            _ => {
                let Id(pos, name) =
                    get_name(self.state.namespace_builder.current_namespace(), &name)?;
                let (tparams, type_variables) = self.into_type_params(generic_params?)?;
                match self.node_to_ty(&aliased_type, &type_variables) {
                    Ok(ty) => {
                        Rc::make_mut(&mut self.state.decls).typedefs.insert(
                            name,
                            Rc::new(TypedefType {
                                pos,
                                vis: aast::TypedefVisibility::Transparent,
                                tparams,
                                constraint: None,
                                type_: ty,
                                // NB: We have no intention of populating this
                                // field. Any errors historically emitted during
                                // shallow decl should be migrated to a NAST
                                // check.
                                decl_errors: Some(Errors::empty()),
                            }),
                        );
                    }
                    Err(msg) => return Err(msg),
                }
            }
        };
        Ok(Node_::Ignored)
    }

    fn make_type_constraint(&mut self, kind: Self::R, value: Self::R) -> Self::R {
        let kind = match kind? {
            Node_::As => ConstraintKind::ConstraintAs,
            Node_::Super => ConstraintKind::ConstraintSuper,
            n => return Err(format!("Expected either As or Super, but was {:?}", n)),
        };
        Ok(Node_::TypeConstraint(Box::new((kind, value?))))
    }

    fn make_type_parameter(
        &mut self,
        _arg0: Self::R,
        reify: Self::R,
        variance: Self::R,
        name: Self::R,
        constraints: Self::R,
    ) -> Self::R {
        let constraints = constraints?.into_iter().fold(
            Ok(Vec::new()),
            |acc: Result<Vec<_>, ParseError>, node| match acc {
                acc @ Err(_) => acc,
                Ok(mut acc) => match node {
                    Node_::TypeConstraint(innards) => {
                        acc.push(innards);
                        Ok(acc)
                    }
                    Node_::Ignored => Ok(acc),
                    n => Err(format!("Expected a type constraint, but was {:?}", n)),
                },
            },
        )?;
        Ok(Node_::TypeParameter(Box::new(TypeParameterDecl {
            name: name?,
            variance: match variance? {
                Node_::Operator(_, OperatorType::Minus) => Variance::Contravariant,
                Node_::Operator(_, OperatorType::Plus) => Variance::Covariant,
                _ => Variance::Invariant,
            },
            reified: match reify? {
                Node_::Reify => aast::ReifyKind::Reified,
                _ => aast::ReifyKind::Erased,
            },
            constraints,
        })))
    }

    fn make_type_parameters(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        Ok(Node_::BracketedList(Box::new((
            arg0?.get_pos()?,
            arg1?.into_vec(),
            arg2?.get_pos()?,
        ))))
    }

    fn make_parameter_declaration(
        &mut self,
        attributes: Self::R,
        visibility: Self::R,
        inout: Self::R,
        hint: Self::R,
        name: Self::R,
        initializer: Self::R,
    ) -> Self::R {
        let (variadic, id) = match name? {
            Node_::ListItem(innards) => {
                let id = get_name("", &innards.1)?;
                match innards.0 {
                    Node_::DotDotDot => (true, id),
                    _ => (false, id),
                }
            }
            name => (false, get_name("", &name)?),
        };
        let attributes = attributes?;
        let visibility = visibility?;
        let initializer = initializer?;
        let kind = match inout? {
            Node_::Inout => ParamMode::FPinout,
            _ => ParamMode::FPnormal,
        };
        let hint = hint?;
        Ok(Node_::Variable(Box::new(VariableDecl {
            attributes,
            visibility,
            kind,
            hint,
            id,
            variadic,
            initializer,
        })))
    }

    fn make_function_declaration(
        &mut self,
        attributes: Self::R,
        header: Self::R,
        body: Self::R,
    ) -> Self::R {
        // The only contents we care about from inside method bodies is whether
        // we saw a yield, so just throw away errors here.
        let body = match body {
            Ok(body) => body,
            Err(_) => Node_::Ignored,
        };
        let attributes = attributes?;
        let parsed_attributes = attributes.as_attributes()?;
        Ok(match header? {
            Node_::FunctionHeader(decl) => {
                let (Id(pos, name), type_, _) = *self.function_into_ty(
                    self.state.namespace_builder.current_namespace(),
                    attributes,
                    *decl,
                    body,
                    &HashSet::new(),
                )?;
                let deprecated = parsed_attributes
                    .deprecated
                    .map(|msg| format!("The function {} is deprecated: {}", name, msg));
                Rc::make_mut(&mut self.state.decls).funs.insert(
                    name,
                    Rc::new(FunElt {
                        deprecated,
                        type_,
                        // NB: We have no intention of populating this field.
                        // Any errors historically emitted during shallow decl
                        // should be migrated to a NAST check.
                        decl_errors: Some(Errors::empty()),
                        pos,
                    }),
                );
                Node_::Ignored
            }
            _ => Node_::Ignored,
        })
    }

    fn make_function_declaration_header(
        &mut self,
        modifiers: Self::R,
        _keyword: Self::R,
        name: Self::R,
        type_params: Self::R,
        _left_parens: Self::R,
        param_list: Self::R,
        _right_parens: Self::R,
        _colon: Self::R,
        ret_hint: Self::R,
        _where: Self::R,
    ) -> Self::R {
        Ok(match name? {
            Node_::Ignored => Node_::Ignored,
            name => Node_::FunctionHeader(Box::new(FunctionHeader {
                name,
                modifiers: modifiers?,
                type_params: type_params?,
                param_list: param_list?,
                ret_hint: ret_hint?,
            })),
        })
    }

    fn make_yield_expression(&mut self, _arg0: Self::R, _arg1: Self::R) -> Self::R {
        Ok(Node_::Yield)
    }

    fn make_yield_from_expression(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
    ) -> Self::R {
        Ok(Node_::Yield)
    }

    fn make_const_declaration(
        &mut self,
        modifiers: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        decls: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // None of the Node_::Ignoreds should happen in a well-formed file, but
        // they could happen in a malformed one. We also bubble up the const
        // declaration instead of inserting it immediately because consts can
        // appear in classes or directly in namespaces.
        let hint = hint?;
        Ok(match decls? {
            Node_::List(nodes) => match nodes.as_slice() {
                [Node_::List(nodes)] => match nodes.as_slice() {
                    [name, initializer] => {
                        let id = get_name(self.state.namespace_builder.current_namespace(), name)?;
                        let modifiers = modifiers?;
                        let ty = self
                            .node_to_ty(&hint, &HashSet::new())
                            .or_else(|_| self.node_to_ty(&initializer, &HashSet::new()))
                            .unwrap_or_else(|_| tany());
                        Node_::Const(Box::new(ConstDecl {
                            modifiers,
                            id,
                            ty,
                            expr: match initializer {
                                Node_::Expr(e) => Some(e.clone()),
                                n => n.as_expr().ok().map(Box::new),
                            },
                        }))
                    }
                    _ => Node_::Ignored,
                },
                _ => Node_::Ignored,
            },
            _ => Node_::Ignored,
        })
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        let (name, initializer) = (name?, initializer?);
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => Node_::List(vec![name, initializer]),
        })
    }

    fn make_namespace_declaration(
        &mut self,
        _keyword: Self::R,
        _name: Self::R,
        _body: Self::R,
    ) -> Self::R {
        Rc::make_mut(&mut self.state.namespace_builder).pop_namespace();
        Ok(Node_::Ignored)
    }

    fn make_namespace_body(&mut self, _arg0: Self::R, body: Self::R, _arg2: Self::R) -> Self::R {
        for item in body?.into_iter() {
            match item {
                Node_::Const(decl) => {
                    Rc::make_mut(&mut self.state.decls)
                        .consts
                        .insert(decl.id.1, Rc::new(decl.ty));
                }
                _ => (),
            }
        }
        Ok(Node_::Ignored)
    }

    fn make_script(&mut self, body: Self::R) -> Self::R {
        for item in body?.into_iter() {
            match item {
                Node_::Const(decl) => {
                    Rc::make_mut(&mut self.state.decls)
                        .consts
                        .insert(decl.id.1, Rc::new(decl.ty));
                }
                _ => (),
            };
        }
        Ok(Node_::Ignored)
    }

    fn make_namespace_use_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        imports: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        for import in imports?.into_iter() {
            if let Node_::NamespaceUseClause(nuc) = import {
                Rc::make_mut(&mut self.state.namespace_builder).add_import(nuc.id.1, nuc.as_);
            }
        }
        Ok(Node_::Ignored)
    }

    fn make_namespace_group_use_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        prefix: Self::R,
        _arg3: Self::R,
        imports: Self::R,
        _arg5: Self::R,
        _arg6: Self::R,
    ) -> Self::R {
        let Id(_, prefix) = get_name("", &prefix?)?;
        for import in imports?.into_iter() {
            if let Node_::NamespaceUseClause(nuc) = import {
                Rc::make_mut(&mut self.state.namespace_builder)
                    .add_import(prefix.clone() + &nuc.id.1, nuc.as_);
            }
        }
        Ok(Node_::Ignored)
    }

    fn make_namespace_use_clause(
        &mut self,
        _arg0: Self::R,
        name: Self::R,
        as_: Self::R,
        aliased_name: Self::R,
    ) -> Self::R {
        let id = get_name("", &name?)?;
        let as_ = if let Node_::As = as_? {
            Some(get_name("", &aliased_name?)?.1)
        } else {
            None
        };
        Ok(Node_::NamespaceUseClause(Box::new(NamespaceUseClause {
            id,
            as_,
        })))
    }

    fn make_classish_declaration(
        &mut self,
        attributes: Self::R,
        modifiers: Self::R,
        xhp_keyword: Self::R,
        class_keyword: Self::R,
        name: Self::R,
        tparams: Self::R,
        _arg5: Self::R,
        extends: Self::R,
        _arg7: Self::R,
        implements: Self::R,
        _arg9: Self::R,
        body: Self::R,
    ) -> Self::R {
        #[derive(Debug)]
        struct Modifiers {
            is_static: bool,
            visibility: aast::Visibility,
            is_abstract: bool,
            is_final: bool,
        }
        fn read_member_modifiers<'a>(modifiers: impl Iterator<Item = &'a Node_>) -> Modifiers {
            let mut ret = Modifiers {
                is_static: false,
                visibility: aast::Visibility::Private,
                is_abstract: false,
                is_final: false,
            };
            for modifier in modifiers {
                if let Ok(vis) = modifier.as_visibility() {
                    ret.visibility = vis;
                }
                match modifier {
                    Node_::Static => ret.is_static = true,
                    Node_::Abstract => ret.is_abstract = true,
                    Node_::Final => ret.is_final = true,
                    _ => (),
                }
            }
            ret
        }

        let Id(pos, name) = get_name(self.state.namespace_builder.current_namespace(), &name?)?;
        let key = name.clone();
        let name = prefix_slash(Cow::Owned(name));
        let (type_params, type_variables) = self.into_type_params(tparams?)?;
        let class_kind = match class_keyword? {
            Node_::Interface => ClassKind::Cinterface,
            Node_::Trait => ClassKind::Ctrait,
            _ => ClassKind::Cnormal,
        };
        let mut cls = shallow_decl_defs::ShallowClass {
            mode: match self.state.file_mode_builder {
                FileModeBuilder::None | FileModeBuilder::Pending => Mode::Mstrict,
                FileModeBuilder::Set(mode) => mode,
            },
            final_: false,
            is_xhp: false,
            has_xhp_keyword: match xhp_keyword? {
                Node_::XHP => true,
                _ => false,
            },
            kind: class_kind,
            name: Id(pos, name.into_owned()),
            tparams: type_params,
            where_constraints: Vec::new(),
            extends: extends?
                .iter()
                .filter(|node| !node.is_ignored())
                .map(|node| self.node_to_ty(node, &type_variables))
                .collect::<Result<Vec<_>, ParseError>>()?,
            uses: Vec::new(),
            method_redeclarations: Vec::new(),
            xhp_attr_uses: Vec::new(),
            req_extends: Vec::new(),
            req_implements: Vec::new(),
            implements: implements?
                .iter()
                .filter(|node| !node.is_ignored())
                .map(|node| self.node_to_ty(node, &type_variables))
                .collect::<Result<Vec<_>, ParseError>>()?,
            consts: Vec::new(),
            typeconsts: Vec::new(),
            pu_enums: Vec::new(),
            props: Vec::new(),
            sprops: Vec::new(),
            constructor: None,
            static_methods: Vec::new(),
            methods: Vec::new(),
            user_attributes: Vec::new(),
            enum_type: None,
            // NB: We have no intention of populating this field. Any errors
            // historically emitted during shallow decl should be migrated to a
            // NAST check.
            decl_errors: Errors::empty(),
        };

        for attribute in attributes?.into_iter() {
            match attribute {
                Node_::Attribute(attribute) => cls.user_attributes.push(aast::UserAttribute {
                    name: attribute.id,
                    params: attribute
                        .args
                        .iter()
                        .map(|node| node.as_expr())
                        .collect::<Result<Vec<_>, ParseError>>()?,
                }),
                _ => (),
            }
        }

        for modifier in modifiers?.into_iter() {
            match modifier {
                Node_::Abstract => cls.kind = ClassKind::Cabstract,
                Node_::Final => cls.final_ = true,
                _ => (),
            }
        }

        fn handle_property(
            this: &mut DirectDeclSmartConstructors,
            cls: &mut shallow_decl_defs::ShallowClass,
            decl: PropertyDecl,
            type_variables: &HashSet<Rc<String>>,
        ) -> Result<(), ParseError> {
            let attributes = decl.attrs.as_attributes()?;
            let modifiers = read_member_modifiers(decl.modifiers.iter());
            let Id(pos, name) = decl.id;
            let name = if modifiers.is_static {
                name
            } else {
                strip_dollar_prefix(Cow::Owned(name)).into_owned()
            };
            let ty = this.node_to_ty(&decl.hint, type_variables)?;
            let prop = shallow_decl_defs::ShallowProp {
                const_: attributes.const_,
                xhp_attr: None,
                lateinit: attributes.late_init,
                lsb: attributes.lsb,
                name: Id(pos, name),
                needs_init: decl.expr.is_none(),
                type_: Some(ty),
                abstract_: modifiers.is_abstract,
                visibility: modifiers.visibility,
                fixme_codes: ISet::new(),
            };
            if modifiers.is_static {
                cls.sprops.push(prop)
            } else {
                cls.props.push(prop)
            }
            Ok(())
        };

        match body? {
            Node_::ClassishBody(body) => {
                for element in body {
                    match element {
                        Node_::TraitUse(names) => {
                            for name in names.iter() {
                                cls.uses.push(self.node_to_ty(name, &type_variables)?);
                            }
                        }
                        Node_::TypeConstant(constant) => {
                            cls.typeconsts.push(shallow_decl_defs::ShallowTypeconst {
                                abstract_: if constant.abstract_ {
                                    typing_defs::TypeconstAbstractKind::TCAbstract(None)
                                } else {
                                    typing_defs::TypeconstAbstractKind::TCConcrete
                                },
                                constraint: constant.constraints.first().and_then(|constraint| {
                                    match constraint {
                                        Node_::TypeConstraint(innards) => {
                                            self.node_to_ty(&innards.1, &type_variables).ok()
                                        }
                                        _ => None,
                                    }
                                }),
                                name: constant.id,
                                type_: match self.node_to_ty(&constant.type_, &type_variables) {
                                    Ok(ty) => Some(ty),
                                    Err(_) => None,
                                },
                                enforceable: (Pos::make_none(), false),
                                reifiable: constant.reified,
                            })
                        }
                        Node_::RequireClause(require) => match require.require_type {
                            Node_::Extends => cls
                                .req_extends
                                .push(self.node_to_ty(&require.name, &type_variables)?),
                            Node_::Implements => cls
                                .req_implements
                                .push(self.node_to_ty(&require.name, &type_variables)?),
                            _ => {}
                        },
                        Node_::Const(decl) => {
                            let modifiers = read_member_modifiers(decl.modifiers.iter());
                            cls.consts.push(shallow_decl_defs::ShallowClassConst {
                                abstract_: modifiers.is_abstract,
                                expr: decl.expr.map(|expr| *expr),
                                name: decl.id,
                                type_: decl.ty,
                            })
                        }
                        Node_::Property(decl) => {
                            handle_property(self, &mut cls, *decl, &type_variables)?
                        }
                        Node_::Function(decl) => {
                            let modifiers = read_member_modifiers(decl.header.modifiers.iter());
                            let is_constructor = match decl.header.name {
                                Node_::Construct(_) => true,
                                _ => false,
                            };

                            let attributes = decl.attributes.as_attributes()?;
                            let (id, ty, properties) = *self.function_into_ty(
                                "",
                                decl.attributes,
                                decl.header,
                                decl.body,
                                &type_variables,
                            )?;
                            for property in properties {
                                handle_property(self, &mut cls, property, &type_variables)?;
                            }
                            let deprecated = attributes
                                .deprecated
                                .map(|msg| format!("The method {} is deprecated: {}", id.1, msg));
                            fn get_condition_type_name(ty_opt: Option<Ty>) -> Option<String> {
                                ty_opt.and_then(|ty| {
                                    let Ty(_, ty_) = ty;
                                    match *ty_ {
                                        Ty_::Tapply(Id(_, class_name), _) => Some(class_name),
                                        _ => None,
                                    }
                                })
                            }
                            let method = shallow_decl_defs::ShallowMethod {
                                abstract_: class_kind == ClassKind::Cinterface
                                    || modifiers.is_abstract,
                                final_: modifiers.is_final,
                                memoizelsb: attributes.memoizelsb,
                                name: id,
                                override_: attributes.override_,
                                reactivity: match attributes.reactivity {
                                    Reactivity::Local(condition_type) => {
                                        Some(MethodReactivity::MethodLocal(
                                            get_condition_type_name(condition_type),
                                        ))
                                    }
                                    Reactivity::Shallow(condition_type) => {
                                        Some(MethodReactivity::MethodShallow(
                                            get_condition_type_name(condition_type),
                                        ))
                                    }
                                    Reactivity::Reactive(condition_type) => {
                                        Some(MethodReactivity::MethodReactive(
                                            get_condition_type_name(condition_type),
                                        ))
                                    }
                                    Reactivity::Nonreactive
                                    | Reactivity::MaybeReactive(_)
                                    | Reactivity::RxVar(_)
                                    | Reactivity::Pure(_) => None,
                                },
                                dynamicallycallable: false,
                                type_: ty,
                                visibility: modifiers.visibility,
                                fixme_codes: ISet::new(),
                                deprecated,
                            };
                            if is_constructor {
                                cls.constructor = Some(method);
                            } else if modifiers.is_static {
                                cls.static_methods.push(method);
                            } else {
                                cls.methods.push(method);
                            }
                        }
                        _ => (), // It's not our job to report errors here.
                    }
                }
            }
            body => return Err(format!("Expected a classish body, but was {:?}", body)),
        }

        // Match ordering of attributes produced by the OCaml decl parser (even
        // though it's the reverse of the syntactic ordering).
        cls.user_attributes.reverse();

        Rc::make_mut(&mut self.state.decls)
            .classes
            .insert(key, Rc::new(cls));

        self.state
            .classish_name_builder
            .parsed_classish_declaration();

        Ok(Node_::Ignored)
    }

    fn make_property_declaration(
        &mut self,
        attrs: Self::R,
        modifiers: Self::R,
        hint: Self::R,
        declarator: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // Sometimes the declarator is a single element list.
        let declarator = match declarator? {
            Node_::List(nodes) => nodes
                .first()
                .ok_or("Expected a declarator, but was given an empty list.".to_owned())?
                .clone(),
            declarator => declarator,
        };
        match declarator {
            Node_::ListItem(innards) => {
                let (name, initializer) = *innards;
                let name = match name {
                    Node_::List(nodes) => nodes
                        .first()
                        .ok_or("Expected a name, but was given an empty list.".to_owned())?
                        .clone(),
                    name => name,
                };
                Ok(Node_::Property(Box::new(PropertyDecl {
                    attrs: attrs?,
                    modifiers: modifiers?,
                    hint: match hint? {
                        Node_::Ignored => initializer.clone(),
                        hint => hint,
                    },
                    id: get_name("", &name)?,
                    expr: match initializer {
                        Node_::Expr(e) => Some(Box::new(*e)),
                        Node_::Ignored => None,
                        n => n.as_expr().ok().map(Box::new),
                    },
                })))
            }
            n => Err(format!("Expected a ListItem, but was {:?}", n)),
        }
    }

    fn make_property_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        Ok(Node_::ListItem(Box::new((name?, initializer?))))
    }

    fn make_methodish_declaration(
        &mut self,
        attributes: Self::R,
        header: Self::R,
        body: Self::R,
        closer: Self::R,
    ) -> Self::R {
        match header? {
            Node_::FunctionHeader(header) => Ok(Node_::Function(Box::new(FunctionDecl {
                attributes: attributes?,
                header: *header,
                body: match body {
                    // If we don't have a FunctionDecl body, use the closing token.
                    // A closing token of '}' indicates a regular FunctionDecl,
                    // while a closing token of ';' indicates an abstract
                    // FunctionDecl.
                    Ok(Node_::Ignored) => closer?,
                    Ok(body) => body,
                    Err(_) => Node_::Ignored,
                },
            }))),
            n => Err(format!("Expected a FunctionDecl header, but was {:?}", n)),
        }
    }

    fn make_classish_body(&mut self, _arg0: Self::R, body: Self::R, _arg2: Self::R) -> Self::R {
        Ok(Node_::ClassishBody(body?.into_vec()))
    }

    fn make_enum_declaration(
        &mut self,
        _arg0: Self::R,
        _arg1: Self::R,
        name: Self::R,
        _arg3: Self::R,
        extends: Self::R,
        _arg5: Self::R,
        _arg6: Self::R,
        cases: Self::R,
        _arg8: Self::R,
    ) -> Self::R {
        let name = name?;
        let id = get_name("", &name)?;
        let hint = self.node_to_ty(&extends?, &HashSet::new())?;
        let extends = self.node_to_ty(
            &self.make_apply(
                Node_::Name("HH\\BuiltinEnum".to_string(), name.get_pos()?),
                name,
                None,
            )?,
            &HashSet::new(),
        )?;
        let key = id.1.clone();
        let cls = shallow_decl_defs::ShallowClass {
            mode: match self.state.file_mode_builder {
                FileModeBuilder::None | FileModeBuilder::Pending => Mode::Mstrict,
                FileModeBuilder::Set(mode) => mode,
            },
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassKind::Cenum,
            name: Id(id.0, prefix_slash(Cow::Owned(id.1)).into_owned()),
            tparams: Vec::new(),
            where_constraints: Vec::new(),
            extends: vec![extends],
            uses: Vec::new(),
            method_redeclarations: Vec::new(),
            xhp_attr_uses: Vec::new(),
            req_extends: Vec::new(),
            req_implements: Vec::new(),
            implements: Vec::new(),
            consts: cases?
                .into_iter()
                .map(|node| match node {
                    Node_::ListItem(innards) => {
                        let (name, value) = *innards;
                        Ok(shallow_decl_defs::ShallowClassConst {
                            abstract_: false,
                            expr: Some(value.as_expr()?),
                            name: get_name("", &name)?,
                            type_: Ty(Reason::Rwitness(value.get_pos()?), hint.1.clone()),
                        })
                    }
                    n => Err(format!("Expected an enum case, got {:?}", n)),
                })
                .collect::<Result<Vec<_>, ParseError>>()?,
            typeconsts: Vec::new(),
            pu_enums: Vec::new(),
            props: Vec::new(),
            sprops: Vec::new(),
            constructor: None,
            static_methods: Vec::new(),
            methods: Vec::new(),
            user_attributes: Vec::new(),
            enum_type: Some(EnumType {
                base: hint,
                constraint: None,
            }),
            decl_errors: Errors::empty(),
        };
        Rc::make_mut(&mut self.state.decls)
            .classes
            .insert(key, Rc::new(cls));
        Ok(Node_::Ignored)
    }

    fn make_enumerator(
        &mut self,
        name: Self::R,
        _arg1: Self::R,
        value: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        Ok(Node_::ListItem(Box::new((name?, value?))))
    }

    fn make_tuple_type_specifier(
        &mut self,
        left_paren: Self::R,
        inner: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        // We don't need to include the inner list in this position merging
        // because by definition it's already contained by the two brackets.
        let pos = Pos::merge(&left_paren?.get_pos()?, &right_paren?.get_pos()?)?;
        Ok(Node_::Hint(HintValue::Tuple(inner?.into_vec()), pos))
    }

    fn make_shape_type_specifier(
        &mut self,
        shape: Self::R,
        _arg1: Self::R,
        fields: Self::R,
        open: Self::R,
        rparen: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| match node {
                Node_::ShapeFieldSpecifier(decl) => Ok(*decl),
                n => Err(format!("Expected a shape field specifier, but was {:?}", n)),
            })
            .collect::<Result<Vec<_>, ParseError>>()?;
        let kind = match open? {
            Node_::DotDotDot => ShapeKind::OpenShape,
            _ => ShapeKind::ClosedShape,
        };
        Ok(Node_::Hint(
            HintValue::Shape(Box::new(ShapeDecl { kind, fields })),
            Pos::merge(&shape?.get_pos()?, &rparen?.get_pos()?)?,
        ))
    }

    fn make_shape_expression(
        &mut self,
        shape: Self::R,
        _left_paren: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| match node {
                Node_::ListItem(innards) => {
                    let (key, value) = *innards;
                    let key = match key {
                        Node_::IntLiteral(s, p) => ShapeFieldName::SFlitInt((p, s)),
                        Node_::StringLiteral(s, p) => ShapeFieldName::SFlitStr((p, s)),
                        n => {
                            return Err(format!(
                            "Expected an int literal, string literal, or class const, but was {:?}",
                            n
                        ))
                        }
                    };
                    let value = value.as_expr()?;
                    Ok((key, value))
                }
                n => Err(format!("Expected a ListItem but was {:?}", n)),
            })
            .collect::<Result<Vec<_>, _>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&shape?.get_pos()?, &right_paren?.get_pos()?)?,
            nast::Expr_::Shape(fields),
        ))))
    }

    fn make_tuple_expression(
        &mut self,
        tuple: Self::R,
        _left_paren: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = fields?
            .into_iter()
            .map(|node| node.as_expr())
            .collect::<Result<Vec<_>, ParseError>>()?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            Pos::merge(&tuple?.get_pos()?, &right_paren?.get_pos()?)?,
            nast::Expr_::List(fields),
        ))))
    }

    fn make_classname_type_specifier(
        &mut self,
        classname: Self::R,
        _lt: Self::R,
        targ: Self::R,
        _arg3: Self::R,
        gt: Self::R,
    ) -> Self::R {
        let (classname, targ, gt) = (classname?, targ?, gt?);
        let id = get_name("\\", &classname)?;
        match gt {
            Node_::Ignored => Ok(Node_::Hint(HintValue::String, id.0)),
            gt => Ok(Node_::Hint(
                HintValue::Apply(Box::new((id, vec![targ]))),
                Pos::merge(&classname.get_pos()?, &gt.get_pos()?)?,
            )),
        }
    }

    fn make_scope_resolution_expression(
        &mut self,
        class_name: Self::R,
        _arg1: Self::R,
        value: Self::R,
    ) -> Self::R {
        let (class_name, value) = (class_name?, value?);
        let pos = Pos::merge(&class_name.get_pos()?, &value.get_pos()?)?;
        let Id(class_name_pos, class_name_str) = get_name("", &class_name)?;
        let class_name_str = prefix_slash(Cow::Owned(class_name_str)).into_owned();
        let class_id = aast::ClassId(
            class_name_pos.clone(),
            match class_name_str.to_ascii_lowercase().as_ref() {
                "\\self" => aast::ClassId_::CIself,
                _ => aast::ClassId_::CI(Id(class_name_pos, class_name_str)),
            },
        );
        let value_id = get_name("", &value)?;
        Ok(Node_::Expr(Box::new(aast::Expr(
            pos,
            nast::Expr_::ClassConst(Box::new((class_id, (value_id.0, value_id.1)))),
        ))))
    }

    fn make_field_specifier(
        &mut self,
        is_optional: Self::R,
        name: Self::R,
        _arg2: Self::R,
        type_: Self::R,
    ) -> Self::R {
        let is_optional = match is_optional? {
            Node_::Question(_) => true,
            _ => false,
        };
        Ok(Node_::ShapeFieldSpecifier(Box::new(ShapeFieldDecl {
            is_optional,
            name: name?,
            type_: type_?,
        })))
    }

    fn make_field_initializer(&mut self, key: Self::R, _arg1: Self::R, value: Self::R) -> Self::R {
        Ok(Node_::ListItem(Box::new((key?, value?))))
    }

    fn make_varray_type_specifier(
        &mut self,
        varray: Self::R,
        _less_than: Self::R,
        tparam: Self::R,
        _arg3: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::Hint(
            HintValue::Varray(Box::new(tparam?)),
            Pos::merge(&varray?.get_pos()?, &greater_than?.get_pos()?)?,
        ))
    }

    fn make_vector_array_type_specifier(
        &mut self,
        array: Self::R,
        _less_than: Self::R,
        tparam: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::Hint(
            HintValue::Array(Box::new((tparam?, Node_::Ignored))),
            Pos::merge(&array?.get_pos()?, &greater_than?.get_pos()?)?,
        ))
    }

    fn make_darray_type_specifier(
        &mut self,
        darray: Self::R,
        _less_than: Self::R,
        key_type: Self::R,
        _comma: Self::R,
        value_type: Self::R,
        _arg5: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::Hint(
            HintValue::Darray(Box::new((key_type?, value_type?))),
            Pos::merge(&darray?.get_pos()?, &greater_than?.get_pos()?)?,
        ))
    }

    fn make_map_array_type_specifier(
        &mut self,
        array: Self::R,
        _less_than: Self::R,
        key_type: Self::R,
        _comma: Self::R,
        value_type: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::Hint(
            HintValue::Array(Box::new((key_type?, value_type?))),
            Pos::merge(&array?.get_pos()?, &greater_than?.get_pos()?)?,
        ))
    }

    fn make_old_attribute_specification(
        &mut self,
        ltlt: Self::R,
        attrs: Self::R,
        gtgt: Self::R,
    ) -> Self::R {
        match attrs? {
            Node_::List(nodes) => Ok(Node_::BracketedList(Box::new((
                ltlt?.get_pos()?,
                nodes,
                gtgt?.get_pos()?,
            )))),
            node => Err(format!(
                "Expected List in old_attribute_specification, but got {:?}",
                node
            )),
        }
    }

    fn make_constructor_call(
        &mut self,
        name: Self::R,
        _arg1: Self::R,
        args: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        let id = get_name("", &name?)?;
        Ok(Node_::Attribute(Box::new(Attribute {
            id,
            args: args?.into_vec(),
        })))
    }

    fn make_trait_use(&mut self, _arg0: Self::R, used: Self::R, _arg2: Self::R) -> Self::R {
        Ok(Node_::TraitUse(Box::new(used?)))
    }

    fn make_require_clause(
        &mut self,
        _arg0: Self::R,
        require_type: Self::R,
        name: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        Ok(Node_::RequireClause(Box::new(RequireClause {
            require_type: require_type?,
            name: name?,
        })))
    }

    fn make_nullable_type_specifier(&mut self, question_mark: Self::R, hint: Self::R) -> Self::R {
        let hint = hint?;
        let hint_pos = hint.get_pos()?;
        Ok(Node_::Hint(
            HintValue::Nullable(Box::new(hint)),
            Pos::merge(&question_mark?.get_pos()?, &hint_pos)?,
        ))
    }

    fn make_like_type_specifier(&mut self, tilde: Self::R, type_: Self::R) -> Self::R {
        let (tilde, type_) = (tilde?, type_?);
        let pos = Pos::merge(&tilde.get_pos()?, &type_.get_pos()?)?;
        Ok(Node_::Hint(HintValue::LikeType(Box::new(type_)), pos))
    }

    fn make_closure_type_specifier(
        &mut self,
        left_paren: Self::R,
        _arg1: Self::R,
        _arg2: Self::R,
        _arg3: Self::R,
        args: Self::R,
        _arg5: Self::R,
        _arg6: Self::R,
        ret_hint: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        Ok(Node_::Hint(
            HintValue::Closure(Box::new(ClosureTypeHint {
                args: args?,
                ret_hint: ret_hint?,
            })),
            Pos::merge(&left_paren?.get_pos()?, &right_paren?.get_pos()?)?,
        ))
    }

    fn make_closure_parameter_type_specifier(&mut self, _arg0: Self::R, name: Self::R) -> Self::R {
        name
    }

    fn make_type_const_declaration(
        &mut self,
        attributes: Self::R,
        modifiers: Self::R,
        _arg2: Self::R,
        _arg3: Self::R,
        name: Self::R,
        _arg5: Self::R,
        constraints: Self::R,
        _arg7: Self::R,
        type_: Self::R,
        _arg9: Self::R,
    ) -> Self::R {
        let attributes = attributes?.as_attributes()?;
        let abstract_ = modifiers?.iter().fold(false, |abstract_, node| match node {
            Node_::Abstract => true,
            _ => abstract_,
        });
        let id = get_name("", &name?)?;
        Ok(Node_::TypeConstant(Box::new(TypeConstant {
            id,
            constraints: constraints?.into_vec(),
            type_: type_?,
            abstract_,
            reified: attributes.reifiable,
        })))
    }

    fn make_decorated_expression(&mut self, decorator: Self::R, expr: Self::R) -> Self::R {
        Ok(Node_::ListItem(Box::new((decorator?, expr?))))
    }

    fn make_type_constant(
        &mut self,
        ty: Self::R,
        coloncolon: Self::R,
        constant_name: Self::R,
    ) -> Self::R {
        let (ty, _coloncolon, constant_name) = (ty?, coloncolon?, constant_name?);
        let id = get_name("", &constant_name)?;
        let pos = Pos::merge(&ty.get_pos()?, &constant_name.get_pos()?)?;
        match ty {
            Node_::Hint(HintValue::Access(mut innards), _) => {
                // Nested applies have to be collapsed.
                innards.1.push(id);
                Ok(Node_::Hint(HintValue::Access(innards), pos))
            }
            ty => Ok(Node_::Hint(
                HintValue::Access(Box::new((ty, vec![id]))),
                pos,
            )),
        }
    }

    fn make_soft_type_specifier(&mut self, at_token: Self::R, hint: Self::R) -> Self::R {
        let hint = hint?;
        let hint_pos = hint.get_pos()?;
        Ok(Node_::Hint(
            HintValue::Soft(Box::new(hint)),
            Pos::merge(&at_token?.get_pos()?, &hint_pos)?,
        ))
    }

    // A type specifier preceded by an attribute list. At the time of writing,
    // only the <<__Soft>> attribute is permitted here.
    fn make_attributized_specifier(&mut self, attributes: Self::R, hint: Self::R) -> Self::R {
        match attributes? {
            Node_::BracketedList(args) => {
                let (ltlt_pos, attributes, gtgt_pos) = &*args;
                match attributes.as_slice() {
                    [Node_::Attribute(attribute)] => {
                        let Id(_, attribute_name) = &attribute.id;
                        if attribute_name == "__Soft" {
                            let attributes_pos = Pos::merge(ltlt_pos, gtgt_pos)?;
                            let hint = hint?;
                            let hint_pos = hint.get_pos()?;
                            return Ok(Node_::Hint(
                                HintValue::Soft(Box::new(hint)),
                                Pos::merge(&attributes_pos, &hint_pos)?,
                            ));
                        }
                    }
                    _ => (),
                }
            }
            _ => (),
        }
        hint
    }

    fn make_vector_type_specifier(
        &mut self,
        vec: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        _arg3: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        self.make_apply(vec?, hint?, Some(greater_than?))
    }

    fn make_dictionary_type_specifier(
        &mut self,
        dict: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        self.make_apply(dict?, hint?, Some(greater_than?))
    }

    fn make_keyset_type_specifier(
        &mut self,
        keyset: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        _arg3: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        self.make_apply(keyset?, hint?, Some(greater_than?))
    }
}

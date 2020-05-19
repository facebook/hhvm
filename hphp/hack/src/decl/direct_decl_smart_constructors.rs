// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::{Cell, RefCell};
use std::rc::Rc;

use bumpalo::{
    collections::{String, Vec},
    Bump,
};

use hh_autoimport_rust as hh_autoimport;
use naming_special_names_rust as naming_special_names;

use arena_collections::{AssocListMut, MultiSetMut};
use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use oxidized_by_ref::{
    aast, aast_defs,
    ast_defs::{Bop, ClassKind, ConstraintKind, FunKind, Id, ShapeFieldName, Uop, Variance},
    decl_defs::MethodReactivity,
    errors::Errors,
    file_info::Mode,
    i_set::ISet,
    nast,
    pos::Pos,
    relative_path::RelativePath,
    s_set::SSet,
    shallow_decl_defs::{self, ShallowClassConst, ShallowMethod, ShallowProp, ShallowTypeconst},
    shape_map::ShapeField,
    tany_sentinel::TanySentinel,
    typing_defs,
    typing_defs::{
        EnumType, FunArity, FunElt, FunParam, FunParams, FunType, ParamMode, ParamMutability,
        PossiblyEnforcedTy, Reactivity, ShapeFieldType, ShapeKind, Tparam, Ty, Ty_,
        TypeconstAbstractKind, TypedefType,
    },
    typing_defs_flags::{FunParamFlags, FunTypeFlags},
    typing_reason::Reason,
};
use parser_core_types::{
    indexed_source_text::IndexedSourceText, lexable_token::LexableToken,
    lexable_trivia::LexablePositionedTrivia, positioned_token::PositionedToken,
    source_text::SourceText, token_kind::TokenKind, trivia_kind::TriviaKind,
};

mod direct_decl_smart_constructors_generated;

pub use direct_decl_smart_constructors_generated::DirectDeclSmartConstructors;

impl<'a> DirectDeclSmartConstructors<'a> {
    pub fn new(src: &SourceText<'a>, arena: &'a Bump) -> Self {
        Self {
            state: State::new(IndexedSourceText::new(src.clone()), arena),
        }
    }

    #[inline(always)]
    #[allow(clippy::mut_from_ref)]
    pub fn alloc<T>(&self, val: T) -> &'a mut T {
        self.state.arena.alloc(val)
    }

    pub fn get_name(&self, namespace: &'a str, name: Node_<'a>) -> Result<Id<'a>, ParseError> {
        fn qualified_name_from_parts<'a>(
            this: &DirectDeclSmartConstructors<'a>,
            namespace: &'a str,
            parts: &'a [Node_<'a>],
            pos: &'a Pos<'a>,
        ) -> Result<Id<'a>, ParseError> {
            let mut qualified_name =
                String::with_capacity_in(namespace.len() + parts.len() * 10, this.state.arena);
            match parts.first() {
                Some(Node_::Backslash(_)) => (), // Already fully-qualified
                _ => qualified_name.push_str(namespace),
            }
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
            Ok(Id(pos, qualified_name.into_bump_str()))
        }

        match name {
            Node_::Name(name, pos) => {
                // always a simple name
                let mut fully_qualified =
                    String::with_capacity_in(namespace.len() + name.len(), self.state.arena);
                fully_qualified.push_str(namespace);
                fully_qualified.push_str(name);
                Ok(Id(pos, fully_qualified.into_bump_str()))
            }
            Node_::XhpName(name, pos) => {
                // xhp names are always unqualified
                Ok(Id(pos, name))
            }
            Node_::QualifiedName(parts, pos) => {
                qualified_name_from_parts(self, namespace, parts, pos)
            }
            Node_::Construct(pos) => Ok(Id(pos, naming_special_names::members::__CONSTRUCT)),
            n => {
                return Err(format!(
                    "Expected a name, XHP name, or qualified name, but got {:?}",
                    n,
                ))
            }
        }
    }

    fn map_to_slice<T>(
        &self,
        node: Node<'a>,
        mut f: impl FnMut(Node_<'a>) -> Result<T, ParseError>,
    ) -> Result<&'a [T], ParseError> {
        let node = node?;
        let nodes = node.iter();
        let mut result = Vec::new_in(self.state.arena);
        for node in nodes {
            result.push(f(*node)?)
        }
        Ok(result.into_bump_slice())
    }

    fn filter_map_to_slice<T>(
        &self,
        node: Node<'a>,
        mut f: impl FnMut(Node_<'a>) -> Result<Option<T>, ParseError>,
    ) -> Result<&'a [T], ParseError> {
        let node = node?;
        let nodes = node.iter();
        let mut result = Vec::new_in(self.state.arena);
        for node in nodes {
            if let Some(mapped) = f(*node)? {
                result.push(mapped)
            }
        }
        Ok(result.into_bump_slice())
    }
}

type ParseError = std::string::String;

#[derive(Clone, Debug)]
pub struct InProgressDecls<'a> {
    pub classes: AssocListMut<'a, &'a str, shallow_decl_defs::ShallowClass<'a>>,
    pub funs: AssocListMut<'a, &'a str, typing_defs::FunElt<'a>>,
    pub typedefs: AssocListMut<'a, &'a str, typing_defs::TypedefType<'a>>,
    pub consts: AssocListMut<'a, &'a str, typing_defs::Ty<'a>>,
}

pub fn empty_decls<'a>(arena: &'a Bump) -> InProgressDecls<'a> {
    InProgressDecls {
        classes: AssocListMut::new_in(arena),
        funs: AssocListMut::new_in(arena),
        typedefs: AssocListMut::new_in(arena),
        consts: AssocListMut::new_in(arena),
    }
}

fn prefix_slash<'a>(arena: &'a Bump, name: &str) -> &'a str {
    let mut s = String::with_capacity_in(1 + name.len(), arena);
    s.push('\\');
    s.push_str(name);
    s.into_bump_str()
}

fn strip_dollar_prefix<'a>(name: &'a str) -> &'a str {
    name.trim_start_matches("$")
}

const TANY: Ty<'_> = Ty(Reason::none(), &Ty_::Tany(TanySentinel));

fn tany() -> Ty<'static> {
    TANY
}

fn tarraykey<'a>(arena: &'a Bump) -> Ty<'a> {
    Ty(
        Reason::none(),
        arena.alloc(Ty_::Tprim(arena.alloc(aast::Tprim::Tarraykey))),
    )
}

#[derive(Debug)]
struct Modifiers {
    is_static: bool,
    visibility: aast::Visibility,
    is_abstract: bool,
    is_final: bool,
}

fn read_member_modifiers<'a: 'b, 'b>(modifiers: impl Iterator<Item = &'b Node_<'a>>) -> Modifiers {
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

#[derive(Clone, Debug)]
struct NamespaceInfo<'a> {
    name: &'a str,
    imports: AssocListMut<'a, &'a str, &'a str>,
}

#[derive(Clone, Debug)]
struct NamespaceBuilder<'a> {
    arena: &'a Bump,
    stack: Vec<'a, NamespaceInfo<'a>>,
}

impl<'a> NamespaceBuilder<'a> {
    fn new_in(arena: &'a Bump) -> Self {
        NamespaceBuilder {
            arena,
            stack: bumpalo::vec![in arena; NamespaceInfo {
                name: "\\",
                imports: AssocListMut::new_in(arena),
            }],
        }
    }

    fn push_namespace(&mut self, name: &str) {
        let current = self.current_namespace();
        let mut fully_qualified =
            String::with_capacity_in(current.len() + name.len() + 1, self.arena);
        fully_qualified.push_str(current);
        fully_qualified.push_str(name);
        fully_qualified.push('\\');
        self.stack.push(NamespaceInfo {
            name: fully_qualified.into_bump_str(),
            imports: AssocListMut::new_in(self.arena),
        });
    }

    fn pop_namespace(&mut self) {
        // We'll never push a namespace for a declaration of items in the global
        // namespace (e.g., `namespace { ... }`), so only pop if we are in some
        // namespace other than the global one.
        if self.stack.len() > 1 {
            self.stack.pop().unwrap();
        }
    }

    fn current_namespace(&self) -> &'a str {
        self.stack.last().map(|ni| ni.name).unwrap_or("\\")
    }

    fn add_import(&mut self, name: &'a str, aliased_name: Option<&'a str>) {
        let imports = &mut self
            .stack
            .last_mut()
            .expect("Attempted to get the current import map, but namespace stack was empty")
            .imports;
        match aliased_name {
            Some(aliased_name) => {
                imports.insert(aliased_name, name);
            }
            None => {
                let aliased_name = name
                    .rsplit_terminator('\\')
                    .nth(0)
                    .expect("Expected at least one entry in import name");
                imports.insert(aliased_name, name);
            }
        };
    }

    fn rename_import(&self, name: &'a str) -> &'a str {
        let trimmed_name = name.trim_start_matches('\\');
        for ni in self.stack.iter().rev() {
            if let Some(name) = ni.imports.get(trimmed_name) {
                if name.starts_with('\\') {
                    return name;
                } else {
                    return prefix_slash(self.arena, name);
                }
            }
        }
        hh_autoimport::TYPES_MAP
            .get(trimmed_name)
            .map(|renamed| prefix_slash(self.arena, renamed))
            .unwrap_or(name)
    }
}

#[derive(Clone, Debug)]
enum ClassishNameBuilder<'a> {
    /// We are not in a classish declaration.
    NotInClassish,

    /// We saw a classish keyword token followed by a Name, so we make it
    /// available as the name of the containing class declaration.
    InClassish(&'a (&'a str, &'a Pos<'a>, TokenKind)),
}

impl<'a> ClassishNameBuilder<'a> {
    fn new() -> Self {
        ClassishNameBuilder::NotInClassish
    }

    fn lexed_name_after_classish_keyword(
        &mut self,
        arena: &'a Bump,
        name: &str,
        pos: &'a Pos<'a>,
        token_kind: TokenKind,
    ) {
        use ClassishNameBuilder::*;
        match self {
            NotInClassish => {
                let mut class_name = String::with_capacity_in(1 + name.len(), arena);
                class_name.push('\\');
                class_name.push_str(name);
                *self = InClassish(arena.alloc((class_name.into_bump_str(), pos, token_kind)))
            }
            InClassish(_) => (),
        }
    }

    fn parsed_classish_declaration(&mut self) {
        *self = ClassishNameBuilder::NotInClassish;
    }

    fn get_current_classish_name(&self) -> Option<(&'a str, &'a Pos<'a>)> {
        use ClassishNameBuilder::*;
        match self {
            NotInClassish => None,
            InClassish((name, pos, _)) => Some((name, pos)),
        }
    }

    fn in_interface(&self) -> bool {
        use ClassishNameBuilder::*;
        match self {
            InClassish((_, _, TokenKind::Interface)) => true,
            InClassish((_, _, _)) | NotInClassish => false,
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
    pub arena: &'a bumpalo::Bump,
    pub decls: Rc<InProgressDecls<'a>>,
    filename: &'a RelativePath<'a>,
    namespace_builder: Rc<NamespaceBuilder<'a>>,
    classish_name_builder: ClassishNameBuilder<'a>,
    type_parameters: Rc<Vec<'a, SSet<'a>>>,

    // We don't need to wrap this in a Cow because it's very small.
    file_mode_builder: FileModeBuilder,

    previous_token_kind: TokenKind,
}

impl<'a> State<'a> {
    pub fn new(source_text: IndexedSourceText<'a>, arena: &'a Bump) -> State<'a> {
        let path = source_text.source_text().file_path();
        let prefix = path.prefix();
        let path = String::from_str_in(path.path_str(), arena).into_bump_str();
        let filename = RelativePath::make(prefix, path);
        State {
            source_text,
            arena,
            filename: arena.alloc(filename),
            decls: Rc::new(empty_decls(arena)),
            namespace_builder: Rc::new(NamespaceBuilder::new_in(arena)),
            classish_name_builder: ClassishNameBuilder::new(),
            type_parameters: Rc::new(Vec::new_in(arena)),
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
pub struct VariableDecl<'a> {
    attributes: Node_<'a>,
    visibility: Node_<'a>,
    kind: ParamMode,
    hint: Node_<'a>,
    id: Id<'a>,
    variadic: bool,
    initializer: Node_<'a>,
}

#[derive(Clone, Debug)]
pub struct Attribute<'a> {
    id: Id<'a>,
    args: &'a [Node_<'a>],
}

#[derive(Clone, Debug)]
pub struct FunctionHeader<'a> {
    name: Node_<'a>,
    modifiers: Node_<'a>,
    type_params: Node_<'a>,
    param_list: Node_<'a>,
    ret_hint: Node_<'a>,
}

#[derive(Clone, Debug)]
pub struct RequireClause<'a> {
    require_type: Node_<'a>,
    name: Node_<'a>,
}

#[derive(Copy, Clone, Debug)]
pub struct ShapeFieldDecl<'a> {
    is_optional: bool,
    name: Node_<'a>,
    type_: Node_<'a>,
}

#[derive(Clone, Debug)]
pub struct ShapeDecl<'a> {
    kind: ShapeKind,
    fields: &'a [ShapeFieldDecl<'a>],
}

#[derive(Clone, Debug)]
pub struct TypeParameterDecl<'a> {
    name: Node_<'a>,
    reified: aast::ReifyKind,
    variance: Variance,
    constraints: &'a [(ConstraintKind, Node_<'a>)],
}

#[derive(Clone, Debug)]
pub struct ClosureTypeHint<'a> {
    args: Node_<'a>,
    ret_hint: Node_<'a>,
}

#[derive(Clone, Debug)]
pub struct NamespaceUseClause<'a> {
    id: Id<'a>,
    as_: Option<&'a str>,
}

#[derive(Copy, Clone, Debug)]
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
    Assignment,
}

#[derive(Copy, Clone, Debug)]
pub enum Node_<'a> {
    List(&'a [Node_<'a>]),
    BracketedList(&'a (&'a Pos<'a>, &'a [Node_<'a>], &'a Pos<'a>)),
    Ignored,
    Name(&'a str, &'a Pos<'a>),
    XhpName(&'a str, &'a Pos<'a>),
    QualifiedName(&'a [Node_<'a>], &'a Pos<'a>),
    Array(&'a Pos<'a>),
    Darray(&'a Pos<'a>),
    Varray(&'a Pos<'a>),
    StringLiteral(&'a str, &'a Pos<'a>), // For shape keys and const expressions.
    IntLiteral(&'a str, &'a Pos<'a>),    // For const expressions.
    FloatingLiteral(&'a str, &'a Pos<'a>), // For const expressions.
    BooleanLiteral(&'a str, &'a Pos<'a>), // For const expressions.
    Null(&'a Pos<'a>),                   // For const expressions.
    Ty(Ty<'a>),
    TypeconstAccess(&'a (Cell<&'a Pos<'a>>, Ty<'a>, RefCell<Vec<'a, Id<'a>>>)),
    Backslash(&'a Pos<'a>), // This needs a pos since it shows up in names.
    ListItem(&'a (Node_<'a>, Node_<'a>)),
    Const(&'a ShallowClassConst<'a>),
    Variable(&'a VariableDecl<'a>),
    Attribute(&'a Attribute<'a>),
    FunctionHeader(&'a FunctionHeader<'a>),
    Constructor {
        method: &'a ShallowMethod<'a>,
        properties: &'a [ShallowProp<'a>],
    },
    Method {
        method: &'a ShallowMethod<'a>,
        is_static: bool,
    },
    Property {
        decls: &'a [ShallowProp<'a>],
        is_static: bool,
    },
    TraitUse(&'a Node_<'a>),
    TypeConstant(&'a ShallowTypeconst<'a>),
    RequireClause(&'a RequireClause<'a>),
    ClassishBody(&'a [Node_<'a>]),
    TypeParameter(&'a TypeParameterDecl<'a>),
    TypeConstraint(&'a (ConstraintKind, Node_<'a>)),
    ShapeFieldSpecifier(&'a ShapeFieldDecl<'a>),
    NamespaceUseClause(&'a NamespaceUseClause<'a>),
    Expr(&'a nast::Expr<'a>),
    Operator(&'a (&'a Pos<'a>, OperatorType)),
    Construct(&'a Pos<'a>),
    LessThan(&'a Pos<'a>),    // This needs a pos since it shows up in generics.
    GreaterThan(&'a Pos<'a>), // This needs a pos since it shows up in generics.
    LeftParen(&'a Pos<'a>),   // This needs a pos since it shows up in tuples and shapes.
    RightParen(&'a Pos<'a>),  // This needs a pos since it shows up in tuples and shapes.
    Shape(&'a Pos<'a>),       // This needs a pos since it shows up in shapes.
    Question(&'a Pos<'a>),    // This needs a pos since it shows up in nullable types.
    This(&'a Pos<'a>),        // This needs a pos since it shows up in Taccess.
    ColonColon(&'a Pos<'a>),  // This needs a pos since it shows up in Taccess.
    TypeParameters(&'a [Tparam<'a>]),

    LessThanLessThan(&'a Pos<'a>), // This needs a pos since it shows up in attributized type specifiers.
    GreaterThanGreaterThan(&'a Pos<'a>), // This needs a pos since it shows up in attributized type specifiers.

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
    Newtype,
    Private,
    Protected,
    Public,
    Reify,
    Semicolon,
    Static,
    Super,
    Trait,
    Type,
    XHP,
    Yield,
}

impl<'a> Node_<'a> {
    pub fn get_pos(self, arena: &'a Bump) -> Result<&'a Pos<'a>, ParseError> {
        match self {
            Node_::Name(_, pos) => Ok(pos),
            Node_::Ty(ty) => Ok(ty.get_pos().unwrap_or(Pos::none())),
            Node_::TypeconstAccess((pos, _, _)) => Ok(pos.get()),
            Node_::XhpName(_, pos) => Ok(pos),
            Node_::QualifiedName(_, pos) => Ok(pos),
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
            | Node_::Operator(&(pos, _)) => Ok(pos),
            Node_::ListItem(items) => {
                let fst = &items.0;
                let snd = &items.1;
                match (fst.get_pos(arena), snd.get_pos(arena)) {
                    (Ok(fst_pos), Ok(snd_pos)) => Pos::merge(arena, fst_pos, snd_pos),
                    (Ok(pos), Err(_)) => Ok(pos),
                    (Err(_), Ok(pos)) => Ok(pos),
                    (Err(_), Err(_)) => Err(format!("No pos found for {:?} or {:?}", fst, snd)),
                }
            }
            Node_::List(items) => self.pos_from_slice(&items, arena),
            Node_::BracketedList(&(first_pos, inner_list, second_pos)) => Pos::merge(
                arena,
                first_pos,
                Pos::merge(arena, self.pos_from_slice(&inner_list, arena)?, second_pos)?,
            ),
            Node_::Expr(&aast::Expr(pos, _)) => Ok(pos),
            _ => Err(format!("No pos found for node {:?}", self)),
        }
    }

    fn pos_from_slice(
        &self,
        nodes: &'a [Node_<'a>],
        arena: &'a Bump,
    ) -> Result<&'a Pos<'a>, ParseError> {
        nodes.iter().fold(
            Err(format!("No pos found for any children under {:?}", self)),
            |acc, elem| match (acc, elem.get_pos(arena)) {
                (Ok(acc_pos), Ok(elem_pos)) => Pos::merge(arena, acc_pos, elem_pos),
                (Err(_), Ok(elem_pos)) => Ok(elem_pos),
                (acc, Err(_)) => acc,
            },
        )
    }

    fn as_slice(self, b: &'a Bump) -> &'a [Self] {
        match self {
            Node_::List(items) => items,
            Node_::BracketedList(innards) => {
                let (_, items, _) = *innards;
                items
            }
            Node_::Ignored => &[],
            n => bumpalo::vec![in b; n].into_bump_slice(),
        }
    }

    fn iter<'b>(&'b self) -> NodeIterHelper<'a, 'b>
    where
        'a: 'b,
    {
        match self {
            Node_::List(items) => NodeIterHelper::Vec(items.iter()),
            Node_::BracketedList(&(_, items, _)) => NodeIterHelper::Vec(items.iter()),
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

    fn as_expr(self, arena: &'a Bump) -> Result<nast::Expr<'a>, ParseError> {
        let expr_ = match self {
            Node_::Expr(&expr) => return Ok(expr),
            Node_::IntLiteral(s, _) => aast::Expr_::Int(s),
            Node_::FloatingLiteral(s, _) => aast::Expr_::Float(s),
            Node_::StringLiteral(s, _) => aast::Expr_::String(s),
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
        let pos = self.get_pos(arena)?;
        Ok(aast::Expr(pos, expr_))
    }

    fn as_attributes(self, arena: &'a Bump) -> Result<Attributes<'a>, ParseError> {
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
            enforceable: None,
        };

        let mut reactivity_condition_type = None;
        for attribute in self.iter() {
            match attribute {
                // If we see the attribute `__OnlyRxIfImpl(Foo::class)`, set
                // `reactivity_condition_type` to `Foo`.
                Node_::Attribute(Attribute {
                    id: Id(_, "__OnlyRxIfImpl"),
                    args:
                        [Node_::Expr(aast::Expr(
                            pos,
                            aast::Expr_::ClassConst((
                                aast::ClassId(_, aast::ClassId_::CI(class_name)),
                                (_, "class"),
                            )),
                        ))],
                }) => {
                    reactivity_condition_type = Some(Ty(
                        arena.alloc(Reason::hint(*pos)),
                        arena.alloc(Ty_::Tapply(arena.alloc((*class_name, &[][..])))),
                    ));
                }
                _ => (),
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
                        attributes.deprecated =
                            attribute.args.first().and_then(|node| match *node {
                                Node_::StringLiteral(val, _) => Some(val),
                                _ => None,
                            })
                    }
                    "__Reifiable" => attributes.reifiable = Some(attribute.id.0),
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
                    "__Enforceable" => {
                        attributes.enforceable = Some(attribute.id.0);
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

pub type Node<'a> = Result<Node_<'a>, ParseError>;

struct Attributes<'a> {
    reactivity: Reactivity<'a>,
    param_mutability: Option<ParamMutability>,
    deprecated: Option<&'a str>,
    reifiable: Option<&'a Pos<'a>>,
    returns_mutable: bool,
    late_init: bool,
    const_: bool,
    lsb: bool,
    memoizelsb: bool,
    override_: bool,
    at_most_rx_as_func: bool,
    enforceable: Option<&'a Pos<'a>>,
}

impl<'a> DirectDeclSmartConstructors<'a> {
    fn set_mode(&mut self, token: &PositionedToken) {
        for trivia in &token.trailing {
            if trivia.kind == TriviaKind::SingleLineComment {
                match &*String::from_utf8_lossy_in(
                    trivia.text_raw(self.state.source_text.source_text()),
                    self.state.arena,
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

    fn node_to_ty(&self, node: Node_<'a>) -> Result<Ty<'a>, ParseError> {
        match node {
            Node_::Ty(ty) => Ok(ty),
            Node_::TypeconstAccess((pos, ty, names)) => {
                let pos = pos.get();
                let names = Vec::from_iter_in(names.borrow().iter().copied(), self.state.arena);
                Ok(Ty(
                    self.alloc(Reason::hint(pos)),
                    self.alloc(Ty_::Taccess(
                        self.alloc(typing_defs::TaccessType(*ty, names.into_bump_slice())),
                    )),
                ))
            }
            Node_::Array(pos) => Ok(Ty(
                self.alloc(Reason::hint(pos)),
                self.alloc(Ty_::Tarray(self.alloc((None, None)))),
            )),
            Node_::Varray(pos) => Ok(Ty(
                self.alloc(Reason::hint(pos)),
                self.alloc(Ty_::Tvarray(tany())),
            )),
            Node_::Darray(pos) => Ok(Ty(
                self.alloc(Reason::hint(pos)),
                self.alloc(Ty_::Tdarray(self.alloc((tany(), tany())))),
            )),
            Node_::This(pos) => Ok(Ty(self.alloc(Reason::hint(pos)), self.alloc(Ty_::Tthis))),
            Node_::Expr(&expr) => {
                fn expr_to_ty<'a>(
                    arena: &'a Bump,
                    expr: nast::Expr<'a>,
                ) -> Result<Ty_<'a>, ParseError> {
                    use aast::Expr_::*;
                    match expr.1 {
                        Null => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tnull))),
                        This => Ok(Ty_::Tthis),
                        True | False => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tbool))),
                        Int(_) => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tint))),
                        Float(_) => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tfloat))),
                        String(_) => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        String2(_) => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        PrefixedString(_) => Ok(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        Unop(&(_op, expr)) => expr_to_ty(arena, expr),
                        ParenthesizedExpr(&expr) => expr_to_ty(arena, expr),
                        Any => Ok(Ty_::Tany(TanySentinel)),

                        Array(_) | ArrayGet(_) | As(_) | Assert(_) | Await(_) | Binop(_)
                        | BracedExpr(_) | Call(_) | Callconv(_) | Cast(_) | ClassConst(_)
                        | ClassGet(_) | Clone(_) | Collection(_) | Darray(_) | Dollardollar(_)
                        | Efun(_) | Eif(_) | ExprList(_) | FunctionPointer(_) | FunId(_)
                        | Id(_) | Import(_) | Is(_) | KeyValCollection(_) | Lfun(_) | List(_)
                        | Lplaceholder(_) | Lvar(_) | MethodCaller(_) | MethodId(_) | New(_)
                        | ObjGet(_) | Omitted | Pair(_) | Pipe(_) | PUAtom(_) | PUIdentifier(_)
                        | Record(_) | Shape(_) | SmethodId(_) | Suspend(_) | ValCollection(_)
                        | Varray(_) | Xml(_) | Yield(_) | YieldBreak | YieldFrom(_) => {
                            Err(format!("Cannot convert expr to type: {:?}", expr))
                        }
                    }
                }

                Ok(Ty(
                    self.alloc(Reason::witness(expr.0)),
                    self.alloc(expr_to_ty(self.state.arena, expr)?),
                ))
            }
            Node_::IntLiteral(_, pos) => Ok(Ty(
                self.alloc(Reason::witness(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tint))),
            )),
            Node_::FloatingLiteral(_, pos) => Ok(Ty(
                self.alloc(Reason::witness(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tfloat))),
            )),
            Node_::StringLiteral(_, pos) => Ok(Ty(
                self.alloc(Reason::witness(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tstring))),
            )),
            Node_::BooleanLiteral(_, pos) => Ok(Ty(
                self.alloc(Reason::witness(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tbool))),
            )),
            Node_::Null(pos) => Ok(Ty(
                self.alloc(Reason::hint(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tnull))),
            )),
            node => {
                let Id(pos, name) = self.get_name("", node)?;
                let reason = self.alloc(Reason::hint(pos));
                let ty_ = if self
                    .state
                    .type_parameters
                    .iter()
                    .any(|tps| tps.contains(&name))
                {
                    Ty_::Tgeneric(name)
                } else {
                    match name.as_ref() {
                        "nothing" => Ty_::Tunion(&[]),
                        "nonnull" => Ty_::Tnonnull,
                        "dynamic" => Ty_::Tdynamic,
                        "varray_or_darray" => {
                            Ty_::TvarrayOrDarray(self.alloc((tarraykey(self.state.arena), tany())))
                        }
                        _ => {
                            let name = self
                                .state
                                .namespace_builder
                                .rename_import(self.prefix_ns(name));
                            Ty_::Tapply(self.alloc((Id(pos, name), &[][..])))
                        }
                    }
                };
                Ok(Ty(reason, self.alloc(ty_)))
            }
        }
    }

    /// Converts any node that can represent a list of Node_::TypeParameter
    /// into the type parameter list and a list of all type variables. Used for
    /// classes, methods, and functions.
    fn as_type_params(&self, node: Node_<'a>) -> Result<(&'a [Tparam<'a>], SSet<'a>), ParseError> {
        let mut type_variables = MultiSetMut::new_in(self.state.arena);
        let type_params = self.map_to_slice(Ok(node), |node| {
            let TypeParameterDecl {
                name,
                variance,
                reified,
                constraints,
            } = match node {
                Node_::TypeParameter(decl) => decl,
                n => return Err(format!("Expected a type parameter, but got {:?}", n)),
            };
            let id = self.get_name("", *name)?;
            let constraints_iter = constraints.iter();
            let mut constraints = Vec::new_in(self.state.arena);
            for constraint in constraints_iter {
                let (kind, value) = *constraint;
                constraints.push((kind, self.node_to_ty(value)?));
            }
            let constraints = constraints.into_bump_slice();
            type_variables.insert(id.1);
            Ok(Tparam {
                variance: *variance,
                name: id,
                constraints,
                reified: *reified,
                user_attributes: &[],
            })
        })?;
        Ok((type_params, type_variables.into()))
    }

    fn pop_type_params(&mut self, node: Node_<'a>) -> Result<&'a [Tparam<'a>], ParseError> {
        match node {
            Node_::TypeParameters(tparams) => {
                Rc::make_mut(&mut self.state.type_parameters).pop().unwrap();
                Ok(tparams)
            }
            _ => Ok(&[]),
        }
    }

    fn function_into_ty(
        &mut self,
        namespace: &'a str,
        attributes: Node_<'a>,
        header: &'a FunctionHeader<'a>,
        body: Node_,
    ) -> Result<(Id<'a>, Ty<'a>, &'a [ShallowProp<'a>]), ParseError> {
        let id = self.get_name(namespace, header.name)?;
        let (params, properties, arity) = self.into_variables_list(header.param_list)?;
        let type_ = match header.name {
            Node_::Construct(pos) => Ty(
                self.alloc(Reason::witness(pos)),
                self.alloc(Ty_::Tprim(self.alloc(aast::Tprim::Tvoid))),
            ),
            _ => self.node_to_ty(header.ret_hint).unwrap_or_else(|_| tany()),
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
        let attributes = attributes.as_attributes(self.state.arena)?;
        // TODO(hrust) Put this in a helper. Possibly do this for all flags.
        let mut flags = match fun_kind {
            FunKind::FSync => FunTypeFlags::empty(),
            FunKind::FAsync => FunTypeFlags::ASYNC,
            FunKind::FGenerator => FunTypeFlags::GENERATOR,
            FunKind::FAsyncGenerator => FunTypeFlags::ASYNC | FunTypeFlags::GENERATOR,
            FunKind::FCoroutine => FunTypeFlags::IS_COROUTINE,
        };
        if attributes.returns_mutable {
            flags |= FunTypeFlags::RETURNS_MUTABLE;
        }
        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(header.type_params)?;
        let ft = self.alloc(FunType {
            arity,
            tparams,
            where_constraints: &[],
            params,
            ret: PossiblyEnforcedTy {
                enforced: false,
                type_,
            },
            reactive: attributes.reactivity,
            flags,
        });

        let ty = Ty(self.alloc(Reason::witness(id.0)), self.alloc(Ty_::Tfun(ft)));
        Ok((id, ty, properties))
    }

    fn prefix_ns(&self, name: &'a str) -> &'a str {
        if name.starts_with("\\") {
            name
        } else {
            let current = self.state.namespace_builder.current_namespace();
            let mut fully_qualified =
                String::with_capacity_in(current.len() + name.len(), self.state.arena);
            fully_qualified.push_str(current);
            fully_qualified.push_str(&name);
            fully_qualified.into_bump_str()
        }
    }

    fn into_variables_list(
        &self,
        list: Node_<'a>,
    ) -> Result<(FunParams<'a>, &'a [ShallowProp<'a>], FunArity<'a>), ParseError> {
        match list {
            Node_::List(nodes) => {
                let mut variables = Vec::new_in(self.state.arena);
                let mut properties = Vec::new_in(self.state.arena);
                let mut arity = FunArity::Fstandard;
                for variable in nodes.iter() {
                    match variable {
                        Node_::Variable(&VariableDecl {
                            attributes,
                            visibility,
                            kind,
                            hint,
                            id,
                            variadic,
                            initializer,
                        }) => {
                            let attributes = attributes.as_attributes(self.state.arena)?;

                            if let Ok(visibility) = visibility.as_visibility() {
                                let Id(pos, name) = id;
                                let name = strip_dollar_prefix(name);
                                properties.push(ShallowProp {
                                    const_: false,
                                    xhp_attr: None,
                                    lateinit: false,
                                    lsb: false,
                                    name: Id(pos, name),
                                    needs_init: true,
                                    type_: self.node_to_ty(hint).ok(),
                                    abstract_: false,
                                    visibility,
                                    fixme_codes: ISet::empty(),
                                });
                            }

                            let type_ = match &hint {
                                Node_::Ignored => tany(),
                                _ => self.node_to_ty(hint).map(|ty| match ty {
                                    Ty(r, &Ty_::Tfun(ref fun_type))
                                        if attributes.at_most_rx_as_func =>
                                    {
                                        let mut fun_type = (*fun_type).clone();
                                        fun_type.reactive = Reactivity::RxVar(None);
                                        Ty(r, self.alloc(Ty_::Tfun(self.alloc(fun_type))))
                                    }
                                    ty => ty,
                                })?,
                            };
                            let mut flags = match attributes.param_mutability {
                                Some(ParamMutability::ParamBorrowedMutable) => {
                                    FunParamFlags::MUTABLE_FLAGS_BORROWED
                                }
                                Some(ParamMutability::ParamOwnedMutable) => {
                                    FunParamFlags::MUTABLE_FLAGS_OWNED
                                }
                                Some(ParamMutability::ParamMaybeMutable) => {
                                    FunParamFlags::MUTABLE_FLAGS_MAYBE
                                }
                                None => FunParamFlags::empty(),
                            };
                            match kind {
                                ParamMode::FPinout => {
                                    flags |= FunParamFlags::INOUT;
                                }
                                ParamMode::FPnormal => {}
                            };
                            if !initializer.is_ignored() {
                                flags |= FunParamFlags::HAS_DEFAULT;
                            }
                            let param = &*self.alloc(FunParam {
                                pos: id.0,
                                name: Some(id.1),
                                type_: PossiblyEnforcedTy {
                                    enforced: false,
                                    type_,
                                },
                                flags,
                                rx_annotation: None,
                            });
                            arity = match (arity, initializer, variadic) {
                                (FunArity::Fstandard, Node_::Ignored, false) => {
                                    variables.push(param);
                                    FunArity::Fstandard
                                }
                                (FunArity::Fstandard, Node_::Ignored, true) => {
                                    FunArity::Fvariadic(param)
                                }
                                (FunArity::Fstandard, _, _) => {
                                    variables.push(param);
                                    FunArity::Fstandard
                                }
                                (arity, _, _) => {
                                    variables.push(param);
                                    arity
                                }
                            };
                        }
                        n => return Err(format!("Expected a variable, but got {:?}", n)),
                    }
                }
                Ok((
                    variables.into_bump_slice(),
                    properties.into_bump_slice(),
                    arity,
                ))
            }
            Node_::Ignored => Ok((&[], &[], FunArity::Fstandard)),
            n => Err(format!("Expected a list of variables, but got {:?}", n)),
        }
    }

    fn make_apply(
        &self,
        base_ty: Id<'a>,
        type_arguments: Node_<'a>,
        pos_to_merge: Option<&'a Pos<'a>>,
    ) -> Node<'a> {
        let Id(base_ty_pos, base_ty_name) = base_ty;
        let id = Id(
            base_ty_pos,
            self.state.namespace_builder.rename_import(base_ty_name),
        );
        let type_arguments_iter = type_arguments.iter();
        let mut type_arguments = Vec::new_in(self.state.arena);
        for node in type_arguments_iter {
            type_arguments.push(self.node_to_ty(*node)?);
        }
        let type_arguments = type_arguments.into_bump_slice();
        let ty_ = Ty_::Tapply(self.alloc((id, type_arguments)));
        let pos = match pos_to_merge {
            Some(p) => Pos::merge(self.state.arena, base_ty_pos, p)?,
            None => base_ty_pos,
        };
        Ok(self.hint_ty(pos, ty_))
    }

    fn hint_ty(&self, pos: &'a Pos<'a>, ty_: Ty_<'a>) -> Node_<'a> {
        Node_::Ty(Ty(self.alloc(Reason::hint(pos)), self.alloc(ty_)))
    }

    fn prim_ty(&self, tprim: aast::Tprim<'a>, pos: &'a Pos<'a>) -> Node_<'a> {
        self.hint_ty(pos, Ty_::Tprim(self.alloc(tprim)))
    }
}

enum NodeIterHelper<'a: 'b, 'b> {
    Empty,
    Single(&'b Node_<'a>),
    Vec(std::slice::Iter<'b, Node_<'a>>),
}

impl<'a, 'b> Iterator for NodeIterHelper<'a, 'b> {
    type Item = &'b Node_<'a>;

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

impl<'a> FlattenOp for DirectDeclSmartConstructors<'a> {
    type S = Node<'a>;

    fn flatten(&self, lst: std::vec::Vec<Self::S>) -> Self::S {
        let lst_iter = lst.iter().cloned();
        let mut r = Vec::new_in(self.state.arena);
        for s in lst_iter {
            match s? {
                Node_::List(children) => {
                    for child in children.iter().cloned() {
                        r.push(child)
                    }
                }
                x => {
                    if !Self::is_zero(&Ok(x)) {
                        r.push(x)
                    }
                }
            }
        }
        Ok(match r.into_bump_slice() {
            [] => Node_::Ignored,
            [node] => *node,
            slice => Node_::List(slice),
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
            String::from_utf8_lossy_in(
                this.state.source_text.source_text().sub(
                    token.leading_start_offset().unwrap_or(0) + token.leading_width(),
                    token.width(),
                ),
                this.state.arena,
            )
            .into_bump_str()
        };
        let token_pos = |this: &Self| {
            let start = this
                .state
                .source_text
                .offset_to_file_pos_triple(token.start_offset());
            let end = this
                .state
                .source_text
                .offset_to_file_pos_triple(token.end_offset() + 1);
            Pos::from_lnum_bol_cnum(this.state.arena, this.state.filename, start, end)
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
                        .lexed_name_after_classish_keyword(
                            self.state.arena,
                            name,
                            pos,
                            self.state.previous_token_kind,
                        );
                }
                Node_::Name(name, pos)
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
                    .trim_end_matches("'"),
                token_pos(self),
            ),
            TokenKind::DoubleQuotedStringLiteral => Node_::StringLiteral(
                token_text(self)
                    .trim_start_matches('"')
                    .trim_end_matches('"'),
                token_pos(self),
            ),
            TokenKind::DecimalLiteral
            | TokenKind::OctalLiteral
            | TokenKind::HexadecimalLiteral
            | TokenKind::BinaryLiteral => Node_::IntLiteral(token_text(self), token_pos(self)),
            TokenKind::FloatingLiteral => Node_::FloatingLiteral(token_text(self), token_pos(self)),
            TokenKind::NullLiteral => Node_::Null(token_pos(self)),
            TokenKind::BooleanLiteral => Node_::BooleanLiteral(token_text(self), token_pos(self)),
            TokenKind::String => self.prim_ty(aast::Tprim::Tstring, token_pos(self)),
            TokenKind::Int => self.prim_ty(aast::Tprim::Tint, token_pos(self)),
            TokenKind::Float => self.prim_ty(aast::Tprim::Tfloat, token_pos(self)),
            // "double" and "boolean" are parse errors--they should be written
            // "float" and "bool". The decl-parser treats the incorrect names as
            // type names rather than primitives.
            TokenKind::Double | TokenKind::Boolean => self.hint_ty(
                token_pos(self),
                Ty_::Tapply(self.alloc((Id(token_pos(self), token_text(self)), &[][..]))),
            ),
            TokenKind::Num => self.prim_ty(aast::Tprim::Tnum, token_pos(self)),
            TokenKind::Bool => self.prim_ty(aast::Tprim::Tbool, token_pos(self)),
            TokenKind::Mixed => Node_::Ty(Ty(
                self.alloc(Reason::hint(token_pos(self))),
                self.alloc(Ty_::Tmixed),
            )),
            TokenKind::Void => self.prim_ty(aast::Tprim::Tvoid, token_pos(self)),
            TokenKind::Arraykey => self.prim_ty(aast::Tprim::Tarraykey, token_pos(self)),
            TokenKind::Noreturn => self.prim_ty(aast::Tprim::Tnoreturn, token_pos(self)),
            TokenKind::Resource => self.prim_ty(aast::Tprim::Tresource, token_pos(self)),
            TokenKind::Array => Node_::Array(token_pos(self)),
            TokenKind::Darray => Node_::Darray(token_pos(self)),
            TokenKind::Varray => Node_::Varray(token_pos(self)),
            TokenKind::Backslash => Node_::Backslash(token_pos(self)),
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
            TokenKind::Tilde => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Tilde)))
            }
            TokenKind::Exclamation => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Not)))
            }
            TokenKind::Plus => Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Plus))),
            TokenKind::Minus => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Minus)))
            }
            TokenKind::PlusPlus => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::PlusPlus)))
            }
            TokenKind::MinusMinus => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::MinusMinus)))
            }
            TokenKind::At => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Silence)))
            }
            TokenKind::Star => Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Star))),
            TokenKind::Slash => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Slash)))
            }
            TokenKind::EqualEqual => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Eqeq)))
            }
            TokenKind::EqualEqualEqual => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Eqeqeq)))
            }
            TokenKind::StarStar => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Starstar)))
            }
            TokenKind::AmpersandAmpersand => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Ampamp)))
            }
            TokenKind::BarBar => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Barbar)))
            }
            TokenKind::LessThan => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Lt)))
            }
            TokenKind::LessThanEqual => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Lte)))
            }
            TokenKind::GreaterThan => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Gt)))
            }
            TokenKind::GreaterThanEqual => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Gte)))
            }
            TokenKind::Dot => Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Dot))),
            TokenKind::Ampersand => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Amp)))
            }
            TokenKind::Bar => Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Bar))),
            TokenKind::LessThanLessThan => Node_::LessThanLessThan(token_pos(self)),
            TokenKind::GreaterThanGreaterThan => Node_::GreaterThanGreaterThan(token_pos(self)),
            TokenKind::Percent => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Percent)))
            }
            TokenKind::QuestionQuestion => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::QuestionQuestion)))
            }
            TokenKind::Equal => {
                Node_::Operator(&*self.alloc((token_pos(self), OperatorType::Assignment)))
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
            TokenKind::Newtype => Node_::Newtype,
            TokenKind::Type => Node_::Type,
            TokenKind::XHP => Node_::XHP,
            TokenKind::Yield => Node_::Yield,
            TokenKind::Semicolon => Node_::Semicolon,
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

    fn make_list(&mut self, items: std::vec::Vec<Self::R>, _: usize) -> Self::R {
        if items.iter().any(|node| matches!(node, Ok(Node_::Yield))) {
            Ok(Node_::Yield)
        } else {
            let items_iter = items.into_iter();
            let mut items = Vec::new_in(self.state.arena);
            for node in items_iter {
                if !(matches!(node, Ok(Node_::Ignored))) {
                    items.push(node?);
                }
            }
            let items = items.into_bump_slice();
            if items.is_empty() {
                Ok(Node_::Ignored)
            } else {
                Ok(Node_::List(items))
            }
        }
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        let arg0 = arg0?;
        let pos = arg0.get_pos(self.state.arena);
        Ok(match arg0 {
            Node_::Ignored => Node_::Ignored,
            Node_::List(nodes) => Node_::QualifiedName(nodes, pos?),
            node => Node_::QualifiedName(
                bumpalo::vec![in self.state.arena; node].into_bump_slice(),
                pos?,
            ),
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

    fn make_simple_initializer(&mut self, equals: Self::R, expr: Self::R) -> Self::R {
        // If the expr is Ignored, bubble up the assignment operator so that we
        // can tell that *some* initializer was here. Useful for class
        // properties, where we need to enforce that properties without default
        // values are initialized in the constructor.
        match expr? {
            Node_::Ignored => equals,
            expr => Ok(expr),
        }
    }

    fn make_array_intrinsic_expression(
        &mut self,
        array: Self::R,
        _arg1: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = self.map_to_slice(fields, |node| match node {
            Node_::ListItem(&(key, value)) => {
                let key = key.as_expr(self.state.arena)?;
                let value = value.as_expr(self.state.arena)?;
                Ok(aast::Afield::AFkvalue(key, value))
            }
            node => Ok(aast::Afield::AFvalue(node.as_expr(self.state.arena)?)),
        })?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                array?.get_pos(self.state.arena)?,
                right_paren?.get_pos(self.state.arena)?,
            )?,
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
        let fields = self.map_to_slice(fields, |node| match node {
            Node_::ListItem(&(key, value)) => {
                let key = key.as_expr(self.state.arena)?;
                let value = value.as_expr(self.state.arena)?;
                Ok((key, value))
            }
            n => return Err(format!("Expected a ListItem but was {:?}", n)),
        })?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                darray?.get_pos(self.state.arena)?,
                right_bracket?.get_pos(self.state.arena)?,
            )?,
            nast::Expr_::Darray(self.alloc((None, fields))),
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
        let fields = self.map_to_slice(fields, |node| match node {
            Node_::ListItem(&(key, value)) => {
                let key = key.as_expr(self.state.arena)?;
                let value = value.as_expr(self.state.arena)?;
                Ok(aast::Field(key, value))
            }
            n => return Err(format!("Expected a ListItem but was {:?}", n)),
        })?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                dict?.get_pos(self.state.arena)?,
                right_bracket?.get_pos(self.state.arena)?,
            )?,
            nast::Expr_::KeyValCollection(self.alloc((aast_defs::KvcKind::Dict, None, fields))),
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
        let fields = self.map_to_slice(fields, |node| node.as_expr(self.state.arena))?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                keyset?.get_pos(self.state.arena)?,
                right_bracket?.get_pos(self.state.arena)?,
            )?,
            nast::Expr_::ValCollection(self.alloc((aast_defs::VcKind::Keyset, None, fields))),
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
        let fields = self.map_to_slice(fields, |node| node.as_expr(self.state.arena))?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                varray?.get_pos(self.state.arena)?,
                right_bracket?.get_pos(self.state.arena)?,
            )?,
            nast::Expr_::Varray(self.alloc((None, fields))),
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
        let fields = self.map_to_slice(fields, |node| node.as_expr(self.state.arena))?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                vec?.get_pos(self.state.arena)?,
                right_bracket?.get_pos(self.state.arena)?,
            )?,
            nast::Expr_::ValCollection(self.alloc((aast_defs::VcKind::Vec, None, fields))),
        ))))
    }

    fn make_element_initializer(
        &mut self,
        key: Self::R,
        _arg1: Self::R,
        value: Self::R,
    ) -> Self::R {
        Ok(Node_::ListItem(self.alloc((key?, value?))))
    }

    fn make_prefix_unary_expression(&mut self, op: Self::R, value: Self::R) -> Self::R {
        let (op, value) = (op?, value?);
        let pos = match (
            op.get_pos(self.state.arena),
            value.get_pos(self.state.arena),
        ) {
            (Ok(op_pos), Ok(value_pos)) => Pos::merge(self.state.arena, op_pos, value_pos)?,
            _ => return Ok(Node_::Ignored),
        };
        let op = match &op {
            Node_::Operator(&(_, op)) => match op {
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
        Ok(Node_::Expr(self.alloc(aast::Expr(
            pos,
            aast::Expr_::Unop(self.alloc((op, value.as_expr(self.state.arena)?))),
        ))))
    }

    fn make_postfix_unary_expression(&mut self, value: Self::R, op: Self::R) -> Self::R {
        let (value, op) = (value?, op?);
        let pos = match (
            value.get_pos(self.state.arena),
            op.get_pos(self.state.arena),
        ) {
            (Ok(value_pos), Ok(op_pos)) => Pos::merge(self.state.arena, value_pos, op_pos)?,
            _ => return Ok(Node_::Ignored),
        };
        let op = match &op {
            Node_::Operator(&(_, op)) => match op {
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
        Ok(Node_::Expr(self.alloc(aast::Expr(
            pos,
            aast::Expr_::Unop(self.alloc((op, value.as_expr(self.state.arena)?))),
        ))))
    }

    fn make_binary_expression(&mut self, lhs: Self::R, op: Self::R, rhs: Self::R) -> Self::R {
        let (lhs, op, rhs) = (lhs?, op?, rhs?);
        let pos = match (
            lhs.get_pos(self.state.arena),
            op.get_pos(self.state.arena),
            rhs.get_pos(self.state.arena),
        ) {
            (Ok(lhs_pos), Ok(op_pos), Ok(rhs_pos)) => Pos::merge(
                self.state.arena,
                Pos::merge(self.state.arena, lhs_pos, op_pos)?,
                rhs_pos,
            )?,
            _ => return Ok(Node_::Ignored),
        };

        let op = match &op {
            Node_::Operator(&(_, op)) => match op {
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

        Ok(Node_::Expr(self.alloc(aast::Expr(
            pos,
            aast::Expr_::Binop(self.alloc((
                op,
                lhs.as_expr(self.state.arena)?,
                rhs.as_expr(self.state.arena)?,
            ))),
        ))))
    }

    fn make_list_item(&mut self, item: Self::R, sep: Self::R) -> Self::R {
        Ok(match (item?, sep?) {
            (Node_::Ignored, Node_::Ignored) => Node_::Ignored,
            (x, Node_::Ignored) | (Node_::Ignored, x) => x,
            (x, y) => Node_::ListItem(self.alloc((x, y))),
        })
    }

    fn make_type_arguments(
        &mut self,
        less_than: Self::R,
        arguments: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Ok(Node_::BracketedList(self.alloc((
            less_than?.get_pos(self.state.arena)?,
            arguments?.as_slice(self.state.arena),
            greater_than?.get_pos(self.state.arena)?,
        ))))
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::R,
        type_arguments: Self::R,
    ) -> Self::R {
        let (class_type, type_arguments) = (class_type?, type_arguments?);
        let unqualified_id = self.get_name("", class_type)?;
        if unqualified_id.1.trim_start_matches("\\") == "varray_or_darray" {
            let pos = Pos::merge(
                self.state.arena,
                unqualified_id.0,
                type_arguments.get_pos(self.state.arena)?,
            )?;
            let type_arguments = type_arguments.as_slice(self.state.arena);
            let ty_ = match type_arguments {
                [tk, tv] => Ty_::TvarrayOrDarray(self.alloc((
                    self.node_to_ty(*tk).unwrap_or_else(|_| tany()),
                    self.node_to_ty(*tv).unwrap_or_else(|_| tany()),
                ))),
                [tv] => Ty_::TvarrayOrDarray(self.alloc((
                    tarraykey(self.state.arena),
                    self.node_to_ty(*tv).unwrap_or_else(|_| tany()),
                ))),
                _ => Ty_::Tany(TanySentinel),
            };
            Ok(self.hint_ty(pos, ty_))
        } else {
            let Id(pos, class_type) =
                self.get_name(self.state.namespace_builder.current_namespace(), class_type)?;
            self.make_apply(
                Id(pos, class_type),
                type_arguments,
                type_arguments.get_pos(self.state.arena).ok(),
            )
        }
    }

    fn make_alias_declaration(
        &mut self,
        _attributes: Self::R,
        keyword: Self::R,
        name: Self::R,
        generic_params: Self::R,
        constraint: Self::R,
        _equal: Self::R,
        aliased_type: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let (name, aliased_type) = (name?, aliased_type?);
        match name {
            Node_::Ignored => (),
            _ => {
                let Id(pos, name) =
                    self.get_name(self.state.namespace_builder.current_namespace(), name)?;
                let ty = self.node_to_ty(aliased_type)?;
                let constraint = match constraint? {
                    Node_::TypeConstraint(kind_and_hint) => {
                        let (_kind, hint) = *kind_and_hint;
                        Some(self.node_to_ty(hint)?)
                    }
                    _ => None,
                };
                // Pop the type params stack only after creating all inner types.
                let tparams = self.pop_type_params(generic_params?)?;
                let typedef = TypedefType {
                    pos,
                    vis: match keyword? {
                        Node_::Type => aast::TypedefVisibility::Transparent,
                        Node_::Newtype => aast::TypedefVisibility::Opaque,
                        _ => aast::TypedefVisibility::Transparent,
                    },
                    tparams,
                    constraint,
                    type_: ty,
                    // NB: We have no intention of populating this
                    // field. Any errors historically emitted during
                    // shallow decl should be migrated to a NAST
                    // check.
                    decl_errors: Some(Errors::empty()),
                };

                Rc::make_mut(&mut self.state.decls)
                    .typedefs
                    .insert(name, typedef);
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
        Ok(Node_::TypeConstraint(self.alloc((kind, value?))))
    }

    fn make_type_parameter(
        &mut self,
        _arg0: Self::R,
        reify: Self::R,
        variance: Self::R,
        name: Self::R,
        constraints: Self::R,
    ) -> Self::R {
        let constraints = self.filter_map_to_slice(constraints, |node| match node {
            Node_::TypeConstraint(&constraint) => Ok(Some(constraint)),
            Node_::Ignored => Ok(None),
            n => return Err(format!("Expected a type constraint, but was {:?}", n)),
        })?;
        Ok(Node_::TypeParameter(self.alloc(TypeParameterDecl {
            name: name?,
            variance: match variance? {
                Node_::Operator(&(_, OperatorType::Minus)) => Variance::Contravariant,
                Node_::Operator(&(_, OperatorType::Plus)) => Variance::Covariant,
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
        let node = Node_::BracketedList(self.alloc((
            arg0?.get_pos(self.state.arena)?,
            arg1?.as_slice(self.state.arena),
            arg2?.get_pos(self.state.arena)?,
        )));
        let (tparams, tparam_names) = self.as_type_params(node)?;
        Rc::make_mut(&mut self.state.type_parameters).push(tparam_names);
        Ok(Node_::TypeParameters(tparams))
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
                let id = self.get_name("", innards.1)?;
                match innards.0 {
                    Node_::DotDotDot => (true, id),
                    _ => (false, id),
                }
            }
            name => (false, self.get_name("", name)?),
        };
        let attributes = attributes?;
        let visibility = visibility?;
        let initializer = initializer?;
        let kind = match inout? {
            Node_::Inout => ParamMode::FPinout,
            _ => ParamMode::FPnormal,
        };
        let hint = hint?;
        Ok(Node_::Variable(self.alloc(VariableDecl {
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
        let parsed_attributes = attributes.as_attributes(self.state.arena)?;
        Ok(match header? {
            Node_::FunctionHeader(header) => {
                let (Id(pos, name), type_, _) = self.function_into_ty(
                    self.state.namespace_builder.current_namespace(),
                    attributes,
                    header,
                    body,
                )?;
                let deprecated = parsed_attributes.deprecated.map(|msg| {
                    let mut s = String::new_in(self.state.arena);
                    s.push_str("The function ");
                    s.push_str(name.trim_start_matches("\\"));
                    s.push_str(" is deprecated: ");
                    s.push_str(msg);
                    s.into_bump_str()
                });
                let fun_elt = FunElt {
                    deprecated,
                    type_,
                    // NB: We have no intention of populating this field.
                    // Any errors historically emitted during shallow decl
                    // should be migrated to a NAST check.
                    decl_errors: Some(Errors::empty()),
                    pos,
                };
                Rc::make_mut(&mut self.state.decls)
                    .funs
                    .insert(name, fun_elt);
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
            name => Node_::FunctionHeader(self.alloc(FunctionHeader {
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
            Node_::List([Node_::List([name, initializer])]) => {
                let id = self.get_name(
                    if self
                        .state
                        .classish_name_builder
                        .get_current_classish_name()
                        .is_some()
                    {
                        ""
                    } else {
                        self.state.namespace_builder.current_namespace()
                    },
                    *name,
                )?;
                let modifiers = modifiers?;
                let ty = self
                    .node_to_ty(hint)
                    .or_else(|_| self.node_to_ty(*initializer))
                    .unwrap_or_else(|_| tany());
                let modifiers = read_member_modifiers(modifiers.iter());
                if self
                    .state
                    .classish_name_builder
                    .get_current_classish_name()
                    .is_some()
                {
                    Node_::Const(self.alloc(shallow_decl_defs::ShallowClassConst {
                        abstract_: modifiers.is_abstract,
                        expr: match initializer {
                            Node_::Expr(e) => Some(*e.clone()),
                            n => n.as_expr(self.state.arena).ok(),
                        },
                        name: id,
                        type_: ty,
                    }))
                } else {
                    Rc::make_mut(&mut self.state.decls).consts.insert(id.1, ty);
                    Node_::Ignored
                }
            }
            _ => Node_::Ignored,
        })
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        let (name, initializer) = (name?, initializer?);
        Ok(match name {
            Node_::Ignored => Node_::Ignored,
            _ => {
                Node_::List(bumpalo::vec![in self.state.arena; name, initializer].into_bump_slice())
            }
        })
    }

    fn make_namespace_declaration_header(&mut self, _keyword: Self::R, name: Self::R) -> Self::R {
        if let Ok(Id(_, name)) = self.get_name("", name?) {
            Rc::make_mut(&mut self.state.namespace_builder).push_namespace(&name);
        }
        Ok(Node_::Ignored)
    }

    fn make_namespace_body(&mut self, _arg0: Self::R, body: Self::R, _arg2: Self::R) -> Self::R {
        let body = body?;
        let is_empty = matches!(body, Node_::Semicolon);
        if !is_empty {
            Rc::make_mut(&mut self.state.namespace_builder).pop_namespace();
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
        for import in imports?.iter() {
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
        let Id(_, prefix) = self.get_name("", prefix?)?;
        for import in imports?.iter() {
            if let Node_::NamespaceUseClause(nuc) = import {
                let mut id = String::new_in(self.state.arena);
                id.push_str(prefix);
                id.push_str(nuc.id.1);
                Rc::make_mut(&mut self.state.namespace_builder)
                    .add_import(id.into_bump_str(), nuc.as_);
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
        let id = self.get_name("", name?)?;
        let as_ = if let Node_::As = as_? {
            Some(self.get_name("", aliased_name?)?.1)
        } else {
            None
        };
        Ok(Node_::NamespaceUseClause(
            self.alloc(NamespaceUseClause { id, as_ }),
        ))
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
        let Id(pos, name) =
            self.get_name(self.state.namespace_builder.current_namespace(), name?)?;

        let mut class_kind = match class_keyword? {
            Node_::Interface => ClassKind::Cinterface,
            Node_::Trait => ClassKind::Ctrait,
            _ => ClassKind::Cnormal,
        };
        let mut final_ = false;

        for modifier in modifiers?.iter() {
            match modifier {
                Node_::Abstract => class_kind = ClassKind::Cabstract,
                Node_::Final => final_ = true,
                _ => (),
            }
        }

        let mut constructor = None;

        let where_constraints = Vec::new_in(self.state.arena);
        let mut uses = Vec::new_in(self.state.arena);
        let method_redeclarations = Vec::new_in(self.state.arena);
        let xhp_attr_uses = Vec::new_in(self.state.arena);
        let mut req_extends = Vec::new_in(self.state.arena);
        let mut req_implements = Vec::new_in(self.state.arena);
        let mut consts = Vec::new_in(self.state.arena);
        let mut typeconsts = Vec::new_in(self.state.arena);
        let pu_enums = Vec::new_in(self.state.arena);
        let mut props = Vec::new_in(self.state.arena);
        let mut sprops = Vec::new_in(self.state.arena);
        let mut static_methods = Vec::new_in(self.state.arena);
        let mut methods = Vec::new_in(self.state.arena);
        let mut user_attributes = Vec::new_in(self.state.arena);

        for attribute in attributes?.iter() {
            match attribute {
                Node_::Attribute(Attribute { id, args }) => {
                    let args_iter = args.iter();
                    let mut params = Vec::new_in(self.state.arena);
                    for node in args_iter {
                        params.push(node.as_expr(self.state.arena)?);
                    }
                    let params = params.into_bump_slice();
                    user_attributes.push(aast::UserAttribute { name: *id, params });
                }
                _ => (),
            }
        }

        match body? {
            Node_::ClassishBody(body) => {
                for element in body.iter().copied() {
                    match element {
                        Node_::TraitUse(names) => {
                            for name in names.iter() {
                                uses.push(self.node_to_ty(*name)?);
                            }
                        }
                        Node_::TypeConstant(constant) => typeconsts.push(constant.clone()),
                        Node_::RequireClause(require) => match require.require_type {
                            Node_::Extends => req_extends.push(self.node_to_ty(require.name)?),
                            Node_::Implements => {
                                req_implements.push(self.node_to_ty(require.name)?)
                            }
                            _ => {}
                        },
                        Node_::Const(const_decl) => consts.push(const_decl.clone()),
                        Node_::Property { decls, is_static } => {
                            for property in decls {
                                if is_static {
                                    sprops.push(property.clone())
                                } else {
                                    props.push(property.clone())
                                }
                            }
                        }
                        Node_::Constructor { method, properties } => {
                            constructor = Some(method.clone());
                            for property in properties {
                                props.push(property.clone())
                            }
                        }
                        Node_::Method { method, is_static } => {
                            if is_static {
                                static_methods.push(method.clone());
                            } else {
                                methods.push(method.clone());
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
        user_attributes.reverse();

        let where_constraints = where_constraints.into_bump_slice();
        let uses = uses.into_bump_slice();
        let method_redeclarations = method_redeclarations.into_bump_slice();
        let xhp_attr_uses = xhp_attr_uses.into_bump_slice();
        let req_extends = req_extends.into_bump_slice();
        let req_implements = req_implements.into_bump_slice();
        let consts = consts.into_bump_slice();
        let typeconsts = typeconsts.into_bump_slice();
        let pu_enums = pu_enums.into_bump_slice();
        let props = props.into_bump_slice();
        let sprops = sprops.into_bump_slice();
        let static_methods = static_methods.into_bump_slice();
        let methods = methods.into_bump_slice();
        let user_attributes = user_attributes.into_bump_slice();

        let extends = self.filter_map_to_slice(extends, |node| {
            if node.is_ignored() {
                Ok(None)
            } else {
                Ok(Some(self.node_to_ty(node)?))
            }
        })?;

        let implements = self.filter_map_to_slice(implements, |node| {
            if node.is_ignored() {
                Ok(None)
            } else {
                Ok(Some(self.node_to_ty(node)?))
            }
        })?;

        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(tparams?)?;

        let cls: shallow_decl_defs::ShallowClass<'a> = shallow_decl_defs::ShallowClass {
            mode: match self.state.file_mode_builder {
                FileModeBuilder::None | FileModeBuilder::Pending => Mode::Mstrict,
                FileModeBuilder::Set(mode) => mode,
            },
            final_,
            is_xhp: false,
            has_xhp_keyword: match xhp_keyword? {
                Node_::XHP => true,
                _ => false,
            },
            kind: class_kind,
            name: Id(pos, name),
            tparams,
            where_constraints,
            extends,
            uses,
            method_redeclarations,
            xhp_attr_uses,
            req_extends,
            req_implements,
            implements,
            consts,
            typeconsts,
            pu_enums,
            props,
            sprops,
            constructor,
            static_methods,
            methods,
            user_attributes,
            enum_type: None,
            // NB: We have no intention of populating this field. Any errors
            // historically emitted during shallow decl should be migrated to a
            // NAST check.
            decl_errors: Errors::empty(),
        };
        Rc::make_mut(&mut self.state.decls)
            .classes
            .insert(name, cls);

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
        declarators: Self::R,
        _arg4: Self::R,
    ) -> Self::R {
        // Sometimes the declarator is a single element list.
        let declarators = match declarators? {
            Node_::List(nodes) => nodes,
            node => return Err(format!("Expected a List, but got {:?}", node)),
        };
        let attrs = attrs?;
        let modifiers = modifiers?;
        let modifiers = read_member_modifiers(modifiers.iter());
        let hint = hint?;
        let mut declarators_vec = Vec::new_in(self.state.arena);
        for declarator in declarators.into_iter() {
            match declarator {
                Node_::ListItem(&(name, initializer)) => {
                    let needs_init = matches!(initializer, Node_::Ignored);
                    let attributes = attrs.as_attributes(self.state.arena)?;
                    let Id(pos, name) = self.get_name("", name)?;
                    let name = if modifiers.is_static {
                        name
                    } else {
                        strip_dollar_prefix(name)
                    };
                    let ty = self.node_to_ty(hint)?;
                    declarators_vec.push(ShallowProp {
                        const_: attributes.const_,
                        xhp_attr: None,
                        lateinit: attributes.late_init,
                        lsb: attributes.lsb,
                        name: Id(pos, name),
                        needs_init,
                        type_: Some(ty),
                        abstract_: modifiers.is_abstract,
                        visibility: modifiers.visibility,
                        fixme_codes: ISet::empty(),
                    });
                }
                n => return Err(format!("Expected a ListItem, but was {:?}", n)),
            }
        }
        let declarators = declarators_vec.into_bump_slice();
        Ok(Node_::Property {
            decls: declarators,
            is_static: modifiers.is_static,
        })
    }

    fn make_property_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        Ok(Node_::ListItem(self.alloc((name?, initializer?))))
    }

    fn make_methodish_declaration(
        &mut self,
        attributes: Self::R,
        header: Self::R,
        body: Self::R,
        closer: Self::R,
    ) -> Self::R {
        match header? {
            Node_::FunctionHeader(header) => {
                let attributes = attributes?;
                let body = match body {
                    // If we don't have a body, use the closing token. A closing
                    // token of '}' indicates a regular function, while a
                    // closing token of ';' indicates an abstract function.
                    Ok(Node_::Ignored) => closer?,
                    Ok(body) => body,
                    Err(_) => Node_::Ignored,
                };
                let modifiers = read_member_modifiers(header.modifiers.iter());
                let is_constructor = match header.name {
                    Node_::Construct(_) => true,
                    _ => false,
                };
                let (id, ty, properties) = self.function_into_ty("", attributes, header, body)?;
                let attributes = attributes.as_attributes(self.state.arena)?;
                let deprecated = attributes.deprecated.map(|msg| {
                    let mut s = String::new_in(self.state.arena);
                    s.push_str("The method ");
                    s.push_str(id.1);
                    s.push_str(" is deprecated: ");
                    s.push_str(msg);
                    s.into_bump_str()
                });
                fn get_condition_type_name<'a>(ty_opt: Option<Ty<'a>>) -> Option<&'a str> {
                    ty_opt.and_then(|ty| {
                        let Ty(_, ty_) = ty;
                        match *ty_ {
                            Ty_::Tapply(&(Id(_, class_name), _)) => Some(class_name),
                            _ => None,
                        }
                    })
                }
                let method = self.alloc(ShallowMethod {
                    abstract_: self.state.classish_name_builder.in_interface()
                        || modifiers.is_abstract,
                    final_: modifiers.is_final,
                    memoizelsb: attributes.memoizelsb,
                    name: id,
                    override_: attributes.override_,
                    reactivity: match attributes.reactivity {
                        Reactivity::Local(condition_type) => Some(MethodReactivity::MethodLocal(
                            get_condition_type_name(condition_type),
                        )),
                        Reactivity::Shallow(condition_type) => {
                            Some(MethodReactivity::MethodShallow(get_condition_type_name(
                                condition_type,
                            )))
                        }
                        Reactivity::Reactive(condition_type) => {
                            Some(MethodReactivity::MethodReactive(get_condition_type_name(
                                condition_type,
                            )))
                        }
                        Reactivity::Nonreactive
                        | Reactivity::MaybeReactive(_)
                        | Reactivity::RxVar(_)
                        | Reactivity::Pure(_) => None,
                    },
                    dynamicallycallable: false,
                    type_: ty,
                    visibility: modifiers.visibility,
                    fixme_codes: ISet::empty(),
                    deprecated,
                });
                if is_constructor {
                    Ok(Node_::Constructor { method, properties })
                } else {
                    Ok(Node_::Method {
                        method,
                        is_static: modifiers.is_static,
                    })
                }
            }
            n => Err(format!("Expected a FunctionDecl header, but was {:?}", n)),
        }
    }

    fn make_classish_body(&mut self, _arg0: Self::R, body: Self::R, _arg2: Self::R) -> Self::R {
        Ok(Node_::ClassishBody(body?.as_slice(self.state.arena)))
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
        let id = self.get_name(self.state.namespace_builder.current_namespace(), name)?;
        let hint = self.node_to_ty(extends?)?;
        let extends = self.node_to_ty(self.make_apply(
            Id(name.get_pos(self.state.arena)?, "\\HH\\BuiltinEnum"),
            name,
            None,
        )?)?;
        let key = id.1;
        let mut consts = Vec::new_in(self.state.arena);
        for node in cases?.iter() {
            match node {
                Node_::ListItem(innards) => {
                    let (name, value) = *innards;
                    consts.push(shallow_decl_defs::ShallowClassConst {
                        abstract_: false,
                        expr: Some(value.as_expr(self.state.arena)?),
                        name: self.get_name("", *name)?,
                        type_: Ty(
                            self.alloc(Reason::witness(value.get_pos(self.state.arena)?)),
                            hint.1,
                        ),
                    })
                }
                n => return Err(format!("Expected an enum case, got {:?}", n)),
            }
        }
        let consts = consts.into_bump_slice();
        let cls = shallow_decl_defs::ShallowClass {
            mode: match self.state.file_mode_builder {
                FileModeBuilder::None | FileModeBuilder::Pending => Mode::Mstrict,
                FileModeBuilder::Set(mode) => mode,
            },
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassKind::Cenum,
            name: id,
            tparams: &[],
            where_constraints: &[],
            extends: bumpalo::vec![in self.state.arena; extends].into_bump_slice(),
            uses: &[],
            method_redeclarations: &[],
            xhp_attr_uses: &[],
            req_extends: &[],
            req_implements: &[],
            implements: &[],
            consts,
            typeconsts: &[],
            pu_enums: &[],
            props: &[],
            sprops: &[],
            constructor: None,
            static_methods: &[],
            methods: &[],
            user_attributes: &[],
            enum_type: Some(EnumType {
                base: hint,
                constraint: None,
            }),
            // NB: We have no intention of populating this field. Any errors
            // historically emitted during shallow decl should be migrated to a
            // NAST check.
            decl_errors: Errors::empty(),
        };
        Rc::make_mut(&mut self.state.decls).classes.insert(key, cls);
        Ok(Node_::Ignored)
    }

    fn make_enumerator(
        &mut self,
        name: Self::R,
        _arg1: Self::R,
        value: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        Ok(Node_::ListItem(self.alloc((name?, value?))))
    }

    fn make_tuple_type_specifier(
        &mut self,
        left_paren: Self::R,
        tys: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        // We don't need to include the tys list in this position merging
        // because by definition it's already contained by the two brackets.
        let pos = Pos::merge(
            self.state.arena,
            left_paren?.get_pos(self.state.arena)?,
            right_paren?.get_pos(self.state.arena)?,
        )?;
        let tys = tys?;
        let tys_iter = tys.iter();
        let mut tys = Vec::new_in(self.state.arena);
        for node in tys_iter {
            tys.push(self.node_to_ty(*node)?);
        }
        let tys = tys.into_bump_slice();
        Ok(self.hint_ty(pos, Ty_::Ttuple(tys)))
    }

    fn make_shape_type_specifier(
        &mut self,
        shape: Self::R,
        _arg1: Self::R,
        fields: Self::R,
        open: Self::R,
        rparen: Self::R,
    ) -> Self::R {
        let mut specifiers = Vec::new_in(self.state.arena);
        for node in fields?.iter() {
            match node {
                Node_::ShapeFieldSpecifier(decl) => specifiers.push(**decl),
                n => return Err(format!("Expected a shape field specifier, but was {:?}", n)),
            }
        }
        let fields = specifiers.into_bump_slice();
        let kind = match open? {
            Node_::DotDotDot => ShapeKind::OpenShape,
            _ => ShapeKind::ClosedShape,
        };
        let fields_iter = fields.iter();
        let mut fields = AssocListMut::new_in(self.state.arena);
        for field_decl in fields_iter {
            let name = match field_decl.name {
                Node_::StringLiteral(s, pos) => ShapeFieldName::SFlitStr((pos, s)),
                n => {
                    return Err(format!(
                        "Expected a string literal for shape key name, but was {:?}",
                        n
                    ))
                }
            };
            fields.insert(
                ShapeField(name),
                ShapeFieldType {
                    optional: field_decl.is_optional,
                    ty: self.node_to_ty(field_decl.type_)?,
                },
            );
        }
        let pos = Pos::merge(
            self.state.arena,
            shape?.get_pos(self.state.arena)?,
            rparen?.get_pos(self.state.arena)?,
        )?;
        Ok(self.hint_ty(pos, Ty_::Tshape(self.alloc((kind, fields.into())))))
    }

    fn make_shape_expression(
        &mut self,
        shape: Self::R,
        _left_paren: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let mut kv_pairs = Vec::new_in(self.state.arena);
        for node in fields?.iter() {
            match node {
                Node_::ListItem((key, value)) => {
                    let key = match key {
                        Node_::IntLiteral(s, p) => ShapeFieldName::SFlitInt((*p, s)),
                        Node_::StringLiteral(s, p) => ShapeFieldName::SFlitStr((*p, s)),
                        n => {
                            return Err(format!(
                            "Expected an int literal, string literal, or class const, but was {:?}",
                            n
                        ))
                        }
                    };
                    let value = value.as_expr(self.state.arena)?;
                    kv_pairs.push((key, value))
                }
                n => return Err(format!("Expected a ListItem but was {:?}", n)),
            }
        }
        let fields = kv_pairs.into_bump_slice();
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                shape?.get_pos(self.state.arena)?,
                right_paren?.get_pos(self.state.arena)?,
            )?,
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
        let mut field_exprs = Vec::new_in(self.state.arena);
        for field in fields?.iter() {
            field_exprs.push(field.as_expr(self.state.arena)?);
        }
        let fields = field_exprs.into_bump_slice();
        Ok(Node_::Expr(self.alloc(aast::Expr(
            Pos::merge(
                self.state.arena,
                tuple?.get_pos(self.state.arena)?,
                right_paren?.get_pos(self.state.arena)?,
            )?,
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
        let id = self.get_name("\\", classname)?;
        match gt {
            Node_::Ignored => Ok(self.prim_ty(aast::Tprim::Tstring, id.0)),
            gt => self.make_apply(
                id,
                targ,
                Some(Pos::merge(
                    self.state.arena,
                    classname.get_pos(self.state.arena)?,
                    gt.get_pos(self.state.arena)?,
                )?),
            ),
        }
    }

    fn make_scope_resolution_expression(
        &mut self,
        class_name: Self::R,
        _arg1: Self::R,
        value: Self::R,
    ) -> Self::R {
        let (class_name, value) = (class_name?, value?);
        let pos = Pos::merge(
            self.state.arena,
            class_name.get_pos(self.state.arena)?,
            value.get_pos(self.state.arena)?,
        )?;
        let Id(class_name_pos, class_name_str) =
            self.get_name(self.state.namespace_builder.current_namespace(), class_name)?;
        let class_id = aast::ClassId(
            class_name_pos,
            match class_name_str.to_ascii_lowercase().as_ref() {
                "\\self" => aast::ClassId_::CIself,
                _ => aast::ClassId_::CI(Id(class_name_pos, class_name_str)),
            },
        );
        let value_id = self.get_name("", value)?;
        Ok(Node_::Expr(self.alloc(aast::Expr(
            pos,
            nast::Expr_::ClassConst(self.alloc((class_id, (value_id.0, value_id.1)))),
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
        Ok(Node_::ShapeFieldSpecifier(self.alloc(ShapeFieldDecl {
            is_optional,
            name: name?,
            type_: type_?,
        })))
    }

    fn make_field_initializer(&mut self, key: Self::R, _arg1: Self::R, value: Self::R) -> Self::R {
        Ok(Node_::ListItem(self.alloc((key?, value?))))
    }

    fn make_varray_type_specifier(
        &mut self,
        varray: Self::R,
        _less_than: Self::R,
        tparam: Self::R,
        _arg3: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        let pos = varray?.get_pos(self.state.arena)?;
        let pos = if let Ok(gt_pos) = greater_than?.get_pos(self.state.arena) {
            Pos::merge(self.state.arena, pos, gt_pos)?
        } else {
            pos
        };
        Ok(self.hint_ty(pos, Ty_::Tvarray(self.node_to_ty(tparam?)?)))
    }

    fn make_vector_array_type_specifier(
        &mut self,
        array: Self::R,
        _less_than: Self::R,
        tparam: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        let pos = array?.get_pos(self.state.arena)?;
        let pos = if let Ok(gt_pos) = greater_than?.get_pos(self.state.arena) {
            Pos::merge(self.state.arena, pos, gt_pos)?
        } else {
            pos
        };
        let key_type = match tparam? {
            Node_::Ignored => None,
            n => Some(self.node_to_ty(n)?),
        };
        Ok(self.hint_ty(pos, Ty_::Tarray(self.alloc((key_type, None)))))
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
        let pos = darray?.get_pos(self.state.arena)?;
        let pos = if let Ok(gt_pos) = greater_than?.get_pos(self.state.arena) {
            Pos::merge(self.state.arena, pos, gt_pos)?
        } else {
            pos
        };
        let key_type = self.node_to_ty(key_type?).unwrap_or(TANY);
        let value_type = self.node_to_ty(value_type?).unwrap_or(TANY);
        Ok(self.hint_ty(pos, Ty_::Tdarray(self.alloc((key_type, value_type)))))
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
        let pos = Pos::merge(
            self.state.arena,
            array?.get_pos(self.state.arena)?,
            greater_than?.get_pos(self.state.arena)?,
        )?;
        let key_type = match key_type? {
            Node_::Ignored => None,
            n => Some(self.node_to_ty(n)?),
        };
        let value_type = match value_type? {
            Node_::Ignored => None,
            n => Some(self.node_to_ty(n)?),
        };
        Ok(self.hint_ty(pos, Ty_::Tarray(self.alloc((key_type, value_type)))))
    }

    fn make_old_attribute_specification(
        &mut self,
        ltlt: Self::R,
        attrs: Self::R,
        gtgt: Self::R,
    ) -> Self::R {
        match attrs? {
            Node_::List(nodes) => Ok(Node_::BracketedList(self.alloc((
                ltlt?.get_pos(self.state.arena)?,
                nodes,
                gtgt?.get_pos(self.state.arena)?,
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
        let id = self.get_name("", name?)?;
        Ok(Node_::Attribute(self.alloc(Attribute {
            id,
            args: args?.as_slice(self.state.arena),
        })))
    }

    fn make_trait_use(&mut self, _arg0: Self::R, used: Self::R, _arg2: Self::R) -> Self::R {
        Ok(Node_::TraitUse(self.alloc(used?)))
    }

    fn make_require_clause(
        &mut self,
        _arg0: Self::R,
        require_type: Self::R,
        name: Self::R,
        _arg3: Self::R,
    ) -> Self::R {
        Ok(Node_::RequireClause(self.alloc(RequireClause {
            require_type: require_type?,
            name: name?,
        })))
    }

    fn make_nullable_type_specifier(&mut self, question_mark: Self::R, hint: Self::R) -> Self::R {
        let hint = hint?;
        let hint_pos = hint.get_pos(self.state.arena)?;
        Ok(self.hint_ty(
            Pos::merge(
                self.state.arena,
                question_mark?.get_pos(self.state.arena)?,
                hint_pos,
            )?,
            Ty_::Toption(self.node_to_ty(hint)?),
        ))
    }

    fn make_like_type_specifier(&mut self, tilde: Self::R, type_: Self::R) -> Self::R {
        let (tilde, type_) = (tilde?, type_?);
        let pos = Pos::merge(
            self.state.arena,
            tilde.get_pos(self.state.arena)?,
            type_.get_pos(self.state.arena)?,
        )?;
        Ok(self.hint_ty(pos, Ty_::Tlike(self.node_to_ty(type_)?)))
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
        let mut params = Vec::new_in(self.state.arena);
        for node in args?.iter().copied() {
            params.push(&*self.alloc(FunParam {
                pos: node.get_pos(self.state.arena)?,
                name: None,
                type_: PossiblyEnforcedTy {
                    enforced: false,
                    type_: self.node_to_ty(node)?,
                },
                flags: FunParamFlags::empty(),
                rx_annotation: None,
            }));
        }
        let params = params.into_bump_slice();
        let ret = self.node_to_ty(ret_hint?)?;
        let pos = Pos::merge(
            self.state.arena,
            left_paren?.get_pos(self.state.arena)?,
            right_paren?.get_pos(self.state.arena)?,
        )?;
        Ok(self.hint_ty(
            pos,
            Ty_::Tfun(self.alloc(FunType {
                arity: FunArity::Fstandard,
                tparams: &[],
                where_constraints: &[],
                params,
                ret: PossiblyEnforcedTy {
                    enforced: false,
                    type_: ret,
                },
                reactive: Reactivity::Nonreactive,
                flags: FunTypeFlags::empty(),
            })),
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
        constraint: Self::R,
        _arg7: Self::R,
        type_: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let attributes = attributes?.as_attributes(self.state.arena)?;
        let has_abstract_keyword = modifiers?.iter().fold(false, |abstract_, node| match node {
            Node_::Abstract => true,
            _ => abstract_,
        });
        let constraint = match constraint? {
            Node_::TypeConstraint(innards) => self.node_to_ty(innards.1).ok(),
            _ => None,
        };
        let type_ = self.node_to_ty(type_?).ok();
        let has_constraint = constraint.is_some();
        let has_type = type_.is_some();
        let (type_, abstract_) = match (has_abstract_keyword, has_constraint, has_type) {
            // Has no assigned type. Technically illegal, so if the constraint
            // is present, proceed as if the constraint was the assigned type.
            //     const type TFoo;
            //     const type TFoo as OtherType;
            (false, _, false) => (constraint, TypeconstAbstractKind::TCConcrete),
            // Has no constraint, but does have an assigned type.
            //     const type TFoo = SomeType;
            (false, false, true) => (type_, TypeconstAbstractKind::TCConcrete),
            // Has both a constraint and an assigned type.
            //     const type TFoo as OtherType = SomeType;
            (false, true, true) => (type_, TypeconstAbstractKind::TCPartiallyAbstract),
            // Has no default type.
            //     abstract const type TFoo;
            //     abstract const type TFoo as OtherType;
            (true, _, false) => (type_, TypeconstAbstractKind::TCAbstract(None)),
            // Has a default type.
            //     abstract const Type TFoo = SomeType;
            //     abstract const Type TFoo as OtherType = SomeType;
            (true, _, true) => (None, TypeconstAbstractKind::TCAbstract(type_)),
        };
        let name = self.get_name("", name?)?;
        Ok(Node_::TypeConstant(self.alloc(ShallowTypeconst {
            abstract_,
            constraint,
            name,
            type_,
            enforceable: match attributes.enforceable {
                Some(pos) => (pos, true),
                None => (Pos::none(), false),
            },
            reifiable: attributes.reifiable,
        })))
    }

    fn make_decorated_expression(&mut self, decorator: Self::R, expr: Self::R) -> Self::R {
        Ok(Node_::ListItem(self.alloc((decorator?, expr?))))
    }

    fn make_type_constant(
        &mut self,
        ty: Self::R,
        coloncolon: Self::R,
        constant_name: Self::R,
    ) -> Self::R {
        let (ty, _coloncolon, constant_name) = (ty?, coloncolon?, constant_name?);
        let id = self.get_name("", constant_name)?;
        let pos = Pos::merge(
            self.state.arena,
            ty.get_pos(self.state.arena)?,
            constant_name.get_pos(self.state.arena)?,
        )?;
        match ty {
            Node_::TypeconstAccess(innards) => {
                innards.0.set(pos);
                // Nested typeconst accesses have to be collapsed.
                innards.2.borrow_mut().push(id);
                Ok(Node_::TypeconstAccess(innards))
            }
            ty => {
                let ty = match ty {
                    Node_::Name("self", self_pos) => {
                        match self.state.classish_name_builder.get_current_classish_name() {
                            Some((name, class_name_pos)) => {
                                // In classes, we modify the position when
                                // rewriting the `self` keyword to point to the
                                // class name. In traits, we don't (because
                                // traits are not types). We indicate that the
                                // position shouldn't be rewritten with the none
                                // Pos.
                                let id_pos = if class_name_pos.is_none() {
                                    self_pos
                                } else {
                                    class_name_pos
                                };
                                let reason = self.alloc(Reason::hint(self_pos));
                                let ty_ = Ty_::Tapply(self.alloc((Id(id_pos, name), &[][..])));
                                Ty(reason, self.alloc(ty_))
                            }
                            None => self.node_to_ty(ty.clone())?,
                        }
                    }
                    _ => self.node_to_ty(ty.clone())?,
                };
                Ok(Node_::TypeconstAccess(self.alloc((
                    Cell::new(pos),
                    ty,
                    RefCell::new(bumpalo::vec![in self.state.arena; id]),
                ))))
            }
        }
    }

    fn make_soft_type_specifier(&mut self, at_token: Self::R, hint: Self::R) -> Self::R {
        let hint = hint?;
        let hint_pos = hint.get_pos(self.state.arena)?;
        // Use the type of the hint as-is (i.e., throw away the knowledge that
        // we had a soft type specifier here--the typechecker does not use it).
        // Replace its Reason with one including the position of the `@` token.
        Ok(self.hint_ty(
            Pos::merge(
                self.state.arena,
                at_token?.get_pos(self.state.arena)?,
                hint_pos,
            )?,
            *self.node_to_ty(hint)?.1,
        ))
    }

    // A type specifier preceded by an attribute list. At the time of writing,
    // only the <<__Soft>> attribute is permitted here.
    fn make_attributized_specifier(&mut self, attributes: Self::R, hint: Self::R) -> Self::R {
        match attributes? {
            Node_::BracketedList((
                ltlt_pos,
                [Node_::Attribute(Attribute {
                    id: Id(_, "__Soft"),
                    ..
                })],
                gtgt_pos,
            )) => {
                let attributes_pos = Pos::merge(self.state.arena, *ltlt_pos, *gtgt_pos)?;
                let hint = hint?;
                let hint_pos = hint.get_pos(self.state.arena)?;
                // Use the type of the hint as-is (i.e., throw away the
                // knowledge that we had a soft type specifier here--the
                // typechecker does not use it). Replace its Reason with one
                // including the position of the attribute list.
                return Ok(self.hint_ty(
                    Pos::merge(self.state.arena, attributes_pos, hint_pos)?,
                    *self.node_to_ty(hint)?.1,
                ));
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
        self.make_apply(
            self.get_name("", vec?)?,
            hint?,
            greater_than?.get_pos(self.state.arena).ok(),
        )
    }

    fn make_dictionary_type_specifier(
        &mut self,
        dict: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        self.make_apply(
            self.get_name("", dict?)?,
            hint?,
            greater_than?.get_pos(self.state.arena).ok(),
        )
    }

    fn make_keyset_type_specifier(
        &mut self,
        keyset: Self::R,
        _arg1: Self::R,
        hint: Self::R,
        _arg3: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        self.make_apply(
            self.get_name("", keyset?)?,
            hint?,
            greater_than?.get_pos(self.state.arena).ok(),
        )
    }
}

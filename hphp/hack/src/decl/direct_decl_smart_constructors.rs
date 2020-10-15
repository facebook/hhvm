// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::{Cell, RefCell};
use std::rc::Rc;

use bstr::BStr;
use bumpalo::{
    collections::{String, Vec},
    Bump,
};

use hh_autoimport_rust as hh_autoimport;
use naming_special_names_rust as naming_special_names;

use arena_collections::{AssocListMut, List, MultiSetMut};
use flatten_smart_constructors::{FlattenOp, FlattenSmartConstructors};
use oxidized_by_ref::{
    aast, aast_defs,
    ast_defs::{Bop, ClassKind, ConstraintKind, FunKind, Id, ShapeFieldName, Uop, Variance},
    decl_defs::MethodReactivity,
    file_info::Mode,
    nast,
    pos::Pos,
    relative_path::RelativePath,
    shallow_decl_defs::{self, ShallowClassConst, ShallowMethod, ShallowProp, ShallowTypeconst},
    shape_map::ShapeField,
    typing_defs,
    typing_defs::{
        ConstDecl, EnumType, FunArity, FunElt, FunImplicitParams, FunParam, FunParams, FunType,
        ParamMode, ParamMutability, PossiblyEnforcedTy, Reactivity, ShapeFieldType, ShapeKind,
        Tparam, Ty, Ty_, TypeconstAbstractKind, TypedefType, WhereConstraint, XhpAttrTag,
    },
    typing_defs_flags::{FunParamFlags, FunTypeFlags},
    typing_reason::Reason,
};
use parser_core_types::{
    compact_token::CompactToken, indexed_source_text::IndexedSourceText, source_text::SourceText,
    syntax_kind::SyntaxKind, token_factory::SimpleTokenFactoryImpl, token_kind::TokenKind,
};

mod direct_decl_smart_constructors_generated;

pub use direct_decl_smart_constructors_generated::DirectDeclSmartConstructors;

type SK = SyntaxKind;

type SSet<'a> = arena_collections::SortedSet<'a, &'a str>;

impl<'a> DirectDeclSmartConstructors<'a> {
    pub fn new(src: &SourceText<'a>, file_mode: Mode, arena: &'a Bump) -> Self {
        Self {
            state: State::new(IndexedSourceText::new(src.clone()), file_mode, arena),
            token_factory: SimpleTokenFactoryImpl::new(),
        }
    }

    #[inline(always)]
    pub fn alloc<T>(&self, val: T) -> &'a T {
        self.state.arena.alloc(val)
    }

    fn qualified_name_from_parts(
        &self,
        namespace: &'a str,
        parts: &'a [Node<'a>],
        pos: &'a Pos<'a>,
    ) -> Id<'a> {
        // If the name is already fully qualified, don't prepend a namespace.
        let fully_qualified = matches!(parts.first(), Some(Node::Backslash(_)));
        let namespace = if fully_qualified { "" } else { namespace };
        // Count the length of the qualified name, so that we can allocate
        // exactly the right amount of space for it in our arena.
        let mut len = namespace.len();
        for part in parts {
            match part {
                Node::Name(&(name, _)) => len += name.len(),
                Node::Backslash(_) => len += 1,
                Node::ListItem(&(Node::Name(&(name, _)), Node::Backslash(_))) => {
                    len += name.len() + 1
                }
                _ => {}
            }
        }
        // If the name is fully qualified, and there's no internal trivia, then
        // we can just reference the fully qualified name in the original source
        // text instead of copying it.
        let source_len = pos.end_cnum() - pos.start_cnum();
        if fully_qualified && source_len == len {
            let qualified_name = self.str_from_utf8(self.source_text_at_pos(pos));
            return Id(pos, qualified_name);
        }
        // Allocate `len` bytes and fill them with the fully qualified name.
        let mut qualified_name = String::with_capacity_in(len, self.state.arena);
        qualified_name.push_str(namespace);
        for part in parts {
            match part {
                Node::Name(&(name, _pos)) => qualified_name.push_str(&name),
                Node::Backslash(_) => qualified_name.push('\\'),
                Node::ListItem(listitem) => {
                    if let (Node::Name(&(name, _)), Node::Backslash(_)) = &**listitem {
                        qualified_name.push_str(&name);
                        qualified_name.push_str("\\");
                    } else {
                        panic!("Expected a name or backslash, but got {:?}", listitem);
                    }
                }
                n => {
                    panic!("Expected a name, backslash, or list item, but got {:?}", n);
                }
            }
        }
        debug_assert_eq!(len, qualified_name.len());
        debug_assert_eq!(len, qualified_name.capacity());
        Id(pos, qualified_name.into_bump_str())
    }

    /// If the given node is a name (i.e., an identifier or a qualified name),
    /// elaborate it in the current namespace and return Some.
    fn elaborate_name(&self, name: Node<'a>) -> Option<Id<'a>> {
        match name {
            Node::Name(&(name, pos)) => Some(Id(pos, self.prefix_ns(name))),
            Node::XhpName(&(name, pos)) => Some(Id(pos, name)),
            Node::QualifiedName(&(parts, pos)) => {
                let namespace = self.state.namespace_builder.current_namespace();
                Some(self.qualified_name_from_parts(namespace, parts, pos))
            }
            _ => None,
        }
    }

    /// If the given node is a name (i.e., an identifier or a qualified name),
    /// return Some. No namespace elaboration is performed.
    fn expect_name(&self, name: Node<'a>) -> Option<Id<'a>> {
        // If it's a simple identifier, return it.
        if let id @ Some(_) = name.as_id() {
            return id;
        }
        match name {
            Node::QualifiedName(&(parts, pos)) => {
                Some(self.qualified_name_from_parts("", parts, pos))
            }
            _ => None,
        }
    }

    fn slice<T>(&self, iter: impl Iterator<Item = T>) -> &'a [T] {
        let mut result = match iter.size_hint().1 {
            Some(upper_bound) => Vec::with_capacity_in(upper_bound, self.state.arena),
            None => Vec::new_in(self.state.arena),
        };
        for item in iter {
            result.push(item);
        }
        result.into_bump_slice()
    }

    fn unwrap_mutability(hint: Node<'a>) -> (Node<'a>, Option<ParamMutability>) {
        match hint {
            Node::Ty(Ty(_, Ty_::Tapply((hn, [t])))) if hn.1 == "\\Mutable" => {
                (Node::Ty(t), Some(ParamMutability::ParamBorrowedMutable))
            }
            Node::Ty(Ty(_, Ty_::Tapply((hn, [t])))) if hn.1 == "\\OwnedMutable" => {
                (Node::Ty(t), Some(ParamMutability::ParamOwnedMutable))
            }
            Node::Ty(Ty(_, Ty_::Tapply((hn, [t])))) if hn.1 == "\\MaybeMutable" => {
                (Node::Ty(t), Some(ParamMutability::ParamMaybeMutable))
            }
            _ => (hint, None),
        }
    }
}

#[derive(Copy, Clone, Debug)]
pub struct InProgressDecls<'a> {
    pub classes: List<'a, (&'a str, &'a shallow_decl_defs::ShallowClass<'a>)>,
    pub funs: List<'a, (&'a str, &'a typing_defs::FunElt<'a>)>,
    pub typedefs: List<'a, (&'a str, &'a typing_defs::TypedefType<'a>)>,
    pub consts: List<'a, (&'a str, &'a typing_defs::ConstDecl<'a>)>,
}

pub fn empty_decls() -> InProgressDecls<'static> {
    InProgressDecls {
        classes: List::empty(),
        funs: List::empty(),
        typedefs: List::empty(),
        consts: List::empty(),
    }
}

fn prefix_slash<'a>(arena: &'a Bump, name: &str) -> &'a str {
    let mut s = String::with_capacity_in(1 + name.len(), arena);
    s.push('\\');
    s.push_str(name);
    s.into_bump_str()
}

fn prefix_colon<'a>(arena: &'a Bump, name: &str) -> &'a str {
    let mut s = String::with_capacity_in(1 + name.len(), arena);
    s.push(':');
    s.push_str(name);
    s.into_bump_str()
}

fn concat<'a>(arena: &'a Bump, str1: &str, str2: &str) -> &'a str {
    let mut result = String::with_capacity_in(str1.len() + str2.len(), arena);
    result.push_str(str1);
    result.push_str(str2);
    result.into_bump_str()
}

fn str_from_utf8<'a>(arena: &'a Bump, slice: &'a [u8]) -> &'a str {
    if let Ok(s) = std::str::from_utf8(slice) {
        s
    } else {
        String::from_utf8_lossy_in(slice, arena).into_bump_str()
    }
}

fn strip_dollar_prefix<'a>(name: &'a str) -> &'a str {
    name.trim_start_matches("$")
}

const TANY_: Ty_<'_> = Ty_::Tany(oxidized_by_ref::tany_sentinel::TanySentinel);
const TANY: &Ty<'_> = &Ty(Reason::none(), TANY_);

fn tany() -> &'static Ty<'static> {
    TANY
}

fn tarraykey<'a>(arena: &'a Bump) -> &'a Ty<'a> {
    arena.alloc(Ty(
        Reason::none(),
        Ty_::Tprim(arena.alloc(aast::Tprim::Tarraykey)),
    ))
}

fn default_capability<'a>(arena: &'a Bump, r: Reason<'a>) -> &'a Ty<'a> {
    arena.alloc(Ty(arena.alloc(r), Ty_::Tunion(&[])))
}

#[derive(Debug)]
struct Modifiers {
    is_static: bool,
    visibility: aast::Visibility,
    is_abstract: bool,
    is_final: bool,
}

fn read_member_modifiers<'a: 'b, 'b>(modifiers: impl Iterator<Item = &'b Node<'a>>) -> Modifiers {
    let mut ret = Modifiers {
        is_static: false,
        visibility: aast::Visibility::Public,
        is_abstract: false,
        is_final: false,
    };
    for modifier in modifiers {
        if let Some(vis) = modifier.as_visibility() {
            ret.visibility = vis;
        }
        match modifier {
            Node::Token(TokenKind::Static) => ret.is_static = true,
            Node::Token(TokenKind::Abstract) => ret.is_abstract = true,
            Node::Token(TokenKind::Final) => ret.is_final = true,
            _ => {}
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

    fn push_namespace(&mut self, name: Option<&str>) {
        let current = self.current_namespace();
        if let Some(name) = name {
            let mut fully_qualified =
                String::with_capacity_in(current.len() + name.len() + 1, self.arena);
            fully_qualified.push_str(current);
            fully_qualified.push_str(name);
            fully_qualified.push('\\');
            self.stack.push(NamespaceInfo {
                name: fully_qualified.into_bump_str(),
                imports: AssocListMut::new_in(self.arena),
            });
        } else {
            self.stack.push(NamespaceInfo {
                name: current,
                imports: AssocListMut::new_in(self.arena),
            });
        }
    }

    fn pop_namespace(&mut self) {
        // We'll never push a namespace for a declaration of items in the global
        // namespace (e.g., `namespace { ... }`), so only pop if we are in some
        // namespace other than the global one.
        if self.stack.len() > 1 {
            self.stack.pop().unwrap();
        }
    }

    // push_namespace(Y) + pop_namespace() + push_namespace(X) should be equivalent to
    // push_namespace(Y) + push_namespace(X) + pop_previous_namespace()
    fn pop_previous_namespace(&mut self) {
        if self.stack.len() > 2 {
            let last = self.stack.pop().unwrap().name;
            let previous = self.stack.pop().unwrap().name;
            // should always be the case, but just to be safe
            if last.starts_with(previous) && last.ends_with("\\") {
                self.push_namespace(Some(&last[previous.len()..last.len() - 1]))
            }
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
        let aliased_name = aliased_name.unwrap_or_else(|| {
            name.rsplit_terminator('\\')
                .nth(0)
                .expect("Expected at least one entry in import name")
        });
        let name = if name.starts_with('\\') {
            name
        } else {
            prefix_slash(self.arena, name)
        };
        imports.insert(aliased_name, name);
    }

    fn rename_import(&self, name: &'a str) -> &'a str {
        if name.starts_with('\\') {
            return name;
        }
        for ni in self.stack.iter().rev() {
            if let Some(name) = ni.imports.get(name) {
                return name;
            }
        }
        if let Some(renamed) = hh_autoimport::TYPES_MAP.get(name) {
            return prefix_slash(self.arena, renamed);
        }
        for ns in hh_autoimport::NAMESPACES {
            if name.starts_with(ns) {
                let ns_trimmed = &name[ns.len()..];
                if ns_trimmed.starts_with('\\') {
                    return concat(self.arena, "\\HH\\", name);
                }
            }
        }
        name
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
            InClassish(_) => {}
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
pub struct State<'a> {
    pub source_text: IndexedSourceText<'a>,
    pub arena: &'a bumpalo::Bump,
    pub decls: InProgressDecls<'a>,
    filename: &'a RelativePath<'a>,
    file_mode: Mode,
    namespace_builder: Rc<NamespaceBuilder<'a>>,
    classish_name_builder: ClassishNameBuilder<'a>,
    type_parameters: Rc<Vec<'a, SSet<'a>>>,

    previous_token_kind: TokenKind,
}

impl<'a> State<'a> {
    pub fn new(source_text: IndexedSourceText<'a>, file_mode: Mode, arena: &'a Bump) -> State<'a> {
        let path = source_text.source_text().file_path();
        let prefix = path.prefix();
        let path = String::from_str_in(path.path_str(), arena).into_bump_str();
        let filename = RelativePath::make(prefix, path);
        State {
            source_text,
            arena,
            filename: arena.alloc(filename),
            file_mode,
            decls: empty_decls(),
            namespace_builder: Rc::new(NamespaceBuilder::new_in(arena)),
            classish_name_builder: ClassishNameBuilder::new(),
            type_parameters: Rc::new(Vec::new_in(arena)),
            // EndOfFile is used here as a None value (signifying "beginning of
            // file") to save space. There is no legitimate circumstance where
            // we would parse a token and the previous token kind would be
            // EndOfFile.
            previous_token_kind: TokenKind::EndOfFile,
        }
    }
}

#[derive(Clone, Debug)]
pub struct FunParamDecl<'a> {
    attributes: Node<'a>,
    visibility: Node<'a>,
    kind: ParamMode,
    hint: Node<'a>,
    id: Id<'a>,
    variadic: bool,
    initializer: Node<'a>,
}

#[derive(Clone, Debug)]
pub struct FunctionHeader<'a> {
    name: Node<'a>,
    modifiers: Node<'a>,
    type_params: Node<'a>,
    param_list: Node<'a>,
    capability: Node<'a>,
    ret_hint: Node<'a>,
}

#[derive(Clone, Debug)]
pub struct RequireClause<'a> {
    require_type: Node<'a>,
    name: Node<'a>,
}

#[derive(Clone, Debug)]
pub struct TypeParameterDecl<'a> {
    name: Node<'a>,
    reified: aast::ReifyKind,
    variance: Variance,
    constraints: &'a [(ConstraintKind, Node<'a>)],
    tparam_params: &'a [&'a Tparam<'a>],
    user_attributes: &'a [&'a UserAttributeNode<'a>],
}

#[derive(Clone, Debug)]
pub struct ClosureTypeHint<'a> {
    args: Node<'a>,
    ret_hint: Node<'a>,
}

#[derive(Clone, Debug)]
pub struct NamespaceUseClause<'a> {
    id: Id<'a>,
    as_: Option<&'a str>,
}

#[derive(Clone, Debug)]
pub struct ConstructorNode<'a> {
    method: &'a ShallowMethod<'a>,
    properties: &'a [ShallowProp<'a>],
}

#[derive(Clone, Debug)]
pub struct MethodNode<'a> {
    method: &'a ShallowMethod<'a>,
    is_static: bool,
}

#[derive(Clone, Debug)]
pub struct PropertyNode<'a> {
    decls: &'a [ShallowProp<'a>],
    is_static: bool,
}

#[derive(Clone, Debug)]
pub struct XhpClassAttributeDeclarationNode<'a> {
    xhp_attr_decls: &'a [ShallowProp<'a>],
    xhp_attr_uses_decls: &'a [Node<'a>],
}

#[derive(Clone, Debug)]
pub struct XhpClassAttributeNode<'a> {
    name: Id<'a>,
    tag: Option<XhpAttrTag>,
    needs_init: bool,
    nullable: bool,
    hint: Node<'a>,
}

#[derive(Clone, Debug)]
pub struct ShapeFieldNode<'a> {
    name: &'a ShapeField<'a>,
    type_: &'a ShapeFieldType<'a>,
}

#[derive(Clone, Debug)]
pub struct UserAttributeNode<'a> {
    name: Id<'a>,
    classname_params: &'a [Id<'a>],
    string_literal_params: &'a [&'a BStr], // this is only used for __Deprecated attribute message and Cipp parameters
}

#[derive(Copy, Clone, Debug)]
pub enum Node<'a> {
    // Nodes which are not useful in constructing a decl are ignored. We keep
    // track of the SyntaxKind for two reasons.
    //
    // One is that the parser needs to know the SyntaxKind of a parsed node in
    // some circumstances (this information is exposed to the parser via an
    // implementation of `smart_constructors::NodeType`). An adapter called
    // WithKind exists to provide a `NodeType` implementation for arbitrary
    // nodes by pairing each node with a SyntaxKind, but in the direct decl
    // parser, we want to avoid the extra 8 bytes of overhead on each node.
    //
    // The second reason is that debugging is difficult when nodes are silently
    // ignored, and providing at least the SyntaxKind of an ignored node helps
    // in tracking down the reason it was ignored.
    Ignored(SyntaxKind),

    List(&'a &'a [Node<'a>]),
    BracketedList(&'a (&'a Pos<'a>, &'a [Node<'a>], &'a Pos<'a>)),
    Name(&'a (&'a str, &'a Pos<'a>)),
    XhpName(&'a (&'a str, &'a Pos<'a>)),
    QualifiedName(&'a (&'a [Node<'a>], &'a Pos<'a>)),
    Array(&'a Pos<'a>),
    Darray(&'a Pos<'a>),
    Varray(&'a Pos<'a>),
    StringLiteral(&'a (&'a BStr, &'a Pos<'a>)), // For shape keys and const expressions.
    IntLiteral(&'a (&'a str, &'a Pos<'a>)),     // For const expressions.
    FloatingLiteral(&'a (&'a str, &'a Pos<'a>)), // For const expressions.
    BooleanLiteral(&'a (&'a str, &'a Pos<'a>)), // For const expressions.
    Null(&'a Pos<'a>),                          // For const expressions.
    Ty(&'a Ty<'a>),
    TypeconstAccess(&'a (Cell<&'a Pos<'a>>, &'a Ty<'a>, RefCell<Vec<'a, Id<'a>>>)),
    Backslash(&'a Pos<'a>), // This needs a pos since it shows up in names.
    ListItem(&'a (Node<'a>, Node<'a>)),
    Const(&'a ShallowClassConst<'a>),
    ConstInitializer(&'a (Node<'a>, Node<'a>)), // Name, initializer expression
    FunParam(&'a FunParamDecl<'a>),
    Attribute(&'a UserAttributeNode<'a>),
    FunctionHeader(&'a FunctionHeader<'a>),
    Constructor(&'a ConstructorNode<'a>),
    Method(&'a MethodNode<'a>),
    Property(&'a PropertyNode<'a>),
    TraitUse(&'a Node<'a>),
    XhpClassAttributeDeclaration(&'a XhpClassAttributeDeclarationNode<'a>),
    XhpClassAttribute(&'a XhpClassAttributeNode<'a>),
    XhpAttributeUse(&'a Node<'a>),
    XhpEnumType(&'a Node<'a>),
    TypeConstant(&'a ShallowTypeconst<'a>),
    RequireClause(&'a RequireClause<'a>),
    ClassishBody(&'a &'a [Node<'a>]),
    TypeParameter(&'a TypeParameterDecl<'a>),
    TypeConstraint(&'a (ConstraintKind, Node<'a>)),
    ShapeFieldSpecifier(&'a ShapeFieldNode<'a>),
    NamespaceUseClause(&'a NamespaceUseClause<'a>),
    Expr(&'a nast::Expr<'a>),
    Operator(&'a (&'a Pos<'a>, TokenKind)),
    Construct(&'a Pos<'a>),
    This(&'a Pos<'a>), // This needs a pos since it shows up in Taccess.
    TypeParameters(&'a &'a [&'a Tparam<'a>]),
    WhereConstraint(&'a WhereConstraint<'a>),

    // For cases where the position of a node is included in some outer
    // position, but we do not need to track any further information about that
    // node (for instance, the parentheses surrounding a tuple type).
    Pos(&'a Pos<'a>),

    // Simple keywords and tokens.
    Token(TokenKind),
}

impl<'a> smart_constructors::NodeType for Node<'a> {
    type R = Node<'a>;

    fn extract(self) -> Self::R {
        self
    }

    fn is_abstract(&self) -> bool {
        matches!(self, Node::Token(TokenKind::Abstract))
    }
    fn is_name(&self) -> bool {
        matches!(self, Node::Name(..))
    }
    fn is_qualified_name(&self) -> bool {
        matches!(self, Node::QualifiedName(..))
    }
    fn is_prefix_unary_expression(&self) -> bool {
        matches!(self, Node::Expr(aast::Expr(_, aast::Expr_::Unop(..))))
    }
    fn is_scope_resolution_expression(&self) -> bool {
        matches!(self, Node::Expr(aast::Expr(_, aast::Expr_::ClassConst(..))))
    }
    fn is_missing(&self) -> bool {
        matches!(self, Node::Ignored(SK::Missing))
    }
    fn is_variable_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::VariableExpression))
    }
    fn is_subscript_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::SubscriptExpression))
    }
    fn is_member_selection_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::MemberSelectionExpression))
    }
    fn is_object_creation_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::ObjectCreationExpression))
    }
    fn is_safe_member_selection_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::SafeMemberSelectionExpression))
    }
    fn is_function_call_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::FunctionCallExpression))
    }
    fn is_list_expression(&self) -> bool {
        matches!(self, Node::Ignored(SK::ListExpression))
    }
}

impl<'a> Node<'a> {
    fn as_slice(self, b: &'a Bump) -> &'a [Self] {
        match self {
            Node::List(&items) | Node::BracketedList(&(_, items, _)) => items,
            n if n.is_ignored() => &[],
            n => std::slice::from_ref(b.alloc(n)),
        }
    }

    fn iter<'b>(&'b self) -> NodeIterHelper<'a, 'b>
    where
        'a: 'b,
    {
        match self {
            &Node::List(&items) | Node::BracketedList(&(_, items, _)) => {
                NodeIterHelper::Vec(items.iter())
            }
            n if n.is_ignored() => NodeIterHelper::Empty,
            n => NodeIterHelper::Single(n),
        }
    }

    // The number of elements which would be yielded by `self.iter()`.
    // Must return the upper bound returned by NodeIterHelper::size_hint.
    fn len(&self) -> usize {
        match self {
            &Node::List(&items) | Node::BracketedList(&(_, items, _)) => items.len(),
            n if n.is_ignored() => 0,
            _ => 1,
        }
    }

    fn as_visibility(&self) -> Option<aast::Visibility> {
        match self {
            Node::Token(TokenKind::Private) => Some(aast::Visibility::Private),
            Node::Token(TokenKind::Protected) => Some(aast::Visibility::Protected),
            Node::Token(TokenKind::Public) => Some(aast::Visibility::Public),
            _ => None,
        }
    }

    // If this node is a simple unqualified identifier, return its position and text.
    fn as_id(&self) -> Option<Id<'a>> {
        match self {
            Node::Name(&(name, pos)) | Node::XhpName(&(name, pos)) => Some(Id(pos, name)),
            _ => None,
        }
    }

    fn is_ignored(&self) -> bool {
        matches!(self, Node::Ignored(..))
    }

    fn is_present(&self) -> bool {
        !self.is_ignored()
    }
}

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
    returns_void_to_rx: bool,
    accept_disposable: bool,
    dynamically_callable: bool,
    returns_disposable: bool,
    php_std_lib: bool,
}

impl<'a> DirectDeclSmartConstructors<'a> {
    fn add_class(&mut self, name: &'a str, decl: &'a shallow_decl_defs::ShallowClass<'a>) {
        self.state.decls.classes =
            List::cons((name, decl), self.state.decls.classes, self.state.arena);
    }
    fn add_fun(&mut self, name: &'a str, decl: &'a typing_defs::FunElt<'a>) {
        self.state.decls.funs = List::cons((name, decl), self.state.decls.funs, self.state.arena);
    }
    fn add_typedef(&mut self, name: &'a str, decl: &'a typing_defs::TypedefType<'a>) {
        self.state.decls.typedefs =
            List::cons((name, decl), self.state.decls.typedefs, self.state.arena);
    }
    fn add_const(&mut self, name: &'a str, decl: &'a typing_defs::ConstDecl<'a>) {
        self.state.decls.consts =
            List::cons((name, decl), self.state.decls.consts, self.state.arena);
    }

    #[inline(always)]
    fn concat(&self, str1: &str, str2: &str) -> &'a str {
        concat(self.state.arena, str1, str2)
    }

    fn token_bytes(&self, token: &CompactToken) -> &'a [u8] {
        self.state
            .source_text
            .source_text()
            .sub(token.start_offset(), token.width())
    }

    // Check that the slice is valid UTF-8. If it is, return a &str referencing
    // the same data. Otherwise, copy the slice into our arena using
    // String::from_utf8_lossy_in, and return a reference to the arena str.
    fn str_from_utf8(&self, slice: &'a [u8]) -> &'a str {
        str_from_utf8(self.state.arena, slice)
    }

    fn merge(
        &self,
        pos1: impl Into<Option<&'a Pos<'a>>>,
        pos2: impl Into<Option<&'a Pos<'a>>>,
    ) -> &'a Pos<'a> {
        match (pos1.into(), pos2.into()) {
            (None, None) => Pos::none(),
            (Some(pos), None) | (None, Some(pos)) => pos,
            (Some(pos1), Some(pos2)) => match (pos1.is_none(), pos2.is_none()) {
                (true, true) => Pos::none(),
                (true, false) => pos2,
                (false, true) => pos1,
                (false, false) => Pos::merge(self.state.arena, pos1, pos2)
                    .ok()
                    .unwrap_or(Pos::none()),
            },
        }
    }

    fn merge_positions(&self, node1: Node<'a>, node2: Node<'a>) -> &'a Pos<'a> {
        self.merge(self.get_pos(node1), self.get_pos(node2))
    }

    fn pos_from_slice(&self, nodes: &'a [Node<'a>]) -> &'a Pos<'a> {
        nodes.iter().fold(Pos::none(), |acc, &node| {
            self.merge(acc, self.get_pos(node))
        })
    }

    fn get_pos(&self, node: Node<'a>) -> &'a Pos<'a> {
        match node {
            Node::Name(&(_, pos)) => pos,
            Node::Ty(ty) => ty.get_pos().unwrap_or(Pos::none()),
            Node::TypeconstAccess((pos, _, _)) => pos.get(),
            Node::XhpName(&(_, pos)) => pos,
            Node::QualifiedName(&(_, pos)) => pos,
            Node::Pos(pos)
            | Node::Backslash(pos)
            | Node::Construct(pos)
            | Node::This(pos)
            | Node::Array(pos)
            | Node::Darray(pos)
            | Node::Varray(pos)
            | Node::IntLiteral(&(_, pos))
            | Node::FloatingLiteral(&(_, pos))
            | Node::Null(pos)
            | Node::StringLiteral(&(_, pos))
            | Node::BooleanLiteral(&(_, pos))
            | Node::Operator(&(pos, _)) => pos,
            Node::ListItem(&(fst, snd)) => self.merge_positions(fst, snd),
            Node::List(items) => self.pos_from_slice(&items),
            Node::BracketedList(&(first_pos, inner_list, second_pos)) => self.merge(
                first_pos,
                self.merge(self.pos_from_slice(inner_list), second_pos),
            ),
            Node::Expr(&aast::Expr(pos, _)) => pos,
            _ => Pos::none(),
        }
    }

    fn node_to_expr(&self, node: Node<'a>) -> Option<&'a nast::Expr<'a>> {
        let expr_ = match node {
            Node::Expr(expr) => return Some(expr),
            Node::IntLiteral(&(s, _)) => aast::Expr_::Int(s),
            Node::FloatingLiteral(&(s, _)) => aast::Expr_::Float(s),
            Node::StringLiteral(&(s, _)) => aast::Expr_::String(s),
            Node::BooleanLiteral((s, _)) => {
                if s.eq_ignore_ascii_case("true") {
                    aast::Expr_::True
                } else {
                    aast::Expr_::False
                }
            }
            Node::Null(_) => aast::Expr_::Null,
            Node::Name(..) | Node::QualifiedName(..) => {
                aast::Expr_::Id(self.alloc(self.elaborate_name(node)?))
            }
            _ => return None,
        };
        let pos = self.get_pos(node);
        Some(self.alloc(aast::Expr(pos, expr_)))
    }

    fn node_to_ty(&self, node: Node<'a>) -> Option<&'a Ty<'a>> {
        match node {
            Node::Ty(ty) => Some(ty),
            Node::TypeconstAccess((pos, ty, names)) => {
                let pos = pos.get();
                let names = self.slice(names.borrow().iter().copied());
                Some(self.alloc(Ty(
                    self.alloc(Reason::hint(pos)),
                    Ty_::Taccess(self.alloc(typing_defs::TaccessType(*ty, names))),
                )))
            }
            Node::Varray(pos) => {
                Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tvarray(tany()))))
            }
            Node::Darray(pos) => Some(self.alloc(Ty(
                self.alloc(Reason::hint(pos)),
                Ty_::Tdarray(self.alloc((tany(), tany()))),
            ))),
            Node::This(pos) => Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tthis))),
            Node::Expr(expr) => {
                fn expr_to_ty<'a>(arena: &'a Bump, expr: &'a nast::Expr<'a>) -> Option<Ty_<'a>> {
                    use aast::Expr_::*;
                    match expr.1 {
                        Null => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tnull))),
                        This => Some(Ty_::Tthis),
                        True | False => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tbool))),
                        Int(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tint))),
                        Float(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tfloat))),
                        String(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        String2(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        PrefixedString(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        Unop(&(_op, expr)) => expr_to_ty(arena, expr),
                        ParenthesizedExpr(expr) => expr_to_ty(arena, expr),
                        Any => Some(TANY_),

                        ArrayGet(_) | As(_) | Assert(_) | Await(_) | Binop(_) | BracedExpr(_)
                        | Call(_) | Callconv(_) | Cast(_) | ClassConst(_) | ClassGet(_)
                        | Clone(_) | Collection(_) | Darray(_) | Dollardollar(_) | Efun(_)
                        | Eif(_) | ETSplice(_) | ExpressionTree(_) | ExprList(_)
                        | FunctionPointer(_) | FunId(_) | Id(_) | Import(_) | Is(_)
                        | KeyValCollection(_) | Lfun(_) | List(_) | Lplaceholder(_) | Lvar(_)
                        | MethodCaller(_) | MethodId(_) | New(_) | ObjGet(_) | Omitted
                        | Pair(_) | Pipe(_) | PUAtom(_) | PUIdentifier(_) | Record(_)
                        | Shape(_) | SmethodId(_) | Suspend(_) | ValCollection(_) | Varray(_)
                        | Xml(_) | Yield(_) | YieldBreak => None,
                    }
                }

                Some(self.alloc(Ty(
                    self.alloc(Reason::witness(expr.0)),
                    expr_to_ty(self.state.arena, expr)?,
                )))
            }
            Node::IntLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tint)),
            ))),
            Node::FloatingLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tfloat)),
            ))),
            Node::StringLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tstring)),
            ))),
            Node::BooleanLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tbool)),
            ))),
            Node::Null(pos) => Some(self.alloc(Ty(
                self.alloc(Reason::hint(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tnull)),
            ))),
            Node::XhpEnumType(enum_values) => {
                enum_values.iter().next().map(|x| self.node_to_ty(*x))?
            }
            node => {
                let Id(pos, name) = self.expect_name(node)?;
                let reason = self.alloc(Reason::hint(pos));
                let ty_ = if self.is_type_param_in_scope(name) {
                    // TODO (T69662957) must fill type args of Tgeneric
                    Ty_::Tgeneric(self.alloc((name, &[])))
                } else {
                    match name.as_ref() {
                        "nothing" => Ty_::Tunion(&[]),
                        "nonnull" => Ty_::Tnonnull,
                        "dynamic" => Ty_::Tdynamic,
                        "varray_or_darray" => {
                            Ty_::TvarrayOrDarray(self.alloc((tarraykey(self.state.arena), tany())))
                        }
                        _ => {
                            let name =
                                self.prefix_ns(self.state.namespace_builder.rename_import(name));
                            Ty_::Tapply(self.alloc((Id(pos, name), &[][..])))
                        }
                    }
                };
                Some(self.alloc(Ty(reason, ty_)))
            }
        }
    }

    fn to_attributes(&self, node: Node<'a>) -> Attributes<'a> {
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
            returns_void_to_rx: false,
            accept_disposable: false,
            dynamically_callable: false,
            returns_disposable: false,
            php_std_lib: false,
        };

        // If we see the attribute `__OnlyRxIfImpl(Foo::class)`, set
        // `reactivity_condition_type` to `Foo`.
        let reactivity_condition_type = node.iter().find_map(|attr| match attr {
            Node::Attribute(UserAttributeNode {
                name: Id(_, "__OnlyRxIfImpl"),
                classname_params: &[class_name],
                ..
            }) => Some(self.alloc(Ty(
                self.alloc(Reason::hint(class_name.0)),
                Ty_::Tapply(self.alloc((class_name, &[][..]))),
            ))),
            _ => None,
        });

        let string_or_classname_arg = |attribute: &'a UserAttributeNode| {
            attribute
                .string_literal_params
                .first()
                .map(|&x| self.str_from_utf8(x))
                .or_else(|| attribute.classname_params.first().map(|x| x.1))
        };

        for attribute in node.iter() {
            if let Node::Attribute(attribute) = attribute {
                match attribute.name.1.as_ref() {
                    // NB: It is an error to specify more than one of __Rx,
                    // __RxShallow, and __RxLocal, so to avoid cloning the
                    // condition type, we use Option::take here.
                    "__Rx" => {
                        attributes.reactivity = Reactivity::Reactive(reactivity_condition_type)
                    }
                    "__RxShallow" => {
                        attributes.reactivity = Reactivity::Shallow(reactivity_condition_type)
                    }
                    "__RxLocal" => {
                        attributes.reactivity = Reactivity::Local(reactivity_condition_type)
                    }
                    "__Pure" => {
                        attributes.reactivity = Reactivity::Pure(reactivity_condition_type);
                    }
                    "__Cipp" => {
                        attributes.reactivity = Reactivity::Cipp(string_or_classname_arg(attribute))
                    }
                    "__CippGlobal" => {
                        attributes.reactivity = Reactivity::CippGlobal;
                    }
                    "__CippLocal" => {
                        attributes.reactivity =
                            Reactivity::CippLocal(string_or_classname_arg(attribute))
                    }
                    "__CippRx" => {
                        attributes.reactivity = Reactivity::CippRx;
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
                    "__ReturnsVoidToRx" => attributes.returns_void_to_rx = true,
                    "__Deprecated" => {
                        attributes.deprecated = attribute
                            .string_literal_params
                            .first()
                            .map(|&x| self.str_from_utf8(x));
                    }
                    "__Reifiable" => attributes.reifiable = Some(attribute.name.0),
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
                        attributes.enforceable = Some(attribute.name.0);
                    }
                    "__AcceptDisposable" => {
                        attributes.accept_disposable = true;
                    }
                    "__DynamicallyCallable" => {
                        attributes.dynamically_callable = true;
                    }
                    "__ReturnDisposable" => {
                        attributes.returns_disposable = true;
                    }
                    "__PHPStdLib" => {
                        attributes.php_std_lib = true;
                    }
                    _ => {}
                }
            } else {
                panic!("Expected an attribute, but was {:?}", node);
            }
        }

        attributes
    }

    // Limited version of node_to_ty that matches behavior of Decl_utils.infer_const
    fn infer_const(&self, name: Node<'a>, node: Node<'a>) -> Option<&'a Ty<'a>> {
        match node {
            Node::StringLiteral(_)
            | Node::BooleanLiteral(_)
            | Node::IntLiteral(_)
            | Node::FloatingLiteral(_)
            | Node::Null(_)
            | Node::Expr(aast::Expr(_, aast::Expr_::Unop(&(Uop::Uminus, _))))
            | Node::Expr(aast::Expr(_, aast::Expr_::Unop(&(Uop::Uplus, _)))) => {
                self.node_to_ty(node)
            }
            _ => Some(self.tany_with_pos(self.get_pos(name))),
        }
    }

    fn pop_type_params(&mut self, node: Node<'a>) -> &'a [&'a Tparam<'a>] {
        match node {
            Node::TypeParameters(tparams) => {
                Rc::make_mut(&mut self.state.type_parameters).pop().unwrap();
                tparams
            }
            _ => &[],
        }
    }

    fn ret_from_fun_kind(&self, kind: FunKind, type_: &'a Ty<'a>) -> &'a Ty<'a> {
        let pos = type_.get_pos().unwrap_or_else(|| Pos::none());
        match kind {
            FunKind::FAsyncGenerator => self.alloc(Ty(
                self.alloc(Reason::RretFunKind(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    Id(pos, naming_special_names::classes::ASYNC_GENERATOR),
                    self.alloc([type_, type_, type_]),
                ))),
            )),
            FunKind::FGenerator => self.alloc(Ty(
                self.alloc(Reason::RretFunKind(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    Id(pos, naming_special_names::classes::GENERATOR),
                    self.alloc([type_, type_, type_]),
                ))),
            )),
            FunKind::FAsync => self.alloc(Ty(
                self.alloc(Reason::RretFunKind(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    Id(pos, naming_special_names::classes::AWAITABLE),
                    self.alloc([type_]),
                ))),
            )),
            _ => type_,
        }
    }

    fn is_type_param_in_scope(&self, name: &str) -> bool {
        self.state
            .type_parameters
            .iter()
            .any(|tps| tps.contains(name))
    }

    fn param_mutability_to_fun_type_flags(
        param_mutability: Option<ParamMutability>,
    ) -> FunTypeFlags {
        match param_mutability {
            Some(ParamMutability::ParamBorrowedMutable) => FunTypeFlags::MUTABLE_FLAGS_BORROWED,
            Some(ParamMutability::ParamOwnedMutable) => FunTypeFlags::MUTABLE_FLAGS_OWNED,
            Some(ParamMutability::ParamMaybeMutable) => FunTypeFlags::MUTABLE_FLAGS_MAYBE,
            None => FunTypeFlags::empty(),
        }
    }

    fn param_mutability_to_fun_param_flags(
        param_mutability: Option<ParamMutability>,
    ) -> FunParamFlags {
        match param_mutability {
            Some(ParamMutability::ParamBorrowedMutable) => FunParamFlags::MUTABLE_FLAGS_BORROWED,
            Some(ParamMutability::ParamOwnedMutable) => FunParamFlags::MUTABLE_FLAGS_OWNED,
            Some(ParamMutability::ParamMaybeMutable) => FunParamFlags::MUTABLE_FLAGS_MAYBE,
            None => FunParamFlags::empty(),
        }
    }

    fn as_fun_implicit_params(
        &mut self,
        capability: Node<'a>,
        default_pos: &'a Pos<'a>,
    ) -> &'a FunImplicitParams<'a> {
        let capability = self.node_to_ty(capability).unwrap_or(default_capability(
            self.state.arena,
            Reason::hint(default_pos),
        ));
        self.alloc(FunImplicitParams { capability })
    }

    fn function_to_ty(
        &mut self,
        is_method: bool,
        attributes: Node<'a>,
        header: &'a FunctionHeader<'a>,
        body: Node,
    ) -> Option<(Id<'a>, &'a Ty<'a>, &'a [ShallowProp<'a>])> {
        let id_opt = match (is_method, header.name) {
            (true, Node::Construct(pos)) => {
                Some(Id(pos, naming_special_names::members::__CONSTRUCT))
            }
            (true, _) => self.expect_name(header.name),
            (false, _) => self.elaborate_name(header.name),
        };
        let id = id_opt.unwrap_or(Id(self.get_pos(header.name), ""));
        let (params, properties, arity) = self.as_fun_params(header.param_list)?;
        let f_pos = self.get_pos(header.name);
        let implicit_params = self.as_fun_implicit_params(header.capability, f_pos);

        let type_ = match header.name {
            Node::Construct(pos) => self.alloc(Ty(
                self.alloc(Reason::witness(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tvoid)),
            )),
            _ => self
                .node_to_ty(header.ret_hint)
                .unwrap_or_else(|| self.tany_with_pos(f_pos)),
        };
        let async_ = header
            .modifiers
            .iter()
            .any(|n| matches!(n, Node::Token(TokenKind::Async)));
        let fun_kind = if body.iter().any(|node| match node {
            Node::Token(TokenKind::Yield) => true,
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
        };
        let type_ = if !header.ret_hint.is_present() {
            self.ret_from_fun_kind(fun_kind, type_)
        } else {
            type_
        };
        let attributes = self.to_attributes(attributes);
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
        if attributes.returns_disposable {
            flags |= FunTypeFlags::RETURN_DISPOSABLE;
        }
        if attributes.returns_void_to_rx {
            flags |= FunTypeFlags::RETURNS_VOID_TO_RX;
        }

        flags |= Self::param_mutability_to_fun_type_flags(attributes.param_mutability);
        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(header.type_params);
        let ft = self.alloc(FunType {
            arity,
            tparams,
            where_constraints: &[],
            params,
            implicit_params,
            ret: self.alloc(PossiblyEnforcedTy {
                enforced: false,
                type_,
            }),
            reactive: attributes.reactivity,
            flags,
        });

        let ty = self.alloc(Ty(self.alloc(Reason::witness(id.0)), Ty_::Tfun(ft)));
        Some((id, ty, properties))
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

    fn as_fun_params(
        &self,
        list: Node<'a>,
    ) -> Option<(&'a FunParams<'a>, &'a [ShallowProp<'a>], FunArity<'a>)> {
        match list {
            Node::List(nodes) => {
                let mut params = Vec::with_capacity_in(nodes.len(), self.state.arena);
                let mut properties = Vec::new_in(self.state.arena);
                let mut arity = FunArity::Fstandard;
                for node in nodes.iter() {
                    match node {
                        Node::FunParam(&FunParamDecl {
                            attributes,
                            visibility,
                            kind,
                            hint,
                            id,
                            variadic,
                            initializer,
                        }) => {
                            let attributes = self.to_attributes(attributes);

                            if let Some(visibility) = visibility.as_visibility() {
                                let Id(pos, name) = id;
                                let name = strip_dollar_prefix(name);
                                properties.push(ShallowProp {
                                    const_: false,
                                    xhp_attr: None,
                                    lateinit: attributes.late_init,
                                    lsb: false,
                                    name: Id(pos, name),
                                    needs_init: true,
                                    type_: self.node_to_ty(hint),
                                    abstract_: false,
                                    visibility,
                                });
                            }

                            let type_ = if hint.is_ignored() {
                                tany()
                            } else {
                                self.node_to_ty(hint).map(|ty| match ty {
                                    Ty(r, Ty_::Tfun(fun_type)) if attributes.at_most_rx_as_func => {
                                        let mut fun_type = (*fun_type).clone();
                                        fun_type.reactive = Reactivity::RxVar(None);
                                        self.alloc(Ty(r, Ty_::Tfun(self.alloc(fun_type))))
                                    }
                                    Ty(r, Ty_::Toption(Ty(r1, Ty_::Tfun(fun_type))))
                                        if attributes.at_most_rx_as_func =>
                                    {
                                        let mut fun_type = (*fun_type).clone();
                                        fun_type.reactive = Reactivity::RxVar(None);
                                        self.alloc(Ty(
                                            r,
                                            Ty_::Toption(
                                                self.alloc(Ty(r1, Ty_::Tfun(self.alloc(fun_type)))),
                                            ),
                                        ))
                                    }
                                    ty => ty,
                                })?
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
                            if attributes.accept_disposable {
                                flags |= FunParamFlags::ACCEPT_DISPOSABLE
                            }
                            match kind {
                                ParamMode::FPinout => {
                                    flags |= FunParamFlags::INOUT;
                                }
                                ParamMode::FPnormal => {}
                            };
                            if initializer.is_present() {
                                flags |= FunParamFlags::HAS_DEFAULT;
                            }
                            let param = self.alloc(FunParam {
                                pos: id.0,
                                name: Some(id.1),
                                type_: self.alloc(PossiblyEnforcedTy {
                                    enforced: false,
                                    type_,
                                }),
                                flags,
                                rx_annotation: None,
                            });
                            arity = match arity {
                                FunArity::Fstandard if initializer.is_ignored() && variadic => {
                                    FunArity::Fvariadic(param)
                                }
                                arity => {
                                    params.push(param);
                                    arity
                                }
                            };
                        }
                        n => panic!("Expected a function parameter, but got {:?}", n),
                    }
                }
                Some((
                    params.into_bump_slice(),
                    properties.into_bump_slice(),
                    arity,
                ))
            }
            n if n.is_ignored() => Some((&[], &[], FunArity::Fstandard)),
            n => panic!("Expected a list of function parameters, but got {:?}", n),
        }
    }

    fn make_shape_field_name(&self, name: Node<'a>) -> Option<ShapeFieldName<'a>> {
        Some(match name {
            Node::StringLiteral(&(s, pos)) => ShapeFieldName::SFlitStr(self.alloc((pos, s))),
            // TODO: OCaml decl produces SFlitStr here instead of SFlitInt, so
            // we must also. Looks like int literal keys have become a parse
            // error--perhaps that's why.
            Node::IntLiteral(&(s, pos)) => ShapeFieldName::SFlitStr(self.alloc((pos, s.into()))),
            Node::Expr(aast::Expr(
                _,
                aast::Expr_::ClassConst(&(
                    aast::ClassId(_, aast::ClassId_::CI(&class_name)),
                    const_name,
                )),
            )) => ShapeFieldName::SFclassConst(self.alloc((class_name, const_name))),
            Node::Expr(aast::Expr(
                _,
                aast::Expr_::ClassConst(&(aast::ClassId(pos, aast::ClassId_::CIself), const_name)),
            )) => ShapeFieldName::SFclassConst(
                self.alloc((
                    Id(
                        pos,
                        self.state
                            .classish_name_builder
                            .get_current_classish_name()?
                            .0,
                    ),
                    const_name,
                )),
            ),
            _ => return None,
        })
    }

    fn make_apply(
        &self,
        base_ty: Id<'a>,
        type_arguments: Node<'a>,
        pos_to_merge: &'a Pos<'a>,
    ) -> Node<'a> {
        let type_arguments = self.slice(
            type_arguments
                .iter()
                .filter_map(|&node| self.node_to_ty(node)),
        );

        let ty_ = match (base_ty, type_arguments) {
            (Id(_, name), &[Ty(_, Ty_::Tfun(f))]) if name == "\\Pure" => {
                Ty_::Tfun(self.alloc(FunType {
                    reactive: Reactivity::Pure(None),
                    ..(*f).clone()
                }))
            }
            (Id(_, name), &[Ty(_, Ty_::Tfun(f))]) if name == "\\Rx" => {
                Ty_::Tfun(self.alloc(FunType {
                    reactive: Reactivity::Reactive(None),
                    ..(*f).clone()
                }))
            }
            (Id(_, name), &[Ty(_, Ty_::Tfun(f))]) if name == "\\RxShallow" => {
                Ty_::Tfun(self.alloc(FunType {
                    reactive: Reactivity::Shallow(None),
                    ..(*f).clone()
                }))
            }
            (Id(_, name), &[Ty(_, Ty_::Tfun(f))]) if name == "\\RxLocal" => {
                Ty_::Tfun(self.alloc(FunType {
                    reactive: Reactivity::Local(None),
                    ..(*f).clone()
                }))
            }
            _ => Ty_::Tapply(self.alloc((base_ty, type_arguments))),
        };
        self.hint_ty(self.merge(base_ty.0, pos_to_merge), ty_)
    }

    fn hint_ty(&self, pos: &'a Pos<'a>, ty_: Ty_<'a>) -> Node<'a> {
        Node::Ty(self.alloc(Ty(self.alloc(Reason::hint(pos)), ty_)))
    }

    fn prim_ty(&self, tprim: aast::Tprim<'a>, pos: &'a Pos<'a>) -> Node<'a> {
        self.hint_ty(pos, Ty_::Tprim(self.alloc(tprim)))
    }

    fn tany_with_pos(&self, pos: &'a Pos<'a>) -> &'a Ty<'a> {
        self.alloc(Ty(self.alloc(Reason::witness(pos)), TANY_))
    }

    fn source_text_at_pos(&self, pos: &'a Pos<'a>) -> &'a [u8] {
        let start = pos.start_cnum();
        let end = pos.end_cnum();
        self.state.source_text.source_text().sub(start, end - start)
    }

    // While we usually can tell whether to allocate a Tapply or Tgeneric based
    // on our type_parameters stack, *constraints* on type parameters may
    // reference type parameters which we have not parsed yet. When constructing
    // a type parameter list, we use this function to rewrite the type of each
    // constraint, considering the full list of type parameters to be in scope.
    fn convert_tapply_to_tgeneric(&self, ty: &'a Ty<'a>) -> &'a Ty<'a> {
        let ty_ = match ty.1 {
            Ty_::Tapply(&(id, targs)) => {
                let converted_targs = self.slice(
                    targs
                        .iter()
                        .map(|&targ| self.convert_tapply_to_tgeneric(targ)),
                );
                match self.tapply_should_be_tgeneric(ty.0, id) {
                    Some(name) => Ty_::Tgeneric(self.alloc((name, converted_targs))),
                    None => Ty_::Tapply(self.alloc((id, converted_targs))),
                }
            }
            Ty_::Tlike(ty) => Ty_::Tlike(self.convert_tapply_to_tgeneric(ty)),
            Ty_::TpuAccess(&(ty, id)) => {
                Ty_::TpuAccess(self.alloc((self.convert_tapply_to_tgeneric(ty), id)))
            }
            Ty_::Toption(ty) => Ty_::Toption(self.convert_tapply_to_tgeneric(ty)),
            Ty_::Tfun(fun_type) => {
                let convert_param = |param: &'a FunParam<'a>| {
                    self.alloc(FunParam {
                        type_: self.alloc(PossiblyEnforcedTy {
                            enforced: param.type_.enforced,
                            type_: self.convert_tapply_to_tgeneric(param.type_.type_),
                        }),
                        ..*param
                    })
                };
                let arity = match fun_type.arity {
                    FunArity::Fstandard => FunArity::Fstandard,
                    FunArity::Fvariadic(param) => FunArity::Fvariadic(convert_param(param)),
                };
                let params = self.slice(fun_type.params.iter().copied().map(convert_param));
                let implicit_params = fun_type.implicit_params;
                let ret = self.alloc(PossiblyEnforcedTy {
                    enforced: fun_type.ret.enforced,
                    type_: self.convert_tapply_to_tgeneric(fun_type.ret.type_),
                });
                Ty_::Tfun(self.alloc(FunType {
                    arity,
                    params,
                    implicit_params,
                    ret,
                    reactive: fun_type.reactive.clone(),
                    ..*fun_type
                }))
            }
            Ty_::Tshape(&(kind, fields)) => {
                let mut converted_fields =
                    AssocListMut::with_capacity_in(fields.len(), self.state.arena);
                for (&name, ty) in fields.iter() {
                    converted_fields.insert(
                        name,
                        self.alloc(ShapeFieldType {
                            optional: ty.optional,
                            ty: self.convert_tapply_to_tgeneric(ty.ty),
                        }),
                    );
                }
                Ty_::Tshape(self.alloc((kind, converted_fields.into())))
            }
            Ty_::Tdarray(&(tk, tv)) => Ty_::Tdarray(self.alloc((
                self.convert_tapply_to_tgeneric(tk),
                self.convert_tapply_to_tgeneric(tv),
            ))),
            Ty_::Tvarray(ty) => Ty_::Tvarray(self.convert_tapply_to_tgeneric(ty)),
            Ty_::TvarrayOrDarray(&(tk, tv)) => Ty_::TvarrayOrDarray(self.alloc((
                self.convert_tapply_to_tgeneric(tk),
                self.convert_tapply_to_tgeneric(tv),
            ))),
            Ty_::Ttuple(tys) => Ty_::Ttuple(
                self.slice(
                    tys.iter()
                        .map(|&targ| self.convert_tapply_to_tgeneric(targ)),
                ),
            ),
            _ => return ty,
        };
        self.alloc(Ty(ty.0, ty_))
    }

    // This is the logic for determining if convert_tapply_to_tgeneric should turn
    // a Tapply into a Tgeneric
    fn tapply_should_be_tgeneric(
        &self,
        reason: &'a Reason<'a>,
        id: nast::Sid<'a>,
    ) -> Option<&'a str> {
        match reason.pos() {
            // If the name contained a namespace delimiter in the original
            // source text, then it can't have referred to a type parameter
            // (since type parameters cannot be namespaced).
            Some(pos) => {
                if self.source_text_at_pos(pos).contains(&b'\\') {
                    return None;
                }
            }
            None => return None,
        }
        // However, the direct decl parser will unconditionally prefix
        // the name with the current namespace (as it does for any
        // Tapply). We need to remove it.
        match id.1.rsplit('\\').next() {
            Some(name) if self.is_type_param_in_scope(name) => return Some(name),
            _ => return None,
        }
    }

    fn user_attribute_to_decl(
        &self,
        attr: &UserAttributeNode<'a>,
    ) -> &'a shallow_decl_defs::UserAttribute<'a> {
        self.alloc(shallow_decl_defs::UserAttribute {
            name: attr.name,
            classname_params: self.slice(attr.classname_params.iter().map(|Id(_, s)| *s)),
        })
    }
}

enum NodeIterHelper<'a: 'b, 'b> {
    Empty,
    Single(&'b Node<'a>),
    Vec(std::slice::Iter<'b, Node<'a>>),
}

impl<'a, 'b> Iterator for NodeIterHelper<'a, 'b> {
    type Item = &'b Node<'a>;

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

    // Must return the upper bound returned by Node::len.
    fn size_hint(&self) -> (usize, Option<usize>) {
        match self {
            NodeIterHelper::Empty => (0, Some(0)),
            NodeIterHelper::Single(_) => (1, Some(1)),
            NodeIterHelper::Vec(iter) => iter.size_hint(),
        }
    }
}

impl<'a, 'b> DoubleEndedIterator for NodeIterHelper<'a, 'b> {
    fn next_back(&mut self) -> Option<Self::Item> {
        match self {
            NodeIterHelper::Empty => None,
            NodeIterHelper::Single(_) => self.next(),
            NodeIterHelper::Vec(ref mut iter) => iter.next_back(),
        }
    }
}

impl<'a> FlattenOp for DirectDeclSmartConstructors<'a> {
    type S = Node<'a>;

    fn flatten(&self, kind: SyntaxKind, lst: std::vec::Vec<Self::S>) -> Self::S {
        let size = lst
            .iter()
            .map(|s| match s {
                Node::List(children) => children.len(),
                x => {
                    if Self::is_zero(x) {
                        0
                    } else {
                        1
                    }
                }
            })
            .sum();
        let mut r = Vec::with_capacity_in(size, self.state.arena);
        for s in lst.into_iter() {
            match s {
                Node::List(children) => r.extend(children.iter().cloned()),
                x => {
                    if !Self::is_zero(&x) {
                        r.push(x)
                    }
                }
            }
        }
        match r.into_bump_slice() {
            [] => Node::Ignored(kind),
            [node] => *node,
            slice => Node::List(self.alloc(slice)),
        }
    }

    fn zero(kind: SyntaxKind) -> Self::S {
        Node::Ignored(kind)
    }

    fn is_zero(s: &Self::S) -> bool {
        match s {
            Node::Token(TokenKind::Yield) => false,
            Node::Token(TokenKind::Required) => false,
            Node::Token(TokenKind::Lateinit) => false,
            Node::List(inner) => inner.iter().all(Self::is_zero),
            _ => true,
        }
    }
}

impl<'a> FlattenSmartConstructors<'a, State<'a>> for DirectDeclSmartConstructors<'a> {
    fn make_token(&mut self, token: CompactToken) -> Self::R {
        let token_text = |this: &Self| this.str_from_utf8(this.token_bytes(&token));
        let token_pos = |this: &Self| {
            let start = this
                .state
                .source_text
                .offset_to_file_pos_triple(token.start_offset());
            let end = this
                .state
                .source_text
                .offset_to_file_pos_triple(token.end_offset());
            Pos::from_lnum_bol_cnum(this.state.arena, this.state.filename, start, end)
        };
        let kind = token.kind();
        let result = match kind {
            TokenKind::Name | TokenKind::XHPClassName => {
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
                if kind == TokenKind::XHPClassName {
                    Node::XhpName(self.alloc((name, pos)))
                } else {
                    Node::Name(self.alloc((name, pos)))
                }
            }
            TokenKind::Class => Node::Name(self.alloc((token_text(self), token_pos(self)))),
            // There are a few types whose string representations we have to
            // grab anyway, so just go ahead and treat them as generic names.
            TokenKind::Variable
            | TokenKind::Vec
            | TokenKind::Dict
            | TokenKind::Keyset
            | TokenKind::Tuple
            | TokenKind::Classname
            | TokenKind::SelfToken => Node::Name(self.alloc((token_text(self), token_pos(self)))),
            TokenKind::XHP | TokenKind::XHPElementName => {
                Node::XhpName(self.alloc((token_text(self), token_pos(self))))
            }
            TokenKind::SingleQuotedStringLiteral => match escaper::unescape_single_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.state.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text.into(), token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::DoubleQuotedStringLiteral => match escaper::unescape_double_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.state.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text.into(), token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::HeredocStringLiteral => match escaper::unescape_heredoc_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.state.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text.into(), token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::NowdocStringLiteral => match escaper::unescape_nowdoc_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.state.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text.into(), token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::DecimalLiteral
            | TokenKind::OctalLiteral
            | TokenKind::HexadecimalLiteral
            | TokenKind::BinaryLiteral => {
                Node::IntLiteral(self.alloc((token_text(self), token_pos(self))))
            }
            TokenKind::FloatingLiteral => {
                Node::FloatingLiteral(self.alloc((token_text(self), token_pos(self))))
            }
            TokenKind::NullLiteral => Node::Null(token_pos(self)),
            TokenKind::BooleanLiteral => {
                Node::BooleanLiteral(self.alloc((token_text(self), token_pos(self))))
            }
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
            TokenKind::Mixed => {
                Node::Ty(self.alloc(Ty(self.alloc(Reason::hint(token_pos(self))), Ty_::Tmixed)))
            }
            TokenKind::Void => self.prim_ty(aast::Tprim::Tvoid, token_pos(self)),
            TokenKind::Arraykey => self.prim_ty(aast::Tprim::Tarraykey, token_pos(self)),
            TokenKind::Noreturn => self.prim_ty(aast::Tprim::Tnoreturn, token_pos(self)),
            TokenKind::Resource => self.prim_ty(aast::Tprim::Tresource, token_pos(self)),
            TokenKind::Darray => Node::Darray(token_pos(self)),
            TokenKind::Varray => Node::Varray(token_pos(self)),
            TokenKind::Backslash => Node::Backslash(token_pos(self)),
            TokenKind::Construct => Node::Construct(token_pos(self)),
            TokenKind::LeftParen
            | TokenKind::RightParen
            | TokenKind::LeftBracket
            | TokenKind::RightBracket
            | TokenKind::Shape
            | TokenKind::Question => Node::Pos(token_pos(self)),
            TokenKind::This => Node::This(token_pos(self)),
            TokenKind::Tilde
            | TokenKind::Exclamation
            | TokenKind::Plus
            | TokenKind::Minus
            | TokenKind::PlusPlus
            | TokenKind::MinusMinus
            | TokenKind::At
            | TokenKind::Star
            | TokenKind::Slash
            | TokenKind::EqualEqual
            | TokenKind::EqualEqualEqual
            | TokenKind::StarStar
            | TokenKind::AmpersandAmpersand
            | TokenKind::BarBar
            | TokenKind::LessThan
            | TokenKind::LessThanEqual
            | TokenKind::GreaterThan
            | TokenKind::GreaterThanEqual
            | TokenKind::Dot
            | TokenKind::Ampersand
            | TokenKind::Bar
            | TokenKind::LessThanLessThan
            | TokenKind::GreaterThanGreaterThan
            | TokenKind::Percent
            | TokenKind::QuestionQuestion
            | TokenKind::Equal => Node::Operator(self.alloc((token_pos(self), kind))),
            TokenKind::Abstract
            | TokenKind::As
            | TokenKind::Super
            | TokenKind::Async
            | TokenKind::DotDotDot
            | TokenKind::Extends
            | TokenKind::Final
            | TokenKind::Implements
            | TokenKind::Inout
            | TokenKind::Interface
            | TokenKind::Newtype
            | TokenKind::Type
            | TokenKind::Yield
            | TokenKind::Semicolon
            | TokenKind::Private
            | TokenKind::Protected
            | TokenKind::Public
            | TokenKind::Reify
            | TokenKind::Static
            | TokenKind::Trait
            | TokenKind::Lateinit
            | TokenKind::Required => Node::Token(kind),
            TokenKind::EndOfFile
            | TokenKind::Attribute
            | TokenKind::Await
            | TokenKind::Binary
            | TokenKind::Break
            | TokenKind::Case
            | TokenKind::Catch
            | TokenKind::Category
            | TokenKind::Children
            | TokenKind::Clone
            | TokenKind::Const
            | TokenKind::Continue
            | TokenKind::Default
            | TokenKind::Define
            | TokenKind::Do
            | TokenKind::Echo
            | TokenKind::Else
            | TokenKind::Elseif
            | TokenKind::Empty
            | TokenKind::Endfor
            | TokenKind::Endforeach
            | TokenKind::Endif
            | TokenKind::Endswitch
            | TokenKind::Endwhile
            | TokenKind::Enum
            | TokenKind::Eval
            | TokenKind::Fallthrough
            | TokenKind::File
            | TokenKind::Finally
            | TokenKind::For
            | TokenKind::Foreach
            | TokenKind::From
            | TokenKind::Function
            | TokenKind::Global
            | TokenKind::Concurrent
            | TokenKind::Goto
            | TokenKind::If
            | TokenKind::Include
            | TokenKind::Includes
            | TokenKind::Include_once
            | TokenKind::Instanceof
            | TokenKind::Insteadof
            | TokenKind::Integer
            | TokenKind::Is
            | TokenKind::Isset
            | TokenKind::List
            | TokenKind::Namespace
            | TokenKind::New
            | TokenKind::Object
            | TokenKind::Parent
            | TokenKind::Print
            | TokenKind::Real
            | TokenKind::Record
            | TokenKind::RecordDec
            | TokenKind::Require
            | TokenKind::Require_once
            | TokenKind::Return
            | TokenKind::Suspend
            | TokenKind::Switch
            | TokenKind::Throw
            | TokenKind::Try
            | TokenKind::Unset
            | TokenKind::Use
            | TokenKind::Using
            | TokenKind::Var
            | TokenKind::Where
            | TokenKind::While
            | TokenKind::LeftBrace
            | TokenKind::RightBrace
            | TokenKind::MinusGreaterThan
            | TokenKind::Dollar
            | TokenKind::LessThanEqualGreaterThan
            | TokenKind::ExclamationEqual
            | TokenKind::ExclamationEqualEqual
            | TokenKind::Carat
            | TokenKind::QuestionAs
            | TokenKind::QuestionColon
            | TokenKind::QuestionQuestionEqual
            | TokenKind::Colon
            | TokenKind::StarStarEqual
            | TokenKind::StarEqual
            | TokenKind::SlashEqual
            | TokenKind::PercentEqual
            | TokenKind::PlusEqual
            | TokenKind::MinusEqual
            | TokenKind::DotEqual
            | TokenKind::LessThanLessThanEqual
            | TokenKind::GreaterThanGreaterThanEqual
            | TokenKind::AmpersandEqual
            | TokenKind::CaratEqual
            | TokenKind::BarEqual
            | TokenKind::Comma
            | TokenKind::ColonColon
            | TokenKind::EqualGreaterThan
            | TokenKind::EqualEqualGreaterThan
            | TokenKind::QuestionMinusGreaterThan
            | TokenKind::DollarDollar
            | TokenKind::BarGreaterThan
            | TokenKind::SlashGreaterThan
            | TokenKind::LessThanSlash
            | TokenKind::LessThanQuestion
            | TokenKind::ColonAt
            | TokenKind::Backtick
            | TokenKind::ErrorToken
            | TokenKind::DoubleQuotedStringLiteralHead
            | TokenKind::StringLiteralBody
            | TokenKind::DoubleQuotedStringLiteralTail
            | TokenKind::HeredocStringLiteralHead
            | TokenKind::HeredocStringLiteralTail
            | TokenKind::XHPCategoryName
            | TokenKind::XHPStringLiteral
            | TokenKind::XHPBody
            | TokenKind::XHPComment
            | TokenKind::Hashbang => Node::Ignored(SK::Token(kind)),
        };
        self.state.previous_token_kind = kind;
        result
    }

    fn make_missing(&mut self, _: usize) -> Self::R {
        Node::Ignored(SK::Missing)
    }

    fn make_list(&mut self, items: std::vec::Vec<Self::R>, _: usize) -> Self::R {
        if items
            .iter()
            .flat_map(|node| node.iter())
            .any(|node| matches!(node, Node::Token(TokenKind::Yield)))
        {
            Node::Token(TokenKind::Yield)
        } else {
            let size = items.iter().filter(|node| node.is_present()).count();
            let items_iter = items.into_iter();
            let mut items = Vec::with_capacity_in(size, self.state.arena);
            for node in items_iter {
                if node.is_present() {
                    items.push(node);
                }
            }
            let items = items.into_bump_slice();
            if items.is_empty() {
                Node::Ignored(SK::SyntaxList)
            } else {
                Node::List(self.alloc(items))
            }
        }
    }

    fn make_qualified_name(&mut self, parts: Self::R) -> Self::R {
        let pos = self.get_pos(parts);
        match parts {
            Node::List(nodes) => Node::QualifiedName(self.alloc((nodes, pos))),
            node if node.is_ignored() => Node::Ignored(SK::QualifiedName),
            node => Node::QualifiedName(self.alloc((
                bumpalo::vec![in self.state.arena; node].into_bump_slice(),
                pos,
            ))),
        }
    }

    fn make_simple_type_specifier(&mut self, specifier: Self::R) -> Self::R {
        // Return this explicitly because flatten filters out zero nodes, and
        // we treat most non-error nodes as zeroes.
        specifier
    }

    fn make_literal_expression(&mut self, expression: Self::R) -> Self::R {
        expression
    }

    fn make_simple_initializer(&mut self, equals: Self::R, expr: Self::R) -> Self::R {
        // If the expr is Ignored, bubble up the assignment operator so that we
        // can tell that *some* initializer was here. Useful for class
        // properties, where we need to enforce that properties without default
        // values are initialized in the constructor.
        if expr.is_ignored() { equals } else { expr }
    }

    fn make_anonymous_function(
        &mut self,
        _attribute_spec: Self::R,
        _static_keyword: Self::R,
        _async_keyword: Self::R,
        _function_keyword: Self::R,
        _left_paren: Self::R,
        _parameters: Self::R,
        _right_paren: Self::R,
        _colon: Self::R,
        _type_: Self::R,
        _use_: Self::R,
        _body: Self::R,
    ) -> Self::R {
        // do not allow Yield to bubble up
        Node::Ignored(SK::AnonymousFunction)
    }

    fn make_lambda_expression(
        &mut self,
        _attribute_spec: Self::R,
        _async_: Self::R,
        _signature: Self::R,
        _arrow: Self::R,
        _body: Self::R,
    ) -> Self::R {
        // do not allow Yield to bubble up
        Node::Ignored(SK::LambdaExpression)
    }

    fn make_awaitable_creation_expression(
        &mut self,
        _attribute_spec: Self::R,
        _async_: Self::R,
        _compound_statement: Self::R,
    ) -> Self::R {
        // do not allow Yield to bubble up
        Node::Ignored(SK::AwaitableCreationExpression)
    }

    fn make_darray_intrinsic_expression(
        &mut self,
        darray: Self::R,
        _explicit_type: Self::R,
        _left_bracket: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|node| match node {
            Node::ListItem(&(key, value)) => {
                let key = self.node_to_expr(key)?;
                let value = self.node_to_expr(value)?;
                Some((key, value))
            }
            n => panic!("Expected a ListItem but was {:?}", n),
        }));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(darray, right_bracket),
            nast::Expr_::Darray(self.alloc((None, fields))),
        )))
    }

    fn make_dictionary_intrinsic_expression(
        &mut self,
        dict: Self::R,
        _explicit_type: Self::R,
        _left_bracket: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|node| match node {
            Node::ListItem(&(key, value)) => {
                let key = self.node_to_expr(key)?;
                let value = self.node_to_expr(value)?;
                Some(self.alloc(aast::Field(key, value)))
            }
            n => panic!("Expected a ListItem but was {:?}", n),
        }));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(dict, right_bracket),
            nast::Expr_::KeyValCollection(self.alloc((aast_defs::KvcKind::Dict, None, fields))),
        )))
    }

    fn make_keyset_intrinsic_expression(
        &mut self,
        keyset: Self::R,
        _explicit_type: Self::R,
        _left_bracket: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|&node| self.node_to_expr(node)));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(keyset, right_bracket),
            nast::Expr_::ValCollection(self.alloc((aast_defs::VcKind::Keyset, None, fields))),
        )))
    }

    fn make_varray_intrinsic_expression(
        &mut self,
        varray: Self::R,
        _explicit_type: Self::R,
        _left_bracket: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|&node| self.node_to_expr(node)));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(varray, right_bracket),
            nast::Expr_::Varray(self.alloc((None, fields))),
        )))
    }

    fn make_vector_intrinsic_expression(
        &mut self,
        vec: Self::R,
        _explicit_type: Self::R,
        _left_bracket: Self::R,
        fields: Self::R,
        right_bracket: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|&node| self.node_to_expr(node)));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(vec, right_bracket),
            nast::Expr_::ValCollection(self.alloc((aast_defs::VcKind::Vec, None, fields))),
        )))
    }

    fn make_element_initializer(
        &mut self,
        key: Self::R,
        _arrow: Self::R,
        value: Self::R,
    ) -> Self::R {
        Node::ListItem(self.alloc((key, value)))
    }

    fn make_prefix_unary_expression(&mut self, op: Self::R, value: Self::R) -> Self::R {
        let pos = self.merge_positions(op, value);
        let op = match &op {
            Node::Operator(&(_, op)) => match op {
                TokenKind::Tilde => Uop::Utild,
                TokenKind::Exclamation => Uop::Unot,
                TokenKind::Plus => Uop::Uplus,
                TokenKind::Minus => Uop::Uminus,
                TokenKind::PlusPlus => Uop::Uincr,
                TokenKind::MinusMinus => Uop::Udecr,
                TokenKind::At => Uop::Usilence,
                _ => return Node::Ignored(SK::PrefixUnaryExpression),
            },
            _ => return Node::Ignored(SK::PrefixUnaryExpression),
        };
        let value = match self.node_to_expr(value) {
            Some(value) => value,
            None => return Node::Ignored(SK::PrefixUnaryExpression),
        };
        Node::Expr(self.alloc(aast::Expr(pos, aast::Expr_::Unop(self.alloc((op, value))))))
    }

    fn make_postfix_unary_expression(&mut self, value: Self::R, op: Self::R) -> Self::R {
        let pos = self.merge_positions(value, op);
        let op = match &op {
            Node::Operator(&(_, op)) => match op {
                TokenKind::PlusPlus => Uop::Upincr,
                TokenKind::MinusMinus => Uop::Updecr,
                _ => return Node::Ignored(SK::PostfixUnaryExpression),
            },
            _ => return Node::Ignored(SK::PostfixUnaryExpression),
        };
        let value = match self.node_to_expr(value) {
            Some(value) => value,
            None => return Node::Ignored(SK::PostfixUnaryExpression),
        };
        Node::Expr(self.alloc(aast::Expr(pos, aast::Expr_::Unop(self.alloc((op, value))))))
    }

    fn make_binary_expression(&mut self, lhs: Self::R, op_node: Self::R, rhs: Self::R) -> Self::R {
        let op = match &op_node {
            Node::Operator(&(_, op)) => match op {
                TokenKind::Plus => Bop::Plus,
                TokenKind::Minus => Bop::Minus,
                TokenKind::Star => Bop::Star,
                TokenKind::Slash => Bop::Slash,
                TokenKind::Equal => Bop::Eq(None),
                TokenKind::EqualEqual => Bop::Eqeq,
                TokenKind::EqualEqualEqual => Bop::Eqeqeq,
                TokenKind::StarStar => Bop::Starstar,
                TokenKind::AmpersandAmpersand => Bop::Ampamp,
                TokenKind::BarBar => Bop::Barbar,
                TokenKind::LessThan => Bop::Lt,
                TokenKind::LessThanEqual => Bop::Lte,
                TokenKind::LessThanLessThan => Bop::Ltlt,
                TokenKind::GreaterThan => Bop::Gt,
                TokenKind::GreaterThanEqual => Bop::Gte,
                TokenKind::GreaterThanGreaterThan => Bop::Gtgt,
                TokenKind::Dot => Bop::Dot,
                TokenKind::Ampersand => Bop::Amp,
                TokenKind::Bar => Bop::Bar,
                TokenKind::Percent => Bop::Percent,
                TokenKind::QuestionQuestion => Bop::QuestionQuestion,
                _ => return Node::Ignored(SK::BinaryExpression),
            },
            _ => return Node::Ignored(SK::BinaryExpression),
        };

        match (&op, rhs) {
            (Bop::Eq(_), Node::Token(TokenKind::Yield)) => return rhs,
            _ => {}
        }

        let pos = self.merge(self.merge_positions(lhs, op_node), self.get_pos(rhs));

        let lhs = match self.node_to_expr(lhs) {
            Some(lhs) => lhs,
            None => return Node::Ignored(SK::BinaryExpression),
        };
        let rhs = match self.node_to_expr(rhs) {
            Some(rhs) => rhs,
            None => return Node::Ignored(SK::BinaryExpression),
        };

        Node::Expr(self.alloc(aast::Expr(
            pos,
            aast::Expr_::Binop(self.alloc((op, lhs, rhs))),
        )))
    }

    fn make_parenthesized_expression(
        &mut self,
        lparen: Self::R,
        expr: Self::R,
        rparen: Self::R,
    ) -> Self::R {
        let pos = self.merge_positions(lparen, rparen);
        let expr_ = match self.node_to_expr(expr) {
            Some(expr) => expr.1,
            None => return Node::Ignored(SK::ParenthesizedExpression),
        };
        Node::Expr(self.alloc(aast::Expr(pos, expr_)))
    }

    fn make_list_item(&mut self, item: Self::R, sep: Self::R) -> Self::R {
        match (item.is_ignored(), sep.is_ignored()) {
            (true, true) => Node::Ignored(SK::ListItem),
            (false, true) => item,
            (true, false) => sep,
            (false, false) => Node::ListItem(self.alloc((item, sep))),
        }
    }

    fn make_type_arguments(
        &mut self,
        less_than: Self::R,
        arguments: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        Node::BracketedList(self.alloc((
            self.get_pos(less_than),
            arguments.as_slice(self.state.arena),
            self.get_pos(greater_than),
        )))
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::R,
        type_arguments: Self::R,
    ) -> Self::R {
        let class_id = match self.expect_name(class_type) {
            Some(id) => id,
            None => return Node::Ignored(SK::GenericTypeSpecifier),
        };
        if class_id.1.trim_start_matches("\\") == "varray_or_darray" {
            let pos = self.merge(class_id.0, self.get_pos(type_arguments));
            let type_arguments = type_arguments.as_slice(self.state.arena);
            let ty_ = match type_arguments {
                [tk, tv] => Ty_::TvarrayOrDarray(self.alloc((
                    self.node_to_ty(*tk).unwrap_or_else(|| tany()),
                    self.node_to_ty(*tv).unwrap_or_else(|| tany()),
                ))),
                [tv] => Ty_::TvarrayOrDarray(self.alloc((
                    tarraykey(self.state.arena),
                    self.node_to_ty(*tv).unwrap_or_else(|| tany()),
                ))),
                _ => TANY_,
            };
            self.hint_ty(pos, ty_)
        } else {
            let Id(pos, class_type) = class_id;
            match class_type.rsplit('\\').next() {
                Some(name) if self.is_type_param_in_scope(name) => {
                    let type_arguments = self.slice(
                        type_arguments
                            .iter()
                            .filter_map(|&node| self.node_to_ty(node)),
                    );
                    let ty_ = Ty_::Tgeneric(self.alloc((name, type_arguments)));
                    self.hint_ty(pos, ty_)
                }
                _ => {
                    let class_type = self.state.namespace_builder.rename_import(class_type);
                    let class_type = if class_type.starts_with("\\") {
                        class_type
                    } else {
                        self.concat(self.state.namespace_builder.current_namespace(), class_type)
                    };
                    self.make_apply(
                        Id(pos, class_type),
                        type_arguments,
                        self.get_pos(type_arguments),
                    )
                }
            }
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
        if name.is_ignored() {
            return Node::Ignored(SK::AliasDeclaration);
        }
        let Id(pos, name) = match self.elaborate_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::AliasDeclaration),
        };
        let ty = match self.node_to_ty(aliased_type) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::AliasDeclaration),
        };
        let constraint = match constraint {
            Node::TypeConstraint(&(_kind, hint)) => self.node_to_ty(hint),
            _ => None,
        };
        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(generic_params);
        let typedef = self.alloc(TypedefType {
            pos,
            vis: match keyword {
                Node::Token(TokenKind::Type) => aast::TypedefVisibility::Transparent,
                Node::Token(TokenKind::Newtype) => aast::TypedefVisibility::Opaque,
                _ => aast::TypedefVisibility::Transparent,
            },
            tparams,
            constraint,
            type_: ty,
        });

        self.add_typedef(name, typedef);

        Node::Ignored(SK::AliasDeclaration)
    }

    fn make_type_constraint(&mut self, kind: Self::R, value: Self::R) -> Self::R {
        let kind = match kind {
            Node::Token(TokenKind::As) => ConstraintKind::ConstraintAs,
            Node::Token(TokenKind::Super) => ConstraintKind::ConstraintSuper,
            n => panic!("Expected either As or Super, but was {:?}", n),
        };
        Node::TypeConstraint(self.alloc((kind, value)))
    }

    fn make_type_parameter(
        &mut self,
        user_attributes: Self::R,
        reify: Self::R,
        variance: Self::R,
        name: Self::R,
        tparam_params: Self::R,
        constraints: Self::R,
    ) -> Self::R {
        let user_attributes = match user_attributes {
            Node::BracketedList((_, attributes, _)) => {
                self.slice(attributes.into_iter().filter_map(|x| match x {
                    Node::Attribute(a) => Some(*a),
                    _ => None,
                }))
            }
            _ => &[][..],
        };

        let constraints = self.slice(constraints.iter().filter_map(|node| match node {
            Node::TypeConstraint(&constraint) => Some(constraint),
            n if n.is_ignored() => None,
            n => panic!("Expected a type constraint, but was {:?}", n),
        }));

        // TODO(T70068435) Once we add support for constraints on higher-kinded types
        // (in particular, constraints on nested type parameters), we need to ensure
        // that we correctly handle the scoping of nested type parameters.
        // This includes making sure that the call to convert_type_appl_to_generic
        // in make_type_parameters handles nested constraints.
        // For now, we just make sure that the nested type parameters that make_type_parameters
        // added to the global list of in-scope type parameters are removed immediately:
        self.pop_type_params(tparam_params);

        let tparam_params = match tparam_params {
            Node::TypeParameters(&params) => params,
            _ => &[],
        };

        Node::TypeParameter(self.alloc(TypeParameterDecl {
            name,
            variance: match variance {
                Node::Operator(&(_, TokenKind::Minus)) => Variance::Contravariant,
                Node::Operator(&(_, TokenKind::Plus)) => Variance::Covariant,
                _ => Variance::Invariant,
            },
            reified: match reify {
                Node::Token(TokenKind::Reify) => aast::ReifyKind::Reified,
                _ => aast::ReifyKind::Erased,
            },
            constraints,
            tparam_params,
            user_attributes,
        }))
    }

    fn make_type_parameters(&mut self, _lt: Self::R, tparams: Self::R, _gt: Self::R) -> Self::R {
        let size = tparams.len();
        let mut tparams_with_name = Vec::with_capacity_in(size, self.state.arena);
        let mut tparam_names = MultiSetMut::with_capacity_in(size, self.state.arena);
        for node in tparams.iter() {
            match node {
                &Node::TypeParameter(decl) => {
                    let name = match decl.name.as_id() {
                        Some(name) => name,
                        None => return Node::Ignored(SK::TypeParameters),
                    };
                    tparam_names.insert(name.1);
                    tparams_with_name.push((decl, name));
                }
                n => panic!("Expected a type parameter, but got {:?}", n),
            }
        }
        Rc::make_mut(&mut self.state.type_parameters).push(tparam_names.into());
        let mut tparams = Vec::with_capacity_in(tparams_with_name.len(), self.state.arena);
        for (decl, name) in tparams_with_name.into_iter() {
            let &TypeParameterDecl {
                name: _,
                variance,
                reified,
                constraints,
                tparam_params,
                user_attributes,
            } = decl;
            let constraints = self.slice(constraints.iter().filter_map(|constraint| {
                let &(kind, ty) = constraint;
                let ty = self.node_to_ty(ty)?;
                let ty = self.convert_tapply_to_tgeneric(ty);
                Some((kind, ty))
            }));

            let user_attributes = self.slice(
                user_attributes
                    .iter()
                    .rev()
                    .map(|x| self.user_attribute_to_decl(x)),
            );
            tparams.push(self.alloc(Tparam {
                variance,
                name,
                constraints,
                reified,
                user_attributes,
                tparams: tparam_params,
            }));
        }
        Node::TypeParameters(self.alloc(tparams.into_bump_slice()))
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
        let (variadic, id) = match name {
            Node::ListItem(&(ellipsis, id)) => {
                let id = match id.as_id() {
                    Some(id) => id,
                    None => return Node::Ignored(SK::ParameterDeclaration),
                };
                match ellipsis {
                    Node::Token(TokenKind::DotDotDot) => (true, id),
                    _ => (false, id),
                }
            }
            name => {
                let name = match name.as_id() {
                    Some(name) => name,
                    None => return Node::Ignored(SK::ParameterDeclaration),
                };
                (false, name)
            }
        };
        let kind = match inout {
            Node::Token(TokenKind::Inout) => ParamMode::FPinout,
            _ => ParamMode::FPnormal,
        };
        Node::FunParam(self.alloc(FunParamDecl {
            attributes,
            visibility,
            kind,
            hint,
            id,
            variadic,
            initializer,
        }))
    }

    fn make_variadic_parameter(&mut self, _: Self::R, hint: Self::R, _: Self::R) -> Self::R {
        Node::FunParam(self.alloc(FunParamDecl {
            attributes: Node::Ignored(SK::Missing),
            visibility: Node::Ignored(SK::Missing),
            kind: ParamMode::FPnormal,
            hint,
            id: Id(self.get_pos(hint), "".into()),
            variadic: true,
            initializer: Node::Ignored(SK::Missing),
        }))
    }

    fn make_function_declaration(
        &mut self,
        attributes: Self::R,
        header: Self::R,
        body: Self::R,
    ) -> Self::R {
        let parsed_attributes = self.to_attributes(attributes);
        match header {
            Node::FunctionHeader(header) => {
                let is_method = false;
                let (Id(pos, name), type_, _) =
                    match self.function_to_ty(is_method, attributes, header, body) {
                        Some(x) => x,
                        None => return Node::Ignored(SK::FunctionDeclaration),
                    };
                let deprecated = parsed_attributes.deprecated.map(|msg| {
                    let mut s = String::new_in(self.state.arena);
                    s.push_str("The function ");
                    s.push_str(name.trim_start_matches("\\"));
                    s.push_str(" is deprecated: ");
                    s.push_str(msg);
                    s.into_bump_str()
                });
                let fun_elt = self.alloc(FunElt {
                    deprecated,
                    type_,
                    pos,
                    php_std_lib: parsed_attributes.php_std_lib,
                });
                self.add_fun(name, fun_elt);
                Node::Ignored(SK::FunctionDeclaration)
            }
            _ => Node::Ignored(SK::FunctionDeclaration),
        }
    }

    fn make_capability(&mut self, lb: Self::R, tys: Self::R, rb: Self::R) -> Self::R {
        Rc::make_mut(&mut self.state.namespace_builder).push_namespace(Some("HH\\Contexts"));
        let cap = self.make_intersection_type_specifier(lb, tys, rb);
        Rc::make_mut(&mut self.state.namespace_builder).pop_namespace();
        cap
    }

    fn make_capability_provisional(
        &mut self,
        _at: Self::R,
        _lb: Self::R,
        ty: Self::R,
        _plus: Self::R,
        _unsafe_ty: Self::R,
        _rb: Self::R,
    ) -> Self::R {
        ty
    }

    fn make_function_declaration_header(
        &mut self,
        modifiers: Self::R,
        _keyword: Self::R,
        name: Self::R,
        type_params: Self::R,
        left_paren: Self::R,
        param_list: Self::R,
        _right_paren: Self::R,
        capability: Self::R,
        capability_provisional: Self::R,
        _colon: Self::R,
        ret_hint: Self::R,
        _where: Self::R,
    ) -> Self::R {
        // Use the position of the left paren if the name is missing.
        let name = if name.is_ignored() { left_paren } else { name };
        let capability = if capability.is_ignored() {
            capability_provisional
        } else {
            capability
        };
        Node::FunctionHeader(self.alloc(FunctionHeader {
            name,
            modifiers,
            type_params,
            param_list,
            capability,
            ret_hint,
        }))
    }

    fn make_yield_expression(&mut self, _keyword: Self::R, _operand: Self::R) -> Self::R {
        Node::Token(TokenKind::Yield)
    }

    fn make_const_declaration(
        &mut self,
        modifiers: Self::R,
        _const_keyword: Self::R,
        hint: Self::R,
        decls: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        match decls {
            // Class consts.
            Node::List(consts)
                if self
                    .state
                    .classish_name_builder
                    .get_current_classish_name()
                    .is_some() =>
            {
                let ty = self.node_to_ty(hint);
                Node::List(
                    self.alloc(self.slice(consts.iter().filter_map(|cst| match cst {
                        Node::ConstInitializer(&(name, initializer)) => {
                            let id = name.as_id()?;
                            let ty = ty
                                .or_else(|| self.infer_const(name, initializer))
                                .unwrap_or_else(|| tany());
                            let modifiers = read_member_modifiers(modifiers.iter());
                            Some(Node::Const(self.alloc(
                                shallow_decl_defs::ShallowClassConst {
                                    abstract_: modifiers.is_abstract,
                                    name: id,
                                    type_: ty,
                                },
                            )))
                        }
                        _ => None,
                    }))),
                )
            }
            // Global consts.
            Node::List([Node::ConstInitializer(&(name, initializer))]) => {
                let Id(pos, id) = match self.elaborate_name(name) {
                    Some(id) => id,
                    None => return Node::Ignored(SK::ConstDeclaration),
                };
                let ty = self
                    .node_to_ty(hint)
                    .or_else(|| self.infer_const(name, initializer))
                    .unwrap_or_else(|| tany());
                self.add_const(id, self.alloc(ConstDecl { pos, type_: ty }));
                Node::Ignored(SK::ConstDeclaration)
            }
            _ => Node::Ignored(SK::ConstDeclaration),
        }
    }

    fn make_constant_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        if name.is_ignored() {
            Node::Ignored(SK::ConstantDeclarator)
        } else {
            Node::ConstInitializer(self.alloc((name, initializer)))
        }
    }

    fn make_namespace_declaration(&mut self, _name: Self::R, body: Self::R) -> Self::R {
        if let Node::Ignored(SK::NamespaceBody) = body {
            Rc::make_mut(&mut self.state.namespace_builder).pop_namespace();
        }
        Node::Ignored(SK::NamespaceDeclaration)
    }

    fn make_namespace_declaration_header(&mut self, _keyword: Self::R, name: Self::R) -> Self::R {
        let name = self.expect_name(name).map(|Id(_, name)| name);
        // if this is header of semicolon-style (one with NamespaceEmptyBody) namespace, we should pop
        // the previous namespace first, but we don't have the body yet. We'll fix it retroactively in
        // make_namespace_empty_body
        Rc::make_mut(&mut self.state.namespace_builder).push_namespace(name);
        Node::Ignored(SK::NamespaceDeclarationHeader)
    }

    fn make_namespace_body(
        &mut self,
        _left_brace: Self::R,
        _declarations: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::NamespaceBody)
    }

    fn make_namespace_empty_body(&mut self, _semicolon: Self::R) -> Self::R {
        Rc::make_mut(&mut self.state.namespace_builder).pop_previous_namespace();
        Node::Ignored(SK::NamespaceEmptyBody)
    }

    fn make_namespace_use_declaration(
        &mut self,
        _keyword: Self::R,
        _kind: Self::R,
        clauses: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        for clause in clauses.iter() {
            if let Node::NamespaceUseClause(nuc) = clause {
                Rc::make_mut(&mut self.state.namespace_builder).add_import(nuc.id.1, nuc.as_);
            }
        }
        Node::Ignored(SK::NamespaceUseDeclaration)
    }

    fn make_namespace_group_use_declaration(
        &mut self,
        _keyword: Self::R,
        _kind: Self::R,
        prefix: Self::R,
        _left_brace: Self::R,
        clauses: Self::R,
        _right_brace: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let Id(_, prefix) = match self.expect_name(prefix) {
            Some(id) => id,
            None => return Node::Ignored(SK::NamespaceGroupUseDeclaration),
        };
        for clause in clauses.iter() {
            if let Node::NamespaceUseClause(nuc) = clause {
                let mut id = String::new_in(self.state.arena);
                id.push_str(prefix);
                id.push_str(nuc.id.1);
                Rc::make_mut(&mut self.state.namespace_builder)
                    .add_import(id.into_bump_str(), nuc.as_);
            }
        }
        Node::Ignored(SK::NamespaceGroupUseDeclaration)
    }

    fn make_namespace_use_clause(
        &mut self,
        _clause_kind: Self::R,
        name: Self::R,
        as_: Self::R,
        aliased_name: Self::R,
    ) -> Self::R {
        let id = match self.expect_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::NamespaceUseClause),
        };
        let as_ = if let Node::Token(TokenKind::As) = as_ {
            match aliased_name.as_id() {
                Some(name) => Some(name.1),
                None => return Node::Ignored(SK::NamespaceUseClause),
            }
        } else {
            None
        };
        Node::NamespaceUseClause(self.alloc(NamespaceUseClause { id, as_ }))
    }

    fn make_where_clause(&mut self, _: Self::R, where_constraints: Self::R) -> Self::R {
        where_constraints
    }

    fn make_where_constraint(
        &mut self,
        left_type: Self::R,
        operator: Self::R,
        right_type: Self::R,
    ) -> Self::R {
        Node::WhereConstraint(self.alloc(WhereConstraint(
            self.node_to_ty(left_type).unwrap_or_else(|| tany()),
            match operator {
                Node::Operator((_, TokenKind::Equal)) => ConstraintKind::ConstraintEq,
                Node::Token(TokenKind::As) => ConstraintKind::ConstraintAs,
                Node::Token(TokenKind::Super) => ConstraintKind::ConstraintSuper,
                _ => ConstraintKind::ConstraintAs,
            },
            self.node_to_ty(right_type).unwrap_or_else(|| tany()),
        )))
    }

    fn make_classish_declaration(
        &mut self,
        attributes: Self::R,
        modifiers: Self::R,
        xhp_keyword: Self::R,
        class_keyword: Self::R,
        name: Self::R,
        tparams: Self::R,
        _extends_keyword: Self::R,
        extends: Self::R,
        _implements_keyword: Self::R,
        implements: Self::R,
        where_clause: Self::R,
        body: Self::R,
    ) -> Self::R {
        let Id(pos, name) = match self.elaborate_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::ClassishDeclaration),
        };
        let (is_xhp, name) = if name.starts_with(":") {
            (true, prefix_slash(self.state.arena, name))
        } else {
            (false, name)
        };

        let mut class_kind = match class_keyword {
            Node::Token(TokenKind::Interface) => ClassKind::Cinterface,
            Node::Token(TokenKind::Trait) => ClassKind::Ctrait,
            _ => ClassKind::Cnormal,
        };
        let mut final_ = false;

        for modifier in modifiers.iter() {
            match modifier {
                Node::Token(TokenKind::Abstract) => class_kind = ClassKind::Cabstract,
                Node::Token(TokenKind::Final) => final_ = true,
                _ => {}
            }
        }

        let where_constraints = self.slice(where_clause.iter().filter_map(|&x| match x {
            Node::WhereConstraint(x) => {
                Some(self.alloc(shallow_decl_defs::WhereConstraint(x.0, x.1, x.2)))
            }
            _ => None,
        }));

        let body = match body {
            Node::ClassishBody(body) => body,
            body => panic!("Expected a classish body, but was {:?}", body),
        };

        let mut uses_len = 0;
        let mut xhp_attr_uses_len = 0;
        let mut req_extends_len = 0;
        let mut req_implements_len = 0;
        let mut consts_len = 0;
        let mut typeconsts_len = 0;
        let mut props_len = 0;
        let mut sprops_len = 0;
        let mut static_methods_len = 0;
        let mut methods_len = 0;

        let mut user_attributes_len = 0;
        for attribute in attributes.iter() {
            match attribute {
                &Node::Attribute(..) => user_attributes_len += 1,
                _ => {}
            }
        }

        for element in body.iter().copied() {
            match element {
                Node::TraitUse(names) => uses_len += names.len(),
                Node::XhpClassAttributeDeclaration(&XhpClassAttributeDeclarationNode {
                    xhp_attr_decls,
                    xhp_attr_uses_decls,
                }) => {
                    props_len += xhp_attr_decls.len();
                    xhp_attr_uses_len += xhp_attr_uses_decls.len();
                }
                Node::TypeConstant(..) => typeconsts_len += 1,
                Node::RequireClause(require) => match require.require_type {
                    Node::Token(TokenKind::Extends) => req_extends_len += 1,
                    Node::Token(TokenKind::Implements) => req_implements_len += 1,
                    _ => {}
                },
                Node::List(consts @ [Node::Const(..), ..]) => consts_len += consts.len(),
                Node::Property(&PropertyNode { decls, is_static }) => {
                    if is_static {
                        sprops_len += decls.len()
                    } else {
                        props_len += decls.len()
                    }
                }
                Node::Constructor(&ConstructorNode { properties, .. }) => {
                    props_len += properties.len()
                }
                Node::Method(&MethodNode { is_static, .. }) => {
                    if is_static {
                        static_methods_len += 1
                    } else {
                        methods_len += 1
                    }
                }
                _ => {}
            }
        }

        let mut constructor = None;

        let mut uses = Vec::with_capacity_in(uses_len, self.state.arena);
        let mut xhp_attr_uses = Vec::with_capacity_in(xhp_attr_uses_len, self.state.arena);
        let mut req_extends = Vec::with_capacity_in(req_extends_len, self.state.arena);
        let mut req_implements = Vec::with_capacity_in(req_implements_len, self.state.arena);
        let mut consts = Vec::with_capacity_in(consts_len, self.state.arena);
        let mut typeconsts = Vec::with_capacity_in(typeconsts_len, self.state.arena);
        let mut props = Vec::with_capacity_in(props_len, self.state.arena);
        let mut sprops = Vec::with_capacity_in(sprops_len, self.state.arena);
        let mut static_methods = Vec::with_capacity_in(static_methods_len, self.state.arena);
        let mut methods = Vec::with_capacity_in(methods_len, self.state.arena);

        let mut user_attributes = Vec::with_capacity_in(user_attributes_len, self.state.arena);
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => user_attributes.push(self.user_attribute_to_decl(&attr)),
                _ => {}
            }
        }
        // Match ordering of attributes produced by the OCaml decl parser (even
        // though it's the reverse of the syntactic ordering).
        user_attributes.reverse();

        // xhp props go after regular props, regardless of their order in file
        let mut xhp_props = vec![];

        for element in body.iter().copied() {
            match element {
                Node::TraitUse(names) => {
                    uses.extend(names.iter().filter_map(|&name| self.node_to_ty(name)))
                }
                Node::XhpClassAttributeDeclaration(&XhpClassAttributeDeclarationNode {
                    xhp_attr_decls,
                    xhp_attr_uses_decls,
                }) => {
                    xhp_props.extend(xhp_attr_decls);
                    xhp_attr_uses.extend(
                        xhp_attr_uses_decls
                            .iter()
                            .filter_map(|&node| self.node_to_ty(node)),
                    )
                }
                Node::TypeConstant(constant) => typeconsts.push(constant),
                Node::RequireClause(require) => match require.require_type {
                    Node::Token(TokenKind::Extends) => {
                        req_extends.extend(self.node_to_ty(require.name).iter())
                    }
                    Node::Token(TokenKind::Implements) => {
                        req_implements.extend(self.node_to_ty(require.name).iter())
                    }
                    _ => {}
                },
                Node::List(&const_nodes @ [Node::Const(..), ..]) => {
                    for node in const_nodes {
                        if let &Node::Const(decl) = node {
                            consts.push(decl)
                        }
                    }
                }
                Node::Property(&PropertyNode { decls, is_static }) => {
                    for property in decls {
                        if is_static {
                            sprops.push(property)
                        } else {
                            props.push(property)
                        }
                    }
                }
                Node::Constructor(&ConstructorNode { method, properties }) => {
                    constructor = Some(method);
                    for property in properties {
                        props.push(property)
                    }
                }
                Node::Method(&MethodNode { method, is_static }) => {
                    if is_static {
                        static_methods.push(method);
                    } else {
                        methods.push(method);
                    }
                }
                _ => {} // It's not our job to report errors here.
            }
        }

        props.extend(xhp_props.into_iter());

        let uses = uses.into_bump_slice();
        let xhp_attr_uses = xhp_attr_uses.into_bump_slice();
        let req_extends = req_extends.into_bump_slice();
        let req_implements = req_implements.into_bump_slice();
        let consts = consts.into_bump_slice();
        let typeconsts = typeconsts.into_bump_slice();
        let props = props.into_bump_slice();
        let sprops = sprops.into_bump_slice();
        let static_methods = static_methods.into_bump_slice();
        let methods = methods.into_bump_slice();
        let user_attributes = user_attributes.into_bump_slice();

        let extends = self.slice(extends.iter().filter_map(|&node| self.node_to_ty(node)));

        let implements = self.slice(implements.iter().filter_map(|&node| self.node_to_ty(node)));

        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(tparams);

        let cls = self.alloc(shallow_decl_defs::ShallowClass {
            mode: self.state.file_mode,
            final_,
            is_xhp,
            has_xhp_keyword: match xhp_keyword {
                Node::Token(TokenKind::XHP) => true,
                _ => false,
            },
            kind: class_kind,
            name: Id(pos, name),
            tparams,
            where_constraints,
            extends,
            uses,
            xhp_attr_uses,
            req_extends,
            req_implements,
            implements,
            consts,
            typeconsts,
            pu_enums: &[],
            props,
            sprops,
            constructor,
            static_methods,
            methods,
            user_attributes,
            enum_type: None,
        });
        self.add_class(name, cls);

        self.state
            .classish_name_builder
            .parsed_classish_declaration();

        Node::Ignored(SK::ClassishDeclaration)
    }

    fn make_property_declaration(
        &mut self,
        attrs: Self::R,
        modifiers: Self::R,
        hint: Self::R,
        declarators: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let (attrs, modifiers, hint) = (attrs, modifiers, hint);
        let modifiers = read_member_modifiers(modifiers.iter());
        let declarators = self.slice(declarators.iter().filter_map(
            |declarator| match declarator {
                Node::ListItem(&(name, initializer)) => {
                    let attributes = self.to_attributes(attrs);
                    let Id(pos, name) = name.as_id()?;
                    let name = if modifiers.is_static {
                        name
                    } else {
                        strip_dollar_prefix(name)
                    };
                    let ty = self.node_to_ty(hint);
                    Some(ShallowProp {
                        const_: attributes.const_,
                        xhp_attr: None,
                        lateinit: attributes.late_init,
                        lsb: attributes.lsb,
                        name: Id(pos, name),
                        needs_init: initializer.is_ignored(),
                        type_: ty,
                        abstract_: modifiers.is_abstract,
                        visibility: modifiers.visibility,
                    })
                }
                n => panic!("Expected a ListItem, but was {:?}", n),
            },
        ));
        Node::Property(self.alloc(PropertyNode {
            decls: declarators,
            is_static: modifiers.is_static,
        }))
    }

    fn make_xhp_class_attribute_declaration(
        &mut self,
        _keyword: Self::R,
        attributes: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let xhp_attr_decls = self.slice(attributes.iter().filter_map(|node| {
            let node = match node {
                Node::XhpClassAttribute(x) => x,
                _ => return None,
            };
            let Id(pos, name) = node.name;
            let name = prefix_colon(self.state.arena, name);

            let type_ = self.node_to_ty(node.hint);
            let type_ = if node.nullable && node.tag.is_none() {
                type_.and_then(|x| match x {
                    Ty(_, Ty_::Toption(_)) => type_, // already nullable
                    _ => self.node_to_ty(self.hint_ty(x.get_pos()?, Ty_::Toption(x))), // make nullable
                })
            } else {
                type_
            };
            Some(ShallowProp {
                abstract_: false,
                const_: false,
                lateinit: false,
                lsb: false,
                name: Id(pos, name),
                needs_init: node.needs_init,
                visibility: aast::Visibility::Public,
                type_,
                xhp_attr: Some(shallow_decl_defs::XhpAttr {
                    tag: node.tag,
                    has_default: !node.needs_init,
                }),
            })
        }));

        let xhp_attr_uses_decls = self.slice(attributes.iter().filter_map(|x| match x {
            Node::XhpAttributeUse(&name) => Some(name),
            _ => None,
        }));

        Node::XhpClassAttributeDeclaration(self.alloc(XhpClassAttributeDeclarationNode {
            xhp_attr_decls,
            xhp_attr_uses_decls,
        }))
    }

    fn make_xhp_enum_type(
        &mut self,
        _keyword: Self::R,
        _left_brace: Self::R,
        xhp_enum_values: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        Node::XhpEnumType(self.alloc(xhp_enum_values))
    }

    fn make_xhp_class_attribute(
        &mut self,
        type_: Self::R,
        name: Self::R,
        initializer: Self::R,
        tag: Self::R,
    ) -> Self::R {
        let name = match name.as_id() {
            Some(name) => name,
            None => return Node::Ignored(SK::XHPClassAttribute),
        };
        Node::XhpClassAttribute(self.alloc(XhpClassAttributeNode {
            name,
            hint: type_,
            needs_init: !initializer.is_present(),
            tag: match tag {
                Node::Token(TokenKind::Required) => Some(XhpAttrTag::Required),
                Node::Token(TokenKind::Lateinit) => Some(XhpAttrTag::Lateinit),
                _ => None,
            },
            nullable: match initializer {
                Node::Null(_) => true,
                _ => !initializer.is_present(),
            },
        }))
    }

    fn make_xhp_simple_class_attribute(&mut self, name: Self::R) -> Self::R {
        Node::XhpAttributeUse(self.alloc(name))
    }

    fn make_property_declarator(&mut self, name: Self::R, initializer: Self::R) -> Self::R {
        Node::ListItem(self.alloc((name, initializer)))
    }

    fn make_methodish_declaration(
        &mut self,
        attributes: Self::R,
        header: Self::R,
        body: Self::R,
        closer: Self::R,
    ) -> Self::R {
        let header = match header {
            Node::FunctionHeader(header) => header,
            n => panic!("Expected a FunctionDecl header, but was {:?}", n),
        };
        // If we don't have a body, use the closing token. A closing token of
        // '}' indicates a regular function, while a closing token of ';'
        // indicates an abstract function.
        let body = if body.is_ignored() { closer } else { body };
        let modifiers = read_member_modifiers(header.modifiers.iter());
        let is_constructor = match header.name {
            Node::Construct(_) => true,
            _ => false,
        };
        let is_method = true;
        let (id, ty, properties) = match self.function_to_ty(is_method, attributes, header, body) {
            Some(tuple) => tuple,
            None => return Node::Ignored(SK::MethodishDeclaration),
        };
        let attributes = self.to_attributes(attributes);
        let deprecated = attributes.deprecated.map(|msg| {
            let mut s = String::new_in(self.state.arena);
            s.push_str("The method ");
            s.push_str(id.1);
            s.push_str(" is deprecated: ");
            s.push_str(msg);
            s.into_bump_str()
        });
        fn get_condition_type_name<'a>(ty_opt: Option<&'a Ty<'a>>) -> Option<&'a str> {
            ty_opt.and_then(|ty| {
                let Ty(_, ty_) = ty;
                match *ty_ {
                    Ty_::Tapply(&(Id(_, class_name), _)) => Some(class_name),
                    _ => None,
                }
            })
        }
        let method = self.alloc(ShallowMethod {
            abstract_: self.state.classish_name_builder.in_interface() || modifiers.is_abstract,
            final_: modifiers.is_final,
            memoizelsb: attributes.memoizelsb,
            name: id,
            override_: attributes.override_,
            reactivity: match attributes.reactivity {
                Reactivity::Local(condition_type) => Some(MethodReactivity::MethodLocal(
                    get_condition_type_name(condition_type),
                )),
                Reactivity::Shallow(condition_type) => Some(MethodReactivity::MethodShallow(
                    get_condition_type_name(condition_type),
                )),
                Reactivity::Reactive(condition_type) => Some(MethodReactivity::MethodReactive(
                    get_condition_type_name(condition_type),
                )),
                Reactivity::Pure(condition_type) => Some(MethodReactivity::MethodPure(
                    get_condition_type_name(condition_type),
                )),
                Reactivity::Nonreactive
                | Reactivity::MaybeReactive(_)
                | Reactivity::RxVar(_)
                | Reactivity::Cipp(_)
                | Reactivity::CippLocal(_)
                | Reactivity::CippGlobal
                | Reactivity::CippRx => None,
            },
            dynamicallycallable: attributes.dynamically_callable,
            type_: ty,
            visibility: modifiers.visibility,
            deprecated,
        });
        if is_constructor {
            Node::Constructor(self.alloc(ConstructorNode { method, properties }))
        } else {
            Node::Method(self.alloc(MethodNode {
                method,
                is_static: modifiers.is_static,
            }))
        }
    }

    fn make_classish_body(
        &mut self,
        _left_brace: Self::R,
        elements: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        Node::ClassishBody(self.alloc(elements.as_slice(self.state.arena)))
    }

    fn make_enum_declaration(
        &mut self,
        attributes: Self::R,
        _keyword: Self::R,
        name: Self::R,
        _colon: Self::R,
        extends: Self::R,
        constraint: Self::R,
        _includes_keyword: Self::R,
        includes: Self::R,
        _left_brace: Self::R,
        cases: Self::R,
        _right_brace: Self::R,
    ) -> Self::R {
        let id = match self.elaborate_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let hint = match self.node_to_ty(extends) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let extends = match self.node_to_ty(self.make_apply(
            Id(self.get_pos(name), "\\HH\\BuiltinEnum"),
            name,
            Pos::none(),
        )) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let key = id.1;
        let consts = self.slice(cases.iter().filter_map(|node| match node {
            Node::ListItem(&(name, value)) => {
                Some(self.alloc(shallow_decl_defs::ShallowClassConst {
                    abstract_: false,
                    name: name.as_id()?,
                    type_: self.infer_const(name, value).unwrap_or_else(|| tany()),
                }))
            }
            n => panic!("Expected an enum case, got {:?}", n),
        }));

        let attributes = attributes;
        let mut user_attributes = Vec::with_capacity_in(attributes.len(), self.state.arena);
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => user_attributes.push(self.user_attribute_to_decl(attr)),
                _ => {}
            }
        }
        // Match ordering of attributes produced by the OCaml decl parser (even
        // though it's the reverse of the syntactic ordering).
        user_attributes.reverse();
        let user_attributes = user_attributes.into_bump_slice();

        let constraint = match constraint {
            Node::TypeConstraint(&(_kind, ty)) => self.node_to_ty(ty),
            _ => None,
        };

        let includes = self.slice(includes.iter().filter_map(|&node| self.node_to_ty(node)));

        let cls = self.alloc(shallow_decl_defs::ShallowClass {
            mode: self.state.file_mode,
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassKind::Cenum,
            name: id,
            tparams: &[],
            where_constraints: &[],
            extends: bumpalo::vec![in self.state.arena; extends].into_bump_slice(),
            uses: &[],
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
            user_attributes,
            enum_type: Some(self.alloc(EnumType {
                base: hint,
                constraint,
                includes,
                enum_class: false,
            })),
        });
        self.add_class(key, cls);
        Node::Ignored(SK::EnumDeclaration)
    }

    fn make_enumerator(
        &mut self,
        name: Self::R,
        _equal: Self::R,
        value: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Node::ListItem(self.alloc((name, value)))
    }

    fn make_tuple_type_specifier(
        &mut self,
        left_paren: Self::R,
        tys: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        // We don't need to include the tys list in this position merging
        // because by definition it's already contained by the two brackets.
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|&node| self.node_to_ty(node)));
        self.hint_ty(pos, Ty_::Ttuple(tys))
    }

    fn make_intersection_type_specifier(
        &mut self,
        left_paren: Self::R,
        tys: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|x| match x {
            Node::ListItem(&(ty, _ampersand)) => self.node_to_ty(ty),
            &x => self.node_to_ty(x),
        }));
        self.hint_ty(pos, Ty_::Tintersection(tys))
    }

    fn make_union_type_specifier(
        &mut self,
        left_paren: Self::R,
        tys: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|x| match x {
            Node::ListItem(&(ty, _bar)) => self.node_to_ty(ty),
            &x => self.node_to_ty(x),
        }));
        self.hint_ty(pos, Ty_::Tunion(tys))
    }

    fn make_shape_type_specifier(
        &mut self,
        shape: Self::R,
        _lparen: Self::R,
        fields: Self::R,
        open: Self::R,
        rparen: Self::R,
    ) -> Self::R {
        let fields = fields;
        let fields_iter = fields.iter();
        let mut fields = AssocListMut::new_in(self.state.arena);
        for node in fields_iter {
            match node {
                &Node::ShapeFieldSpecifier(&ShapeFieldNode { name, type_ }) => {
                    fields.insert(*name, type_)
                }
                n => panic!("Expected a shape field specifier, but was {:?}", n),
            }
        }
        let kind = match open {
            Node::Token(TokenKind::DotDotDot) => ShapeKind::OpenShape,
            _ => ShapeKind::ClosedShape,
        };
        let pos = self.merge_positions(shape, rparen);
        self.hint_ty(pos, Ty_::Tshape(self.alloc((kind, fields.into()))))
    }

    fn make_shape_expression(
        &mut self,
        shape: Self::R,
        _left_paren: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|node| match node {
            Node::ListItem(&(key, value)) => {
                let key = self.make_shape_field_name(key)?;
                let value = self.node_to_expr(value)?;
                Some((key, value))
            }
            n => panic!("Expected a ListItem but was {:?}", n),
        }));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(shape, right_paren),
            nast::Expr_::Shape(fields),
        )))
    }

    fn make_tuple_expression(
        &mut self,
        tuple: Self::R,
        _left_paren: Self::R,
        fields: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let fields = self.slice(fields.iter().filter_map(|&field| self.node_to_expr(field)));
        Node::Expr(self.alloc(aast::Expr(
            self.merge_positions(tuple, right_paren),
            nast::Expr_::List(fields),
        )))
    }

    fn make_classname_type_specifier(
        &mut self,
        classname: Self::R,
        _lt: Self::R,
        targ: Self::R,
        _trailing_comma: Self::R,
        gt: Self::R,
    ) -> Self::R {
        let id = match classname.as_id() {
            Some(id) => id,
            None => return Node::Ignored(SK::ClassnameTypeSpecifier),
        };
        if gt.is_ignored() {
            self.prim_ty(aast::Tprim::Tstring, id.0)
        } else {
            self.make_apply(
                Id(id.0, self.state.namespace_builder.rename_import(id.1)),
                targ,
                self.merge_positions(classname, gt),
            )
        }
    }

    fn make_scope_resolution_expression(
        &mut self,
        class_name: Self::R,
        _operator: Self::R,
        value: Self::R,
    ) -> Self::R {
        let pos = self.merge_positions(class_name, value);
        let Id(class_name_pos, class_name_str) = match self.elaborate_name(class_name) {
            Some(id) => id,
            None => return Node::Ignored(SK::ScopeResolutionExpression),
        };
        let class_id = self.alloc(aast::ClassId(
            class_name_pos,
            match class_name_str.to_ascii_lowercase().as_ref() {
                "\\self" => aast::ClassId_::CIself,
                _ => aast::ClassId_::CI(self.alloc(Id(class_name_pos, class_name_str))),
            },
        ));
        let value_id = match self.expect_name(value) {
            Some(id) => id,
            None => return Node::Ignored(SK::ScopeResolutionExpression),
        };
        Node::Expr(self.alloc(aast::Expr(
            pos,
            nast::Expr_::ClassConst(self.alloc((class_id, self.alloc((value_id.0, value_id.1))))),
        )))
    }

    fn make_field_specifier(
        &mut self,
        question_token: Self::R,
        name: Self::R,
        _arrow: Self::R,
        type_: Self::R,
    ) -> Self::R {
        let optional = question_token.is_present();
        let ty = match self.node_to_ty(type_) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::FieldSpecifier),
        };
        let name = match self.make_shape_field_name(name) {
            Some(name) => name,
            None => return Node::Ignored(SK::FieldSpecifier),
        };
        Node::ShapeFieldSpecifier(self.alloc(ShapeFieldNode {
            name: self.alloc(ShapeField(name)),
            type_: self.alloc(ShapeFieldType { optional, ty }),
        }))
    }

    fn make_field_initializer(&mut self, key: Self::R, _arrow: Self::R, value: Self::R) -> Self::R {
        Node::ListItem(self.alloc((key, value)))
    }

    fn make_varray_type_specifier(
        &mut self,
        varray: Self::R,
        _less_than: Self::R,
        tparam: Self::R,
        _trailing_comma: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        let tparam = match self.node_to_ty(tparam) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::VarrayTypeSpecifier),
        };
        self.hint_ty(
            self.merge_positions(varray, greater_than),
            Ty_::Tvarray(tparam),
        )
    }

    fn make_darray_type_specifier(
        &mut self,
        darray: Self::R,
        _less_than: Self::R,
        key_type: Self::R,
        _comma: Self::R,
        value_type: Self::R,
        _trailing_comma: Self::R,
        greater_than: Self::R,
    ) -> Self::R {
        let pos = self.merge_positions(darray, greater_than);
        let key_type = self.node_to_ty(key_type).unwrap_or(TANY);
        let value_type = self.node_to_ty(value_type).unwrap_or(TANY);
        self.hint_ty(pos, Ty_::Tdarray(self.alloc((key_type, value_type))))
    }

    fn make_old_attribute_specification(
        &mut self,
        ltlt: Self::R,
        attrs: Self::R,
        gtgt: Self::R,
    ) -> Self::R {
        match attrs {
            Node::List(nodes) => {
                Node::BracketedList(self.alloc((self.get_pos(ltlt), nodes, self.get_pos(gtgt))))
            }
            node => panic!(
                "Expected List in old_attribute_specification, but got {:?}",
                node
            ),
        }
    }

    fn make_constructor_call(
        &mut self,
        name: Self::R,
        _left_paren: Self::R,
        args: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        let unqualified_name = match self.expect_name(name) {
            Some(name) => name,
            None => return Node::Ignored(SK::ConstructorCall),
        };
        let name = if unqualified_name.1.starts_with("__") {
            unqualified_name
        } else {
            match self.elaborate_name(name) {
                Some(name) => name,
                None => return Node::Ignored(SK::ConstructorCall),
            }
        };
        let classname_params = self.slice(args.iter().filter_map(|node| match node {
            Node::Expr(aast::Expr(
                _,
                aast::Expr_::ClassConst(&(
                    aast::ClassId(_, aast::ClassId_::CI(&class_name)),
                    (_, "class"),
                )),
            )) => Some(class_name),
            _ => None,
        }));

        let string_literal_params = if match name.1 {
            "__Deprecated" | "__Cipp" | "__CippLocal" => true,
            _ => false,
        } {
            fn fold_string_concat<'a>(expr: &nast::Expr<'a>, acc: &mut Vec<'a, u8>) {
                match expr {
                    &aast::Expr(_, aast::Expr_::String(val)) => acc.extend_from_slice(val),
                    &aast::Expr(_, aast::Expr_::Binop(&(Bop::Dot, e1, e2))) => {
                        fold_string_concat(&e1, acc);
                        fold_string_concat(&e2, acc);
                    }
                    _ => {}
                }
            }

            self.slice(args.iter().filter_map(|expr| match expr {
                Node::StringLiteral((x, _)) => Some(*x),
                Node::Expr(e @ aast::Expr(_, aast::Expr_::Binop(_))) => {
                    let mut acc = Vec::new_in(self.state.arena);
                    fold_string_concat(e, &mut acc);
                    Some(acc.into_bump_slice().into())
                }
                _ => None,
            }))
        } else {
            &[]
        };

        Node::Attribute(self.alloc(UserAttributeNode {
            name,
            classname_params,
            string_literal_params,
        }))
    }

    fn make_trait_use(
        &mut self,
        _keyword: Self::R,
        names: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Node::TraitUse(self.alloc(names))
    }

    fn make_require_clause(
        &mut self,
        _keyword: Self::R,
        require_type: Self::R,
        name: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        Node::RequireClause(self.alloc(RequireClause { require_type, name }))
    }

    fn make_nullable_type_specifier(&mut self, question_mark: Self::R, hint: Self::R) -> Self::R {
        let pos = self.merge_positions(question_mark, hint);
        let ty = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::NullableTypeSpecifier),
        };
        self.hint_ty(pos, Ty_::Toption(ty))
    }

    fn make_like_type_specifier(&mut self, tilde: Self::R, hint: Self::R) -> Self::R {
        let pos = self.merge_positions(tilde, hint);
        let ty = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::LikeTypeSpecifier),
        };
        self.hint_ty(pos, Ty_::Tlike(ty))
    }

    fn make_closure_type_specifier(
        &mut self,
        outer_left_paren: Self::R,
        _function_keyword: Self::R,
        _inner_left_paren: Self::R,
        parameter_list: Self::R,
        _inner_right_paren: Self::R,
        capability: Self::R,
        _colon: Self::R,
        return_type: Self::R,
        outer_right_paren: Self::R,
    ) -> Self::R {
        let make_param = |fp: &'a FunParamDecl<'a>| -> &'a FunParam<'a> {
            let mut flags = FunParamFlags::empty();
            let (hint, mutability) = Self::unwrap_mutability(fp.hint);
            flags |= Self::param_mutability_to_fun_param_flags(mutability);

            match fp.kind {
                ParamMode::FPinout => {
                    flags |= FunParamFlags::INOUT;
                }
                ParamMode::FPnormal => {}
            };

            self.alloc(FunParam {
                pos: self.get_pos(fp.hint),
                name: None,
                type_: self.alloc(PossiblyEnforcedTy {
                    enforced: false,
                    type_: self.node_to_ty(hint).unwrap_or_else(|| tany()),
                }),
                flags,
                rx_annotation: None,
            })
        };

        let arity = parameter_list
            .iter()
            .find_map(|&node| match node {
                Node::FunParam(fp) if fp.variadic => Some(FunArity::Fvariadic(make_param(fp))),
                _ => None,
            })
            .unwrap_or(FunArity::Fstandard);

        let params = self.slice(parameter_list.iter().filter_map(|&node| match node {
            Node::FunParam(fp) if !fp.variadic => Some(make_param(fp)),
            _ => None,
        }));

        let (hint, mutability) = Self::unwrap_mutability(return_type);
        let ret = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::ClosureTypeSpecifier),
        };
        let pos = self.merge_positions(outer_left_paren, outer_right_paren);
        let implicit_params = self.as_fun_implicit_params(capability, pos);

        let mut flags = FunTypeFlags::empty();
        if mutability.is_some() {
            flags |= FunTypeFlags::RETURNS_MUTABLE;
        }

        self.hint_ty(
            pos,
            Ty_::Tfun(self.alloc(FunType {
                arity,
                tparams: &[],
                where_constraints: &[],
                params,
                implicit_params,
                ret: self.alloc(PossiblyEnforcedTy {
                    enforced: false,
                    type_: ret,
                }),
                reactive: Reactivity::Nonreactive,
                flags,
            })),
        )
    }

    fn make_closure_parameter_type_specifier(&mut self, inout: Self::R, hint: Self::R) -> Self::R {
        let kind = match inout {
            Node::Token(TokenKind::Inout) => ParamMode::FPinout,
            _ => ParamMode::FPnormal,
        };
        Node::FunParam(self.alloc(FunParamDecl {
            attributes: Node::Ignored(SK::Missing),
            visibility: Node::Ignored(SK::Missing),
            kind,
            hint,
            id: Id(self.get_pos(hint), "".into()),
            variadic: false,
            initializer: Node::Ignored(SK::Missing),
        }))
    }

    fn make_type_const_declaration(
        &mut self,
        attributes: Self::R,
        modifiers: Self::R,
        _const_keyword: Self::R,
        _type_keyword: Self::R,
        name: Self::R,
        _type_parameters: Self::R,
        constraint: Self::R,
        _equal: Self::R,
        type_: Self::R,
        _semicolon: Self::R,
    ) -> Self::R {
        let attributes = self.to_attributes(attributes);
        let has_abstract_keyword = modifiers.iter().fold(false, |abstract_, node| match node {
            Node::Token(TokenKind::Abstract) => true,
            _ => abstract_,
        });
        let constraint = match constraint {
            Node::TypeConstraint(innards) => self.node_to_ty(innards.1),
            _ => None,
        };
        let type_ = self.node_to_ty(type_);
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
        let name = match name.as_id() {
            Some(name) => name,
            None => return Node::Ignored(SK::TypeConstDeclaration),
        };
        Node::TypeConstant(self.alloc(ShallowTypeconst {
            abstract_,
            constraint,
            name,
            type_,
            enforceable: match attributes.enforceable {
                Some(pos) => (pos, true),
                None => (Pos::none(), false),
            },
            reifiable: attributes.reifiable,
        }))
    }

    fn make_decorated_expression(&mut self, decorator: Self::R, expr: Self::R) -> Self::R {
        Node::ListItem(self.alloc((decorator, expr)))
    }

    fn make_type_constant(
        &mut self,
        ty: Self::R,
        _coloncolon: Self::R,
        constant_name: Self::R,
    ) -> Self::R {
        let id = match self.expect_name(constant_name) {
            Some(id) => id,
            None => return Node::Ignored(SK::TypeConstant),
        };
        let pos = self.merge_positions(ty, constant_name);
        match ty {
            Node::TypeconstAccess(innards) => {
                innards.0.set(pos);
                // Nested typeconst accesses have to be collapsed.
                innards.2.borrow_mut().push(id);
                Node::TypeconstAccess(innards)
            }
            ty => {
                let ty = match (
                    ty,
                    self.state.classish_name_builder.get_current_classish_name(),
                ) {
                    (Node::Name(("self", self_pos)), Some((name, class_name_pos))) => {
                        // In classes, we modify the position when rewriting the
                        // `self` keyword to point to the class name. In traits,
                        // we don't (because traits are not types). We indicate
                        // that the position shouldn't be rewritten with the
                        // none Pos.
                        let id_pos = if class_name_pos.is_none() {
                            self_pos
                        } else {
                            class_name_pos
                        };
                        let reason = self.alloc(Reason::hint(self_pos));
                        let ty_ = Ty_::Tapply(self.alloc((Id(id_pos, name), &[][..])));
                        self.alloc(Ty(reason, ty_))
                    }
                    _ => match self.node_to_ty(ty) {
                        Some(ty) => ty,
                        None => return Node::Ignored(SK::TypeConstant),
                    },
                };
                Node::TypeconstAccess(self.alloc((
                    Cell::new(pos),
                    ty,
                    RefCell::new(bumpalo::vec![in self.state.arena; id]),
                )))
            }
        }
    }

    fn make_soft_type_specifier(&mut self, at_token: Self::R, hint: Self::R) -> Self::R {
        let pos = self.merge_positions(at_token, hint);
        let hint = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::SoftTypeSpecifier),
        };
        // Use the type of the hint as-is (i.e., throw away the knowledge that
        // we had a soft type specifier here--the typechecker does not use it).
        // Replace its Reason with one including the position of the `@` token.
        self.hint_ty(pos, hint.1)
    }

    // A type specifier preceded by an attribute list. At the time of writing,
    // only the <<__Soft>> attribute is permitted here.
    fn make_attributized_specifier(&mut self, attributes: Self::R, hint: Self::R) -> Self::R {
        match attributes {
            Node::BracketedList((
                ltlt_pos,
                [Node::Attribute(UserAttributeNode {
                    name: Id(_, "__Soft"),
                    ..
                })],
                gtgt_pos,
            )) => {
                let attributes_pos = self.merge(*ltlt_pos, *gtgt_pos);
                let hint_pos = self.get_pos(hint);
                // Use the type of the hint as-is (i.e., throw away the
                // knowledge that we had a soft type specifier here--the
                // typechecker does not use it). Replace its Reason with one
                // including the position of the attribute list.
                let hint = match self.node_to_ty(hint) {
                    Some(ty) => ty,
                    None => return Node::Ignored(SK::AttributizedSpecifier),
                };
                self.hint_ty(self.merge(attributes_pos, hint_pos), hint.1)
            }
            _ => hint,
        }
    }

    fn make_vector_type_specifier(
        &mut self,
        vec: Self::R,
        _left_angle: Self::R,
        hint: Self::R,
        _trailing_comma: Self::R,
        right_angle: Self::R,
    ) -> Self::R {
        let id = match self.expect_name(vec) {
            Some(id) => id,
            None => return Node::Ignored(SK::VectorTypeSpecifier),
        };
        let id = Id(id.0, self.state.namespace_builder.rename_import(id.1));
        self.make_apply(id, hint, self.get_pos(right_angle))
    }

    fn make_dictionary_type_specifier(
        &mut self,
        dict: Self::R,
        _left_angle: Self::R,
        type_arguments: Self::R,
        right_angle: Self::R,
    ) -> Self::R {
        let id = match self.expect_name(dict) {
            Some(id) => id,
            None => return Node::Ignored(SK::DictionaryTypeSpecifier),
        };
        let id = Id(id.0, self.state.namespace_builder.rename_import(id.1));
        self.make_apply(id, type_arguments, self.get_pos(right_angle))
    }

    fn make_keyset_type_specifier(
        &mut self,
        keyset: Self::R,
        _left_angle: Self::R,
        hint: Self::R,
        _trailing_comma: Self::R,
        right_angle: Self::R,
    ) -> Self::R {
        let id = match self.expect_name(keyset) {
            Some(id) => id,
            None => return Node::Ignored(SK::KeysetTypeSpecifier),
        };
        let id = Id(id.0, self.state.namespace_builder.rename_import(id.1));
        self.make_apply(id, hint, self.get_pos(right_angle))
    }

    fn make_variable_expression(&mut self, _expression: Self::R) -> Self::R {
        Node::Ignored(SK::VariableExpression)
    }

    fn make_subscript_expression(
        &mut self,
        _receiver: Self::R,
        _left_bracket: Self::R,
        _index: Self::R,
        _right_bracket: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::SubscriptExpression)
    }

    fn make_member_selection_expression(
        &mut self,
        _object: Self::R,
        _operator: Self::R,
        _name: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::MemberSelectionExpression)
    }

    fn make_object_creation_expression(
        &mut self,
        _new_keyword: Self::R,
        _object: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::ObjectCreationExpression)
    }

    fn make_safe_member_selection_expression(
        &mut self,
        _object: Self::R,
        _operator: Self::R,
        _name: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::SafeMemberSelectionExpression)
    }

    fn make_function_call_expression(
        &mut self,
        _receiver: Self::R,
        _type_args: Self::R,
        _left_paren: Self::R,
        _argument_list: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::FunctionCallExpression)
    }

    fn make_list_expression(
        &mut self,
        _keyword: Self::R,
        _left_paren: Self::R,
        _members: Self::R,
        _right_paren: Self::R,
    ) -> Self::R {
        Node::Ignored(SK::ListExpression)
    }
}

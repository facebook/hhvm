// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod direct_decl_smart_constructors_generated;

use std::collections::BTreeMap;
use std::rc::Rc;

use arena_collections::AssocListMut;
use arena_collections::List;
use arena_collections::MultiSetMut;
use bstr::BStr;
use bumpalo::collections as bump;
use bumpalo::Bump;
use flatten_smart_constructors::FlattenSmartConstructors;
use hash::HashSet;
use hh_autoimport_rust as hh_autoimport;
use namespaces::ElaborateKind;
use namespaces_rust as namespaces;
use naming_special_names_rust as naming_special_names;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized_by_ref::aast;
use oxidized_by_ref::ast_defs::Abstraction;
use oxidized_by_ref::ast_defs::Bop;
use oxidized_by_ref::ast_defs::ClassishKind;
use oxidized_by_ref::ast_defs::ConstraintKind;
use oxidized_by_ref::ast_defs::FunKind;
use oxidized_by_ref::ast_defs::Id;
use oxidized_by_ref::ast_defs::Id_;
use oxidized_by_ref::ast_defs::ShapeFieldName;
use oxidized_by_ref::ast_defs::Uop;
use oxidized_by_ref::ast_defs::Variance;
use oxidized_by_ref::ast_defs::XhpEnumValue;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::file_info::Mode;
use oxidized_by_ref::method_flags::MethodFlags;
use oxidized_by_ref::namespace_env::Env as NamespaceEnv;
use oxidized_by_ref::nast;
use oxidized_by_ref::pos::Pos;
use oxidized_by_ref::prop_flags::PropFlags;
use oxidized_by_ref::relative_path::RelativePath;
use oxidized_by_ref::s_map::SMap;
use oxidized_by_ref::shallow_decl_defs;
use oxidized_by_ref::shallow_decl_defs::Decl;
use oxidized_by_ref::shallow_decl_defs::ShallowClassConst;
use oxidized_by_ref::shallow_decl_defs::ShallowMethod;
use oxidized_by_ref::shallow_decl_defs::ShallowProp;
use oxidized_by_ref::shallow_decl_defs::ShallowTypeconst;
use oxidized_by_ref::shape_map::ShapeField;
use oxidized_by_ref::t_shape_map::TShapeField;
use oxidized_by_ref::typing_defs;
use oxidized_by_ref::typing_defs::AbstractTypeconst;
use oxidized_by_ref::typing_defs::Capability::*;
use oxidized_by_ref::typing_defs::ClassConstKind;
use oxidized_by_ref::typing_defs::ClassRefinement;
use oxidized_by_ref::typing_defs::ConcreteTypeconst;
use oxidized_by_ref::typing_defs::ConstDecl;
use oxidized_by_ref::typing_defs::Enforcement;
use oxidized_by_ref::typing_defs::EnumType;
use oxidized_by_ref::typing_defs::FunElt;
use oxidized_by_ref::typing_defs::FunImplicitParams;
use oxidized_by_ref::typing_defs::FunParam;
use oxidized_by_ref::typing_defs::FunParams;
use oxidized_by_ref::typing_defs::FunType;
use oxidized_by_ref::typing_defs::ParamMode;
use oxidized_by_ref::typing_defs::PosByteString;
use oxidized_by_ref::typing_defs::PosId;
use oxidized_by_ref::typing_defs::PosString;
use oxidized_by_ref::typing_defs::PossiblyEnforcedTy;
use oxidized_by_ref::typing_defs::RefinedConst;
use oxidized_by_ref::typing_defs::RefinedConstBound;
use oxidized_by_ref::typing_defs::RefinedConstBounds;
use oxidized_by_ref::typing_defs::ShapeFieldType;
use oxidized_by_ref::typing_defs::ShapeType;
use oxidized_by_ref::typing_defs::TaccessType;
use oxidized_by_ref::typing_defs::Tparam;
use oxidized_by_ref::typing_defs::TshapeFieldName;
use oxidized_by_ref::typing_defs::Ty;
use oxidized_by_ref::typing_defs::Ty_;
use oxidized_by_ref::typing_defs::TypeOrigin;
use oxidized_by_ref::typing_defs::Typeconst;
use oxidized_by_ref::typing_defs::TypedefType;
use oxidized_by_ref::typing_defs::WhereConstraint;
use oxidized_by_ref::typing_defs_flags::FunParamFlags;
use oxidized_by_ref::typing_defs_flags::FunTypeFlags;
use oxidized_by_ref::typing_reason::Reason;
use oxidized_by_ref::xhp_attribute;
use parser_core_types::compact_token::CompactToken;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_kind::SyntaxKind;
use parser_core_types::token_factory::SimpleTokenFactoryImpl;
use parser_core_types::token_kind::TokenKind;

type SK = SyntaxKind;

type SSet<'a> = arena_collections::SortedSet<'a, &'a str>;

#[derive(Clone)]
pub struct DirectDeclSmartConstructors<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> {
    state: Rc<Impl<'a, 'o, 't, S>>,
    pub token_factory: SimpleTokenFactoryImpl<CompactToken>,
    previous_token_kind: TokenKind,
}

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> std::ops::Deref
    for DirectDeclSmartConstructors<'a, 'o, 't, S>
{
    type Target = Impl<'a, 'o, 't, S>;
    fn deref(&self) -> &Self::Target {
        &self.state
    }
}

#[derive(Clone)]
pub struct Impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> {
    pub source_text: IndexedSourceText<'t>,
    pub arena: &'a bumpalo::Bump,
    pub decls: Decls<'a>,
    pub file_attributes: List<'a, &'a typing_defs::UserAttribute<'a>>,

    // const_refs will accumulate all scope-resolution-expressions it
    // encounters while it's "Some"
    const_refs: Option<HashSet<typing_defs::ClassConstRef<'a>>>,

    opts: &'o DeclParserOptions,
    filename: &'a RelativePath<'a>,
    file_mode: Mode,
    namespace_builder: Rc<NamespaceBuilder<'a>>,
    classish_name_builder: Option<ClassishNameBuilder<'a>>,
    type_parameters: Rc<Vec<SSet<'a>>>,
    under_no_auto_dynamic: bool,
    under_no_auto_likes: bool,
    inside_no_auto_dynamic_class: bool,
    source_text_allocator: S,
    module: Option<Id<'a>>,
}

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> DirectDeclSmartConstructors<'a, 'o, 't, S> {
    pub fn new(
        opts: &'o DeclParserOptions,
        src: &SourceText<'t>,
        file_mode: Mode,
        arena: &'a Bump,
        source_text_allocator: S,
        elaborate_xhp_namespaces_for_facts: bool,
    ) -> Self {
        let source_text = IndexedSourceText::new(src.clone());
        let path = source_text.source_text().file_path();
        let prefix = path.prefix();
        let path = bump::String::from_str_in(path.path_str(), arena).into_bump_str();
        let filename = RelativePath::make(prefix, path);
        Self {
            state: Rc::new(Impl {
                source_text,
                arena,
                filename: arena.alloc(filename),
                file_mode,
                decls: Decls::empty(),
                file_attributes: List::empty(),
                const_refs: None,
                namespace_builder: Rc::new(NamespaceBuilder::new_in(
                    &opts.auto_namespace_map,
                    opts.disable_xhp_element_mangling,
                    elaborate_xhp_namespaces_for_facts,
                    arena,
                )),
                opts,
                classish_name_builder: None,
                type_parameters: Rc::new(Vec::new()),
                source_text_allocator,
                under_no_auto_dynamic: false,
                under_no_auto_likes: false,
                inside_no_auto_dynamic_class: false,
                module: None,
            }),
            token_factory: SimpleTokenFactoryImpl::new(),
            // EndOfFile is used here as a None value (signifying "beginning of
            // file") to save space. There is no legitimate circumstance where
            // we would parse a token and the previous token kind would be
            // EndOfFile.
            previous_token_kind: TokenKind::EndOfFile,
        }
    }

    fn qualified_name_from_parts(&self, parts: &'a [Node<'a>], pos: &'a Pos<'a>) -> Id<'a> {
        // Count the length of the qualified name, so that we can allocate
        // exactly the right amount of space for it in our arena.
        let mut len = 0;
        for part in parts {
            match part {
                Node::Name(&(name, _)) => len += name.len(),
                Node::Token(t) if t.kind() == TokenKind::Backslash => len += 1,
                Node::ListItem(&(Node::Name(&(name, _)), _backslash)) => len += name.len() + 1,
                Node::ListItem(&(Node::Token(t), _backslash))
                    if t.kind() == TokenKind::Namespace =>
                {
                    len += t.width() + 1;
                }
                _ => {}
            }
        }
        // If there's no internal trivia, then we can just reference the
        // qualified name in the original source text instead of copying it.
        let source_len = pos.end_offset() - pos.start_offset();
        if source_len == len {
            let qualified_name: &'a str = self.str_from_utf8(self.source_text_at_pos(pos));
            return Id(pos, qualified_name);
        }
        // Allocate `len` bytes and fill them with the fully qualified name.
        let mut qualified_name = bump::String::with_capacity_in(len, self.arena);
        for part in parts {
            match part {
                Node::Name(&(name, _pos)) => qualified_name.push_str(name),
                Node::Token(t) if t.kind() == TokenKind::Backslash => qualified_name.push('\\'),
                &Node::ListItem(&(Node::Name(&(name, _)), _backslash)) => {
                    qualified_name.push_str(name);
                    qualified_name.push_str("\\");
                }
                &Node::ListItem(&(Node::Token(t), _backslash))
                    if t.kind() == TokenKind::Namespace =>
                {
                    qualified_name.push_str("namespace\\");
                }
                _ => {}
            }
        }
        debug_assert_eq!(len, qualified_name.len());
        debug_assert_eq!(len, qualified_name.capacity());
        Id(pos, qualified_name.into_bump_str())
    }

    fn module_name_string_from_parts(&self, parts: &'a [Node<'a>], pos: &'a Pos<'a>) -> &'a str {
        // Count the length of the qualified name, so that we can allocate
        // exactly the right amount of space for it in our arena.
        let mut len = 0;
        for part in parts {
            match part {
                Node::Name(&(name, _)) => len += name.len(),
                Node::ListItem(&(Node::Name(&(name, _)), _dot)) => len += name.len() + 1,
                _ => {}
            }
        }
        // If there's no internal trivia, then we can just reference the
        // qualified name in the original source text instead of copying it.
        let source_len = pos.end_offset() - pos.start_offset();
        if source_len == len {
            return self.str_from_utf8(self.source_text_at_pos(pos));
        }
        // Allocate `len` bytes and fill them with the fully qualified name.
        let mut qualified_name = bump::String::with_capacity_in(len, self.arena);
        for part in parts {
            match part {
                Node::Name(&(name, _pos)) => qualified_name.push_str(name),
                &Node::ListItem(&(Node::Name(&(name, _)), _)) => {
                    qualified_name.push_str(name);
                    qualified_name.push_str(".");
                }
                _ => {}
            }
        }
        debug_assert_eq!(len, qualified_name.len());
        debug_assert_eq!(len, qualified_name.capacity());
        qualified_name.into_bump_str()
    }

    fn module_reference_from_parts(
        &self,
        module_name: &'a str,
        parts: &'a [Node<'a>],
    ) -> shallow_decl_defs::ModuleReference<'a> {
        let mut s = bump::String::new_in(self.arena);

        for part in parts.iter() {
            match part {
                Node::ListItem(&(item, _)) => {
                    if !s.is_empty() {
                        s += ".";
                    }

                    match item {
                        Node::Name(&(n, _)) => {
                            if n == "self" {
                                s += module_name;
                            } else {
                                s += n;
                            }
                        }
                        _ => {}
                    }
                }
                Node::Token(t) => match t.kind() {
                    TokenKind::Global => {
                        return shallow_decl_defs::ModuleReference::MRGlobal;
                    }
                    TokenKind::Star => {
                        return shallow_decl_defs::ModuleReference::MRPrefix(s.into_bump_str());
                    }
                    _ => {}
                },
                Node::Name(&(n, _)) => {
                    if !s.is_empty() {
                        s += ".";
                    }
                    s += n;
                }
                _ => {}
            }
        }
        shallow_decl_defs::ModuleReference::MRExact(s.into_bump_str())
    }

    /// If the given node is an identifier, XHP name, or qualified name,
    /// elaborate it in the current namespace and return Some. To be used for
    /// the name of a decl in its definition (e.g., "C" in `class C {}` or "f"
    /// in `function f() {}`).
    fn elaborate_defined_id(&self, name: Node<'a>) -> Option<Id<'a>> {
        let id = match name {
            Node::Name(&(name, pos)) => Id(pos, name),
            Node::XhpName(&(name, pos)) => Id(pos, name),
            Node::QualifiedName(&(parts, pos)) => self.qualified_name_from_parts(parts, pos),
            // This is always an error; e.g. using a reserved word where a name
            // is expected.
            Node::Token(t) | Node::IgnoredToken(t) => {
                let pos = self.token_pos(t);
                let text = self.str_from_utf8(self.source_text_at_pos(pos));
                Id(pos, text)
            }
            _ => return None,
        };
        Some(self.namespace_builder.elaborate_defined_id(id))
    }

    /// If the given node is a name (i.e., an identifier or a qualified name),
    /// return Some. No namespace elaboration is performed.
    fn expect_name(&self, name: Node<'a>) -> Option<Id<'a>> {
        // If it's a simple identifier, return it.
        if let id @ Some(_) = name.as_id() {
            return id;
        }
        match name {
            Node::QualifiedName(&(parts, pos)) => Some(self.qualified_name_from_parts(parts, pos)),
            // The IgnoredToken case is always an error; e.g. using a reserved
            // word where a name is expected. The Token case is not an error if
            // the token is TokenKind::XHP (which is legal to use as a name),
            // but an error otherwise (since we expect a Name or QualifiedName
            // here, and the Name case would have been handled in `as_id`
            // above).
            Node::Token(t) | Node::IgnoredToken(t) => {
                let pos = self.token_pos(t);
                let text = self.str_from_utf8(self.source_text_at_pos(pos));
                Some(Id(pos, text))
            }
            _ => None,
        }
    }

    /// Fully qualify the given identifier as a type name (with consideration
    /// to `use` statements in scope).
    fn elaborate_id(&self, id: Id<'a>) -> Id<'a> {
        let Id(pos, name) = id;
        Id(pos, self.elaborate_raw_id(name))
    }

    /// Fully qualify the given identifier as a type name (with consideration
    /// to `use` statements in scope).
    fn elaborate_raw_id(&self, id: &'a str) -> &'a str {
        self.namespace_builder
            .elaborate_raw_id(ElaborateKind::Class, id)
    }

    fn fold_string_concat(&self, expr: &nast::Expr<'a>, acc: &mut bump::Vec<'a, u8>) -> bool {
        match *expr {
            aast::Expr(
                _,
                _,
                aast::Expr_::ClassConst((
                    aast::ClassId(_, _, aast::ClassId_::CI(&Id(_, class_name))),
                    _,
                )),
            ) => {
                // Imagine the case <<MyFancyEnum('foo'.X::class)>>
                // We would expect a user attribute parameter to concatenate
                // and return a string of 'fooX'.
                // Since `X::class` after elaboration returns the string '\X'
                // we opt to strip the prefix here to successfully concatenate the string
                // into human-readable formats.
                // The Facts parser handles this for AttributeParam::Classname
                // but not AttributeParam::String
                let mut name = self.elaborate_raw_id(class_name);
                if name.starts_with('\\') {
                    name = &name[1..];
                }
                acc.extend_from_slice(name.as_bytes());
                true
            }
            aast::Expr(_, _, aast::Expr_::String(val)) => {
                acc.extend_from_slice(val);
                true
            }
            aast::Expr(
                _,
                _,
                aast::Expr_::Binop(&aast::Binop {
                    bop: Bop::Dot,
                    lhs,
                    rhs,
                }),
            ) => self.fold_string_concat(lhs, acc) && self.fold_string_concat(rhs, acc),
            _ => false,
        }
    }

    /// Fully qualify the given identifier as a constant name (with
    /// consideration to `use` statements in scope).
    fn elaborate_const_id(&self, id: Id<'a>) -> Id<'a> {
        let Id(pos, name) = id;
        Id(
            pos,
            self.namespace_builder
                .elaborate_raw_id(ElaborateKind::Const, name),
        )
    }

    fn start_accumulating_const_refs(&mut self) {
        let this = Rc::make_mut(&mut self.state);
        this.const_refs = Some(Default::default());
    }

    fn accumulate_const_ref(&mut self, class_id: &'a aast::ClassId<'_, (), ()>, value_id: &Id<'a>) {
        let this = Rc::make_mut(&mut self.state);
        // The decl for a class constant stores a list of all the scope-resolution expressions
        // it contains. For example "const C=A::X" stores A::X, and "const D=self::Y" stores self::Y.
        // (This is so we can detect cross-type circularity in constant initializers).
        // TODO: Hack is the wrong place to detect circularity (because we can never do
        // it completely soundly, and because it's a cross-body problem). The right place
        // to do it is in a linter. All this should be removed from here and put into a linter.
        if let Some(const_refs) = &mut this.const_refs {
            match class_id.2 {
                nast::ClassId_::CI(sid) => {
                    const_refs.insert(typing_defs::ClassConstRef(
                        typing_defs::ClassConstFrom::From(sid.1),
                        value_id.1,
                    ));
                }
                nast::ClassId_::CIself => {
                    const_refs.insert(typing_defs::ClassConstRef(
                        typing_defs::ClassConstFrom::Self_,
                        value_id.1,
                    ));
                }
                nast::ClassId_::CIparent | nast::ClassId_::CIstatic | nast::ClassId_::CIexpr(_) => {
                    // Not allowed
                }
            }
        }
    }

    fn stop_accumulating_const_refs(&mut self) -> &'a [typing_defs::ClassConstRef<'a>] {
        let this = Rc::make_mut(&mut self.state);
        match this.const_refs.take() {
            Some(const_refs) => {
                let mut elements: bump::Vec<'_, typing_defs::ClassConstRef<'_>> =
                    bumpalo::collections::Vec::with_capacity_in(const_refs.len(), self.arena);
                elements.extend(const_refs.into_iter());
                elements.sort_unstable();
                elements.into_bump_slice()
            }
            None => &[],
        }
    }
}

pub trait SourceTextAllocator<'s, 'd>: Clone {
    fn alloc(&self, text: &'s str) -> &'d str;
}

#[derive(Clone)]
pub struct NoSourceTextAllocator;

impl<'t> SourceTextAllocator<'t, 't> for NoSourceTextAllocator {
    #[inline]
    fn alloc(&self, text: &'t str) -> &'t str {
        text
    }
}

#[derive(Clone)]
pub struct ArenaSourceTextAllocator<'a>(pub &'a bumpalo::Bump);

impl<'t, 'a> SourceTextAllocator<'t, 'a> for ArenaSourceTextAllocator<'a> {
    #[inline]
    fn alloc(&self, text: &'t str) -> &'a str {
        self.0.alloc_str(text)
    }
}

fn prefix_slash<'a>(arena: &'a Bump, name: &str) -> &'a str {
    let mut s = bump::String::with_capacity_in(1 + name.len(), arena);
    s.push('\\');
    s.push_str(name);
    s.into_bump_str()
}

fn prefix_colon<'a>(arena: &'a Bump, name: &str) -> &'a str {
    let mut s = bump::String::with_capacity_in(1 + name.len(), arena);
    s.push(':');
    s.push_str(name);
    s.into_bump_str()
}

fn concat<'a>(arena: &'a Bump, str1: &str, str2: &str) -> &'a str {
    let mut result = bump::String::with_capacity_in(str1.len() + str2.len(), arena);
    result.push_str(str1);
    result.push_str(str2);
    result.into_bump_str()
}

fn strip_dollar_prefix<'a>(name: &'a str) -> &'a str {
    name.trim_start_matches('$')
}

fn is_no_auto_attribute(name: &str) -> bool {
    name == "__NoAutoDynamic" || name == "__NoAutoLikes"
}

const TANY_: Ty_<'_> = Ty_::Tany(oxidized_by_ref::tany_sentinel::TanySentinel);
const TANY: &Ty<'_> = &Ty(Reason::none(), TANY_);

const NO_POS: &Pos<'_> = Pos::none();

#[derive(Debug)]
struct Modifiers {
    is_static: bool,
    visibility: aast::Visibility,
    is_abstract: bool,
    is_final: bool,
    is_readonly: bool,
}

fn read_member_modifiers<'a: 'b, 'b>(modifiers: impl Iterator<Item = &'b Node<'a>>) -> Modifiers {
    let mut ret = Modifiers {
        is_static: false,
        visibility: aast::Visibility::Public,
        is_abstract: false,
        is_final: false,
        is_readonly: false,
    };
    for modifier in modifiers {
        if let Some(vis) = modifier.as_visibility() {
            ret.visibility = vis;
        }
        match modifier.token_kind() {
            Some(TokenKind::Static) => ret.is_static = true,
            Some(TokenKind::Abstract) => ret.is_abstract = true,
            Some(TokenKind::Final) => ret.is_final = true,
            Some(TokenKind::Readonly) => ret.is_readonly = true,
            _ => {}
        }
    }
    ret
}

#[derive(Clone, Debug)]
struct NamespaceBuilder<'a> {
    arena: &'a Bump,
    stack: Vec<NamespaceEnv<'a>>,
    elaborate_xhp_namespaces_for_facts: bool,
}

impl<'a> NamespaceBuilder<'a> {
    fn new_in(
        auto_ns_map: &[(String, String)],
        disable_xhp_element_mangling: bool,
        elaborate_xhp_namespaces_for_facts: bool,
        arena: &'a Bump,
    ) -> Self {
        // Copy auto_namespace_map entries into the arena so decls can use them.
        let auto_ns_map = arena.alloc_slice_fill_iter(
            auto_ns_map
                .iter()
                .map(|(n, v)| (arena.alloc_str(n) as &str, arena.alloc_str(v) as &str)),
        );

        let mut ns_uses = SMap::empty();
        for &alias in hh_autoimport::NAMESPACES {
            ns_uses = ns_uses.add(arena, alias, concat(arena, "HH\\", alias));
        }
        for (alias, ns) in auto_ns_map.iter() {
            ns_uses = ns_uses.add(arena, alias, ns);
        }

        let mut class_uses = SMap::empty();
        for &alias in hh_autoimport::TYPES {
            class_uses = class_uses.add(arena, alias, concat(arena, "HH\\", alias));
        }

        Self {
            arena,
            stack: vec![NamespaceEnv {
                ns_uses,
                class_uses,
                fun_uses: SMap::empty(),
                const_uses: SMap::empty(),
                name: None,
                is_codegen: false,
                disable_xhp_element_mangling,
            }],
            elaborate_xhp_namespaces_for_facts,
        }
    }

    fn push_namespace(&mut self, name: Option<&str>) {
        let current = self.current_namespace();
        let nsenv = self.stack.last().unwrap().clone(); // shallow clone
        if let Some(name) = name {
            let mut fully_qualified = match current {
                None => bump::String::with_capacity_in(name.len(), self.arena),
                Some(current) => {
                    let mut fully_qualified =
                        bump::String::with_capacity_in(current.len() + name.len() + 1, self.arena);
                    fully_qualified.push_str(current);
                    fully_qualified.push('\\');
                    fully_qualified
                }
            };
            fully_qualified.push_str(name);
            self.stack.push(NamespaceEnv {
                name: Some(fully_qualified.into_bump_str()),
                ..nsenv
            });
        } else {
            self.stack.push(NamespaceEnv {
                name: current,
                ..nsenv
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
            let last = self.stack.pop().unwrap().name.unwrap_or("\\");
            let previous = self.stack.pop().unwrap().name.unwrap_or("\\");
            assert!(last.starts_with(previous));
            let name = &last[previous.len() + 1..last.len()];
            self.push_namespace(Some(name));
        }
    }

    fn current_namespace(&self) -> Option<&'a str> {
        self.stack.last().and_then(|nsenv| nsenv.name)
    }

    fn add_import(&mut self, kind: NamespaceUseKind, name: &'a str, aliased_name: Option<&'a str>) {
        let stack_top = &mut self
            .stack
            .last_mut()
            .expect("Attempted to get the current import map, but namespace stack was empty");
        let aliased_name = aliased_name.unwrap_or_else(|| {
            name.rsplit_terminator('\\')
                .next()
                .expect("Expected at least one entry in import name")
        });
        let name = name.trim_end_matches('\\');
        let name = if name.starts_with('\\') {
            name
        } else {
            prefix_slash(self.arena, name)
        };
        match kind {
            NamespaceUseKind::Type => {
                stack_top.class_uses = stack_top.class_uses.add(self.arena, aliased_name, name);
            }
            NamespaceUseKind::Namespace => {
                stack_top.ns_uses = stack_top.ns_uses.add(self.arena, aliased_name, name);
            }
            NamespaceUseKind::Mixed => {
                stack_top.class_uses = stack_top.class_uses.add(self.arena, aliased_name, name);
                stack_top.ns_uses = stack_top.ns_uses.add(self.arena, aliased_name, name);
            }
        }
    }

    fn elaborate_raw_id(&self, kind: ElaborateKind, name: &'a str) -> &'a str {
        if name.starts_with('\\') {
            return name;
        }
        let env = self.stack.last().unwrap();
        namespaces::elaborate_raw_id_in(
            self.arena,
            env,
            kind,
            name,
            self.elaborate_xhp_namespaces_for_facts,
        )
    }

    fn elaborate_defined_id(&self, id: Id<'a>) -> Id<'a> {
        let Id(pos, name) = id;
        let env = self.stack.last().unwrap();
        let name = if env.disable_xhp_element_mangling && name.contains(':') {
            let xhp_name_opt = namespaces::elaborate_xhp_namespace(name);
            let name = xhp_name_opt.map_or(name, |s| self.arena.alloc_str(&s));
            if !name.starts_with('\\') {
                namespaces::elaborate_into_current_ns_in(self.arena, env, name)
            } else if self.elaborate_xhp_namespaces_for_facts {
                // allow :foo:bar to be elaborated into \currentnamespace\foo\bar
                namespaces::elaborate_into_current_ns_in(self.arena, env, &name[1..])
            } else {
                name
            }
        } else {
            namespaces::elaborate_into_current_ns_in(self.arena, env, name)
        };
        Id(pos, name)
    }
}

/// We saw a classish keyword token followed by a Name, so we make it
/// available as the name of the containing class declaration.
#[derive(Clone, Debug)]
struct ClassishNameBuilder<'a> {
    name: &'a str,
    pos: &'a Pos<'a>,
    token_kind: TokenKind,
}

#[derive(Debug)]
pub struct FunParamDecl<'a> {
    attributes: Node<'a>,
    visibility: Node<'a>,
    kind: ParamMode,
    readonly: bool,
    hint: Node<'a>,
    pos: &'a Pos<'a>,
    name: Option<&'a str>,
    variadic: bool,
    initializer: Node<'a>,
}

#[derive(Debug)]
pub struct FunctionHeader<'a> {
    name: Node<'a>,
    modifiers: Node<'a>,
    type_params: Node<'a>,
    param_list: Node<'a>,
    capability: Node<'a>,
    ret_hint: Node<'a>,
    readonly_return: Node<'a>,
    where_constraints: Node<'a>,
}

#[derive(Debug)]
pub struct RequireClause<'a> {
    require_type: Node<'a>,
    name: Node<'a>,
}

#[derive(Debug)]
pub struct TypeParameterDecl<'a> {
    name: Node<'a>,
    reified: aast::ReifyKind,
    variance: Variance,
    constraints: &'a [(ConstraintKind, Node<'a>)],
    tparam_params: &'a [&'a Tparam<'a>],
    user_attributes: &'a [&'a UserAttributeNode<'a>],
}

#[derive(Debug)]
pub struct ClosureTypeHint<'a> {
    #[allow(dead_code)]
    args: Node<'a>,
    #[allow(dead_code)]
    ret_hint: Node<'a>,
}

#[derive(Debug)]
pub struct NamespaceUseClause<'a> {
    kind: NamespaceUseKind,
    id: Id<'a>,
    as_: Option<&'a str>,
}

#[derive(Copy, Clone, Debug)]
enum NamespaceUseKind {
    Type,
    Namespace,
    Mixed,
}

#[derive(Debug)]
pub struct ConstructorNode<'a> {
    method: &'a ShallowMethod<'a>,
    properties: &'a [ShallowProp<'a>],
}

#[derive(Debug)]
pub struct MethodNode<'a> {
    method: &'a ShallowMethod<'a>,
    is_static: bool,
}

#[derive(Debug)]
pub struct PropertyNode<'a> {
    decls: &'a [ShallowProp<'a>],
    is_static: bool,
}

#[derive(Debug)]
pub struct XhpClassAttributeDeclarationNode<'a> {
    xhp_attr_enum_values: &'a [(&'a str, &'a [XhpEnumValue<'a>])],
    xhp_attr_decls: &'a [ShallowProp<'a>],
    xhp_attr_uses_decls: &'a [Node<'a>],
}

#[derive(Debug)]
pub struct XhpClassAttributeNode<'a> {
    name: Id<'a>,
    tag: Option<xhp_attribute::Tag>,
    needs_init: bool,
    nullable: bool,
    hint: Node<'a>,
}

#[derive(Debug)]
pub struct ShapeFieldNode<'a> {
    name: &'a ShapeField<'a>,
    type_: &'a ShapeFieldType<'a>,
}

#[derive(Copy, Clone, Debug)]
enum AttributeParam<'a> {
    Classname(Id<'a>),
    EnumClassLabel(&'a Id_<'a>),
    String(&'a Pos<'a>, &'a BStr),
    Int(&'a Id_<'a>),
}

#[derive(Debug)]
pub struct UserAttributeNode<'a> {
    name: Id<'a>,
    params: &'a [AttributeParam<'a>],
    /// This is only used for __Deprecated attribute message and CIPP parameters
    string_literal_param: Option<(&'a Pos<'a>, &'a BStr)>,
}

mod fixed_width_token {
    use parser_core_types::token_kind::TokenKind;

    #[derive(Copy, Clone)]
    pub struct FixedWidthToken(u64); // { offset: u56, kind: TokenKind }

    const KIND_BITS: u8 = 8;
    const KIND_MASK: u64 = u8::MAX as u64;
    const MAX_OFFSET: u64 = !(KIND_MASK << (64 - KIND_BITS));

    impl FixedWidthToken {
        pub fn new(kind: TokenKind, offset: usize) -> Self {
            // We don't want to spend bits tracking the width of fixed-width
            // tokens. Since we don't track width, verify that this token kind
            // is in fact a fixed-width kind.
            debug_assert!(kind.fixed_width().is_some());

            let offset: u64 = offset.try_into().unwrap();
            if offset > MAX_OFFSET {
                panic!("FixedWidthToken: offset too large: {}", offset);
            }
            Self(offset << KIND_BITS | kind as u8 as u64)
        }

        pub fn offset(self) -> usize {
            (self.0 >> KIND_BITS).try_into().unwrap()
        }

        pub fn kind(self) -> TokenKind {
            TokenKind::try_from_u8(self.0 as u8).unwrap()
        }

        pub fn width(self) -> usize {
            self.kind().fixed_width().unwrap().get()
        }
    }

    impl std::fmt::Debug for FixedWidthToken {
        fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            fmt.debug_struct("FixedWidthToken")
                .field("kind", &self.kind())
                .field("offset", &self.offset())
                .finish()
        }
    }
}
use fixed_width_token::FixedWidthToken;

#[derive(Copy, Clone, Debug)]
pub enum XhpChildrenKind {
    Empty,
    Other,
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

    // For tokens with a fixed width (like `using`), we also keep its offset in
    // the source text, so that we can reference the text of the token if it's
    // (erroneously) used as an identifier (e.g., `function using() {}`).
    IgnoredToken(FixedWidthToken),

    List(&'a &'a [Node<'a>]),
    BracketedList(&'a (&'a Pos<'a>, &'a [Node<'a>], &'a Pos<'a>)),
    Name(&'a (&'a str, &'a Pos<'a>)),
    XhpName(&'a (&'a str, &'a Pos<'a>)),
    Variable(&'a (&'a str, &'a Pos<'a>)),
    QualifiedName(&'a (&'a [Node<'a>], &'a Pos<'a>)),
    ModuleName(&'a (&'a [Node<'a>], &'a Pos<'a>)),
    StringLiteral(&'a (&'a BStr, &'a Pos<'a>)), // For shape keys and const expressions.
    IntLiteral(&'a (&'a str, &'a Pos<'a>)),     // For const expressions.
    FloatingLiteral(&'a (&'a str, &'a Pos<'a>)), // For const expressions.
    BooleanLiteral(&'a (&'a str, &'a Pos<'a>)), // For const expressions.
    Ty(&'a Ty<'a>),
    XhpEnumTy(&'a (Option<&'a Pos<'a>>, &'a Ty<'a>, &'a [XhpEnumValue<'a>])),
    ListItem(&'a (Node<'a>, Node<'a>)),

    // For the "X=1" in enums "enum E {X=1}" and enum-classes "enum class C {int X=1}",
    // and also for consts via make_const_declaration
    Const(&'a ShallowClassConst<'a>),

    // Stores (X,1,refs) for "X=1" in top-level "const int X=1" and
    // class-const "public const int X=1".
    ConstInitializer(&'a (Node<'a>, Node<'a>, &'a [typing_defs::ClassConstRef<'a>])),

    FunParam(&'a FunParamDecl<'a>),
    Attribute(&'a UserAttributeNode<'a>),
    FunctionHeader(&'a FunctionHeader<'a>),
    Constructor(&'a ConstructorNode<'a>),
    Method(&'a MethodNode<'a>),
    Property(&'a PropertyNode<'a>),
    EnumUse(&'a Node<'a>),
    TraitUse(&'a Node<'a>),
    XhpClassAttributeDeclaration(&'a XhpClassAttributeDeclarationNode<'a>),
    XhpClassAttribute(&'a XhpClassAttributeNode<'a>),
    XhpAttributeUse(&'a Node<'a>),
    XhpChildrenDeclaration(XhpChildrenKind),
    TypeConstant(&'a ShallowTypeconst<'a>),
    ContextConstraint(&'a (ConstraintKind, Node<'a>)),
    RequireClause(&'a RequireClause<'a>),
    ClassishBody(&'a &'a [Node<'a>]),
    TypeParameter(&'a TypeParameterDecl<'a>),
    TypeConstraint(&'a (ConstraintKind, Node<'a>)),
    ShapeFieldSpecifier(&'a ShapeFieldNode<'a>),
    NamespaceUseClause(&'a NamespaceUseClause<'a>),
    Expr(&'a nast::Expr<'a>),
    TypeParameters(&'a &'a [&'a Tparam<'a>]),
    WhereConstraint(&'a WhereConstraint<'a>),
    RefinedConst(&'a (&'a str, RefinedConst<'a>)),
    EnumClassLabel(&'a str),

    // Non-ignored, fixed-width tokens (e.g., keywords, operators, braces, etc.).
    Token(FixedWidthToken),
}

impl<'a> smart_constructors::NodeType for Node<'a> {
    type Output = Node<'a>;

    fn extract(self) -> Self::Output {
        self
    }

    fn is_abstract(&self) -> bool {
        self.is_token(TokenKind::Abstract) || self.is_ignored_token_with_kind(TokenKind::Abstract)
    }
    fn is_name(&self) -> bool {
        matches!(self, Node::Name(..)) || self.is_ignored_token_with_kind(TokenKind::Name)
    }
    fn is_qualified_name(&self) -> bool {
        matches!(self, Node::QualifiedName(..)) || matches!(self, Node::Ignored(SK::QualifiedName))
    }
    fn is_prefix_unary_expression(&self) -> bool {
        matches!(self, Node::Expr(aast::Expr(_, _, aast::Expr_::Unop(..))))
            || matches!(self, Node::Ignored(SK::PrefixUnaryExpression))
    }
    fn is_scope_resolution_expression(&self) -> bool {
        matches!(
            self,
            Node::Expr(aast::Expr(_, _, aast::Expr_::ClassConst(..)))
        ) || matches!(self, Node::Ignored(SK::ScopeResolutionExpression))
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
    fn is_token(self, kind: TokenKind) -> bool {
        self.token_kind() == Some(kind)
    }

    fn token_kind(self) -> Option<TokenKind> {
        match self {
            Node::Token(token) => Some(token.kind()),
            _ => None,
        }
    }

    fn is_ignored_token_with_kind(self, kind: TokenKind) -> bool {
        match self {
            Node::IgnoredToken(token) => token.kind() == kind,
            _ => false,
        }
    }

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
        match self.token_kind() {
            Some(TokenKind::Private) => Some(aast::Visibility::Private),
            Some(TokenKind::Protected) => Some(aast::Visibility::Protected),
            Some(TokenKind::Public) => Some(aast::Visibility::Public),
            Some(TokenKind::Internal) => Some(aast::Visibility::Internal),
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

    // If this node is a Variable token, return its position and text.
    // As an attempt at error recovery (when the dollar sign is omitted), also
    // return other unqualified identifiers (i.e., the Name token kind).
    fn as_variable(&self) -> Option<Id<'a>> {
        match self {
            Node::Variable(&(name, pos)) | Node::Name(&(name, pos)) => Some(Id(pos, name)),
            _ => None,
        }
    }

    fn is_ignored(&self) -> bool {
        matches!(self, Node::Ignored(..) | Node::IgnoredToken(..))
    }

    fn is_present(&self) -> bool {
        !self.is_ignored()
    }

    fn contains_marker_attribute(&self, name: &str) -> bool {
        self.iter().any(|node| match node {
            Node::Attribute(&UserAttributeNode {
                name: Id(_pos, attr_name),
                params: [],
                string_literal_param: None,
            }) => attr_name == name,
            _ => false,
        })
    }
}

#[derive(Debug)]
struct Attributes<'a> {
    deprecated: Option<&'a str>,
    reifiable: Option<&'a Pos<'a>>,
    late_init: bool,
    const_: bool,
    lsb: bool,
    memoize: bool,
    memoizelsb: bool,
    override_: bool,
    enforceable: Option<&'a Pos<'a>>,
    accept_disposable: bool,
    dynamically_callable: bool,
    returns_disposable: bool,
    php_std_lib: bool,
    soft: bool,
    support_dynamic_type: bool,
    no_auto_likes: bool,
    safe_global_variable: bool,
    cross_package: Option<&'a str>,
    sort_text: Option<&'a str>,
    dynamically_referenced: bool,
}

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> Impl<'a, 'o, 't, S> {
    fn add_class(&mut self, name: &'a str, decl: &'a shallow_decl_defs::ShallowClass<'a>) {
        self.under_no_auto_dynamic = false;
        self.under_no_auto_likes = false;
        self.inside_no_auto_dynamic_class = false;
        self.decls.add(name, Decl::Class(decl), self.arena);
    }
    fn add_fun(&mut self, name: &'a str, decl: &'a typing_defs::FunElt<'a>) {
        self.under_no_auto_dynamic = false;
        self.under_no_auto_likes = false;
        self.decls.add(name, Decl::Fun(decl), self.arena);
    }
    fn add_typedef(&mut self, name: &'a str, decl: &'a typing_defs::TypedefType<'a>) {
        self.under_no_auto_dynamic = false;
        self.under_no_auto_likes = false;
        self.decls.add(name, Decl::Typedef(decl), self.arena);
    }
    fn add_const(&mut self, name: &'a str, decl: &'a typing_defs::ConstDecl<'a>) {
        self.under_no_auto_dynamic = false;
        self.under_no_auto_likes = false;
        self.decls.add(name, Decl::Const(decl), self.arena);
    }
    fn add_module(&mut self, name: &'a str, decl: &'a typing_defs::ModuleDefType<'a>) {
        self.under_no_auto_dynamic = false;
        self.under_no_auto_likes = false;
        self.decls.add(name, Decl::Module(decl), self.arena)
    }

    #[inline(always)]
    pub fn alloc<T>(&self, val: T) -> &'a T {
        self.arena.alloc(val)
    }

    fn slice<T>(&self, iter: impl Iterator<Item = T>) -> &'a [T] {
        let mut result = match iter.size_hint().1 {
            Some(upper_bound) => bump::Vec::with_capacity_in(upper_bound, self.arena),
            None => bump::Vec::new_in(self.arena),
        };
        for item in iter {
            result.push(item);
        }
        result.into_bump_slice()
    }

    fn user_attribute_to_decl(
        &self,
        attr: &UserAttributeNode<'a>,
    ) -> &'a shallow_decl_defs::UserAttribute<'a> {
        use shallow_decl_defs::UserAttributeParam as UAP;
        self.alloc(shallow_decl_defs::UserAttribute {
            name: attr.name.into(),
            params: self.slice(attr.params.iter().map(|p| match p {
                AttributeParam::Classname(cls) => UAP::Classname(cls.1),
                AttributeParam::EnumClassLabel(lbl) => UAP::EnumClassLabel(lbl),
                AttributeParam::String(_, s) => UAP::String(s),
                AttributeParam::Int(i) => UAP::Int(i),
            })),
        })
    }

    fn get_current_classish_name(&self) -> Option<(&'a str, &'a Pos<'a>)> {
        let builder = self.classish_name_builder.as_ref()?;
        Some((builder.name, builder.pos))
    }

    fn in_interface(&self) -> bool {
        matches!(
            self.classish_name_builder.as_ref(),
            Some(ClassishNameBuilder {
                token_kind: TokenKind::Interface,
                ..
            })
        )
    }

    fn lexed_name_after_classish_keyword(
        &mut self,
        arena: &'a Bump,
        name: &'a str,
        pos: &'a Pos<'a>,
        token_kind: TokenKind,
    ) {
        if self.classish_name_builder.is_none() {
            let name = if name.starts_with(':') {
                prefix_slash(arena, name)
            } else {
                name
            };
            self.classish_name_builder = Some(ClassishNameBuilder {
                name,
                pos,
                token_kind,
            });
        }
    }
}

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> DirectDeclSmartConstructors<'a, 'o, 't, S> {
    #[inline]
    fn concat(&self, str1: &str, str2: &str) -> &'a str {
        concat(self.arena, str1, str2)
    }

    fn token_bytes(&self, token: &CompactToken) -> &'t [u8] {
        self.source_text
            .source_text()
            .sub(token.start_offset(), token.width())
    }

    // Check that the slice is valid UTF-8. If it is, return a &str referencing
    // the same data. Otherwise, copy the slice into our arena using
    // String::from_utf8_lossy_in, and return a reference to the arena str.
    fn str_from_utf8(&self, slice: &'t [u8]) -> &'a str {
        if let Ok(s) = std::str::from_utf8(slice) {
            self.source_text_allocator.alloc(s)
        } else {
            bump::String::from_utf8_lossy_in(slice, self.arena).into_bump_str()
        }
    }

    // Check that the slice is valid UTF-8. If it is, return a &str referencing
    // the same data. Otherwise, copy the slice into our arena using
    // String::from_utf8_lossy_in, and return a reference to the arena str.
    fn str_from_utf8_for_bytes_in_arena(&self, slice: &'a [u8]) -> &'a str {
        if let Ok(s) = std::str::from_utf8(slice) {
            s
        } else {
            bump::String::from_utf8_lossy_in(slice, self.arena).into_bump_str()
        }
    }

    fn merge(
        &self,
        pos1: impl Into<Option<&'a Pos<'a>>>,
        pos2: impl Into<Option<&'a Pos<'a>>>,
    ) -> &'a Pos<'a> {
        match (pos1.into(), pos2.into()) {
            (None, None) => NO_POS,
            (Some(pos), None) | (None, Some(pos)) => pos,
            (Some(pos1), Some(pos2)) => match (pos1.is_none(), pos2.is_none()) {
                (true, true) => NO_POS,
                (true, false) => pos2,
                (false, true) => pos1,
                (false, false) => Pos::merge_without_checking_filename(self.arena, pos1, pos2),
            },
        }
    }

    fn merge_positions(&self, node1: Node<'a>, node2: Node<'a>) -> &'a Pos<'a> {
        self.merge(self.get_pos(node1), self.get_pos(node2))
    }

    fn pos_from_slice(&self, nodes: &[Node<'a>]) -> &'a Pos<'a> {
        nodes
            .iter()
            .fold(NO_POS, |acc, &node| self.merge(acc, self.get_pos(node)))
    }

    fn get_pos(&self, node: Node<'a>) -> &'a Pos<'a> {
        self.get_pos_opt(node).unwrap_or(NO_POS)
    }

    fn get_pos_opt(&self, node: Node<'a>) -> Option<&'a Pos<'a>> {
        let pos = match node {
            Node::Name(&(_, pos)) | Node::Variable(&(_, pos)) => pos,
            Node::Ty(ty) => return ty.get_pos(),
            Node::XhpName(&(_, pos)) => pos,
            Node::QualifiedName(&(_, pos)) => pos,
            Node::ModuleName(&(_, pos)) => pos,
            Node::IntLiteral(&(_, pos))
            | Node::FloatingLiteral(&(_, pos))
            | Node::StringLiteral(&(_, pos))
            | Node::BooleanLiteral(&(_, pos)) => pos,
            Node::ListItem(&(fst, snd)) => self.merge_positions(fst, snd),
            Node::List(items) => self.pos_from_slice(items),
            Node::BracketedList(&(first_pos, inner_list, second_pos)) => self.merge(
                first_pos,
                self.merge(self.pos_from_slice(inner_list), second_pos),
            ),
            Node::Expr(&aast::Expr(_, pos, _)) => pos,
            Node::Token(token) => self.token_pos(token),
            _ => return None,
        };
        if pos.is_none() { None } else { Some(pos) }
    }

    fn token_pos(&self, token: FixedWidthToken) -> &'a Pos<'a> {
        let start = token.offset();
        let end = start + token.width();
        let start = self.source_text.offset_to_file_pos_triple(start);
        let end = self.source_text.offset_to_file_pos_triple(end);
        Pos::from_lnum_bol_offset(self.arena, self.filename, start, end)
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
            Node::Token(t) if t.kind() == TokenKind::NullLiteral => aast::Expr_::Null,
            Node::Name(..) | Node::QualifiedName(..) => {
                aast::Expr_::Id(self.alloc(self.elaborate_const_id(self.expect_name(node)?)))
            }
            _ => return None,
        };
        let pos = self.get_pos(node);
        Some(self.alloc(aast::Expr((), pos, expr_)))
    }

    fn node_to_non_ret_ty(&self, node: Node<'a>) -> Option<&'a Ty<'a>> {
        self.node_to_ty_(node, false)
    }

    fn node_to_ty(&self, node: Node<'a>) -> Option<&'a Ty<'a>> {
        self.node_to_ty_(node, true)
    }

    fn make_supportdyn(&self, pos: &'a Pos<'a>, ty: Ty_<'a>) -> Ty_<'a> {
        Ty_::Tapply(self.alloc((
            (pos, naming_special_names::typehints::HH_SUPPORTDYN),
            self.alloc([self.alloc(Ty(self.alloc(Reason::witness_from_decl(pos)), ty))]),
        )))
    }

    fn implicit_sdt(&self) -> bool {
        self.opts.everything_sdt && !self.under_no_auto_dynamic
    }

    fn no_auto_likes(&self) -> bool {
        self.under_no_auto_likes
    }

    fn node_to_ty_(&self, node: Node<'a>, allow_non_ret_ty: bool) -> Option<&'a Ty<'a>> {
        match node {
            Node::Ty(Ty(reason, Ty_::Tprim(aast::Tprim::Tvoid))) if !allow_non_ret_ty => {
                Some(self.alloc(Ty(reason, Ty_::Tprim(self.alloc(aast::Tprim::Tnull)))))
            }
            Node::Ty(Ty(reason, Ty_::Tprim(aast::Tprim::Tnoreturn))) if !allow_non_ret_ty => {
                Some(self.alloc(Ty(reason, Ty_::Tunion(&[]))))
            }
            Node::Ty(ty) => Some(ty),
            Node::Expr(expr) => {
                fn expr_to_ty<'a>(arena: &'a Bump, expr: &'a nast::Expr<'a>) -> Option<Ty_<'a>> {
                    use aast::Expr_::*;
                    match expr.2 {
                        Null => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tnull))),
                        This => Some(Ty_::Tthis),
                        True | False => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tbool))),
                        Int(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tint))),
                        Float(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tfloat))),
                        String(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        String2(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        PrefixedString(_) => Some(Ty_::Tprim(arena.alloc(aast::Tprim::Tstring))),
                        Unop(&(_op, expr)) => expr_to_ty(arena, expr),
                        Hole(&(expr, _, _, _)) => expr_to_ty(arena, expr),

                        ArrayGet(_) | As(_) | Await(_) | Binop(_) | Call(_) | Cast(_)
                        | ClassConst(_) | ClassGet(_) | Clone(_) | Collection(_)
                        | Dollardollar(_) | Efun(_) | Eif(_) | EnumClassLabel(_) | ETSplice(_)
                        | ExpressionTree(_) | FunctionPointer(_) | Id(_) | Import(_) | Is(_)
                        | KeyValCollection(_) | Lfun(_) | List(_) | Lplaceholder(_) | Lvar(_)
                        | MethodCaller(_) | New(_) | ObjGet(_) | Omitted | Pair(_) | Pipe(_)
                        | ReadonlyExpr(_) | Shape(_) | Tuple(_) | Upcast(_) | ValCollection(_)
                        | Xml(_) | Yield(_) | Invalid(_) | Package(_) | Nameof(_) => None,
                    }
                }
                Some(self.alloc(Ty(
                    self.alloc(Reason::witness_from_decl(expr.1)),
                    expr_to_ty(self.arena, expr)?,
                )))
            }
            Node::IntLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness_from_decl(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tint)),
            ))),
            Node::FloatingLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness_from_decl(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tfloat)),
            ))),
            Node::StringLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness_from_decl(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tstring)),
            ))),
            Node::BooleanLiteral((_, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::witness_from_decl(pos)),
                Ty_::Tprim(self.alloc(aast::Tprim::Tbool)),
            ))),
            Node::Token(t) if t.kind() == TokenKind::Varray => {
                let pos = self.token_pos(t);
                let tany = self.alloc(Ty(self.alloc(Reason::hint(pos)), TANY_));
                let ty_ = Ty_::Tapply(self.alloc((
                    (self.token_pos(t), naming_special_names::collections::VEC),
                    self.alloc([tany]),
                )));
                Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), ty_)))
            }
            Node::Token(t) if t.kind() == TokenKind::Darray => {
                let pos = self.token_pos(t);
                let tany = self.alloc(Ty(self.alloc(Reason::hint(pos)), TANY_));
                let ty_ = Ty_::Tapply(self.alloc((
                    (self.token_pos(t), naming_special_names::collections::DICT),
                    self.alloc([tany, tany]),
                )));
                Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), ty_)))
            }
            Node::Token(t) if t.kind() == TokenKind::This => {
                Some(self.alloc(Ty(self.alloc(Reason::hint(self.token_pos(t))), Ty_::Tthis)))
            }
            Node::Token(t) if t.kind() == TokenKind::NullLiteral => {
                let pos = self.token_pos(t);
                Some(self.alloc(Ty(
                    self.alloc(Reason::hint(pos)),
                    Ty_::Tprim(self.alloc(aast::Tprim::Tnull)),
                )))
            }
            // In coeffects contexts, we get types like `ctx $f` or `$v::C`.
            // Node::Variable is used for the `$f` and `$v`, so that we don't
            // incorrectly attempt to elaborate them as names.
            Node::Variable(&(name, pos)) => Some(self.alloc(Ty(
                self.alloc(Reason::hint(pos)),
                Ty_::Tapply(self.alloc(((pos, name), &[][..]))),
            ))),
            node => {
                let Id(pos, name) = self.expect_name(node)?;
                let reason = self.alloc(Reason::hint(pos));
                let ty_ = if self.is_type_param_in_scope(name) {
                    // TODO (T69662957) must fill type args of Tgeneric
                    Ty_::Tgeneric(self.alloc((name, &[])))
                } else {
                    match name {
                        "nothing" => Ty_::Tunion(&[]),
                        "nonnull" => {
                            if self.implicit_sdt() {
                                self.make_supportdyn(pos, Ty_::Tnonnull)
                            } else {
                                Ty_::Tnonnull
                            }
                        }
                        "dynamic" => Ty_::Tdynamic,
                        "varray_or_darray" | "vec_or_dict" => {
                            let key_type = self.vec_or_dict_key(pos);
                            let value_type = self.alloc(Ty(self.alloc(Reason::hint(pos)), TANY_));
                            Ty_::TvecOrDict(self.alloc((key_type, value_type)))
                        }
                        "_" => Ty_::Twildcard,
                        _ => {
                            let name = self.elaborate_raw_id(name);
                            Ty_::Tapply(self.alloc(((pos, name), &[][..])))
                        }
                    }
                };
                Some(self.alloc(Ty(reason, ty_)))
            }
        }
    }

    fn partition_bounds_into_lower_and_upper(
        &self,
        constraints: Node<'a>,
        match_constraint: impl Fn(Node<'a>) -> Option<(ConstraintKind, Node<'a>)>,
    ) -> (bump::Vec<'a, &'a Ty<'a>>, bump::Vec<'a, &'a Ty<'a>>) {
        let append = |tys: &mut bump::Vec<'_, _>, ty: Option<_>| {
            if let Some(ty) = ty {
                tys.push(ty);
            }
        };
        constraints.iter().fold(
            (bump::Vec::new_in(self.arena), bump::Vec::new_in(self.arena)),
            |(mut super_, mut as_), constraint| {
                if let Some((kind, hint)) = match_constraint(*constraint) {
                    use ConstraintKind::*;
                    match kind {
                        ConstraintAs => append(&mut as_, self.node_to_ty(hint)),
                        ConstraintSuper => append(&mut super_, self.node_to_ty(hint)),
                        _ => (),
                    };
                };
                (super_, as_)
            },
        )
    }

    fn partition_type_bounds_into_lower_and_upper(
        &self,
        constraints: Node<'a>,
    ) -> (bump::Vec<'a, &'a Ty<'a>>, bump::Vec<'a, &'a Ty<'a>>) {
        self.partition_bounds_into_lower_and_upper(constraints, |constraint| {
            if let Node::TypeConstraint(kind_hint) = constraint {
                Some(*kind_hint)
            } else {
                None
            }
        })
    }

    fn partition_ctx_bounds_into_lower_and_upper(
        &self,
        constraints: Node<'a>,
    ) -> (bump::Vec<'a, &'a Ty<'a>>, bump::Vec<'a, &'a Ty<'a>>) {
        self.partition_bounds_into_lower_and_upper(constraints, |constraint| {
            if let Node::ContextConstraint(kind_hint) = constraint {
                Some(*kind_hint)
            } else {
                None
            }
        })
    }

    fn to_attributes(&self, node: Node<'a>) -> Attributes<'a> {
        let mut attributes = Attributes {
            deprecated: None,
            reifiable: None,
            late_init: false,
            const_: false,
            lsb: false,
            memoize: false,
            memoizelsb: false,
            override_: false,
            enforceable: None,
            accept_disposable: false,
            dynamically_callable: false,
            returns_disposable: false,
            php_std_lib: false,
            soft: false,
            support_dynamic_type: false,
            no_auto_likes: false,
            safe_global_variable: false,
            cross_package: None,
            sort_text: None,
            dynamically_referenced: false,
        };

        let nodes = match node {
            Node::List(&nodes) | Node::BracketedList(&(_, nodes, _)) => nodes,
            _ => return attributes,
        };

        // Iterate in reverse, to match the behavior of OCaml decl in error conditions.
        for attribute in nodes.iter().rev() {
            if let Node::Attribute(attribute) = attribute {
                match attribute.name.1 {
                    "__Deprecated" => {
                        attributes.deprecated = attribute
                            .string_literal_param
                            .map(|(_, x)| self.str_from_utf8_for_bytes_in_arena(x));
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
                    "__Memoize" => {
                        attributes.memoize = true;
                    }
                    "__MemoizeLSB" => {
                        attributes.memoizelsb = true;
                    }
                    "__Override" => {
                        attributes.override_ = true;
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
                    "__DynamicallyReferenced" => {
                        attributes.dynamically_referenced = true;
                    }
                    "__ReturnDisposable" => {
                        attributes.returns_disposable = true;
                    }
                    "__PHPStdLib" => {
                        attributes.php_std_lib = true;
                    }
                    "__Soft" => {
                        attributes.soft = true;
                    }
                    "__SupportDynamicType" => {
                        attributes.support_dynamic_type = true;
                    }
                    "__NoAutoLikes" => {
                        attributes.no_auto_likes = true;
                    }
                    "__SafeForGlobalAccessCheck" => {
                        attributes.safe_global_variable = true;
                    }
                    "__CrossPackage" => {
                        attributes.cross_package = attribute
                            .string_literal_param
                            .map(|(_, x)| self.str_from_utf8_for_bytes_in_arena(x));
                    }
                    "__AutocompleteSortText" => {
                        attributes.sort_text = attribute
                            .string_literal_param
                            .map(|(_, x)| self.str_from_utf8_for_bytes_in_arena(x));
                    }
                    _ => {}
                }
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
            | Node::Expr(aast::Expr(_, _, aast::Expr_::Unop(&(Uop::Uminus, _))))
            | Node::Expr(aast::Expr(_, _, aast::Expr_::Unop(&(Uop::Uplus, _))))
            | Node::Expr(aast::Expr(_, _, aast::Expr_::String(..))) => self.node_to_ty(node),
            Node::Token(t) if t.kind() == TokenKind::NullLiteral => {
                let pos = self.token_pos(t);
                Some(self.alloc(Ty(
                    self.alloc(Reason::witness_from_decl(pos)),
                    Ty_::Tprim(self.alloc(aast::Tprim::Tnull)),
                )))
            }
            _ => Some(self.tany_with_pos(self.get_pos(name))),
        }
    }

    fn pop_type_params(&mut self, node: Node<'a>) -> &'a [&'a Tparam<'a>] {
        match node {
            Node::TypeParameters(tparams) => {
                let this = Rc::make_mut(&mut self.state);
                Rc::make_mut(&mut this.type_parameters).pop().unwrap();
                tparams
            }
            _ => &[],
        }
    }

    fn ret_from_fun_kind(&self, kind: FunKind, type_: &'a Ty<'a>) -> &'a Ty<'a> {
        let pos = type_.get_pos().unwrap_or(NO_POS);
        match kind {
            FunKind::FAsyncGenerator => self.alloc(Ty(
                self.alloc(Reason::RretFunKindFromDecl(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    (pos, naming_special_names::classes::ASYNC_GENERATOR),
                    self.alloc([type_, type_, type_]),
                ))),
            )),
            FunKind::FGenerator => self.alloc(Ty(
                self.alloc(Reason::RretFunKindFromDecl(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    (pos, naming_special_names::classes::GENERATOR),
                    self.alloc([type_, type_, type_]),
                ))),
            )),
            FunKind::FAsync => self.alloc(Ty(
                self.alloc(Reason::RretFunKindFromDecl(self.alloc((pos, kind)))),
                Ty_::Tapply(self.alloc((
                    (pos, naming_special_names::classes::AWAITABLE),
                    self.alloc([type_]),
                ))),
            )),
            _ => type_,
        }
    }

    fn is_type_param_in_scope(&self, name: &str) -> bool {
        self.type_parameters.iter().any(|tps| tps.contains(name))
    }

    fn as_fun_implicit_params(
        &self,
        capability: Node<'a>,
        default_pos: &'a Pos<'a>,
    ) -> &'a FunImplicitParams<'a> {
        /* Note: do not simplify intersections, keep empty / singleton intersections
         * for coeffect contexts
         */
        let capability = match self.node_to_ty(capability) {
            Some(ty) => CapTy(ty),
            None => CapDefaults(default_pos),
        };
        self.alloc(FunImplicitParams { capability })
    }

    fn function_to_ty(
        &mut self,
        is_method: bool,
        attributes: Node<'a>,
        header: &'a FunctionHeader<'a>,
        body: Node<'_>,
    ) -> Option<(PosId<'a>, &'a Ty<'a>, &'a [ShallowProp<'a>])> {
        let id_opt = match (is_method, header.name) {
            // If the name is missing, we use the left paren here, just to get a
            // position to point to.
            (_, Node::Token(t)) if t.kind() == TokenKind::LeftParen => {
                let pos = self.token_pos(t);
                Some(Id(pos, ""))
            }
            (true, Node::Token(t)) if t.kind() == TokenKind::Construct => {
                let pos = self.token_pos(t);
                Some(Id(pos, naming_special_names::members::__CONSTRUCT))
            }
            (true, _) => self.expect_name(header.name),
            (false, _) => self.elaborate_defined_id(header.name),
        };
        let id = id_opt.unwrap_or_else(|| Id(self.get_pos(header.name), ""));
        let attributes = self.to_attributes(attributes);
        let (params, properties, variadic) =
            self.as_fun_params(attributes.no_auto_likes, header.param_list)?;
        let f_pos = self.get_pos(header.name);
        let implicit_params = self.as_fun_implicit_params(header.capability, f_pos);

        let type_ = match header.name {
            Node::Token(t) if t.kind() == TokenKind::Construct => {
                let pos = self.token_pos(t);
                self.alloc(Ty(
                    self.alloc(Reason::witness_from_decl(pos)),
                    Ty_::Tprim(self.alloc(aast::Tprim::Tvoid)),
                ))
            }
            _ => self
                .node_to_ty(header.ret_hint)
                .unwrap_or_else(|| self.tany_with_pos(f_pos)),
        };
        let async_ = header
            .modifiers
            .iter()
            .any(|n| n.is_token(TokenKind::Async));
        let readonly = header
            .modifiers
            .iter()
            .any(|n| n.is_token(TokenKind::Readonly));

        let fun_kind = if body.iter().any(|node| node.is_token(TokenKind::Yield)) {
            if async_ {
                FunKind::FAsyncGenerator
            } else {
                FunKind::FGenerator
            }
        } else if async_ {
            FunKind::FAsync
        } else {
            FunKind::FSync
        };
        let type_ = if !header.ret_hint.is_present() {
            self.ret_from_fun_kind(fun_kind, type_)
        } else {
            type_
        };
        // TODO(hrust) Put this in a helper. Possibly do this for all flags.
        let mut flags = match fun_kind {
            FunKind::FSync => FunTypeFlags::empty(),
            FunKind::FAsync => FunTypeFlags::ASYNC,
            FunKind::FGenerator => FunTypeFlags::GENERATOR,
            FunKind::FAsyncGenerator => FunTypeFlags::ASYNC | FunTypeFlags::GENERATOR,
        };

        if attributes.returns_disposable {
            flags |= FunTypeFlags::RETURN_DISPOSABLE;
        }
        if attributes.support_dynamic_type || attributes.dynamically_callable {
            flags |= FunTypeFlags::SUPPORT_DYNAMIC_TYPE;
        }
        if header.readonly_return.is_token(TokenKind::Readonly) {
            flags |= FunTypeFlags::RETURNS_READONLY;
        }
        if attributes.memoize || attributes.memoizelsb {
            flags |= FunTypeFlags::IS_MEMOIZED;
        }
        if readonly {
            flags |= FunTypeFlags::READONLY_THIS
        }
        if variadic {
            flags |= FunTypeFlags::VARIADIC
        }

        let cross_package = attributes.cross_package;

        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(header.type_params);

        let where_constraints =
            self.slice(header.where_constraints.iter().filter_map(|&x| match x {
                Node::WhereConstraint(x) => Some(x),
                _ => None,
            }));

        let (params, tparams, implicit_params, where_constraints) =
            self.rewrite_effect_polymorphism(params, tparams, implicit_params, where_constraints);

        let ft = self.alloc(FunType {
            tparams,
            where_constraints,
            params,
            implicit_params,
            ret: self.alloc(PossiblyEnforcedTy {
                enforced: Enforcement::Unenforced,
                type_,
            }),
            flags,
            cross_package,
        });

        let ty = self.alloc(Ty(
            self.alloc(Reason::witness_from_decl(id.0)),
            Ty_::Tfun(ft),
        ));
        Some((id.into(), ty, properties))
    }

    fn as_fun_params(
        &self,
        no_auto_likes: bool,
        list: Node<'a>,
    ) -> Option<(&'a FunParams<'a>, &'a [ShallowProp<'a>], bool)> {
        match list {
            Node::List(nodes) => {
                let mut params = bump::Vec::with_capacity_in(nodes.len(), self.arena);
                let mut properties = bump::Vec::new_in(self.arena);
                let mut ft_variadic = false;
                for node in nodes.iter() {
                    match node {
                        Node::FunParam(&FunParamDecl {
                            attributes,
                            visibility,
                            kind,
                            readonly,
                            hint,
                            pos,
                            name,
                            variadic,
                            initializer,
                        }) => {
                            let attributes = self.to_attributes(attributes);

                            let type_ = self
                                .node_to_ty(hint)
                                .unwrap_or_else(|| self.tany_with_pos(pos));
                            if let Some(visibility) = visibility.as_visibility() {
                                let name = name.unwrap_or("");
                                let name = strip_dollar_prefix(name);
                                let mut flags = PropFlags::empty();
                                flags.set(PropFlags::CONST, attributes.const_);
                                flags.set(PropFlags::NEEDS_INIT, self.file_mode != Mode::Mhhi);
                                flags.set(PropFlags::PHP_STD_LIB, attributes.php_std_lib);
                                flags.set(PropFlags::READONLY, readonly);
                                // Promoted property either marked <<__NoAutoLikes>> on the parameter or on the constructor
                                flags.set(
                                    PropFlags::NO_AUTO_LIKES,
                                    attributes.no_auto_likes || no_auto_likes,
                                );
                                properties.push(ShallowProp {
                                    xhp_attr: None,
                                    name: (pos, name),
                                    type_,
                                    visibility,
                                    flags,
                                });
                            }

                            // These are illegal here--they can only be used on
                            // parameters in a function type hint (see
                            // make_closure_type_specifier and unwrap_mutability).
                            // Unwrap them here anyway for better error recovery.
                            let type_ = match type_ {
                                Ty(_, Ty_::Tapply(((_, "\\Mutable"), [t]))) => t,
                                Ty(_, Ty_::Tapply(((_, "\\OwnedMutable"), [t]))) => t,
                                Ty(_, Ty_::Tapply(((_, "\\MaybeMutable"), [t]))) => t,
                                _ => type_,
                            };
                            let mut flags = FunParamFlags::empty();
                            if attributes.accept_disposable {
                                flags |= FunParamFlags::ACCEPT_DISPOSABLE
                            }
                            if readonly {
                                flags |= FunParamFlags::READONLY
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
                            if variadic {
                                ft_variadic = true;
                            }
                            let variadic = initializer.is_ignored() && variadic;
                            let type_ = if variadic {
                                self.alloc(Ty(
                                    self.alloc(if name.is_some() {
                                        Reason::RvarParamFromDecl(pos)
                                    } else {
                                        Reason::witness_from_decl(pos)
                                    }),
                                    type_.1,
                                ))
                            } else {
                                type_
                            };
                            let param = self.alloc(FunParam {
                                pos,
                                name,
                                type_: self.alloc(PossiblyEnforcedTy {
                                    enforced: Enforcement::Unenforced,
                                    type_,
                                }),
                                flags,
                            });
                            params.push(param);
                        }
                        _ => {}
                    }
                }
                Some((
                    params.into_bump_slice(),
                    properties.into_bump_slice(),
                    ft_variadic,
                ))
            }
            n if n.is_ignored() => Some((&[], &[], false)),
            _ => None,
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
                _,
                aast::Expr_::ClassConst(&(
                    aast::ClassId(_, _, aast::ClassId_::CI(&class_name)),
                    const_name,
                )),
            )) => ShapeFieldName::SFclassConst(self.alloc((class_name, const_name))),
            Node::Expr(aast::Expr(
                _,
                _,
                aast::Expr_::ClassConst(&(
                    aast::ClassId(_, pos, aast::ClassId_::CIself),
                    const_name,
                )),
            )) => {
                let (classish_name, _) = self.get_current_classish_name()?;
                ShapeFieldName::SFclassConst(self.alloc((Id(pos, classish_name), const_name)))
            }
            _ => return None,
        })
    }

    fn make_t_shape_field_name(&self, ShapeField(field): &ShapeField<'a>) -> TShapeField<'a> {
        TShapeField(match field {
            ShapeFieldName::SFlitInt(&(pos, x)) => {
                TshapeFieldName::TSFlitInt(self.alloc(PosString(pos, x)))
            }
            ShapeFieldName::SFlitStr(&(pos, x)) => {
                TshapeFieldName::TSFlitStr(self.alloc(PosByteString(pos, x)))
            }
            ShapeFieldName::SFclassConst(&(id, &(pos, x))) => {
                TshapeFieldName::TSFclassConst(self.alloc((id.into(), PosString(pos, x))))
            }
        })
    }

    fn make_apply(
        &self,
        base_ty: PosId<'a>,
        type_arguments: Node<'a>,
        pos_to_merge: &'a Pos<'a>,
    ) -> Node<'a> {
        let type_arguments = self.slice(
            type_arguments
                .iter()
                .filter_map(|&node| self.node_to_ty(node)),
        );

        let pos = self.merge(base_ty.0, pos_to_merge);

        // OCaml decl creates a capability with a hint pointing to the entire
        // type (i.e., pointing to `Rx<(function(): void)>` rather than just
        // `(function(): void)`), so we extend the hint position similarly here.
        let extend_capability_pos = |implicit_params: &'a FunImplicitParams<'_>| {
            let capability = match implicit_params.capability {
                CapTy(ty) => {
                    let ty = self.alloc(Ty(self.alloc(Reason::hint(pos)), ty.1));
                    CapTy(ty)
                }
                CapDefaults(_) => CapDefaults(pos),
            };
            self.alloc(FunImplicitParams {
                capability,
                ..*implicit_params
            })
        };

        let ty_ = match (base_ty, type_arguments) {
            ((_, name), &[&Ty(_, Ty_::Tfun(f))]) if name == "\\Pure" => {
                Ty_::Tfun(self.alloc(FunType {
                    implicit_params: extend_capability_pos(f.implicit_params),
                    ..*f
                }))
            }
            _ => Ty_::Tapply(self.alloc((base_ty, type_arguments))),
        };

        self.hint_ty(pos, ty_)
    }

    fn hint_ty(&self, pos: &'a Pos<'a>, ty_: Ty_<'a>) -> Node<'a> {
        Node::Ty(self.alloc(Ty(self.alloc(Reason::hint(pos)), ty_)))
    }

    fn prim_ty(&self, tprim: aast::Tprim, pos: &'a Pos<'a>) -> Node<'a> {
        self.hint_ty(pos, Ty_::Tprim(self.alloc(tprim)))
    }

    fn tany_with_pos(&self, pos: &'a Pos<'a>) -> &'a Ty<'a> {
        self.alloc(Ty(self.alloc(Reason::witness_from_decl(pos)), TANY_))
    }

    /// The type used when a `vec_or_dict` typehint is missing its key type argument.
    fn vec_or_dict_key(&self, pos: &'a Pos<'a>) -> &'a Ty<'a> {
        self.alloc(Ty(
            self.alloc(Reason::RvecOrDictKey(pos)),
            Ty_::Tprim(self.alloc(aast::Tprim::Tarraykey)),
        ))
    }

    fn source_text_at_pos(&self, pos: &'a Pos<'a>) -> &'t [u8] {
        let start = pos.start_offset();
        let end = pos.end_offset();
        self.source_text.source_text().sub(start, end - start)
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
                let params = self.slice(fun_type.params.iter().copied().map(convert_param));
                let implicit_params = fun_type.implicit_params;
                let ret = self.alloc(PossiblyEnforcedTy {
                    enforced: fun_type.ret.enforced,
                    type_: self.convert_tapply_to_tgeneric(fun_type.ret.type_),
                });
                Ty_::Tfun(self.alloc(FunType {
                    params,
                    implicit_params,
                    ret,
                    ..*fun_type
                }))
            }
            Ty_::Tshape(&ShapeType {
                origin: _,
                unknown_value: kind,
                fields,
            }) => {
                let mut converted_fields = AssocListMut::with_capacity_in(fields.len(), self.arena);
                for (&name, ty) in fields.iter() {
                    converted_fields.insert(
                        name,
                        self.alloc(ShapeFieldType {
                            optional: ty.optional,
                            ty: self.convert_tapply_to_tgeneric(ty.ty),
                        }),
                    );
                }
                let origin = TypeOrigin::MissingOrigin;
                Ty_::Tshape(self.alloc(ShapeType {
                    origin,
                    unknown_value: kind,
                    fields: converted_fields.into(),
                }))
            }
            Ty_::TvecOrDict(&(tk, tv)) => Ty_::TvecOrDict(self.alloc((
                self.convert_tapply_to_tgeneric(tk),
                self.convert_tapply_to_tgeneric(tv),
            ))),
            Ty_::Ttuple(tys) => Ty_::Ttuple(
                self.slice(
                    tys.iter()
                        .map(|&targ| self.convert_tapply_to_tgeneric(targ)),
                ),
            ),
            Ty_::Tintersection(tys) => Ty_::Tintersection(
                self.slice(tys.iter().map(|&ty| self.convert_tapply_to_tgeneric(ty))),
            ),
            Ty_::Tunion(tys) => {
                Ty_::Tunion(self.slice(tys.iter().map(|&ty| self.convert_tapply_to_tgeneric(ty))))
            }
            Ty_::Trefinement(&(root_ty, class_ref)) => {
                let convert_refined_const = |rc: &'a RefinedConst<'a>| {
                    let RefinedConst { bound, is_ctx } = rc;
                    let bound = match bound {
                        RefinedConstBound::TRexact(ty) => {
                            RefinedConstBound::TRexact(self.convert_tapply_to_tgeneric(ty))
                        }
                        RefinedConstBound::TRloose(bnds) => {
                            let convert_tys = |tys: &'a [&'a Ty<'a>]| {
                                self.slice(
                                    tys.iter().map(|&ty| self.convert_tapply_to_tgeneric(ty)),
                                )
                            };
                            RefinedConstBound::TRloose(self.alloc(RefinedConstBounds {
                                lower: convert_tys(bnds.lower),
                                upper: convert_tys(bnds.upper),
                            }))
                        }
                    };
                    RefinedConst {
                        bound,
                        is_ctx: *is_ctx,
                    }
                };
                Ty_::Trefinement(
                    self.alloc((
                        self.convert_tapply_to_tgeneric(root_ty),
                        ClassRefinement {
                            cr_consts: arena_collections::map::Map::from(
                                self.arena,
                                class_ref
                                    .cr_consts
                                    .iter()
                                    .map(|(id, ctr)| (*id, convert_refined_const(ctr))),
                            ),
                        },
                    )),
                )
            }
            Ty_::Taccess(_)
            | Ty_::Tany(_)
            | Ty_::Tclass(_)
            | Ty_::Tdynamic
            | Ty_::Tgeneric(_)
            | Ty_::Tmixed
            | Ty_::Twildcard
            | Ty_::Tnonnull
            | Ty_::Tprim(_)
            | Ty_::Tthis => return ty,
            Ty_::Tdependent(_)
            | Ty_::Tneg(_)
            | Ty_::Tnewtype(_)
            | Ty_::Tvar(_)
            | Ty_::TunappliedAlias(_) => panic!("unexpected decl type in constraint"),
        };
        self.alloc(Ty(ty.0, ty_))
    }

    // This is the logic for determining if convert_tapply_to_tgeneric should turn
    // a Tapply into a Tgeneric
    fn tapply_should_be_tgeneric(&self, reason: &'a Reason<'a>, id: PosId<'a>) -> Option<&'a str> {
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
            Some(name) if self.is_type_param_in_scope(name) => Some(name),
            _ => None,
        }
    }

    fn rewrite_taccess_reasons(&self, ty: &'a Ty<'a>, r: &'a Reason<'a>) -> &'a Ty<'a> {
        let ty_ = match ty.1 {
            Ty_::Taccess(&TaccessType(ty, id)) => {
                Ty_::Taccess(self.alloc(TaccessType(self.rewrite_taccess_reasons(ty, r), id)))
            }
            ty_ => ty_,
        };
        self.alloc(Ty(r, ty_))
    }

    fn namespace_use_kind(use_kind: &Node<'_>) -> Option<NamespaceUseKind> {
        match use_kind.token_kind() {
            Some(TokenKind::Const) => None,
            Some(TokenKind::Function) => None,
            Some(TokenKind::Type) => Some(NamespaceUseKind::Type),
            Some(TokenKind::Namespace) => Some(NamespaceUseKind::Namespace),
            _ if !use_kind.is_present() => Some(NamespaceUseKind::Mixed),
            _ => None,
        }
    }

    fn has_polymorphic_context(contexts: &[&Ty<'_>]) -> bool {
        contexts.iter().any(|&ty| match ty.1 {
            Ty_::Tapply((root, &[])) // Hfun_context in the AST
            | Ty_::Taccess(TaccessType(Ty(_, Ty_::Tapply((root, &[]))), _)) => root.1.contains('$'),
            | Ty_::Taccess(TaccessType(t, _)) => Self::taccess_root_is_generic(t),
            _ => false,
        })
    }

    fn ctx_generic_for_fun(&self, name: &str) -> &'a str {
        bumpalo::format!(in self.arena, "T/[ctx {}]", name).into_bump_str()
    }

    fn ctx_generic_for_dependent(&self, name: &str, cst: &str) -> &'a str {
        bumpalo::format!(in self.arena, "T/[{}::{}]", name, cst).into_bump_str()
    }

    // Note: the reason for the divergence between this and the lowerer is that
    // hint Haccess is a flat list, whereas decl ty Taccess is a tree.
    fn taccess_root_is_generic(ty: &Ty<'_>) -> bool {
        match ty {
            Ty(_, Ty_::Tgeneric((_, &[]))) => true,
            Ty(_, Ty_::Taccess(&TaccessType(t, _))) => Self::taccess_root_is_generic(t),
            _ => false,
        }
    }

    fn ctx_generic_for_generic_taccess_inner(ty: &Ty<'_>, cst: &str) -> std::string::String {
        let left = match ty {
            Ty(_, Ty_::Tgeneric((name, &[]))) => name.to_string(),
            Ty(_, Ty_::Taccess(&TaccessType(ty, cst))) => {
                Self::ctx_generic_for_generic_taccess_inner(ty, cst.1)
            }
            _ => panic!("Unexpected element in Taccess"),
        };
        format!("{}::{}", left, cst)
    }

    fn ctx_generic_for_generic_taccess(&self, ty: &Ty<'_>, cst: &str) -> &'a str {
        bumpalo::format!(
            in self.arena,
            "T/[{}]",
            Self::ctx_generic_for_generic_taccess_inner(ty, cst)
        )
        .into_bump_str()
    }

    // For a polymorphic context with form `ctx $f` (represented here as
    // `Tapply "$f"`), add a type parameter named `Tctx$f`, and rewrite the
    // parameter `(function (ts)[_]: t) $f` as `(function (ts)[Tctx$f]: t) $f`
    fn rewrite_fun_ctx(
        &self,
        tparams: &mut bump::Vec<'_, &'a Tparam<'a>>,
        ty: &Ty<'a>,
        param_name: &str,
    ) -> Ty<'a> {
        match ty.1 {
            Ty_::Tfun(ft) => {
                let cap_ty = match ft.implicit_params.capability {
                    CapTy(&Ty(_, Ty_::Tintersection(&[ty]))) | CapTy(ty) => ty,
                    _ => return ty.clone(),
                };
                let pos = match cap_ty.1 {
                    Ty_::Tapply(((pos, "_"), _)) => pos,
                    _ => return ty.clone(),
                };
                let name = self.ctx_generic_for_fun(param_name);
                let tparam = self.alloc(Tparam {
                    variance: Variance::Invariant,
                    name: (pos, name),
                    tparams: &[],
                    constraints: &[],
                    reified: aast::ReifyKind::Erased,
                    user_attributes: &[],
                });
                tparams.push(tparam);
                let cap_ty = self.alloc(Ty(cap_ty.0, Ty_::Tgeneric(self.alloc((name, &[])))));
                let ft = self.alloc(FunType {
                    implicit_params: self.alloc(FunImplicitParams {
                        capability: CapTy(cap_ty),
                    }),
                    ..*ft
                });
                Ty(ty.0, Ty_::Tfun(ft))
            }
            Ty_::Tlike(t) => Ty(
                ty.0,
                Ty_::Tlike(self.alloc(self.rewrite_fun_ctx(tparams, t, param_name))),
            ),
            Ty_::Toption(t) => Ty(
                ty.0,
                Ty_::Toption(self.alloc(self.rewrite_fun_ctx(tparams, t, param_name))),
            ),
            Ty_::Tapply(((p, name), targs))
                if *name == naming_special_names::typehints::HH_SUPPORTDYN =>
            {
                if let Some(t) = targs.first() {
                    Ty(
                        ty.0,
                        Ty_::Tapply(self.alloc((
                            (p, name),
                            self.alloc([self.alloc(self.rewrite_fun_ctx(tparams, t, param_name))]),
                        ))),
                    )
                } else {
                    ty.clone()
                }
            }
            _ => ty.clone(),
        }
    }

    fn rewrite_effect_polymorphism(
        &self,
        params: &'a [&'a FunParam<'a>],
        tparams: &'a [&'a Tparam<'a>],
        implicit_params: &'a FunImplicitParams<'a>,
        where_constraints: &'a [&'a WhereConstraint<'a>],
    ) -> (
        &'a [&'a FunParam<'a>],
        &'a [&'a Tparam<'a>],
        &'a FunImplicitParams<'a>,
        &'a [&'a WhereConstraint<'a>],
    ) {
        let (cap_reason, context_tys) = match implicit_params.capability {
            CapTy(&Ty(r, Ty_::Tintersection(tys))) if Self::has_polymorphic_context(tys) => {
                (r, tys)
            }
            CapTy(ty) if Self::has_polymorphic_context(&[ty]) => {
                (ty.0, std::slice::from_ref(self.alloc(ty)))
            }
            _ => return (params, tparams, implicit_params, where_constraints),
        };
        let tp = |name, constraints| {
            self.alloc(Tparam {
                variance: Variance::Invariant,
                name,
                tparams: &[],
                constraints,
                reified: aast::ReifyKind::Erased,
                user_attributes: &[],
            })
        };

        // For a polymorphic context with form `$g::C`, if we have a function
        // parameter `$g` with type `G` (where `G` is not a type parameter),
        //   - add a type parameter constrained by $g's type: `T/$g as G`
        //   - replace $g's type hint (`G`) with the new type parameter `T/$g`
        // Then, for each polymorphic context with form `$g::C`,
        //   - add a type parameter `T/[$g::C]`
        //   - add a where constraint `T/[$g::C] = T$g :: C`
        let rewrite_arg_ctx = |tparams: &mut bump::Vec<'_, &'a Tparam<'a>>,
                               where_constraints: &mut bump::Vec<'_, &'a WhereConstraint<'a>>,
                               ty: &Ty<'a>,
                               param_pos: &'a Pos<'a>,
                               name: &str,
                               context_reason: &'a Reason<'a>,
                               cst: PosId<'a>|
         -> Ty<'a> {
            let rewritten_ty = match ty.1 {
                // If the type hint for this function parameter is a type
                // parameter introduced in this function declaration, don't add
                // a new type parameter.
                Ty_::Tgeneric(&(type_name, _))
                    if tparams.iter().any(|tp| tp.name.1 == type_name) =>
                {
                    ty.clone()
                }
                // Otherwise, if the parameter is `G $g`, create tparam
                // `T$g as G` and replace $g's type hint
                _ => {
                    let id = (param_pos, self.concat("T/", name));
                    tparams.push(tp(
                        id,
                        std::slice::from_ref(
                            self.alloc((ConstraintKind::ConstraintAs, self.alloc(ty.clone()))),
                        ),
                    ));
                    Ty(
                        self.alloc(Reason::hint(param_pos)),
                        Ty_::Tgeneric(self.alloc((id.1, &[]))),
                    )
                }
            };
            let ty = self.alloc(Ty(context_reason, rewritten_ty.1));
            let right = self.alloc(Ty(
                context_reason,
                Ty_::Taccess(self.alloc(TaccessType(ty, cst))),
            ));
            let left_id = (
                context_reason.pos().unwrap_or(NO_POS),
                self.ctx_generic_for_dependent(name, cst.1),
            );
            tparams.push(tp(left_id, &[]));
            let left = self.alloc(Ty(
                context_reason,
                Ty_::Tgeneric(self.alloc((left_id.1, &[]))),
            ));
            where_constraints.push(self.alloc(WhereConstraint(
                left,
                ConstraintKind::ConstraintEq,
                right,
            )));
            rewritten_ty
        };

        let mut tparams = bump::Vec::from_iter_in(tparams.iter().copied(), self.arena);
        let mut where_constraints =
            bump::Vec::from_iter_in(where_constraints.iter().copied(), self.arena);

        // The divergence here from the lowerer comes from using oxidized_by_ref instead of oxidized
        let mut ty_by_param: BTreeMap<&str, (Ty<'a>, &'a Pos<'a>)> = params
            .iter()
            .filter_map(|param| Some((param.name?, (param.type_.type_.clone(), param.pos))))
            .collect();

        for context_ty in context_tys {
            match context_ty.1 {
                // Hfun_context in the AST.
                Ty_::Tapply(((_, name), _)) if name.starts_with('$') => {
                    if let Some((param_ty, _)) = ty_by_param.get_mut(name) {
                        *param_ty = self.rewrite_fun_ctx(&mut tparams, param_ty, name);
                    }
                }
                Ty_::Taccess(&TaccessType(Ty(_, Ty_::Tapply(((_, name), _))), cst)) => {
                    if let Some((param_ty, param_pos)) = ty_by_param.get_mut(name) {
                        let mut rewrite = |t| {
                            rewrite_arg_ctx(
                                &mut tparams,
                                &mut where_constraints,
                                t,
                                param_pos,
                                name,
                                context_ty.0,
                                cst,
                            )
                        };
                        match param_ty.1 {
                            Ty_::Tlike(ref mut ty) => match ty {
                                Ty(r, Ty_::Toption(tinner)) => {
                                    *ty =
                                        self.alloc(Ty(r, Ty_::Toption(self.alloc(rewrite(tinner)))))
                                }
                                _ => {
                                    *ty = self.alloc(rewrite(ty));
                                }
                            },
                            Ty_::Toption(ref mut ty) => {
                                *ty = self.alloc(rewrite(ty));
                            }
                            _ => {
                                *param_ty = rewrite(param_ty);
                            }
                        }
                    }
                }
                Ty_::Taccess(&TaccessType(t, cst)) if Self::taccess_root_is_generic(t) => {
                    let left_id = (
                        context_ty.0.pos().unwrap_or(NO_POS),
                        self.ctx_generic_for_generic_taccess(t, cst.1),
                    );
                    tparams.push(tp(left_id, &[]));
                    let left = self.alloc(Ty(
                        context_ty.0,
                        Ty_::Tgeneric(self.alloc((left_id.1, &[]))),
                    ));
                    where_constraints.push(self.alloc(WhereConstraint(
                        left,
                        ConstraintKind::ConstraintEq,
                        context_ty,
                    )));
                }
                _ => {}
            }
        }

        let params = self.slice(params.iter().copied().map(|param| match param.name {
            None => param,
            Some(name) => match ty_by_param.get(name) {
                Some((type_, _)) if param.type_.type_ != type_ => self.alloc(FunParam {
                    type_: self.alloc(PossiblyEnforcedTy {
                        type_: self.alloc(type_.clone()),
                        ..*param.type_
                    }),
                    ..*param
                }),
                _ => param,
            },
        }));

        let context_tys = self.slice(context_tys.iter().copied().map(|ty| {
            let ty_ = match ty.1 {
                Ty_::Tapply(((_, name), &[])) if name.starts_with('$') => {
                    Ty_::Tgeneric(self.alloc((self.ctx_generic_for_fun(name), &[])))
                }
                Ty_::Taccess(&TaccessType(Ty(_, Ty_::Tapply(((_, name), &[]))), cst))
                    if name.starts_with('$') =>
                {
                    let name = self.ctx_generic_for_dependent(name, cst.1);
                    Ty_::Tgeneric(self.alloc((name, &[])))
                }
                Ty_::Taccess(&TaccessType(t, cst)) if Self::taccess_root_is_generic(t) => {
                    let name = self.ctx_generic_for_generic_taccess(t, cst.1);
                    Ty_::Tgeneric(self.alloc((name, &[])))
                }
                _ => return ty,
            };
            self.alloc(Ty(ty.0, ty_))
        }));
        let cap_ty = match context_tys {
            [ty] => ty,
            _ => self.alloc(Ty(cap_reason, Ty_::Tintersection(context_tys))),
        };
        let implicit_params = self.alloc(FunImplicitParams {
            capability: CapTy(cap_ty),
        });

        (
            params,
            tparams.into_bump_slice(),
            implicit_params,
            where_constraints.into_bump_slice(),
        )
    }
}

enum NodeIterHelper<'a, 'b> {
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

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> FlattenSmartConstructors
    for DirectDeclSmartConstructors<'a, 'o, 't, S>
{
    // type Output = Node<'a> in direct_decl_smart_constructors_generated.rs

    fn flatten(&self, kind: SyntaxKind, lst: Vec<Self::Output>) -> Self::Output {
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
        let mut r = bump::Vec::with_capacity_in(size, self.arena);
        for s in lst.into_iter() {
            match s {
                Node::List(children) => r.extend(children.iter().copied()),
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

    fn zero(kind: SyntaxKind) -> Node<'a> {
        Node::Ignored(kind)
    }

    fn is_zero(s: &Self::Output) -> bool {
        match s {
            Node::Token(token) => match token.kind() {
                TokenKind::Yield | TokenKind::Required | TokenKind::Lateinit => false,
                _ => true,
            },
            Node::List(inner) => inner.iter().all(Self::is_zero),
            _ => true,
        }
    }

    fn make_token(&mut self, token: CompactToken) -> Self::Output {
        let token_text = |this: &Self| this.str_from_utf8(this.token_bytes(&token));
        let token_pos = |this: &Self| {
            let start = this
                .source_text
                .offset_to_file_pos_triple(token.start_offset());
            let end = this
                .source_text
                .offset_to_file_pos_triple(token.end_offset());
            Pos::from_lnum_bol_offset(this.arena, this.filename, start, end)
        };
        let kind = token.kind();

        let result = match kind {
            TokenKind::Name | TokenKind::XHPClassName => {
                let text = token_text(self);
                let pos = token_pos(self);

                let name = if kind == TokenKind::XHPClassName {
                    Node::XhpName(self.alloc((text, pos)))
                } else {
                    Node::Name(self.alloc((text, pos)))
                };

                if self.previous_token_kind == TokenKind::Class
                    || self.previous_token_kind == TokenKind::Trait
                    || self.previous_token_kind == TokenKind::Interface
                {
                    if let Some(current_class_name) = self.elaborate_defined_id(name) {
                        let previous_token_kind = self.previous_token_kind;
                        let this = Rc::make_mut(&mut self.state);
                        this.lexed_name_after_classish_keyword(
                            this.arena,
                            current_class_name.1,
                            pos,
                            previous_token_kind,
                        );
                    }
                }
                name
            }
            TokenKind::Variable => Node::Variable(self.alloc((token_text(self), token_pos(self)))),
            // There are a few types whose string representations we have to
            // grab anyway, so just go ahead and treat them as generic names.
            TokenKind::Vec
            | TokenKind::Dict
            | TokenKind::Keyset
            | TokenKind::Tuple
            | TokenKind::Classname
            | TokenKind::SelfToken => Node::Name(self.alloc((token_text(self), token_pos(self)))),
            TokenKind::XHPElementName => {
                Node::XhpName(self.alloc((token_text(self), token_pos(self))))
            }
            TokenKind::SingleQuotedStringLiteral => match escaper::unescape_single_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text.into(), token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::DoubleQuotedStringLiteral => match escaper::unescape_double_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text, token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::HeredocStringLiteral => match escaper::unescape_heredoc_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.arena,
            ) {
                Ok(text) => Node::StringLiteral(self.alloc((text, token_pos(self)))),
                Err(_) => Node::Ignored(SK::Token(kind)),
            },
            TokenKind::NowdocStringLiteral => match escaper::unescape_nowdoc_in(
                self.str_from_utf8(escaper::unquote_slice(self.token_bytes(&token))),
                self.arena,
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
                Ty_::Tapply(self.alloc(((token_pos(self), token_text(self)), &[][..]))),
            ),
            TokenKind::Num => self.prim_ty(aast::Tprim::Tnum, token_pos(self)),
            TokenKind::Bool => self.prim_ty(aast::Tprim::Tbool, token_pos(self)),
            TokenKind::Mixed => {
                let reason = self.alloc(Reason::hint(token_pos(self)));
                if self.implicit_sdt() {
                    let ty_ = self.make_supportdyn(token_pos(self), Ty_::Tmixed);
                    Node::Ty(self.alloc(Ty(reason, ty_)))
                } else {
                    Node::Ty(self.alloc(Ty(reason, Ty_::Tmixed)))
                }
            }
            TokenKind::Void => self.prim_ty(aast::Tprim::Tvoid, token_pos(self)),
            TokenKind::Arraykey => self.prim_ty(aast::Tprim::Tarraykey, token_pos(self)),
            TokenKind::Noreturn => self.prim_ty(aast::Tprim::Tnoreturn, token_pos(self)),
            TokenKind::Resource => self.prim_ty(aast::Tprim::Tresource, token_pos(self)),
            TokenKind::Class | TokenKind::Interface | TokenKind::Trait => {
                if self.under_no_auto_dynamic {
                    let this = Rc::make_mut(&mut self.state);
                    this.inside_no_auto_dynamic_class = true;
                }
                Node::Token(FixedWidthToken::new(kind, token.start_offset()))
            }
            TokenKind::NullLiteral
            | TokenKind::Darray
            | TokenKind::Varray
            | TokenKind::Backslash
            | TokenKind::Construct
            | TokenKind::LeftParen
            | TokenKind::RightParen
            | TokenKind::LeftBracket
            | TokenKind::RightBracket
            | TokenKind::Shape
            | TokenKind::Question
            | TokenKind::This
            | TokenKind::Tilde
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
            | TokenKind::Equal
            | TokenKind::Abstract
            | TokenKind::As
            | TokenKind::Super
            | TokenKind::Async
            | TokenKind::DotDotDot
            | TokenKind::Extends
            | TokenKind::Final
            | TokenKind::Implements
            | TokenKind::Inout
            | TokenKind::Newctx
            | TokenKind::Newtype
            | TokenKind::Type
            | TokenKind::Yield
            | TokenKind::Semicolon
            | TokenKind::Private
            | TokenKind::Protected
            | TokenKind::Public
            | TokenKind::Reify
            | TokenKind::Static
            | TokenKind::Lateinit
            | TokenKind::RightBrace
            | TokenKind::Enum
            | TokenKind::Const
            | TokenKind::Function
            | TokenKind::Namespace
            | TokenKind::XHP
            | TokenKind::Required
            | TokenKind::Ctx
            | TokenKind::Readonly
            | TokenKind::Internal
            | TokenKind::Global => Node::Token(FixedWidthToken::new(kind, token.start_offset())),
            _ if kind.fixed_width().is_some() => {
                Node::IgnoredToken(FixedWidthToken::new(kind, token.start_offset()))
            }
            _ => Node::Ignored(SK::Token(kind)),
        };
        self.previous_token_kind = kind;
        result
    }

    fn make_error(&mut self, error: Self::Output) -> Self::Output {
        // If it's a Token or IgnoredToken, we can use it for error recovery.
        // For instance, in `function using() {}`, the `using` keyword will be a
        // token wrapped in an error CST node, since the keyword isn't legal in
        // that position.
        error
    }

    fn make_missing(&mut self, _: usize) -> Self::Output {
        Node::Ignored(SK::Missing)
    }

    fn make_list(&mut self, mut items: Vec<Self::Output>, _: usize) -> Self::Output {
        if let Some(&yield_) = items
            .iter()
            .flat_map(|node| node.iter())
            .find(|node| node.is_token(TokenKind::Yield))
        {
            yield_
        } else {
            items.retain(|node| node.is_present());
            if items.is_empty() {
                Node::Ignored(SK::SyntaxList)
            } else {
                let items = self.arena.alloc_slice_fill_iter(items);
                Node::List(self.alloc(items))
            }
        }
    }

    fn make_qualified_name(&mut self, parts: Self::Output) -> Self::Output {
        let pos = self.get_pos(parts);
        match parts {
            Node::List(nodes) => Node::QualifiedName(self.alloc((nodes, pos))),
            node if node.is_ignored() => Node::Ignored(SK::QualifiedName),
            node => Node::QualifiedName(
                self.alloc((bumpalo::vec![in self.arena; node].into_bump_slice(), pos)),
            ),
        }
    }

    fn make_module_name(&mut self, parts: Self::Output) -> Self::Output {
        let pos = self.get_pos(parts);
        match parts {
            Node::List(nodes) => Node::ModuleName(self.alloc((nodes, pos))),
            node if node.is_ignored() => Node::Ignored(SK::ModuleName),
            node => Node::ModuleName(
                self.alloc((bumpalo::vec![in self.arena; node].into_bump_slice(), pos)),
            ),
        }
    }

    fn make_simple_type_specifier(&mut self, specifier: Self::Output) -> Self::Output {
        // Return this explicitly because flatten filters out zero nodes, and
        // we treat most non-error nodes as zeroes.
        specifier
    }

    fn make_literal_expression(&mut self, expression: Self::Output) -> Self::Output {
        expression
    }

    fn make_simple_initializer(
        &mut self,
        equals: Self::Output,
        expr: Self::Output,
    ) -> Self::Output {
        // If the expr is Ignored, bubble up the assignment operator so that we
        // can tell that *some* initializer was here. Useful for class
        // properties, where we need to enforce that properties without default
        // values are initialized in the constructor.
        if expr.is_ignored() { equals } else { expr }
    }

    fn make_anonymous_function(
        &mut self,
        _attribute_spec: Self::Output,
        _async_keyword: Self::Output,
        _function_keyword: Self::Output,
        _left_paren: Self::Output,
        _parameters: Self::Output,
        _right_paren: Self::Output,
        _ctx_list: Self::Output,
        _colon: Self::Output,
        _readonly_return: Self::Output,
        _type_: Self::Output,
        _use_: Self::Output,
        _body: Self::Output,
    ) -> Self::Output {
        // do not allow Yield to bubble up
        Node::Ignored(SK::AnonymousFunction)
    }

    fn make_lambda_expression(
        &mut self,
        _attribute_spec: Self::Output,
        _async_: Self::Output,
        _signature: Self::Output,
        _arrow: Self::Output,
        _body: Self::Output,
    ) -> Self::Output {
        // do not allow Yield to bubble up
        Node::Ignored(SK::LambdaExpression)
    }

    fn make_awaitable_creation_expression(
        &mut self,
        _attribute_spec: Self::Output,
        _async_: Self::Output,
        _compound_statement: Self::Output,
    ) -> Self::Output {
        // do not allow Yield to bubble up
        Node::Ignored(SK::AwaitableCreationExpression)
    }

    fn make_element_initializer(
        &mut self,
        key: Self::Output,
        _arrow: Self::Output,
        value: Self::Output,
    ) -> Self::Output {
        Node::ListItem(self.alloc((key, value)))
    }

    fn make_prefix_unary_expression(
        &mut self,
        op: Self::Output,
        value: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(op, value);
        let op = match op.token_kind() {
            Some(TokenKind::Tilde) => Uop::Utild,
            Some(TokenKind::Exclamation) => Uop::Unot,
            Some(TokenKind::Plus) => Uop::Uplus,
            Some(TokenKind::Minus) => Uop::Uminus,
            Some(TokenKind::PlusPlus) => Uop::Uincr,
            Some(TokenKind::MinusMinus) => Uop::Udecr,
            Some(TokenKind::At) => Uop::Usilence,
            _ => return Node::Ignored(SK::PrefixUnaryExpression),
        };
        let value = match self.node_to_expr(value) {
            Some(value) => value,
            None => return Node::Ignored(SK::PrefixUnaryExpression),
        };
        Node::Expr(self.alloc(aast::Expr(
            (),
            pos,
            aast::Expr_::Unop(self.alloc((op, value))),
        )))
    }

    fn make_postfix_unary_expression(
        &mut self,
        value: Self::Output,
        op: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(value, op);
        let op = match op.token_kind() {
            Some(TokenKind::PlusPlus) => Uop::Upincr,
            Some(TokenKind::MinusMinus) => Uop::Updecr,
            _ => return Node::Ignored(SK::PostfixUnaryExpression),
        };
        let value = match self.node_to_expr(value) {
            Some(value) => value,
            None => return Node::Ignored(SK::PostfixUnaryExpression),
        };
        Node::Expr(self.alloc(aast::Expr(
            (),
            pos,
            aast::Expr_::Unop(self.alloc((op, value))),
        )))
    }

    fn make_binary_expression(
        &mut self,
        lhs: Self::Output,
        op_node: Self::Output,
        rhs: Self::Output,
    ) -> Self::Output {
        let op = match op_node.token_kind() {
            Some(TokenKind::Plus) => Bop::Plus,
            Some(TokenKind::Minus) => Bop::Minus,
            Some(TokenKind::Star) => Bop::Star,
            Some(TokenKind::Slash) => Bop::Slash,
            Some(TokenKind::Equal) => Bop::Eq(None),
            Some(TokenKind::EqualEqual) => Bop::Eqeq,
            Some(TokenKind::EqualEqualEqual) => Bop::Eqeqeq,
            Some(TokenKind::StarStar) => Bop::Starstar,
            Some(TokenKind::AmpersandAmpersand) => Bop::Ampamp,
            Some(TokenKind::BarBar) => Bop::Barbar,
            Some(TokenKind::LessThan) => Bop::Lt,
            Some(TokenKind::LessThanEqual) => Bop::Lte,
            Some(TokenKind::LessThanLessThan) => Bop::Ltlt,
            Some(TokenKind::GreaterThan) => Bop::Gt,
            Some(TokenKind::GreaterThanEqual) => Bop::Gte,
            Some(TokenKind::GreaterThanGreaterThan) => Bop::Gtgt,
            Some(TokenKind::Dot) => Bop::Dot,
            Some(TokenKind::Ampersand) => Bop::Amp,
            Some(TokenKind::Bar) => Bop::Bar,
            Some(TokenKind::Percent) => Bop::Percent,
            Some(TokenKind::QuestionQuestion) => Bop::QuestionQuestion,
            _ => return Node::Ignored(SK::BinaryExpression),
        };

        match (&op, rhs.is_token(TokenKind::Yield)) {
            (Bop::Eq(_), true) => return rhs,
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
            (),
            pos,
            aast::Expr_::Binop(self.alloc(aast::Binop { bop: op, lhs, rhs })),
        )))
    }

    fn make_parenthesized_expression(
        &mut self,
        _lparen: Self::Output,
        expr: Self::Output,
        _rparen: Self::Output,
    ) -> Self::Output {
        expr
    }

    fn make_enum_class_label_expression(
        &mut self,
        _class: Self::Output,
        _hash: Self::Output,
        label: Self::Output,
    ) -> Self::Output {
        // In case we want it later on, _class is either Ignored(Missing)
        // or a Name node, like label
        match label {
            Node::Name((lbl, _)) => Node::EnumClassLabel(lbl),
            _ => Node::Ignored(SK::EnumClassLabelExpression),
        }
    }

    fn make_list_item(&mut self, item: Self::Output, sep: Self::Output) -> Self::Output {
        match (item.is_ignored(), sep.is_ignored()) {
            (true, true) => Node::Ignored(SK::ListItem),
            (false, true) => item,
            (true, false) => sep,
            (false, false) => Node::ListItem(self.alloc((item, sep))),
        }
    }

    fn make_type_arguments(
        &mut self,
        less_than: Self::Output,
        arguments: Self::Output,
        greater_than: Self::Output,
    ) -> Self::Output {
        Node::BracketedList(self.alloc((
            self.get_pos(less_than),
            arguments.as_slice(self.arena),
            self.get_pos(greater_than),
        )))
    }

    fn make_generic_type_specifier(
        &mut self,
        class_type: Self::Output,
        type_arguments: Self::Output,
    ) -> Self::Output {
        let class_id = match self.expect_name(class_type) {
            Some(id) => id,
            None => return Node::Ignored(SK::GenericTypeSpecifier),
        };
        match class_id.1.trim_start_matches('\\') {
            "varray_or_darray" | "vec_or_dict" => {
                let id_pos = class_id.0;
                let pos = self.merge(id_pos, self.get_pos(type_arguments));
                let type_arguments = type_arguments.as_slice(self.arena);
                let ty_ = match type_arguments {
                    [tk, tv] => Ty_::TvecOrDict(
                        self.alloc((
                            self.node_to_ty(*tk)
                                .unwrap_or_else(|| self.tany_with_pos(id_pos)),
                            self.node_to_ty(*tv)
                                .unwrap_or_else(|| self.tany_with_pos(id_pos)),
                        )),
                    ),
                    [tv] => Ty_::TvecOrDict(
                        self.alloc((
                            self.vec_or_dict_key(pos),
                            self.node_to_ty(*tv)
                                .unwrap_or_else(|| self.tany_with_pos(id_pos)),
                        )),
                    ),
                    _ => TANY_,
                };
                self.hint_ty(pos, ty_)
            }
            _ => {
                let Id(pos, class_type) = class_id;
                match class_type.rsplit('\\').next() {
                    Some(name) if self.is_type_param_in_scope(name) => {
                        let pos = self.merge(pos, self.get_pos(type_arguments));
                        let type_arguments = self.slice(
                            type_arguments
                                .iter()
                                .filter_map(|&node| self.node_to_ty(node)),
                        );
                        let ty_ = Ty_::Tgeneric(self.alloc((name, type_arguments)));
                        self.hint_ty(pos, ty_)
                    }
                    _ => {
                        let class_type = self.elaborate_raw_id(class_type);
                        self.make_apply(
                            (pos, class_type),
                            type_arguments,
                            self.get_pos(type_arguments),
                        )
                    }
                }
            }
        }
    }

    fn make_alias_declaration(
        &mut self,
        attributes: Self::Output,
        modifiers: Self::Output,
        module_kw_opt: Self::Output,
        keyword: Self::Output,
        name: Self::Output,
        generic_params: Self::Output,
        constraint: Self::Output,
        _equal: Self::Output,
        aliased_type: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        if name.is_ignored() {
            return Node::Ignored(SK::AliasDeclaration);
        }
        let Id(pos, name) = match self.elaborate_defined_id(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::AliasDeclaration),
        };
        let ty = match self.node_to_ty(aliased_type) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::AliasDeclaration),
        };
        let mut as_constraint = None;
        let mut super_constraint = None;
        for c in constraint.iter() {
            if let Node::TypeConstraint(&(kind, hint)) = c {
                let ty = self.node_to_ty(hint);
                match kind {
                    ConstraintKind::ConstraintAs => as_constraint = ty,
                    ConstraintKind::ConstraintSuper => super_constraint = ty,
                    _ => {}
                }
            }
        }

        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(generic_params);

        // Parse the user attributes
        // in facts-mode all attributes are saved, otherwise only __NoAutoDynamic/__NoAutoLikes is
        let user_attributes = self.slice(attributes.iter().rev().filter_map(|attribute| {
            if let Node::Attribute(attr) = attribute {
                if self.opts.keep_user_attributes || is_no_auto_attribute(attr.name.1) {
                    Some(self.user_attribute_to_decl(attr))
                } else {
                    None
                }
            } else {
                None
            }
        }));

        let mut docs_url = None;
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => {
                    if attr.name.1 == "__Docs" {
                        if let Some((_, bstr)) = attr.string_literal_param {
                            docs_url = Some(self.str_from_utf8_for_bytes_in_arena(bstr));
                        }
                    }
                }
                _ => {}
            }
        }

        let internal = modifiers
            .iter()
            .any(|m| m.as_visibility() == Some(aast::Visibility::Internal));
        let is_module_newtype = module_kw_opt.is_ignored_token_with_kind(TokenKind::Module);
        let typedef = self.alloc(TypedefType {
            module: self.module,
            pos,
            vis: match keyword.token_kind() {
                Some(TokenKind::Type) => aast::TypedefVisibility::Transparent,
                Some(TokenKind::Newtype) if is_module_newtype => {
                    aast::TypedefVisibility::OpaqueModule
                }
                Some(TokenKind::Newtype) => aast::TypedefVisibility::Opaque,
                _ => aast::TypedefVisibility::Transparent,
            },
            tparams,
            as_constraint,
            super_constraint,
            type_: ty,
            is_ctx: false,
            attributes: user_attributes,
            internal,
            docs_url,
        });

        let this = Rc::make_mut(&mut self.state);
        this.add_typedef(name, typedef);

        Node::Ignored(SK::AliasDeclaration)
    }

    fn make_context_alias_declaration(
        &mut self,
        attributes: Self::Output,
        _keyword: Self::Output,
        name: Self::Output,
        generic_params: Self::Output,
        constraint: Self::Output,
        _equal: Self::Output,
        ctx_list: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        if name.is_ignored() {
            return Node::Ignored(SK::ContextAliasDeclaration);
        }
        let Id(pos, name) = match self.elaborate_defined_id(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::ContextAliasDeclaration),
        };
        let ty = match self.node_to_ty(ctx_list) {
            Some(ty) => ty,
            None => self.alloc(Ty(
                self.alloc(Reason::hint(pos)),
                Ty_::Tapply(self.alloc(((pos, "\\HH\\Contexts\\defaults"), &[]))),
            )),
        };

        // lowerer ensures there is only one as constraint
        let mut as_constraint = None;
        let mut super_constraint = None;
        for c in constraint.iter() {
            if let Node::ContextConstraint(&(kind, hint)) = c {
                let ty = self.node_to_ty(hint);
                match kind {
                    ConstraintKind::ConstraintAs => as_constraint = ty,
                    ConstraintKind::ConstraintSuper => super_constraint = ty,
                    _ => {}
                }
            }
        }
        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(generic_params);
        let user_attributes = if self.opts.keep_user_attributes {
            self.slice(attributes.iter().rev().filter_map(|attribute| {
                if let Node::Attribute(attr) = attribute {
                    Some(self.user_attribute_to_decl(attr))
                } else {
                    None
                }
            }))
        } else {
            &[][..]
        };
        let typedef = self.alloc(TypedefType {
            module: self.module,
            pos,
            vis: aast::TypedefVisibility::Opaque,
            tparams,
            as_constraint,
            super_constraint,
            type_: ty,
            is_ctx: true,
            attributes: user_attributes,
            internal: false,
            docs_url: None,
        });

        let this = Rc::make_mut(&mut self.state);
        this.add_typedef(name, typedef);

        Node::Ignored(SK::ContextAliasDeclaration)
    }

    fn make_case_type_declaration(
        &mut self,
        attribute_spec: Self::Output,
        modifiers: Self::Output,
        _case_keyword: Self::Output,
        _type_keyword: Self::Output,
        name: Self::Output,
        generic_parameter: Self::Output,
        _as: Self::Output,
        bounds: Self::Output,
        _equal: Self::Output,
        variants: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        if name.is_ignored() {
            return Node::Ignored(SK::CaseTypeDeclaration);
        }
        let Id(pos, name) = match self.elaborate_defined_id(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::CaseTypeDeclaration),
        };

        let as_constraint = match bounds.len() {
            0 => None,
            1 => self.node_to_ty(*bounds.iter().next().unwrap()),
            _ => {
                let pos = self.get_pos(bounds);
                let tys = self.slice(bounds.iter().filter_map(|x| match x {
                    Node::ListItem(&(ty, _commas)) => self.node_to_ty(ty),
                    &x => self.node_to_ty(x),
                }));
                Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tintersection(tys))))
            }
        };

        let ty = match variants.len() {
            0 => None,
            1 => self.node_to_ty(*variants.iter().next().unwrap()),
            _ => {
                let pos = self.get_pos(variants);
                let tys = self.slice(variants.iter().filter_map(|x| self.node_to_ty(*x)));
                Some(self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tunion(tys))))
            }
        };

        let type_ = match ty {
            Some(x) => x,
            None => return Node::Ignored(SK::CaseTypeDeclaration),
        };

        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(generic_parameter);

        // Parse the user attributes
        // in facts-mode all attributes are saved, otherwise only __NoAutoDynamic/__NoAutoLikes is
        let user_attributes = self.slice(attribute_spec.iter().rev().filter_map(|attribute| {
            if let Node::Attribute(attr) = attribute {
                if self.opts.keep_user_attributes || is_no_auto_attribute(attr.name.1) {
                    Some(self.user_attribute_to_decl(attr))
                } else {
                    None
                }
            } else {
                None
            }
        }));

        let mut docs_url = None;
        for attribute in attribute_spec.iter() {
            match attribute {
                Node::Attribute(attr) => {
                    if attr.name.1 == "__Docs" {
                        if let Some((_, bstr)) = attr.string_literal_param {
                            docs_url = Some(self.str_from_utf8_for_bytes_in_arena(bstr));
                        }
                    }
                }
                _ => {}
            }
        }

        let internal = modifiers
            .iter()
            .any(|m| m.as_visibility() == Some(aast::Visibility::Internal));
        let typedef = self.alloc(TypedefType {
            module: self.module,
            pos,
            vis: aast::TypedefVisibility::CaseType,
            tparams,
            as_constraint,
            super_constraint: None,
            type_,
            is_ctx: false,
            attributes: user_attributes,
            internal,
            docs_url,
        });

        let this = Rc::make_mut(&mut self.state);
        this.add_typedef(name, typedef);

        Node::Ignored(SK::CaseTypeDeclaration)
    }

    fn make_case_type_variant(&mut self, _bar: Self::Output, type_: Self::Output) -> Self::Output {
        if type_.is_ignored() {
            Node::Ignored(SK::CaseTypeVariant)
        } else {
            type_
        }
    }

    fn make_type_constraint(&mut self, kind: Self::Output, value: Self::Output) -> Self::Output {
        let kind = match kind.token_kind() {
            Some(TokenKind::As) => ConstraintKind::ConstraintAs,
            Some(TokenKind::Super) => ConstraintKind::ConstraintSuper,
            _ => return Node::Ignored(SK::TypeConstraint),
        };
        Node::TypeConstraint(self.alloc((kind, value)))
    }

    fn make_context_constraint(&mut self, kind: Self::Output, value: Self::Output) -> Self::Output {
        let kind = match kind.token_kind() {
            Some(TokenKind::As) => ConstraintKind::ConstraintAs,
            Some(TokenKind::Super) => ConstraintKind::ConstraintSuper,
            _ => return Node::Ignored(SK::ContextConstraint),
        };
        Node::ContextConstraint(self.alloc((kind, value)))
    }

    fn make_type_parameter(
        &mut self,
        user_attributes: Self::Output,
        reify: Self::Output,
        variance: Self::Output,
        name: Self::Output,
        tparam_params: Self::Output,
        constraints: Self::Output,
    ) -> Self::Output {
        let user_attributes = match user_attributes {
            Node::BracketedList((_, attributes, _)) => {
                self.slice(attributes.iter().filter_map(|x| match x {
                    Node::Attribute(a) => Some(*a),
                    _ => None,
                }))
            }
            _ => &[][..],
        };

        let constraints = self.slice(constraints.iter().filter_map(|node| match node {
            Node::TypeConstraint(&constraint) => Some(constraint),
            _ => None,
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
            variance: match variance.token_kind() {
                Some(TokenKind::Minus) => Variance::Contravariant,
                Some(TokenKind::Plus) => Variance::Covariant,
                _ => Variance::Invariant,
            },
            reified: if reify.is_token(TokenKind::Reify) {
                if user_attributes.iter().any(|node| node.name.1 == "__Soft") {
                    aast::ReifyKind::SoftReified
                } else {
                    aast::ReifyKind::Reified
                }
            } else {
                aast::ReifyKind::Erased
            },
            constraints,
            tparam_params,
            user_attributes,
        }))
    }

    fn make_type_parameters(
        &mut self,
        _lt: Self::Output,
        tparams: Self::Output,
        _gt: Self::Output,
    ) -> Self::Output {
        let size = tparams.len();
        let mut tparams_with_name = bump::Vec::with_capacity_in(size, self.arena);
        let mut tparam_names = MultiSetMut::with_capacity_in(size, self.arena);
        for node in tparams.iter() {
            match *node {
                Node::TypeParameter(decl) => {
                    let name = match decl.name.as_id() {
                        Some(name) => name,
                        None => return Node::Ignored(SK::TypeParameters),
                    };
                    tparam_names.insert(name.1);
                    tparams_with_name.push((decl, name));
                }
                _ => {}
            }
        }
        let this = Rc::make_mut(&mut self.state);
        Rc::make_mut(&mut this.type_parameters).push(tparam_names.into());
        let mut tparams = bump::Vec::with_capacity_in(tparams_with_name.len(), self.arena);
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
                name: name.into(),
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
        attributes: Self::Output,
        visibility: Self::Output,
        inout: Self::Output,
        readonly: Self::Output,
        hint: Self::Output,
        name: Self::Output,
        initializer: Self::Output,
    ) -> Self::Output {
        let (variadic, pos, name) = match name {
            Node::ListItem(&(ellipsis, id)) => {
                let Id(pos, name) = match id.as_variable() {
                    Some(id) => id,
                    None => return Node::Ignored(SK::ParameterDeclaration),
                };
                let variadic = ellipsis.is_token(TokenKind::DotDotDot);
                (variadic, pos, Some(name))
            }
            name => {
                let Id(pos, name) = match name.as_variable() {
                    Some(id) => id,
                    None => return Node::Ignored(SK::ParameterDeclaration),
                };
                (false, pos, Some(name))
            }
        };
        let kind = if inout.is_token(TokenKind::Inout) {
            ParamMode::FPinout
        } else {
            ParamMode::FPnormal
        };
        let is_readonly = readonly.is_token(TokenKind::Readonly);
        let hint = if self.opts.interpret_soft_types_as_like_types {
            let attributes = self.to_attributes(attributes);
            if attributes.soft {
                match hint {
                    Node::Ty(ty) => self.hint_ty(self.get_pos(hint), Ty_::Tlike(ty)),
                    _ => hint,
                }
            } else {
                hint
            }
        } else {
            hint
        };
        Node::FunParam(self.alloc(FunParamDecl {
            attributes,
            visibility,
            kind,
            readonly: is_readonly,
            hint,
            pos,
            name,
            variadic,
            initializer,
        }))
    }

    fn make_variadic_parameter(
        &mut self,
        _: Self::Output,
        hint: Self::Output,
        ellipsis: Self::Output,
    ) -> Self::Output {
        Node::FunParam(
            self.alloc(FunParamDecl {
                attributes: Node::Ignored(SK::Missing),
                visibility: Node::Ignored(SK::Missing),
                kind: ParamMode::FPnormal,
                readonly: false,
                hint,
                pos: self
                    .get_pos_opt(hint)
                    .unwrap_or_else(|| self.get_pos(ellipsis)),
                name: None,
                variadic: true,
                initializer: Node::Ignored(SK::Missing),
            }),
        )
    }

    fn make_function_declaration(
        &mut self,
        attributes: Self::Output,
        header: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let parsed_attributes = self.to_attributes(attributes);
        match header {
            Node::FunctionHeader(header) => {
                let is_method = false;
                let ((pos, name), type_, _) =
                    match self.function_to_ty(is_method, attributes, header, body) {
                        Some(x) => x,
                        None => return Node::Ignored(SK::FunctionDeclaration),
                    };
                let deprecated = parsed_attributes.deprecated.map(|msg| {
                    let mut s = bump::String::new_in(self.arena);
                    s.push_str("The function ");
                    s.push_str(name.trim_start_matches('\\'));
                    s.push_str(" is deprecated: ");
                    s.push_str(msg);
                    s.into_bump_str()
                });
                let internal = header
                    .modifiers
                    .iter()
                    .any(|m| m.as_visibility() == Some(aast::Visibility::Internal));
                let fun_elt = self.alloc(FunElt {
                    module: self.module,
                    internal,
                    deprecated,
                    type_,
                    pos,
                    php_std_lib: parsed_attributes.php_std_lib,
                    support_dynamic_type: self.implicit_sdt()
                        || parsed_attributes.support_dynamic_type
                        || parsed_attributes.dynamically_callable,
                    no_auto_dynamic: self.under_no_auto_dynamic,
                    no_auto_likes: parsed_attributes.no_auto_likes,
                });
                let this = Rc::make_mut(&mut self.state);
                this.add_fun(name, fun_elt);
                Node::Ignored(SK::FunctionDeclaration)
            }
            _ => Node::Ignored(SK::FunctionDeclaration),
        }
    }

    fn make_contexts(
        &mut self,
        left_bracket: Self::Output,
        tys: Self::Output,
        right_bracket: Self::Output,
    ) -> Self::Output {
        let tys = self.slice(tys.iter().filter_map(|ty| match ty {
            Node::ListItem(&(ty, _)) | &ty => {
                // A wildcard is used for the context of a closure type on a
                // parameter of a function with a function context (e.g.,
                // `function f((function ()[_]: void) $f)[ctx $f]: void {}`).
                if let Some(Id(pos, "_")) = self.expect_name(ty) {
                    return Some(self.alloc(Ty(
                        self.alloc(Reason::hint(pos)),
                        Ty_::Tapply(self.alloc(((pos, "_"), &[]))),
                    )));
                }
                let ty = self.node_to_ty(ty)?;
                match ty.1 {
                    // Only three forms of type can appear here in a valid program:
                    //   - function contexts (`ctx $f`)
                    //   - value-dependent paths (`$v::C`)
                    //   - built-in contexts (`rx`, `cipp_of<EntFoo>`)
                    // The first and last will be represented with `Tapply`,
                    // but function contexts will use a variable name
                    // (containing a `$`). Built-in contexts are always in the
                    // \HH\Contexts namespace, so we rewrite those names here.
                    Ty_::Tapply(&((pos, name), targs)) if !name.starts_with('$') => {
                        // The name will have been elaborated in the current
                        // namespace, but we actually want it to be in the
                        // \HH\Contexts namespace. Grab the last component of
                        // the name, and rewrite it in the correct namespace.
                        // Note that this makes it impossible to express names
                        // in any sub-namespace of \HH\Contexts (e.g.,
                        // "Unsafe\\cipp" will be rewritten as
                        // "\\HH\\Contexts\\cipp" rather than
                        // "\\HH\\Contexts\\Unsafe\\cipp").
                        let name = match name.trim_end_matches('\\').split('\\').next_back() {
                            Some(ctxname) => match ctxname.chars().next() {
                                Some(first_char) if first_char.is_lowercase() => {
                                    self.concat("\\HH\\Contexts\\", ctxname)
                                }
                                Some(_) | None => name,
                            },
                            None => name,
                        };
                        Some(self.alloc(Ty(ty.0, Ty_::Tapply(self.alloc(((pos, name), targs))))))
                    }
                    _ => Some(ty),
                }
            }
        }));
        /* Like in as_fun_implicit_params, we keep the intersection as is: we do not simplify
         * empty or singleton intersections.
         */
        let pos = self.merge_positions(left_bracket, right_bracket);
        self.hint_ty(pos, Ty_::Tintersection(tys))
    }

    fn make_function_ctx_type_specifier(
        &mut self,
        ctx_keyword: Self::Output,
        variable: Self::Output,
    ) -> Self::Output {
        match variable.as_variable() {
            Some(Id(pos, name)) => {
                Node::Variable(self.alloc((name, self.merge(pos, self.get_pos(ctx_keyword)))))
            }
            None => Node::Ignored(SK::FunctionCtxTypeSpecifier),
        }
    }

    fn make_function_declaration_header(
        &mut self,
        modifiers: Self::Output,
        _keyword: Self::Output,
        name: Self::Output,
        type_params: Self::Output,
        left_paren: Self::Output,
        param_list: Self::Output,
        _right_paren: Self::Output,
        capability: Self::Output,
        _colon: Self::Output,
        readonly_return: Self::Output,
        ret_hint: Self::Output,
        where_constraints: Self::Output,
    ) -> Self::Output {
        // Use the position of the left paren if the name is missing.
        // Keep the name if it's an IgnoredToken rather than an Ignored. An
        // IgnoredToken here should always be an error, but it's better to treat
        // a keyword as a name than to claim the function has no name at all.
        let name = if matches!(name, Node::Ignored(..)) {
            left_paren
        } else {
            name
        };
        Node::FunctionHeader(self.alloc(FunctionHeader {
            name,
            modifiers,
            type_params,
            param_list,
            capability,
            ret_hint,
            readonly_return,
            where_constraints,
        }))
    }

    fn make_yield_expression(
        &mut self,
        keyword: Self::Output,
        _operand: Self::Output,
    ) -> Self::Output {
        assert!(keyword.token_kind() == Some(TokenKind::Yield));
        keyword
    }

    fn make_const_declaration(
        &mut self,
        _attributes: Self::Output,
        modifiers: Self::Output,
        const_keyword: Self::Output,
        hint: Self::Output,
        decls: Self::Output,
        semicolon: Self::Output,
    ) -> Self::Output {
        match decls {
            // Class consts.
            Node::List(consts) if self.classish_name_builder.is_some() => {
                let ty = self.node_to_ty(hint);
                Node::List(
                    self.alloc(self.slice(consts.iter().filter_map(|cst| match cst {
                        Node::ConstInitializer(&(name, initializer, refs)) => {
                            let id = name.as_id()?;
                            let modifiers = read_member_modifiers(modifiers.iter());
                            let abstract_ = if modifiers.is_abstract {
                                ClassConstKind::CCAbstract(!initializer.is_ignored())
                            } else {
                                ClassConstKind::CCConcrete
                            };
                            let ty = ty
                                .or_else(|| self.infer_const(name, initializer))
                                .unwrap_or(TANY);
                            Some(Node::Const(self.alloc(
                                shallow_decl_defs::ShallowClassConst {
                                    abstract_,
                                    name: id.into(),
                                    type_: ty,
                                    refs,
                                },
                            )))
                        }
                        _ => None,
                    }))),
                )
            }
            // Global consts.
            Node::List(consts) => {
                // This case always returns Node::Ignored,
                // but has the side effect of calling self.add_const

                // Note: given "const int X=1,Y=2;", the legacy decl-parser
                // allows both decls, and it gives them both an identical text-span -
                // from start of "const" to end of semicolon. This is a bug but
                // the code here preserves it.
                let pos = self.merge_positions(const_keyword, semicolon);
                for cst in consts.iter() {
                    match cst {
                        Node::ConstInitializer(&(name, initializer, _refs)) => {
                            if let Some(Id(id_pos, id)) = self.elaborate_defined_id(name) {
                                let ty = self
                                    .node_to_ty(hint)
                                    .or_else(|| self.infer_const(name, initializer))
                                    .unwrap_or_else(|| self.tany_with_pos(id_pos));
                                let this = Rc::make_mut(&mut self.state);
                                this.add_const(id, this.alloc(ConstDecl { pos, type_: ty }));
                            }
                        }
                        _ => {}
                    }
                }
                Node::Ignored(SK::ConstDeclaration)
            }
            _ => Node::Ignored(SK::ConstDeclaration),
        }
    }

    fn begin_constant_declarator(&mut self) {
        self.start_accumulating_const_refs();
    }

    fn make_constant_declarator(
        &mut self,
        name: Self::Output,
        initializer: Self::Output,
    ) -> Self::Output {
        // The "X=1" part of either a member const "class C {const int X=1;}" or a top-level const "const int X=1;"
        // Note: the the declarator itself doesn't yet know whether a type was provided by the user;
        // that's only known in the parent, make_const_declaration
        let refs = self.stop_accumulating_const_refs();
        if name.is_ignored() {
            Node::Ignored(SK::ConstantDeclarator)
        } else {
            Node::ConstInitializer(self.alloc((name, initializer, refs)))
        }
    }

    fn make_namespace_declaration(
        &mut self,
        _name: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        if let Node::Ignored(SK::NamespaceBody) = body {
            let this = Rc::make_mut(&mut self.state);
            Rc::make_mut(&mut this.namespace_builder).pop_namespace();
        }
        Node::Ignored(SK::NamespaceDeclaration)
    }

    fn make_namespace_declaration_header(
        &mut self,
        _keyword: Self::Output,
        name: Self::Output,
    ) -> Self::Output {
        let name = self.expect_name(name).map(|Id(_, name)| name);
        // if this is header of semicolon-style (one with NamespaceEmptyBody) namespace, we should pop
        // the previous namespace first, but we don't have the body yet. We'll fix it retroactively in
        // make_namespace_empty_body
        let this = Rc::make_mut(&mut self.state);
        Rc::make_mut(&mut this.namespace_builder).push_namespace(name);
        Node::Ignored(SK::NamespaceDeclarationHeader)
    }

    fn make_namespace_body(
        &mut self,
        _left_brace: Self::Output,
        _declarations: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::NamespaceBody)
    }

    fn make_namespace_empty_body(&mut self, _semicolon: Self::Output) -> Self::Output {
        let this = Rc::make_mut(&mut self.state);
        Rc::make_mut(&mut this.namespace_builder).pop_previous_namespace();
        Node::Ignored(SK::NamespaceEmptyBody)
    }

    fn make_namespace_use_declaration(
        &mut self,
        _keyword: Self::Output,
        namespace_use_kind: Self::Output,
        clauses: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        if let Some(import_kind) = Self::namespace_use_kind(&namespace_use_kind) {
            for clause in clauses.iter() {
                if let Node::NamespaceUseClause(nuc) = clause {
                    let this = Rc::make_mut(&mut self.state);
                    Rc::make_mut(&mut this.namespace_builder).add_import(
                        import_kind,
                        nuc.id.1,
                        nuc.as_,
                    );
                }
            }
        }
        Node::Ignored(SK::NamespaceUseDeclaration)
    }

    fn make_namespace_group_use_declaration(
        &mut self,
        _keyword: Self::Output,
        _kind: Self::Output,
        prefix: Self::Output,
        _left_brace: Self::Output,
        clauses: Self::Output,
        _right_brace: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let Id(_, prefix) = match self.expect_name(prefix) {
            Some(id) => id,
            None => return Node::Ignored(SK::NamespaceGroupUseDeclaration),
        };
        for clause in clauses.iter() {
            if let Node::NamespaceUseClause(nuc) = clause {
                let mut id = bump::String::new_in(self.arena);
                id.push_str(prefix);
                id.push_str(nuc.id.1);
                let this = Rc::make_mut(&mut self.state);
                Rc::make_mut(&mut this.namespace_builder).add_import(
                    nuc.kind,
                    id.into_bump_str(),
                    nuc.as_,
                );
            }
        }
        Node::Ignored(SK::NamespaceGroupUseDeclaration)
    }

    fn make_namespace_use_clause(
        &mut self,
        clause_kind: Self::Output,
        name: Self::Output,
        as_: Self::Output,
        aliased_name: Self::Output,
    ) -> Self::Output {
        let id = match self.expect_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::NamespaceUseClause),
        };
        let as_ = if as_.is_token(TokenKind::As) {
            match aliased_name.as_id() {
                Some(name) => Some(name.1),
                None => return Node::Ignored(SK::NamespaceUseClause),
            }
        } else {
            None
        };
        if let Some(kind) = Self::namespace_use_kind(&clause_kind) {
            Node::NamespaceUseClause(self.alloc(NamespaceUseClause { kind, id, as_ }))
        } else {
            Node::Ignored(SK::NamespaceUseClause)
        }
    }

    fn make_where_clause(
        &mut self,
        _: Self::Output,
        where_constraints: Self::Output,
    ) -> Self::Output {
        where_constraints
    }

    fn make_where_constraint(
        &mut self,
        left_type: Self::Output,
        operator: Self::Output,
        right_type: Self::Output,
    ) -> Self::Output {
        Node::WhereConstraint(self.alloc(WhereConstraint(
            self.node_to_ty(left_type).unwrap_or(TANY),
            match operator.token_kind() {
                Some(TokenKind::Equal) => ConstraintKind::ConstraintEq,
                Some(TokenKind::Super) => ConstraintKind::ConstraintSuper,
                _ => ConstraintKind::ConstraintAs,
            },
            self.node_to_ty(right_type).unwrap_or(TANY),
        )))
    }

    fn make_classish_declaration(
        &mut self,
        attributes: Self::Output,
        modifiers: Self::Output,
        xhp_keyword: Self::Output,
        class_keyword: Self::Output,
        name: Self::Output,
        tparams: Self::Output,
        _extends_keyword: Self::Output,
        extends: Self::Output,
        _implements_keyword: Self::Output,
        implements: Self::Output,
        where_clause: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let raw_name = match self.expect_name(name) {
            Some(Id(_, name)) => name,
            None => return Node::Ignored(SK::ClassishDeclaration),
        };
        let Id(pos, name) = match self.elaborate_defined_id(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::ClassishDeclaration),
        };
        let is_xhp = raw_name.starts_with(':') || xhp_keyword.is_present();

        let mut final_ = false;
        let mut abstract_ = false;
        let mut internal = false;

        for modifier in modifiers.iter() {
            match modifier.token_kind() {
                Some(TokenKind::Abstract) => {
                    abstract_ = true;
                }
                Some(TokenKind::Final) => final_ = true,
                Some(TokenKind::Internal) => internal = true,
                _ => {}
            }
        }
        let class_kind = match class_keyword.token_kind() {
            Some(TokenKind::Interface) => ClassishKind::Cinterface,
            Some(TokenKind::Trait) => ClassishKind::Ctrait,
            _ => ClassishKind::Cclass(if abstract_ {
                Abstraction::Abstract
            } else {
                Abstraction::Concrete
            }),
        };

        let where_constraints = self.slice(where_clause.iter().filter_map(|&x| match x {
            Node::WhereConstraint(x) => Some(x),
            _ => None,
        }));

        let body = match body {
            Node::ClassishBody(body) => body,
            _ => return Node::Ignored(SK::ClassishDeclaration),
        };

        let mut uses_len = 0;
        let mut xhp_attr_uses_len = 0;
        let mut xhp_enum_values = SMap::empty();
        let mut xhp_marked_empty = false;
        let mut req_extends_len = 0;
        let mut req_implements_len = 0;
        let mut req_class_len = 0;
        let mut consts_len = 0;
        let mut typeconsts_len = 0;
        let mut props_len = 0;
        let mut sprops_len = 0;
        let mut static_methods_len = 0;
        let mut methods_len = 0;

        let mut user_attributes_len = 0;
        for attribute in attributes.iter() {
            match *attribute {
                Node::Attribute(..) => user_attributes_len += 1,
                _ => {}
            }
        }

        for element in body.iter().copied() {
            match element {
                Node::TraitUse(names) => uses_len += names.len(),
                Node::XhpClassAttributeDeclaration(&XhpClassAttributeDeclarationNode {
                    xhp_attr_decls,
                    xhp_attr_uses_decls,
                    xhp_attr_enum_values,
                }) => {
                    props_len += xhp_attr_decls.len();
                    xhp_attr_uses_len += xhp_attr_uses_decls.len();

                    for (name, values) in xhp_attr_enum_values {
                        xhp_enum_values = xhp_enum_values.add(self.arena, name, *values);
                    }
                }
                Node::XhpChildrenDeclaration(XhpChildrenKind::Empty) => {
                    xhp_marked_empty = true;
                }
                Node::TypeConstant(..) => typeconsts_len += 1,
                Node::RequireClause(require) => match require.require_type.token_kind() {
                    Some(TokenKind::Extends) => req_extends_len += 1,
                    Some(TokenKind::Implements) => req_implements_len += 1,
                    Some(TokenKind::Class) => req_class_len += 1,
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

        let mut uses = bump::Vec::with_capacity_in(uses_len, self.arena);
        let mut xhp_attr_uses = bump::Vec::with_capacity_in(xhp_attr_uses_len, self.arena);
        let mut req_extends = bump::Vec::with_capacity_in(req_extends_len, self.arena);
        let mut req_implements = bump::Vec::with_capacity_in(req_implements_len, self.arena);
        let mut req_class = bump::Vec::with_capacity_in(req_class_len, self.arena);
        let mut consts = bump::Vec::with_capacity_in(consts_len, self.arena);
        let mut typeconsts = bump::Vec::with_capacity_in(typeconsts_len, self.arena);
        let mut props = bump::Vec::with_capacity_in(props_len, self.arena);
        let mut sprops = bump::Vec::with_capacity_in(sprops_len, self.arena);
        let mut static_methods = bump::Vec::with_capacity_in(static_methods_len, self.arena);
        let mut methods = bump::Vec::with_capacity_in(methods_len, self.arena);

        let mut user_attributes = bump::Vec::with_capacity_in(user_attributes_len, self.arena);
        let mut docs_url = None;
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => {
                    if attr.name.1 == "__Docs" {
                        if let Some((_, bstr)) = attr.string_literal_param {
                            docs_url = Some(self.str_from_utf8_for_bytes_in_arena(bstr));
                        }
                    }

                    user_attributes.push(self.user_attribute_to_decl(attr));
                }
                _ => {}
            }
        }
        // Match ordering of attributes produced by the OCaml decl parser (even
        // though it's the reverse of the syntactic ordering).
        user_attributes.reverse();

        let class_attributes = self.to_attributes(attributes);

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
                    ..
                }) => {
                    xhp_props.extend(xhp_attr_decls);
                    xhp_attr_uses.extend(
                        xhp_attr_uses_decls
                            .iter()
                            .filter_map(|&node| self.node_to_ty(node)),
                    )
                }
                Node::TypeConstant(constant) => typeconsts.push(constant),
                Node::RequireClause(require) => match require.require_type.token_kind() {
                    Some(TokenKind::Extends) => {
                        req_extends.extend(self.node_to_ty(require.name).iter())
                    }
                    Some(TokenKind::Implements) => {
                        req_implements.extend(self.node_to_ty(require.name).iter())
                    }
                    Some(TokenKind::Class) => {
                        req_class.extend(self.node_to_ty(require.name).iter())
                    }
                    _ => {}
                },
                Node::List(&const_nodes @ [Node::Const(..), ..]) => {
                    for node in const_nodes {
                        if let Node::Const(decl) = *node {
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
                    // Annoyingly, the <<__SupportDynamicType>> annotation on a
                    // class implicitly changes the decls of every method inside
                    // it, so we have to reallocate them here.
                    let method = if (self.implicit_sdt()
                        || class_attributes.support_dynamic_type
                        || class_attributes.dynamically_referenced)
                        && !method.flags.contains(MethodFlags::SUPPORT_DYNAMIC_TYPE)
                    {
                        let type_ = match method.type_.1 {
                            Ty_::Tfun(ft) => {
                                let flags = ft.flags | FunTypeFlags::SUPPORT_DYNAMIC_TYPE;
                                let ft = self.alloc(FunType { flags, ..*ft });
                                self.alloc(Ty(method.type_.0, Ty_::Tfun(ft)))
                            }
                            _ => method.type_,
                        };
                        let flags = method.flags | MethodFlags::SUPPORT_DYNAMIC_TYPE;
                        self.alloc(ShallowMethod {
                            type_,
                            flags,
                            ..*method
                        })
                    } else {
                        method
                    };
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

        if class_attributes.const_ {
            for prop in props.iter_mut() {
                if !prop.flags.contains(PropFlags::CONST) {
                    *prop = self.alloc(ShallowProp {
                        flags: prop.flags | PropFlags::CONST,
                        ..**prop
                    })
                }
            }
        }

        let uses = uses.into_bump_slice();
        let xhp_attr_uses = xhp_attr_uses.into_bump_slice();
        let xhp_enum_values = xhp_enum_values;
        let req_extends = req_extends.into_bump_slice();
        let req_implements = req_implements.into_bump_slice();
        let req_class = req_class.into_bump_slice();
        let consts = consts.into_bump_slice();
        let typeconsts = typeconsts.into_bump_slice();
        let props = props.into_bump_slice();
        let sprops = sprops.into_bump_slice();
        let static_methods = static_methods.into_bump_slice();
        let methods = methods.into_bump_slice();
        let user_attributes = user_attributes.into_bump_slice();
        let extends = self.slice(extends.iter().filter_map(|&node| self.node_to_ty(node)));
        let implements = self.slice(implements.iter().filter_map(|&node| self.node_to_ty(node)));
        let support_dynamic_type = self.implicit_sdt() || class_attributes.support_dynamic_type;
        // Pop the type params stack only after creating all inner types.
        let tparams = self.pop_type_params(tparams);
        let module = self.module;

        let cls = self.alloc(shallow_decl_defs::ShallowClass {
            mode: self.file_mode,
            final_,
            abstract_,
            is_xhp,
            has_xhp_keyword: xhp_keyword.is_token(TokenKind::XHP),
            kind: class_kind,
            module,
            internal,
            name: (pos, name),
            tparams,
            where_constraints,
            extends,
            uses,
            xhp_attr_uses,
            xhp_enum_values,
            xhp_marked_empty,
            req_extends,
            req_implements,
            req_class,
            implements,
            support_dynamic_type,
            consts,
            typeconsts,
            props,
            sprops,
            constructor,
            static_methods,
            methods,
            user_attributes,
            enum_type: None,
            docs_url,
        });
        let this = Rc::make_mut(&mut self.state);
        this.add_class(name, cls);

        this.classish_name_builder = None;

        Node::Ignored(SK::ClassishDeclaration)
    }

    fn make_property_declaration(
        &mut self,
        attrs: Self::Output,
        modifiers: Self::Output,
        hint: Self::Output,
        declarators: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let (attrs, modifiers, hint) = (attrs, modifiers, hint);
        let modifiers = read_member_modifiers(modifiers.iter());
        let declarators = self.slice(declarators.iter().filter_map(
            |declarator| match declarator {
                Node::ListItem(&(name, initializer)) => {
                    let attributes = self.to_attributes(attrs);
                    let Id(pos, name) = name.as_variable()?;
                    let name = if modifiers.is_static {
                        name
                    } else {
                        strip_dollar_prefix(name)
                    };
                    let ty = self.node_to_non_ret_ty(hint);
                    let ty = ty.unwrap_or_else(|| self.tany_with_pos(pos));
                    let ty = if self.opts.interpret_soft_types_as_like_types {
                        if attributes.soft {
                            self.alloc(Ty(
                                self.alloc(Reason::hint(self.get_pos(hint))),
                                Ty_::Tlike(ty),
                            ))
                        } else {
                            ty
                        }
                    } else {
                        ty
                    };
                    let needs_init = if self.file_mode == Mode::Mhhi {
                        false
                    } else {
                        initializer.is_ignored()
                    };
                    let mut flags = PropFlags::empty();
                    flags.set(PropFlags::CONST, attributes.const_);
                    flags.set(PropFlags::LATEINIT, attributes.late_init);
                    flags.set(PropFlags::LSB, attributes.lsb);
                    flags.set(PropFlags::NEEDS_INIT, needs_init);
                    flags.set(PropFlags::ABSTRACT, modifiers.is_abstract);
                    flags.set(PropFlags::READONLY, modifiers.is_readonly);
                    flags.set(PropFlags::PHP_STD_LIB, attributes.php_std_lib);
                    flags.set(
                        PropFlags::SAFE_GLOBAL_VARIABLE,
                        attributes.safe_global_variable,
                    );
                    flags.set(PropFlags::NO_AUTO_LIKES, attributes.no_auto_likes);
                    Some(ShallowProp {
                        xhp_attr: None,
                        name: (pos, name),
                        type_: ty,
                        visibility: modifiers.visibility,
                        flags,
                    })
                }
                _ => None,
            },
        ));
        Node::Property(self.alloc(PropertyNode {
            decls: declarators,
            is_static: modifiers.is_static,
        }))
    }

    fn make_xhp_children_declaration(
        &mut self,
        _keyword: Self::Output,
        expression: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        match expression {
            Node::IgnoredToken(token) if token.kind() == TokenKind::Empty => {
                Node::XhpChildrenDeclaration(XhpChildrenKind::Empty)
            }
            _ => Node::XhpChildrenDeclaration(XhpChildrenKind::Other),
        }
    }

    fn make_xhp_class_attribute_declaration(
        &mut self,
        _keyword: Self::Output,
        attributes: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let mut xhp_attr_enum_values = bump::Vec::new_in(self.arena);

        let xhp_attr_decls = self.slice(attributes.iter().filter_map(|node| {
            let node = match node {
                Node::XhpClassAttribute(x) => x,
                _ => return None,
            };
            let Id(pos, name) = node.name;
            let name = prefix_colon(self.arena, name);

            let (like, type_, enum_values) = match node.hint {
                Node::XhpEnumTy((like, ty, values)) => (*like, *ty, Some(values)),
                _ => (
                    None,
                    self.node_to_ty(node.hint)
                        .unwrap_or_else(|| self.tany_with_pos(pos)),
                    None,
                ),
            };
            if let Some(enum_values) = enum_values {
                xhp_attr_enum_values.push((name, *enum_values));
            };

            let type_ = if node.nullable && node.tag.is_none() {
                match type_ {
                    // already nullable
                    Ty(_, Ty_::Toption(_)) | Ty(_, Ty_::Tmixed) => type_,
                    // make nullable
                    _ => self.alloc(Ty(
                        self.alloc(Reason::hint(type_.get_pos()?)),
                        Ty_::Toption(type_),
                    )),
                }
            } else {
                type_
            };
            let type_ = match like {
                Some(p) => self.alloc(Ty(self.alloc(Reason::hint(p)), Ty_::Tlike(type_))),
                None => type_,
            };

            let mut flags = PropFlags::empty();
            flags.set(PropFlags::NEEDS_INIT, node.needs_init);
            Some(ShallowProp {
                name: (pos, name),
                visibility: aast::Visibility::Public,
                type_,
                xhp_attr: Some(xhp_attribute::XhpAttribute {
                    tag: node.tag,
                    has_default: !node.needs_init,
                }),
                flags,
            })
        }));

        let xhp_attr_uses_decls = self.slice(attributes.iter().filter_map(|x| match x {
            Node::XhpAttributeUse(&name) => Some(name),
            _ => None,
        }));

        Node::XhpClassAttributeDeclaration(self.alloc(XhpClassAttributeDeclarationNode {
            xhp_attr_enum_values: xhp_attr_enum_values.into_bump_slice(),
            xhp_attr_decls,
            xhp_attr_uses_decls,
        }))
    }

    /// Handle XHP attribute enum declarations.
    ///
    ///   class :foo implements XHPChild {
    ///     attribute
    ///       enum {'big', 'small'} size; // this line
    ///   }
    fn make_xhp_enum_type(
        &mut self,
        like: Self::Output,
        enum_keyword: Self::Output,
        _left_brace: Self::Output,
        xhp_enum_values: Self::Output,
        right_brace: Self::Output,
    ) -> Self::Output {
        // Infer the type hint from the first value.
        // TODO: T88207956 consider all the values.
        let ty = xhp_enum_values
            .iter()
            .next()
            .and_then(|node| self.node_to_ty(*node))
            .map(|node_ty| {
                let pos = self.merge_positions(enum_keyword, right_brace);
                let ty_ = node_ty.1;
                self.alloc(Ty(self.alloc(Reason::hint(pos)), ty_))
            });
        let mut values = bump::Vec::new_in(self.arena);
        for node in xhp_enum_values.iter() {
            // XHP enum values may only be string or int literals.
            match node {
                Node::IntLiteral(&(s, _)) => {
                    let i = s.parse::<isize>().unwrap_or(0);
                    values.push(XhpEnumValue::XEVInt(i));
                }
                Node::StringLiteral(&(s, _)) => {
                    let owned_str = String::from_utf8_lossy(s);
                    values.push(XhpEnumValue::XEVString(self.arena.alloc_str(&owned_str)));
                }
                _ => {}
            };
        }

        match ty {
            Some(ty) => {
                Node::XhpEnumTy(self.alloc((self.get_pos_opt(like), ty, values.into_bump_slice())))
            }
            None => Node::Ignored(SK::XHPEnumType),
        }
    }

    fn make_xhp_class_attribute(
        &mut self,
        type_: Self::Output,
        name: Self::Output,
        initializer: Self::Output,
        tag: Self::Output,
    ) -> Self::Output {
        let name = match name.as_id() {
            Some(name) => name,
            None => return Node::Ignored(SK::XHPClassAttribute),
        };
        Node::XhpClassAttribute(self.alloc(XhpClassAttributeNode {
            name,
            hint: type_,
            needs_init: !initializer.is_present(),
            tag: match tag.token_kind() {
                Some(TokenKind::Required) => Some(xhp_attribute::Tag::Required),
                Some(TokenKind::Lateinit) => Some(xhp_attribute::Tag::LateInit),
                _ => None,
            },
            nullable: initializer.is_token(TokenKind::NullLiteral) || !initializer.is_present(),
        }))
    }

    fn make_xhp_simple_class_attribute(&mut self, name: Self::Output) -> Self::Output {
        Node::XhpAttributeUse(self.alloc(name))
    }

    fn make_property_declarator(
        &mut self,
        name: Self::Output,
        initializer: Self::Output,
    ) -> Self::Output {
        Node::ListItem(self.alloc((name, initializer)))
    }

    fn make_methodish_declaration(
        &mut self,
        attrs: Self::Output,
        header: Self::Output,
        body: Self::Output,
        closer: Self::Output,
    ) -> Self::Output {
        let header = match header {
            Node::FunctionHeader(header) => header,
            _ => return Node::Ignored(SK::MethodishDeclaration),
        };
        // If we don't have a body, use the closing token. A closing token of
        // '}' indicates a regular function, while a closing token of ';'
        // indicates an abstract function.
        let body = if body.is_ignored() { closer } else { body };
        let modifiers = read_member_modifiers(header.modifiers.iter());
        let is_constructor = header.name.is_token(TokenKind::Construct);
        let is_method = true;
        let (id, ty, properties) = match self.function_to_ty(is_method, attrs, header, body) {
            Some(tuple) => tuple,
            None => return Node::Ignored(SK::MethodishDeclaration),
        };
        let attributes = self.to_attributes(attrs);
        let deprecated = attributes.deprecated.map(|msg| {
            let mut s = bump::String::new_in(self.arena);
            s.push_str("The method ");
            s.push_str(id.1);
            s.push_str(" is deprecated: ");
            s.push_str(msg);
            s.into_bump_str()
        });
        let sort_text = attributes.sort_text.map(|msg| {
            let mut s = bump::String::new_in(self.arena);
            s.push_str(msg);
            s.into_bump_str()
        });
        let mut flags = MethodFlags::empty();
        flags.set(
            MethodFlags::ABSTRACT,
            self.in_interface() || modifiers.is_abstract,
        );
        flags.set(MethodFlags::FINAL, modifiers.is_final);
        flags.set(MethodFlags::OVERRIDE, attributes.override_);
        flags.set(
            MethodFlags::DYNAMICALLYCALLABLE,
            attributes.dynamically_callable,
        );
        flags.set(MethodFlags::PHP_STD_LIB, attributes.php_std_lib);
        flags.set(
            MethodFlags::SUPPORT_DYNAMIC_TYPE,
            !is_constructor && attributes.support_dynamic_type,
        );

        // Parse the user attributes
        // in facts-mode all attributes are saved, otherwise only __NoAutoDynamic/__NoAutoLikes is
        let user_attributes = self.slice(attrs.iter().rev().filter_map(|attribute| {
            if let Node::Attribute(attr) = attribute {
                if self.opts.keep_user_attributes || is_no_auto_attribute(attr.name.1) {
                    Some(self.user_attribute_to_decl(attr))
                } else {
                    None
                }
            } else {
                None
            }
        }));

        let method = self.alloc(ShallowMethod {
            name: id,
            type_: ty,
            visibility: modifiers.visibility,
            deprecated,
            flags,
            attributes: user_attributes,
            sort_text,
        });
        if !self.inside_no_auto_dynamic_class {
            let this = Rc::make_mut(&mut self.state);
            this.under_no_auto_dynamic = false;
            this.under_no_auto_likes = false;
        }
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
        _left_brace: Self::Output,
        elements: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        Node::ClassishBody(self.alloc(elements.as_slice(self.arena)))
    }

    fn make_enum_declaration(
        &mut self,
        attributes: Self::Output,
        modifiers: Self::Output,
        _keyword: Self::Output,
        name: Self::Output,
        _colon: Self::Output,
        extends: Self::Output,
        constraint: Self::Output,
        _left_brace: Self::Output,
        use_clauses: Self::Output,
        enumerators: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        let id = match self.elaborate_defined_id(name) {
            Some(id) => id,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let hint = match self.node_to_ty(extends) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let extends = match self.node_to_ty(self.make_apply(
            (self.get_pos(name), "\\HH\\BuiltinEnum"),
            name,
            NO_POS,
        )) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::EnumDeclaration),
        };
        let internal = modifiers
            .iter()
            .any(|m| m.as_visibility() == Some(aast::Visibility::Internal));
        let key = id.1;
        let consts = self.slice(enumerators.iter().filter_map(|node| match *node {
            Node::Const(const_) => Some(const_),
            _ => None,
        }));
        let mut user_attributes = bump::Vec::with_capacity_in(attributes.len(), self.arena);
        let mut docs_url = None;
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => {
                    if attr.name.1 == "__Docs" {
                        if let Some((_, bstr)) = attr.string_literal_param {
                            docs_url = Some(self.str_from_utf8_for_bytes_in_arena(bstr));
                        }
                    }

                    user_attributes.push(self.user_attribute_to_decl(attr));
                }
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

        let mut includes_len = 0;
        for element in use_clauses.iter() {
            match element {
                Node::EnumUse(names) => includes_len += names.len(),
                _ => {}
            }
        }
        let mut includes = bump::Vec::with_capacity_in(includes_len, self.arena);
        for element in use_clauses.iter() {
            match element {
                Node::EnumUse(names) => {
                    includes.extend(names.iter().filter_map(|&name| self.node_to_ty(name)))
                }
                _ => {}
            }
        }
        let includes = includes.into_bump_slice();

        let parsed_attributes = self.to_attributes(attributes);

        let cls = self.alloc(shallow_decl_defs::ShallowClass {
            mode: self.file_mode,
            final_: false,
            abstract_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassishKind::Cenum,
            module: self.module,
            internal,
            name: id.into(),
            tparams: &[],
            where_constraints: &[],
            extends: bumpalo::vec![in self.arena; extends].into_bump_slice(),
            uses: &[],
            xhp_attr_uses: &[],
            xhp_enum_values: SMap::empty(),
            xhp_marked_empty: false,
            req_extends: &[],
            req_implements: &[],
            req_class: &[],
            implements: &[],
            support_dynamic_type: parsed_attributes.support_dynamic_type,
            consts,
            typeconsts: &[],
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
            })),
            docs_url,
        });
        let this = Rc::make_mut(&mut self.state);
        this.add_class(key, cls);

        this.classish_name_builder = None;

        Node::Ignored(SK::EnumDeclaration)
    }

    fn make_enum_use(
        &mut self,
        _keyword: Self::Output,
        names: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        Node::EnumUse(self.alloc(names))
    }

    fn begin_enumerator(&mut self) {
        self.start_accumulating_const_refs();
    }

    fn make_enumerator(
        &mut self,
        name: Self::Output,
        _equal: Self::Output,
        value: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let refs = self.stop_accumulating_const_refs();
        let id = match self.expect_name(name) {
            Some(id) => id,
            None => return Node::Ignored(SyntaxKind::Enumerator),
        };

        Node::Const(
            self.alloc(ShallowClassConst {
                abstract_: ClassConstKind::CCConcrete,
                name: id.into(),
                type_: self
                    .infer_const(name, value)
                    .unwrap_or_else(|| self.tany_with_pos(id.0)),
                refs,
            }),
        )
    }

    fn make_enum_class_declaration(
        &mut self,
        attributes: Self::Output,
        modifiers: Self::Output,
        _enum_keyword: Self::Output,
        _class_keyword: Self::Output,
        name: Self::Output,
        _colon: Self::Output,
        base: Self::Output,
        _extends_keyword: Self::Output,
        extends_list: Self::Output,
        _left_brace: Self::Output,
        elements: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        let name = match self.elaborate_defined_id(name) {
            Some(name) => name,
            None => return Node::Ignored(SyntaxKind::EnumClassDeclaration),
        };

        let base_pos = self.get_pos(base);
        let base = self
            .node_to_ty(base)
            .unwrap_or_else(|| self.tany_with_pos(name.0));

        let base = if self.opts.everything_sdt {
            self.alloc(Ty(self.alloc(Reason::hint(base_pos)), Ty_::Tlike(base)))
        } else {
            base
        };

        let mut is_abstract = false;
        let mut is_final = false;
        for modifier in modifiers.iter() {
            match modifier.token_kind() {
                Some(TokenKind::Abstract) => is_abstract = true,
                Some(TokenKind::Final) => is_final = true,
                _ => {}
            }
        }

        let class_kind = if is_abstract {
            ClassishKind::CenumClass(Abstraction::Abstract)
        } else {
            ClassishKind::CenumClass(Abstraction::Concrete)
        };

        let builtin_enum_class_ty = {
            let pos = name.0;
            let enum_class_ty_ = Ty_::Tapply(self.alloc((name.into(), &[])));
            let enum_class_ty = self.alloc(Ty(self.alloc(Reason::hint(pos)), enum_class_ty_));
            let elt_ty_ = Ty_::Tapply(self.alloc((
                (pos, "\\HH\\MemberOf"),
                bumpalo::vec![in self.arena; enum_class_ty, base].into_bump_slice(),
            )));
            let elt_ty = self.alloc(Ty(self.alloc(Reason::hint(pos)), elt_ty_));
            let builtin_enum_ty_ = if is_abstract {
                Ty_::Tapply(self.alloc(((pos, "\\HH\\BuiltinAbstractEnumClass"), &[])))
            } else {
                Ty_::Tapply(self.alloc((
                    (pos, "\\HH\\BuiltinEnumClass"),
                    std::slice::from_ref(self.alloc(elt_ty)),
                )))
            };
            self.alloc(Ty(self.alloc(Reason::hint(pos)), builtin_enum_ty_))
        };

        let consts = self.slice(elements.iter().filter_map(|node| match *node {
            Node::Const(const_) => Some(const_),
            _ => None,
        }));

        let typeconsts = self.slice(elements.iter().filter_map(|node| match *node {
            Node::TypeConstant(tconst) => Some(tconst),
            _ => None,
        }));

        let mut extends = bump::Vec::with_capacity_in(extends_list.len() + 1, self.arena);
        extends.push(builtin_enum_class_ty);
        extends.extend(extends_list.iter().filter_map(|&n| self.node_to_ty(n)));
        let extends = extends.into_bump_slice();
        let includes = &extends[1..];

        let mut user_attributes = bump::Vec::with_capacity_in(attributes.len() + 1, self.arena);
        let mut docs_url = None;
        for attribute in attributes.iter() {
            match attribute {
                Node::Attribute(attr) => {
                    if attr.name.1 == "__Docs" {
                        if let Some((_, bstr)) = attr.string_literal_param {
                            docs_url = Some(self.str_from_utf8_for_bytes_in_arena(bstr));
                        }
                    }

                    user_attributes.push(self.user_attribute_to_decl(attr));
                }
                _ => {}
            }
        }
        let internal = modifiers
            .iter()
            .any(|m| m.as_visibility() == Some(aast::Visibility::Internal));
        user_attributes.push(self.alloc(shallow_decl_defs::UserAttribute {
            name: (name.0, "__EnumClass"),
            params: &[],
        }));
        // Match ordering of attributes produced by the OCaml decl parser (even
        // though it's the reverse of the syntactic ordering).
        user_attributes.reverse();
        let user_attributes = user_attributes.into_bump_slice();

        let parsed_attributes = self.to_attributes(attributes);
        let support_dynamic_type = self.implicit_sdt() || parsed_attributes.support_dynamic_type;

        let cls = self.alloc(shallow_decl_defs::ShallowClass {
            mode: self.file_mode,
            final_: is_final,
            abstract_: is_abstract,
            is_xhp: false,
            has_xhp_keyword: false,
            internal,
            kind: class_kind,
            module: self.module,
            name: name.into(),
            tparams: &[],
            where_constraints: &[],
            extends,
            uses: &[],
            xhp_attr_uses: &[],
            xhp_enum_values: SMap::empty(),
            xhp_marked_empty: false,
            req_extends: &[],
            req_implements: &[],
            req_class: &[],
            implements: &[],
            support_dynamic_type,
            consts,
            typeconsts,
            props: &[],
            sprops: &[],
            constructor: None,
            static_methods: &[],
            methods: &[],
            user_attributes,
            enum_type: Some(self.alloc(EnumType {
                base,
                constraint: None,
                includes,
            })),
            docs_url,
        });
        let this = Rc::make_mut(&mut self.state);
        this.add_class(name.1, cls);

        this.classish_name_builder = None;

        Node::Ignored(SyntaxKind::EnumClassDeclaration)
    }

    fn begin_enum_class_enumerator(&mut self) {
        self.start_accumulating_const_refs();
    }

    fn make_enum_class_enumerator(
        &mut self,
        modifiers: Self::Output,
        type_: Self::Output,
        name: Self::Output,
        _initializer: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let refs = self.stop_accumulating_const_refs();
        let name = match self.expect_name(name) {
            Some(name) => name,
            None => return Node::Ignored(SyntaxKind::EnumClassEnumerator),
        };
        let pos = name.0;
        let has_abstract_keyword = modifiers
            .iter()
            .any(|node| node.is_token(TokenKind::Abstract));
        let abstract_ = if has_abstract_keyword {
            /* default values not allowed atm */
            ClassConstKind::CCAbstract(false)
        } else {
            ClassConstKind::CCConcrete
        };
        let type_pos = self.get_pos(type_);
        let type_ = self
            .node_to_ty(type_)
            .unwrap_or_else(|| self.tany_with_pos(name.0));
        let type_ = if self.opts.everything_sdt {
            self.alloc(Ty(self.alloc(Reason::hint(type_pos)), Ty_::Tlike(type_)))
        } else {
            type_
        };
        let class_name = match self.get_current_classish_name() {
            Some((name, _)) => name,
            None => return Node::Ignored(SyntaxKind::EnumClassEnumerator),
        };
        let enum_class_ty_ = Ty_::Tapply(self.alloc(((pos, class_name), &[])));
        let enum_class_ty = self.alloc(Ty(self.alloc(Reason::hint(pos)), enum_class_ty_));
        let type_ = Ty_::Tapply(self.alloc((
            (pos, "\\HH\\MemberOf"),
            bumpalo::vec![in self.arena; enum_class_ty, type_].into_bump_slice(),
        )));
        let type_ = self.alloc(Ty(self.alloc(Reason::hint(pos)), type_));
        Node::Const(self.alloc(ShallowClassConst {
            abstract_,
            name: name.into(),
            type_,
            refs,
        }))
    }

    fn make_tuple_type_specifier(
        &mut self,
        left_paren: Self::Output,
        tys: Self::Output,
        right_paren: Self::Output,
    ) -> Self::Output {
        // We don't need to include the tys list in this position merging
        // because by definition it's already contained by the two brackets.
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|&node| self.node_to_ty(node)));
        self.hint_ty(pos, Ty_::Ttuple(tys))
    }

    fn make_tuple_type_explicit_specifier(
        &mut self,
        keyword: Self::Output,
        _left_angle: Self::Output,
        types: Self::Output,
        right_angle: Self::Output,
    ) -> Self::Output {
        let id = (self.get_pos(keyword), "\\tuple");
        // This is an error--tuple syntax is (A, B), not tuple<A, B>.
        // OCaml decl makes a Tapply rather than a Ttuple here.
        self.make_apply(id, types, self.get_pos(right_angle))
    }

    fn make_intersection_type_specifier(
        &mut self,
        left_paren: Self::Output,
        tys: Self::Output,
        right_paren: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|x| match x {
            Node::ListItem(&(ty, _ampersand)) => self.node_to_ty(ty),
            &x => self.node_to_ty(x),
        }));
        self.hint_ty(pos, Ty_::Tintersection(tys))
    }

    fn make_union_type_specifier(
        &mut self,
        left_paren: Self::Output,
        tys: Self::Output,
        right_paren: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(left_paren, right_paren);
        let tys = self.slice(tys.iter().filter_map(|x| match x {
            Node::ListItem(&(ty, _bar)) => self.node_to_ty(ty),
            &x => self.node_to_ty(x),
        }));
        self.hint_ty(pos, Ty_::Tunion(tys))
    }

    fn make_shape_type_specifier(
        &mut self,
        shape: Self::Output,
        _lparen: Self::Output,
        fields: Self::Output,
        open: Self::Output,
        rparen: Self::Output,
    ) -> Self::Output {
        let fields = fields;
        let fields_iter = fields.iter();
        let mut fields = AssocListMut::new_in(self.arena);
        for node in fields_iter {
            if let Node::ShapeFieldSpecifier(&ShapeFieldNode { name, type_ }) = *node {
                fields.insert(self.make_t_shape_field_name(name), type_)
            }
        }
        let pos = self.get_pos(open);
        let reason = self.alloc(Reason::hint(pos));
        let kind = match open.token_kind() {
            // Type of unknown fields is mixed, or supportdyn<mixed> under implicit SD
            Some(TokenKind::DotDotDot) => self.alloc(Ty(
                reason,
                if self.implicit_sdt() {
                    self.make_supportdyn(pos, Ty_::Tmixed)
                } else {
                    Ty_::Tmixed
                },
            )),
            // Closed shapes are expressed using `nothing` (empty union) as the type of unknown fields
            _ => self.alloc(Ty(reason, Ty_::Tunion(&[]))),
        };
        let pos = self.merge_positions(shape, rparen);
        let origin = TypeOrigin::MissingOrigin;
        self.hint_ty(
            pos,
            Ty_::Tshape(self.alloc(ShapeType {
                origin,
                unknown_value: kind,
                fields: fields.into(),
            })),
        )
    }

    fn make_classname_type_specifier(
        &mut self,
        classname: Self::Output,
        _lt: Self::Output,
        targ: Self::Output,
        _trailing_comma: Self::Output,
        gt: Self::Output,
    ) -> Self::Output {
        let id = match classname.as_id() {
            Some(id) => id,
            None => return Node::Ignored(SK::ClassnameTypeSpecifier),
        };
        if gt.is_ignored() {
            self.prim_ty(aast::Tprim::Tstring, id.0)
        } else {
            self.make_apply(
                (id.0, self.elaborate_raw_id(id.1)),
                targ,
                self.merge_positions(classname, gt),
            )
        }
    }

    fn make_class_args_type_specifier(
        &mut self,
        class: Self::Output,
        _lt: Self::Output,
        targ: Self::Output,
        _trailing_comma: Self::Output,
        _gt: Self::Output,
    ) -> Self::Output {
        self.make_apply(
            (
                self.get_pos(class),
                naming_special_names::classes::CLASS_NAME,
            ),
            targ,
            self.get_pos(targ),
        )
    }

    fn make_scope_resolution_expression(
        &mut self,
        class_name: Self::Output,
        _operator: Self::Output,
        value: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(class_name, value);
        let Id(class_name_pos, class_name_str) = match self.expect_name(class_name) {
            Some(id) => {
                if matches!(class_name, Node::XhpName(..))
                    && self.opts.disable_xhp_element_mangling
                    && self.opts.keep_user_attributes
                {
                    // for facts, allow xhp class consts to be mangled later
                    // on even when xhp_element_mangling is disabled
                    let mut qualified = bump::String::with_capacity_in(id.1.len() + 1, self.arena);
                    qualified.push_str("\\");
                    qualified.push_str(id.1);
                    Id(id.0, self.arena.alloc_str(&qualified))
                } else {
                    self.elaborate_id(id)
                }
            }
            None => return Node::Ignored(SK::ScopeResolutionExpression),
        };
        let class_id = self.alloc(aast::ClassId(
            (),
            class_name_pos,
            match class_name {
                Node::Name(("self", _)) => aast::ClassId_::CIself,
                _ => aast::ClassId_::CI(self.alloc(Id(class_name_pos, class_name_str))),
            },
        ));
        let value_id = match self.expect_name(value) {
            Some(id) => id,
            None => return Node::Ignored(SK::ScopeResolutionExpression),
        };
        self.accumulate_const_ref(class_id, &value_id);
        Node::Expr(self.alloc(aast::Expr(
            (),
            pos,
            nast::Expr_::ClassConst(self.alloc((class_id, self.alloc((value_id.0, value_id.1))))),
        )))
    }

    fn make_field_specifier(
        &mut self,
        question_token: Self::Output,
        name: Self::Output,
        _arrow: Self::Output,
        type_: Self::Output,
    ) -> Self::Output {
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

    fn make_field_initializer(
        &mut self,
        key: Self::Output,
        _arrow: Self::Output,
        value: Self::Output,
    ) -> Self::Output {
        Node::ListItem(self.alloc((key, value)))
    }

    fn make_varray_type_specifier(
        &mut self,
        varray_keyword: Self::Output,
        _less_than: Self::Output,
        tparam: Self::Output,
        _trailing_comma: Self::Output,
        greater_than: Self::Output,
    ) -> Self::Output {
        let tparam = match self.node_to_ty(tparam) {
            Some(ty) => ty,
            None => self.tany_with_pos(self.get_pos(varray_keyword)),
        };
        self.hint_ty(
            self.merge_positions(varray_keyword, greater_than),
            Ty_::Tapply(self.alloc((
                (
                    self.get_pos(varray_keyword),
                    naming_special_names::collections::VEC,
                ),
                self.alloc([tparam]),
            ))),
        )
    }

    fn make_darray_type_specifier(
        &mut self,
        darray: Self::Output,
        _less_than: Self::Output,
        key_type: Self::Output,
        _comma: Self::Output,
        value_type: Self::Output,
        _trailing_comma: Self::Output,
        greater_than: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(darray, greater_than);
        let key_type = self.node_to_ty(key_type).unwrap_or(TANY);
        let value_type = self.node_to_ty(value_type).unwrap_or(TANY);
        self.hint_ty(
            pos,
            Ty_::Tapply(self.alloc((
                (
                    self.get_pos(darray),
                    naming_special_names::collections::DICT,
                ),
                self.alloc([key_type, value_type]),
            ))),
        )
    }

    fn make_old_attribute_specification(
        &mut self,
        ltlt: Self::Output,
        attrs: Self::Output,
        gtgt: Self::Output,
    ) -> Self::Output {
        if attrs.contains_marker_attribute("__NoAutoDynamic") {
            let this = Rc::make_mut(&mut self.state);
            this.under_no_auto_dynamic = true;
        }
        if attrs.contains_marker_attribute("__NoAutoLikes") {
            let this = Rc::make_mut(&mut self.state);
            this.under_no_auto_likes = true;
        }
        match attrs {
            Node::List(nodes) => {
                Node::BracketedList(self.alloc((self.get_pos(ltlt), nodes, self.get_pos(gtgt))))
            }
            _ => Node::Ignored(SK::OldAttributeSpecification),
        }
    }

    fn make_constructor_call(
        &mut self,
        name: Self::Output,
        _left_paren: Self::Output,
        args: Self::Output,
        _right_paren: Self::Output,
    ) -> Self::Output {
        let unqualified_name = match self.expect_name(name) {
            Some(name) => name,
            None => return Node::Ignored(SK::ConstructorCall),
        };
        let name = if unqualified_name.1.starts_with("__") {
            unqualified_name
        } else {
            match self.expect_name(name) {
                Some(name) => self.elaborate_id(name),
                None => return Node::Ignored(SK::ConstructorCall),
            }
        };
        let params = self.slice(args.iter().filter_map(|node| match node {
            Node::Expr(aast::Expr(
                _,
                _,
                aast::Expr_::ClassConst(&(
                    aast::ClassId(_, _, aast::ClassId_::CI(&Id(pos, class_name))),
                    (_, "class"),
                )),
            )) => {
                let name = if class_name.starts_with(':') && self.opts.disable_xhp_element_mangling
                {
                    // for facts, allow xhp class consts to be mangled later on
                    // even when xhp_element_mangling is disabled
                    let mut qualified =
                        bump::String::with_capacity_in(class_name.len() + 1, self.arena);
                    qualified.push_str("\\");
                    qualified.push_str(class_name);
                    Id(pos, self.arena.alloc_str(&qualified))
                } else {
                    self.elaborate_id(Id(pos, class_name))
                };
                Some(AttributeParam::Classname(name))
            }
            Node::EnumClassLabel(label) => Some(AttributeParam::EnumClassLabel(label)),
            Node::Expr(e @ aast::Expr(_, pos, _)) => {
                // Try to parse a sequence of string concatenations
                let mut acc = bump::Vec::new_in(self.arena);
                self.fold_string_concat(e, &mut acc)
                    .then(|| AttributeParam::String(pos, acc.into_bump_slice().into()))
            }
            Node::StringLiteral((slit, pos)) => Some(AttributeParam::String(pos, slit)),
            Node::IntLiteral((ilit, _)) => Some(AttributeParam::Int(ilit)),
            _ => None,
        }));
        let string_literal_param = params.first().and_then(|p| match *p {
            AttributeParam::String(pos, s) => Some((pos, s)),
            _ => None,
        });
        Node::Attribute(self.alloc(UserAttributeNode {
            name,
            params,
            string_literal_param,
        }))
    }

    fn make_trait_use(
        &mut self,
        _keyword: Self::Output,
        names: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        Node::TraitUse(self.alloc(names))
    }

    fn make_require_clause(
        &mut self,
        _keyword: Self::Output,
        require_type: Self::Output,
        name: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        Node::RequireClause(self.alloc(RequireClause { require_type, name }))
    }

    fn make_nullable_type_specifier(
        &mut self,
        question_mark: Self::Output,
        hint: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(question_mark, hint);
        let ty = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::NullableTypeSpecifier),
        };
        self.hint_ty(pos, Ty_::Toption(ty))
    }

    fn make_like_type_specifier(
        &mut self,
        tilde: Self::Output,
        hint: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(tilde, hint);
        let ty = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::LikeTypeSpecifier),
        };
        self.hint_ty(pos, Ty_::Tlike(ty))
    }

    fn make_closure_type_specifier(
        &mut self,
        outer_left_paren: Self::Output,
        readonly_keyword: Self::Output,
        _function_keyword: Self::Output,
        _inner_left_paren: Self::Output,
        parameter_list: Self::Output,
        _inner_right_paren: Self::Output,
        capability: Self::Output,
        _colon: Self::Output,
        readonly_ret: Self::Output,
        return_type: Self::Output,
        outer_right_paren: Self::Output,
    ) -> Self::Output {
        let mut ft_variadic = false;
        let mut make_param = |fp: &'a FunParamDecl<'a>| -> &'a FunParam<'a> {
            let mut flags = FunParamFlags::empty();

            let pos = self.get_pos(fp.hint);
            let mut param_type = self.node_to_ty(fp.hint).unwrap_or(TANY);
            if let ParamMode::FPinout = fp.kind {
                flags |= FunParamFlags::INOUT;
                // Pessimise type for inout
                param_type = if self.implicit_sdt() && !self.no_auto_likes() {
                    self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tlike(param_type)))
                } else {
                    param_type
                }
            };

            if fp.readonly {
                flags |= FunParamFlags::READONLY;
            }
            if fp.variadic {
                ft_variadic = true;
            }

            self.alloc(FunParam {
                pos,
                name: None,
                type_: self.alloc(PossiblyEnforcedTy {
                    enforced: Enforcement::Unenforced,
                    type_: param_type,
                }),
                flags,
            })
        };

        let params = self.slice(parameter_list.iter().filter_map(|&node| match node {
            Node::FunParam(fp) => Some(make_param(fp)),
            _ => None,
        }));

        let ret = match self.node_to_ty(return_type) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::ClosureTypeSpecifier),
        };
        let pos = self.merge_positions(outer_left_paren, outer_right_paren);
        let implicit_params = self.as_fun_implicit_params(capability, pos);

        let mut flags = FunTypeFlags::empty();
        if readonly_ret.is_token(TokenKind::Readonly) {
            flags |= FunTypeFlags::RETURNS_READONLY;
        }
        if readonly_keyword.is_token(TokenKind::Readonly) {
            flags |= FunTypeFlags::READONLY_THIS;
        }
        if ft_variadic {
            flags |= FunTypeFlags::VARIADIC
        }

        let pess_return_type = if self.implicit_sdt() && !self.no_auto_likes() {
            self.alloc(Ty(self.alloc(Reason::hint(pos)), Ty_::Tlike(ret)))
        } else {
            ret
        };
        let fty = Ty_::Tfun(self.alloc(FunType {
            tparams: &[],
            where_constraints: &[],
            params,
            implicit_params,
            ret: self.alloc(PossiblyEnforcedTy {
                enforced: Enforcement::Unenforced,
                type_: pess_return_type,
            }),
            flags,
            cross_package: None,
        }));

        if self.implicit_sdt() {
            self.hint_ty(pos, self.make_supportdyn(pos, fty))
        } else {
            self.hint_ty(pos, fty)
        }
    }

    fn make_closure_parameter_type_specifier(
        &mut self,
        inout: Self::Output,
        readonly: Self::Output,
        hint: Self::Output,
    ) -> Self::Output {
        let kind = if inout.is_token(TokenKind::Inout) {
            ParamMode::FPinout
        } else {
            ParamMode::FPnormal
        };
        Node::FunParam(self.alloc(FunParamDecl {
            attributes: Node::Ignored(SK::Missing),
            visibility: Node::Ignored(SK::Missing),
            kind,
            hint,
            readonly: readonly.is_token(TokenKind::Readonly),
            pos: self.get_pos(hint),
            name: Some(""),
            variadic: false,
            initializer: Node::Ignored(SK::Missing),
        }))
    }

    fn make_type_const_declaration(
        &mut self,
        attributes: Self::Output,
        modifiers: Self::Output,
        _const_keyword: Self::Output,
        _type_keyword: Self::Output,
        name: Self::Output,
        _type_parameters: Self::Output,
        constraints: Self::Output,
        _equal: Self::Output,
        type_: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let attributes = self.to_attributes(attributes);
        let has_abstract_keyword = modifiers
            .iter()
            .any(|node| node.is_token(TokenKind::Abstract));
        let reduce_bounds = |mut constraints: bump::Vec<'a, &Ty<'a>>,
                             f: fn(&'a [&Ty<'a>]) -> Ty_<'a>| {
            if constraints.len() == 1 {
                constraints.pop().map(|ty| self.alloc(ty.clone()))
            } else {
                #[allow(clippy::manual_map)]
                // map doesn't allow moving out of borrowed constraints
                match constraints.first() {
                    None => None, // no bounds
                    Some(fst) => Some(self.alloc(Ty(fst.0, f(constraints.into_bump_slice())))),
                }
            }
        };
        let type_ = self.node_to_ty(type_);
        let kind = if has_abstract_keyword {
            // Abstract type constant in EBNF-like notation:
            //     abstract const type T {as U | super L} [= D];
            let (lower, upper) = self.partition_type_bounds_into_lower_and_upper(constraints);
            Typeconst::TCAbstract(self.alloc(AbstractTypeconst {
                // `as T1 as T2 as ...` == `as (T1 & T2 & ...)`
                as_constraint: reduce_bounds(upper, |tys| Ty_::Tintersection(tys)),
                // `super T1 super T2 super ...` == `super (T1 | T2 | ...)`
                super_constraint: reduce_bounds(lower, |tys| Ty_::Tunion(tys)),
                default: type_,
            }))
        } else if let Some(tc_type) = type_ {
            // Concrete type constant:
            //     const type T = Z;
            Typeconst::TCConcrete(self.alloc(ConcreteTypeconst { tc_type }))
        } else {
            // concrete or type constant requires a value
            return Node::Ignored(SK::TypeConstDeclaration);
        };
        let name = match name.as_id() {
            Some(name) => name,
            None => return Node::Ignored(SK::TypeConstDeclaration),
        };
        Node::TypeConstant(self.alloc(ShallowTypeconst {
            name: name.into(),
            kind,
            enforceable: match attributes.enforceable {
                Some(pos) => (pos, true),
                None => (NO_POS, false),
            },
            reifiable: attributes.reifiable,
            is_ctx: false,
        }))
    }

    fn make_context_const_declaration(
        &mut self,
        modifiers: Self::Output,
        _const_keyword: Self::Output,
        _ctx_keyword: Self::Output,
        name: Self::Output,
        _type_parameters: Self::Output,
        constraints: Self::Output,
        _equal: Self::Output,
        ctx_list: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        let name = match name.as_id() {
            Some(name) => name,
            None => return Node::Ignored(SK::TypeConstDeclaration),
        };
        let has_abstract_keyword = modifiers
            .iter()
            .any(|node| node.is_token(TokenKind::Abstract));
        let context = self.node_to_ty(ctx_list);

        // note: lowerer ensures that there's at most 1 constraint of each kind
        let mut as_constraint = None;
        let mut super_constraint = None;
        for c in constraints.iter() {
            if let Node::ContextConstraint(&(kind, hint)) = c {
                let ty = self.node_to_ty(hint);
                match kind {
                    ConstraintKind::ConstraintSuper => super_constraint = ty,
                    ConstraintKind::ConstraintAs => as_constraint = ty,
                    _ => {}
                }
            }
        }
        let kind = if has_abstract_keyword {
            Typeconst::TCAbstract(self.alloc(AbstractTypeconst {
                as_constraint,
                super_constraint,
                default: context,
            }))
        } else if let Some(tc_type) = context {
            Typeconst::TCConcrete(self.alloc(ConcreteTypeconst { tc_type }))
        } else {
            /* Concrete type const must have a value */
            return Node::Ignored(SK::TypeConstDeclaration);
        };
        Node::TypeConstant(self.alloc(ShallowTypeconst {
            name: name.into(),
            kind,
            enforceable: (NO_POS, false),
            reifiable: None,
            is_ctx: true,
        }))
    }

    fn make_decorated_expression(
        &mut self,
        decorator: Self::Output,
        expr: Self::Output,
    ) -> Self::Output {
        Node::ListItem(self.alloc((decorator, expr)))
    }

    fn make_type_constant(
        &mut self,
        ty: Self::Output,
        _coloncolon: Self::Output,
        constant_name: Self::Output,
    ) -> Self::Output {
        let id = match self.expect_name(constant_name) {
            Some(id) => id,
            None => return Node::Ignored(SK::TypeConstant),
        };
        let pos = self.merge_positions(ty, constant_name);
        let ty = match (ty, self.get_current_classish_name()) {
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
                let ty_ = Ty_::Tapply(self.alloc(((id_pos, name), &[][..])));
                self.alloc(Ty(reason, ty_))
            }
            _ => match self.node_to_ty(ty) {
                Some(ty) => ty,
                None => return Node::Ignored(SK::TypeConstant),
            },
        };
        let reason = self.alloc(Reason::hint(pos));
        // The reason-rewriting here is only necessary to match the
        // behavior of OCaml decl (which flattens and then unflattens
        // Haccess hints, losing some position information).
        let ty = self.rewrite_taccess_reasons(ty, reason);
        Node::Ty(self.alloc(Ty(
            reason,
            Ty_::Taccess(self.alloc(TaccessType(ty, id.into()))),
        )))
    }

    fn make_type_in_refinement(
        &mut self,
        _type_keyword: Self::Output,
        type_constant_name: Self::Output,
        _type_params: Self::Output,
        constraints: Self::Output,
        _equal_token: Self::Output,
        type_specifier: Self::Output,
    ) -> Self::Output {
        let Id(_, id) = match self.expect_name(type_constant_name) {
            Some(id) => id,
            None => return Node::Ignored(SK::TypeInRefinement),
        };
        let bound = if type_specifier.is_ignored() {
            // A loose refinement, with bounds
            let (lower, upper) = self.partition_type_bounds_into_lower_and_upper(constraints);
            RefinedConstBound::TRloose(self.alloc(RefinedConstBounds {
                lower: lower.into_bump_slice(),
                upper: upper.into_bump_slice(),
            }))
        } else {
            // An exact refinement
            let ty = match self.node_to_ty(type_specifier) {
                Some(ty) => ty,
                None => return Node::Ignored(SK::TypeInRefinement),
            };
            RefinedConstBound::TRexact(ty)
        };
        Node::RefinedConst(self.alloc((
            id,
            RefinedConst {
                bound,
                is_ctx: false,
            },
        )))
    }

    fn make_ctx_in_refinement(
        &mut self,
        _ctx_keyword: Self::Output,
        ctx_constant_name: Self::Output,
        _type_params: Self::Output,
        constraints: Self::Output,
        _equal_token: Self::Output,
        ctx_list: Self::Output,
    ) -> Self::Output {
        let Id(_, id) = match self.expect_name(ctx_constant_name) {
            Some(id) => id,
            None => return Node::Ignored(SK::TypeInRefinement),
        };
        let bound = if ctx_list.is_ignored() {
            // A loose refinement, with bounds
            let (lower, upper) = self.partition_ctx_bounds_into_lower_and_upper(constraints);
            RefinedConstBound::TRloose(self.alloc(RefinedConstBounds {
                lower: lower.into_bump_slice(),
                upper: upper.into_bump_slice(),
            }))
        } else {
            // An exact refinement
            let ty = match self.node_to_ty(ctx_list) {
                Some(ty) => ty,
                None => return Node::Ignored(SK::TypeInRefinement),
            };
            RefinedConstBound::TRexact(ty)
        };
        Node::RefinedConst(self.alloc((
            id,
            RefinedConst {
                bound,
                is_ctx: true,
            },
        )))
    }

    fn make_type_refinement(
        &mut self,
        root_type: Self::Output,
        _with_keyword: Self::Output,
        _left_brace: Self::Output,
        members: Self::Output,
        right_brace: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(root_type, right_brace);
        let reason = self.alloc(Reason::hint(pos));
        let root_type = match self.node_to_ty(root_type) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::TypeRefinement),
        };
        let const_members = arena_collections::map::Map::from(
            self.arena,
            members.iter().filter_map(|node| match node {
                Node::ListItem(&(node, _)) | &node => match node {
                    Node::RefinedConst(&(id, ctr)) => Some((id, ctr)),
                    _ => None,
                },
            }),
        );
        let class_ref = ClassRefinement {
            cr_consts: const_members,
        };
        Node::Ty(self.alloc(Ty(
            reason,
            Ty_::Trefinement(self.alloc((root_type, class_ref))),
        )))
    }

    fn make_soft_type_specifier(
        &mut self,
        at_token: Self::Output,
        hint: Self::Output,
    ) -> Self::Output {
        let pos = self.merge_positions(at_token, hint);
        let hint = match self.node_to_ty(hint) {
            Some(ty) => ty,
            None => return Node::Ignored(SK::SoftTypeSpecifier),
        };
        // Use the type of the hint as-is (i.e., throw away the knowledge that
        // we had a soft type specifier here--the typechecker does not use it).
        // Replace its Reason with one including the position of the `@` token.
        self.hint_ty(
            pos,
            if self.opts.interpret_soft_types_as_like_types {
                Ty_::Tlike(hint)
            } else {
                hint.1
            },
        )
    }

    fn make_attribute_specification(&mut self, attributes: Self::Output) -> Self::Output {
        if attributes.contains_marker_attribute("__NoAutoDynamic") {
            let this = Rc::make_mut(&mut self.state);
            this.under_no_auto_dynamic = true;
        }
        if attributes.contains_marker_attribute("__NoAutoLikes") {
            let this = Rc::make_mut(&mut self.state);
            this.under_no_auto_likes = true;
        }
        if self.opts.keep_user_attributes {
            attributes
        } else {
            Node::Ignored(SK::AttributeSpecification)
        }
    }

    fn make_attribute(&mut self, _at: Self::Output, attribute: Self::Output) -> Self::Output {
        if self.opts.keep_user_attributes {
            attribute
        } else {
            Node::Ignored(SK::Attribute)
        }
    }

    // A type specifier preceded by an attribute list. At the time of writing,
    // only the <<__Soft>> attribute is permitted here.
    fn make_attributized_specifier(
        &mut self,
        attributes: Self::Output,
        hint: Self::Output,
    ) -> Self::Output {
        match attributes {
            Node::BracketedList((
                ltlt_pos,
                [
                    Node::Attribute(UserAttributeNode {
                        name: Id(_, "__Soft"),
                        ..
                    }),
                ],
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

                self.hint_ty(
                    self.merge(attributes_pos, hint_pos),
                    if self.opts.interpret_soft_types_as_like_types {
                        Ty_::Tlike(hint)
                    } else {
                        hint.1
                    },
                )
            }
            _ => hint,
        }
    }

    fn make_vector_type_specifier(
        &mut self,
        vec: Self::Output,
        _left_angle: Self::Output,
        hint: Self::Output,
        _trailing_comma: Self::Output,
        right_angle: Self::Output,
    ) -> Self::Output {
        let id = match self.expect_name(vec) {
            Some(id) => id,
            None => return Node::Ignored(SK::VectorTypeSpecifier),
        };
        let id = (id.0, self.elaborate_raw_id(id.1));
        self.make_apply(id, hint, self.get_pos(right_angle))
    }

    fn make_dictionary_type_specifier(
        &mut self,
        dict: Self::Output,
        _left_angle: Self::Output,
        type_arguments: Self::Output,
        right_angle: Self::Output,
    ) -> Self::Output {
        let id = match self.expect_name(dict) {
            Some(id) => id,
            None => return Node::Ignored(SK::DictionaryTypeSpecifier),
        };
        let id = (id.0, self.elaborate_raw_id(id.1));
        self.make_apply(id, type_arguments, self.get_pos(right_angle))
    }

    fn make_keyset_type_specifier(
        &mut self,
        keyset: Self::Output,
        _left_angle: Self::Output,
        hint: Self::Output,
        _trailing_comma: Self::Output,
        right_angle: Self::Output,
    ) -> Self::Output {
        let id = match self.expect_name(keyset) {
            Some(id) => id,
            None => return Node::Ignored(SK::KeysetTypeSpecifier),
        };
        let id = (id.0, self.elaborate_raw_id(id.1));
        self.make_apply(id, hint, self.get_pos(right_angle))
    }

    fn make_variable_expression(&mut self, _expression: Self::Output) -> Self::Output {
        Node::Ignored(SK::VariableExpression)
    }

    fn make_file_attribute_specification(
        &mut self,
        _left_double_angle: Self::Output,
        _keyword: Self::Output,
        _colon: Self::Output,
        attributes: Self::Output,
        _right_double_angle: Self::Output,
    ) -> Self::Output {
        if self.opts.keep_user_attributes {
            let this = Rc::make_mut(&mut self.state);
            this.file_attributes = List::empty();
            for attr in attributes.iter() {
                match attr {
                    Node::Attribute(attr) => this
                        .file_attributes
                        .push_front(this.user_attribute_to_decl(attr), this.arena),
                    _ => {}
                }
            }
        }
        Node::Ignored(SK::FileAttributeSpecification)
    }

    fn make_subscript_expression(
        &mut self,
        _receiver: Self::Output,
        _left_bracket: Self::Output,
        _index: Self::Output,
        _right_bracket: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::SubscriptExpression)
    }

    fn make_member_selection_expression(
        &mut self,
        _object: Self::Output,
        _operator: Self::Output,
        _name: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::MemberSelectionExpression)
    }

    fn make_object_creation_expression(
        &mut self,
        _new_keyword: Self::Output,
        _object: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::ObjectCreationExpression)
    }

    fn make_safe_member_selection_expression(
        &mut self,
        _object: Self::Output,
        _operator: Self::Output,
        _name: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::SafeMemberSelectionExpression)
    }

    fn make_function_call_expression(
        &mut self,
        _receiver: Self::Output,
        _type_args: Self::Output,
        _left_paren: Self::Output,
        _argument_list: Self::Output,
        _right_paren: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::FunctionCallExpression)
    }

    fn make_list_expression(
        &mut self,
        _keyword: Self::Output,
        _left_paren: Self::Output,
        _members: Self::Output,
        _right_paren: Self::Output,
    ) -> Self::Output {
        Node::Ignored(SK::ListExpression)
    }

    fn make_module_declaration(
        &mut self,
        _attributes: Self::Output,
        _new_keyword: Self::Output,
        _module_keyword: Self::Output,
        name: Self::Output,
        _left_brace: Self::Output,
        exports: Self::Output,
        imports: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        if let Node::ModuleName(&(parts, pos)) = name {
            let module_name = self.module_name_string_from_parts(parts, pos);
            let map_references = |references_list| match references_list {
                Node::List(&references) => Some(self.slice(references.iter().filter_map(
                    |reference| match reference {
                        Node::ModuleName(&(name, _)) => {
                            Some(self.module_reference_from_parts(module_name, name))
                        }
                        _ => None,
                    },
                ))),
                _ => None,
            };
            let exports = map_references(exports);
            let imports = map_references(imports);
            let module = self.alloc(shallow_decl_defs::ModuleDefType {
                pos,
                exports,
                imports,
            });
            let this = Rc::make_mut(&mut self.state);
            this.add_module(module_name, module);
        }
        Node::Ignored(SK::ModuleDeclaration)
    }

    fn make_module_exports(
        &mut self,
        _exports_keyword: Self::Output,
        _left_brace: Self::Output,
        clauses: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        match clauses {
            Node::List(_) => clauses,
            _ => Node::List(self.alloc(bumpalo::vec![in self.arena;].into_bump_slice())),
        }
    }

    fn make_module_imports(
        &mut self,
        _imports_keyword: Self::Output,
        _left_brace: Self::Output,
        clauses: Self::Output,
        _right_brace: Self::Output,
    ) -> Self::Output {
        match clauses {
            Node::List(_) => clauses,
            _ => Node::List(self.alloc(bumpalo::vec![in self.arena;].into_bump_slice())),
        }
    }

    fn make_module_membership_declaration(
        &mut self,
        _module_keyword: Self::Output,
        name: Self::Output,
        _semicolon: Self::Output,
    ) -> Self::Output {
        match name {
            Node::ModuleName(&(parts, pos)) => {
                if self.module.is_none() {
                    let name = self.module_name_string_from_parts(parts, pos);
                    let this = Rc::make_mut(&mut self.state);
                    this.module = Some(Id(pos, name));
                }
            }
            _ => {}
        }
        Node::Ignored(SK::ModuleMembershipDeclaration)
    }
}

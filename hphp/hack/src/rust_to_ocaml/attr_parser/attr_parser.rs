// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use syn::LitStr;
use syn::Result;

static DOC: &str = "doc";
static RUST_TO_OCAML: &str = "rust_to_ocaml";
static PREFIX: &str = "prefix";
static AND: &str = "and";
static ATTR: &str = "attr";
static NAME: &str = "name";
static INLINE_TUPLE: &str = "inline_tuple";

/// The attributes understood by `rust_to_ocaml`.
#[derive(Clone, Debug)]
pub struct Attrs {
    /// Doc comments (and their desugared form, the `#[doc]` attribute) are
    /// picked up by rust_to_ocaml and included as ocamldoc comments.
    ///
    ///     /// Type A
    ///     pub type A = X;
    ///
    /// is converted to:
    ///
    ///     (** Type A *)
    ///     type a = x
    pub doc: Vec<String>,

    /// Sometimes OCaml programs use prefixes to distinguish fields of the same
    /// name in different records (avoiding OCaml warning 30). The `prefix`
    /// attribute (on a declaration of a struct or a struct-like enum variant)
    /// indicates the prefix that should be added to each field name.
    ///
    ///     #[rust_to_ocaml(prefix = "a_")]
    ///     pub struct StructA { pub foo: isize, pub bar: isize }
    ///     #[rust_to_ocaml(prefix = "b_")]
    ///     pub struct StructB { pub foo: isize, pub bar: isize }
    ///
    /// is converted to:
    ///
    ///     type struct_a = { a_foo: int; a_bar: int; }
    ///     type struct_b = { b_foo: int; b_bar: int; }
    pub prefix: Option<String>,

    /// OCaml attributes (in OCaml syntax) to annotate a type or field
    /// declaration with in the generated OCaml.
    ///
    ///     #[rust_to_ocaml(attr = "deriving show")]
    ///     pub type X = A;
    ///
    /// is converted to:
    ///
    ///     type x = a [@@deriving show]
    pub attrs: Vec<String>,

    /// Mutual recursion in type definitions is opt-in in OCaml; one writes
    /// `type x = y list and y = x list` rather than `type x = y list ;; type y
    /// = x list` (which is an error because the type name `y` is not bound in
    /// the declaration of `x`). Use the `#[rust_to_ocaml(and)]` attribute to
    /// indicate when the `and` keyword should be used to continue a mutually
    /// recursive type declaration.
    ///
    ///     pub struct Foo(pub Bar, pub Bar);
    ///     #[rust_to_ocaml(and)]
    ///     pub struct Bar(pub Option<Foo>, pub Option<Foo>);
    ///
    /// is converted to:
    ///
    ///     type foo = bar * bar
    ///     and bar = foo option * foo option
    pub mutual_rec: bool,

    /// Normally, rust_to_ocaml will convert the names of fields and enum
    /// variants by attempting to convert idiomatic Rust casing to idiomatic
    /// OCaml casing. Use the `#[rust_to_ocaml(name = "my_name")]` attribute to
    /// override this behavior and provide some other name. This attribute takes
    /// precedence over the `prefix` attribute (no prefix will be applied to the
    /// given name). This attribute cannot be used to rename types (use
    /// rust_to_ocaml_config.toml instead).
    pub name: Option<String>,

    /// In OCaml, a variant declared as `Foo of (a * b)` is a variant with one
    /// field which is a pointer to a heap-allocated tuple. A variant declared
    /// as `Baz of a * b` is a variant with two fields of type `a` and `b`.
    ///
    /// By default, rust_to_ocaml will produce variants with a single field. But
    /// this behavior can be overridden with the `inline_tuple` attribute,
    /// converting the fields of a tuple (possibly behind a reference, `Box`, or
    /// any other wrapper type declared in the `types.transparent` section in
    /// rust_to_ocaml_config.toml) to fields of the OCaml variant.
    ///
    ///     pub enum E {
    ///         Foo((A, B)),
    ///         Bar(Box<(A, B)>),
    ///         #[rust_to_ocaml(inline_tuple)]
    ///         Baz((A, B)),
    ///         #[rust_to_ocaml(inline_tuple)]
    ///         Qux(Box<(A, B)>),
    ///     }
    ///
    /// is converted to:
    ///
    ///     type e =
    ///       | Foo of (a * b)
    ///       | Bar of (a * b)
    ///       | Baz of a * b
    ///       | Qux of a * b
    pub inline_tuple: bool,
}

impl Attrs {
    pub fn from_type(item: &syn::ItemType) -> Result<Self> {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }
    pub fn from_struct(item: &syn::ItemStruct) -> Result<Self> {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }
    pub fn from_enum(item: &syn::ItemEnum) -> Result<Self> {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }

    pub fn from_variant(variant: &syn::Variant) -> Result<Self> {
        Self::from_attributes(&variant.attrs, AttrKind::Variant)
    }
    pub fn from_field(field: &syn::Field) -> Result<Self> {
        Self::from_attributes(&field.attrs, AttrKind::Field)
    }

    fn from_attributes(attrs: &[syn::Attribute], kind: AttrKind) -> Result<Self> {
        let mut doc = vec![];
        let mut prefix = None;
        let mut ocaml_attrs = vec![];
        let mut mutual_rec = false;
        let mut name = None;
        let mut inline_tuple = false;

        for attr in attrs {
            if let syn::Meta::NameValue(meta) = &attr.meta
                && meta.path.is_ident(DOC)
                && let syn::Expr::Lit(value) = &meta.value
                && let syn::Lit::Str(lit) = &value.lit
            {
                doc.push(lit.value());
            } else if attr.path().is_ident(RUST_TO_OCAML) {
                attr.parse_nested_meta(|meta| {
                    if meta.path.is_ident(PREFIX) {
                        // Parse `#[rust_to_ocaml(prefix = "foo")]`
                        if kind == AttrKind::Field {
                            return Err(meta.error("not allowed on field; use `name` instead"));
                        }
                        let lit: LitStr = meta.value()?.parse()?;
                        prefix = Some(lit.value());
                        Ok(())
                    } else if meta.path.is_ident(ATTR) {
                        // Parse `#[rust_to_ocaml(attr = "deriving eq")]`
                        let lit: LitStr = meta.value()?.parse()?;
                        ocaml_attrs.push(lit.value());
                        Ok(())
                    } else if meta.path.is_ident(AND) {
                        // Parse `#[rust_to_ocaml(and)]`
                        if kind != AttrKind::Container {
                            return Err(meta.error("only allowed as a container attribute"));
                        }
                        mutual_rec = true;
                        Ok(())
                    } else if meta.path.is_ident(NAME) {
                        // Parse `#[rust_to_ocaml(name = "foo")]`
                        if kind == AttrKind::Container {
                            return Err(meta.error("not allowed as a container attribute"));
                        }
                        let lit: LitStr = meta.value()?.parse()?;
                        name = Some(lit.value());
                        Ok(())
                    } else if meta.path.is_ident(INLINE_TUPLE) {
                        // Parse `#[rust_to_ocaml(inline_tuple)]`
                        if kind != AttrKind::Variant {
                            return Err(meta.error("not allowed as a variant attribute"));
                        }
                        inline_tuple = true;
                        Ok(())
                    } else {
                        Err(meta.error("unrecognized rust_to_ocaml attribute"))
                    }
                })?;
            }
        }

        Ok(Self {
            doc,
            prefix,
            attrs: ocaml_attrs,
            mutual_rec,
            name,
            inline_tuple,
        })
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum AttrKind {
    Container,
    Variant,
    Field,
}

impl std::fmt::Display for AttrKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Container => write!(f, "container"),
            Self::Variant => write!(f, "variant"),
            Self::Field => write!(f, "field"),
        }
    }
}

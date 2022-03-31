// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    coeffects,
    context::Context,
    write::{
        self, angle, braces, concat, concat_by, concat_str, concat_str_by, fmt_separated,
        fmt_separated_with, newline, option, option_or, paren, quotes, square, triple_quotes,
        wrap_by, Error,
    },
};
use ffi::{Maybe, Maybe::*, Pair, Quadruple, Slice, Str, Triple};
use hackc_unit::HackCUnit;
use hash::HashSet;
use hhas_adata::{HhasAdata, DICT_PREFIX, KEYSET_PREFIX, VEC_PREFIX};
use hhas_attribute::{self, HhasAttribute};
use hhas_body::{HhasBody, HhasBodyEnv};
use hhas_class::{self, HhasClass};
use hhas_coeffects::{HhasCoeffects, HhasCtxConstant};
use hhas_constant::HhasConstant;
use hhas_function::HhasFunction;
use hhas_method::{HhasMethod, HhasMethodFlags};
use hhas_param::HhasParam;
use hhas_pos::{HhasPos, HhasSpan};
use hhas_property::HhasProperty;
use hhas_symbol_refs::{HhasSymbolRefs, IncludePath};
use hhas_type::HhasTypeInfo;
use hhas_type_const::HhasTypeConstant;
use hhas_typedef::HhasTypedef;
use hhbc_ast::*;
use hhbc_id::class::ClassType;
use hhbc_string_utils::float;
use hhvm_types_ffi::ffi::*;
use itertools::Itertools;
use label::Label;
use ocaml_helper::escaped_bytes;
use oxidized::ast_defs;
use runtime::TypedValue;
use std::{
    borrow::Cow,
    ffi::OsStr,
    io::{self, Result, Write},
    os::unix::ffi::OsStrExt,
    path::Path,
    write,
};
use write_bytes::write_bytes;

macro_rules! write_if {
    ($pred:expr, $($rest:tt)*) => {
        if ($pred) { write!($($rest)*) } else { Ok(()) }
    };
}

pub struct ExprEnv<'arena, 'e> {
    pub codegen_env: Option<&'e HhasBodyEnv<'arena>>,
}

fn print_unit(ctx: &Context<'_>, w: &mut dyn Write, prog: &HackCUnit<'_>) -> Result<()> {
    match ctx.path {
        Some(p) => {
            let abs = p.to_absolute();
            let p = escaper::escape(
                abs.to_str()
                    .ok_or_else(|| <io::Error as From<Error>>::from(Error::InvalidUTF8))?,
            );

            concat_str_by(w, " ", ["#", p.as_ref(), "starts here"])?;

            newline(w)?;

            newline(w)?;
            concat_str(w, [".filepath ", &format!("\"{}\"", p), ";"])?;

            newline(w)?;
            handle_not_impl(|| print_unit_(ctx, w, prog))?;

            newline(w)?;
            concat_str_by(w, " ", ["#", p.as_ref(), "ends here"])?;

            newline(w)
        }
        None => {
            w.write_all(b"#starts here")?;

            newline(w)?;
            handle_not_impl(|| print_unit_(ctx, w, prog))?;

            newline(w)?;
            w.write_all(b"#ends here")?;

            newline(w)
        }
    }
}

fn get_fatal_op(f: &FatalOp) -> &str {
    match *f {
        FatalOp::Parse => "Parse",
        FatalOp::Runtime => "Runtime",
        FatalOp::RuntimeOmitFrame => "RuntimeOmitFrame",
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_unit_(ctx: &Context<'_>, w: &mut dyn Write, prog: &HackCUnit<'_>) -> Result<()> {
    if let Just(Triple(fop, p, msg)) = &prog.fatal {
        newline(w)?;
        let HhasPos {
            line_begin,
            line_end,
            col_begin,
            col_end,
        } = p;
        write_bytes!(
            w,
            ".fatal {}:{},{}:{} {} \"{}\";",
            line_begin,
            col_begin,
            line_end,
            col_end,
            get_fatal_op(fop),
            escaper::escape_bstr(msg.as_bstr()),
        )?;
    }

    newline(w)?;
    print_module(w, &prog.module)?;
    concat(w, &prog.adata, |w, a| print_adata_region(ctx, w, a))?;
    concat(w, &prog.functions, |w, f| print_fun_def(ctx, w, f))?;
    concat(w, &prog.classes, |w, cd| print_class_def(ctx, w, cd))?;
    concat(w, &prog.constants, |w, c| print_constant(ctx, w, c))?;
    concat(w, &prog.typedefs, |w, td| print_typedef(ctx, w, td))?;
    print_file_attributes(ctx, w, prog.file_attributes.as_ref())?;

    if ctx.dump_symbol_refs() {
        print_include_region(ctx, w, &prog.symbol_refs.includes)?;
        print_symbol_ref_regions(ctx, w, &prog.symbol_refs)?;
    }
    Ok(())
}

fn print_include_region(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    includes: &Slice<'_, IncludePath<'_>>,
) -> Result<()> {
    fn print_path(w: &mut dyn Write, p: &Path) -> Result<()> {
        option(w, p.to_str(), |w, p: &str| write!(w, "\n  {}", p))
    }
    fn print_if_exists(w: &mut dyn Write, p: &Path) -> Result<()> {
        if p.exists() { print_path(w, p) } else { Ok(()) }
    }
    fn print_include(ctx: &Context<'_>, w: &mut dyn Write, inc: IncludePath<'_>) -> Result<()> {
        let include_roots = ctx.include_roots;
        let alloc = bumpalo::Bump::new();
        match inc.into_doc_root_relative(&alloc, include_roots) {
            IncludePath::Absolute(p) => print_if_exists(w, Path::new(OsStr::from_bytes(&p))),
            IncludePath::SearchPathRelative(p) => {
                let path_from_cur_dirname = ctx
                    .path
                    .and_then(|p| p.path().parent())
                    .unwrap_or_else(|| Path::new(""))
                    .join(OsStr::from_bytes(&p));
                if path_from_cur_dirname.exists() {
                    print_path(w, &path_from_cur_dirname)
                } else {
                    let search_paths = ctx.include_search_paths;
                    for prefix in search_paths.iter() {
                        let path = Path::new(OsStr::from_bytes(prefix)).join(OsStr::from_bytes(&p));
                        if path.exists() {
                            return print_path(w, &path);
                        }
                    }
                    Ok(())
                }
            }
            IncludePath::IncludeRootRelative(v, p) => {
                if !p.is_empty() {
                    include_roots.get(v.as_bstr()).iter().try_for_each(|ir| {
                        let doc_root = ctx.doc_root;
                        let resolved = Path::new(OsStr::from_bytes(doc_root))
                            .join(OsStr::from_bytes(ir))
                            .join(OsStr::from_bytes(&p));
                        print_if_exists(w, &resolved)
                    })?
                }
                Ok(())
            }
            IncludePath::DocRootRelative(p) => {
                let doc_root = ctx.doc_root;
                let resolved = Path::new(OsStr::from_bytes(doc_root)).join(OsStr::from_bytes(&p));
                print_if_exists(w, &resolved)
            }
        }
    }
    if !includes.is_empty() {
        w.write_all(b"\n.includes {")?;
        for inc in includes.as_ref().iter() {
            // TODO(hrust): avoid clone. Rethink onwership of inc in
            // hhas_symbol_refs_rust::IncludePath::into_doc_root_relative
            print_include(ctx, w, inc.clone())?;
        }
        w.write_all(b"\n}\n")?;
    }
    Ok(())
}

fn print_symbol_ref_regions<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    symbol_refs: &HhasSymbolRefs<'arena>,
) -> Result<()> {
    let mut print_region = |name, refs: &Slice<'arena, Str<'arena>>| {
        if !refs.is_empty() {
            ctx.newline(w)?;
            write!(w, ".{} {{", name)?;
            ctx.block(w, |c, w| {
                for s in refs.as_ref().iter() {
                    c.newline(w)?;
                    w.write_all(s)?;
                }
                Ok(())
            })?;
            w.write_all(b"\n}\n")
        } else {
            Ok(())
        }
    };
    print_region("constant_refs", &symbol_refs.constants)?;
    print_region("function_refs", &symbol_refs.functions)?;
    print_region("class_refs", &symbol_refs.classes)
}

fn print_adata_region(ctx: &Context<'_>, w: &mut dyn Write, adata: &HhasAdata<'_>) -> Result<()> {
    write_bytes!(w, ".adata {} = ", adata.id)?;
    triple_quotes(w, |w| print_adata(ctx, w, &adata.value))?;
    w.write_all(b";")?;
    ctx.newline(w)
}

fn print_typedef(ctx: &Context<'_>, w: &mut dyn Write, td: &HhasTypedef<'_>) -> Result<()> {
    newline(w)?;
    w.write_all(b".alias ")?;
    print_special_and_user_attrs(
        ctx,
        w,
        td.attributes.as_ref(),
        &AttrContext::Alias,
        &td.attrs,
    )?;
    w.write_all(td.name.as_bstr())?;
    w.write_all(b" = ")?;
    print_typedef_info(w, &td.type_info)?;
    w.write_all(b" ")?;
    print_span(w, &td.span)?;
    w.write_all(b" ")?;
    triple_quotes(w, |w| print_adata(ctx, w, &td.type_structure))?;
    w.write_all(b";")
}

fn handle_not_impl<F>(f: F) -> Result<()>
where
    F: FnOnce() -> Result<()>,
{
    f().or_else(|e| {
        match write::get_embedded_error(&e) {
            Some(Error::NotImpl(msg)) => {
                println!("#### NotImpl: {}", msg);
                eprintln!("NotImpl: {}", msg);
                return Ok(());
            }
            _ => {}
        }
        Err(e)
    })
}

fn print_fun_def(ctx: &Context<'_>, w: &mut dyn Write, fun_def: &HhasFunction<'_>) -> Result<()> {
    let body = &fun_def.body;
    newline(w)?;
    w.write_all(b".function ")?;
    print_upper_bounds_(w, &body.upper_bounds)?;
    w.write_all(b" ")?;
    print_special_and_user_attrs(
        ctx,
        w,
        fun_def.attributes.as_ref(),
        &AttrContext::Func,
        &fun_def.attrs,
    )?;
    print_span(w, &fun_def.span)?;
    w.write_all(b" ")?;
    option(w, body.return_type_info.as_ref(), |w, ti| {
        print_type_info(w, ti)?;
        w.write_all(b" ")
    })?;
    w.write_all(fun_def.name.as_bstr())?;
    let params = fun_def.params();
    let dv_labels = find_dv_labels(params);
    print_params(ctx, w, fun_def.params(), &dv_labels)?;
    if fun_def.is_generator() {
        w.write_all(b" isGenerator")?;
    }
    if fun_def.is_async() {
        w.write_all(b" isAsync")?;
    }
    if fun_def.is_pair_generator() {
        w.write_all(b" isPairGenerator")?;
    }
    w.write_all(b" ")?;
    braces(w, |w| {
        ctx.block(w, |c, w| {
            print_body(c, w, body, &fun_def.coeffects, &dv_labels)
        })?;
        newline(w)
    })?;

    newline(w)
}

fn print_requirement(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    r: &Pair<ClassType<'_>, hhas_class::TraitReqKind>,
) -> Result<()> {
    ctx.newline(w)?;
    w.write_all(b".require ")?;
    match r {
        Pair(name, hhas_class::TraitReqKind::MustExtend) => {
            write_bytes!(w, "extends <{}>;", name)
        }
        Pair(name, hhas_class::TraitReqKind::MustImplement) => {
            write_bytes!(w, "implements <{}>;", name)
        }
    }
}

fn print_type_constant(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    c: &HhasTypeConstant<'_>,
) -> Result<()> {
    ctx.newline(w)?;
    write_bytes!(w, ".const {} isType", c.name)?;
    if c.is_abstract {
        w.write_all(b" isAbstract")?;
    }
    option(w, c.initializer.as_ref(), |w, init| {
        w.write_all(b" = ")?;
        triple_quotes(w, |w| print_adata(ctx, w, init))
    })?;
    w.write_all(b";")
}

fn print_ctx_constant(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasCtxConstant<'_>) -> Result<()> {
    ctx.newline(w)?;
    write_bytes!(w, ".ctx {}", c.name)?;
    if c.is_abstract {
        w.write_all(b" isAbstract")?;
    }
    let recognized = c.recognized.as_ref();
    if !recognized.is_empty() {
        write_bytes!(w, " {}", fmt_separated(" ", recognized))?;
    }
    let unrecognized = c.unrecognized.as_ref();
    if !unrecognized.is_empty() {
        write_bytes!(w, " {}", fmt_separated(" ", unrecognized))?;
    }
    w.write_all(b";")?;
    Ok(())
}

fn print_property_doc_comment(w: &mut dyn Write, p: &HhasProperty<'_>) -> Result<()> {
    if let Just(s) = p.doc_comment.as_ref() {
        write_bytes!(w, r#""""{}""""#, escaper::escape_bstr(s.as_bstr()))?;
        w.write_all(b" ")?;
    }
    Ok(())
}

fn print_property_type_info(w: &mut dyn Write, p: &HhasProperty<'_>) -> Result<()> {
    print_type_info(w, &p.type_info)?;
    w.write_all(b" ")
}

fn print_property(ctx: &Context<'_>, w: &mut dyn Write, property: &HhasProperty<'_>) -> Result<()> {
    newline(w)?;
    w.write_all(b"  .property ")?;
    print_special_and_user_attrs(
        ctx,
        w,
        property.attributes.as_ref(),
        &AttrContext::Prop,
        &property.flags,
    )?;
    print_property_doc_comment(w, property)?;
    print_property_type_info(w, property)?;
    w.write_all(property.name.as_bstr())?;
    w.write_all(b" =\n    ")?;
    let initial_value = property.initial_value.as_ref();
    if initial_value == Just(&TypedValue::Uninit) {
        w.write_all(b"uninit;")
    } else {
        triple_quotes(w, |w| match initial_value {
            Nothing => w.write_all(b"N;"),
            Just(value) => print_adata(ctx, w, value),
        })?;
        w.write_all(b";")
    }
}

fn print_constant(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasConstant<'_>) -> Result<()> {
    ctx.newline(w)?;
    w.write_all(b".const ")?;
    w.write_all(c.name.as_bstr())?;
    if c.is_abstract {
        w.write_all(b" isAbstract")?;
    }
    match c.value.as_ref() {
        Just(TypedValue::Uninit) => w.write_all(b" = uninit")?,
        Just(value) => {
            w.write_all(b" = ")?;
            triple_quotes(w, |w| print_adata(ctx, w, value))?;
        }
        Nothing => {}
    }
    w.write_all(b";")
}

fn print_enum_ty(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasClass<'_>) -> Result<()> {
    if let Just(et) = c.enum_type.as_ref() {
        ctx.newline(w)?;
        w.write_all(b".enum_ty ")?;
        print_type_info_(w, true, et)?;
        w.write_all(b";")?;
    }
    Ok(())
}

fn print_doc_comment<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    doc_comment: Maybe<&Str<'arena>>,
) -> Result<()> {
    if let Just(cmt) = doc_comment {
        ctx.newline(w)?;
        write_bytes!(w, r#".doc """{}""";"#, escaper::escape_bstr(cmt.as_bstr()))?;
    }
    Ok(())
}

fn print_use_precedence<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    Triple(id1, id2, ids): &Triple<
        ClassType<'arena>,
        ClassType<'arena>,
        Slice<'arena, ClassType<'arena>>,
    >,
) -> Result<()> {
    ctx.newline(w)?;
    write_bytes!(
        w,
        "{}::{} insteadof {};",
        id1,
        id2,
        fmt_separated(" ", ids.as_ref().iter().unique())
    )
}

fn print_use_alias<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    Quadruple(ido1, id, ido2, attr): &Quadruple<
        Maybe<ClassType<'arena>>,
        ClassType<'arena>,
        Maybe<ClassType<'arena>>,
        Attr,
    >,
) -> Result<()> {
    ctx.newline(w)?;
    let id = id.unsafe_as_str();
    option_or(
        w,
        ido1.as_ref(),
        |w, i: &ClassType<'arena>| write_bytes!(w, "{}::{}", i, id),
        id,
    )?;
    w.write_all(b" as ")?;
    if !attr.is_empty() {
        square(w, |w| {
            write!(
                w,
                "{}",
                attrs_to_string_ffi(AttrContext::TraitImport, *attr)
            )
        })?;
    }
    write_if!(!attr.is_empty() && ido2.is_just(), w, " ")?;
    option(w, ido2.as_ref(), |w, i: &ClassType<'arena>| {
        w.write_all(i.as_bstr())
    })?;
    w.write_all(b";")
}

fn print_uses<'arena>(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasClass<'arena>) -> Result<()> {
    if c.uses.is_empty() {
        Ok(())
    } else {
        newline(w)?;
        write_bytes!(w, "  .use {}", fmt_separated(" ", c.uses.iter()))?;

        if c.use_aliases.is_empty() && c.use_precedences.is_empty() {
            w.write_all(b";")
        } else {
            w.write_all(b" {")?;
            ctx.block(w, |ctx, w| {
                let precs: &[Triple<
                    ClassType<'arena>,
                    ClassType<'arena>,
                    Slice<'arena, ClassType<'arena>>,
                >] = c.use_precedences.as_ref();
                for x in precs {
                    print_use_precedence(ctx, w, x)?;
                }
                let aliases: &[Quadruple<
                    Maybe<ClassType<'arena>>,
                    ClassType<'arena>,
                    Maybe<ClassType<'arena>>,
                    Attr,
                >] = c.use_aliases.as_ref();
                for x in aliases {
                    print_use_alias(ctx, w, x)?;
                }
                Ok(())
            })?;
            newline(w)?;
            w.write_all(b"  }")
        }
    }
}

fn print_implements(w: &mut dyn Write, implements: &[ClassType<'_>]) -> Result<()> {
    if implements.is_empty() {
        return Ok(());
    }
    write_bytes!(w, " implements ({})", fmt_separated(" ", implements))
}

fn print_enum_includes(w: &mut dyn Write, enum_includes: &[ClassType<'_>]) -> Result<()> {
    if enum_includes.is_empty() {
        return Ok(());
    }
    write_bytes!(w, " enum_includes ({})", fmt_separated(" ", enum_includes))
}

fn print_shadowed_tparams<'arena>(
    w: &mut dyn Write,
    shadowed_tparams: impl AsRef<[Str<'arena>]>,
) -> Result<()> {
    write_bytes!(w, "{{{}}}", fmt_separated(", ", shadowed_tparams.as_ref()))
}

fn print_method_def(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    method_def: &HhasMethod<'_>,
) -> Result<()> {
    let body = &method_def.body;
    newline(w)?;
    w.write_all(b"  .method ")?;
    print_shadowed_tparams(w, &body.shadowed_tparams)?;
    print_upper_bounds_(w, &body.upper_bounds)?;
    w.write_all(b" ")?;
    print_special_and_user_attrs(
        ctx,
        w,
        method_def.attributes.as_ref(),
        &AttrContext::Func,
        &method_def.attrs,
    )?;
    print_span(w, &method_def.span)?;
    w.write_all(b" ")?;
    option(w, body.return_type_info.as_ref(), |w, t| {
        print_type_info(w, t)?;
        w.write_all(b" ")
    })?;
    w.write_all(method_def.name.as_bstr())?;
    let dv_labels = find_dv_labels(&body.params);
    print_params(ctx, w, &body.params, &dv_labels)?;
    if method_def.flags.contains(HhasMethodFlags::IS_GENERATOR) {
        w.write_all(b" isGenerator")?;
    }
    if method_def.flags.contains(HhasMethodFlags::IS_ASYNC) {
        w.write_all(b" isAsync")?;
    }
    if method_def
        .flags
        .contains(HhasMethodFlags::IS_PAIR_GENERATOR)
    {
        w.write_all(b" isPairGenerator")?;
    }
    if method_def.flags.contains(HhasMethodFlags::IS_CLOSURE_BODY) {
        w.write_all(b" isClosureBody")?;
    }
    w.write_all(b" ")?;
    braces(w, |w| {
        ctx.block(w, |c, w| {
            print_body(c, w, body, &method_def.coeffects, &dv_labels)
        })?;
        newline(w)?;
        w.write_all(b"  ")
    })
}

fn print_class_def<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    class_def: &HhasClass<'arena>,
) -> Result<()> {
    newline(w)?;
    w.write_all(b".class ")?;
    print_upper_bounds(w, class_def.upper_bounds.as_ref())?;
    w.write_all(b" ")?;
    print_special_and_user_attrs(
        ctx,
        w,
        class_def.attributes.as_ref(),
        &AttrContext::Class,
        &class_def.flags,
    )?;
    w.write_all(class_def.name.as_bstr())?;
    w.write_all(b" ")?;
    print_span(w, &class_def.span)?;
    print_extends(
        w,
        class_def
            .base
            .as_ref()
            .map(|x: &ClassType<'arena>| x.unsafe_as_str())
            .into(),
    )?;
    print_implements(w, class_def.implements.as_ref())?;
    print_enum_includes(w, class_def.enum_includes.as_ref())?;
    w.write_all(b" {")?;
    ctx.block(w, |c, w| {
        print_doc_comment(c, w, class_def.doc_comment.as_ref())?;
        print_uses(c, w, class_def)?;
        print_enum_ty(c, w, class_def)?;
        for x in class_def.requirements.as_ref() {
            print_requirement(c, w, x)?;
        }
        for x in class_def.constants.as_ref() {
            print_constant(c, w, x)?;
        }
        for x in class_def.type_constants.as_ref() {
            print_type_constant(c, w, x)?;
        }
        for x in class_def.ctx_constants.as_ref() {
            print_ctx_constant(c, w, x)?;
        }
        for x in class_def.properties.as_ref() {
            print_property(c, w, x)?;
        }
        for m in class_def.methods.as_ref() {
            print_method_def(c, w, m)?;
        }
        Ok(())
    })?;
    newline(w)?;
    w.write_all(b"}")?;
    newline(w)
}

fn print_pos_as_prov_tag(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    loc: Option<&ast_defs::Pos>,
) -> Result<()> {
    match loc {
        Some(l) if ctx.array_provenance => {
            let (line, ..) = l.info_pos();
            let filename = l.filename().to_absolute();
            let filename = match filename.to_str().unwrap() {
                "" => "(unknown hackc filename)",
                x => x,
            };
            write!(
                w,
                r#"p:i:{};s:{}:\"{}\";"#,
                line,
                filename.len(),
                escaper::escape(filename)
            )
        }
        _ => Ok(()),
    }
}

fn print_adata_mapped_argument<F, V>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    col_type: &str,
    loc: Option<&ast_defs::Pos>,
    values: &[V],
    f: F,
) -> Result<()>
where
    F: Fn(&Context<'_>, &mut dyn Write, &V) -> Result<()>,
{
    write!(w, "{}:{}:{{", col_type, values.len(),)?;
    print_pos_as_prov_tag(ctx, w, loc)?;
    for v in values {
        f(ctx, w, v)?
    }
    write!(w, "}}")
}

fn print_adata_collection_argument(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    col_type: &str,
    loc: Option<&ast_defs::Pos>,
    values: &[TypedValue<'_>],
) -> Result<()> {
    print_adata_mapped_argument(ctx, w, col_type, loc, values, &print_adata)
}

fn print_adata_dict_collection_argument(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    col_type: &str,
    loc: Option<&ast_defs::Pos>,
    pairs: &[Pair<TypedValue<'_>, TypedValue<'_>>],
) -> Result<()> {
    print_adata_mapped_argument(ctx, w, col_type, loc, pairs, |ctx, w, Pair(v1, v2)| {
        print_adata(ctx, w, v1)?;
        print_adata(ctx, w, v2)
    })
}

fn print_adata(ctx: &Context<'_>, w: &mut dyn Write, tv: &TypedValue<'_>) -> Result<()> {
    match tv {
        TypedValue::Uninit => w.write_all(b"uninit"),
        TypedValue::Null => w.write_all(b"N;"),
        TypedValue::String(s) => {
            write_bytes!(
                w,
                r#"s:{}:\"{}\";"#,
                s.len(),
                escaper::escape_bstr(s.as_bstr())
            )
        }
        TypedValue::LazyClass(s) => {
            write_bytes!(
                w,
                r#"l:{}:\"{}\";"#,
                s.len(),
                escaper::escape_bstr(s.as_bstr())
            )
        }
        TypedValue::Double(f) => {
            write!(w, "d:{};", float::to_string(f.to_f64()))
        }
        TypedValue::Int(i) => write!(w, "i:{};", i),
        // TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why?
        TypedValue::Bool(false) => w.write_all(b"b:0;"),
        TypedValue::Bool(true) => w.write_all(b"b:1;"),
        TypedValue::Vec(values) => {
            print_adata_collection_argument(ctx, w, VEC_PREFIX, None, values.as_ref())
        }
        TypedValue::Dict(pairs) => {
            print_adata_dict_collection_argument(ctx, w, DICT_PREFIX, None, pairs.as_ref())
        }
        TypedValue::Keyset(values) => {
            print_adata_collection_argument(ctx, w, KEYSET_PREFIX, None, values.as_ref())
        }
        TypedValue::HhasAdata(s) => w.write_all(&escaped_bytes(s.as_ref())),
    }
}

fn print_attribute(ctx: &Context<'_>, w: &mut dyn Write, a: &HhasAttribute<'_>) -> Result<()> {
    let unescaped = a.name.as_bstr();
    let escaped = if a.name.starts_with(b"__") {
        Cow::Borrowed(unescaped)
    } else {
        escaper::escape_bstr(unescaped)
    };
    write_bytes!(
        w,
        "\"{}\"(\"\"\"{}:{}:{{",
        escaped,
        VEC_PREFIX,
        a.arguments.len()
    )?;
    concat(w, &a.arguments, |w, arg| print_adata(ctx, w, arg))?;
    w.write_all(b"}\"\"\")")
}

fn print_attributes<'a>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    al: &[HhasAttribute<'a>],
) -> Result<()> {
    // Adjust for underscore coming before alphabet
    let al = al
        .iter()
        .sorted_by_key(|a| (!a.name.starts_with(b"__"), a.name));
    write_bytes!(
        w,
        "{}",
        fmt_separated_with(" ", al, |w, a| print_attribute(ctx, w, a))
    )
}

fn print_file_attributes(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    al: &[HhasAttribute<'_>],
) -> Result<()> {
    if al.is_empty() {
        return Ok(());
    }
    newline(w)?;
    w.write_all(b".file_attributes [")?;
    print_attributes(ctx, w, al)?;
    w.write_all(b"] ;")?;
    newline(w)
}

fn print_module(w: &mut dyn Write, m_opt: &Maybe<Str<'_>>) -> Result<()> {
    if let Just(m) = m_opt {
        newline(w)?;
        write_bytes!(w, ".module \"{}\";", m)?;
        newline(w)?;
    }
    Ok(())
}

fn is_bareword_char(c: &u8) -> bool {
    match *c {
        b'_' | b'.' | b'$' | b'\\' => true,
        c => (b'0'..=b'9').contains(&c) || (b'a'..=b'z').contains(&c) || (b'A'..=b'Z').contains(&c),
    }
}

fn print_body(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    body: &HhasBody<'_>,
    coeffects: &HhasCoeffects<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    print_doc_comment(ctx, w, body.doc_comment.as_ref())?;
    if body.is_memoize_wrapper {
        ctx.newline(w)?;
        w.write_all(b".ismemoizewrapper;")?;
    }
    if body.is_memoize_wrapper_lsb {
        ctx.newline(w)?;
        w.write_all(b".ismemoizewrapperlsb;")?;
    }
    if body.num_iters > 0 {
        ctx.newline(w)?;
        write!(w, ".numiters {};", body.num_iters)?;
    }
    if !body.decl_vars.is_empty() {
        ctx.newline(w)?;
        w.write_all(b".declvars ")?;
        concat_by(w, " ", &body.decl_vars, |w, var| {
            if var.iter().all(is_bareword_char) {
                w.write_all(var)
            } else {
                quotes(w, |w| w.write_all(&escaper::escape_bstr(var.as_bstr())))
            }
        })?;
        w.write_all(b";")?;
    }
    coeffects::coeffects_to_hhas(ctx, w, coeffects)?;
    let local_names: Vec<Str<'_>> = body
        .params
        .iter()
        .map(|param| param.name)
        .chain(body.decl_vars.iter().copied())
        .collect();
    print_instructions(ctx, w, &body.body_instrs, dv_labels, &local_names)
}

fn print_instructions<'a, 'b>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    instrs: &'b [Instruct<'a>],
    dv_labels: &'b HashSet<Label>,
    local_names: &'b [Str<'a>],
) -> Result<()> {
    let mut ctx = ctx.clone();
    for instr in instrs {
        match instr {
            Instruct::Pseudo(Pseudo::Continue(_) | Pseudo::Break(_)) => {
                return Err(Error::fail("Cannot break/continue 1 level").into());
            }
            Instruct::Pseudo(Pseudo::Comment(_)) => {
                // indentation = 0
                newline(w)?;
                print_instr(w, instr, dv_labels, local_names)?;
            }
            Instruct::Pseudo(Pseudo::Label(_)) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr, dv_labels, local_names)
            })?,
            Instruct::Pseudo(Pseudo::TryCatchBegin) => {
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels, local_names)?;
                ctx.indent_inc();
            }
            Instruct::Pseudo(Pseudo::TryCatchMiddle) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr, dv_labels, local_names)
            })?,
            Instruct::Pseudo(Pseudo::TryCatchEnd) => {
                ctx.indent_dec();
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels, local_names)?;
            }
            _ => {
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels, local_names)?;
            }
        }
    }
    Ok(())
}

pub(crate) fn print_fcall_args(
    w: &mut dyn Write,
    args @ FCallArgs {
        flags,
        num_args,
        num_rets,
        inouts,
        readonly,
        async_eager_target,
        context,
    }: &FCallArgs<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    angle(w, |w| {
        let flags = hhvm_hhbc_defs_ffi::ffi::fcall_flags_to_string_ffi(*flags);
        write!(w, "{}", flags)
    })?;
    w.write_all(b" ")?;
    print_int(w, num_args)?;
    w.write_all(b" ")?;
    print_int(w, num_rets)?;
    w.write_all(b" ")?;
    quotes(w, |w| {
        concat_by(w, "", inouts, |w, i| {
            w.write_all(if *i { b"1" } else { b"0" })
        })
    })?;
    w.write_all(b" ")?;
    quotes(w, |w| {
        concat_by(w, "", readonly, |w, i| {
            w.write_all(if *i { b"1" } else { b"0" })
        })
    })?;
    w.write_all(b" ")?;
    if args.has_async_eager_target() {
        print_label(w, async_eager_target, dv_labels)?;
    } else {
        w.write_all(b"-")?;
    }
    w.write_all(b" ")?;
    quotes(w, |w| w.write_all(context))
}

fn print_pseudo(w: &mut dyn Write, instr: &Pseudo<'_>, dv_labels: &HashSet<Label>) -> Result<()> {
    match instr {
        Pseudo::TypedValue(_) => Err(Error::fail("print_lit_const: TypedValue").into()),
        Pseudo::Label(l) => {
            print_label(w, l, dv_labels)?;
            w.write_all(b":")
        }
        Pseudo::TryCatchBegin => w.write_all(b".try {"),
        Pseudo::TryCatchMiddle => w.write_all(b"} .catch {"),
        Pseudo::TryCatchEnd => w.write_all(b"}"),
        Pseudo::Comment(s) => write_bytes!(w, "# {}", s),
        Pseudo::SrcLoc(p) => write!(
            w,
            ".srcloc {}:{},{}:{};",
            p.line_begin, p.col_begin, p.line_end, p.col_end
        ),
        Pseudo::Break(_) | Pseudo::Continue(_) => Err(Error::fail("invalid instruction").into()),
    }
}

fn print_instr<'a, 'b>(
    w: &mut dyn Write,
    instr: &'b Instruct<'a>,
    dv_labels: &'b HashSet<Label>,
    local_names: &'b [Str<'a>],
) -> Result<()> {
    match instr {
        Instruct::Opcode(opcode) => {
            crate::print_opcode::PrintOpcode::new(opcode, dv_labels, local_names).print_opcode(w)
        }
        Instruct::Pseudo(pseudo) => print_pseudo(w, pseudo, dv_labels),
    }
}

/// Build a set containing the labels for param default-value initializers
/// so they can be formatted as `DV123` instead of `L123`.
fn find_dv_labels(params: &[HhasParam<'_>]) -> HashSet<Label> {
    params
        .iter()
        .filter_map(|param| match &param.default_value {
            Just(Pair(label, _)) => Some(*label),
            _ => None,
        })
        .collect()
}

fn print_params<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    params: &[HhasParam<'arena>],
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    paren(w, |w| {
        concat_by(w, ", ", params, |w, i| print_param(ctx, w, i, dv_labels))
    })
}

fn print_param<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    param: &HhasParam<'arena>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    print_param_user_attributes(ctx, w, param)?;
    write_if!(param.is_inout, w, "inout ")?;
    write_if!(param.is_readonly, w, "readonly ")?;
    write_if!(param.is_variadic, w, "...")?;
    option(w, param.type_info.as_ref(), |w, ty| {
        print_type_info(w, ty)?;
        w.write_all(b" ")
    })?;
    w.write_all(&param.name)?;
    option(
        w,
        param
            .default_value
            .map(|Pair(label, php_code)| (label, php_code))
            .as_ref(),
        |w, &(label, php_code)| print_param_default_value(w, label, php_code, dv_labels),
    )
}

fn print_param_default_value<'arena>(
    w: &mut dyn Write,
    label: Label,
    php_code: Str<'arena>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    w.write_all(b" = ")?;
    print_label(w, &label, dv_labels)?;
    paren(w, |w| triple_quotes(w, |w| w.write_all(&php_code)))
}

pub(crate) fn print_label(
    w: &mut dyn Write,
    label: &Label,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    let prefix = if dv_labels.contains(label) { "DV" } else { "L" };
    write!(w, "{}{}", prefix, label)
}

pub(crate) fn print_int<T: std::fmt::Display>(w: &mut dyn Write, i: T) -> Result<()> {
    write!(w, "{}", i)
}

fn print_param_user_attributes(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    param: &HhasParam<'_>,
) -> Result<()> {
    match param.user_attributes.as_ref()[..] {
        [] => Ok(()),
        _ => square(w, |w| print_attributes(ctx, w, &param.user_attributes)),
    }
}

fn print_span(
    w: &mut dyn Write,
    &HhasSpan {
        line_begin,
        line_end,
    }: &HhasSpan,
) -> Result<()> {
    write!(w, "({},{})", line_begin, line_end)
}

fn print_special_and_user_attrs(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    users: &[HhasAttribute<'_>],
    attr_ctx: &AttrContext,
    attrs: &Attr,
) -> Result<()> {
    if !users.is_empty() || !attrs.is_empty() {
        square(w, |w| {
            write!(w, "{}", attrs_to_string_ffi(*attr_ctx, *attrs))?;
            if !users.is_empty() {
                w.write_all(b" ")?;
            }
            print_attributes(ctx, w, users)
        })?;
        w.write_all(b" ")?;
    }
    Ok(())
}

fn print_upper_bounds<'arena>(
    w: &mut dyn Write,
    ubs: impl AsRef<[Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>]>,
) -> Result<()> {
    braces(w, |w| concat_by(w, ", ", ubs, print_upper_bound))
}

fn print_upper_bound<'arena>(
    w: &mut dyn Write,
    Pair(id, tys): &Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'_>>>,
) -> Result<()> {
    paren(w, |w| {
        write_bytes!(w, "{} as ", id)?;
        concat_by(w, ", ", &tys, print_type_info)
    })
}

fn print_upper_bounds_<'arena>(
    w: &mut dyn Write,
    ubs: impl AsRef<[Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>]>,
) -> Result<()> {
    braces(w, |w| concat_by(w, ", ", ubs, print_upper_bound_))
}

fn print_upper_bound_<'arena>(
    w: &mut dyn Write,
    Pair(id, tys): &Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>,
) -> Result<()> {
    paren(w, |w| {
        write_bytes!(w, "{} as ", id)?;
        concat_by(w, ", ", &tys, print_type_info)
    })
}

fn print_type_info(w: &mut dyn Write, ti: &HhasTypeInfo<'_>) -> Result<()> {
    print_type_info_(w, false, ti)
}

fn print_type_flags(w: &mut dyn Write, flag: TypeConstraintFlags) -> Result<()> {
    write!(w, "{}", type_flags_to_string_ffi(flag))
}

fn print_type_info_(w: &mut dyn Write, is_enum: bool, ti: &HhasTypeInfo<'_>) -> Result<()> {
    let print_quote_str = |w: &mut dyn Write, opt: Option<&str>| {
        option_or(
            w,
            opt,
            |w, s: &str| quotes(w, |w| w.write_all(escaper::escape(s).as_bytes())),
            "N",
        )
    };
    angle(w, |w| {
        print_quote_str(w, ti.user_type.map(|n| n.unsafe_as_str()).into())?;
        w.write_all(b" ")?;
        if !is_enum {
            print_quote_str(w, ti.type_constraint.name.map(|n| n.unsafe_as_str()).into())?;
            w.write_all(b" ")?;
        }
        print_type_flags(w, ti.type_constraint.flags)
    })
}

fn print_typedef_info(w: &mut dyn Write, ti: &HhasTypeInfo<'_>) -> Result<()> {
    angle(w, |w| {
        write_bytes!(
            w,
            r#""{}""#,
            escaper::escape_bstr(
                ti.type_constraint
                    .name
                    .as_ref()
                    .unwrap_or(&Default::default())
                    .as_bstr()
            )
        )?;
        let flags = ti.type_constraint.flags & TypeConstraintFlags::Nullable;
        if !flags.is_empty() {
            wrap_by(w, " ", |w| {
                print_type_flags(w, ti.type_constraint.flags & TypeConstraintFlags::Nullable)
            })?;
        }
        Ok(())
    })
}

fn print_extends(w: &mut dyn Write, base: Option<&str>) -> Result<()> {
    match base {
        None => Ok(()),
        Some(b) => concat_str_by(w, " ", [" extends", b]),
    }
}

pub fn external_print_unit(
    ctx: &Context<'_>,
    w: &mut dyn std::io::Write,
    prog: &HackCUnit<'_>,
) -> std::result::Result<(), Error> {
    print_unit(ctx, w, prog).map_err(write::into_error)?;
    w.flush().map_err(write::into_error)?;
    Ok(())
}

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
use hhvm_hhbc_defs_ffi::ffi::{
    fcall_flags_to_string_ffi, BareThisOp, CollectionType, ContCheckOp, FatalOp, IncDecOp,
    InitPropOp, IsLogAsDynamicCallOp, IsTypeOp, MOpMode, ObjMethodOp, QueryMOp, ReadonlyOp,
    SetOpOp, SetRangeOp, SilenceOp, SpecialClsRef, SwitchKind, TypeStructResolveOp,
};
use hhvm_types_ffi::ffi::*;
use iterator::IterId;
use itertools::Itertools;
use label::Label;
use local::Local;
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

fn print_function_id(w: &mut dyn Write, id: &FunctionId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_class_id(w: &mut dyn Write, id: &ClassId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_method_id(w: &mut dyn Write, id: &MethodId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_const_id(w: &mut dyn Write, id: &ConstId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_prop_id(w: &mut dyn Write, id: &PropId<'_>) -> Result<()> {
    write_bytes!(w, r#""{}""#, escaper::escape_bstr(id.as_bstr()))
}

fn print_adata_id(w: &mut dyn Write, id: &AdataId<'_>) -> Result<()> {
    write_bytes!(w, "@{}", id)
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
        TypedValue::Float(f) => write!(w, "d:{};", float::to_string(*f)),
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
    print_instructions(ctx, w, &body.body_instrs, dv_labels)
}

fn print_instructions(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    instrs: &[Instruct<'_>],
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    let mut ctx = ctx.clone();
    for instr in instrs {
        match instr {
            Instruct::SpecialFlow(_) => {
                return Err(Error::fail("Cannot break/continue 1 level").into());
            }
            Instruct::Comment(_) => {
                // indentation = 0
                newline(w)?;
                print_instr(w, instr, dv_labels)?;
            }
            Instruct::Label(_) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr, dv_labels)
            })?,
            Instruct::Try(InstructTry::TryCatchBegin) => {
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels)?;
                ctx.indent_inc();
            }
            Instruct::Try(InstructTry::TryCatchMiddle) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr, dv_labels)
            })?,
            Instruct::Try(InstructTry::TryCatchEnd) => {
                ctx.indent_dec();
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels)?;
            }
            _ => {
                ctx.newline(w)?;
                print_instr(w, instr, dv_labels)?;
            }
        }
    }
    Ok(())
}

fn print_fcall_args(
    w: &mut dyn Write,
    FcallArgs {
        flags,
        num_args,
        num_rets,
        inouts,
        readonly,
        async_eager_target,
        context,
    }: &FcallArgs<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    angle(w, |w| write!(w, "{}", fcall_flags_to_string_ffi(*flags)))?;
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
    if let Just(label) = async_eager_target {
        print_label(w, label, dv_labels)?;
    } else {
        w.write_all(b"-")?;
    }
    w.write_all(b" ")?;
    match context {
        Just(s) => quotes(w, |w| w.write_all(s)),
        Nothing => w.write_all(b"\"\""),
    }
}

fn print_special_cls_ref(w: &mut dyn Write, cls_ref: &SpecialClsRef) -> Result<()> {
    w.write_all(match *cls_ref {
        SpecialClsRef::Static => b"Static",
        SpecialClsRef::Self_ => b"Self_",
        SpecialClsRef::Parent => b"Parent",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_null_flavor(w: &mut dyn Write, f: &ObjMethodOp) -> Result<()> {
    w.write_all(match *f {
        ObjMethodOp::NullThrows => b"NullThrows",
        ObjMethodOp::NullSafe => b"NullSafe",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_new(w: &mut dyn Write, new: &InstructNew<'_>) -> Result<()> {
    match new {
        InstructNew::NewObj => w.write_all(b"NewObj"),
        InstructNew::NewObjR => w.write_all(b"NewObjR"),
        InstructNew::NewObjD(cid) => {
            w.write_all(b"NewObjD ")?;
            print_class_id(w, cid)
        }
        InstructNew::NewObjRD(cid) => {
            w.write_all(b"NewObjRD ")?;
            print_class_id(w, cid)
        }
        InstructNew::NewObjS(r) => {
            w.write_all(b"NewObjS ")?;
            print_special_cls_ref(w, r)
        }
    }
}

fn print_call(
    w: &mut dyn Write,
    call: &InstructCall<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    match call {
        InstructCall::FCallClsMethod { fcall_args, log } => {
            w.write_all(b"FCallClsMethod ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            w.write_all(match *log {
                IsLogAsDynamicCallOp::LogAsDynamicCall => b"LogAsDynamicCall",
                IsLogAsDynamicCallOp::DontLogAsDynamicCall => b"DontLogAsDynamicCall",
                _ => panic!("Enum value does not match one of listed variants"),
            })
        }
        InstructCall::FCallClsMethodD {
            fcall_args,
            class,
            method,
        } => {
            w.write_all(b"FCallClsMethodD ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            print_class_id(w, class)?;
            w.write_all(b" ")?;
            print_method_id(w, method)
        }
        InstructCall::FCallClsMethodS { fcall_args, clsref } => {
            w.write_all(b"FCallClsMethodS ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            print_special_cls_ref(w, clsref)
        }
        InstructCall::FCallClsMethodSD {
            fcall_args,
            clsref,
            method,
        } => {
            w.write_all(b"FCallClsMethodSD ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            print_special_cls_ref(w, clsref)?;
            w.write_all(b" ")?;
            print_method_id(w, method)
        }
        InstructCall::FCallCtor(fcall_args) => {
            w.write_all(b"FCallCtor ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" """#)
        }
        InstructCall::FCallFunc(fcall_args) => {
            w.write_all(b"FCallFunc ")?;
            print_fcall_args(w, fcall_args, dv_labels)
        }
        InstructCall::FCallFuncD { fcall_args, func } => {
            w.write_all(b"FCallFuncD ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(b" ")?;
            print_function_id(w, func)
        }
        InstructCall::FCallObjMethod { fcall_args, flavor } => {
            w.write_all(b"FCallObjMethod ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            print_null_flavor(w, flavor)
        }
        InstructCall::FCallObjMethodD {
            fcall_args,
            flavor,
            method,
        } => {
            w.write_all(b"FCallObjMethodD ")?;
            print_fcall_args(w, fcall_args, dv_labels)?;
            w.write_all(br#" "" "#)?;
            print_null_flavor(w, flavor)?;
            w.write_all(b" ")?;
            print_method_id(w, method)
        }
    }
}

fn print_get(w: &mut dyn Write, get: &InstructGet<'_>) -> Result<()> {
    use InstructGet as IG;
    match get {
        IG::CGetL(id) => {
            w.write_all(b"CGetL ")?;
            print_local(w, id)
        }
        IG::CGetQuietL(id) => {
            w.write_all(b"CGetQuietL ")?;
            print_local(w, id)
        }
        IG::CGetL2(id) => {
            w.write_all(b"CGetL2 ")?;
            print_local(w, id)
        }
        IG::CUGetL(id) => {
            w.write_all(b"CUGetL ")?;
            print_local(w, id)
        }
        IG::PushL(id) => {
            w.write_all(b"PushL ")?;
            print_local(w, id)
        }
        IG::CGetG => w.write_all(b"CGetG"),
        IG::CGetS(op) => {
            w.write_all(b"CGetS ")?;
            print_readonly_op(w, op)
        }
        IG::ClassGetC => w.write_all(b"ClassGetC"),
        IG::ClassGetTS => w.write_all(b"ClassGetTS"),
    }
}

fn print_instr(w: &mut dyn Write, instr: &Instruct<'_>, dv_labels: &HashSet<Label>) -> Result<()> {
    match instr {
        Instruct::Iterator(i) => print_iterator(w, i, dv_labels),
        Instruct::Basic(b) => w.write_all(match b {
            InstructBasic::Nop => b"Nop",
            InstructBasic::EntryNop => b"EntryNop",
            InstructBasic::PopC => b"PopC",
            InstructBasic::PopU => b"PopU",
            InstructBasic::Dup => b"Dup",
        }),
        Instruct::LitConst(lit) => print_lit_const(w, lit),
        Instruct::Op(op) => print_op(w, op),
        Instruct::ContFlow(cf) => print_control_flow(w, cf, dv_labels),
        Instruct::Call(c) => print_call(w, c, dv_labels),
        Instruct::New(n) => print_new(w, n),
        Instruct::Misc(misc) => print_misc(w, misc, dv_labels),
        Instruct::Get(get) => print_get(w, get),
        Instruct::Mutator(mutator) => print_mutator(w, mutator),
        Instruct::Label(l) => {
            print_label(w, l, dv_labels)?;
            w.write_all(b":")
        }
        Instruct::Isset(i) => print_isset(w, i),
        Instruct::Base(i) => print_base(w, i),
        Instruct::Final(i) => print_final(w, i),
        Instruct::Try(itry) => print_try(w, itry),
        Instruct::Comment(s) => write_bytes!(w, "# {}", s),
        Instruct::SrcLoc(p) => write!(
            w,
            ".srcloc {}:{},{}:{};",
            p.line_begin, p.col_begin, p.line_end, p.col_end
        ),
        Instruct::Async(a) => print_async(w, a),
        Instruct::Generator(gen) => print_gen_creation_execution(w, gen),
        Instruct::IncludeEvalDefine(ed) => print_include_eval_define(w, ed),
        _ => Err(Error::fail("invalid instruction").into()),
    }
}

fn print_base(w: &mut dyn Write, i: &InstructBase<'_>) -> Result<()> {
    use InstructBase as I;
    match i {
        I::BaseGC(si, m) => {
            w.write_all(b"BaseGC ")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_member_opmode(w, m)
        }
        I::BaseGL(id, m) => {
            w.write_all(b"BaseGL ")?;
            print_local(w, id)?;
            w.write_all(b" ")?;
            print_member_opmode(w, m)
        }
        I::BaseSC(si1, si2, m, op) => {
            w.write_all(b"BaseSC ")?;
            print_stack_index(w, si1)?;
            w.write_all(b" ")?;
            print_stack_index(w, si2)?;
            w.write_all(b" ")?;
            print_member_opmode(w, m)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        I::BaseL(id, m, op) => {
            w.write_all(b"BaseL ")?;
            print_local(w, id)?;
            w.write_all(b" ")?;
            print_member_opmode(w, m)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        I::BaseC(si, m) => {
            w.write_all(b"BaseC ")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_member_opmode(w, m)
        }
        I::BaseH => w.write_all(b"BaseH"),
        I::Dim(m, mk) => {
            w.write_all(b"Dim ")?;
            print_member_opmode(w, m)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
    }
}

fn print_stack_index(w: &mut dyn Write, si: &StackIndex) -> Result<()> {
    w.write_all(si.to_string().as_bytes())
}

fn print_member_opmode(w: &mut dyn Write, m: &MOpMode) -> Result<()> {
    use MOpMode as M;
    w.write_all(match *m {
        M::None => b"None",
        M::Warn => b"Warn",
        M::Define => b"Define",
        M::Unset => b"Unset",
        M::InOut => b"InOut",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_member_key(w: &mut dyn Write, mk: &MemberKey<'_>) -> Result<()> {
    use MemberKey as M;
    match mk {
        M::EC(si, op) => {
            w.write_all(b"EC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EL(local, op) => {
            w.write_all(b"EL:")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::ET(s, op) => {
            w.write_all(b"ET:")?;
            quotes(w, |w| w.write_all(&escaper::escape_bstr(s.as_bstr())))?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::EI(i, op) => {
            concat_str(w, ["EI:", i.to_string().as_ref()])?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PC(si, op) => {
            w.write_all(b"PC:")?;
            print_stack_index(w, si)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PL(local, op) => {
            w.write_all(b"PL:")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::PT(id, op) => {
            w.write_all(b"PT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::QT(id, op) => {
            w.write_all(b"QT:")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            print_readonly_op(w, op)
        }
        M::W => w.write_all(b"W"),
    }
}

fn print_iterator(
    w: &mut dyn Write,
    i: &InstructIterator<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    use InstructIterator as I;
    match i {
        I::IterInit(iter_args, label) => {
            w.write_all(b"IterInit ")?;
            print_iter_args(w, iter_args)?;
            w.write_all(b" ")?;
            print_label(w, label, dv_labels)
        }
        I::IterNext(iter_args, label) => {
            w.write_all(b"IterNext ")?;
            print_iter_args(w, iter_args)?;
            w.write_all(b" ")?;
            print_label(w, label, dv_labels)
        }
        I::IterFree(id) => {
            w.write_all(b"IterFree ")?;
            print_iterator_id(w, id)
        }
    }
}

fn print_iter_args(w: &mut dyn Write, iter_args: &IterArgs<'_>) -> Result<()> {
    print_iterator_id(w, &iter_args.iter_id)?;
    w.write_all(b" ")?;
    match &iter_args.key_id {
        Nothing => w.write_all(b"NK")?,
        Just(k) => {
            w.write_all(b"K:")?;
            print_local(w, k)?;
        }
    };
    w.write_all(b" ")?;
    w.write_all(b"V:")?;
    print_local(w, &iter_args.val_id)
}

fn print_iterator_id(w: &mut dyn Write, i: &IterId) -> Result<()> {
    write!(w, "{}", i)
}

fn print_async(w: &mut dyn Write, a: &AsyncFunctions<'_>) -> Result<()> {
    use AsyncFunctions as A;
    match a {
        A::WHResult => w.write_all(b"WHResult"),
        A::Await => w.write_all(b"Await"),
        A::AwaitAll(Just(Pair(Local::Unnamed(id), count))) => {
            write!(w, "AwaitAll L:{}+{}", id, count)
        }
        A::AwaitAll(Nothing) => w.write_all(b"AwaitAll L:0+0"),
        _ => Err(Error::fail("AwaitAll needs an unnamed local").into()),
    }
}

fn print_query_op(w: &mut dyn Write, q: QueryMOp) -> Result<()> {
    w.write_all(match q {
        QueryMOp::CGet => b"CGet",
        QueryMOp::CGetQuiet => b"CGetQuiet",
        QueryMOp::Isset => b"Isset",
        QueryMOp::InOut => b"InOut",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_final(w: &mut dyn Write, f: &InstructFinal<'_>) -> Result<()> {
    use InstructFinal as F;
    match f {
        F::QueryM(n, op, mk) => {
            w.write_all(b"QueryM ")?;
            print_int(w, n)?;
            w.write_all(b" ")?;
            print_query_op(w, *op)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::UnsetM(n, mk) => {
            w.write_all(b"UnsetM ")?;
            print_int(w, n)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::SetM(i, mk) => {
            w.write_all(b"SetM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::SetOpM(i, op, mk) => {
            w.write_all(b"SetOpM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_eq_op(w, op)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::IncDecM(i, op, mk) => {
            w.write_all(b"IncDecM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_incdec_op(w, op)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::SetRangeM(i, s, op) => {
            w.write_all(b"SetRangeM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_int(w, &(*s as usize))?;
            w.write_all(b" ")?;
            w.write_all(match *op {
                SetRangeOp::Forward => b"Forward",
                SetRangeOp::Reverse => b"Reverse",
                _ => panic!("Enum value does not match one of listed variants"),
            })
        }
    }
}

fn print_isset(w: &mut dyn Write, isset: &InstructIsset<'_>) -> Result<()> {
    use InstructIsset as I;
    match isset {
        I::IssetC => w.write_all(b"IssetC"),
        I::IssetL(local) => {
            w.write_all(b"IssetL ")?;
            print_local(w, local)
        }
        I::IsUnsetL(local) => {
            w.write_all(b"IsUnsetL ")?;
            print_local(w, local)
        }
        I::IssetG => w.write_all(b"IssetG"),
        I::IssetS => w.write_all(b"IssetS"),
        I::IsTypeC(op) => {
            w.write_all(b"IsTypeC ")?;
            print_istype_op(w, op)
        }
        I::IsTypeL(local, op) => {
            w.write_all(b"IsTypeL ")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            print_istype_op(w, op)
        }
    }
}

fn print_istype_op(w: &mut dyn Write, op: &IsTypeOp) -> Result<()> {
    use IsTypeOp as Op;
    match *op {
        Op::Null => w.write_all(b"Null"),
        Op::Bool => w.write_all(b"Bool"),
        Op::Int => w.write_all(b"Int"),
        Op::Dbl => w.write_all(b"Dbl"),
        Op::Str => w.write_all(b"Str"),
        Op::Obj => w.write_all(b"Obj"),
        Op::Res => w.write_all(b"Res"),
        Op::Scalar => w.write_all(b"Scalar"),
        Op::Keyset => w.write_all(b"Keyset"),
        Op::Dict => w.write_all(b"Dict"),
        Op::Vec => w.write_all(b"Vec"),
        Op::ArrLike => w.write_all(b"ArrLike"),
        Op::LegacyArrLike => w.write_all(b"LegacyArrLike"),
        Op::ClsMeth => w.write_all(b"ClsMeth"),
        Op::Func => w.write_all(b"Func"),
        Op::Class => w.write_all(b"Class"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_try(w: &mut dyn Write, itry: &InstructTry) -> Result<()> {
    use InstructTry as T;
    match itry {
        T::TryCatchBegin => w.write_all(b".try {"),
        T::TryCatchMiddle => w.write_all(b"} .catch {"),
        T::TryCatchEnd => w.write_all(b"}"),
    }
}

fn print_mutator(w: &mut dyn Write, mutator: &InstructMutator<'_>) -> Result<()> {
    use InstructMutator as M;
    match mutator {
        M::SetL(local) => {
            w.write_all(b"SetL ")?;
            print_local(w, local)
        }
        M::PopL(id) => {
            w.write_all(b"PopL ")?;
            print_local(w, id)
        }
        M::SetG => w.write_all(b"SetG"),
        M::SetS(op) => {
            w.write_all(b"SetS ")?;
            print_readonly_op(w, op)
        }
        M::SetOpL(id, op) => {
            w.write_all(b"SetOpL ")?;
            print_local(w, id)?;
            w.write_all(b" ")?;
            print_eq_op(w, op)
        }
        M::SetOpG(op) => {
            w.write_all(b"SetOpG ")?;
            print_eq_op(w, op)
        }
        M::SetOpS(op) => {
            w.write_all(b"SetOpS ")?;
            print_eq_op(w, op)
        }
        M::IncDecL(id, op) => {
            w.write_all(b"IncDecL ")?;
            print_local(w, id)?;
            w.write_all(b" ")?;
            print_incdec_op(w, op)
        }
        M::IncDecG(op) => {
            w.write_all(b"IncDecG ")?;
            print_incdec_op(w, op)
        }
        M::IncDecS(op) => {
            w.write_all(b"IncDecS ")?;
            print_incdec_op(w, op)
        }
        M::UnsetL(id) => {
            w.write_all(b"UnsetL ")?;
            print_local(w, id)
        }
        M::UnsetG => w.write_all(b"UnsetG"),
        M::CheckProp(id) => {
            w.write_all(b"CheckProp ")?;
            print_prop_id(w, id)
        }
        M::InitProp(id, op) => {
            w.write_all(b"InitProp ")?;
            print_prop_id(w, id)?;
            w.write_all(b" ")?;
            match *op {
                InitPropOp::Static => w.write_all(b"Static"),
                InitPropOp::NonStatic => w.write_all(b"NonStatic"),
                _ => panic!("Enum value does not match one of listed variants"),
            }?;
            w.write_all(b" ")
        }
    }
}

fn print_eq_op(w: &mut dyn Write, op: &SetOpOp) -> Result<()> {
    w.write_all(match *op {
        SetOpOp::PlusEqual => b"PlusEqual",
        SetOpOp::MinusEqual => b"MinusEqual",
        SetOpOp::MulEqual => b"MulEqual",
        SetOpOp::ConcatEqual => b"ConcatEqual",
        SetOpOp::DivEqual => b"DivEqual",
        SetOpOp::PowEqual => b"PowEqual",
        SetOpOp::ModEqual => b"ModEqual",
        SetOpOp::AndEqual => b"AndEqual",
        SetOpOp::OrEqual => b"OrEqual",
        SetOpOp::XorEqual => b"XorEqual",
        SetOpOp::SlEqual => b"SlEqual",
        SetOpOp::SrEqual => b"SrEqual",
        SetOpOp::PlusEqualO => b"PlusEqualO",
        SetOpOp::MinusEqualO => b"MinusEqualO",
        SetOpOp::MulEqualO => b"MulEqualO",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_readonly_op(w: &mut dyn Write, op: &ReadonlyOp) -> Result<()> {
    w.write_all(match *op {
        ReadonlyOp::Readonly => b"Readonly",
        ReadonlyOp::Mutable => b"Mutable",
        ReadonlyOp::Any => b"Any",
        ReadonlyOp::CheckROCOW => b"CheckROCOW",
        ReadonlyOp::CheckMutROCOW => b"CheckMutROCOW",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_incdec_op(w: &mut dyn Write, op: &IncDecOp) -> Result<()> {
    w.write_all(match *op {
        IncDecOp::PreInc => b"PreInc",
        IncDecOp::PostInc => b"PostInc",
        IncDecOp::PreDec => b"PreDec",
        IncDecOp::PostDec => b"PostDec",
        IncDecOp::PreIncO => b"PreIncO",
        IncDecOp::PostIncO => b"PostIncO",
        IncDecOp::PreDecO => b"PreDecO",
        IncDecOp::PostDecO => b"PostDecO",
        _ => panic!("Enum value does not match one of listed variants"),
    })
}

fn print_gen_creation_execution(w: &mut dyn Write, gen: &GenCreationExecution) -> Result<()> {
    use GenCreationExecution as G;
    match gen {
        G::CreateCont => w.write_all(b"CreateCont"),
        G::ContEnter => w.write_all(b"ContEnter"),
        G::ContRaise => w.write_all(b"ContRaise"),
        G::Yield => w.write_all(b"Yield"),
        G::YieldK => w.write_all(b"YieldK"),
        G::ContCheck(ContCheckOp::IgnoreStarted) => w.write_all(b"ContCheck IgnoreStarted"),
        G::ContCheck(ContCheckOp::CheckStarted) => w.write_all(b"ContCheck CheckStarted"),
        G::ContValid => w.write_all(b"ContValid"),
        G::ContKey => w.write_all(b"ContKey"),
        G::ContGetReturn => w.write_all(b"ContGetReturn"),
        G::ContCurrent => w.write_all(b"ContCurrent"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_misc(
    w: &mut dyn Write,
    misc: &InstructMisc<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    use InstructMisc as M;
    match misc {
        M::This => w.write_all(b"This"),
        M::CheckThis => w.write_all(b"CheckThis"),
        M::FuncNumArgs => w.write_all(b"FuncNumArgs"),
        M::ChainFaults => w.write_all(b"ChainFaults"),
        M::SetImplicitContextByValue => w.write_all(b"SetImplicitContextByValue"),
        M::VerifyRetTypeC => w.write_all(b"VerifyRetTypeC"),
        M::VerifyRetTypeTS => w.write_all(b"VerifyRetTypeTS"),
        M::Self_ => w.write_all(b"Self"),
        M::Parent => w.write_all(b"Parent"),
        M::LateBoundCls => w.write_all(b"LateBoundCls"),
        M::ClassName => w.write_all(b"ClassName"),
        M::LazyClassFromClass => w.write_all(b"LazyClassFromClass"),
        M::RecordReifiedGeneric => w.write_all(b"RecordReifiedGeneric"),
        M::CheckReifiedGenericMismatch => w.write_all(b"CheckReifiedGenericMismatch"),
        M::NativeImpl => w.write_all(b"NativeImpl"),
        M::AKExists => w.write_all(b"AKExists"),
        M::Idx => w.write_all(b"Idx"),
        M::ArrayIdx => w.write_all(b"ArrayIdx"),
        M::ArrayMarkLegacy => w.write_all(b"ArrayMarkLegacy"),
        M::ArrayUnmarkLegacy => w.write_all(b"ArrayUnmarkLegacy"),
        M::BreakTraceHint => w.write_all(b"BreakTraceHint"),
        M::CGetCUNop => w.write_all(b"CGetCUNop"),
        M::UGetCUNop => w.write_all(b"UGetCUNop"),
        M::LockObj => w.write_all(b"LockObj"),
        M::ThrowNonExhaustiveSwitch => w.write_all(b"ThrowNonExhaustiveSwitch"),
        M::RaiseClassStringConversionWarning => w.write_all(b"RaiseClassStringConversionWarning"),
        M::VerifyParamType(id) => {
            w.write_all(b"VerifyParamType ")?;
            print_param_id(w, id)
        }
        M::VerifyParamTypeTS(id) => {
            w.write_all(b"VerifyParamTypeTS ")?;
            print_param_id(w, id)
        }
        M::Silence(local, op) => {
            w.write_all(b"Silence ")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            match *op {
                SilenceOp::Start => w.write_all(b"Start"),
                SilenceOp::End => w.write_all(b"End"),
                _ => panic!("Enum value does not match one of listed variants"),
            }
        }
        M::VerifyOutType(id) => {
            w.write_all(b"VerifyOutType ")?;
            print_param_id(w, id)
        }
        M::CreateCl(n, cid) => concat_str_by(
            w,
            " ",
            ["CreateCl", n.to_string().as_str(), cid.to_string().as_str()],
        ),
        M::BareThis(op) => concat_str_by(
            w,
            " ",
            [
                "BareThis",
                match *op {
                    BareThisOp::Notice => "Notice",
                    BareThisOp::NoNotice => "NoNotice",
                    BareThisOp::NeverNull => "NeverNull",
                    _ => panic!("Enum value does not match one of listed variants"),
                },
            ],
        ),

        M::MemoGet(label, Just(Pair(Local::Unnamed(first), local_count))) => {
            w.write_all(b"MemoGet ")?;
            print_label(w, label, dv_labels)?;
            write!(w, " L:{}+{}", first, local_count)
        }
        M::MemoGet(label, Nothing) => {
            w.write_all(b"MemoGet ")?;
            print_label(w, label, dv_labels)?;
            w.write_all(b" L:0+0")
        }
        M::MemoGet(_, _) => Err(Error::fail("MemoGet needs an unnamed local").into()),

        M::MemoSet(Just(Pair(Local::Unnamed(first), local_count))) => {
            write!(w, "MemoSet L:{}+{}", first, local_count)
        }
        M::MemoSet(Nothing) => w.write_all(b"MemoSet L:0+0"),
        M::MemoSet(_) => Err(Error::fail("MemoSet needs an unnamed local").into()),

        M::MemoGetEager([label1, label2], Just(Pair(Local::Unnamed(first), local_count))) => {
            w.write_all(b"MemoGetEager ")?;
            print_label(w, label1, dv_labels)?;
            w.write_all(b" ")?;
            print_label(w, label2, dv_labels)?;
            write!(w, " L:{}+{}", first, local_count)
        }
        M::MemoGetEager([label1, label2], Nothing) => {
            w.write_all(b"MemoGetEager ")?;
            print_label(w, label1, dv_labels)?;
            w.write_all(b" ")?;
            print_label(w, label2, dv_labels)?;
            w.write_all(b" L:0+0")
        }
        M::MemoGetEager(_, _) => Err(Error::fail("MemoGetEager needs an unnamed local").into()),

        M::MemoSetEager(Just(Pair(Local::Unnamed(first), local_count))) => {
            write!(w, "MemoSetEager L:{}+{}", first, local_count)
        }
        M::MemoSetEager(Nothing) => w.write_all(b"MemoSetEager L:0+0"),
        M::MemoSetEager(_) => Err(Error::fail("MemoSetEager needs an unnamed local").into()),

        M::OODeclExists(k) => concat_str_by(
            w,
            " ",
            [
                "OODeclExists",
                match k {
                    ClassishKind::Class => "Class",
                    ClassishKind::Interface => "Interface",
                    ClassishKind::Trait => "Trait",
                    ClassishKind::Enum => "Enum",
                    ClassishKind::EnumClass => "EnumClass",
                },
            ],
        ),
        M::AssertRATL(local, s) => {
            w.write_all(b"AssertRATL ")?;
            print_local(w, local)?;
            w.write_all(b" ")?;
            w.write_all(s)
        }
        M::AssertRATStk(n, s) => write_bytes!(w, "AssertRATStk {} {}", n, s,),
        M::GetMemoKeyL(local) => {
            w.write_all(b"GetMemoKeyL ")?;
            print_local(w, local)
        }
    }
}

fn print_include_eval_define(w: &mut dyn Write, ed: &InstructIncludeEvalDefine) -> Result<()> {
    match ed {
        InstructIncludeEvalDefine::Incl => w.write_all(b"Incl"),
        InstructIncludeEvalDefine::InclOnce => w.write_all(b"InclOnce"),
        InstructIncludeEvalDefine::Req => w.write_all(b"Req"),
        InstructIncludeEvalDefine::ReqOnce => w.write_all(b"ReqOnce"),
        InstructIncludeEvalDefine::ReqDoc => w.write_all(b"ReqDoc"),
        InstructIncludeEvalDefine::Eval => w.write_all(b"Eval"),
    }
}

fn print_control_flow(
    w: &mut dyn Write,
    cf: &InstructControlFlow<'_>,
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    use InstructControlFlow as CF;
    match cf {
        CF::Jmp(l) => {
            w.write_all(b"Jmp ")?;
            print_label(w, l, dv_labels)
        }
        CF::JmpNS(l) => {
            w.write_all(b"JmpNS ")?;
            print_label(w, l, dv_labels)
        }
        CF::JmpZ(l) => {
            w.write_all(b"JmpZ ")?;
            print_label(w, l, dv_labels)
        }
        CF::JmpNZ(l) => {
            w.write_all(b"JmpNZ ")?;
            print_label(w, l, dv_labels)
        }
        CF::RetC => w.write_all(b"RetC"),
        CF::RetCSuspended => w.write_all(b"RetCSuspended"),
        CF::RetM(p) => concat_str_by(w, " ", ["RetM", p.to_string().as_str()]),
        CF::Throw => w.write_all(b"Throw"),
        CF::Switch {
            kind,
            base,
            targets,
        } => print_switch(w, kind, base, targets.as_ref(), dv_labels),
        CF::SSwitch { cases, targets } => match (cases.as_ref(), targets.as_ref()) {
            ([], _) | (_, []) => Err(Error::fail("sswitch should have at least one case").into()),
            ([rest_cases @ .., _last_case], [rest_targets @ .., last_target]) => {
                w.write_all(b"SSwitch ")?;
                let rest: Vec<_> = rest_cases
                    .iter()
                    .zip(rest_targets)
                    .map(|(case, target)| Pair(case, target))
                    .collect();
                angle(w, |w| {
                    concat_by(w, " ", rest, |w, Pair(case, target)| {
                        write_bytes!(w, r#""{}":"#, escaper::escape_bstr(case.as_bstr()))?;
                        print_label(w, target, dv_labels)
                    })?;
                    w.write_all(b" -:")?;
                    print_label(w, last_target, dv_labels)
                })
            }
        },
    }
}

fn print_switch(
    w: &mut dyn Write,
    kind: &SwitchKind,
    base: &isize,
    labels: &[Label],
    dv_labels: &HashSet<Label>,
) -> Result<()> {
    w.write_all(b"Switch ")?;
    w.write_all(match *kind {
        SwitchKind::Bounded => b"Bounded ",
        SwitchKind::Unbounded => b"Unbounded ",
        _ => panic!("Enum value does not match one of listed variants"),
    })?;
    w.write_all(base.to_string().as_bytes())?;
    w.write_all(b" ")?;
    angle(w, |w| {
        concat_by(w, " ", labels, |w, label| print_label(w, label, dv_labels))
    })
}

fn print_lit_const(w: &mut dyn Write, lit: &InstructLitConst<'_>) -> Result<()> {
    use InstructLitConst as LC;
    match lit {
        LC::Null => w.write_all(b"Null"),
        LC::Int(i) => concat_str_by(w, " ", ["Int", i.to_string().as_str()]),
        LC::String(s) => {
            w.write_all(b"String ")?;
            quotes(w, |w| w.write_all(&escaper::escape_bstr(s.as_bstr())))
        }
        LC::LazyClass(id) => {
            w.write_all(b"LazyClass ")?;
            print_class_id(w, id)
        }
        LC::True => w.write_all(b"True"),
        LC::False => w.write_all(b"False"),
        LC::Double(d) => write_bytes!(w, "Double {}", d),
        LC::AddElemC => w.write_all(b"AddElemC"),
        LC::AddNewElemC => w.write_all(b"AddNewElemC"),
        LC::NewPair => w.write_all(b"NewPair"),
        LC::File => w.write_all(b"File"),
        LC::Dir => w.write_all(b"Dir"),
        LC::Method => w.write_all(b"Method"),
        LC::FuncCred => w.write_all(b"FuncCred"),
        LC::Dict(id) => {
            w.write_all(b"Dict ")?;
            print_adata_id(w, id)
        }
        LC::Keyset(id) => {
            w.write_all(b"Keyset ")?;
            print_adata_id(w, id)
        }
        LC::Vec(id) => {
            w.write_all(b"Vec ")?;
            print_adata_id(w, id)
        }
        LC::NewDictArray(i) => concat_str_by(w, " ", ["NewDictArray", i.to_string().as_str()]),
        LC::NewVec(i) => concat_str_by(w, " ", ["NewVec", i.to_string().as_str()]),
        LC::NewKeysetArray(i) => concat_str_by(w, " ", ["NewKeysetArray", i.to_string().as_str()]),
        LC::NewStructDict(l) => {
            let ls: Vec<&str> = l.as_ref().iter().map(|s| s.unsafe_as_str()).collect();
            w.write_all(b"NewStructDict ")?;
            angle(w, |w| print_shape_fields(w, &ls[0..]))
        }
        LC::NewRecord(cid, l) => {
            let ls: Vec<&str> = l.as_ref().iter().map(|s| s.unsafe_as_str()).collect();
            w.write_all(b"NewRecord ")?;
            print_class_id(w, cid)?;
            w.write_all(b" ")?;
            angle(w, |w| print_shape_fields(w, &ls[0..]))
        }
        LC::CnsE(id) => {
            w.write_all(b"CnsE ")?;
            print_const_id(w, id)
        }
        LC::ClsCns(id) => {
            w.write_all(b"ClsCns ")?;
            print_const_id(w, id)
        }
        LC::ClsCnsD(const_id, cid) => {
            w.write_all(b"ClsCnsD ")?;
            print_const_id(w, const_id)?;
            w.write_all(b" ")?;
            print_class_id(w, cid)
        }
        LC::ClsCnsL(id) => {
            w.write_all(b"ClsCnsL ")?;
            print_local(w, id)
        }
        LC::NewCol(ct) => {
            w.write_all(b"NewCol ")?;
            print_collection_type(w, ct)
        }
        LC::ColFromArray(ct) => {
            w.write_all(b"ColFromArray ")?;
            print_collection_type(w, ct)
        }
        LC::NullUninit => w.write_all(b"NullUninit"),
        LC::TypedValue(_) => Err(Error::fail("print_lit_const: TypedValue").into()),
    }
}

fn print_collection_type(w: &mut dyn Write, ct: &CollectionType) -> Result<()> {
    use CollectionType as CT;
    match *ct {
        CT::Vector => w.write_all(b"Vector"),
        CT::Map => w.write_all(b"Map"),
        CT::Set => w.write_all(b"Set"),
        CT::Pair => w.write_all(b"Pair"),
        CT::ImmVector => w.write_all(b"ImmVector"),
        CT::ImmMap => w.write_all(b"ImmMap"),
        CT::ImmSet => w.write_all(b"ImmSet"),
        _ => panic!("Enum value does not match one of listed variants"),
    }
}

fn print_shape_fields(w: &mut dyn Write, sf: &[&str]) -> Result<()> {
    concat_by(w, " ", sf, |w, f| {
        quotes(w, |w| w.write_all(escaper::escape(*f).as_bytes()))
    })
}

fn print_op(w: &mut dyn Write, op: &InstructOperator<'_>) -> Result<()> {
    use InstructOperator as I;
    match op {
        I::Concat => w.write_all(b"Concat"),
        I::ConcatN(n) => concat_str_by(w, " ", ["ConcatN", n.to_string().as_str()]),
        I::Add => w.write_all(b"Add"),
        I::Sub => w.write_all(b"Sub"),
        I::Mul => w.write_all(b"Mul"),
        I::AddO => w.write_all(b"AddO"),
        I::SubO => w.write_all(b"SubO"),
        I::MulO => w.write_all(b"MulO"),
        I::Div => w.write_all(b"Div"),
        I::Mod => w.write_all(b"Mod"),
        I::Pow => w.write_all(b"Pow"),
        I::Not => w.write_all(b"Not"),
        I::Same => w.write_all(b"Same"),
        I::NSame => w.write_all(b"NSame"),
        I::Eq => w.write_all(b"Eq"),
        I::Neq => w.write_all(b"Neq"),
        I::Lt => w.write_all(b"Lt"),
        I::Lte => w.write_all(b"Lte"),
        I::Gt => w.write_all(b"Gt"),
        I::Gte => w.write_all(b"Gte"),
        I::Cmp => w.write_all(b"Cmp"),
        I::BitAnd => w.write_all(b"BitAnd"),
        I::BitOr => w.write_all(b"BitOr"),
        I::BitXor => w.write_all(b"BitXor"),
        I::BitNot => w.write_all(b"BitNot"),
        I::Shl => w.write_all(b"Shl"),
        I::Shr => w.write_all(b"Shr"),
        I::CastBool => w.write_all(b"CastBool"),
        I::CastInt => w.write_all(b"CastInt"),
        I::CastDouble => w.write_all(b"CastDouble"),
        I::CastString => w.write_all(b"CastString"),
        I::CastVec => w.write_all(b"CastVec"),
        I::CastDict => w.write_all(b"CastDict"),
        I::CastKeyset => w.write_all(b"CastKeyset"),
        I::InstanceOf => w.write_all(b"InstanceOf"),
        I::InstanceOfD(id) => {
            w.write_all(b"InstanceOfD ")?;
            print_class_id(w, id)
        }
        I::IsLateBoundCls => w.write_all(b"IsLateBoundCls"),
        I::IsTypeStructC(op) => concat_str_by(
            w,
            " ",
            [
                "IsTypeStructC",
                match *op {
                    TypeStructResolveOp::Resolve => "Resolve",
                    TypeStructResolveOp::DontResolve => "DontResolve",
                    _ => panic!("Enum value does not match one of listed variants"),
                },
            ],
        ),
        I::ThrowAsTypeStructException => w.write_all(b"ThrowAsTypeStructException"),
        I::CombineAndResolveTypeStruct(n) => concat_str_by(
            w,
            " ",
            ["CombineAndResolveTypeStruct", n.to_string().as_str()],
        ),
        I::Print => w.write_all(b"Print"),
        I::Clone => w.write_all(b"Clone"),
        I::Exit => w.write_all(b"Exit"),
        I::ResolveFunc(id) => {
            w.write_all(b"ResolveFunc ")?;
            print_function_id(w, id)
        }
        I::ResolveRFunc(id) => {
            w.write_all(b"ResolveRFunc ")?;
            print_function_id(w, id)
        }
        I::ResolveMethCaller(id) => {
            w.write_all(b"ResolveMethCaller ")?;
            print_function_id(w, id)
        }
        I::ResolveClsMethod(mid) => {
            w.write_all(b"ResolveClsMethod ")?;
            print_method_id(w, mid)
        }
        I::ResolveClsMethodD(cid, mid) => {
            w.write_all(b"ResolveClsMethodD ")?;
            print_class_id(w, cid)?;
            w.write_all(b" ")?;
            print_method_id(w, mid)
        }
        I::ResolveClsMethodS(r, mid) => {
            w.write_all(b"ResolveClsMethodS ")?;
            print_special_cls_ref(w, r)?;
            w.write_all(b" ")?;
            print_method_id(w, mid)
        }
        I::ResolveRClsMethod(mid) => {
            w.write_all(b"ResolveRClsMethod ")?;
            print_method_id(w, mid)
        }
        I::ResolveRClsMethodD(cid, mid) => {
            w.write_all(b"ResolveRClsMethodD ")?;
            print_class_id(w, cid)?;
            w.write_all(b" ")?;
            print_method_id(w, mid)
        }
        I::ResolveRClsMethodS(r, mid) => {
            w.write_all(b"ResolveRClsMethodS ")?;
            print_special_cls_ref(w, r)?;
            w.write_all(b" ")?;
            print_method_id(w, mid)
        }
        I::ResolveClass(id) => {
            w.write_all(b"ResolveClass ")?;
            print_class_id(w, id)
        }
        I::Fatal(fatal_op) => print_fatal_op(w, fatal_op),
    }
}

fn print_fatal_op(w: &mut dyn Write, f: &FatalOp) -> Result<()> {
    match *f {
        FatalOp::Parse => w.write_all(b"Fatal Parse"),
        FatalOp::Runtime => w.write_all(b"Fatal Runtime"),
        FatalOp::RuntimeOmitFrame => w.write_all(b"Fatal RuntimeOmitFrame"),
        _ => panic!("Enum value does not match one of listed variants"),
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

fn print_param_id(w: &mut dyn Write, param_id: &ParamId<'_>) -> Result<()> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamId::ParamNamed(s) => w.write_all(s),
    }
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

fn print_label(w: &mut dyn Write, label: &Label, dv_labels: &HashSet<Label>) -> Result<()> {
    let prefix = if dv_labels.contains(label) { "DV" } else { "L" };
    write!(w, "{}{}", prefix, label)
}

fn print_local(w: &mut dyn Write, local: &Local<'_>) -> Result<()> {
    match local {
        Local::Unnamed(id) => {
            w.write_all(b"_")?;
            print_int(w, id)
        }
        Local::Named(id) => w.write_all(id),
    }
}

fn print_int<T: std::fmt::Display>(w: &mut dyn Write, i: T) -> Result<()> {
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

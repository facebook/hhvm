// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{
    context::Context,
    not_impl,
    write::{
        self, angle, braces, concat, concat_by, concat_str, concat_str_by, newline, option,
        option_or, paren, quotes, square, triple_quotes, wrap_by, wrap_by_, Error,
    },
};
use core_utils_rust::add_ns;
use escaper::{escape, escape_by, is_lit_printable};
use ffi::{Maybe, Maybe::*, Pair, Quadruple, Slice, Str, Triple};
use hhas_adata::{HhasAdata, DICT_PREFIX, KEYSET_PREFIX, VEC_PREFIX};
use hhas_attribute::{self as hhas_attribute, HhasAttribute};
use hhas_body::{HhasBody, HhasBodyEnv};
use hhas_class::{self as hhas_class, HhasClass};
use hhas_coeffects::{HhasCoeffects, HhasCtxConstant};
use hhas_constant::HhasConstant;
use hhas_function::HhasFunction;
use hhas_method::{HhasMethod, HhasMethodFlags};
use hhas_param::HhasParam;
use hhas_pos::{HhasPos, HhasSpan};
use hhas_program::HhasProgram;
use hhas_property::HhasProperty;
use hhas_record_def::{Field, HhasRecord};
use hhas_symbol_refs::{HhasSymbolRefs, IncludePath};
use hhas_type::{constraint, HhasTypeInfo};
use hhas_type_const::HhasTypeConstant;
use hhas_typedef::HhasTypedef;
use hhbc_ast::*;
use hhbc_by_ref_hhbc_string_utils::{
    float, integer, is_class, is_parent, is_self, is_static, is_xhp, lstrip, mangle, quote_string,
    quote_string_with_escape, strip_global_ns, strip_ns, triple_quote_string, types,
};
use hhbc_id::{class::ClassType, Id};
use indexmap::IndexSet;
use instruction_sequence::{Error::Unrecoverable, InstrSeq};
use iterator::Id as IterId;
use itertools::Itertools;
use label::Label;
use lazy_static::lazy_static;
use local::Local;
use naming_special_names_rust::classes;
use ocaml_helper::escaped;
use oxidized::{
    ast,
    ast_defs::{self, ParamKind},
    local_id,
};
use regex::Regex;
use runtime::TypedValue;
use std::{
    borrow::Cow,
    io::{self, Result, Write},
    path::Path,
    write,
};

macro_rules! write_if {
    ($pred:expr, $($rest:tt)*) => {
        if ($pred) { write!($($rest)*) } else { Ok(()) }
    };
}

pub struct ExprEnv<'arena, 'e> {
    pub codegen_env: Option<&'e HhasBodyEnv<'arena>>,
}

fn print_program(ctx: &Context<'_>, w: &mut dyn Write, prog: &HhasProgram<'_>) -> Result<()> {
    match ctx.path {
        Some(p) => {
            let abs = p.to_absolute();
            let p = escape(
                abs.to_str()
                    .ok_or(<io::Error as From<Error>>::from(Error::InvalidUTF8))?,
            );

            concat_str_by(w, " ", ["#", p.as_ref(), "starts here"])?;

            newline(w)?;

            newline(w)?;
            concat_str(w, [".filepath ", format!("\"{}\"", p).as_str(), ";"])?;

            newline(w)?;
            handle_not_impl(|| print_program_(ctx, w, prog))?;

            newline(w)?;
            concat_str_by(w, " ", ["#", p.as_ref(), "ends here"])?;

            newline(w)
        }
        None => {
            w.write_all(b"#starts here")?;

            newline(w)?;
            handle_not_impl(|| print_program_(ctx, w, prog))?;

            newline(w)?;
            w.write_all(b"#ends here")?;

            newline(w)
        }
    }
}

fn get_fatal_op(f: &FatalOp) -> &str {
    match f {
        FatalOp::Parse => "Parse",
        FatalOp::Runtime => "Runtime",
        FatalOp::RuntimeOmitFrame => "RuntimeOmitFrame",
    }
}

fn print_program_(ctx: &Context<'_>, w: &mut dyn Write, prog: &HhasProgram<'_>) -> Result<()> {
    if let Just(Triple(fop, p, msg)) = &prog.fatal {
        newline(w)?;
        let HhasPos {
            line_begin,
            line_end,
            col_begin,
            col_end,
        } = p;
        let pos = format!("{}:{},{}:{}", line_begin, col_begin, line_end, col_end);
        concat_str(
            w,
            [
                ".fatal ",
                pos.as_ref(),
                " ",
                get_fatal_op(fop),
                " \"",
                escape(msg.unsafe_as_str()).as_ref(),
                "\";",
            ],
        )?;
    }

    newline(w)?;
    concat(w, &prog.adata, |w, a| print_adata_region(ctx, w, a))?;
    concat(w, &prog.functions, |w, f| print_fun_def(ctx, w, f))?;
    concat(w, &prog.classes, |w, cd| print_class_def(ctx, w, cd))?;
    concat(w, &prog.record_defs, |w, rd| print_record_def(ctx, w, rd))?;
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
            IncludePath::Absolute(p) => print_if_exists(w, Path::new(&p.unsafe_as_str())),
            IncludePath::SearchPathRelative(p) => {
                let path_from_cur_dirname = ctx
                    .path
                    .and_then(|p| p.path().parent())
                    .unwrap_or_else(|| Path::new(""))
                    .join(&p.unsafe_as_str());
                if path_from_cur_dirname.exists() {
                    print_path(w, &path_from_cur_dirname)
                } else {
                    let search_paths = ctx.include_search_paths;
                    for prefix in search_paths.iter() {
                        let path = Path::new(prefix).join(&p.unsafe_as_str());
                        if path.exists() {
                            return print_path(w, &path);
                        }
                    }
                    Ok(())
                }
            }
            IncludePath::IncludeRootRelative(v, p) => {
                if !p.is_empty() {
                    include_roots
                        .get(&v.unsafe_as_str().to_owned())
                        .iter()
                        .try_for_each(|ir| {
                            let doc_root = ctx.doc_root;
                            let resolved = Path::new(doc_root).join(ir).join(&p.unsafe_as_str());
                            print_if_exists(w, &resolved)
                        })?
                }
                Ok(())
            }
            IncludePath::DocRootRelative(p) => {
                let doc_root = ctx.doc_root;
                let resolved = Path::new(doc_root).join(&p.unsafe_as_str());
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
                    w.write_all(s.unsafe_as_str().as_bytes())?;
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
    concat_str_by(w, " ", [".adata", adata.id.unsafe_as_str(), "= "])?;
    triple_quotes(w, |w| print_adata(ctx, w, &adata.value))?;
    w.write_all(b";")?;
    ctx.newline(w)
}

fn print_typedef(ctx: &Context<'_>, w: &mut dyn Write, td: &HhasTypedef<'_>) -> Result<()> {
    newline(w)?;
    w.write_all(b".alias ")?;
    print_typedef_attributes(ctx, w, td)?;
    w.write_all(td.name.to_raw_string().as_bytes())?;
    w.write_all(b" = ")?;
    print_typedef_info(w, &td.type_info)?;
    w.write_all(b" ")?;
    print_span(w, &td.span)?;
    w.write_all(b" ")?;
    triple_quotes(w, |w| print_adata(ctx, w, &td.type_structure))?;
    w.write_all(b";")
}

fn print_typedef_attributes(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    td: &HhasTypedef<'_>,
) -> Result<()> {
    let mut specials = vec![];
    if ctx.is_system_lib() {
        specials.push("persistent");
    }
    print_special_and_user_attrs(ctx, w, &specials[..], td.attributes.as_ref())
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
    print_fun_attrs(ctx, w, fun_def)?;
    print_span(w, &fun_def.span)?;
    w.write_all(b" ")?;
    option(
        w,
        &(Option::from(body.return_type_info.clone())),
        |w, ti| {
            print_type_info(w, ti)?;
            w.write_all(b" ")
        },
    )?;
    w.write_all(fun_def.name.to_raw_string().as_bytes())?;
    print_params(ctx, w, fun_def.params())?;
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
        ctx.block(w, |c, w| print_body(c, w, body, &fun_def.coeffects))?;
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
            write!(w, "extends <{}>;", name.to_raw_string())
        }
        Pair(name, hhas_class::TraitReqKind::MustImplement) => {
            write!(w, "implements <{}>;", name.to_raw_string())
        }
    }
}

fn print_type_constant(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    c: &HhasTypeConstant<'_>,
) -> Result<()> {
    ctx.newline(w)?;
    concat_str_by(w, " ", [".const", c.name.unsafe_as_str(), "isType"])?;
    if c.is_abstract {
        w.write_all(b" isAbstract")?;
    }
    option(w, Option::from(c.initializer.as_ref()), |w, init| {
        w.write_all(b" = ")?;
        triple_quotes(w, |w| print_adata(ctx, w, init))
    })?;
    w.write_all(b";")
}

fn print_ctx_constant(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasCtxConstant<'_>) -> Result<()> {
    ctx.newline(w)?;
    concat_str_by(w, " ", [".ctx", &c.name.unsafe_as_str()])?;
    if c.is_abstract {
        w.write_all(b" isAbstract")?;
    }
    if let Some(coeffects) =
        HhasCoeffects::vec_to_string(&c.coeffects.0.as_ref(), |c| c.to_string())
    {
        w.write_all(b" ")?;
        w.write_all(coeffects.as_bytes())?;
    }
    if let Some(coeffects) =
        HhasCoeffects::vec_to_string(&c.coeffects.1.as_ref(), |c| c.unsafe_as_str().to_string())
    {
        w.write_all(b" ")?;
        w.write_all(coeffects.as_bytes())?;
    }
    w.write_all(b";")?;
    Ok(())
}

fn print_property_doc_comment(w: &mut dyn Write, p: &HhasProperty<'_>) -> Result<()> {
    if let Just(s) = p.doc_comment.as_ref() {
        w.write_all(triple_quote_string(s.unsafe_as_str()).as_bytes())?;
        w.write_all(b" ")?;
    }
    Ok(())
}

fn print_property_attributes(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    property: &HhasProperty<'_>,
) -> Result<()> {
    let mut special_attributes = vec![];
    if property.is_late_init() {
        special_attributes.push("late_init")
    };
    if property.is_no_bad_redeclare() {
        special_attributes.push("no_bad_redeclare")
    };
    if property.initial_satisfies_tc() {
        special_attributes.push("initial_satisfies_tc")
    }
    if property.no_implicit_null() {
        special_attributes.push("no_implicit_null")
    }
    if property.has_system_initial() {
        special_attributes.push("sys_initial_val")
    }
    if property.is_const() {
        special_attributes.push("is_const")
    }
    if property.is_readonly() {
        special_attributes.push("readonly")
    }
    if property.is_deep_init() {
        special_attributes.push("deep_init")
    }
    if property.is_lsb() {
        special_attributes.push("lsb")
    }
    if property.is_static() {
        special_attributes.push("static")
    }
    special_attributes.push(property.visibility.as_ref());
    special_attributes.reverse();

    w.write_all(b"[")?;
    concat_by(w, " ", &special_attributes, |w, a| {
        w.write_all(a.as_bytes())
    })?;
    if !special_attributes.is_empty() && !property.attributes.is_empty() {
        w.write_all(b" ")?;
    }
    print_attributes(ctx, w, &property.attributes)?;
    w.write_all(b"] ")
}

fn print_property_type_info(w: &mut dyn Write, p: &HhasProperty<'_>) -> Result<()> {
    print_type_info(w, &p.type_info)?;
    w.write_all(b" ")
}

fn print_property(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    class_def: &HhasClass<'_>,
    property: &HhasProperty<'_>,
) -> Result<()> {
    newline(w)?;
    w.write_all(b"  .property ")?;
    print_property_attributes(ctx, w, property)?;
    print_property_doc_comment(w, property)?;
    print_property_type_info(w, property)?;
    w.write_all(property.name.to_raw_string().as_bytes())?;
    w.write_all(b" =\n    ")?;
    let initial_value = property.initial_value.as_ref();
    if class_def.is_closure() || initial_value == Just(&TypedValue::Uninit) {
        w.write_all(b"uninit;")
    } else {
        triple_quotes(w, |w| match initial_value {
            Nothing => w.write_all(b"N;"),
            Just(value) => print_adata(ctx, w, &value),
        })?;
        w.write_all(b";")
    }
}

fn print_constant(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasConstant<'_>) -> Result<()> {
    ctx.newline(w)?;
    w.write_all(b".const ")?;
    w.write_all(c.name.to_raw_string().as_bytes())?;
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
    doc_comment: &Maybe<Str<'arena>>,
) -> Result<()> {
    if let Just(cmt) = doc_comment {
        ctx.newline(w)?;
        write!(w, ".doc {};", triple_quote_string(cmt.unsafe_as_str()))?;
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
    concat_str(w, [id1.to_raw_string(), "::", id2.to_raw_string()])?;
    w.write_all(b" insteadof ")?;
    let unique_ids: IndexSet<&str> = ids.as_ref().iter().map(|i| i.to_raw_string()).collect();
    concat_str_by(w, " ", unique_ids.iter().collect::<Vec<_>>())?;
    w.write_all(b";")
}

fn print_use_as_visibility(w: &mut dyn Write, u: UseAsVisibility) -> Result<()> {
    w.write_all(match u {
        UseAsVisibility::UseAsPublic => b"public",
        UseAsVisibility::UseAsPrivate => b"private",
        UseAsVisibility::UseAsProtected => b"protected",
        UseAsVisibility::UseAsFinal => b"final",
    })
}

fn print_use_alias<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    Quadruple(ido1, id, ido2, kindl): &Quadruple<
        Maybe<ClassType<'arena>>,
        ClassType<'arena>,
        Maybe<ClassType<'arena>>,
        Slice<'arena, UseAsVisibility>,
    >,
) -> Result<()> {
    ctx.newline(w)?;
    let id = id.to_raw_string();
    option_or(
        w,
        ido1.as_ref(),
        |w, i: &ClassType<'arena>| concat_str(w, [i.to_raw_string(), "::", id]),
        id,
    )?;
    w.write_all(b" as ")?;
    if !kindl.is_empty() {
        square(w, |w| {
            concat_by(w, " ", kindl, |w, k| print_use_as_visibility(w, *k))
        })?;
    }
    write_if!(!kindl.is_empty() && ido2.is_just(), w, " ")?;
    option(w, ido2.as_ref(), |w, i: &ClassType<'arena>| {
        w.write_all(i.to_raw_string().as_bytes())
    })?;
    w.write_all(b";")
}

fn print_uses<'arena>(ctx: &Context<'_>, w: &mut dyn Write, c: &HhasClass<'arena>) -> Result<()> {
    if c.uses.is_empty() {
        Ok(())
    } else {
        let unique_ids: IndexSet<&str> = c
            .uses
            .as_ref()
            .iter()
            .map(|e| strip_global_ns(e.unsafe_as_str()))
            .collect();
        let unique_ids: Vec<_> = unique_ids.into_iter().collect();

        newline(w)?;
        w.write_all(b"  .use ")?;
        concat_by(w, " ", unique_ids, |w, id| w.write_all(id.as_bytes()))?;

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
                    Slice<'arena, UseAsVisibility>,
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

fn print_class_special_attributes(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    c: &HhasClass<'_>,
) -> Result<()> {
    let user_attrs = c.attributes.as_ref();
    let is_system_lib = ctx.is_system_lib();

    let mut special_attributes: Vec<&str> = vec![];
    if c.needs_no_reifiedinit() {
        special_attributes.push("noreifiedinit")
    }
    if c.no_dynamic_props() {
        special_attributes.push("no_dynamic_props")
    }
    if c.is_const() {
        special_attributes.push("is_const")
    }
    if hhas_attribute::has_foldable(user_attrs) {
        special_attributes.push("foldable")
    }
    if hhas_attribute::has_enum_class(user_attrs) {
        special_attributes.push("enum_class")
    }
    if is_system_lib {
        special_attributes.extend(&["persistent", "builtin", "unique"])
    }
    if hhas_attribute::has_dynamically_constructible(user_attrs) {
        special_attributes.push("dyn_constructible");
    }
    if c.is_closure() {
        special_attributes.push("no_override");
    }
    if c.is_trait() {
        special_attributes.push("trait");
    }
    if c.is_interface() {
        special_attributes.push("interface");
    }
    if c.is_final() {
        special_attributes.push("final");
    }
    if c.is_sealed() {
        special_attributes.push("sealed");
    }
    if c.enum_type.is_just() && !hhas_attribute::has_enum_class(user_attrs) {
        special_attributes.push("enum");
    }
    if c.is_abstract() {
        special_attributes.push("abstract");
    }
    if special_attributes.is_empty() && user_attrs.is_empty() {
        return Ok(());
    }

    special_attributes.reverse();
    wrap_by_(w, "[", "] ", |w| {
        concat_by(w, " ", &special_attributes, |w, a| {
            w.write_all(a.as_bytes())
        })?;
        write_if!(
            !special_attributes.is_empty() && !user_attrs.is_empty(),
            w,
            " ",
        )?;
        print_attributes(ctx, w, &user_attrs)
    })
}

fn print_implements(w: &mut dyn Write, implements: &[ClassType<'_>]) -> Result<()> {
    if implements.is_empty() {
        return Ok(());
    }
    w.write_all(b" implements (")?;
    concat_str_by(
        w,
        " ",
        implements
            .iter()
            .map(|x| x.to_raw_string())
            .collect::<Vec<_>>(),
    )?;
    w.write_all(b")")
}

fn print_enum_includes(w: &mut dyn Write, enum_includes: &[ClassType<'_>]) -> Result<()> {
    if enum_includes.is_empty() {
        return Ok(());
    }
    w.write_all(b" enum_includes (")?;
    concat_str_by(
        w,
        " ",
        enum_includes
            .iter()
            .map(|x| x.to_raw_string())
            .collect::<Vec<_>>(),
    )?;
    w.write_all(b")")
}

fn print_shadowed_tparams<'arena>(
    w: &mut dyn Write,
    shadowed_tparams: impl AsRef<[Str<'arena>]>,
) -> Result<()> {
    braces(w, |w| {
        let tps: Vec<&str> = shadowed_tparams
            .as_ref()
            .iter()
            .map(|s| s.unsafe_as_str())
            .collect();
        concat_str_by(w, ", ", tps)
    })
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
    print_method_attrs(ctx, w, method_def)?;
    print_span(w, &method_def.span)?;
    w.write_all(b" ")?;
    option(w, &(Option::from(body.return_type_info.clone())), |w, t| {
        print_type_info(w, t)?;
        w.write_all(b" ")
    })?;
    w.write_all(method_def.name.to_raw_string().as_bytes())?;
    print_params(ctx, w, &body.params)?;
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
        ctx.block(w, |c, w| print_body(c, w, body, &method_def.coeffects))?;
        newline(w)?;
        w.write_all(b"  ")
    })
}

fn print_method_attrs(ctx: &Context<'_>, w: &mut dyn Write, m: &HhasMethod<'_>) -> Result<()> {
    use hhas_attribute::*;
    let user_attrs = m.attributes.as_ref();
    let mut special_attrs = vec![];
    if has_provenance_skip_frame(user_attrs) {
        special_attrs.push("prov_skip_frame")
    }
    if m.is_interceptable() {
        special_attrs.push("interceptable");
    }
    let visibility = m.visibility.as_ref().to_string();
    special_attrs.push(&visibility);
    if m.flags.contains(HhasMethodFlags::IS_STATIC) {
        special_attrs.push("static");
    }
    if m.flags.contains(HhasMethodFlags::IS_FINAL) {
        special_attrs.push("final");
    }
    if m.flags.contains(HhasMethodFlags::IS_ABSTRACT) {
        special_attrs.push("abstract");
    }
    if m.flags.contains(HhasMethodFlags::IS_READONLY_RETURN) {
        special_attrs.push("readonly_return");
    }
    if m.flags.contains(HhasMethodFlags::IS_READONLY_THIS) {
        special_attrs.push("readonly_this");
    }
    if has_foldable(user_attrs) {
        special_attrs.push("foldable");
    }
    if m.is_no_injection() {
        special_attrs.push("no_injection");
    }
    if ctx.is_system_lib() && has_native(user_attrs) && !is_native_opcode_impl(user_attrs) {
        special_attrs.push("unique");
    }
    if ctx.is_system_lib() {
        special_attrs.push("builtin");
    }
    if ctx.is_system_lib() && has_native(user_attrs) && !is_native_opcode_impl(user_attrs) {
        special_attrs.push("persistent");
    }
    if ctx.is_system_lib() || (has_dynamically_callable(user_attrs) && !m.is_memoize_impl()) {
        special_attrs.push("dyn_callable")
    }
    print_special_and_user_attrs(ctx, w, &special_attrs, user_attrs)
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
    print_class_special_attributes(ctx, w, class_def)?;
    w.write_all(class_def.name.to_raw_string().as_bytes())?;
    w.write_all(b" ")?;
    print_span(w, &class_def.span)?;
    print_extends(
        w,
        Option::from(class_def.base.as_ref()).map(|x: &ClassType<'arena>| x.to_raw_string()),
    )?;
    print_implements(w, class_def.implements.as_ref())?;
    print_enum_includes(w, class_def.enum_includes.as_ref())?;
    w.write_all(b" {")?;
    ctx.block(w, |c, w| {
        print_doc_comment(c, w, &class_def.doc_comment)?;
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
            print_property(c, w, class_def, x)?;
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
    loc: &Option<ast_defs::Pos>,
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
                "p:i:{};s:{}:{};",
                line,
                filename.len(),
                quote_string_with_escape(filename)
            )
        }
        _ => Ok(()),
    }
}

fn print_hhbc_id<'a>(w: &mut dyn Write, id: &impl Id<'a>) -> Result<()> {
    quotes(w, |w| w.write_all(escape(id.to_raw_string()).as_bytes()))
}

fn print_function_id(w: &mut dyn Write, id: &FunctionId<'_>) -> Result<()> {
    print_hhbc_id(w, id)
}

fn print_class_id(w: &mut dyn Write, id: &ClassId<'_>) -> Result<()> {
    print_hhbc_id(w, id)
}

fn print_method_id(w: &mut dyn Write, id: &MethodId<'_>) -> Result<()> {
    print_hhbc_id(w, id)
}

fn print_const_id(w: &mut dyn Write, id: &ConstId<'_>) -> Result<()> {
    print_hhbc_id(w, id)
}

fn print_prop_id(w: &mut dyn Write, id: &PropId<'_>) -> Result<()> {
    print_hhbc_id(w, id)
}

fn print_adata_id(w: &mut dyn Write, id: &AdataId<'_>) -> Result<()> {
    concat_str(w, ["@", id.unsafe_as_str()])
}

fn print_adata_mapped_argument<F, V>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    col_type: &str,
    loc: &Option<ast_defs::Pos>,
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
    loc: &Option<ast_defs::Pos>,
    values: &[TypedValue<'_>],
) -> Result<()> {
    print_adata_mapped_argument(ctx, w, col_type, loc, values, &print_adata)
}

fn print_adata_dict_collection_argument(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    col_type: &str,
    loc: &Option<ast_defs::Pos>,
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
            write!(
                w,
                "s:{}:{};",
                s.len(),
                quote_string_with_escape(s.unsafe_as_str())
            )
        }
        TypedValue::LazyClass(s) => {
            write!(
                w,
                "l:{}:{};",
                s.len(),
                quote_string_with_escape(s.unsafe_as_str())
            )
        }
        TypedValue::Float(f) => write!(w, "d:{};", float::to_string(*f)),
        TypedValue::Int(i) => write!(w, "i:{};", i),
        // TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why?
        TypedValue::Bool(false) => w.write_all(b"b:0;"),
        TypedValue::Bool(true) => w.write_all(b"b:1;"),
        TypedValue::Vec(values) => {
            print_adata_collection_argument(ctx, w, VEC_PREFIX, &None, values.as_ref())
        }
        TypedValue::Dict(pairs) => {
            print_adata_dict_collection_argument(ctx, w, DICT_PREFIX, &None, pairs.as_ref())
        }
        TypedValue::Keyset(values) => {
            print_adata_collection_argument(ctx, w, KEYSET_PREFIX, &None, values.as_ref())
        }
        TypedValue::HhasAdata(s) => w.write_all(escaped(s.unsafe_as_str()).as_bytes()),
    }
}

fn print_attribute(ctx: &Context<'_>, w: &mut dyn Write, a: &HhasAttribute<'_>) -> Result<()> {
    write!(
        w,
        "\"{}\"(\"\"\"{}:{}:{{",
        a.name.unsafe_as_str(),
        VEC_PREFIX,
        a.arguments.len()
    )?;
    concat(w, &a.arguments, |w, arg| print_adata(ctx, w, arg))?;
    w.write_all(b"}\"\"\")")
}

fn print_attributes<'a>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    al: impl AsRef<[HhasAttribute<'a>]>,
) -> Result<()> {
    // Adjust for underscore coming before alphabet
    let al: Vec<&HhasAttribute<'_>> = al
        .as_ref()
        .iter()
        .sorted_by_key(|a| {
            (
                !a.name.unsafe_as_str().starts_with("__"),
                a.name.unsafe_as_str(),
            )
        })
        .collect();
    concat_by(w, " ", &al, |w, a| print_attribute(ctx, w, a))
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
) -> Result<()> {
    print_doc_comment(ctx, w, &body.doc_comment)?;
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
            if var.unsafe_as_str().as_bytes().iter().all(is_bareword_char) {
                w.write_all(var.unsafe_as_str().as_bytes())
            } else {
                quotes(w, |w| w.write_all(escape(var.unsafe_as_str()).as_bytes()))
            }
        })?;
        w.write_all(b";")?;
    }
    if body.num_closures > 0 {
        ctx.newline(w)?;
        write!(w, ".numclosures {};", body.num_closures)?;
    }
    for s in HhasCoeffects::coeffects_to_hhas(&coeffects).iter() {
        ctx.newline(w)?;
        w.write_all(s.as_bytes())?;
    }
    print_instructions(ctx, w, &body.body_instrs)
}

fn print_instructions(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    instr_seq: &InstrSeq<'_>,
) -> Result<()> {
    use Instruct::*;
    use InstructTry::*;
    let mut ctx = ctx.clone();
    for instr in instr_seq.compact_iter() {
        match instr {
            ISpecialFlow(_) => {
                return Err(Error::fail("Cannot break/continue 1 level").into());
            }
            IComment(_) => {
                // indetation = 0
                newline(w)?;
                print_instr(w, instr)?;
            }
            ILabel(_) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr)
            })?,
            ITry(TryCatchBegin) => {
                ctx.newline(w)?;
                print_instr(w, instr)?;
                ctx.indent_inc();
            }
            ITry(TryCatchMiddle) => ctx.unblock(w, |c, w| {
                c.newline(w)?;
                print_instr(w, instr)
            })?,
            ITry(TryCatchEnd) => {
                ctx.indent_dec();
                ctx.newline(w)?;
                print_instr(w, instr)?;
            }
            _ => {
                ctx.newline(w)?;
                print_instr(w, instr)?;
            }
        }
    }
    Ok(())
}

fn if_then<F, R>(cond: bool, f: F) -> Option<R>
where
    F: FnOnce() -> R,
{
    if cond { Some(f()) } else { None }
}

fn print_fcall_args(
    w: &mut dyn Write,
    FcallArgs(fls, num_args, num_rets, inouts, readonly, async_eager_label, context): &FcallArgs<
        '_,
    >,
) -> Result<()> {
    use FcallFlags as F;
    let mut flags = vec![];
    if_then(fls.contains(F::HAS_UNPACK), || flags.push("Unpack"));
    if_then(fls.contains(F::HAS_GENERICS), || flags.push("Generics"));
    if_then(fls.contains(F::LOCK_WHILE_UNWINDING), || {
        flags.push("LockWhileUnwinding")
    });
    if_then(fls.contains(F::ENFORCE_MUTABLE_RETURN), || {
        flags.push("EnforceMutableReturn")
    });
    if_then(fls.contains(F::ENFORCE_READONLY_THIS), || {
        flags.push("EnforceReadonlyThis")
    });
    angle(w, |w| concat_str_by(w, " ", flags))?;
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
    option_or(w, async_eager_label.as_ref(), print_label, "-")?;
    w.write_all(b" ")?;
    match context {
        Just(s) => quotes(w, |w| w.write_all(s.unsafe_as_str().as_bytes())),
        Nothing => w.write_all(b"\"\""),
    }
}

fn print_special_cls_ref(w: &mut dyn Write, cls_ref: &SpecialClsRef) -> Result<()> {
    w.write_all(match cls_ref {
        SpecialClsRef::Static => b"Static",
        SpecialClsRef::Self_ => b"Self",
        SpecialClsRef::Parent => b"Parent",
    })
}

fn print_null_flavor(w: &mut dyn Write, f: &ObjNullFlavor) -> Result<()> {
    w.write_all(match f {
        ObjNullFlavor::NullThrows => b"NullThrows",
        ObjNullFlavor::NullSafe => b"NullSafe",
    })
}

fn print_instr(w: &mut dyn Write, instr: &Instruct<'_>) -> Result<()> {
    fn print_call(w: &mut dyn Write, call: &InstructCall<'_>) -> Result<()> {
        use InstructCall as I;
        match call {
            I::NewObj => w.write_all(b"NewObj"),
            I::NewObjR => w.write_all(b"NewObjR"),
            I::NewObjD(cid) => {
                w.write_all(b"NewObjD ")?;
                print_class_id(w, cid)
            }
            I::NewObjRD(cid) => {
                w.write_all(b"NewObjRD ")?;
                print_class_id(w, cid)
            }
            I::NewObjS(r) => {
                w.write_all(b"NewObjS ")?;
                print_special_cls_ref(w, r)
            }
            I::FCall(fcall_args) => {
                w.write_all(b"FCall ")?;
                print_fcall_args(w, &fcall_args)?;
                w.write_all(br#" "" """#)
            }
            I::FCallClsMethod(fcall_args, is_log_as_dynamic_call) => {
                w.write_all(b"FCallClsMethod ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                w.write_all(match is_log_as_dynamic_call {
                    IsLogAsDynamicCallOp::LogAsDynamicCall => b"LogAsDynamicCall",
                    IsLogAsDynamicCallOp::DontLogAsDynamicCall => b"DontLogAsDynamicCall",
                })
            }
            I::FCallClsMethodD(fcall_args, cid, mid) => {
                w.write_all(b"FCallClsMethodD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                print_class_id(w, cid)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            I::FCallClsMethodS(fcall_args, r) => {
                w.write_all(b"FCallClsMethodS ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                print_special_cls_ref(w, r)
            }
            I::FCallClsMethodSD(fcall_args, r, mid) => {
                w.write_all(b"FCallClsMethodSD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                print_special_cls_ref(w, r)?;
                w.write_all(b" ")?;
                print_method_id(w, mid)
            }
            I::FCallCtor(fcall_args) => {
                w.write_all(b"FCallCtor ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" """#)
            }
            I::FCallFunc(fcall_args) => {
                w.write_all(b"FCallFunc ")?;
                print_fcall_args(w, fcall_args)
            }
            I::FCallFuncD(fcall_args, id) => {
                w.write_all(b"FCallFuncD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(b" ")?;
                print_function_id(w, id)
            }
            I::FCallObjMethod(fcall_args, nf) => {
                w.write_all(b"FCallObjMethod ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                print_null_flavor(w, nf)
            }
            I::FCallObjMethodD(fcall_args, nf, id) => {
                w.write_all(b"FCallObjMethodD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write_all(br#" "" "#)?;
                print_null_flavor(w, nf)?;
                w.write_all(b" ")?;
                print_method_id(w, id)
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

    use Instruct::*;
    use InstructBasic as IB;
    match instr {
        IIterator(i) => print_iterator(w, i),
        IBasic(b) => w.write_all(match b {
            IB::Nop => b"Nop",
            IB::EntryNop => b"EntryNop",
            IB::PopC => b"PopC",
            IB::PopU => b"PopU",
            IB::Dup => b"Dup",
        }),
        ILitConst(lit) => print_lit_const(w, lit),
        IOp(op) => print_op(w, op),
        IContFlow(cf) => print_control_flow(w, cf),
        ICall(c) => print_call(w, c),
        IMisc(misc) => print_misc(w, misc),
        IGet(get) => print_get(w, get),
        IMutator(mutator) => print_mutator(w, mutator),
        ILabel(l) => {
            print_label(w, l)?;
            w.write_all(b":")
        }
        IIsset(i) => print_isset(w, i),
        IBase(i) => print_base(w, i),
        IFinal(i) => print_final(w, i),
        ITry(itry) => print_try(w, itry),
        IComment(s) => concat_str_by(w, " ", ["#", s.unsafe_as_str()]),
        ISrcLoc(p) => write!(
            w,
            ".srcloc {}:{},{}:{};",
            p.line_begin, p.col_begin, p.line_end, p.col_end
        ),
        IAsync(a) => print_async(w, a),
        IGenerator(gen) => print_gen_creation_execution(w, gen),
        IIncludeEvalDefine(ed) => print_include_eval_define(w, ed),
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
    w.write_all(&si.to_string().as_bytes())
}

fn print_member_opmode(w: &mut dyn Write, m: &MemberOpMode) -> Result<()> {
    use MemberOpMode as M;
    w.write_all(match m {
        M::ModeNone => b"None",
        M::Warn => b"Warn",
        M::Define => b"Define",
        M::Unset => b"Unset",
        M::InOut => b"InOut",
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
            quotes(w, |w| w.write_all(escape(s.unsafe_as_str()).as_bytes()))?;
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

fn print_iterator(w: &mut dyn Write, i: &InstructIterator<'_>) -> Result<()> {
    use InstructIterator as I;
    match i {
        I::IterInit(iter_args, label) => {
            w.write_all(b"IterInit ")?;
            print_iter_args(w, iter_args)?;
            w.write_all(b" ")?;
            print_label(w, label)
        }
        I::IterNext(iter_args, label) => {
            w.write_all(b"IterNext ")?;
            print_iter_args(w, iter_args)?;
            w.write_all(b" ")?;
            print_label(w, label)
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
            print_local(w, &k)?;
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

fn print_query_op(w: &mut dyn Write, q: QueryOp) -> Result<()> {
    w.write_all(match q {
        QueryOp::CGet => b"CGet",
        QueryOp::CGetQuiet => b"CGetQuiet",
        QueryOp::Isset => b"Isset",
        QueryOp::InOut => b"InOut",
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
            print_eq_op(w, &op)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::IncDecM(i, op, mk) => {
            w.write_all(b"IncDecM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_incdec_op(w, &op)?;
            w.write_all(b" ")?;
            print_member_key(w, mk)
        }
        F::SetRangeM(i, s, op) => {
            w.write_all(b"SetRangeM ")?;
            print_int(w, i)?;
            w.write_all(b" ")?;
            print_int(w, &(*s as usize))?;
            w.write_all(b" ")?;
            w.write_all(match op {
                SetrangeOp::Forward => b"Forward",
                SetrangeOp::Reverse => b"Reverse",
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

fn print_istype_op(w: &mut dyn Write, op: &IstypeOp) -> Result<()> {
    use IstypeOp as Op;
    match op {
        Op::OpNull => w.write_all(b"Null"),
        Op::OpBool => w.write_all(b"Bool"),
        Op::OpInt => w.write_all(b"Int"),
        Op::OpDbl => w.write_all(b"Dbl"),
        Op::OpStr => w.write_all(b"Str"),
        Op::OpObj => w.write_all(b"Obj"),
        Op::OpRes => w.write_all(b"Res"),
        Op::OpScalar => w.write_all(b"Scalar"),
        Op::OpKeyset => w.write_all(b"Keyset"),
        Op::OpDict => w.write_all(b"Dict"),
        Op::OpVec => w.write_all(b"Vec"),
        Op::OpArrLike => w.write_all(b"ArrLike"),
        Op::OpLegacyArrLike => w.write_all(b"LegacyArrLike"),
        Op::OpClsMeth => w.write_all(b"ClsMeth"),
        Op::OpFunc => w.write_all(b"Func"),
        Op::OpClass => w.write_all(b"Class"),
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
            match op {
                InitpropOp::Static => w.write_all(b"Static"),
                InitpropOp::NonStatic => w.write_all(b"NonStatic"),
            }?;
            w.write_all(b" ")
        }
    }
}

fn print_eq_op(w: &mut dyn Write, op: &EqOp) -> Result<()> {
    w.write_all(match op {
        EqOp::PlusEqual => b"PlusEqual",
        EqOp::MinusEqual => b"MinusEqual",
        EqOp::MulEqual => b"MulEqual",
        EqOp::ConcatEqual => b"ConcatEqual",
        EqOp::DivEqual => b"DivEqual",
        EqOp::PowEqual => b"PowEqual",
        EqOp::ModEqual => b"ModEqual",
        EqOp::AndEqual => b"AndEqual",
        EqOp::OrEqual => b"OrEqual",
        EqOp::XorEqual => b"XorEqual",
        EqOp::SlEqual => b"SlEqual",
        EqOp::SrEqual => b"SrEqual",
        EqOp::PlusEqualO => b"PlusEqualO",
        EqOp::MinusEqualO => b"MinusEqualO",
        EqOp::MulEqualO => b"MulEqualO",
    })
}

fn print_readonly_op(w: &mut dyn Write, op: &ReadonlyOp) -> Result<()> {
    w.write_all(match op {
        ReadonlyOp::Readonly => b"Readonly",
        ReadonlyOp::Mutable => b"Mutable",
        ReadonlyOp::Any => b"Any",
        ReadonlyOp::CheckROCOW => b"CheckROCOW",
        ReadonlyOp::CheckMutROCOW => b"CheckMutROCOW",
    })
}

fn print_incdec_op(w: &mut dyn Write, op: &IncdecOp) -> Result<()> {
    w.write_all(match op {
        IncdecOp::PreInc => b"PreInc",
        IncdecOp::PostInc => b"PostInc",
        IncdecOp::PreDec => b"PreDec",
        IncdecOp::PostDec => b"PostDec",
        IncdecOp::PreIncO => b"PreIncO",
        IncdecOp::PostIncO => b"PostIncO",
        IncdecOp::PreDecO => b"PreDecO",
        IncdecOp::PostDecO => b"PostDecO",
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
        G::ContCheck(CheckStarted::IgnoreStarted) => w.write_all(b"ContCheck IgnoreStarted"),
        G::ContCheck(CheckStarted::CheckStarted) => w.write_all(b"ContCheck CheckStarted"),
        G::ContValid => w.write_all(b"ContValid"),
        G::ContKey => w.write_all(b"ContKey"),
        G::ContGetReturn => w.write_all(b"ContGetReturn"),
        G::ContCurrent => w.write_all(b"ContCurrent"),
    }
}

fn print_misc(w: &mut dyn Write, misc: &InstructMisc<'_>) -> Result<()> {
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
            match op {
                OpSilence::Start => w.write_all(b"Start"),
                OpSilence::End => w.write_all(b"End"),
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
                match op {
                    BareThisOp::Notice => "Notice",
                    BareThisOp::NoNotice => "NoNotice",
                    BareThisOp::NeverNull => "NeverNull",
                },
            ],
        ),

        M::MemoGet(label, Just(Pair(Local::Unnamed(first), local_count))) => {
            w.write_all(b"MemoGet ")?;
            print_label(w, label)?;
            write!(w, " L:{}+{}", first, local_count)
        }
        M::MemoGet(label, Nothing) => {
            w.write_all(b"MemoGet ")?;
            print_label(w, label)?;
            w.write_all(b" L:0+0")
        }
        M::MemoGet(_, _) => Err(Error::fail("MemoGet needs an unnamed local").into()),

        M::MemoSet(Just(Pair(Local::Unnamed(first), local_count))) => {
            write!(w, "MemoSet L:{}+{}", first, local_count)
        }
        M::MemoSet(Nothing) => w.write_all(b"MemoSet L:0+0"),
        M::MemoSet(_) => Err(Error::fail("MemoSet needs an unnamed local").into()),

        M::MemoGetEager(label1, label2, Just(Pair(Local::Unnamed(first), local_count))) => {
            w.write_all(b"MemoGetEager ")?;
            print_label(w, label1)?;
            w.write_all(b" ")?;
            print_label(w, label2)?;
            write!(w, " L:{}+{}", first, local_count)
        }
        M::MemoGetEager(label1, label2, Nothing) => {
            w.write_all(b"MemoGetEager ")?;
            print_label(w, label1)?;
            w.write_all(b" ")?;
            print_label(w, label2)?;
            w.write_all(b" L:0+0")
        }
        M::MemoGetEager(_, _, _) => Err(Error::fail("MemoGetEager needs an unnamed local").into()),

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
            w.write_all(s.unsafe_as_str().as_bytes())
        }
        M::AssertRATStk(n, s) => concat_str_by(
            w,
            " ",
            ["AssertRATStk", n.to_string().as_str(), s.unsafe_as_str()],
        ),
        M::GetMemoKeyL(local) => {
            w.write_all(b"GetMemoKeyL ")?;
            print_local(w, local)
        }
    }
}

fn print_include_eval_define(w: &mut dyn Write, ed: &InstructIncludeEvalDefine) -> Result<()> {
    use InstructIncludeEvalDefine::*;
    match ed {
        Incl => w.write_all(b"Incl"),
        InclOnce => w.write_all(b"InclOnce"),
        Req => w.write_all(b"Req"),
        ReqOnce => w.write_all(b"ReqOnce"),
        ReqDoc => w.write_all(b"ReqDoc"),
        Eval => w.write_all(b"Eval"),
    }
}

fn print_control_flow(w: &mut dyn Write, cf: &InstructControlFlow<'_>) -> Result<()> {
    use InstructControlFlow as CF;
    match cf {
        CF::Jmp(l) => {
            w.write_all(b"Jmp ")?;
            print_label(w, l)
        }
        CF::JmpNS(l) => {
            w.write_all(b"JmpNS ")?;
            print_label(w, l)
        }
        CF::JmpZ(l) => {
            w.write_all(b"JmpZ ")?;
            print_label(w, l)
        }
        CF::JmpNZ(l) => {
            w.write_all(b"JmpNZ ")?;
            print_label(w, l)
        }
        CF::RetC => w.write_all(b"RetC"),
        CF::RetCSuspended => w.write_all(b"RetCSuspended"),
        CF::RetM(p) => concat_str_by(w, " ", ["RetM", p.to_string().as_str()]),
        CF::Throw => w.write_all(b"Throw"),
        CF::Switch(kind, base, labels) => print_switch(w, kind, base, labels.as_ref()),
        CF::SSwitch(cases) => match cases.as_ref() {
            [] => Err(Error::fail("sswitch should have at least one case").into()),
            [rest @ .., Pair(_, lastlabel)] => {
                w.write_all(b"SSwitch ")?;
                angle(w, |w| {
                    concat_by(w, " ", rest, |w, Pair(s, l)| {
                        concat_str(w, [quote_string(s.unsafe_as_str()).as_str(), ":"])?;
                        print_label(w, l)
                    })?;
                    w.write_all(b" -:")?;
                    print_label(w, &lastlabel)
                })
            }
        },
    }
}

fn print_switch(
    w: &mut dyn Write,
    kind: &Switchkind,
    base: &isize,
    labels: &[Label],
) -> Result<()> {
    w.write_all(b"Switch ")?;
    w.write_all(match kind {
        Switchkind::Bounded => b"Bounded ",
        Switchkind::Unbounded => b"Unbounded ",
    })?;
    w.write_all(base.to_string().as_bytes())?;
    w.write_all(b" ")?;
    angle(w, |w| concat_by(w, " ", labels, print_label))
}

fn print_lit_const(w: &mut dyn Write, lit: &InstructLitConst<'_>) -> Result<()> {
    use InstructLitConst as LC;
    match lit {
        LC::Null => w.write_all(b"Null"),
        LC::Int(i) => concat_str_by(w, " ", ["Int", i.to_string().as_str()]),
        LC::String(s) => {
            w.write_all(b"String ")?;
            quotes(w, |w| w.write_all(escape(s.unsafe_as_str()).as_bytes()))
        }
        LC::LazyClass(id) => {
            w.write_all(b"LazyClass ")?;
            print_class_id(w, id)
        }
        LC::True => w.write_all(b"True"),
        LC::False => w.write_all(b"False"),
        LC::Double(d) => concat_str_by(w, " ", ["Double", d.unsafe_as_str()]),
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
            let ls: Vec<&str> = l.as_ref().into_iter().map(|s| s.unsafe_as_str()).collect();
            w.write_all(b"NewStructDict ")?;
            angle(w, |w| print_shape_fields(w, &ls[0..]))
        }
        LC::NewRecord(cid, l) => {
            let ls: Vec<&str> = l.as_ref().into_iter().map(|s| s.unsafe_as_str()).collect();
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
    match ct {
        CT::Vector => w.write_all(b"Vector"),
        CT::Map => w.write_all(b"Map"),
        CT::Set => w.write_all(b"Set"),
        CT::Pair => w.write_all(b"Pair"),
        CT::ImmVector => w.write_all(b"ImmVector"),
        CT::ImmMap => w.write_all(b"ImmMap"),
        CT::ImmSet => w.write_all(b"ImmSet"),
        CT::Dict => w.write_all(b"dict"),
        CT::Array => w.write_all(b"array"),
        CT::Keyset => w.write_all(b"keyset"),
        CT::Vec => w.write_all(b"vec"),
    }
}

fn print_shape_fields(w: &mut dyn Write, sf: &[&str]) -> Result<()> {
    concat_by(w, " ", sf, |w, f| {
        quotes(w, |w| w.write_all(escape(*f).as_bytes()))
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
                match op {
                    TypestructResolveOp::Resolve => "Resolve",
                    TypestructResolveOp::DontResolve => "DontResolve",
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
    match f {
        FatalOp::Parse => w.write_all(b"Fatal Parse"),
        FatalOp::Runtime => w.write_all(b"Fatal Runtime"),
        FatalOp::RuntimeOmitFrame => w.write_all(b"Fatal RuntimeOmitFrame"),
    }
}

fn print_params<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    params: impl AsRef<[HhasParam<'arena>]>,
) -> Result<()> {
    paren(w, |w| {
        concat_by(w, ", ", params, |w, i| print_param(ctx, w, i))
    })
}

fn print_param<'arena>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    param: &HhasParam<'arena>,
) -> Result<()> {
    print_param_user_attributes(ctx, w, param)?;
    write_if!(param.is_inout, w, "inout ")?;
    write_if!(param.is_readonly, w, "readonly ")?;
    write_if!(param.is_variadic, w, "...")?;
    option(w, &Option::from(param.type_info.clone()), |w, ty| {
        print_type_info(w, ty)?;
        w.write_all(b" ")
    })?;
    w.write_all(param.name.unsafe_as_str().as_bytes())?;
    option(
        w,
        &Option::from(param.default_value.map(|x| (x.0, x.1))),
        |w, i| print_param_default_value(w, i),
    )
}

fn print_param_id(w: &mut dyn Write, param_id: &ParamId<'_>) -> Result<()> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write_all(i.to_string().as_bytes()),
        ParamId::ParamNamed(s) => w.write_all(s.unsafe_as_str().as_bytes()),
    }
}

fn print_param_default_value<'arena>(
    w: &mut dyn Write,
    default_val: &(Label, Str<'arena>),
) -> Result<()> {
    w.write_all(b" = ")?;
    print_label(w, &default_val.0)?;
    paren(w, |w| {
        triple_quotes(w, |w| w.write_all(default_val.1.unsafe_as_str().as_bytes()))
    })
}

fn print_label(w: &mut dyn Write, label: &Label) -> Result<()> {
    match label {
        Label::Regular(id) => {
            w.write_all(b"L")?;
            print_int(w, id)
        }
        Label::DefaultArg(id) => {
            w.write_all(b"DV")?;
            print_int(w, id)
        }
    }
}

fn print_local(w: &mut dyn Write, local: &Local<'_>) -> Result<()> {
    match local {
        Local::Unnamed(id) => {
            w.write_all(b"_")?;
            print_int(w, id)
        }
        Local::Named(id) => w.write_all(id.unsafe_as_str().as_bytes()),
    }
}

fn print_int(w: &mut dyn Write, i: &usize) -> Result<()> {
    write!(w, "{}", i)
}

fn print_key_value(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    k: &ast::Expr,
    v: &ast::Expr,
) -> Result<()> {
    print_key_value_(ctx, w, env, k, print_expr, v)
}

fn print_key_value_<K, KeyPrinter>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    k: K,
    mut kp: KeyPrinter,
    v: &ast::Expr,
) -> Result<()>
where
    KeyPrinter: FnMut(&Context<'_>, &mut dyn Write, &ExprEnv<'_, '_>, K) -> Result<()>,
{
    kp(ctx, w, env, k)?;
    w.write_all(b" => ")?;
    print_expr(ctx, w, env, v)
}

fn print_afield(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    afield: &ast::Afield,
) -> Result<()> {
    use ast::Afield as A;
    match afield {
        A::AFvalue(e) => print_expr(ctx, w, env, &e),
        A::AFkvalue(k, v) => print_key_value(ctx, w, env, &k, &v),
    }
}

fn print_afields(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    afields: impl AsRef<[ast::Afield]>,
) -> Result<()> {
    concat_by(w, ", ", afields, |w, i| print_afield(ctx, w, env, i))
}

fn print_uop(w: &mut dyn Write, op: ast::Uop) -> Result<()> {
    use ast::Uop as U;
    w.write_all(match op {
        U::Utild => b"~",
        U::Unot => b"!",
        U::Uplus => b"+",
        U::Uminus => b"-",
        U::Uincr => b"++",
        U::Udecr => b"--",
        U::Usilence => b"@",
        U::Upincr | U::Updecr => {
            return Err(Error::fail("string_of_uop - should have been captures earlier").into());
        }
    })
}

fn print_key_values(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    kvs: impl AsRef<[(ast::Expr, ast::Expr)]>,
) -> Result<()> {
    print_key_values_(ctx, w, env, print_expr, kvs)
}

fn print_key_values_<K, KeyPrinter>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    mut kp: KeyPrinter,
    kvs: impl AsRef<[(K, ast::Expr)]>,
) -> Result<()>
where
    KeyPrinter: Fn(&Context<'_>, &mut dyn Write, &ExprEnv<'_, '_>, &K) -> Result<()>,
{
    concat_by(w, ", ", kvs, |w, (k, v)| {
        print_key_value_(ctx, w, env, k, &mut kp, v)
    })
}

fn print_expr_darray<K, KeyPrinter>(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    kp: KeyPrinter,
    kvs: impl AsRef<[(K, ast::Expr)]>,
) -> Result<()>
where
    KeyPrinter: Fn(&Context<'_>, &mut dyn Write, &ExprEnv<'_, '_>, &K) -> Result<()>,
{
    wrap_by_(w, "darray[", "]", |w| {
        print_key_values_(ctx, w, env, kp, kvs)
    })
}

fn print_expr_varray(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    varray: &[ast::Expr],
) -> Result<()> {
    wrap_by_(w, "varray[", "]", |w| {
        concat_by(w, ", ", varray, |w, e| print_expr(ctx, w, env, e))
    })
}

fn print_shape_field_name(
    _: &Context<'_>,
    w: &mut dyn Write,
    _: &ExprEnv<'_, '_>,
    field: &ast::ShapeFieldName,
) -> Result<()> {
    use ast::ShapeFieldName as S;
    match field {
        S::SFlitInt((_, s)) => print_expr_int(w, s.as_ref()),
        S::SFlitStr((_, s)) => print_expr_string(w, s),
        S::SFclassConst(_, (_, s)) => print_expr_string(w, s.as_bytes()),
    }
}

fn print_expr_int(w: &mut dyn Write, i: &str) -> Result<()> {
    match integer::to_decimal(i) {
        Ok(s) => w.write_all(s.as_bytes()),
        Err(_) => Err(Error::fail("ParseIntError").into()),
    }
}

fn print_expr_string(w: &mut dyn Write, s: &[u8]) -> Result<()> {
    fn escape_char(c: u8) -> Option<Cow<'static, [u8]>> {
        match c {
            b'\n' => Some((&b"\\\\n"[..]).into()),
            b'\r' => Some((&b"\\\\r"[..]).into()),
            b'\t' => Some((&b"\\\\t"[..]).into()),
            b'\\' => Some((&b"\\\\\\\\"[..]).into()),
            b'"' => Some((&b"\\\\\\\""[..]).into()),
            b'$' => Some((&b"\\\\$"[..]).into()),
            c if is_lit_printable(c) => None,
            c => {
                let mut r = vec![];
                write!(r, "\\\\{:03o}", c).unwrap();
                Some(r.into())
            }
        }
    }
    // FIXME: This is not safe--string literals are binary strings.
    // There's no guarantee that they're valid UTF-8.
    let s = unsafe { std::str::from_utf8_unchecked(s) };
    wrap_by(w, "\\\"", |w| {
        w.write_all(escape_by(s.into(), escape_char).as_bytes())
    })
}

fn print_expr_to_string(
    ctx: &Context<'_>,
    env: &ExprEnv<'_, '_>,
    expr: &ast::Expr,
) -> Result<String> {
    let mut buf = Vec::new();
    print_expr(ctx, &mut buf, env, expr).map_err(|e| match write::into_error(e) {
        Error::NotImpl(m) => Error::NotImpl(m),
        e => Error::Fail(format!("Failed: {}", e)),
    })?;
    Ok(unsafe { String::from_utf8_unchecked(buf) })
}

fn print_expr(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    ast::Expr(_, _, expr): &ast::Expr,
) -> Result<()> {
    fn adjust_id<'a>(env: &ExprEnv<'_, '_>, id: &'a str) -> Cow<'a, str> {
        let s: Cow<'a, str> = match env.codegen_env {
            Some(env) => {
                if env.is_namespaced
                    && id
                        .as_bytes()
                        .iter()
                        .rposition(|c| *c == b'\\')
                        .map_or(true, |i| i < 1)
                {
                    strip_global_ns(id).into()
                } else {
                    add_ns(id)
                }
            }
            _ => id.into(),
        };
        escaper::escape(s)
    }
    fn print_expr_id<'a>(w: &mut dyn Write, env: &ExprEnv<'_, '_>, s: &'a str) -> Result<()> {
        w.write_all(adjust_id(env, s).as_bytes())
    }
    fn fmt_class_name<'a>(is_class_constant: bool, id: Cow<'a, str>) -> Cow<'a, str> {
        let cn: Cow<'a, str> = if is_xhp(strip_global_ns(&id)) {
            escaper::escape(strip_global_ns(&mangle(id.into())))
                .to_string()
                .into()
        } else {
            escaper::escape(strip_global_ns(&id)).to_string().into()
        };
        if is_class_constant {
            format!("\\\\{}", cn).into()
        } else {
            cn
        }
    }
    fn get_class_name_from_id<'e>(
        ctx: &Context<'_>,
        env: Option<&'e HhasBodyEnv<'_>>,
        should_format: bool,
        is_class_constant: bool,
        id: &'e str,
    ) -> Cow<'e, str> {
        if id == classes::SELF || id == classes::PARENT || id == classes::STATIC {
            let name = ctx.special_class_resolver.resolve(env, id);
            return fmt_class_name(is_class_constant, name);
        }
        fn get<'a>(should_format: bool, is_class_constant: bool, id: &'a str) -> Cow<'a, str> {
            if should_format {
                fmt_class_name(is_class_constant, id.into())
            } else {
                id.into()
            }
        }

        if env.is_some() {
            let alloc = bumpalo::Bump::new();
            let class_id = ClassType::from_ast_name(&alloc, id);
            let id = class_id.to_raw_string();
            get(should_format, is_class_constant, id)
                .into_owned()
                .into()
        } else {
            get(should_format, is_class_constant, id)
        }
    }
    fn handle_possible_colon_colon_class_expr(
        ctx: &Context<'_>,
        w: &mut dyn Write,
        env: &ExprEnv<'_, '_>,
        is_array_get: bool,
        e_: &ast::Expr_,
    ) -> Result<Option<()>> {
        match e_.as_class_const() {
            Some((
                ast::ClassId(_, _, ast::ClassId_::CIexpr(ast::Expr(_, _, ast::Expr_::Id(id)))),
                (_, s2),
            )) if is_class(&s2) && !(is_self(&id.1) || is_parent(&id.1) || is_static(&id.1)) => {
                Ok(Some({
                    let s1 = get_class_name_from_id(ctx, env.codegen_env, false, false, &id.1);
                    if is_array_get {
                        print_expr_id(w, env, s1.as_ref())?
                    } else {
                        print_expr_string(w, s1.as_bytes())?
                    }
                }))
            }
            _ => Ok(None),
        }
    }
    use ast::Expr_ as E_;
    match expr {
        E_::Id(id) => print_expr_id(w, env, id.1.as_ref()),
        E_::Lvar(lid) => w.write_all(escape(&(lid.1).1).as_bytes()),
        E_::Float(f) => {
            if f.contains('E') || f.contains('e') {
                let s = format!(
                    "{:.1E}",
                    f.parse::<f64>()
                        .map_err(|_| Error::fail(format!("ParseFloatError: {}", f)))?
                )
                // to_uppercase() here because s might be "inf" or "nan"
                .to_uppercase();

                lazy_static! {
                    static ref UNSIGNED_EXP: Regex =
                        Regex::new(r"(?P<first>E)(?P<second>\d+)").unwrap();
                    static ref SIGNED_SINGLE_DIGIT_EXP: Regex =
                        Regex::new(r"(?P<first>E[+-])(?P<second>\d$)").unwrap();
                }
                // turn mEn into mE+n
                let s = UNSIGNED_EXP.replace(&s, "${first}+${second}");
                // turn mE+n or mE-n into mE+0n or mE-0n (where n is a single digit)
                let s = SIGNED_SINGLE_DIGIT_EXP.replace(&s, "${first}0${second}");
                w.write_all(s.as_bytes())
            } else {
                w.write_all(f.as_bytes())
            }
        }
        E_::Int(i) => print_expr_int(w, i.as_ref()),
        E_::String(s) => print_expr_string(w, s),
        E_::Null => w.write_all(b"NULL"),
        E_::True => w.write_all(b"true"),
        E_::False => w.write_all(b"false"),
        // For arrays and collections, we are making a conscious decision to not
        // match HHMV has HHVM's emitter has inconsistencies in the pretty printer
        // https://fburl.com/tzom2qoe
        E_::Collection(c) if (c.0).1 == "vec" || (c.0).1 == "dict" || (c.0).1 == "keyset" => {
            w.write_all((c.0).1.as_bytes())?;
            square(w, |w| print_afields(ctx, w, env, &c.2))
        }
        E_::Collection(c) => {
            let name = strip_ns((c.0).1.as_str());
            let name = types::fix_casing(&name);
            match name {
                "Set" | "Pair" | "Vector" | "Map" | "ImmSet" | "ImmVector" | "ImmMap" => {
                    w.write_all(b"HH\\\\")?;
                    w.write_all(name.as_bytes())?;
                    wrap_by_(w, " {", "}", |w| {
                        Ok(if !c.2.is_empty() {
                            w.write_all(b" ")?;
                            print_afields(ctx, w, env, &c.2)?;
                            w.write_all(b" ")?;
                        })
                    })
                }
                _ => Err(
                    Error::fail(format!("Default value for an unknow collection - {}", name))
                        .into(),
                ),
            }
        }
        E_::Shape(fl) => print_expr_darray(ctx, w, env, print_shape_field_name, fl),
        E_::Binop(x) => {
            let (bop, e1, e2) = &**x;
            print_expr(ctx, w, env, e1)?;
            w.write_all(b" ")?;
            print_bop(w, bop)?;
            w.write_all(b" ")?;
            print_expr(ctx, w, env, e2)
        }
        E_::Call(c) => {
            let (e, _, es, unpacked_element) = &**c;
            match e.as_id() {
                Some(ast_defs::Id(_, call_id)) => {
                    w.write_all(lstrip(adjust_id(env, &call_id).as_ref(), "\\\\").as_bytes())?
                }
                None => {
                    let buf = print_expr_to_string(ctx, env, e)?;
                    w.write_all(lstrip(&buf, "\\\\").as_bytes())?
                }
            };
            paren(w, |w| {
                concat_by(w, ", ", &es, |w, (pk, e)| match pk {
                    ParamKind::Pnormal => print_expr(ctx, w, env, e),
                    ParamKind::Pinout(_) => Err(Error::fail("illegal default value").into()),
                })?;
                match unpacked_element {
                    None => Ok(()),
                    Some(e) => {
                        if !es.is_empty() {
                            w.write_all(b", ")?;
                        }
                        // TODO: Should probably have ... also but we are not doing that in ocaml
                        print_expr(ctx, w, env, e)
                    }
                }
            })
        }
        E_::New(x) => {
            let (cid, _, es, unpacked_element, _) = &**x;
            match cid.2.as_ciexpr() {
                Some(ci_expr) => {
                    w.write_all(b"new ")?;
                    match ci_expr.2.as_id() {
                        Some(ast_defs::Id(_, cname)) => w.write_all(
                            lstrip(
                                &adjust_id(
                                    env,
                                    &ClassType::from_ast_name(&bumpalo::Bump::new(), cname)
                                        .to_raw_string(),
                                ),
                                "\\\\",
                            )
                            .as_bytes(),
                        )?,
                        None => {
                            let buf = print_expr_to_string(ctx, env, ci_expr)?;
                            w.write_all(lstrip(&buf, "\\\\").as_bytes())?
                        }
                    }
                    paren(w, |w| {
                        concat_by(w, ", ", es, |w, e| print_expr(ctx, w, env, e))?;
                        match unpacked_element {
                            None => Ok(()),
                            Some(e) => {
                                w.write_all(b", ")?;
                                print_expr(ctx, w, env, e)
                            }
                        }
                    })
                }
                None => {
                    match cid.2.as_ci() {
                        Some(id) => {
                            // Xml exprs rewritten as New exprs come
                            // through here.
                            print_xml(ctx, w, env, &id.1, es)
                        }
                        None => not_impl!(),
                    }
                }
            }
        }
        E_::Record(r) => {
            w.write_all(lstrip(adjust_id(env, &(r.0).1).as_ref(), "\\\\").as_bytes())?;
            print_key_values(ctx, w, env, &r.1)
        }
        E_::ClassGet(cg) => {
            match &(cg.0).2 {
                ast::ClassId_::CIexpr(e) => match e.as_id() {
                    Some(id) => w.write_all(
                        get_class_name_from_id(
                            ctx,
                            env.codegen_env,
                            true,  /* should_format */
                            false, /* is_class_constant */
                            &id.1,
                        )
                        .as_bytes(),
                    )?,
                    _ => print_expr(ctx, w, env, e)?,
                },
                _ => return Err(Error::fail("TODO Unimplemented unexpected non-CIexpr").into()),
            }
            w.write_all(b"::")?;
            match &cg.1 {
                ast::ClassGetExpr::CGstring((_, litstr)) => w.write_all(escape(litstr).as_bytes()),
                ast::ClassGetExpr::CGexpr(e) => print_expr(ctx, w, env, e),
            }
        }
        E_::ClassConst(cc) => {
            if let Some(e1) = (cc.0).2.as_ciexpr() {
                handle_possible_colon_colon_class_expr(ctx, w, env, false, expr)?.map_or_else(
                    || {
                        let s2 = &(cc.1).1;
                        match e1.2.as_id() {
                            Some(ast_defs::Id(_, s1)) => {
                                let s1 =
                                    get_class_name_from_id(ctx, env.codegen_env, true, true, s1);
                                concat_str_by(w, "::", [&s1.into(), s2])
                            }
                            _ => {
                                print_expr(ctx, w, env, e1)?;
                                w.write_all(b"::")?;
                                w.write_all(s2.as_bytes())
                            }
                        }
                    },
                    Ok,
                )
            } else {
                Err(Error::fail("TODO: Only expected CIexpr in class_const").into())
            }
        }
        E_::Unop(u) => match u.0 {
            ast::Uop::Upincr => {
                print_expr(ctx, w, env, &u.1)?;
                w.write_all(b"++")
            }
            ast::Uop::Updecr => {
                print_expr(ctx, w, env, &u.1)?;
                w.write_all(b"--")
            }
            _ => {
                print_uop(w, u.0)?;
                print_expr(ctx, w, env, &u.1)
            }
        },
        E_::ObjGet(og) => {
            print_expr(ctx, w, env, &og.0)?;
            w.write_all(match og.2 {
                ast::OgNullFlavor::OGNullthrows => b"->",
                ast::OgNullFlavor::OGNullsafe => b"?->",
            })?;
            print_expr(ctx, w, env, &og.1)
        }
        E_::Clone(e) => {
            w.write_all(b"clone ")?;
            print_expr(ctx, w, env, e)
        }
        E_::ArrayGet(ag) => {
            print_expr(ctx, w, env, &ag.0)?;
            square(w, |w| {
                option(w, &ag.1, |w, e: &ast::Expr| {
                    handle_possible_colon_colon_class_expr(ctx, w, env, true, &e.2)
                        .transpose()
                        .unwrap_or_else(|| print_expr(ctx, w, env, e))
                })
            })
        }
        E_::String2(ss) => concat_by(w, " . ", ss, |w, s| print_expr(ctx, w, env, s)),
        E_::PrefixedString(s) => {
            w.write_all(s.0.as_bytes())?;
            w.write_all(b" . ")?;
            print_expr(ctx, w, env, &s.1)
        }
        E_::Eif(eif) => {
            print_expr(ctx, w, env, &eif.0)?;
            w.write_all(b" ? ")?;
            option(w, &eif.1, |w, etrue| print_expr(ctx, w, env, etrue))?;
            w.write_all(b" : ")?;
            print_expr(ctx, w, env, &eif.2)
        }
        E_::Cast(c) => {
            paren(w, |w| print_hint(w, false, &c.0))?;
            print_expr(ctx, w, env, &c.1)
        }
        E_::Pipe(p) => {
            print_expr(ctx, w, env, &p.1)?;
            w.write_all(b" |> ")?;
            print_expr(ctx, w, env, &p.2)
        }
        E_::Is(i) => {
            print_expr(ctx, w, env, &i.0)?;
            w.write_all(b" is ")?;
            print_hint(w, true, &i.1)
        }
        E_::As(a) => {
            print_expr(ctx, w, env, &a.0)?;
            w.write_all(if a.2 { b" ?as " } else { b" as " })?;
            print_hint(w, true, &a.1)
        }
        E_::Varray(va) => print_expr_varray(ctx, w, env, &va.1),
        E_::Darray(da) => print_expr_darray(ctx, w, env, print_expr, &da.1),
        E_::Tuple(t) => wrap_by_(w, "varray[", "]", |w| {
            // A tuple is represented by a varray when using reflection.
            concat_by(w, ", ", t, |w, i| print_expr(ctx, w, env, i))
        }),
        E_::List(l) => wrap_by_(w, "list(", ")", |w| {
            concat_by(w, ", ", l, |w, i| print_expr(ctx, w, env, i))
        }),
        E_::Yield(y) => {
            w.write_all(b"yield ")?;
            print_afield(ctx, w, env, y)
        }
        E_::Await(e) => {
            w.write_all(b"await ")?;
            print_expr(ctx, w, env, e)
        }
        E_::Import(i) => {
            print_import_flavor(w, &i.0)?;
            w.write_all(b" ")?;
            print_expr(ctx, w, env, &i.1)
        }
        E_::Xml(_) => {
            Err(Error::fail("expected Xml to be converted to New during rewriting").into())
        }
        E_::Efun(f) => print_efun(ctx, w, env, &f.0, &f.1),
        E_::FunctionPointer(fp) => {
            let (fp_id, targs) = &**fp;
            match fp_id {
                ast::FunctionPtrId::FPId(ast::Id(_, sid)) => {
                    w.write_all(lstrip(adjust_id(env, &sid).as_ref(), "\\\\").as_bytes())?
                }
                ast::FunctionPtrId::FPClassConst(ast::ClassId(_, _, class_id), (_, meth_name)) => {
                    match class_id {
                        ast::ClassId_::CIexpr(e) => match e.as_id() {
                            Some(id) => w.write_all(
                                get_class_name_from_id(
                                    ctx,
                                    env.codegen_env,
                                    true,  /* should_format */
                                    false, /* is_class_constant */
                                    &id.1,
                                )
                                .as_bytes(),
                            )?,
                            _ => print_expr(ctx, w, env, e)?,
                        },
                        _ => {
                            return Err(Error::fail(
                                "TODO Unimplemented unexpected non-CIexpr in function pointer",
                            )
                            .into());
                        }
                    }
                    w.write_all(b"::")?;
                    w.write_all(meth_name.as_bytes())?
                }
            };
            wrap_by_(w, "<", ">", |w| {
                concat_by(w, ", ", targs, |w, _targ| w.write_all(b"_"))
            })
        }
        E_::Omitted => Ok(()),
        E_::Lfun(lfun) => {
            if ctx.dump_lambdas {
                let fun_ = &lfun.0;
                paren(w, |w| {
                    paren(w, |w| {
                        concat_by(w, ", ", &fun_.params, |w, param| {
                            print_fparam(ctx, w, env, param)
                        })
                    })?;
                    w.write_all(b" ==> ")?;
                    print_block_(ctx, w, env, &fun_.body.fb_ast)
                })
            } else {
                Err(Error::fail(
                    "expected Lfun to be converted to Efun during closure conversion print_expr",
                )
                .into())
            }
        }
        E_::ETSplice(splice) => {
            w.write_all(b"${")?;
            print_expr(ctx, w, env, splice)?;
            w.write_all(b"}")
        }
        _ => Err(Error::fail(format!("TODO Unimplemented: Cannot print: {:?}", expr)).into()),
    }
}

fn print_xml(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    id: &str,
    es: &[ast::Expr],
) -> Result<()> {
    use ast::{Expr as E, Expr_ as E_};

    fn syntax_error() -> Error {
        Error::NotImpl(String::from("print_xml: unexpected syntax"))
    }
    fn print_xhp_attr(
        ctx: &Context<'_>,
        w: &mut dyn Write,
        env: &ExprEnv<'_, '_>,
        attr: &(ast_defs::ShapeFieldName, ast::Expr),
    ) -> Result<()> {
        match attr {
            (ast_defs::ShapeFieldName::SFlitStr(s), e) => print_key_value_(
                ctx,
                w,
                env,
                &s.1,
                |_, w, _, k| print_expr_string(w, k.as_slice()),
                e,
            ),
            _ => Err(syntax_error().into()),
        }
    }

    let (attrs, children) = if es.len() < 2 {
        return Err(syntax_error().into());
    } else {
        match (&es[0], &es[1]) {
            (E(_, _, E_::Shape(attrs)), E(_, _, E_::Varray(children))) => (attrs, &children.1),
            _ => return Err(syntax_error().into()),
        }
    };
    let env = ExprEnv {
        codegen_env: env.codegen_env,
    };
    write!(w, "new {}", mangle(id.into()))?;
    paren(w, |w| {
        wrap_by_(w, "darray[", "]", |w| {
            concat_by(w, ", ", attrs, |w, attr| print_xhp_attr(ctx, w, &env, attr))
        })?;
        w.write_all(b", ")?;
        print_expr_varray(ctx, w, &env, children)?;
        w.write_all(b", __FILE__, __LINE__")
    })
}

fn print_efun(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    f: &ast::Fun_,
    use_list: &[ast::Lid],
) -> Result<()> {
    write_if!(
        f.fun_kind.is_fasync() || f.fun_kind.is_fasync_generator(),
        w,
        "async ",
    )?;
    w.write_all(b"function ")?;
    paren(w, |w| {
        concat_by(w, ", ", &f.params, |w, p| print_fparam(ctx, w, env, p))
    })?;
    w.write_all(b" ")?;
    if !use_list.is_empty() {
        w.write_all(b"use ")?;
        paren(w, |w| {
            concat_by(w, ", ", use_list, |w: &mut dyn Write, ast::Lid(_, id)| {
                w.write_all(local_id::get_name(id).as_bytes())
            })
        })?;
        w.write_all(b" ")?;
    }
    print_block_(ctx, w, env, &f.body.fb_ast)
}

fn print_block(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    block: &[ast::Stmt],
) -> Result<()> {
    match &block[..] {
        [] | [ast::Stmt(_, ast::Stmt_::Noop)] => Ok(()),
        [ast::Stmt(_, ast::Stmt_::Block(b))] if b.len() == 1 => print_block_(ctx, w, env, b),
        [_, _, ..] => print_block_(ctx, w, env, block),
        [stmt] => print_statement(ctx, w, env, stmt),
    }
}

fn print_block_(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    block: &[ast::Stmt],
) -> Result<()> {
    wrap_by_(w, "{\\n", "}\\n", |w| {
        concat(w, block, |w, stmt| {
            if !matches!(stmt.1, ast::Stmt_::Noop) {
                w.write_all(b"  ")?;
                print_statement(ctx, w, env, stmt)?;
            }
            Ok(())
        })
    })
}

fn print_statement(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    stmt: &ast::Stmt,
) -> Result<()> {
    use ast::Stmt_ as S_;
    match &stmt.1 {
        S_::Return(expr) => wrap_by_(w, "return", ";\\n", |w| {
            option(w, &**expr, |w, e| {
                w.write_all(b" ")?;
                print_expr(ctx, w, env, e)
            })
        }),
        S_::Expr(expr) => {
            print_expr(ctx, w, env, &**expr)?;
            w.write_all(b";\\n")
        }
        S_::Throw(expr) => wrap_by_(w, "throw ", ";\\n", |w| print_expr(ctx, w, env, &**expr)),
        S_::Break => w.write_all(b"break;\\n"),
        S_::Continue => w.write_all(b"continue;\\n"),
        S_::While(x) => {
            let (cond, block) = &**x;
            wrap_by_(w, "while (", ") ", |w| print_expr(ctx, w, env, cond))?;
            print_block(ctx, w, env, block.as_ref())
        }
        S_::If(x) => {
            let (cond, if_block, else_block) = &**x;
            wrap_by_(w, "if (", ") ", |w| print_expr(ctx, w, env, cond))?;
            print_block(ctx, w, env, if_block)?;
            let mut buf = Vec::new();
            print_block(ctx, &mut buf, env, else_block).map_err(|e| {
                match write::into_error(e) {
                    e @ Error::NotImpl(_) => e,
                    e => Error::Fail(format!("Failed: {}", e)),
                }
            })?;
            write_if!(!buf.is_empty(), w, " else ")?;
            write_if!(!buf.is_empty(), w, "{}", unsafe {
                std::str::from_utf8_unchecked(&buf)
            })
        }
        S_::Block(block) => print_block_(ctx, w, env, block),
        S_::Noop => Ok(()),
        /* TODO(T29869930) */
        _ => w.write_all(b"TODO Unimplemented NYI: Default value printing"),
    }
}

fn print_fparam(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    param: &ast::FunParam,
) -> Result<()> {
    if param.callconv.is_pinout() {
        w.write_all(b"inout ")?;
    }
    if param.is_variadic {
        w.write_all(b"...")?;
    }
    option(w, &(param.type_hint).1, |w, h| {
        print_hint(w, true, h)?;
        w.write_all(b" ")
    })?;
    w.write_all(param.name.as_bytes())?;
    option(w, &param.expr, |w, e| {
        w.write_all(b" = ")?;
        print_expr(ctx, w, env, e)
    })
}

fn print_bop(w: &mut dyn Write, bop: &ast_defs::Bop) -> Result<()> {
    use ast_defs::Bop;
    match bop {
        Bop::Plus => w.write_all(b"+"),
        Bop::Minus => w.write_all(b"-"),
        Bop::Star => w.write_all(b"*"),
        Bop::Slash => w.write_all(b"/"),
        Bop::Eqeq => w.write_all(b"=="),
        Bop::Eqeqeq => w.write_all(b"==="),
        Bop::Starstar => w.write_all(b"**"),
        Bop::Eq(None) => w.write_all(b"="),
        Bop::Eq(Some(bop)) => {
            w.write_all(b"=")?;
            print_bop(w, bop)
        }
        Bop::Ampamp => w.write_all(b"&&"),
        Bop::Barbar => w.write_all(b"||"),
        Bop::Lt => w.write_all(b"<"),
        Bop::Lte => w.write_all(b"<="),
        Bop::Cmp => w.write_all(b"<=>"),
        Bop::Gt => w.write_all(b">"),
        Bop::Gte => w.write_all(b">="),
        Bop::Dot => w.write_all(b"."),
        Bop::Amp => w.write_all(b"&"),
        Bop::Bar => w.write_all(b"|"),
        Bop::Ltlt => w.write_all(b"<<"),
        Bop::Gtgt => w.write_all(b">>"),
        Bop::Percent => w.write_all(b"%"),
        Bop::Xor => w.write_all(b"^"),
        Bop::Diff => w.write_all(b"!="),
        Bop::Diff2 => w.write_all(b"!=="),
        Bop::QuestionQuestion => w.write_all(b"??"),
    }
}

fn print_hint(w: &mut dyn Write, ns: bool, hint: &ast::Hint) -> Result<()> {
    let alloc = bumpalo::Bump::new();
    let h = emit_type_hint::fmt_hint(&alloc, &[], false, hint).map_err(|e| match e {
        Unrecoverable(s) => Error::fail(s).into(),
        _ => Error::fail("Error printing hint"),
    })?;
    if ns {
        w.write_all(escape(h).as_bytes())
    } else {
        w.write_all(escape(strip_ns(&h)).as_bytes())
    }
}

fn print_import_flavor(w: &mut dyn Write, flavor: &ast::ImportFlavor) -> Result<()> {
    use ast::ImportFlavor as F;
    w.write_all(match flavor {
        F::Include => b"include",
        F::Require => b"require",
        F::IncludeOnce => b"include_once",
        F::RequireOnce => b"require_once",
    })
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

fn print_span(w: &mut dyn Write, &HhasSpan(line_begin, line_end): &HhasSpan) -> Result<()> {
    write!(w, "({},{})", line_begin, line_end)
}

fn print_fun_attrs(ctx: &Context<'_>, w: &mut dyn Write, f: &HhasFunction<'_>) -> Result<()> {
    use hhas_attribute::*;
    let user_attrs = f.attributes.as_ref();
    let mut special_attrs = vec![];
    if has_meth_caller(user_attrs) {
        special_attrs.push("builtin");
        special_attrs.push("is_meth_caller");
    }
    if f.is_interceptable() {
        special_attrs.push("interceptable");
    }
    if has_foldable(user_attrs) {
        special_attrs.push("foldable");
    }
    if has_provenance_skip_frame(user_attrs) {
        special_attrs.push("prov_skip_frame");
    }
    if f.is_no_injection() {
        special_attrs.push("no_injection");
    }
    if f.is_readonly_return() {
        special_attrs.push("readonly_return");
    }
    if ctx.is_system_lib() || (has_dynamically_callable(user_attrs) && !f.is_memoize_impl()) {
        special_attrs.push("dyn_callable")
    }
    if ctx.is_system_lib() {
        special_attrs.push("unique");
        special_attrs.push("builtin");
        special_attrs.push("persistent");
    }
    print_special_and_user_attrs(ctx, w, &special_attrs, user_attrs)
}

fn print_special_and_user_attrs(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    specials: &[&str],
    users: &[HhasAttribute<'_>],
) -> Result<()> {
    if !users.is_empty() || !specials.is_empty() {
        square(w, |w| {
            concat_str_by(w, " ", specials)?;
            if !specials.is_empty() && !users.is_empty() {
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
        concat_str_by(w, " ", [id.unsafe_as_str(), "as", ""])?;
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
        concat_str_by(w, " ", [id.unsafe_as_str(), "as", ""])?;
        concat_by(w, ", ", &tys, print_type_info)
    })
}

fn print_type_info(w: &mut dyn Write, ti: &HhasTypeInfo<'_>) -> Result<()> {
    print_type_info_(w, false, ti)
}

fn print_type_flags(w: &mut dyn Write, flag: constraint::ConstraintFlags) -> Result<()> {
    let mut first = true;
    let mut print_space = |w: &mut dyn Write| -> Result<()> {
        if !first {
            w.write_all(b" ")
        } else {
            Ok(first = false)
        }
    };
    use constraint::ConstraintFlags as F;
    if flag.contains(F::DISPLAY_NULLABLE) {
        print_space(w)?;
        w.write_all(b"display_nullable")?;
    }
    if flag.contains(F::EXTENDED_HINT) {
        print_space(w)?;
        w.write_all(b"extended_hint")?;
    }
    if flag.contains(F::NULLABLE) {
        print_space(w)?;
        w.write_all(b"nullable")?;
    }

    if flag.contains(F::SOFT) {
        print_space(w)?;
        w.write_all(b"soft")?;
    }
    if flag.contains(F::TYPE_CONSTANT) {
        print_space(w)?;
        w.write_all(b"type_constant")?;
    }

    if flag.contains(F::TYPE_VAR) {
        print_space(w)?;
        w.write_all(b"type_var")?;
    }

    if flag.contains(F::UPPERBOUND) {
        print_space(w)?;
        w.write_all(b"upper_bound")?;
    }
    Ok(())
}

fn print_type_info_(w: &mut dyn Write, is_enum: bool, ti: &HhasTypeInfo<'_>) -> Result<()> {
    let print_quote_str = |w: &mut dyn Write, opt: &Option<String>| {
        option_or(
            w,
            opt,
            |w, s: &String| quotes(w, |w| w.write_all(escape(s).as_bytes())),
            "N",
        )
    };
    angle(w, |w| {
        print_quote_str(
            w,
            &Option::from(ti.user_type.map(|n| n.unsafe_as_str().to_owned())),
        )?;
        w.write_all(b" ")?;
        if !is_enum {
            print_quote_str(
                w,
                &Option::from(
                    ti.type_constraint
                        .name
                        .map(|n| n.unsafe_as_str().to_owned()),
                ),
            )?;
            w.write_all(b" ")?;
        }
        print_type_flags(w, ti.type_constraint.flags)
    })
}

fn print_typedef_info(w: &mut dyn Write, ti: &HhasTypeInfo<'_>) -> Result<()> {
    angle(w, |w| {
        w.write_all(
            quote_string(
                ti.type_constraint
                    .name
                    .as_ref()
                    .map_or("", |n| n.unsafe_as_str()),
            )
            .as_bytes(),
        )?;
        let flags = ti.type_constraint.flags & constraint::ConstraintFlags::NULLABLE;
        if !flags.is_empty() {
            wrap_by(w, " ", |w| {
                print_type_flags(
                    w,
                    ti.type_constraint.flags & constraint::ConstraintFlags::NULLABLE,
                )
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

fn print_record_field(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    Field(name, type_info, intial_value): &Field<'_>,
) -> Result<()> {
    ctx.newline(w)?;
    w.write_all(b".property ")?;
    match intial_value {
        Just(_) => w.write_all(b"[public] ")?,
        Nothing => w.write_all(b"[public sys_initial_val] ")?,
    }
    print_type_info(w, type_info)?;
    concat_str_by(w, " ", ["", name.unsafe_as_str(), "="])?;

    ctx.block(w, |c, w| {
        c.newline(w)?;
        match intial_value {
            Nothing => w.write_all(b"uninit")?,
            Just(value) => triple_quotes(w, |w| print_adata(c, w, value))?,
        }
        w.write_all(b";")
    })
}

fn print_record_def(ctx: &Context<'_>, w: &mut dyn Write, record: &HhasRecord<'_>) -> Result<()> {
    newline(w)?;
    if record.is_abstract {
        concat_str_by(w, " ", [".record", record.name.to_raw_string()])?;
    } else {
        concat_str_by(w, " ", [".record", "[final]", record.name.to_raw_string()])?;
    }
    w.write_all(b" ")?;
    print_span(w, &record.span)?;
    print_extends(
        w,
        Option::from(record.base.as_ref().map(|b| b.to_raw_string())),
    )?;
    w.write_all(b" ")?;

    braces(w, |w| {
        ctx.block(w, |c, w| {
            concat(w, &record.fields, |w, rf| print_record_field(c, w, rf))
        })?;
        ctx.newline(w)
    })?;
    newline(w)
}

/// Convert an `Expr` to a `String` of the equivalent source code.
///
/// This is a debugging tool abusing a printer written for bytecode
/// emission. It does not support all Hack expressions, and panics
/// on unsupported syntax.
///
/// If you have an `Expr` with positions, you are much better off
/// getting the source code at those positions rather than using this.
pub fn expr_to_string_lossy(mut ctx: Context<'_>, expr: &ast::Expr) -> String {
    ctx.dump_lambdas = true;

    let env = ExprEnv { codegen_env: None };
    let mut escaped_src = Vec::new();
    print_expr(&mut ctx, &mut escaped_src, &env, expr).expect("Printing failed");

    let bs = escaper::unescape_double(unsafe { std::str::from_utf8_unchecked(&escaped_src) })
        .expect("Unescaping failed");
    let s = String::from_utf8_lossy(&bs);
    s.to_string()
}

pub fn external_print_program(
    ctx: &Context<'_>,
    w: &mut dyn std::io::Write,
    prog: &HhasProgram<'_>,
) -> std::result::Result<(), Error> {
    let mut ctx = ctx.clone();
    let mut w = Box::new(w);
    print_program(&mut ctx, &mut w, prog).map_err(write::into_error)?;
    w.flush().map_err(write::into_error)?;
    Ok(())
}

pub fn external_print_expr(
    ctx: &Context<'_>,
    w: &mut dyn std::io::Write,
    env: &ExprEnv<'_, '_>,
    expr: &ast::Expr,
) -> std::result::Result<(), Error> {
    let mut ctx = ctx.clone();
    let mut w = Box::new(w);
    print_expr(&mut ctx, &mut w, env, expr).map_err(write::into_error)?;
    w.flush().map_err(write::into_error)?;
    Ok(())
}

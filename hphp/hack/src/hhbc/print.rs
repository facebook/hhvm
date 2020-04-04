// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused_variables)]

mod print_env;
mod write;

use indexmap::IndexSet;
use itertools::Itertools;
pub use write::{Error, IoWrite, Result, Write};

use ast_class_expr_rust::ClassExpr;
use ast_scope_rust as ast_scope;
use context::Context;
use core_utils_rust::add_ns;
use env::{iterator::Id as IterId, local::Type as Local, Env as BodyEnv};
use escaper::{escape, escape_by, is_lit_printable};
use hhas_adata_rust::HhasAdata;
use hhas_adata_rust::{
    ARRAY_PREFIX, DARRAY_PREFIX, DICT_PREFIX, KEYSET_PREFIX, VARRAY_PREFIX, VEC_PREFIX,
};
use hhas_attribute_rust::{self as hhas_attribute, HhasAttribute};
use hhas_body_rust::HhasBody;
use hhas_class_rust::{self as hhas_class, HhasClass};
use hhas_constant_rust::HhasConstant;
use hhas_function_rust::HhasFunction;
use hhas_method_rust::{HhasMethod, HhasMethodFlags};
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_program_rust::HhasProgram;
use hhas_property_rust::HhasProperty;
use hhas_record_def_rust::{Field, HhasRecord};
use hhas_type::{constraint, Info as HhasTypeInfo};
use hhas_type_const::HhasTypeConstant;
use hhas_typedef_rust::Typedef as HhasTypedef;
use hhbc_ast_rust::*;
use hhbc_id_rust::{class, Id};
use hhbc_string_utils_rust::{
    float, integer, is_class, is_parent, is_self, is_static, is_xhp, lstrip, mangle, quote_string,
    quote_string_with_escape, strip_global_ns, strip_ns, triple_quote_string, types,
};
use instruction_sequence_rust::InstrSeq;
use label_rust::Label;
use naming_special_names_rust::classes;
use oxidized::{ast, ast_defs, doc_comment::DocComment, local_id, pos::Pos};
use runtime::TypedValue;
use write::*;

use std::{borrow::Cow, convert::TryInto, io::Write as _};

pub mod context {
    use crate::write::*;
    use env::emitter::Emitter;
    use oxidized::relative_path::RelativePath;

    /// Indent is an abstraction of indentation. Configurable indentation
    /// and perf tweaking will be easier.
    struct Indent(usize);

    impl Indent {
        pub fn new() -> Self {
            Self(0)
        }

        pub fn inc(&mut self) {
            self.0 += 1;
        }

        pub fn dec(&mut self) {
            self.0 -= 1;
        }

        pub fn write<W: Write>(&self, w: &mut W) -> Result<(), W::Error> {
            Ok(for _ in 0..self.0 {
                w.write("  ")?;
            })
        }
    }

    pub struct Context<'a> {
        pub emitter: &'a mut Emitter,
        pub path: Option<&'a RelativePath>,

        dump_symbol_refs: bool,
        indent: Indent,
        is_system_lib: bool,
    }

    impl<'a> Context<'a> {
        pub fn new(
            emitter: &'a mut Emitter,
            path: Option<&'a RelativePath>,
            dump_symbol_refs: bool,
            is_system_lib: bool,
        ) -> Self {
            Self {
                emitter,
                path,
                dump_symbol_refs,
                indent: Indent::new(),
                is_system_lib,
            }
        }

        pub fn dump_symbol_refs(&self) -> bool {
            self.dump_symbol_refs
        }

        /// Insert a newline with indentation
        pub fn newline<W: Write>(&self, w: &mut W) -> Result<(), W::Error> {
            newline(w)?;
            self.indent.write(w)
        }

        /// Start a new indented block
        pub fn block<W, F>(&mut self, w: &mut W, f: F) -> Result<(), W::Error>
        where
            W: Write,
            F: FnOnce(&mut Self, &mut W) -> Result<(), W::Error>,
        {
            self.indent.inc();
            let r = f(self, w);
            self.indent.dec();
            r
        }

        pub fn unblock<W, F>(&mut self, w: &mut W, f: F) -> Result<(), W::Error>
        where
            W: Write,
            F: FnOnce(&mut Self, &mut W) -> Result<(), W::Error>,
        {
            self.indent.dec();
            let r = f(self, w);
            self.indent.inc();
            r
        }

        /// Printing instruction list requies manually control indentation,
        /// where indent_inc/indent_dec are called
        pub fn indent_inc(&mut self) {
            self.indent.inc();
        }

        pub fn indent_dec(&mut self) {
            self.indent.dec();
        }

        pub fn is_system_lib(&self) -> bool {
            self.is_system_lib
        }
    }
}

struct ExprEnv<'e> {
    pub codegen_env: Option<&'e BodyEnv<'e>>,
    pub is_xhp: bool,
}

pub fn print_program<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    prog: &HhasProgram,
) -> Result<(), W::Error> {
    match ctx.path {
        Some(p) => {
            let abs = p.to_absoute();
            let p = escape(abs.to_str().ok_or(Error::InvalidUTF8)?);

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
            w.write("#starts here")?;

            newline(w)?;
            handle_not_impl(|| print_program_(ctx, w, prog))?;

            newline(w)?;
            w.write("#ends here")?;

            newline(w)
        }
    }
}

fn print_program_<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    prog: &HhasProgram,
) -> Result<(), W::Error> {
    let is_hh = if prog.is_hh { "1" } else { "0" };
    newline(w)?;
    write_list(w, &[".hh_file ", is_hh, ";"])?;

    print_data_region(w, vec![])?;
    concat(w, &prog.adata, |w, a| print_adata_region(ctx, w, a))?;
    newline(w)?;
    print_main(ctx, w, &prog.main)?;
    concat(w, &prog.functions, |w, f| print_fun_def(ctx, w, f))?;
    concat(w, &prog.classes, |w, cd| print_class_def(ctx, w, cd))?;
    concat(w, &prog.record_defs, |w, rd| print_record_def(ctx, w, rd))?;
    concat(w, &prog.constants, |w, c| print_constant(ctx, w, c))?;
    concat(w, &prog.typedefs, |w, td| print_typedef(ctx, w, td))?;
    print_file_attributes(ctx, w, &prog.file_attributes)?;

    if ctx.dump_symbol_refs() {
        return not_impl!();
    }
    Ok(())
}

fn print_adata_region<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    adata: &HhasAdata,
) -> Result<(), W::Error> {
    concat_str_by(w, " ", [".adata", adata.id.as_str(), "= "])?;
    wrap_by_triple_quotes(w, |w| print_adata(ctx, w, &adata.value))?;
    w.write(";")?;
    ctx.newline(w)
}

fn print_typedef<W: Write>(ctx: &mut Context, w: &mut W, td: &HhasTypedef) -> Result<(), W::Error> {
    newline(w)?;
    w.write(".alias ")?;
    print_typedef_attributes(ctx, w, td)?;
    w.write(td.name.to_raw_string())?;
    w.write(" = ")?;
    print_typedef_info(w, &td.type_info)?;
    w.write(" ")?;
    wrap_by_triple_quotes(w, |w| print_adata(ctx, w, &td.type_structure))?;
    w.write(";")
}

fn print_typedef_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    td: &HhasTypedef,
) -> Result<(), W::Error> {
    let mut specials = vec![];
    if ctx.is_system_lib() {
        specials.push("persistent");
    }
    print_special_and_user_attrs(ctx, w, &specials[..], td.attributes.as_slice())
}

fn print_data_region_element<W: Write>(w: &mut W, fake_elem: usize) -> Result<(), W::Error> {
    not_impl!()
}

fn print_data_region<W: Write>(w: &mut W, fake_adata: Vec<usize>) -> Result<(), W::Error> {
    concat(w, fake_adata, |w, i| print_data_region_element(w, *i))?;
    newline(w)
}

fn handle_not_impl<E: std::fmt::Debug, F: FnOnce() -> Result<(), E>>(f: F) -> Result<(), E> {
    let r = f();
    match &r {
        Err(Error::NotImpl(msg)) => {
            println!("#### NotImpl: {}", msg);
            eprintln!("NotImpl: {}", msg);
            Ok(())
        }
        _ => r,
    }
}

fn print_fun_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    fun_def: &HhasFunction,
) -> Result<(), W::Error> {
    let body = &fun_def.body;
    newline(w)?;
    w.write(".function ")?;
    if ctx
        .emitter
        .options()
        .hack_compiler_flags
        .contains(options::CompilerFlags::EMIT_GENERICS_UB)
    {
        print_upper_bounds(w, &body.upper_bounds)?;
    }
    print_fun_attrs(ctx, w, fun_def)?;
    w.write(string_of_span(&fun_def.span))?;
    w.write(" ")?;
    option(w, &body.return_type_info, |w, ti| {
        print_type_info(w, ti)?;
        w.write(" ")
    })?;
    w.write(fun_def.name.to_raw_string())?;
    print_params(ctx, w, fun_def.body.env.as_ref(), fun_def.params())?;
    if fun_def.is_generator() {
        w.write(" isGenerator")?;
    }
    if fun_def.is_async() {
        w.write(" isAsync")?;
    }
    if fun_def.is_pair_generator() {
        w.write(" isPairGenerator")?;
    }
    if fun_def.rx_disabled() {
        w.write(" isRxDisabled")?;
    }
    w.write(" ")?;
    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| print_body(c, w, body))?;
        newline(w)
    })?;
    newline(w)
}

fn print_requirement<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    r: &(class::Type<'_>, hhas_class::TraitReqKind),
) -> Result<(), W::Error> {
    w.write("\n  .require ")?;
    match r {
        (name, hhas_class::TraitReqKind::MustExtend) => {
            w.write(format!("extends <{}>;", name.to_raw_string()))
        }
        (name, hhas_class::TraitReqKind::MustImplement) => {
            w.write(format!("implements <{}>;", name.to_raw_string()))
        }
    }
}

fn print_type_constant<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    c: &HhasTypeConstant,
) -> Result<(), W::Error> {
    w.write("\n  .const ")?;
    w.write(&c.name)?;
    w.write(" isType")?;
    match c.initializer.as_ref() {
        Some(init) => {
            w.write(" = \"\"\"")?;
            print_adata(ctx, w, init)?;
            w.write("\"\"\";")
        }
        None => w.write(";"),
    }
}

fn print_property_doc_comment<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    p: &HhasProperty,
) -> Result<(), W::Error> {
    if let Some(s) = p.doc_comment.as_ref() {
        w.write(triple_quote_string(&s.0))?;
        w.write(" ")?;
    }
    Ok(())
}

fn print_property_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    property: &HhasProperty,
) -> Result<(), W::Error> {
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

    w.write("[")?;
    concat_by(w, " ", &special_attributes, |w, a| w.write(a))?;
    if !special_attributes.is_empty() && !property.attributes.is_empty() {
        w.write(" ")?;
    }
    print_attributes(ctx, w, &property.attributes)?;
    w.write("] ")
}

fn print_property_type_info<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    p: &HhasProperty,
) -> Result<(), W::Error> {
    print_type_info(w, &p.type_info)?;
    w.write(" ")
}

fn print_property<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    class_def: &HhasClass,
    property: &HhasProperty,
) -> Result<(), W::Error> {
    newline(w)?;
    w.write("  .property ")?;
    print_property_attributes(ctx, w, property)?;
    print_property_doc_comment(ctx, w, property)?;
    print_property_type_info(ctx, w, property)?;
    w.write(property.name.to_raw_string())?;
    w.write(" =\n    ")?;
    let initial_value = property.initial_value.as_ref();
    if class_def.is_closure() || initial_value == Some(&TypedValue::Uninit) {
        w.write("uninit;")
    } else {
        w.write("\"\"\"")?;
        match initial_value {
            None => w.write("N;"),
            Some(value) => print_adata(ctx, w, &value),
        }?;
        w.write("\"\"\";")
    }
}

fn print_constant<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    c: &HhasConstant,
) -> Result<(), W::Error> {
    ctx.newline(w)?;
    w.write(".const ")?;
    w.write(c.name.to_raw_string())?;
    match c.value.as_ref() {
        Some(TypedValue::Uninit) => w.write(" = uninit")?,
        Some(value) => {
            w.write(" = \"\"\"")?;
            print_adata(ctx, w, value)?;
            w.write("\"\"\"")?
        }
        None => (),
    }
    w.write(";")
}

fn print_enum_ty<W: Write>(ctx: &mut Context, w: &mut W, c: &HhasClass) -> Result<(), W::Error> {
    if let Some(et) = c.enum_type.as_ref() {
        newline(w)?;
        w.write("  .enum_ty ")?;
        print_type_info_(w, true, et)?;
        w.write(";")?;
    }
    Ok(())
}

fn print_doc_comment<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    doc_comment: &Option<DocComment>,
) -> Result<(), W::Error> {
    if let Some(cmt) = doc_comment {
        ctx.newline(w)?;
        w.write(format!(".doc {};", triple_quote_string(&cmt.0)))?;
    }
    Ok(())
}

fn print_use_precedence<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    (id1, id2, ids): &(class::Type, class::Type, Vec<class::Type>),
) -> Result<(), W::Error> {
    ctx.newline(w)?;
    concat_str(w, [id1.to_raw_string(), "::", id2.to_raw_string()])?;
    w.write(" insteadof ")?;
    let unique_ids: IndexSet<&str> = ids.iter().map(|i| i.to_raw_string()).collect();
    concat_str_by(w, " ", unique_ids.iter().collect::<Vec<_>>())?;
    w.write(";")
}

fn print_use_as_visibility<W: Write>(w: &mut W, u: ast::UseAsVisibility) -> Result<(), W::Error> {
    w.write(match u {
        ast::UseAsVisibility::UseAsPublic => "public",
        ast::UseAsVisibility::UseAsPrivate => "private",
        ast::UseAsVisibility::UseAsProtected => "protected",
        ast::UseAsVisibility::UseAsFinal => "final",
    })
}

fn print_use_alias<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    (ido1, id, ido2, kindl): &(
        Option<class::Type>,
        class::Type,
        Option<class::Type>,
        &Vec<ast::UseAsVisibility>,
    ),
) -> Result<(), W::Error> {
    ctx.newline(w)?;
    let id = id.to_raw_string();
    option_or(
        w,
        ido1,
        |w, i: &class::Type| concat_str(w, [i.to_raw_string(), "::", id]),
        id,
    )?;
    w.write(" as ")?;
    if !kindl.is_empty() {
        wrap_by_square(w, |w| {
            concat_by(w, " ", kindl, |w, k| print_use_as_visibility(w, *k))
        })?;
    }
    w.write_if(!kindl.is_empty() && ido2.is_some(), " ")?;
    option(w, ido2, |w, i: &class::Type| w.write(i.to_raw_string()))?;
    w.write(";")
}

fn print_method_trait_resolutions<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    (mtr, kind_as_tring): &(&ast::MethodRedeclaration, class::Type),
) -> Result<(), W::Error> {
    w.write(format!(
        "\n    {}::{} as strict ",
        kind_as_tring.to_raw_string(),
        mtr.method.1
    ))?;
    w.write_if(mtr.fun_kind.is_async(), "async ")?;
    wrap_by_square(w, |w| {
        w.write_if(mtr.final_, "final ")?;
        w.write(mtr.visibility.to_string())?;
        w.write_if(mtr.abstract_, " abstract")?;
        w.write_if(mtr.static_, " static")
    })?;
    w.write(format!(" {};", mtr.name.1))
}

fn print_uses<W: Write>(ctx: &mut Context, w: &mut W, c: &HhasClass) -> Result<(), W::Error> {
    if c.uses.is_empty() && c.method_trait_resolutions.is_empty() {
        Ok(())
    } else {
        let unique_ids: IndexSet<&str> = c
            .uses
            .iter()
            .map(|e| strip_global_ns(e.to_raw_string()))
            .collect();
        let unique_ids: Vec<_> = unique_ids.into_iter().collect();

        newline(w)?;
        w.write("  .use ")?;
        concat_by(w, " ", unique_ids, |w, id| w.write(id))?;

        if c.use_aliases.is_empty()
            && c.use_precedences.is_empty()
            && c.method_trait_resolutions.is_empty()
        {
            w.write(";")
        } else {
            w.write(" {")?;
            ctx.block(w, |ctx, w| {
                for x in &c.use_precedences {
                    print_use_precedence(ctx, w, x)?;
                }
                for x in &c.use_aliases {
                    print_use_alias(ctx, w, x)?;
                }
                Ok(())
            })?;
            for x in &c.method_trait_resolutions {
                print_method_trait_resolutions(ctx, w, x)?;
            }
            newline(w)?;
            w.write("  }")
        }
    }
}

fn print_class_special_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    c: &HhasClass,
) -> Result<(), W::Error> {
    let user_attrs = &c.attributes;
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
    if is_system_lib {
        special_attributes.extend(&["unique", "builtin", "persistent"])
    }
    if hhas_attribute::has_dynamically_constructible(user_attrs) {
        special_attributes.push("dyn_constructible");
    }
    if !c.is_top() {
        special_attributes.push("nontop");
    }
    if c.is_closure() && !is_system_lib {
        special_attributes.push("unique");
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
    if c.enum_type.is_some() {
        special_attributes.push("enum");
    }
    if c.is_abstract() {
        special_attributes.push("abstract");
    }
    if special_attributes.is_empty() && user_attrs.is_empty() {
        return Ok(());
    }

    w.write("[")?;
    special_attributes.reverse();
    concat_by(w, " ", &special_attributes, |w, a| w.write(a))?;
    if !special_attributes.is_empty() && !user_attrs.is_empty() {
        w.write(" ")?;
    }
    print_attributes(ctx, w, &user_attrs)?;
    w.write("] ")?;
    Ok(())
}

fn print_implements<W: Write>(
    w: &mut W,
    implements: &Vec<class::Type<'_>>,
) -> Result<(), W::Error> {
    if implements.is_empty() {
        return Ok(());
    }
    w.write(" implements (")?;
    concat_str_by(
        w,
        " ",
        implements
            .iter()
            .map(|x| x.to_raw_string())
            .collect::<Vec<_>>(),
    )?;
    w.write(")")
}

fn print_shadowed_tparams<W: Write>(
    w: &mut W,
    shadowed_tparams: &[String],
) -> Result<(), W::Error> {
    wrap_by_braces(w, |w| concat_str_by(w, ", ", shadowed_tparams))
}

fn print_method_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    method_def: &HhasMethod,
) -> Result<(), W::Error> {
    let body = &method_def.body;
    newline(w)?;
    w.write("  .method ")?;
    if ctx
        .emitter
        .options()
        .hack_compiler_flags
        .contains(options::CompilerFlags::EMIT_GENERICS_UB)
    {
        print_upper_bounds(w, &body.upper_bounds)?;
    }
    print_method_attrs(ctx, w, method_def)?;
    w.write(string_of_span(&method_def.span))?;
    w.write(" ")?;
    option(w, &body.return_type_info, |w, t| {
        print_type_info(w, t)?;
        w.write(" ")
    })?;
    w.write(method_def.name.to_raw_string())?;
    print_params(ctx, w, body.env.as_ref(), &body.params)?;
    if method_def.flags.contains(HhasMethodFlags::IS_GENERATOR) {
        w.write(" isGenerator")?;
    }
    if method_def.flags.contains(HhasMethodFlags::IS_ASYNC) {
        w.write(" isAsync")?;
    }
    if method_def
        .flags
        .contains(HhasMethodFlags::IS_PAIR_GENERATOR)
    {
        w.write(" isPairGenerator")?;
    }
    if method_def.flags.contains(HhasMethodFlags::IS_CLOSURE_BODY) {
        w.write(" isClosureBody")?;
    }
    if method_def.flags.contains(HhasMethodFlags::RX_DISABLED) {
        w.write(" isRxDisabled")?;
    }
    w.write(" ")?;
    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| print_body(c, w, body))?;
        newline(w)?;
        w.write("  ")
    })
}

fn print_method_attrs<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    m: &HhasMethod,
) -> Result<(), W::Error> {
    use hhas_attribute::*;
    let user_attrs = &m.attributes;
    let mut special_attrs = vec![];
    if let Ok(attr) = m.rx_level.try_into() {
        special_attrs.push(attr);
    }
    if has_provenance_skip_frame(user_attrs) {
        special_attrs.push("prov_skip_frame")
    }
    if m.is_interceptable() {
        special_attrs.push("interceptable");
    }
    let visibility = m.visibility.to_string();
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

fn print_class_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    class_def: &HhasClass,
) -> Result<(), W::Error> {
    newline(w)?;
    w.write(".class ")?;
    if ctx
        .emitter
        .options()
        .hack_compiler_flags
        .contains(options::CompilerFlags::EMIT_GENERICS_UB)
    {
        print_upper_bounds(w, &class_def.upper_bounds)?;
    }
    print_class_special_attributes(ctx, w, class_def)?;
    w.write(class_def.name.to_raw_string())?;
    w.write(format!(" {}", string_of_span(&class_def.span)))?;
    print_extends(w, class_def.base.as_ref().map(|x| x.to_raw_string()))?;
    print_implements(w, &class_def.implements)?;
    w.write(" {")?;
    ctx.block(w, |c, w| {
        print_doc_comment(c, w, &class_def.doc_comment)?;
        print_uses(c, w, class_def)?;
        print_enum_ty(c, w, class_def)?;
        for x in &class_def.requirements {
            print_requirement(c, w, x)?;
        }
        for x in &class_def.constants {
            print_constant(c, w, x)?;
        }
        for x in &class_def.type_constants {
            print_type_constant(c, w, x)?;
        }
        for x in &class_def.properties {
            print_property(c, w, class_def, x)?;
        }
        for m in &class_def.methods {
            print_method_def(c, w, m)?;
        }
        Ok(())
    })?;
    newline(w)?;
    w.write("}")?;
    newline(w)
}

fn pos_to_prov_tag(ctx: &Context, loc: &Option<ast_defs::Pos>) -> String {
    match loc {
        Some(_) if ctx.emitter.options().array_provenance() => unimplemented!(),
        _ => "".into(),
    }
}

fn print_function_id<W: Write>(w: &mut W, id: &FunctionId) -> Result<(), W::Error> {
    wrap_by_quotes(w, |w| w.write(escape(id.to_raw_string())))
}

fn print_class_id<W: Write>(w: &mut W, id: &ClassId) -> Result<(), W::Error> {
    wrap_by_quotes(w, |w| w.write(escape(id.to_raw_string())))
}

fn print_method_id<W: Write>(w: &mut W, id: &MethodId) -> Result<(), W::Error> {
    wrap_by_quotes(w, |w| w.write(escape(id.to_raw_string())))
}

fn print_const_id<W: Write>(w: &mut W, id: &ConstId) -> Result<(), W::Error> {
    wrap_by_quotes(w, |w| w.write(escape(id.to_raw_string())))
}

fn print_prop_id<W: Write>(w: &mut W, id: &PropId) -> Result<(), W::Error> {
    wrap_by_quotes(w, |w| w.write(escape(id.to_raw_string())))
}

fn print_adata_id<W: Write>(w: &mut W, id: &AdataId) -> Result<(), W::Error> {
    concat_str(w, ["@", id.as_str()])
}

fn print_adata_mapped_argument<W: Write, F, V>(
    ctx: &mut Context,
    w: &mut W,
    col_type: &str,
    loc: &Option<ast_defs::Pos>,
    values: &Vec<V>,
    f: F,
) -> Result<(), W::Error>
where
    F: Fn(&mut Context, &mut W, &V) -> Result<(), W::Error>,
{
    w.write(format!(
        "{}:{}:{{{}",
        col_type,
        values.len(),
        pos_to_prov_tag(ctx, loc)
    ))?;
    for v in values {
        f(ctx, w, v)?
    }
    w.write(format!("}}"))
}

fn print_adata_collection_argument<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    col_type: &str,
    loc: &Option<ast_defs::Pos>,
    values: &Vec<TypedValue>,
) -> Result<(), W::Error> {
    print_adata_mapped_argument(ctx, w, col_type, loc, values, &print_adata)
}

fn print_adata_dict_collection_argument<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    col_type: &str,
    loc: &Option<ast_defs::Pos>,
    pairs: &Vec<(TypedValue, TypedValue)>,
) -> Result<(), W::Error> {
    print_adata_mapped_argument(ctx, w, col_type, loc, pairs, |ctx, w, (v1, v2)| {
        print_adata(ctx, w, v1)?;
        print_adata(ctx, w, v2)
    })
}

fn print_adata<W: Write>(ctx: &mut Context, w: &mut W, tv: &TypedValue) -> Result<(), W::Error> {
    match tv {
        TypedValue::Uninit => w.write("uninit"),
        TypedValue::Null => w.write("N;"),
        TypedValue::String(s) => w.write(format!("s:{}:{};", s.len(), quote_string_with_escape(s))),
        TypedValue::Float(f) => w.write(format!("d:{};", float::to_string(*f))),
        TypedValue::Int(i) => w.write(format!("i:{};", i)),
        // TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why?
        TypedValue::Bool(false) => w.write("b:0;"),
        TypedValue::Bool(true) => w.write("b:1;"),
        TypedValue::Dict((pairs, loc)) => {
            print_adata_dict_collection_argument(ctx, w, DICT_PREFIX, loc, pairs)
        }
        TypedValue::Vec((values, loc)) => {
            print_adata_collection_argument(ctx, w, VEC_PREFIX, loc, values)
        }
        TypedValue::DArray((pairs, loc)) => {
            print_adata_dict_collection_argument(ctx, w, DARRAY_PREFIX, loc, pairs)
        }
        TypedValue::Array(pairs) => {
            print_adata_dict_collection_argument(ctx, w, ARRAY_PREFIX, &None, pairs)
        }
        TypedValue::Keyset(values) => {
            print_adata_collection_argument(ctx, w, KEYSET_PREFIX, &None, values)
        }
        TypedValue::VArray((values, loc)) => {
            print_adata_collection_argument(ctx, w, VARRAY_PREFIX, loc, values)
        }
        TypedValue::HhasAdata(s) => not_impl!(),
    }
}

fn print_attribute<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    a: &HhasAttribute,
) -> Result<(), W::Error> {
    w.write(format!(
        "\"{}\"(\"\"\"{}:{}:{{",
        a.name,
        VARRAY_PREFIX,
        a.arguments.len()
    ))?;
    concat(w, &a.arguments, |w, arg| print_adata(ctx, w, arg))?;
    w.write("}\"\"\")")
}

fn print_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    al: impl AsRef<[HhasAttribute]>,
) -> Result<(), W::Error> {
    // Adjust for underscore coming before alphabet
    let al: Vec<&HhasAttribute> = al
        .as_ref()
        .iter()
        .sorted_by_key(|a| (!a.name.starts_with("__"), &a.name))
        .collect();
    concat_by(w, " ", &al, |w, a| print_attribute(ctx, w, a))
}

fn print_file_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    al: &Vec<HhasAttribute>,
) -> Result<(), W::Error> {
    if al.is_empty() {
        return Ok(());
    }
    newline(w)?;
    w.write(".file_attributes [")?;
    print_attributes(ctx, w, al)?;
    w.write("] ;")?;
    newline(w)
}

fn print_main<W: Write>(ctx: &mut Context, w: &mut W, body: &HhasBody) -> Result<(), W::Error> {
    w.write(".main ")?;
    w.write("(1,1) ")?;
    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| print_body(c, w, body))?;
        newline(w)
    })?;
    newline(w)
}

fn is_bareword_char(c: &u8) -> bool {
    match *c {
        b'_' | b'.' | b'$' | b'\\' => true,
        c => (c >= b'0' && c <= b'9') || (c >= b'a' && c <= b'z') || (c >= b'A' && c <= b'Z'),
    }
}

fn print_body<W: Write>(ctx: &mut Context, w: &mut W, body: &HhasBody) -> Result<(), W::Error> {
    print_doc_comment(ctx, w, &body.doc_comment)?;
    if body.is_memoize_wrapper {
        ctx.newline(w)?;
        w.write(".ismemoizewrapper;")?;
    }
    if body.is_memoize_wrapper_lsb {
        ctx.newline(w)?;
        w.write(".ismemoizewrapperlsb;")?;
    }
    if body.num_iters > 0 {
        ctx.newline(w)?;
        w.write(format!(".numiters {};", body.num_iters))?;
    }
    if !body.decl_vars.is_empty() {
        ctx.newline(w)?;
        w.write(".declvars ")?;
        concat_by(w, " ", &body.decl_vars, |w, var| {
            if var.as_bytes().iter().all(is_bareword_char) {
                w.write(var)
            } else {
                wrap_by_quotes(w, |w| w.write(escaper::escape(var)))
            }
        })?;
        w.write(";")?;
    }
    print_instructions(ctx, w, &body.body_instrs)
}

fn print_instructions<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    instr_seq: &InstrSeq,
) -> Result<(), W::Error> {
    use Instruct::*;
    use InstructTry::*;
    for instr in instr_seq.compact_iter() {
        match instr {
            ISpecialFlow(_) => return not_impl!(),
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

fn if_then<F: FnOnce() -> R, R>(cond: bool, f: F) -> Option<R> {
    if cond {
        Some(f())
    } else {
        None
    }
}

fn print_fcall_args<W: Write>(
    w: &mut W,
    FcallArgs(fls, num_args, num_rets, inouts, async_eager_label, context): &FcallArgs,
) -> Result<(), W::Error> {
    use FcallFlags as F;
    let mut flags = vec![];
    if_then(fls.contains(F::HAS_UNPACK), || flags.push("Unpack"));
    if_then(fls.contains(F::HAS_GENERICS), || flags.push("Generics"));
    if_then(fls.contains(F::SUPPORTS_ASYNC_EAGER_RETURN), || {
        flags.push("SupportsAER")
    });
    if_then(fls.contains(F::LOCK_WHILE_UNWINDING), || {
        flags.push("LockWhileUnwinding")
    });
    wrap_by_angle(w, |w| concat_str_by(w, " ", flags))?;
    w.write(" ")?;
    print_int(w, num_args)?;
    w.write(" ")?;
    print_int(w, num_rets)?;
    w.write(" ")?;
    wrap_by_quotes(w, |w| {
        concat_by(w, "", inouts, |w, i| w.write(if *i { "1" } else { "0" }))
    })?;
    w.write(" ")?;
    option_or(w, async_eager_label, print_label, "-")?;
    w.write(" ")?;
    match context {
        Some(s) => wrap_by_quotes(w, |w| w.write(s)),
        None => w.write("\"\""),
    }
}

fn print_special_cls_ref<W: Write>(w: &mut W, cls_ref: &SpecialClsRef) -> Result<(), W::Error> {
    w.write(match cls_ref {
        SpecialClsRef::Static => "Static",
        SpecialClsRef::Self_ => "Self",
        SpecialClsRef::Parent => "Parent",
    })
}

fn print_null_flavor<W: Write>(w: &mut W, f: &ObjNullFlavor) -> Result<(), W::Error> {
    w.write(match f {
        ObjNullFlavor::NullThrows => "NullThrows",
        ObjNullFlavor::NullSafe => "NullSafe",
    })
}

fn print_instr<W: Write>(w: &mut W, instr: &Instruct) -> Result<(), W::Error> {
    fn print_call<W: Write>(w: &mut W, call: &InstructCall) -> Result<(), W::Error> {
        use InstructCall as I;
        match call {
            I::NewObj => w.write("NewObj"),
            I::NewObjR => w.write("NewObjR"),
            I::NewObjD(cid) => {
                w.write("NewObjD ")?;
                print_class_id(w, cid)
            }
            I::NewObjRD(cid) => {
                w.write("NewObjRD ")?;
                print_class_id(w, cid)
            }
            I::NewObjS(r) => {
                w.write("NewObjS ")?;
                print_special_cls_ref(w, r)
            }
            I::FCall(fcall_args) => {
                w.write("FCall ")?;
                print_fcall_args(w, &fcall_args)?;
                w.write(r#" "" """#)
            }
            I::FCallBuiltin(n, un, io, id) => concat_str_by(
                w,
                " ",
                [
                    "FCallBuiltin",
                    n.to_string().as_str(),
                    un.to_string().as_str(),
                    io.to_string().as_str(),
                    quote_string(id).as_str(),
                ],
            ),
            I::FCallClsMethod(fcall_args, is_log_as_dynamic_call) => {
                w.write("FCallClsMethod ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                w.write(match is_log_as_dynamic_call {
                    IsLogAsDynamicCallOp::LogAsDynamicCall => "LogAsDynamicCall",
                    IsLogAsDynamicCallOp::DontLogAsDynamicCall => "DontLogAsDynamicCall",
                })
            }
            I::FCallClsMethodD(fcall_args, cid, mid) => {
                w.write("FCallClsMethodD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                print_class_id(w, cid)?;
                w.write(" ")?;
                print_method_id(w, mid)
            }
            I::FCallClsMethodS(fcall_args, r) => {
                w.write("FCallClsMethodS ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                print_special_cls_ref(w, r)
            }
            I::FCallClsMethodSD(fcall_args, r, mid) => {
                w.write("FCallClsMethodSD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                print_special_cls_ref(w, r)?;
                w.write(" ")?;
                print_method_id(w, mid)
            }
            I::FCallCtor(fcall_args) => {
                w.write("FCallCtor ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" """#)
            }
            I::FCallFunc(fcall_args) => {
                w.write("FCallFunc ")?;
                print_fcall_args(w, fcall_args)
            }
            I::FCallFuncD(fcall_args, id) => {
                w.write("FCallFuncD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(" ")?;
                print_function_id(w, id)
            }
            I::FCallObjMethod(fcall_args, nf) => {
                w.write("FCallObjMethod ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                print_null_flavor(w, nf)
            }
            I::FCallObjMethodD(fcall_args, nf, id) => {
                w.write("FCallObjMethodD ")?;
                print_fcall_args(w, fcall_args)?;
                w.write(r#" "" "#)?;
                print_null_flavor(w, nf)?;
                w.write(" ")?;
                print_method_id(w, id)
            }
        }
    }

    fn print_get<W: Write>(w: &mut W, get: &InstructGet) -> Result<(), W::Error> {
        use InstructGet as IG;
        match get {
            IG::CGetL(id) => {
                w.write("CGetL ")?;
                print_local(w, id)
            }
            IG::CGetQuietL(id) => {
                w.write("CGetQuietL ")?;
                print_local(w, id)
            }
            IG::CGetL2(id) => {
                w.write("CGetL2 ")?;
                print_local(w, id)
            }
            IG::CUGetL(id) => {
                w.write("CUGetL ")?;
                print_local(w, id)
            }
            IG::PushL(id) => {
                w.write("PushL ")?;
                print_local(w, id)
            }
            IG::CGetG => w.write("CGetG"),
            IG::CGetS => w.write("CGetS"),
            IG::ClassGetC => w.write("ClassGetC"),
            IG::ClassGetTS => w.write("ClassGetTS"),
        }
    }

    use Instruct::*;
    use InstructBasic as IB;
    match instr {
        IIterator(i) => print_iterator(w, i),
        IBasic(b) => w.write(match b {
            IB::Nop => "Nop",
            IB::EntryNop => "EntryNop",
            IB::PopC => "PopC",
            IB::PopU => "PopU",
            IB::Dup => "Dup",
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
            w.write(":")
        }
        IIsset(i) => print_isset(w, i),
        IBase(i) => print_base(w, i),
        IFinal(i) => print_final(w, i),
        ITry(itry) => print_try(w, itry),
        IComment(s) => concat_str_by(w, " ", ["#", s.as_str()]),
        ISrcLoc(p) => concat_str(
            w,
            [
                ".srcloc ",
                format!(
                    "{}:{},{}:{}",
                    p.line_begin, p.col_begin, p.line_end, p.col_end
                )
                .as_str(),
                ";",
            ],
        ),
        IAsync(a) => print_async(w, a),
        IGenerator(gen) => print_gen_creation_execution(w, gen),
        IIncludeEvalDefine(ed) => print_include_eval_define(w, ed),
        IGenDelegation(g) => print_gen_delegation(w, g),
        _ => Err(Error::fail("invalid instruction")),
    }
}

fn print_gen_delegation<W: Write>(w: &mut W, g: &GenDelegation) -> Result<(), W::Error> {
    use GenDelegation as G;
    match g {
        G::ContAssignDelegate(i) => {
            w.write("ContAssignDelegate ")?;
            print_iterator_id(w, i)
        }
        G::ContEnterDelegate => w.write("ContEnterDelegate"),
        G::YieldFromDelegate(i, l) => {
            w.write("YieldFromDelegate ")?;
            print_iterator_id(w, i)?;
            w.write(" ")?;
            print_label(w, l)
        }
        G::ContUnsetDelegate(free, i) => {
            w.write("ContUnsetDelegate ")?;
            w.write(match free {
                FreeIterator::IgnoreIter => "IgnoreIter ",
                FreeIterator::FreeIter => "FreeIter ",
            })?;
            print_iterator_id(w, i)
        }
    }
}

fn print_base<W: Write>(w: &mut W, i: &InstructBase) -> Result<(), W::Error> {
    use InstructBase as I;
    match i {
        I::BaseGC(si, m) => {
            w.write("BaseGC ")?;
            print_stack_index(w, si)?;
            w.write(" ")?;
            print_member_opmode(w, m)
        }
        I::BaseGL(id, m) => {
            w.write("BaseGL ")?;
            print_local(w, id)?;
            w.write(" ")?;
            print_member_opmode(w, m)
        }
        I::BaseSC(si1, si2, m) => {
            w.write("BaseSC ")?;
            print_stack_index(w, si1)?;
            w.write(" ")?;
            print_stack_index(w, si2)?;
            w.write(" ")?;
            print_member_opmode(w, m)
        }
        I::BaseL(id, m) => {
            w.write("BaseL ")?;
            print_local(w, id)?;
            w.write(" ")?;
            print_member_opmode(w, m)
        }
        I::BaseC(si, m) => {
            w.write("BaseC ")?;
            print_stack_index(w, si)?;
            w.write(" ")?;
            print_member_opmode(w, m)
        }
        I::BaseH => w.write("BaseH"),
        I::Dim(m, mk) => {
            w.write("Dim ")?;
            print_member_opmode(w, m)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
    }
}

fn print_stack_index<W: Write>(w: &mut W, si: &StackIndex) -> Result<(), W::Error> {
    w.write(si.to_string())
}

fn print_member_opmode<W: Write>(w: &mut W, m: &MemberOpMode) -> Result<(), W::Error> {
    use MemberOpMode as M;
    w.write(match m {
        M::ModeNone => "None",
        M::Warn => "Warn",
        M::Define => "Define",
        M::Unset => "Unset",
        M::InOut => "InOut",
    })
}

fn print_member_key<W: Write>(w: &mut W, mk: &MemberKey) -> Result<(), W::Error> {
    use MemberKey as M;
    match mk {
        M::EC(si) => {
            w.write("EC:")?;
            print_stack_index(w, si)
        }
        M::EL(local) => {
            w.write("EL:")?;
            print_local(w, local)
        }
        M::ET(s) => {
            w.write("ET:")?;
            wrap_by_quotes(w, |w| w.write(escape(s)))
        }
        M::EI(i) => concat_str(w, ["EI:", i.to_string().as_ref()]),
        M::PC(si) => {
            w.write("PC:")?;
            print_stack_index(w, si)
        }
        M::PL(local) => {
            w.write("PL:")?;
            print_local(w, local)
        }
        M::PT(id) => {
            w.write("PT:")?;
            print_prop_id(w, id)
        }
        M::QT(id) => {
            w.write("QT:")?;
            print_prop_id(w, id)
        }
        M::W => w.write("W"),
    }
}

fn print_iterator<W: Write>(w: &mut W, i: &InstructIterator) -> Result<(), W::Error> {
    use InstructIterator as I;
    match i {
        I::IterInit(iter_args, label) => {
            w.write("IterInit ")?;
            print_iter_args(w, iter_args)?;
            w.write(" ")?;
            print_label(w, label)
        }
        I::IterNext(iter_args, label) => {
            w.write("IterNext ")?;
            print_iter_args(w, iter_args)?;
            w.write(" ")?;
            print_label(w, label)
        }
        I::IterFree(id) => {
            w.write("IterFree ")?;
            print_iterator_id(w, id)
        }
    }
}

fn print_iter_args<W: Write>(w: &mut W, iter_args: &IterArgs) -> Result<(), W::Error> {
    print_iterator_id(w, &iter_args.iter_id)?;
    w.write(" ")?;
    match &iter_args.key_id {
        None => w.write("NK")?,
        Some(k) => {
            w.write("K:")?;
            print_local(w, &k)?;
        }
    };
    w.write(" ")?;
    w.write("V:")?;
    print_local(w, &iter_args.val_id)
}

fn print_iterator_id<W: Write>(w: &mut W, i: &IterId) -> Result<(), W::Error> {
    w.write(format!("{}", i))
}

fn print_async<W: Write>(w: &mut W, a: &AsyncFunctions) -> Result<(), W::Error> {
    use AsyncFunctions as A;
    match a {
        A::WHResult => w.write("WHResult"),
        A::Await => w.write("Await"),
        A::AwaitAll(Some((Local::Unnamed(id), count))) => w.write(format!("L:{}+{}", id, count)),
        A::AwaitAll(None) => w.write("L:0+0"),
        _ => Err(Error::fail("AwaitAll needs an unnamed local")),
    }
}

fn print_query_op<W: Write>(w: &mut W, q: QueryOp) -> Result<(), W::Error> {
    w.write(match q {
        QueryOp::CGet => "CGet",
        QueryOp::CGetQuiet => "CGetQuiet",
        QueryOp::Isset => "Isset",
        QueryOp::InOut => "InOut",
    })
}

fn print_final<W: Write>(w: &mut W, f: &InstructFinal) -> Result<(), W::Error> {
    use InstructFinal as F;
    match f {
        F::QueryM(n, op, mk) => {
            w.write("QueryM ")?;
            print_int(w, n)?;
            w.write(" ")?;
            print_query_op(w, *op)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
        F::UnsetM(n, mk) => {
            w.write("UnsetM ")?;
            print_int(w, n)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
        F::SetM(i, mk) => {
            w.write("SetM ")?;
            print_int(w, i)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
        F::SetOpM(i, op, mk) => {
            w.write("SetOpM ")?;
            print_int(w, i)?;
            w.write(" ")?;
            print_eq_op(w, &op)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
        F::IncDecM(i, op, mk) => {
            w.write("IncDecM ")?;
            print_int(w, i)?;
            w.write(" ")?;
            print_incdec_op(w, &op)?;
            w.write(" ")?;
            print_member_key(w, mk)
        }
        F::SetRangeM(i, op, s) => {
            w.write("SetRangeM ")?;
            print_int(w, i)?;
            w.write(" ")?;
            w.write(match op {
                SetrangeOp::Forward => "Forward",
                SetrangeOp::Reverse => "Reverse",
            })?;
            w.write(" ")?;
            print_int(w, &(*s as usize))
        }
    }
}

fn print_isset<W: Write>(w: &mut W, isset: &InstructIsset) -> Result<(), W::Error> {
    use InstructIsset as I;
    match isset {
        I::IssetC => w.write("IssetC"),
        I::IssetL(local) => {
            w.write("IssetL ")?;
            print_local(w, local)
        }
        I::IssetG => w.write("IssetG"),
        I::IssetS => w.write("IssetS"),
        I::IsTypeC(op) => {
            w.write("IsTypeC ")?;
            print_istype_op(w, op)
        }
        I::IsTypeL(local, op) => {
            w.write("IsTypeL ")?;
            print_local(w, local)?;
            w.write(" ")?;
            print_istype_op(w, op)
        }
    }
}

fn print_istype_op<W: Write>(w: &mut W, op: &IstypeOp) -> Result<(), W::Error> {
    use IstypeOp as Op;
    match op {
        Op::OpNull => w.write("Null"),
        Op::OpBool => w.write("Bool"),
        Op::OpInt => w.write("Int"),
        Op::OpDbl => w.write("Dbl"),
        Op::OpStr => w.write("Str"),
        Op::OpArr => w.write("Arr"),
        Op::OpObj => w.write("Obj"),
        Op::OpRes => w.write("Res"),
        Op::OpScalar => w.write("Scalar"),
        Op::OpKeyset => w.write("Keyset"),
        Op::OpDict => w.write("Dict"),
        Op::OpVec => w.write("Vec"),
        Op::OpArrLike => w.write("ArrLike"),
        Op::OpVArray => w.write("VArray"),
        Op::OpDArray => w.write("DArray"),
        Op::OpClsMeth => w.write("ClsMeth"),
        Op::OpFunc => w.write("Func"),
        Op::OpPHPArr => w.write("PHPArr"),
    }
}

fn print_try<W: Write>(w: &mut W, itry: &InstructTry) -> Result<(), W::Error> {
    use InstructTry as T;
    match itry {
        T::TryCatchBegin => w.write(".try {"),
        T::TryCatchMiddle => w.write("} .catch {"),
        T::TryCatchEnd => w.write("}"),
    }
}

fn print_mutator<W: Write>(w: &mut W, mutator: &InstructMutator) -> Result<(), W::Error> {
    use InstructMutator as M;
    match mutator {
        M::SetL(local) => {
            w.write("SetL ")?;
            print_local(w, local)
        }
        M::PopL(id) => {
            w.write("PopL ")?;
            print_local(w, id)
        }
        M::SetG => w.write("SetG"),
        M::SetS => w.write("SetS"),
        M::SetOpL(id, op) => {
            w.write("SetOpL ")?;
            print_local(w, id)?;
            w.write(" ")?;
            print_eq_op(w, op)
        }
        M::SetOpG(op) => {
            w.write("SetOpG ")?;
            print_eq_op(w, op)
        }
        M::SetOpS(op) => {
            w.write("SetOpS ")?;
            print_eq_op(w, op)
        }
        M::IncDecL(id, op) => {
            w.write("IncDecL ")?;
            print_local(w, id)?;
            w.write(" ")?;
            print_incdec_op(w, op)
        }
        M::IncDecG(op) => {
            w.write("IncDecG ")?;
            print_incdec_op(w, op)
        }
        M::IncDecS(op) => {
            w.write("IncDecS ")?;
            print_incdec_op(w, op)
        }
        M::UnsetL(id) => {
            w.write("UnsetL ")?;
            print_local(w, id)
        }
        M::UnsetG => w.write("UnsetG"),
        M::CheckProp(id) => {
            w.write("CheckProp ")?;
            print_prop_id(w, id)
        }
        M::InitProp(id, op) => {
            w.write("InitProp ")?;
            print_prop_id(w, id)?;
            w.write(" ")?;
            match op {
                InitpropOp::Static => w.write("Static"),
                InitpropOp::NonStatic => w.write("NonStatic"),
            }
        }
    }
}

fn print_eq_op<W: Write>(w: &mut W, op: &EqOp) -> Result<(), W::Error> {
    w.write(match op {
        EqOp::PlusEqual => "PlusEqual",
        EqOp::MinusEqual => "MinusEqual",
        EqOp::MulEqual => "MulEqual",
        EqOp::ConcatEqual => "ConcatEqual",
        EqOp::DivEqual => "DivEqual",
        EqOp::PowEqual => "PowEqual",
        EqOp::ModEqual => "ModEqual",
        EqOp::AndEqual => "AndEqual",
        EqOp::OrEqual => "OrEqual",
        EqOp::XorEqual => "XorEqual",
        EqOp::SlEqual => "SlEqual",
        EqOp::SrEqual => "SrEqual",
        EqOp::PlusEqualO => "PlusEqualO",
        EqOp::MinusEqualO => "MinusEqualO",
        EqOp::MulEqualO => "MulEqualO",
    })
}

fn print_incdec_op<W: Write>(w: &mut W, op: &IncdecOp) -> Result<(), W::Error> {
    w.write(match op {
        IncdecOp::PreInc => "PreInc",
        IncdecOp::PostInc => "PostInc",
        IncdecOp::PreDec => "PreDec",
        IncdecOp::PostDec => "PostDec",
        IncdecOp::PreIncO => "PreIncO",
        IncdecOp::PostIncO => "PostIncO",
        IncdecOp::PreDecO => "PreDecO",
        IncdecOp::PostDecO => "PostDecO",
    })
}

fn print_gen_creation_execution<W: Write>(
    w: &mut W,
    gen: &GenCreationExecution,
) -> Result<(), W::Error> {
    use GenCreationExecution as G;
    match gen {
        G::CreateCont => w.write("CreateCont"),
        G::ContEnter => w.write("ContEnter"),
        G::ContRaise => w.write("ContRaise"),
        G::Yield => w.write("Yield"),
        G::YieldK => w.write("YieldK"),
        G::ContCheck(CheckStarted::IgnoreStarted) => w.write("ContCheck IgnoreStarted"),
        G::ContCheck(CheckStarted::CheckStarted) => w.write("ContCheck CheckStarted"),
        G::ContValid => w.write("ContValid"),
        G::ContKey => w.write("ContKey"),
        G::ContGetReturn => w.write("ContGetReturn"),
        G::ContCurrent => w.write("ContCurrent"),
    }
}

fn print_misc<W: Write>(w: &mut W, misc: &InstructMisc) -> Result<(), W::Error> {
    use InstructMisc as M;
    match misc {
        M::This => w.write("This"),
        M::CheckThis => w.write("CheckThis"),
        M::FuncNumArgs => w.write("FuncNumArgs"),
        M::ChainFaults => w.write("ChainFaults"),
        M::VerifyRetTypeC => w.write("VerifyRetTypeC"),
        M::VerifyRetTypeTS => w.write("VerifyRetTypeTS"),
        M::Self_ => w.write("Self"),
        M::Parent => w.write("Parent"),
        M::LateBoundCls => w.write("LateBoundCls"),
        M::ClassName => w.write("ClassName"),
        M::RecordReifiedGeneric => w.write("RecordReifiedGeneric"),
        M::CheckReifiedGenericMismatch => w.write("CheckReifiedGenericMismatch"),
        M::NativeImpl => w.write("NativeImpl"),
        M::AKExists => w.write("AKExists"),
        M::Idx => w.write("Idx"),
        M::ArrayIdx => w.write("ArrayIdx"),
        M::BreakTraceHint => w.write("BreakTraceHint"),
        M::CGetCUNop => w.write("CGetCUNop"),
        M::UGetCUNop => w.write("UGetCUNop"),
        M::LockObj => w.write("LockObj"),
        M::ThrowNonExhaustiveSwitch => w.write("ThrowNonExhaustiveSwitch"),
        M::VerifyParamType(id) => {
            w.write("VerifyParamType ")?;
            print_param_id(w, id)
        }
        M::VerifyParamTypeTS(id) => {
            w.write("VerifyParamTypeTS ")?;
            print_param_id(w, id)
        }
        M::Silence(local, op) => {
            w.write("Silence ")?;
            print_local(w, local)?;
            w.write(" ")?;
            match op {
                OpSilence::Start => w.write("Start"),
                OpSilence::End => w.write("End"),
            }
        }
        M::VerifyOutType(id) => {
            w.write("VerifyOutType ")?;
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

        M::MemoGet(label, Some((Local::Unnamed(first), local_count))) => {
            w.write("MemoGet ")?;
            print_label(w, label)?;
            w.write(format!(" L:{}+{}", first, local_count))
        }
        M::MemoGet(label, None) => {
            w.write("MemoGet ")?;
            print_label(w, label)?;
            w.write(" L:0+0")
        }
        M::MemoGet(_, _) => Err(Error::fail("MemoGet needs an unnamed local")),

        M::MemoSet(Some((Local::Unnamed(first), local_count))) => {
            w.write(format!("MemoSet L:{}+{}", first, local_count))
        }
        M::MemoSet(None) => w.write("MemoSet L:0+0"),
        M::MemoSet(_) => Err(Error::fail("MemoSet needs an unnamed local")),

        M::MemoGetEager(label1, label2, Some((Local::Unnamed(first), local_count))) => {
            w.write("MemoGetEager ")?;
            print_label(w, label1)?;
            w.write(" ")?;
            print_label(w, label2)?;
            w.write(format!(" L:{}+{}", first, local_count))
        }
        M::MemoGetEager(label1, label2, None) => {
            w.write("MemoGetEager ")?;
            print_label(w, label1)?;
            w.write(" ")?;
            print_label(w, label2)?;
            w.write(" L:0+0")
        }
        M::MemoGetEager(_, _, _) => Err(Error::fail("MemoGetEager needs an unnamed local")),

        M::MemoSetEager(Some((Local::Unnamed(first), local_count))) => {
            w.write(format!("MemoSetEager L:{}+{}", first, local_count))
        }
        M::MemoSetEager(None) => w.write("MemoSetEager L:0+0"),
        M::MemoSetEager(_) => Err(Error::fail("MemoSetEager needs an unnamed local")),

        M::InitThisLoc(id) => {
            w.write("InitThisLoc ")?;
            print_local(w, id)
        }
        M::OODeclExists(k) => concat_str_by(
            w,
            " ",
            [
                "OODeclExists",
                match k {
                    ClassKind::Class => "Class",
                    ClassKind::Interface => "Interface",
                    ClassKind::Trait => "Trait",
                },
            ],
        ),
        M::AssertRATL(local, s) => {
            w.write("AssertRATL ")?;
            print_local(w, local)?;
            w.write(" ")?;
            w.write(s)
        }
        M::AssertRATStk(n, s) => {
            concat_str_by(w, " ", ["AssertRATStk", n.to_string().as_str(), s.as_str()])
        }
        M::GetMemoKeyL(local) => {
            w.write("GetMemoKeyL ")?;
            print_local(w, local)
        }
    }
}

fn print_include_eval_define<W: Write>(
    w: &mut W,
    ed: &InstructIncludeEvalDefine,
) -> Result<(), W::Error> {
    use InstructIncludeEvalDefine::*;
    match ed {
        Incl => w.write("Incl"),
        InclOnce => w.write("InclOnce"),
        Req => w.write("Req"),
        ReqOnce => w.write("ReqOnce"),
        ReqDoc => w.write("ReqDoc"),
        Eval => w.write("Eval"),
        DefCls(n) => concat_str_by(w, " ", ["DefCls", n.to_string().as_str()]),
        DefClsNop(n) => concat_str_by(w, " ", ["DefClsNop", n.to_string().as_str()]),
        DefRecord(n) => concat_str_by(w, " ", ["DefRecord", n.to_string().as_str()]),
        DefCns(n) => concat_str_by(w, " ", ["DefCns", n.to_string().as_str()]),
        DefTypeAlias(id) => w.write(format!("DefTypeAlias {}", id)),
    }
}

fn print_control_flow<W: Write>(w: &mut W, cf: &InstructControlFlow) -> Result<(), W::Error> {
    use InstructControlFlow as CF;
    match cf {
        CF::Jmp(l) => {
            w.write("Jmp ")?;
            print_label(w, l)
        }
        CF::JmpNS(l) => {
            w.write("JmpNS ")?;
            print_label(w, l)
        }
        CF::JmpZ(l) => {
            w.write("JmpZ ")?;
            print_label(w, l)
        }
        CF::JmpNZ(l) => {
            w.write("JmpNZ ")?;
            print_label(w, l)
        }
        CF::RetC => w.write("RetC"),
        CF::RetCSuspended => w.write("RetCSuspended"),
        CF::RetM(p) => concat_str_by(w, " ", ["RetM", p.to_string().as_str()]),
        CF::Throw => w.write("Throw"),
        CF::Switch(kind, base, labels) => print_switch(w, kind, base, labels),
        CF::SSwitch(cases) => match &cases[..] {
            [] => Err(Error::fail("sswitch should have at least one case")),
            [rest @ .., (_, lastlabel)] => {
                w.write("SSwitch ")?;
                wrap_by_angle(w, |w| {
                    concat_by(w, " ", rest, |w, (s, l)| {
                        concat_str(w, [quote_string(s).as_str(), ":"])?;
                        print_label(w, l)
                    })?;
                    w.write(" -:")?;
                    print_label(w, lastlabel)
                })
            }
        },
    }
}

fn print_switch<W: Write>(
    w: &mut W,
    kind: &Switchkind,
    base: &isize,
    labels: &[Label],
) -> Result<(), W::Error> {
    w.write("Switch ")?;
    w.write(match kind {
        Switchkind::Bounded => "Bounded ",
        Switchkind::Unbounded => "Unbounded ",
    })?;
    w.write(base.to_string())?;
    w.write(" ")?;
    wrap_by_angle(w, |w| concat_by(w, " ", labels, print_label))
}

fn print_lit_const<W: Write>(w: &mut W, lit: &InstructLitConst) -> Result<(), W::Error> {
    use InstructLitConst as LC;
    match lit {
        LC::Null => w.write("Null"),
        LC::Int(i) => concat_str_by(w, " ", ["Int", i.to_string().as_str()]),
        LC::String(s) => {
            w.write("String ")?;
            wrap_by_quotes(w, |w| w.write(escape(s)))
        }
        LC::True => w.write("True"),
        LC::False => w.write("False"),
        LC::Double(d) => concat_str_by(w, " ", ["Double", d.as_str()]),
        LC::AddElemC => w.write("AddElemC"),
        LC::AddNewElemC => w.write("AddNewElemC"),
        LC::NewPair => w.write("NewPair"),
        LC::File => w.write("File"),
        LC::Dir => w.write("Dir"),
        LC::Method => w.write("Method"),
        LC::FuncCred => w.write("FuncCred"),
        LC::Array(id) => {
            w.write("Array ")?;
            print_adata_id(w, id)
        }
        LC::Dict(id) => {
            w.write("Dict ")?;
            print_adata_id(w, id)
        }
        LC::Keyset(id) => {
            w.write("Keyset ")?;
            print_adata_id(w, id)
        }
        LC::Vec(id) => {
            w.write("Vec ")?;
            print_adata_id(w, id)
        }
        LC::NewArray(i) => concat_str_by(w, " ", ["NewArray", i.to_string().as_str()]),
        LC::NewMixedArray(i) => concat_str_by(w, " ", ["NewMixedArray", i.to_string().as_str()]),
        LC::NewDictArray(i) => concat_str_by(w, " ", ["NewDictArray", i.to_string().as_str()]),
        LC::NewDArray(i) => concat_str_by(w, " ", ["NewDArray", i.to_string().as_str()]),
        LC::NewPackedArray(i) => concat_str_by(w, " ", ["NewPackedArray", i.to_string().as_str()]),
        LC::NewVArray(i) => concat_str_by(w, " ", ["NewVArray", i.to_string().as_str()]),
        LC::NewVecArray(i) => concat_str_by(w, " ", ["NewVecArray", i.to_string().as_str()]),
        LC::NewKeysetArray(i) => concat_str_by(w, " ", ["NewKeysetArray", i.to_string().as_str()]),
        LC::NewLikeArrayL(local, i) => {
            w.write("NewLikeArrayL ")?;
            print_local(w, local)?;
            w.write(" ")?;
            w.write(i.to_string().as_str())
        }
        LC::NewStructArray(l) => {
            w.write("NewStructArray ")?;
            wrap_by_angle(w, |w| print_shape_fields(w, l))
        }
        LC::NewStructDArray(l) => {
            w.write("NewStructDArray ")?;
            wrap_by_angle(w, |w| print_shape_fields(w, l))
        }
        LC::NewStructDict(l) => {
            w.write("NewStructDict ")?;
            wrap_by_angle(w, |w| print_shape_fields(w, l))
        }
        LC::NewRecord(cid, l) => {
            w.write("NewRecord ")?;
            print_class_id(w, cid)?;
            w.write(" ")?;
            wrap_by_angle(w, |w| print_shape_fields(w, l))
        }
        LC::NewRecordArray(cid, l) => {
            w.write("NewRecordArray ")?;
            print_class_id(w, cid)?;
            w.write(" ")?;
            wrap_by_angle(w, |w| print_shape_fields(w, l))
        }
        LC::CnsE(id) => {
            w.write("CnsE ")?;
            print_const_id(w, id)
        }
        LC::ClsCns(id) => {
            w.write("ClsCns ")?;
            print_const_id(w, id)
        }
        LC::ClsCnsD(const_id, cid) => {
            w.write("ClsCnsD ")?;
            print_const_id(w, const_id)?;
            w.write(" ")?;
            print_class_id(w, cid)
        }
        LC::NewCol(ct) => {
            w.write("NewCol ")?;
            print_collection_type(w, ct)
        }
        LC::ColFromArray(ct) => {
            w.write("ColFromArray ")?;
            print_collection_type(w, ct)
        }
        LC::NullUninit => w.write("NullUninit"),
        LC::TypedValue(tv) => not_impl!(),
    }
}

fn print_collection_type<W: Write>(w: &mut W, ct: &CollectionType) -> Result<(), W::Error> {
    use CollectionType as CT;
    match ct {
        CT::Vector => w.write("Vector"),
        CT::Map => w.write("Map"),
        CT::Set => w.write("Set"),
        CT::Pair => w.write("Pair"),
        CT::ImmVector => w.write("ImmVector"),
        CT::ImmMap => w.write("ImmMap"),
        CT::ImmSet => w.write("ImmSet"),
        CT::Dict => w.write("dict"),
        CT::Array => w.write("array"),
        CT::Keyset => w.write("keyset"),
        CT::Vec => w.write("vec"),
    }
}

fn print_shape_fields<W: Write>(w: &mut W, sf: &Vec<String>) -> Result<(), W::Error> {
    concat_by(w, " ", sf, |w, f| wrap_by_quotes(w, |w| w.write(escape(f))))
}

fn print_op<W: Write>(w: &mut W, op: &InstructOperator) -> Result<(), W::Error> {
    use InstructOperator as I;
    match op {
        I::Concat => w.write("Concat"),
        I::ConcatN(n) => concat_str_by(w, " ", ["ConcatN", n.to_string().as_str()]),
        I::Abs => w.write("Abs"),
        I::Add => w.write("Add"),
        I::Sub => w.write("Sub"),
        I::Mul => w.write("Mul"),
        I::AddO => w.write("AddO"),
        I::SubO => w.write("SubO"),
        I::MulO => w.write("MulO"),
        I::Div => w.write("Div"),
        I::Mod => w.write("Mod"),
        I::Pow => w.write("Pow"),
        I::Sqrt => w.write("Sqrt"),
        I::Xor => w.write("Xor"),
        I::Not => w.write("Not"),
        I::Same => w.write("Same"),
        I::NSame => w.write("NSame"),
        I::Eq => w.write("Eq"),
        I::Neq => w.write("Neq"),
        I::Lt => w.write("Lt"),
        I::Lte => w.write("Lte"),
        I::Gt => w.write("Gt"),
        I::Gte => w.write("Gte"),
        I::Cmp => w.write("Cmp"),
        I::BitAnd => w.write("BitAnd"),
        I::BitOr => w.write("BitOr"),
        I::BitXor => w.write("BitXor"),
        I::BitNot => w.write("BitNot"),
        I::Shl => w.write("Shl"),
        I::Shr => w.write("Shr"),
        I::Floor => w.write("Floor"),
        I::Ceil => w.write("Ceil"),
        I::CastBool => w.write("CastBool"),
        I::CastInt => w.write("CastInt"),
        I::CastDouble => w.write("CastDouble"),
        I::CastString => w.write("CastString"),
        I::CastArray => w.write("CastArray"),
        I::CastVec => w.write("CastVec"),
        I::CastDict => w.write("CastDict"),
        I::CastKeyset => w.write("CastKeyset"),
        I::CastVArray => w.write("CastVArray"),
        I::CastDArray => w.write("CastDArray"),
        I::InstanceOf => w.write("InstanceOf"),
        I::InstanceOfD(id) => {
            w.write("InstanceOfD ")?;
            print_class_id(w, id)
        }
        I::IsLateBoundCls => w.write("IsLateBoundCls"),
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
        I::ThrowAsTypeStructException => w.write("ThrowAsTypeStructException"),
        I::CombineAndResolveTypeStruct(n) => concat_str_by(
            w,
            " ",
            ["CombineAndResolveTypeStruct", n.to_string().as_str()],
        ),
        I::Print => w.write("Print"),
        I::Clone => w.write("Clone"),
        I::Exit => w.write("Exit"),
        I::ResolveFunc(id) => {
            w.write("ResolveFunc ")?;
            print_function_id(w, id)
        }
        I::ResolveMethCaller(id) => {
            w.write("ResolveMethCaller ")?;
            print_function_id(w, id)
        }
        I::ResolveObjMethod => w.write("ResolveObjMethod"),
        I::ResolveClsMethod(mid) => {
            w.write("ResolveClsMethod ")?;
            print_method_id(w, mid)
        }
        I::ResolveClsMethodD(cid, mid) => {
            w.write("ResolveClsMethodD ")?;
            print_class_id(w, cid)?;
            w.write(" ")?;
            print_method_id(w, mid)
        }
        I::ResolveClsMethodS(r, mid) => {
            w.write("ResolveClsMethodS ")?;
            print_special_cls_ref(w, r)?;
            w.write(" ")?;
            print_method_id(w, mid)
        }
        I::Fatal(fatal_op) => print_fatal_op(w, fatal_op),
    }
}

fn print_fatal_op<W: Write>(w: &mut W, f: &FatalOp) -> Result<(), W::Error> {
    match f {
        FatalOp::Parse => w.write("Fatal Parse"),
        FatalOp::Runtime => w.write("Fatal Runtime"),
        FatalOp::RuntimeOmitFrame => w.write("Fatal RuntimeOmitFrame"),
    }
}

fn print_params<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    body_env: Option<&BodyEnv>,
    params: &[HhasParam],
) -> Result<(), W::Error> {
    wrap_by_paren(w, |w| {
        concat_by(w, ", ", params, |w, i| print_param(ctx, w, body_env, i))
    })
}

fn print_param<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    body_env: Option<&BodyEnv>,
    param: &HhasParam,
) -> Result<(), W::Error> {
    print_param_user_attributes(ctx, w, param)?;
    w.write_if(param.is_inout, "inout ")?;
    w.write_if(param.is_variadic, "...")?;
    option(w, &param.type_info, |w, ty| {
        print_type_info(w, ty)?;
        w.write(" ")
    })?;
    w.write(&param.name)?;
    option(w, &param.default_value, |w, i| {
        print_param_default_value(ctx, w, body_env, i)
    })
}

fn print_param_id<W: Write>(w: &mut W, param_id: &ParamId) -> Result<(), W::Error> {
    match param_id {
        ParamId::ParamUnnamed(i) => w.write(i.to_string()),
        ParamId::ParamNamed(s) => w.write(s),
    }
}

fn print_param_default_value<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    body_env: Option<&BodyEnv>,
    default_val: &(Label, ast::Expr),
) -> Result<(), W::Error> {
    let expr_env = ExprEnv {
        codegen_env: body_env,
        is_xhp: false,
    };
    w.write(" = ")?;
    print_label(w, &default_val.0)?;
    wrap_by_paren(w, |w| {
        wrap_by_triple_quotes(w, |w| print_expr(ctx, w, &expr_env, &default_val.1))
    })
}

fn print_label<W: Write>(w: &mut W, label: &Label) -> Result<(), W::Error> {
    match label {
        Label::Regular(id) => {
            w.write("L")?;
            print_int(w, id)
        }
        Label::DefaultArg(id) => {
            w.write("DV")?;
            print_int(w, id)
        }
        Label::Named(id) => w.write(id),
    }
}

fn print_local<W: Write>(w: &mut W, local: &Local) -> Result<(), W::Error> {
    match local {
        Local::Unnamed(id) => {
            w.write("_")?;
            print_int(w, id)
        }
        Local::Named(id) => w.write(id),
    }
}

fn print_int<W: Write>(w: &mut W, i: &usize) -> Result<(), W::Error> {
    // TODO(shiqicao): avoid allocating intermediate string
    w.write(format!("{}", i))
}

fn print_key_value<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    k: &ast::Expr,
    v: &ast::Expr,
) -> Result<(), W::Error> {
    print_key_value_(ctx, w, env, k, print_expr, v)
}

fn print_key_value_<W: Write, K, KeyPrinter>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    k: K,
    mut kp: KeyPrinter,
    v: &ast::Expr,
) -> Result<(), W::Error>
where
    KeyPrinter: FnMut(&mut Context, &mut W, &ExprEnv, K) -> Result<(), W::Error>,
{
    kp(ctx, w, env, k)?;
    w.write(" => ")?;
    print_expr(ctx, w, env, v)
}

fn print_afield<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    afield: &ast::Afield,
) -> Result<(), W::Error> {
    use ast::Afield as A;
    match afield {
        A::AFvalue(e) => print_expr(ctx, w, env, &e),
        A::AFkvalue(k, v) => print_key_value(ctx, w, env, &k, &v),
    }
}

fn print_afields<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    afields: impl AsRef<[ast::Afield]>,
) -> Result<(), W::Error> {
    concat_by(w, ", ", afields, |w, i| print_afield(ctx, w, env, i))
}

fn print_uop<W: Write>(w: &mut W, op: ast::Uop) -> Result<(), W::Error> {
    use ast::Uop as U;
    w.write(match op {
        U::Utild => "~",
        U::Unot => "!",
        U::Uplus => "+",
        U::Uminus => "-",
        U::Uincr => "++",
        U::Udecr => "--",
        U::Usilence => "@",
        U::Upincr | U::Updecr => {
            return Err(Error::fail(
                "string_of_uop - should have been captures earlier",
            ))
        }
    })
}

fn print_key_values<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    kvs: impl AsRef<[(ast::Expr, ast::Expr)]>,
) -> Result<(), W::Error> {
    print_key_values_(ctx, w, env, print_expr, kvs)
}

fn print_key_values_<W: Write, K, KeyPrinter>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    mut kp: KeyPrinter,
    kvs: impl AsRef<[(K, ast::Expr)]>,
) -> Result<(), W::Error>
where
    KeyPrinter: Fn(&mut Context, &mut W, &ExprEnv, &K) -> Result<(), W::Error>,
{
    concat_by(w, ", ", kvs, |w, (k, v)| {
        print_key_value_(ctx, w, env, k, &mut kp, v)
    })
}

fn print_expr_darray<W: Write, K, KeyPrinter>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    kp: KeyPrinter,
    kvs: impl AsRef<[(K, ast::Expr)]>,
) -> Result<(), W::Error>
where
    KeyPrinter: Fn(&mut Context, &mut W, &ExprEnv, &K) -> Result<(), W::Error>,
{
    wrap_by_(w, "darray[", "]", |w| {
        print_key_values_(ctx, w, env, kp, kvs)
    })
}

fn print_shape_field_name<W: Write>(
    _ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    field: &ast::ShapeFieldName,
) -> Result<(), W::Error> {
    use ast::ShapeFieldName as S;
    match field {
        S::SFlitInt((_, s)) => print_expr_int(w, s),
        S::SFlitStr((_, s)) | S::SFclassConst(_, (_, s)) => print_expr_string(w, s),
    }
}

fn print_expr_int<W: Write>(w: &mut W, i: &String) -> Result<(), W::Error> {
    w.write(integer::to_decimal(i.as_str()).map_err(|_| Error::fail("ParseIntError"))?)
}

fn print_expr_string<W: Write>(w: &mut W, s: &String) -> Result<(), W::Error> {
    fn escape_char(c: u8) -> Option<Cow<'static, [u8]>> {
        match c {
            b'\n' => Some((&b"\\\\n"[..]).into()),
            b'\r' => Some((&b"\\\\r"[..]).into()),
            b'\t' => Some((&b"\\\\t"[..]).into()),
            b'\\' => Some((&b"\\\\\\\\"[..]).into()),
            b'"' => Some((&b"\\\\\\\""[..]).into()),
            b'$' => Some((&b"\\\\$"[..]).into()),
            b'?' => Some((&b"\\?"[..]).into()),
            c if is_lit_printable(c) => None,
            c => {
                let mut r = vec![];
                write!(r, "\\\\{:03o}", c).unwrap();
                Some(r.into())
            }
        }
    }
    wrap_by(w, "\\\"", |w| w.write(escape_by(s.into(), escape_char)))
}

fn print_expr<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    ast::Expr(p, expr): &ast::Expr,
) -> Result<(), W::Error> {
    fn adjust_id<'a>(env: &ExprEnv, id: &'a String) -> Cow<'a, str> {
        let s: Cow<'a, str> = match env.codegen_env {
            Some(env) => {
                if env.namespace.name.is_none()
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
    fn print_expr_id<'a, W: Write>(
        w: &mut W,
        env: &ExprEnv,
        s: &'a String,
    ) -> Result<(), W::Error> {
        w.write(adjust_id(env, s))
    }
    fn fmt_class_name<'a>(is_class_constant: bool, id: Cow<'a, str>) -> Cow<'a, str> {
        let cn: Cow<'a, str> = if is_xhp(&strip_ns(&id)) {
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
        ctx: &mut Context,
        env: Option<&'e BodyEnv<'e>>,
        should_format: bool,
        is_class_constant: bool,
        id: &'e str,
    ) -> Cow<'e, str> {
        if id == classes::SELF || id == classes::PARENT || id == classes::STATIC {
            return get_special_class_name(ctx, env, is_class_constant, id);
        }
        fn get<'a>(should_format: bool, is_class_constant: bool, id: &'a str) -> Cow<'a, str> {
            if should_format {
                fmt_class_name(is_class_constant, id.into())
            } else {
                id.into()
            }
        };

        if env.is_some() {
            let class_id = class::Type::from_ast_name(id);
            let id = class_id.to_raw_string();
            get(should_format, is_class_constant, id)
                .into_owned()
                .into()
        } else {
            get(should_format, is_class_constant, id)
        }
    }
    fn get_special_class_name<'e>(
        ctx: &mut Context,
        env: Option<&'e BodyEnv<'e>>,
        is_class_constant: bool,
        id: &'e str,
    ) -> Cow<'e, str> {
        let class_expr = match env {
            None => ClassExpr::expr_to_class_expr(
                ctx.emitter,
                true,
                true,
                &ast_scope::Scope::toplevel(),
                ast::Expr(
                    Pos::make_none(),
                    ast::Expr_::mk_id(ast_defs::Id(Pos::make_none(), id.into())),
                ),
            ),
            Some(body_env) => ClassExpr::expr_to_class_expr(
                ctx.emitter,
                true,
                true,
                &body_env.scope,
                ast::Expr(
                    Pos::make_none(),
                    ast::Expr_::mk_id(ast_defs::Id(Pos::make_none(), id.into())),
                ),
            ),
        };
        let name = match class_expr {
            ClassExpr::Id(ast_defs::Id(_, name)) => Cow::Owned(name),
            _ => Cow::Borrowed(id),
        };
        fmt_class_name(is_class_constant, name)
    }
    fn handle_possible_colon_colon_class_expr<W: Write>(
        ctx: &mut Context,
        w: &mut W,
        env: &ExprEnv,
        is_array_get: bool,
        e_: &ast::Expr_,
    ) -> Result<Option<()>, W::Error> {
        match e_.as_class_const() {
            Some((
                ast::ClassId(_, ast::ClassId_::CIexpr(ast::Expr(_, ast::Expr_::Id(id)))),
                (_, s2),
            )) if is_class(&s2) && !(is_self(&id.1) || is_parent(&id.1) || is_static(&id.1)) => {
                Ok(Some({
                    let s1 =
                        &get_class_name_from_id(ctx, env.codegen_env, false, false, &id.1).into();
                    if is_array_get {
                        print_expr_id(w, env, s1)?
                    } else {
                        print_expr_string(w, s1)?
                    }
                }))
            }
            _ => Ok(None),
        }
    }
    use ast::Expr_ as E_;
    match expr {
        E_::Id(id) => print_expr_id(w, env, &id.1),
        E_::Lvar(lid) => w.write(escaper::escape(&(lid.1).1)),
        E_::Float(f) => {
            w.write(if f.contains('E') || f.contains('e') {
                format!("{:.1}E", f.parse::<f64>().map_err(|_| Error::fail(format!("ParseFloatError: {}", f)))?)
            } else {
                f.into()
            })
        }
        E_::Int(i) => print_expr_int(w, i),
        E_::String(s) => print_expr_string(w, s),
        E_::Null => w.write("NULL"),
        E_::True => w.write("true"),
        E_::False => w.write("false"),
        // For arrays and collections, we are making a conscious decision to not
        // match HHMV has HHVM's emitter has inconsistencies in the pretty printer
        // https://fburl.com/tzom2qoe
        E_::Array(afl) => wrap_by_(w, "array(", ")", |w| print_afields(ctx, w, env, afl)),
        E_::Collection(c) if (c.0).1 == "vec" || (c.0).1 == "dict" || (c.0).1 == "keyset" => {
            w.write(&(c.0).1)?;
            wrap_by_square(w, |w| print_afields(ctx, w, env, &c.2))
        }
        E_::Collection(c) => {
            let name = strip_ns((c.0).1.as_str());
            let name = types::fix_casing(&name);
            match name {
                "Set" | "Pair" | "Vector" | "Map" | "ImmSet" | "ImmVector" | "ImmMap" => {
                    w.write("HH\\\\")?;
                    w.write(name)?;
                    wrap_by_(w, " {", "}", |w| {
                        Ok(if !c.2.is_empty() {
                            w.write(" ")?;
                            print_afields(ctx, w, env, &c.2)?;
                            w.write(" ")?;
                        })
                    })
                }
                _ => Err(Error::fail(format!(
                    "Default value for an unknow collection - {}",
                    name
                ))),
            }
        }
        E_::Shape(fl) => {
            print_expr_darray(ctx, w, env, print_shape_field_name, fl)
        },
        E_::Binop(x) => {
            let (bop, e1, e2) = &**x;
            print_expr(ctx, w, env, e1)?;
            w.write(" ")?;
            print_bop(w, bop)?;
            w.write(" ")?;
            print_expr(ctx, w, env, e2)
        }
        E_::Call(c) => {
            let (_, e, _, es, unpacked_element) = &**c;
            match e.as_id() {
                Some(ast_defs::Id(_, call_id)) => {
                    w.write(lstrip(adjust_id(env, &call_id).as_ref(), "\\\\"))?
                }
                None => {
                    let mut buf = String::new();
                    print_expr(ctx, &mut buf, env, e).map_err(|e| Error::Fail(format!("{}", e)) )?;
                    w.write(lstrip(&buf, "\\\\"))?
                }
            };
            wrap_by_paren(w, |w| {
                concat_by(w, ", ", &es, |w, e| print_expr(ctx, w, env, e))?;
                match unpacked_element {
                    None => Ok(()),
                    Some(e) => {
                        w.write(", ")?;
                        print_expr(ctx, w, env, e)
                    }
            }})
        }
        E_::New(x) => {
            let (cid, _, es, unpacked_element, _) = &**x;
            match cid.1.as_ciexpr() {
                Some(ci_expr) => {
                    w.write("new ")?;
                    match ci_expr.1.as_id() {
                        Some(ast_defs::Id(_, cname)) => {
                            w.write(lstrip(&adjust_id(env, &class::Type::from_ast_name(cname).to_raw_string().into()), "\\\\"))?
                        }
                        None => {
                            let mut buf = String::new();
                            print_expr(ctx, &mut buf, env, ci_expr).map_err(|e| Error::Fail(format!("{}", e)) )?;
                            w.write(lstrip(&buf, "\\\\"))?
                        }
                    }
                    wrap_by_paren(w, |w| {
                        concat_by(w, ", ", es, |w, e| print_expr(ctx, w, env, e))?;
                        match unpacked_element {
                            None => Ok(()),
                            Some(e) => {
                                w.write(", ")?;
                                print_expr(ctx, w, env, e)
                            }
                        }
                    })
                }
                None => not_impl!()
            }
        }
        E_::Record(r) => {
            w.write(lstrip(adjust_id(env, &(r.0).1).as_ref(), "\\\\"))?;
            print_key_values(ctx, w, env, &r.2)
        }
        E_::ClassConst(cc) => {
            if let Some(e1) = (cc.0).1.as_ciexpr() {
                handle_possible_colon_colon_class_expr(ctx, w, env, false, expr)?.map_or_else(|| {
                    let s2 = &(cc.1).1;
                    match e1.1.as_id() {
                        Some(ast_defs::Id(_, s1)) => {
                            let s1 = get_class_name_from_id(ctx, env.codegen_env, true, true, s1);
                            concat_str_by(w, "::", [&s1.into(), s2])
                        }
                        _ => {
                            print_expr(ctx, w, env, e1)?;
                            w.write("::")?;
                            w.write(s2)
                        }
                    }
                },
            |x| Ok(x))
            } else {
                Err(Error::fail("TODO: Only expected CIexpr in class_const"))
            }
        }
        E_::Unop(u) => match u.0 {
            ast::Uop::Upincr => {
                print_expr(ctx, w, env, &u.1)?;
                w.write("++")
            }
            ast::Uop::Updecr => {
                print_expr(ctx, w, env, &u.1)?;
                w.write("--")
            }
            _ => {
                print_uop(w, u.0)?;
                print_expr(ctx, w, env, &u.1)
            }
        },
        E_::ObjGet(og) => {
            print_expr(ctx, w, env, &og.0)?;
            w.write(match og.2 {
                ast::OgNullFlavor::OGNullthrows => "->",
                ast::OgNullFlavor::OGNullsafe => "\\?->",
            })?;
            print_expr(ctx, w, env, &og.1)
        }
        E_::Clone(e) => {
            w.write("clone ")?;
            print_expr(ctx, w, env, e)
        }
        E_::ArrayGet(ag) => not_impl!(),
        E_::String2(ss) => concat_by(w, " . ", ss, |w, s| print_expr(ctx, w, env, s)),
        E_::PrefixedString(s) => {
            w.write(&s.0)?;
            w.write(" . ")?;
            print_expr(ctx, w, env, &s.1)
        }
        E_::Eif(eif) => {
            print_expr(ctx, w, env, &eif.0)?;
            w.write(" \\? ")?;
            option(w, &eif.1, |w, etrue| print_expr(ctx, w, env, etrue))?;
            w.write(" : ")?;
            print_expr(ctx, w, env, &eif.2)
        }
        E_::BracedExpr(e) => wrap_by_braces(w, |w| print_expr(ctx, w, env, e)),
        E_::ParenthesizedExpr(e) => wrap_by_paren(w, |w| print_expr(ctx, w, env, e)),
        E_::Cast(c) => {
            wrap_by_paren(w, |w| print_hint(w, &c.0))?;
            print_expr(ctx, w, env, &c.1)
        }
        E_::Pipe(p) => {
            print_expr(ctx, w, env, &p.1)?;
            w.write(" |> ")?;
            print_expr(ctx, w, env, &p.2)
        }
        E_::Is(i) => {
            print_expr(ctx, w, env, &i.0)?;
            w.write(" is ")?;
            print_hint(w, &i.1)
        }
        E_::As(a) => {
            print_expr(ctx, w, env, &a.0)?;
            w.write(if a.2 { " ?as " } else { " as " })?;
            print_hint(w, &a.1)
        }
        E_::Varray(va) => wrap_by_(w, "varray[", "]", |w| {
            concat_by(w, ", ", &va.1, |w, e| print_expr(ctx, w, env, e))
        }),
        E_::Darray(da) => print_expr_darray(ctx, w, env, print_expr, &da.1),
        E_::List(l) => wrap_by_(w, "list(", ")", |w| {
            concat_by(w, ", ", l, |w, i| print_expr(ctx, w, env, i))
        }),
        E_::Yield(y) => {
            w.write("yield ")?;
            print_afield(ctx, w, env, y)
        }
        E_::Await(e) => {
            w.write("await ")?;
            print_expr(ctx, w, env, e)
        }
        E_::YieldBreak => w.write("return"),
        E_::YieldFrom(e) => {
            w.write("yield from ")?;
            print_expr(ctx, w, env, e)
        }
        E_::Import(i) => {
            print_import_flavor(w, &i.0)?;
            w.write(" ")?;
            print_expr(ctx, w, env, &i.1)
        }
        E_::Xml(_) => not_impl!(),
        E_::Efun(f) => print_efun(ctx, w, env, &f.0, &f.1),
        E_::Omitted => Ok(()),
        E_::Lfun(_) => Err(Error::fail(
            "expected Lfun to be converted to Efun during closure conversion print_expr",
        )),
        E_::Suspend(_) | E_::Callconv(_) | E_::ExprList(_) => {
            Err(Error::fail("illegal default value"))
        },
        _ => Err(Error::fail(
            "TODO Unimplemented: We are missing a lot of cases in the case match. Delete this catchall"
        ))
    }
}

fn print_efun<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    f: &ast::Fun_,
    use_list: &[ast::Lid],
) -> Result<(), W::Error> {
    w.write_if(f.static_, "static ")?;
    w.write_if(
        f.fun_kind.is_fasync() || f.fun_kind.is_fasync_generator(),
        "async ",
    )?;
    w.write("function ")?;
    wrap_by_paren(w, |w| {
        concat_by(w, ", ", &f.params, |w, p| print_fparam(ctx, w, env, p))
    })?;
    w.write(" ")?;
    if !use_list.is_empty() {
        w.write("use ")?;
        wrap_by_paren(w, |w| {
            concat_by(w, ", ", use_list, |w: &mut W, ast::Lid(_, id)| {
                w.write(local_id::get_name(id))
            })
        })?;
    }
    print_block(w, env, &f.body.ast)
}

fn print_block<W: Write>(w: &mut W, env: &ExprEnv, block: &ast::Block) -> Result<(), W::Error> {
    not_impl!()
}

fn print_fparam<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    env: &ExprEnv,
    param: &ast::FunParam,
) -> Result<(), W::Error> {
    if let Some(ast_defs::ParamKind::Pinout) = param.callconv {
        w.write("inout ")?;
    }
    if param.is_variadic {
        w.write("...")?;
    }
    option(w, &(param.type_hint).1, |w, h| {
        print_hint(w, h)?;
        w.write(" ")
    })?;
    w.write(&param.name)?;
    option(w, &param.expr, |w, e| {
        w.write(" = ")?;
        print_expr(ctx, w, env, e)
    })
}

fn print_bop<W: Write>(w: &mut W, bop: &ast_defs::Bop) -> Result<(), W::Error> {
    use ast_defs::Bop;
    match bop {
        Bop::Plus => w.write("+"),
        Bop::Minus => w.write("-"),
        Bop::Star => w.write("*"),
        Bop::Slash => w.write("/"),
        Bop::Eqeq => w.write("=="),
        Bop::Eqeqeq => w.write("==="),
        Bop::Starstar => w.write("**"),
        Bop::Eq(None) => w.write("="),
        Bop::Eq(Some(bop)) => {
            w.write("=")?;
            print_bop(w, bop)
        }
        Bop::Ampamp => w.write("&&"),
        Bop::Barbar => w.write("||"),
        Bop::Lt => w.write("<"),
        Bop::Lte => w.write("<="),
        Bop::Cmp => w.write("<=>"),
        Bop::Gt => w.write(">"),
        Bop::Gte => w.write(">="),
        Bop::Dot => w.write("."),
        Bop::Amp => w.write("&"),
        Bop::Bar => w.write("|"),
        Bop::Ltlt => w.write("<<"),
        Bop::Gtgt => w.write(">>"),
        Bop::Percent => w.write("%"),
        Bop::Xor => w.write("^"),
        Bop::LogXor => w.write("xor"),
        Bop::Diff => w.write("!="),
        Bop::Diff2 => w.write("!=="),
        Bop::QuestionQuestion => w.write("\\?\\?"),
    }
}

fn print_hint<W: Write>(w: &mut W, hint: &ast::Hint) -> Result<(), W::Error> {
    not_impl!()
}

fn print_import_flavor<W: Write>(w: &mut W, flavor: &ast::ImportFlavor) -> Result<(), W::Error> {
    use ast::ImportFlavor as F;
    w.write(match flavor {
        F::Include => "include",
        F::Require => "require",
        F::IncludeOnce => "include_once",
        F::RequireOnce => "require_once",
    })
}

fn print_param_user_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    param: &HhasParam,
) -> Result<(), W::Error> {
    match &param.user_attributes[..] {
        [] => Ok(()),
        _ => wrap_by_square(w, |w| print_attributes(ctx, w, &param.user_attributes)),
    }
}

fn string_of_span(&Span(line_begin, line_end): &Span) -> String {
    format!("({},{})", line_begin, line_end)
}

fn print_fun_attrs<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    f: &HhasFunction,
) -> Result<(), W::Error> {
    use hhas_attribute::*;
    let user_attrs = &f.attributes;
    let mut special_attrs = vec![];
    if let Ok(attr) = f.rx_level.try_into() {
        special_attrs.push(attr);
    }
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
    if !f.is_top() {
        special_attrs.push("nontop");
    }
    if ctx.is_system_lib() || (has_dynamically_callable(user_attrs) && !f.is_memoize_impl()) {
        special_attrs.push("dyn_callable")
    }
    print_special_and_user_attrs(ctx, w, &special_attrs, user_attrs)
}

fn print_special_and_user_attrs<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    specials: &[&str],
    users: &[HhasAttribute],
) -> Result<(), W::Error> {
    if !users.is_empty() || !specials.is_empty() {
        wrap_by_square(w, |w| {
            concat_str_by(w, " ", specials)?;
            if !specials.is_empty() && !users.is_empty() {
                w.write(" ")?;
            }
            print_attributes(ctx, w, users)
        })?;
        w.write(" ")?;
    }
    Ok(())
}

fn print_upper_bounds<W: Write>(
    w: &mut W,
    ubs: impl AsRef<[(String, Vec<HhasTypeInfo>)]>,
) -> Result<(), W::Error> {
    wrap_by_braces(w, |w| concat_by(w, ", ", ubs, print_upper_bound))
}

fn print_upper_bound<W: Write>(
    w: &mut W,
    (id, tys): &(String, Vec<HhasTypeInfo>),
) -> Result<(), W::Error> {
    wrap_by_paren(w, |w| {
        concat_str_by(w, " ", [id.as_str(), "as", ""])?;
        concat_by(w, ", ", &tys, print_type_info)
    })
}

fn print_type_info<W: Write>(w: &mut W, ti: &HhasTypeInfo) -> Result<(), W::Error> {
    print_type_info_(w, false, ti)
}

fn print_type_flags<W: Write>(w: &mut W, flag: constraint::Flags) -> Result<(), W::Error> {
    let mut first = true;
    let mut print_space = |w: &mut W| -> Result<(), W::Error> {
        if !first {
            w.write(" ")
        } else {
            Ok(first = false)
        }
    };
    use constraint::Flags as F;
    if flag.contains(F::DISPLAY_NULLABLE) {
        print_space(w)?;
        w.write("display_nullable")?;
    }
    if flag.contains(F::EXTENDED_HINT) {
        print_space(w)?;
        w.write("extended_hint")?;
    }
    if flag.contains(F::NULLABLE) {
        print_space(w)?;
        w.write("nullable")?;
    }

    if flag.contains(F::SOFT) {
        print_space(w)?;
        w.write("soft")?;
    }
    if flag.contains(F::TYPE_CONSTANT) {
        print_space(w)?;
        w.write("type_constant")?;
    }

    if flag.contains(F::TYPE_VAR) {
        print_space(w)?;
        w.write("type_var")?;
    }

    if flag.contains(F::UPPERBOUND) {
        print_space(w)?;
        w.write("upperbound")?;
    }
    Ok(())
}

fn print_type_info_<W: Write>(w: &mut W, is_enum: bool, ti: &HhasTypeInfo) -> Result<(), W::Error> {
    let print_quote_str = |w: &mut W, opt: &Option<String>| {
        option_or(
            w,
            opt,
            |w, s: &String| wrap_by_quotes(w, |w| w.write(escape(s))),
            "N",
        )
    };
    wrap_by_angle(w, |w| {
        print_quote_str(w, &ti.user_type)?;
        w.write(" ")?;
        if !is_enum {
            print_quote_str(w, &ti.type_constraint.name)?;
            w.write(" ")?;
        }
        print_type_flags(w, ti.type_constraint.flags)
    })
}

fn print_typedef_info<W: Write>(w: &mut W, ti: &HhasTypeInfo) -> Result<(), W::Error> {
    wrap_by_angle(w, |w| {
        w.write(quote_string(
            ti.type_constraint.name.as_ref().map_or("", |n| n.as_str()),
        ))?;
        let flags = ti.type_constraint.flags & constraint::Flags::NULLABLE;
        if !flags.is_empty() {
            wrap_by(w, " ", |w| {
                print_type_flags(w, ti.type_constraint.flags & constraint::Flags::NULLABLE)
            })?;
        }
        Ok(())
    })
}

fn print_extends<W: Write>(w: &mut W, base: Option<&str>) -> Result<(), W::Error> {
    match base {
        None => Ok(()),
        Some(b) => concat_str_by(w, " ", [" extends", b]),
    }
}

fn print_record_field<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    Field(name, type_info, intial_value): &Field,
) -> Result<(), W::Error> {
    ctx.newline(w)?;
    w.write(".property ")?;
    match intial_value {
        Some(_) => w.write("[public] ")?,
        None => w.write("[public sys_initial_val] ")?,
    }
    print_type_info(w, type_info)?;
    concat_str_by(w, " ", ["", name, "="])?;

    ctx.block(w, |c, w| {
        c.newline(w)?;
        match intial_value {
            None => w.write("uninit")?,
            Some(value) => wrap_by_triple_quotes(w, |w| print_adata(c, w, value))?,
        }
        w.write(";")
    })
}

fn print_record_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    record: &HhasRecord,
) -> Result<(), W::Error> {
    newline(w)?;
    if record.is_abstract {
        concat_str_by(w, " ", [".record", record.name.to_raw_string()])?;
    } else {
        concat_str_by(w, " ", [".record", "[final]", record.name.to_raw_string()])?;
    }
    print_extends(w, record.base.as_ref().map(|b| b.to_raw_string()))?;
    w.write(" ")?;

    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| {
            concat(w, &record.fields, |w, rf| print_record_field(c, w, rf))
        })?;
        ctx.newline(w)
    })?;
    newline(w)
}

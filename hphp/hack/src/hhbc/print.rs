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

use escaper::escape;
use hhas_attribute_rust::{self as hhas_attribute, HhasAttribute};
use hhas_body_rust::HhasBody;
use hhas_class_rust::{self as hhas_class, HhasClass};
use hhas_function_rust::HhasFunction;
use hhas_method_rust::HhasMethod;
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_program_rust::HhasProgram;
use hhas_property_rust::HhasProperty;
use hhas_record_def_rust::{Field, HhasRecord};
use hhas_type::{constraint, Info as HhasTypeInfo};
use hhas_type_const::HhasTypeConstant;
use hhbc_ast_rust::*;
use hhbc_id_rust::{class, Id};
use hhbc_string_utils_rust::{
    float, quote_string_with_escape, strip_global_ns, triple_quote_string,
};
use instruction_sequence_rust::InstrSeq;
use options::Options;
use oxidized::{ast, ast_defs, doc_comment::DocComment, relative_path::RelativePath};
use runtime::TypedValue;
use write::*;

const ADATA_ARRAY_PREFIX: &str = "a";
const ADATA_VARRAY_PREFIX: &str = "y";
const ADATA_VEC_PREFIX: &str = "v";
const ADATA_DICT_PREFIX: &str = "D";
const ADATA_DARRAY_PREFIX: &str = "Y";
const ADATA_KEYSET_PREFIX: &str = "k";

/// Indent is an abstraction of indentation. Configurable indentation
/// and perf tweaking will be easier.
pub struct Indent(usize);

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
    pub opts: &'a Options,
    pub path: Option<&'a RelativePath>,

    dump_symbol_refs: bool,
    indent: Indent,
}

impl<'a> Context<'a> {
    pub fn new(opts: &'a Options, path: Option<&'a RelativePath>, dump_symbol_refs: bool) -> Self {
        Self {
            opts,
            path,
            dump_symbol_refs,
            indent: Indent::new(),
        }
    }

    fn dump_symbol_refs(&self) -> bool {
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

    /// Printing instruction list requies manually control indentation,
    /// where indent_inc/indent_dec are called
    pub fn indent_inc(&mut self) {
        self.indent.inc();
    }

    pub fn indent_dec(&mut self) {
        self.indent.dec();
    }
}

pub fn print_program<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    prog: &HhasProgram,
) -> Result<(), W::Error> {
    match ctx.path {
        Some(p) => {
            let p = &escape(p.to_absoute().to_str().ok_or(Error::InvalidUTF8)?);

            concat_str_by(w, " ", ["#", p, "starts here"])?;

            newline(w)?;

            newline(w)?;
            concat_str(w, [".filepath ", format!("\"{}\"", p).as_str(), ";"])?;

            newline(w)?;
            handle_not_impl(|| print_program_(ctx, w, prog))?;

            newline(w)?;
            concat_str_by(w, " ", ["#", p, "ends here"])?;

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
    newline(w)?;

    print_data_region(w, vec![])?;
    print_main(ctx, w, &prog.main)?;
    concat(w, &prog.functions, |w, f| print_fun_def(ctx, w, f))?;
    concat(w, &prog.record_defs, |w, rd| print_record_def(ctx, w, rd))?;
    concat(w, &prog.classes, |w, cd| print_class_def(ctx, w, cd))?;
    print_file_attributes(ctx, w, &prog.file_attributes)?;

    if ctx.dump_symbol_refs() {
        return not_impl!();
    }
    Ok(())
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
    w.write(".function ")?;
    if ctx.opts.enforce_generic_ub() {
        print_upper_bounds(w, &body.upper_bounds)?;
    }
    print_fun_attrs(w, fun_def)?;
    if ctx.opts.source_map() {
        w.write(string_of_span(&fun_def.span))?;
        w.write(" ")?;
    }
    option(w, body.return_type_info.as_ref(), print_type_info)?;
    w.write(" ")?;
    w.write(fun_def.name.to_raw_string())?;
    print_params(w, fun_def.params())?;
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
    wrap_by_braces(w, |w| print_body(ctx, w, body))
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
        ctx.block(w, |ctx, w| {
            ctx.newline(w)?;
            w.write(format!(".doc {};", triple_quote_string(&cmt.0)))
        })?;
    }
    Ok(())
}

fn print_use_precedence<W: Write, X>(ctx: &mut Context, w: &mut W, _: X) -> Result<(), W::Error> {
    not_impl!()
}

fn print_use_alias<W: Write, X>(ctx: &mut Context, w: &mut W, _: X) -> Result<(), W::Error> {
    not_impl!()
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
    if mtr.fun_kind.is_async() {
        w.write("async ")?;
    }
    w.write("[")?;
    if mtr.final_ {
        w.write("final ")?;
    }
    w.write(mtr.visibility.to_string())?;
    if mtr.abstract_ {
        w.write(" abstract")?;
    }
    if mtr.static_ {
        w.write(" static")?;
    }
    w.write("] ")?;
    w.write(format!("{};", mtr.name.1))
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
            for x in &c.use_precedences {
                print_use_precedence(ctx, w, x)?;
            }
            for x in &c.use_aliases {
                print_use_alias(ctx, w, x)?;
            }
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
    let is_system_lib = false; // TODO(hrust)

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

fn print_method_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    method_def: &HhasMethod,
) -> Result<(), W::Error> {
    newline(w)?;
    w.write("  .method ")?;
    w.write(method_def.name.to_raw_string())?;
    w.write(" {")?;
    w.write("  }")?;
    not_impl!()
}

fn print_class_def<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    class_def: &HhasClass,
) -> Result<(), W::Error> {
    newline(w)?;
    w.write(".class ")?;
    if ctx.opts.enforce_generic_ub() {
        print_upper_bounds(w, &class_def.upper_bounds)?;
    }
    print_class_special_attributes(ctx, w, class_def)?;
    w.write(class_def.name.to_raw_string())?;
    if ctx.opts.source_map() {
        w.write(format!(" {}", string_of_span(&class_def.span)))?;
    }
    print_extends(w, class_def.base.as_ref().map(|x| x.to_raw_string()))?;
    print_implements(w, &class_def.implements)?;
    w.write(" {")?;
    print_doc_comment(ctx, w, &class_def.doc_comment)?;
    print_uses(ctx, w, class_def)?;
    print_enum_ty(ctx, w, class_def)?;
    for x in &class_def.requirements {
        print_requirement(ctx, w, x)?;
    }
    for x in &class_def.type_constants {
        print_type_constant(ctx, w, x)?;
    }
    for x in &class_def.properties {
        print_property(ctx, w, class_def, x)?;
    }
    for m in &class_def.methods {
        print_method_def(ctx, w, m)?;
    }
    newline(w)?;
    w.write("}")?;
    newline(w)
}

fn pos_to_prov_tag(ctx: &Context, loc: &Option<ast_defs::Pos>) -> String {
    match loc {
        Some(_) if ctx.opts.array_provenance() => unimplemented!(),
        _ => "".into(),
    }
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
            print_adata_dict_collection_argument(ctx, w, ADATA_DICT_PREFIX, loc, pairs)
        }
        TypedValue::Vec((values, loc)) => {
            print_adata_collection_argument(ctx, w, ADATA_VEC_PREFIX, loc, values)
        }
        TypedValue::DArray((pairs, loc)) => {
            print_adata_dict_collection_argument(ctx, w, ADATA_DARRAY_PREFIX, loc, pairs)
        }
        TypedValue::Array(pairs) => {
            print_adata_dict_collection_argument(ctx, w, ADATA_ARRAY_PREFIX, &None, pairs)
        }
        TypedValue::Keyset(values) => {
            print_adata_collection_argument(ctx, w, ADATA_KEYSET_PREFIX, &None, values)
        }
        _ => not_impl!(),
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
        ADATA_VARRAY_PREFIX,
        a.arguments.len()
    ))?;
    concat(w, &a.arguments, |w, arg| print_adata(ctx, w, arg))?;
    w.write("}\"\"\")")
}

fn print_attributes<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    al: &Vec<HhasAttribute>,
) -> Result<(), W::Error> {
    // Adjust for underscore coming before alphabet
    let al: Vec<&HhasAttribute> = al
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
    if ctx.opts.source_map() {
        w.write("(1,1) ")?;
    }
    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| print_body(c, w, body))?;
        newline(w)
    })?;
    newline(w)
}

fn print_body<W: Write>(ctx: &mut Context, w: &mut W, body: &HhasBody) -> Result<(), W::Error> {
    // TODO(hrust): add `add_doc buf indent (Hhas_body.doc_comment body);`
    if body.is_memoize_wrapper {
        ctx.newline(w)?;
        w.write(".ismemoizewrapper;")?;
    }
    if body.is_memoize_wrapper_lsb {
        ctx.newline(w)?;
        w.write(".ismemoizewrapperlsb;")?;
    }
    // TODO(hrust):
    // add_num_iters buf indent (Hhas_body.num_iters body);
    // add_decl_vars buf indent (Hhas_body.decl_vars body);
    print_instructions(ctx, w, &body.body_instrs)
}

fn print_instructions<W: Write>(
    ctx: &mut Context,
    w: &mut W,
    instr_seq: &InstrSeq,
) -> Result<(), W::Error> {
    use Instruct::*;
    use InstructTry::*;
    for instr in instr_seq.iter() {
        match instr {
            ISpecialFlow(_) => return not_impl!(),
            IComment(_) => {
                // indetation = 0
                newline(w)?;
                print_instr(w, instr)?;
            }
            ILabel(_) => {
                ctx.indent_dec();
                ctx.newline(w)?;
                print_instr(w, instr)?;
                ctx.indent_inc();
            }
            ITry(TryCatchBegin) => {
                ctx.newline(w)?;
                print_instr(w, instr)?;
                ctx.indent_inc();
            }
            ITry(TryCatchMiddle) => {
                ctx.indent_dec();
                ctx.newline(w)?;
                print_instr(w, instr)?;
                ctx.indent_inc();
            }
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

fn print_instr<W: Write>(w: &mut W, instr: &Instruct) -> Result<(), W::Error> {
    use Instruct::*;
    match instr {
        IIterator(_) => not_impl!(),
        IBasic(_) => not_impl!(),
        ILitConst(lit) => print_lit_const(w, lit),
        IOp(op) => print_op(w, op),
        IContFlow(cf) => print_control_flow(w, cf),
        ICall(_) => not_impl!(),
        IMisc(_) => not_impl!(),
        IGet(_) => not_impl!(),
        IMutator(_) => not_impl!(),
        ILabel(_) => not_impl!(),
        IIsset(_) => not_impl!(),
        IBase(_) => not_impl!(),
        IFinal(_) => not_impl!(),
        ITry(_) => not_impl!(),
        IComment(s) => concat_str_by(w, " ", ["#", s.as_str()]),
        ISrcLoc(_) => not_impl!(),
        IAsync(_) => not_impl!(),
        IGenerator(_) => not_impl!(),
        IIncludeEvalDefine(_) => not_impl!(),
        IGenDelegation(_) => not_impl!(),
        _ => Err(Error::Fail("invalid instruction".into())),
    }
}

fn print_control_flow<W: Write>(w: &mut W, cf: &InstructControlFlow) -> Result<(), W::Error> {
    use InstructControlFlow::*;
    match cf {
        Jmp(l) => not_impl!(),
        JmpNS(l) => not_impl!(),
        JmpZ(l) => not_impl!(),
        JmpNZ(l) => not_impl!(),
        RetC => w.write("RetC"),
        RetCSuspended => w.write("RetCSuspended"),
        RetM(p) => not_impl!(),
        Throw => w.write("Throw"),
        Switch(_, _, _) => not_impl!(),
        SSwitch(_) => not_impl!(),
    }
}

fn print_lit_const<W: Write>(w: &mut W, lit: &InstructLitConst) -> Result<(), W::Error> {
    use InstructLitConst::*;
    match lit {
        Null => w.write("Null"),
        Int(i) => concat_str_by(w, " ", ["Int", i.to_string().as_str()]),
        String(s) => {
            w.write("String ")?;
            wrap_by_quotes(w, |w| w.write(s))
        }
        _ => not_impl!(),
    }
}

fn print_op<W: Write>(w: &mut W, op: &InstructOperator) -> Result<(), W::Error> {
    use InstructOperator::*;
    match op {
        Fatal(fatal_op) => print_fatal_op(w, fatal_op),
        _ => not_impl!(),
    }
}

fn print_fatal_op<W: Write>(w: &mut W, f: &FatalOp) -> Result<(), W::Error> {
    use FatalOp::*;
    match f {
        Parse => w.write("Fatal Parse"),
        Runtime => w.write("Fatal Runtime"),
        RuntimeOmitFrame => w.write("Fatal RuntimeOmitFrame"),
    }
}

fn print_params<W: Write>(w: &mut W, params: &[HhasParam]) -> Result<(), W::Error> {
    not_impl!()
}

fn string_of_span(&Span(line_begin, line_end): &Span) -> String {
    format!("({},{})", line_begin, line_end)
}

fn print_fun_attrs<W: Write>(w: &mut W, fun_def: &HhasFunction) -> Result<(), W::Error> {
    not_impl!()
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

fn print_type_info_<W: Write>(w: &mut W, is_enum: bool, ti: &HhasTypeInfo) -> Result<(), W::Error> {
    let print_flag = |w: &mut W, flag: constraint::Flags| {
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
    };
    let print_quote_str = |w: &mut W, opt: &Option<String>| {
        option_or(
            w,
            opt.as_ref(),
            |w, s| wrap_by_quotes(w, |w| w.write(escape(s.as_ref()))),
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
        print_flag(w, ti.type_constraint.flags)
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
            Some(value) => wrap_by_quotes(w, |w| print_adata(c, w, value))?,
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

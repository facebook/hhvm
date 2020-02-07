// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(unused_variables)]

mod print_env;
mod write;

use itertools::Itertools;
pub use write::{Error, IoWrite, Result, Write};

use escaper::escape;
use hhas_attribute_rust::HhasAttribute;
use hhas_body_rust::HhasBody;
use hhas_function_rust::HhasFunction;
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_program_rust::HhasProgram;
use hhas_record_def_rust::{Field, HhasRecord};
use hhas_type::Info as HhasTypeInfo;
use hhbc_ast_rust::*;
use hhbc_id_rust::Id;
use hhbc_string_utils_rust::quote_string_with_escape;
use instruction_sequence_rust::InstrSeq;
use options::Options;
use oxidized::relative_path::RelativePath;
use runtime::TypedValue;
use write::*;

const ADATA_VARRAY_PREFIX: &str = "y";

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
            newline(w)?;
            concat_str_by(w, " ", ["#", p, "ends here"])?;

            newline(w)
        }
        None => {
            w.write("#starts here")?;

            newline(w)?;
            handle_not_impl(|| print_program_(ctx, w, prog))?;

            newline(w)?;
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
    // TODO(hrust):
    // add_data_region buf adata;
    print_main(ctx, w, &prog.main)?;
    concat(w, &prog.record_defs, print_record_def)?;
    concat(w, &prog.functions, |w, f| print_fun_def(ctx, w, f))?;
    print_file_attributes(ctx, w, &prog.file_attributes)?;

    if ctx.dump_symbol_refs() {
        return not_impl!();
    }
    Ok(())
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
        print_span(w, &fun_def.span)?;
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

fn print_adata<W: Write>(ctx: &mut Context, w: &mut W, tv: &TypedValue) -> Result<(), W::Error> {
    match tv {
        TypedValue::String(s) => w.write(format!("s:{}:{};", s.len(), quote_string_with_escape(s))),
        TypedValue::Int(i) => w.write(format!("i:{};", i)),
        _ => unimplemented!("{:?}", tv),
    }
}

fn print_file_attribute<W: Write>(
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
    w.write("}\"\"\")")?;

    Ok(())
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
    newline(w)?;
    w.write(".file_attributes [")?;

    // Adjust for underscore coming before alphabet
    let al: Vec<&HhasAttribute> = al
        .iter()
        .sorted_by_key(|a| (!a.name.starts_with("__"), &a.name))
        .collect();

    concat_by(w, " ", &al, |w, a| print_file_attribute(ctx, w, a))?;
    w.write("] ;")?;

    Ok(())
}

fn print_main<W: Write>(ctx: &mut Context, w: &mut W, body: &HhasBody) -> Result<(), W::Error> {
    newline(w)?;
    w.write(".main ")?;
    if ctx.opts.source_map() {
        w.write("(1,1) ")?;
    }
    wrap_by_braces(w, |w| {
        ctx.block(w, |c, w| print_body(c, w, body))?;
        newline(w)
    })
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
        IOp(_) => not_impl!(),
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
        _ => not_impl!(),
    }
}

fn print_params<W: Write>(w: &mut W, params: &[HhasParam]) -> Result<(), W::Error> {
    not_impl!()
}

fn print_span<W: Write>(w: &mut W, span: &Span) -> Result<(), W::Error> {
    not_impl!()
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

fn print_type_info<W: Write>(w: &mut W, ty: &HhasTypeInfo) -> Result<(), W::Error> {
    not_impl!()
}

fn print_extends<W: Write>(w: &mut W, base: Option<&str>) -> Result<(), W::Error> {
    match base {
        None => Ok(()),
        Some(b) => concat_str_by(w, " ", ["extends", b]),
    }
}

fn print_record_field<W: Write>(w: &mut W, field: &Field) -> Result<(), W::Error> {
    newline(w)
}

fn print_record_def<W: Write>(w: &mut W, record: &HhasRecord) -> Result<(), W::Error> {
    newline(w)?;
    if record.is_abstract {
        concat_str_by(w, " ", [".record", record.name.to_raw_string()])?;
    } else {
        concat_str_by(w, " ", [".record", "[final]", record.name.to_raw_string()])?;
    }
    w.write(" ")?;
    print_extends(w, record.base.as_ref().map(|b| b.to_raw_string()))?;
    w.write(" ")?;

    wrap_by_braces(w, |w| {
        concat(w, &record.fields, print_record_field)?;
        newline(w)
    })?;
    newline(w)
}

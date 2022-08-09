// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashMap;
use std::collections::VecDeque;
use std::ffi::OsString;
use std::fmt;
use std::fs;
use std::fs::File;
use std::io::stdout;
use std::io::Write;
use std::os::unix::ffi::OsStringExt;
use std::path::Path;
use std::path::PathBuf;
use std::str::FromStr;
use std::sync::Mutex;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Context;
use anyhow::Result;
use bstr::ByteSlice;
use bumpalo::Bump;
use clap::Parser;
use ffi::Maybe;
use ffi::Pair;
use ffi::Slice;
use ffi::Str;
use log::trace;
use naming_special_names_rust::coeffects::Ctx;
use once_cell::sync::OnceCell;
use options::Options;
use oxidized::relative_path;
use oxidized::relative_path::RelativePath;
use rayon::prelude::*;
use regex::bytes::Regex;

use crate::regex;
use crate::FileOpts;

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    /// The input hhas file(s) to assemble back to HackCUnit
    #[clap(flatten)]
    files: FileOpts,

    /// Print tokens
    #[clap(short = 't')]
    print_toks: bool,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

pub fn run(mut opts: Opts) -> Result<()> {
    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };
    let files = opts.files.gather_input_files()?;
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &opts, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Assemble the hhas in a given file to a HackCUnit. Then use bytecode printer
/// to write the hhas representation of that HCU to output.
pub fn process_one_file(f: &Path, opts: &Opts, w: &SyncWrite) -> Result<()> {
    let alloc = Bump::default();
    let (hcu, fp) = assemble(&alloc, f, opts)?;
    let filepath = RelativePath::make(relative_path::Prefix::Dummy, fp);
    let comp_options: Options = Default::default();
    let ctxt = bytecode_printer::Context::new(&comp_options, Some(&filepath), false);
    let mut output = Vec::new();
    match bytecode_printer::print_unit(&ctxt, &mut output, &hcu) {
        Err(e) => {
            eprintln!("Error bytecode_printing file {}: {}", f.display(), e);
            Err(anyhow!("bytecode_printer problem"))
        }
        Ok(_) => {
            w.lock().unwrap().write_all(&output)?;
            Ok(())
        }
    }
}

/// Assembles the hhas within f to a HackCUnit
pub fn assemble<'arena>(
    alloc: &'arena Bump,
    f: &Path,
    opts: &Opts,
) -> Result<(hhbc::hackc_unit::HackCUnit<'arena>, PathBuf)> {
    let s: Vec<u8> = fs::read(f)?;
    assemble_from_bytes(alloc, &s, opts.print_toks)
}

/// Assembles the hhas represented by the slice of bytes input
pub fn assemble_from_bytes<'arena>(
    alloc: &'arena Bump,
    s: &[u8],
    print_toks: bool,
) -> Result<(hhbc::hackc_unit::HackCUnit<'arena>, PathBuf)> {
    if print_toks {
        print_tokens(s);
    }
    let mut lex = Lexer::from_slice(s, 1);
    let unit = assemble_from_toks(alloc, &mut lex)?;
    trace!("ASM UNIT: {unit:?}");
    Ok(unit)
}

/// Assembles the HCU. Parses over the top level of the .hhas file
/// File is either empty OR looks like:
/// .filepath <str_literal>
/// (.function <...>)+
/// (.function_refs <...>)+
fn assemble_from_toks<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<(hhbc::hackc_unit::HackCUnit<'arena>, PathBuf)> {
    let mut funcs = Vec::new();
    let mut classes = Vec::new();
    let mut adatas = Vec::new();
    let mut func_refs = None;
    let mut class_refs = None;
    let mut constant_refs = None;
    let mut include_refs = None;
    let mut typedefs = Vec::new();
    let mut constants = Vec::new();
    let mut type_constants = Vec::new();
    let mut fatal = None;
    let mut file_attributes = Vec::new();
    let mut module_use = Maybe::Nothing;
    let mut modules = Vec::new();
    // First token should be the filepath
    let fp = assemble_filepath(token_iter)?;
    while token_iter.peek().is_some() {
        if token_iter.peek_if_str(Token::is_decl, ".fatal") {
            fatal = Some(assemble_fatal(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".adata") {
            adatas.push(assemble_adata(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".function") {
            funcs.push(assemble_function(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".class") {
            classes.push(assemble_class(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".function_refs") {
            ensure!(
                func_refs.is_none(),
                "Func refs defined multiple times in file"
            );

            func_refs = Some(assemble_refs(
                alloc,
                token_iter,
                ".function_refs",
                assemble_function_name,
            )?);
        } else if token_iter.peek_if_str(Token::is_decl, ".class_refs") {
            ensure!(
                class_refs.is_none(),
                "Class refs defined multiple times in file"
            );

            class_refs = Some(assemble_refs(
                alloc,
                token_iter,
                ".class_refs",
                assemble_class_name,
            )?);
        } else if token_iter.peek_if_str(Token::is_decl, ".constant_refs") {
            ensure!(
                constant_refs.is_none(),
                "Constant refs defined multiple times in file"
            );

            constant_refs = Some(assemble_refs(
                alloc,
                token_iter,
                ".constant_refs",
                assemble_const_name,
            )?);
        } else if token_iter.peek_if_str(Token::is_decl, ".includes") {
            ensure!(
                include_refs.is_none(),
                "Includes defined mulitple times in file"
            );

            include_refs = Some(assemble_refs(
                alloc,
                token_iter,
                ".includes",
                |alloc, t| {
                    let path_str = if t.peek_if(Token::is_decl) {
                        t.expect_decl_into_ffi_str(alloc)?
                    } else {
                        t.expect_identifier_into_ffi_str(alloc)?
                    };
                    Ok(hhbc::hhas_symbol_refs::IncludePath::Absolute(path_str))
                },
            )?)
        } else if token_iter.peek_if_str(Token::is_decl, ".alias") {
            typedefs.push(assemble_typedef(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".const") {
            assemble_const_or_type_const(alloc, token_iter, &mut constants, &mut type_constants)?;

            ensure!(
                type_constants.is_empty(),
                "Type constants defined outside of a class"
            );
        } else if token_iter.peek_if_str(Token::is_decl, ".file_attributes") {
            assemble_file_attributes(alloc, token_iter, &mut file_attributes)?;
        } else if token_iter.peek_if_str(Token::is_decl, ".module_use") {
            module_use = Maybe::Just(assemble_module_use(alloc, token_iter)?);
        } else if token_iter.peek_if_str(Token::is_decl, ".module") {
            modules.push(assemble_module(alloc, token_iter)?);
        } else {
            bail!(
                "Unknown top level identifier: {}",
                token_iter.next().unwrap()
            )
        }
    }
    let hcu = hhbc::hackc_unit::HackCUnit {
        adata: Slice::fill_iter(alloc, adatas.into_iter()),
        functions: Slice::fill_iter(alloc, funcs.into_iter()),
        classes: Slice::fill_iter(alloc, classes.into_iter()),
        typedefs: Slice::fill_iter(alloc, typedefs.into_iter()),
        file_attributes: Slice::fill_iter(alloc, file_attributes.into_iter()),
        modules: Slice::from_vec(alloc, modules),
        module_use,
        symbol_refs: hhbc::hhas_symbol_refs::HhasSymbolRefs {
            functions: func_refs.unwrap_or_default(),
            classes: class_refs.unwrap_or_default(),
            constants: constant_refs.unwrap_or_default(),
            includes: include_refs.unwrap_or_default(),
        },
        constants: Slice::fill_iter(alloc, constants.into_iter()),
        fatal: fatal.into(),
    };

    Ok((hcu, fp))
}

/// Ex:
/// .module m0 (64, 64) {
///}
fn assemble_module<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_module::HhasModule<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".module")?;
    let (attr, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;

    ensure!(
        attr == hhvm_types_ffi::ffi::Attr::AttrNone,
        "Unexpected HHVM attrs in module definition"
    );

    let name = assemble_class_name(alloc, token_iter)?;
    let span = assemble_span(token_iter)?;
    token_iter.expect(Token::into_open_curly)?;
    token_iter.expect(Token::into_close_curly)?;
    Ok(hhbc::hhas_module::HhasModule {
        attributes,
        name,
        span,
    })
}

/// Ex:
/// .module_use "(module_use)";
fn assemble_module_use<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Str<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".module_use")?;
    let st = Str::new_slice(
        alloc,
        escaper::unquote_slice(token_iter.expect(Token::into_str_literal)?),
    );
    token_iter.expect(Token::into_semicolon)?;
    Ok(st)
}

/// Ex:
/// .file_attributes ["__EnableUnstableFeatures"("""v:1:{s:8:\"readonly\";}""")] ;
fn assemble_file_attributes<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    file_attributes: &mut Vec<hhbc::hhas_attribute::HhasAttribute<'arena>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".file_attributes")?;
    token_iter.expect(Token::into_open_bracket)?;
    while !token_iter.peek_if(Token::is_close_bracket) {
        file_attributes.push(assemble_user_attr(alloc, token_iter)?);
    }
    token_iter.expect(Token::into_close_bracket)?;
    token_iter.expect(Token::into_semicolon)?;
    Ok(())
}

/// Ex:
/// .alias ShapeKeyEscaping = <"HH\\darray"> (3,6) """D:2:{s:4:\"kind\";i:14;s:6:\"fields\";D:2:{s:11:\"Whomst'd've\";D:1:{s:5:\"value\";D:1:{s:4:\"kind\";i:1;}}s:25:\"Whomst\\u{0027}d\\u{0027}ve\";D:1:{s:5:\"value\";D:1:{s:4:\"kind\";i:4;}}}}""";
/// Note that in BCP, TypeDef's typeinfo's user_type is not printed. What's between <> is the typeinfo's constraint's name.
fn assemble_typedef<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_typedef::HhasTypedef<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".alias")?;
    let (attrs, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;
    let name = assemble_class_name(alloc, token_iter)?;
    token_iter.expect(Token::into_equal)?;
    if let Maybe::Just(type_info) = assemble_type_info(alloc, token_iter, TypeInfoKind::TypeDef)? {
        let span = assemble_span(token_iter)?;
        //tv
        let type_structure = assemble_triple_quoted_typed_value(alloc, token_iter)?;
        token_iter.expect(Token::into_semicolon)?;
        Ok(hhbc::hhas_typedef::HhasTypedef {
            name,
            attributes,
            type_info,
            type_structure,
            span,
            attrs,
        })
    } else {
        bail!("No type info provided to type def")
    }
}

fn assemble_class<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_class::HhasClass<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".class")?;
    let upper_bounds = assemble_upper_bounds(alloc, token_iter)?;
    let (flags, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;
    let name = assemble_class_name(alloc, token_iter)?;
    let span = assemble_span(token_iter)?;
    let base = assemble_base(alloc, token_iter)?;
    let implements = assemble_imp_or_enum_includes(alloc, token_iter, "implements")?;
    let enum_includes = assemble_imp_or_enum_includes(alloc, token_iter, "enum_includes")?;
    token_iter.expect(Token::into_open_curly)?;
    let doc_comment = assemble_doc_comment(alloc, token_iter)?;
    let uses = assemble_uses(alloc, token_iter)?;
    let enum_type = assemble_enum_ty(alloc, token_iter)?;
    // Prints content of class in this order: all requirements,
    // all constants, all type_constants, all ctx_constants, all properties, all method_defs.
    let mut requirements = Vec::new();
    while token_iter.peek_if_str(Token::is_decl, ".require") {
        requirements.push(assemble_requirement(alloc, token_iter)?)
    }
    // Both constants and type_constants start with .const
    // the differences is that type constants are printed as
    // .constant {} isType = """tv""" and constants are just .constant {} = """tv"""
    let (constants, type_constants) = if token_iter.peek_if_str(Token::is_decl, ".const") {
        assemble_constants(alloc, token_iter)?
    } else {
        (Vec::new(), Vec::new())
    };
    let mut ctx_constants = Vec::new();
    while token_iter.peek_if_str(Token::is_decl, ".ctx") {
        ctx_constants.push(assemble_ctx_constant(alloc, token_iter)?);
    }
    let mut properties = Vec::new();
    while token_iter.peek_if_str(Token::is_decl, ".property") {
        properties.push(assemble_property(alloc, token_iter)?);
    }
    let mut methods = Vec::new();
    while token_iter.peek_if_str(Token::is_decl, ".method") {
        methods.push(assemble_method(alloc, token_iter)?);
    }
    token_iter.expect(Token::into_close_curly)?;

    let hhas_class = hhbc::hhas_class::HhasClass {
        attributes,
        base,
        implements,
        enum_includes,
        name,
        span,
        uses,
        enum_type,
        methods: Slice::from_vec(alloc, methods),
        properties: Slice::from_vec(alloc, properties),
        constants: Slice::from_vec(alloc, constants),
        type_constants: Slice::from_vec(alloc, type_constants),
        ctx_constants: Slice::from_vec(alloc, ctx_constants),
        requirements: Slice::from_vec(alloc, requirements),
        upper_bounds,
        doc_comment,
        flags,
    };
    Ok(hhas_class)
}

fn is_enforced_static_coeffect(d: &&[u8]) -> bool {
    match *d {
        b"pure" | b"defaults" | b"rx" | b"zoned" | b"write_props" | b"rx_local" | b"zoned_with"
        | b"zoned_local" | b"zoned_shallow" | b"leak_safe_local" | b"leak_safe_shallow"
        | b"leak_safe" | b"read_globals" | b"globals" | b"write_this_props" | b"rx_shallow" => true,
        _ => false,
    }
}

/// Ex:
/// .ctx C isAbstract;
/// .ctx Clazy pure;
/// .ctx Cdebug isAbstract;
fn assemble_ctx_constant<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_coeffects::HhasCtxConstant<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".ctx")?;
    let name = token_iter.expect_identifier_into_ffi_str(alloc)?;
    let is_abstract = token_iter.next_if_str(Token::is_identifier, "isAbstract");
    // .ctx has slice of recognized and unrecognized constants.
    // Making an assumption that recognized ~~ static coeffects and
    // unrecognized ~~ unenforced static coeffects
    let mut tokens: Vec<&[u8]> = Vec::new();
    while !token_iter.peek_if(Token::is_semicolon) {
        tokens.push(token_iter.expect(Token::into_identifier)?);
    }
    let (r, u): (Vec<&[u8]>, Vec<&[u8]>) =
        tokens.into_iter().partition(is_enforced_static_coeffect);
    let r = r.iter().map(|s| Str::new_slice(alloc, s)).collect();
    let u = u.iter().map(|s| Str::new_slice(alloc, s)).collect();
    token_iter.expect(Token::into_semicolon)?;
    Ok(hhbc::hhas_coeffects::HhasCtxConstant {
        name,
        recognized: Slice::from_vec(alloc, r),
        unrecognized: Slice::from_vec(alloc, u),
        is_abstract,
    })
}

/// Ex:
/// .require extends <C>;
fn assemble_requirement<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Pair<hhbc::ClassName<'arena>, hhbc::hhas_class::TraitReqKind>> {
    token_iter.expect_is_str(Token::into_decl, ".require")?;
    let trq = match token_iter.expect(Token::into_identifier)? {
        b"extends" => hhbc::hhas_class::TraitReqKind::MustExtend,
        b"implements" => hhbc::hhas_class::TraitReqKind::MustImplement,
        b"class" => hhbc::hhas_class::TraitReqKind::MustBeClass,
        trq => bail!("Expected TraitReqKind, saw: {:?}", trq),
    };
    token_iter.expect(Token::into_lt)?;
    let cn = assemble_class_name(alloc, token_iter)?;
    token_iter.expect(Token::into_gt)?;
    token_iter.expect(Token::into_semicolon)?;
    Ok(Pair(cn, trq))
}

/// Ex:
/// .const MYCONST = """s:10:\"B::MYCONST\";""";
/// .const TType isType isAbstract;
fn assemble_constants<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<(
    Vec<hhbc::hhas_constant::HhasConstant<'arena>>,
    Vec<hhbc::hhas_type_const::HhasTypeConstant<'arena>>,
)> {
    let mut consts = Vec::new();
    let mut type_consts = Vec::new();
    while token_iter.peek_if_str(Token::is_decl, ".const") {
        assemble_const_or_type_const(alloc, token_iter, &mut consts, &mut type_consts)?;
    }
    Ok((consts, type_consts))
}

fn assemble_const_or_type_const<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    consts: &mut Vec<hhbc::hhas_constant::HhasConstant<'arena>>,
    type_consts: &mut Vec<hhbc::hhas_type_const::HhasTypeConstant<'arena>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".const")?;
    let name = token_iter.expect_identifier_into_ffi_str(alloc)?;
    if token_iter.next_if_str(Token::is_identifier, "isType") {
        //type const
        let is_abstract = token_iter.next_if_str(Token::is_identifier, "isAbstract");
        let initializer = if token_iter.next_if(Token::is_equal) {
            Maybe::Just(assemble_triple_quoted_typed_value(alloc, token_iter)?)
        } else {
            Maybe::Nothing
        };
        type_consts.push(hhbc::hhas_type_const::HhasTypeConstant {
            name,
            initializer,
            is_abstract,
        });
    } else {
        //const
        let name = hhbc::ConstName::new(name);
        let is_abstract = token_iter.next_if_str(Token::is_identifier, "isAbstract");
        let value = if token_iter.next_if(Token::is_equal) {
            if token_iter.next_if_str(Token::is_identifier, "uninit") {
                Maybe::Just(hhbc::TypedValue::Uninit)
            } else {
                Maybe::Just(assemble_triple_quoted_typed_value(alloc, token_iter)?)
            }
        } else {
            Maybe::Nothing
        };
        consts.push(hhbc::hhas_constant::HhasConstant {
            name,
            value,
            is_abstract,
        });
    }
    token_iter.expect(Token::into_semicolon)?;
    Ok(())
}

/// Ex:
/// .method {}{} [public abstract] (15,15) <"" N > a(<"?HH\\varray" "HH\\varray" nullable extended_hint display_nullable> $a1 = DV1("""NULL"""), <"HH\\varray" "HH\\varray" > $a2 = DV2("""varray[]""")) {
///    ...
/// }
fn assemble_method<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_method::HhasMethod<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".method")?;
    let shadowed_tparams = assemble_shadowed_tparams(alloc, token_iter)?;
    let upper_bounds = assemble_upper_bounds(alloc, token_iter)?;
    let (attr, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;
    let span = assemble_span(token_iter)?;
    let return_type_info = match token_iter.peek() {
        Some(Token::Lt(_)) => {
            assemble_type_info(alloc, token_iter, TypeInfoKind::NotEnumOrTypeDef)?
        }
        _ => Maybe::Nothing,
    };
    let name = assemble_method_name(alloc, token_iter)?;
    let mut decl_map: HashMap<Vec<u8>, u32> = HashMap::new();
    let params = assemble_params(alloc, token_iter, &mut decl_map)?;
    let flags = assemble_method_flags(token_iter)?;
    let (partial_body, coeffects) = assemble_body(alloc, token_iter, &mut decl_map)?;
    let body = hhbc::hhas_body::HhasBody {
        params,
        return_type_info,
        upper_bounds,
        shadowed_tparams,
        ..partial_body
    };
    // the visibility is printed in the attrs
    // confusion: Visibility::Internal is a mix of AttrInternal and AttrPublic?
    let visibility = determine_visibility(&attr)?;
    let met = hhbc::hhas_method::HhasMethod {
        attributes,
        visibility,
        name,
        body,
        span,
        coeffects,
        flags,
        attrs: attr,
    };
    Ok(met)
}

fn assemble_shadowed_tparams<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Slice<'arena, Str<'arena>>> {
    token_iter.expect(Token::into_open_curly)?;
    let mut stp = Vec::new();
    while token_iter.peek_if(Token::is_identifier) {
        stp.push(token_iter.expect_identifier_into_ffi_str(alloc)?);
        if !token_iter.peek_if(Token::is_close_curly) {
            token_iter.expect(Token::into_comma)?;
        }
    }
    token_iter.expect(Token::into_close_curly)?;
    Ok(Slice::from_vec(alloc, stp))
}

fn assemble_method_flags(token_iter: &mut Lexer<'_>) -> Result<hhbc::hhas_method::HhasMethodFlags> {
    let mut flag = hhbc::hhas_method::HhasMethodFlags::empty();
    while token_iter.peek_if(Token::is_identifier) {
        match token_iter.expect(Token::into_identifier)? {
            b"isPairGenerator" => flag |= hhbc::hhas_method::HhasMethodFlags::IS_PAIR_GENERATOR,
            b"isAsync" => flag |= hhbc::hhas_method::HhasMethodFlags::IS_ASYNC,
            b"isGenerator" => flag |= hhbc::hhas_method::HhasMethodFlags::IS_GENERATOR,
            b"isClosureBody" => flag |= hhbc::hhas_method::HhasMethodFlags::IS_CLOSURE_BODY,
            f => bail!("Unknown function flag: {:?}", f),
        }
    }
    Ok(flag)
}

fn assemble_property<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_property::HhasProperty<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".property")?;
    let (flags, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;
    // A doc comment is just a triple string literal : """{}"""
    let doc_comment = if token_iter.peek_if(Token::is_triple_str_literal) {
        Maybe::Just(assemble_unescaped_unquoted_triple_str(alloc, token_iter)?)
    } else {
        Maybe::Nothing
    };
    let type_info = if let Maybe::Just(ti) =
        assemble_type_info(alloc, token_iter, TypeInfoKind::NotEnumOrTypeDef)?
    {
        ti
    } else {
        bail!("No type_info for class property")
    };
    let name = assemble_prop_name(alloc, token_iter)?;
    token_iter.expect(Token::into_equal)?;
    let initial_value = assemble_property_initial_value(alloc, token_iter)?;
    token_iter.expect(Token::into_semicolon)?;
    let visibility = determine_visibility(&flags)?;
    Ok(hhbc::hhas_property::HhasProperty {
        name,
        flags,
        attributes,
        visibility,
        initial_value,
        type_info,
        doc_comment,
    })
}

fn determine_visibility(attr: &hhvm_types_ffi::ffi::Attr) -> Result<hhbc::Visibility> {
    let v = if attr.is_internal() && attr.is_public() {
        hhbc::Visibility::Internal
    } else if attr.is_public() {
        hhbc::Visibility::Public
    } else if attr.is_private() {
        hhbc::Visibility::Private
    } else if attr.is_protected() {
        hhbc::Visibility::Protected
    } else {
        bail!("No visibility specified")
    };
    Ok(v)
}

/// Initial values are printed slightly differently from typed values:
/// while a TV prints uninit as "uninit", initial value is printed as "uninit;""
/// for all other typed values, initial_value is printed nested in triple quotes.
fn assemble_property_initial_value<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Maybe<hhbc::TypedValue<'arena>>> {
    let tok = token_iter
        .peek()
        .ok_or_else(|| anyhow!("Expected init value for property, reached EOF"))?;
    let tv = match tok.as_bytes() {
        b"uninit" => {
            token_iter.expect_is_str(Token::into_identifier, "uninit")?;
            Maybe::Just(hhbc::TypedValue::Uninit)
        }
        b"\"\"\"N;\"\"\"" => {
            token_iter.expect_is_str(Token::into_triple_str_literal, "\"\"\"N;\"\"\"")?;
            Maybe::Nothing
        }
        _ => Maybe::Just(assemble_triple_quoted_typed_value(alloc, token_iter)?),
    };
    Ok(tv)
}

fn assemble_class_name_from_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::ClassName<'arena>> {
    Ok(hhbc::ClassName::new(assemble_unescaped_unquoted_str(
        alloc, token_iter,
    )?))
}

fn assemble_class_name<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::ClassName<'arena>> {
    Ok(hhbc::ClassName::new(
        token_iter.expect_identifier_into_ffi_str(alloc)?,
    ))
}

fn assemble_prop_name_from_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::PropName<'arena>> {
    Ok(hhbc::PropName::new(assemble_unescaped_unquoted_str(
        alloc, token_iter,
    )?))
}

fn assemble_prop_name<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::PropName<'arena>> {
    // Only properties that can start with #s start with 0 or
    // 86 and are compiler added ones
    let nm = if token_iter.peek_if_str(Token::is_number, "86")
        || token_iter.peek_if_str(Token::is_number, "0")
    {
        let num_prefix = token_iter.expect(Token::into_number)?;
        let name = token_iter.expect(Token::into_identifier)?;
        let mut num_prefix = num_prefix.to_vec();
        num_prefix.extend_from_slice(name);
        Str::new_slice(alloc, &num_prefix)
    } else {
        token_iter.expect_identifier_into_ffi_str(alloc)?
    };
    Ok(hhbc::PropName::new(nm))
}

fn assemble_method_name_from_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::MethodName<'arena>> {
    Ok(hhbc::MethodName::new(assemble_unescaped_unquoted_str(
        alloc, token_iter,
    )?))
}

fn assemble_method_name<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::MethodName<'arena>> {
    let nm = if token_iter.peek_if_str(Token::is_number, "86") {
        // Only methods that can start with #s start with
        // 86 and are compiler added ones
        let under86 = token_iter.expect(Token::into_number)?;
        let name = token_iter.expect(Token::into_identifier)?;
        let mut under86 = under86.to_vec();
        under86.extend_from_slice(name);
        Str::new_slice(alloc, &under86)
    } else {
        token_iter.expect_identifier_into_ffi_str(alloc)?
    };
    Ok(hhbc::MethodName::new(nm))
}

fn assemble_const_name_from_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::ConstName<'arena>> {
    Ok(hhbc::ConstName::new(assemble_unescaped_unquoted_str(
        alloc, token_iter,
    )?))
}

fn assemble_const_name<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::ConstName<'arena>> {
    Ok(hhbc::ConstName::new(
        token_iter.expect_identifier_into_ffi_str(alloc)?,
    ))
}

fn assemble_function_name_from_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::FunctionName<'arena>> {
    Ok(hhbc::FunctionName::new(assemble_unescaped_unquoted_str(
        alloc, token_iter,
    )?))
}

fn assemble_function_name<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::FunctionName<'arena>> {
    let nm = if token_iter.peek_if_str(Token::is_number, "86") {
        // Only functions that can start with #s start with
        // 86 and are compiler added ones
        let under86 = token_iter.expect(Token::into_number)?;
        let name = token_iter.expect(Token::into_identifier)?;
        let mut under86 = under86.to_vec();
        under86.extend_from_slice(name);
        Str::new_slice(alloc, &under86)
    } else {
        token_iter.expect_identifier_into_ffi_str(alloc)?
    };
    Ok(hhbc::FunctionName::new(nm))
}

/// Ex:
/// extends C
/// There is only one base per HhasClass
fn assemble_base<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Maybe<hhbc::ClassName<'arena>>> {
    if token_iter.next_if_str(Token::is_identifier, "extends") {
        Ok(Maybe::Just(assemble_class_name(alloc, token_iter)?))
    } else {
        Ok(Maybe::Nothing)
    }
}

/// Ex:
/// implements (Fooable Barable)
/// enum_includes (Fooable Barable)
fn assemble_imp_or_enum_includes<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    imp_or_inc: &str,
) -> Result<Slice<'arena, hhbc::ClassName<'arena>>> {
    let mut classes = Vec::new();
    if token_iter.next_if_str(Token::is_identifier, imp_or_inc) {
        token_iter.expect(Token::into_open_paren)?;
        while !token_iter.peek_if(Token::is_close_paren) {
            classes.push(assemble_class_name(alloc, token_iter)?);
        }
        token_iter.expect(Token::into_close_paren)?;
    }
    Ok(Slice::from_vec(alloc, classes))
}

/// Ex: .doc """doc""";
fn assemble_doc_comment<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Maybe<Str<'arena>>> {
    if token_iter.next_if_str(Token::is_decl, ".doc") {
        let com = assemble_unescaped_unquoted_triple_str(alloc, token_iter)?;
        token_iter.expect(Token::into_semicolon)?;
        Ok(Maybe::Just(com))
    } else {
        Ok(Maybe::Nothing)
    }
}

/// Ex: .use TNonScalar;
fn assemble_uses<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Slice<'arena, hhbc::ClassName<'arena>>> {
    let mut classes = Vec::new();
    if token_iter.next_if_str(Token::is_decl, ".use") {
        while !token_iter.peek_if(Token::is_semicolon) {
            classes.push(assemble_class_name(alloc, token_iter)?);
        }
        token_iter.expect(Token::into_semicolon)?;
    }
    Ok(Slice::from_vec(alloc, classes))
}

/// Ex: .enum_ty <type_info>
fn assemble_enum_ty<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Maybe<hhbc::hhas_type::HhasTypeInfo<'arena>>> {
    if token_iter.next_if_str(Token::is_decl, ".enum_ty") {
        let ti = assemble_type_info(alloc, token_iter, TypeInfoKind::Enum)?;
        token_iter.expect(Token::into_semicolon)?;
        Ok(ti)
    } else {
        Ok(Maybe::Nothing)
    }
}

/// Ex: .fatal 2:63,2:63 Parse "A right parenthesis `)` is expected here.";
fn assemble_fatal<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<ffi::Triple<hhbc::FatalOp, hhbc::hhas_pos::HhasPos, Str<'arena>>> {
    token_iter.expect_is_str(Token::into_decl, ".fatal")?;
    let pos = hhbc::hhas_pos::HhasPos {
        line_begin: token_iter.expect_and_get_number()?,
        col_begin: {
            token_iter.expect(Token::into_colon)?;
            token_iter.expect_and_get_number()?
        },
        line_end: {
            token_iter.expect(Token::into_comma)?;
            token_iter.expect_and_get_number()?
        },
        col_end: {
            token_iter.expect(Token::into_colon)?;
            token_iter.expect_and_get_number()?
        },
    };
    let fat_op = token_iter.expect(Token::into_identifier)?;
    let fat_op = match fat_op {
        b"Parse" => hhbc::FatalOp::Parse,
        b"Runtime" => hhbc::FatalOp::Runtime,
        b"RuntimeOmitFrame" => hhbc::FatalOp::RuntimeOmitFrame,
        _ => bail!("Unknown fatal op: {:?}", fat_op),
    };
    let msg = escaper::unescape_literal_bytes_into_vec_bytes(escaper::unquote_slice(
        token_iter.expect(Token::into_str_literal)?,
    ))?;
    token_iter.expect(Token::into_semicolon)?;
    Ok(ffi::Triple::from((
        fat_op,
        pos,
        Str::new_slice(alloc, &msg),
    )))
}

/// A line of adata looks like:
/// .adata id = """<tv>"""
/// with tv being a typed value; see `assemble_typed_value` doc for what <tv> looks like.
fn assemble_adata<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_adata::HhasAdata<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".adata")?;
    let id = token_iter.expect_identifier_into_ffi_str(alloc)?;
    token_iter.expect(Token::into_equal)?;
    // What's left here is tv
    let value = assemble_triple_quoted_typed_value(alloc, token_iter)?;
    token_iter.expect(Token::into_semicolon)?;
    Ok(hhbc::hhas_adata::HhasAdata { id, value })
}

/// For use by initial value
fn assemble_triple_quoted_typed_value<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::TypedValue<'arena>> {
    let (st, line) = token_iter.expect(Token::into_triple_str_literal_and_line)?;
    // Guaranteed st is encased by """ """
    let st = &st[3..st.len() - 3];
    let st = escaper::unescape_literal_bytes_into_vec_bytes(st)?;
    // TvLexer expects an unescaped string.
    assemble_typed_value(alloc, &st, line)
}

/// tv can look like:
/// uninit | N; | s:s.len():"(escaped s)"; | l:s.len():"(escaped s)"; | d:#; | i:#; | b:0; | b:1; | D:dict.len():{((tv1);(tv2);)*}
/// | v:vec.len():{(tv;)*} | k:keyset.len():{(tv;)*}
fn assemble_typed_value<'arena>(
    alloc: &'arena Bump,
    src: &[u8],
    line: usize,
) -> Result<hhbc::TypedValue<'arena>> {
    fn deserialize<'arena, 'a>(
        alloc: &'arena Bump,
        src: &'a [u8],
        line: usize,
    ) -> Result<hhbc::TypedValue<'arena>> {
        /// Returns s after ch if ch is the first character in s, else bails.
        fn expect<'a>(s: &'a [u8], ch: &'_ [u8]) -> Result<&'a [u8]> {
            s.strip_prefix(ch)
                .ok_or_else(|| anyhow!("Expected {:?} in src {:?}", ch, s))
        }

        /// Returns the usize (if exists) at the head of the string and the remainder of the string.
        fn expect_usize(src: &[u8]) -> Result<(usize, &[u8])> {
            let (us, rest) = read_while_matches(src, |c| c.is_ascii_digit());
            Ok((std::str::from_utf8(us)?.parse()?, rest))
        }

        /// Advances and consumes the start of s until f is no longer true on the
        /// first character of s. Returns (characters consumed, rest of src)
        fn read_while_matches(s: &[u8], f: impl Fn(u8) -> bool) -> (&[u8], &[u8]) {
            let stake = s.iter().position(|c| !f(*c)).unwrap_or(s.len());
            s.split_at(stake)
        }

        /// A read_while_matches that also expects c at the end.
        /// Note that s != first_return + second_return.
        /// s = first_return + c + second_return
        fn read_until<'a>(
            s: &'a [u8],
            f: impl Fn(u8) -> bool,
            c: &'_ [u8],
        ) -> Result<(&'a [u8], &'a [u8])> {
            let (fst, snd) = read_while_matches(s, f);
            Ok((fst, expect(snd, c)?))
        }

        /// i:#;
        fn deserialize_int<'arena>(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"i")?;
            let src = expect(src, b":")?;
            let (num, src) =
                read_until(src, |c| c.is_ascii_digit() || c == b'-' || c == b'+', b";")?;
            ensure!(!num.is_empty(), "No # specified for int TV");
            let num = std::str::from_utf8(num)?.parse::<i64>()?;
            Ok((src, hhbc::TypedValue::Int(num)))
        }

        // r"d:[-+]?(NAN|INF|([0-9]+\.?[0-9]*([eE][-+]?[0-9]+\.?[0-9]*)?))"
        /// d:#;
        fn deserialize_float<'arena>(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"d")?;
            let src = expect(src, b":")?;
            let (num, src) = read_until(src, |c| c != b';', b";")?;
            let num = std::str::from_utf8(num)?.parse()?;
            Ok((src, hhbc::TypedValue::Float(hhbc::FloatBits(num))))
        }

        /// b:0;
        fn deserialize_bool<'arena>(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"b")?;
            let src = expect(src, b":")?;
            let val = src[0] == b'1';
            let src = expect(&src[1..], b";")?;
            Ok((src, hhbc::TypedValue::Bool(val)))
        }

        /// N;
        fn deserialize_null<'arena>(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"N")?;
            let src = expect(src, b";")?;
            Ok((src, hhbc::TypedValue::Null))
        }

        /// uninit
        fn deserialize_uninit<'arena>(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue<'arena>)> {
            Ok((
                src.strip_prefix(b"uninit")
                    .ok_or_else(|| anyhow!("Expected uninit in src {:?}", src))?,
                hhbc::TypedValue::Uninit,
            ))
        }

        /// s:s.len():"(escaped s)";
        fn deserialize_string<'arena, 'a>(
            alloc: &'arena Bump,
            src: &'a [u8],
        ) -> Result<(&'a [u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"s")?;
            let src = expect(src, b":")?;
            let (len, src) = expect_usize(src)?;
            let src = expect(src, b":")?;
            let src = expect(src, b"\"")?;

            ensure!(
                src.len() >= len,
                "Length specified greater than source length: {}  vs {}",
                len,
                src.len()
            );

            let s = &src[0..len];
            let src = &src[len..];
            let src = expect(src, b"\"")?;
            let src = expect(src, b";")?;
            Ok((src, hhbc::TypedValue::String(Str::new_slice(alloc, s))))
        }

        #[derive(PartialEq)]
        pub enum VecOrKeyset {
            Vec,
            Keyset,
        }

        impl VecOrKeyset {
            pub fn prefix(&self) -> &[u8] {
                match self {
                    VecOrKeyset::Vec => b"v",
                    VecOrKeyset::Keyset => b"k",
                }
            }

            pub fn build<'arena>(
                &self,
                content: Slice<'arena, hhbc::TypedValue<'arena>>,
            ) -> hhbc::TypedValue<'arena> {
                match self {
                    VecOrKeyset::Vec => hhbc::TypedValue::Vec(content),
                    VecOrKeyset::Keyset => hhbc::TypedValue::Keyset(content),
                }
            }
        }
        /// v:vec.len():{(tv;)*} or k:keyset.len():{(tv;)*}
        fn deserialize_vec_or_keyset<'arena, 'a>(
            alloc: &'arena Bump,
            src: &'a [u8],
            v_or_k: VecOrKeyset,
        ) -> Result<(&'a [u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, v_or_k.prefix())?;
            let src = expect(src, b":")?;
            let (len, src) = expect_usize(src)?;
            let src = expect(src, b":")?;
            let mut src = expect(src, b"{")?;
            let mut tv;
            let mut tv_vec = Vec::new();
            for _ in 0..len {
                (src, tv) = deserialize_tv(alloc, src)?;
                tv_vec.push(tv);
            }
            let src = expect(src, b"}")?;
            let slice = Slice::from_vec(alloc, tv_vec);
            Ok((src, v_or_k.build(slice)))
        }

        /// D:(D.len):{p1_0; p1_1; ...; pD.len_0; pD.len_1}
        fn deserialize_dict<'arena, 'a>(
            alloc: &'arena Bump,
            src: &'a [u8],
        ) -> Result<(&'a [u8], hhbc::TypedValue<'arena>)> {
            let src = expect(src, b"D")?;
            let src = expect(src, b":")?;
            let (len, src) = expect_usize(src)?;
            let src = expect(src, b":")?;
            let mut src = expect(src, b"{")?;
            let mut tv0;
            let mut tv1;
            let mut tv_vec = Vec::new();
            for _ in 0..len {
                (src, tv0) = deserialize_tv(alloc, src)?;
                (src, tv1) = deserialize_tv(alloc, src)?;
                tv_vec.push(Pair::from((tv0, tv1)));
            }
            let src = expect(src, b"}")?;
            Ok((src, hhbc::TypedValue::Dict(Slice::from_vec(alloc, tv_vec))))
        }

        fn deserialize_tv<'arena, 'a>(
            alloc: &'arena Bump,
            src: &'a [u8],
        ) -> Result<(&'a [u8], hhbc::TypedValue<'arena>)> {
            let (src, tr) = match src[0] {
                b'i' => deserialize_int(src).context("Assembling a TV int")?,
                b'b' => deserialize_bool(src).context("Assembling a TV bool")?,
                b'd' => deserialize_float(src).context("Assembling a TV float")?,
                b'N' => deserialize_null(src).context("Assembling a TV Null")?,
                b'u' => deserialize_uninit(src).context("Assembling a uninit")?,
                b's' => deserialize_string(alloc, src).context("Assembling a TV string")?,
                b'v' => deserialize_vec_or_keyset(alloc, src, VecOrKeyset::Vec)
                    .context("Assembling a TV vec")?,
                b'k' => deserialize_vec_or_keyset(alloc, src, VecOrKeyset::Keyset)
                    .context("Assembling a TV keyset")?,
                b'D' => deserialize_dict(alloc, src).context("Assembling a TV dict")?,
                b'l' => todo!(), // Never encounter LazyClass in all WWW...
                _ => bail!("Unknown tv: {}", src[0]),
            };
            Ok((src, tr))
        }

        ensure!(!src.is_empty(), "Empty typed value. Line {}", line);

        let (src, tv) = deserialize_tv(alloc, src).context(format!("Line {}", line))?;

        ensure!(
            src.is_empty(),
            "Unassemble-able TV. Unassembled: {:?}. Line {}",
            src,
            line
        );

        Ok(tv)
    }
    deserialize(alloc, src, line)
}

/// Class refs look like:
/// .class_refs {
///    (identifier)+
/// }
/// Function ref looks like:
/// .functionrefs {
///    (identifier)+
/// }
///
fn assemble_refs<'arena, 'a, T: 'arena, F>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    name_str: &str,
    assemble_name: F,
) -> Result<Slice<'arena, T>>
where
    F: Fn(&'arena Bump, &mut Lexer<'_>) -> Result<T>,
{
    token_iter.expect_is_str(Token::into_decl, name_str)?;
    token_iter.expect(Token::into_open_curly)?;
    let mut names = Vec::new();
    while !token_iter.peek_if(Token::is_close_curly) {
        names.push(assemble_name(alloc, token_iter)?);
    }
    token_iter.expect(Token::into_close_curly)?;
    Ok(Slice::from_vec(alloc, names))
}

/// A function def is composed of the following:
/// .function {upper bounds} [special_and_user_attrs] (span) <type_info> name (params) flags? {body}
fn assemble_function<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_function::HhasFunction<'arena>> {
    token_iter.expect_is_str(Token::into_decl, ".function")?;
    let upper_bounds = assemble_upper_bounds(alloc, token_iter)?;
    // Special and user attrs may or may not be specified. If not specified, no [] printed
    let (attr, attributes) = assemble_special_and_user_attrs(alloc, token_iter)?;
    let span = assemble_span(token_iter)?;
    // Body may not have return type info, so check if next token is a < or not
    // Specifically if body doesn't have a return type info bytecode printer doesn't print anything
    // (doesn't print <>)
    let return_type_info = match token_iter.peek() {
        Some(Token::Lt(_)) => {
            assemble_type_info(alloc, token_iter, TypeInfoKind::NotEnumOrTypeDef)?
        }
        _ => Maybe::Nothing,
    };
    // Assemble_name

    let name = assemble_function_name(alloc, token_iter)?;
    let mut decl_map: HashMap<Vec<u8>, u32> = HashMap::new(); // Will store decls in this order: params, decl_vars, unnamed
    let params = assemble_params(alloc, token_iter, &mut decl_map)?;
    let flags = assemble_function_flags(name, token_iter)?;
    let (partial_body, coeffects) = assemble_body(alloc, token_iter, &mut decl_map)?;
    // Fill partial_body in with params, return_type_info, and bd_upper_bounds
    let body = hhbc::hhas_body::HhasBody {
        params,
        return_type_info,
        upper_bounds,
        ..partial_body
    };
    let hhas_func = hhbc::hhas_function::HhasFunction {
        attributes,
        name,
        body,
        span,
        coeffects,
        flags,
        attrs: attr,
    };
    Ok(hhas_func)
}

/// Have to parse flags which may or may not appear: isGenerator isAsync isPairGenerator
/// Also, MEMOIZE_IMPL appears in the function name. Example:
/// .function {} [ "__Memoize"("""v:1:{s:9:\"KeyedByIC\";}""") "__Reified"("""v:4:{i:1;i:0;i:0;i:0;}""")] (4,10) <"" N > memo$memoize_impl($a, $b)
fn assemble_function_flags(
    name: hhbc::FunctionName<'_>,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_function::HhasFunctionFlags> {
    let mut flag = hhbc::hhas_function::HhasFunctionFlags::empty();
    while token_iter.peek_if(Token::is_identifier) {
        match token_iter.expect(Token::into_identifier)? {
            b"isPairGenerator" => flag |= hhbc::hhas_function::HhasFunctionFlags::PAIR_GENERATOR,
            b"isAsync" => flag |= hhbc::hhas_function::HhasFunctionFlags::ASYNC,
            b"isGenerator" => flag |= hhbc::hhas_function::HhasFunctionFlags::GENERATOR,
            f => bail!("Unknown function flag: {:?}", f),
        }
    }
    if name.as_bstr().ends_with(b"$memoize_impl") {
        flag |= hhbc::hhas_function::HhasFunctionFlags::MEMOIZE_IMPL;
    }
    Ok(flag)
}

/// Parses over filepath. Note that HCU doesn't hold the filepath; filepath is in the context passed to the
/// bytecode printer. So we don't store the filepath in our HCU or anywhere, but still parse over it
/// Filepath: .filepath strliteral semicolon (.filepath already consumed in `assemble_from_toks)
fn assemble_filepath(token_iter: &mut Lexer<'_>) -> Result<PathBuf> {
    // Filepath is .filepath strliteral and semicolon
    token_iter.expect_is_str(Token::into_decl, ".filepath")?;
    let fp = token_iter.expect(Token::into_str_literal)?;
    let fp = escaper::unquote_slice(fp);
    let fp = PathBuf::from(OsString::from_vec(fp.to_vec()));
    token_iter.expect(Token::into_semicolon)?;
    Ok(fp)
}

/// Span ex: (2, 4)
fn assemble_span(token_iter: &mut Lexer<'_>) -> Result<hhbc::hhas_pos::HhasSpan> {
    token_iter.expect(Token::into_open_paren)?;
    let line_begin: usize = token_iter.expect_and_get_number()?;
    token_iter.expect(Token::into_comma)?;
    let line_end: usize = token_iter.expect_and_get_number()?;
    token_iter.expect(Token::into_close_paren)?;
    Ok(hhbc::hhas_pos::HhasSpan {
        line_begin,
        line_end,
    })
}

/// Ex: {(T as <"HH\\int" "HH\\int" upper_bound>)}
/// {(id as type_info, type_info ... ), (id as type_info, type_info ...)*}
fn assemble_upper_bounds<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Slice<'arena, Pair<Str<'arena>, Slice<'arena, hhbc::hhas_type::HhasTypeInfo<'arena>>>>>
{
    token_iter.expect(Token::into_open_curly)?;
    let mut ubs = Vec::new();
    while !token_iter.peek_if(Token::is_close_curly) {
        ubs.push(assemble_upper_bound(alloc, token_iter)?);
        if !token_iter.peek_if(Token::is_close_curly) {
            token_iter.expect(Token::into_comma)?;
        }
    }
    token_iter.expect(Token::into_close_curly)?;
    Ok(Slice::from_vec(alloc, ubs))
}

/// Ex: (T as <"HH\\int" "HH\\int" upper_bound>)
fn assemble_upper_bound<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Pair<Str<'arena>, Slice<'arena, hhbc::hhas_type::HhasTypeInfo<'arena>>>> {
    token_iter.expect(Token::into_open_paren)?;
    let id = token_iter.expect_identifier_into_ffi_str(alloc)?;
    token_iter.expect_is_str(Token::into_identifier, "as")?;
    let mut tis = Vec::new();
    while !token_iter.peek_if(Token::is_close_paren) {
        if let Maybe::Just(ti) =
            assemble_type_info(alloc, token_iter, TypeInfoKind::NotEnumOrTypeDef)?
        {
            tis.push(ti);
        } else {
            bail!("Unexpected \"N\" in upper bound type info");
        }
        if !token_iter.peek_if(Token::is_close_paren) {
            token_iter.expect(Token::into_comma)?;
        }
    }
    token_iter.expect(Token::into_close_paren)?;
    Ok(Pair(id, Slice::from_vec(alloc, tis)))
}

/// Ex: [ "__EntryPoint"("""v:0:{}""")]. This example lacks Attrs
/// Ex: [abstract final] This example lacks HhasAttributes
fn assemble_special_and_user_attrs<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<(
    hhvm_types_ffi::ffi::Attr,
    Slice<'arena, hhbc::hhas_attribute::HhasAttribute<'arena>>,
)> {
    let mut user_atts = Vec::new();
    let mut tr = hhvm_types_ffi::ffi::Attr::AttrNone;
    if token_iter.next_if(Token::is_open_bracket) {
        while !token_iter.peek_if(Token::is_close_bracket) {
            if token_iter.peek_if(Token::is_str_literal) {
                user_atts.push(assemble_user_attr(alloc, token_iter)?)
            } else if token_iter.peek_if(Token::is_identifier) {
                tr.add(assemble_hhvm_attr(token_iter)?);
            } else {
                bail!("Unknown token in special and user attrs");
            }
        }
        token_iter.expect(Token::into_close_bracket)?;
    }
    // If no special and user attrs then no [] printed
    let user_atts = Slice::from_vec(alloc, user_atts);
    Ok((tr, user_atts))
}

fn assemble_hhvm_attr(token_iter: &mut Lexer<'_>) -> Result<hhvm_types_ffi::ffi::Attr> {
    use hhvm_types_ffi::ffi::Attr;
    let flag = match token_iter.expect(Token::into_identifier)? {
        b"abstract" => Attr::AttrAbstract,
        b"bad_redeclare" => Attr::AttrNoBadRedeclare,
        b"builtin" => Attr::AttrBuiltin,
        b"deep_init" => Attr::AttrDeepInit,
        b"dyn_callable" => Attr::AttrDynamicallyCallable,
        b"dyn_constructible" => Attr::AttrDynamicallyConstructible,
        b"enum" => Attr::AttrEnum,
        b"enum_class" => Attr::AttrEnumClass,
        b"final" => Attr::AttrFinal,
        b"has_closure_coeffects_prop" => Attr::AttrHasClosureCoeffectsProp,
        b"has_coeffect_rules" => Attr::AttrHasCoeffectRules,
        b"initial_satisifes_tc" => Attr::AttrInitialSatisfiesTC,
        b"interceptable" => Attr::AttrInterceptable,
        b"interface" => Attr::AttrInterface,
        b"internal" => Attr::AttrInternal,
        b"is_closure_class" => Attr::AttrIsClosureClass,
        b"is_const" => Attr::AttrIsConst,
        b"is_foldable" => Attr::AttrIsFoldable,
        b"is_meth_caller" => Attr::AttrIsMethCaller,
        b"late_init" => Attr::AttrLateInit,
        b"lsb" => Attr::AttrLSB,
        b"none" => Attr::AttrNone,
        b"noreifiedinit" => Attr::AttrNoReifiedInit,
        b"no_dynamic_props" => Attr::AttrForbidDynamicProps,
        b"no_expand_trait" => Attr::AttrNoExpandTrait,
        b"no_fcall_builtin" => Attr::AttrNoFCallBuiltin,
        b"no_implicit_nullable" => Attr::AttrNoImplicitNullable,
        b"no_injection" => Attr::AttrNoInjection,
        b"no_override" => Attr::AttrNoOverride,
        b"persistent" => Attr::AttrPersistent,
        b"private" => Attr::AttrPrivate,
        b"protected" => Attr::AttrProtected,
        b"prov_skip_frame" => Attr::AttrProvenanceSkipFrame,
        b"public" => Attr::AttrPublic,
        b"readonly" => Attr::AttrIsReadonly,
        b"readonly_return" => Attr::AttrReadonlyReturn,
        b"readonly_this" => Attr::AttrReadonlyThis,
        b"sealed" => Attr::AttrSealed,
        b"static" => Attr::AttrStatic,
        b"support_async_eager_return" => Attr::AttrSupportsAsyncEagerReturn,
        b"sys_initial_val" => Attr::AttrSystemInitialValue,
        b"trait" => Attr::AttrTrait,
        b"unique" => Attr::AttrUnique,
        b"unused_max_attr" => Attr::AttrUnusedMaxAttr,
        b"variadic_param" => Attr::AttrVariadicParam,
        o => bail!("Unknown attr: {:?}", o),
    };
    Ok(flag)
}

/// HhasAttributes are printed as follows:
/// "name"("""v:args.len:{args}""") where args are typed values.
fn assemble_user_attr<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_attribute::HhasAttribute<'arena>> {
    let nm = escaper::unescape_literal_bytes_into_vec_bytes(escaper::unquote_slice(
        token_iter.expect(Token::into_str_literal)?,
    ))?;
    let name = Str::new_slice(alloc, &nm);
    token_iter.expect(Token::into_open_paren)?;
    let arguments = assemble_user_attr_args(alloc, token_iter)?;
    token_iter.expect(Token::into_close_paren)?;
    Ok(hhbc::hhas_attribute::HhasAttribute { name, arguments })
}

/// Printed as follows (print_attributes in bcp)
/// "v:args.len:{args}" where args are typed values
fn assemble_user_attr_args<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Slice<'arena, hhbc::TypedValue<'arena>>> {
    if let hhbc::TypedValue::Vec(sl) = assemble_triple_quoted_typed_value(alloc, token_iter)? {
        Ok(sl)
    } else {
        bail!("Malformed user_attr_args -- should be a vec")
    }
}

#[derive(PartialEq)]
pub enum TypeInfoKind {
    TypeDef,
    Enum,
    NotEnumOrTypeDef,
}

/// Ex: <"HH\\void" N >
/// < "user_type" "type_constraint.name" type_constraint.flags >
/// if 'is_enum', no  type_constraint name printed
/// if 'is_typedef', no user_type name printed

fn assemble_type_info<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    tik: TypeInfoKind,
) -> Result<Maybe<hhbc::hhas_type::HhasTypeInfo<'arena>>> {
    token_iter.expect(Token::into_lt)?;
    let first = escaper::unquote_slice(token_iter.expect(Token::into_str_literal)?);
    let first = escaper::unescape_literal_bytes_into_vec_bytes(first)?;
    let type_cons_name = if tik == TypeInfoKind::TypeDef {
        if first.is_empty() {
            Maybe::Nothing
        } else {
            Maybe::Just(Str::new_slice(alloc, &first))
        }
    } else if tik == TypeInfoKind::Enum || token_iter.next_if_str(Token::is_identifier, "N") {
        Maybe::Nothing
    } else {
        Maybe::Just(Str::new_slice(
            alloc,
            &escaper::unescape_literal_bytes_into_vec_bytes(escaper::unquote_slice(
                token_iter.expect(Token::into_str_literal)?,
            ))?,
        ))
    };
    let user_type = if !(tik == TypeInfoKind::TypeDef) {
        Maybe::Just(Str::new_slice(alloc, &first))
    } else {
        Maybe::Nothing
    };
    let mut tcflags = hhvm_types_ffi::ffi::TypeConstraintFlags::NoFlags;
    while !token_iter.peek_if(Token::is_gt) {
        tcflags = tcflags | assemble_type_constraint(token_iter)?;
    }
    token_iter.expect(Token::into_gt)?;
    let cons = hhbc::hhas_type::Constraint::make(type_cons_name, tcflags);
    Ok(Maybe::Just(hhbc::hhas_type::HhasTypeInfo::make(
        user_type, cons,
    )))
}

fn assemble_type_constraint(
    token_iter: &mut Lexer<'_>,
) -> Result<hhvm_types_ffi::ffi::TypeConstraintFlags> {
    use hhvm_types_ffi::ffi::TypeConstraintFlags;
    match token_iter.expect(Token::into_identifier)? {
        b"upper_bound" => Ok(TypeConstraintFlags::UpperBound),
        b"display_nullable" => Ok(TypeConstraintFlags::DisplayNullable),
        //no mock objects
        //resolved
        b"type_constant" => Ok(TypeConstraintFlags::TypeConstant),
        b"soft" => Ok(TypeConstraintFlags::Soft),
        b"type_var" => Ok(TypeConstraintFlags::TypeVar),
        b"extended_hint" => Ok(TypeConstraintFlags::ExtendedHint),
        b"nullable" => Ok(TypeConstraintFlags::Nullable),

        _ => todo!(),
    }
}

/// ((a, )*a) | () where a is a param
fn assemble_params<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
) -> Result<Slice<'arena, hhbc::hhas_param::HhasParam<'arena>>> {
    token_iter.expect(Token::into_open_paren)?;
    let mut params = Vec::new();
    while !token_iter.peek_if(Token::is_close_paren) {
        params.push(assemble_param(alloc, token_iter, decl_map)?);
        if !token_iter.peek_if(Token::is_close_paren) {
            token_iter.expect(Token::into_comma)?;
        }
    }
    token_iter.expect(Token::into_close_paren)?;
    Ok(Slice::from_vec(alloc, params))
}

/// a: [user_attributes]? inout? readonly? ...?<type_info>?name (= default_value)?
fn assemble_param<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
) -> Result<hhbc::hhas_param::HhasParam<'arena>> {
    let mut ua_vec = Vec::new();
    let user_attributes = {
        if token_iter.peek_if(Token::is_open_bracket) {
            token_iter.expect(Token::into_open_bracket)?;
            while !token_iter.peek_if(Token::is_close_bracket) {
                ua_vec.push(assemble_user_attr(alloc, token_iter)?);
            }
            token_iter.expect(Token::into_close_bracket)?;
        }
        Slice::from_vec(alloc, ua_vec)
    };
    let is_inout = token_iter.next_if_str(Token::is_identifier, "inout");
    let is_readonly = token_iter.next_if_str(Token::is_identifier, "readonly");
    let is_variadic = token_iter.next_if(Token::is_variadic);
    let type_info = if token_iter.peek_if(Token::is_lt) {
        assemble_type_info(alloc, token_iter, TypeInfoKind::NotEnumOrTypeDef)?
    } else {
        Maybe::Nothing
    };
    let name = token_iter.expect(Token::into_variable)?;
    decl_map.insert(name.to_vec(), decl_map.len() as u32);
    let name = Str::new_slice(alloc, name);
    let default_value = assemble_default_value(alloc, token_iter)?;
    Ok(hhbc::hhas_param::HhasParam {
        name,
        is_variadic,
        is_inout,
        is_readonly,
        user_attributes,
        type_info,
        default_value,
    })
}

/// Ex: $skip_top_libcore = DV13("""true""")
/// Parsing after the variable name
fn assemble_default_value<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Maybe<Pair<hhbc::Label, Str<'arena>>>> {
    let tr = if token_iter.next_if(Token::is_equal) {
        let lbl = assemble_label(token_iter, false)?; // false: not expecting colon
        token_iter.expect(Token::into_open_paren)?;
        let st = assemble_unescaped_unquoted_triple_str(alloc, token_iter)?;
        token_iter.expect(Token::into_close_paren)?;
        Maybe::Just(Pair(lbl, st))
    } else {
        Maybe::Nothing
    };
    Ok(tr)
}

/// { (.doc ...)? (.ismemoizewrapper)? (.ismemoizewrapperlsb)? (.numiters)? (.declvars)? (instructions)* }
fn assemble_body<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
) -> Result<(
    hhbc::hhas_body::HhasBody<'arena>,
    hhbc::hhas_coeffects::HhasCoeffects<'arena>,
)> {
    let mut doc_comment = Maybe::Nothing;
    let mut instrs = Vec::new();
    let mut decl_vars = Slice::default();
    let mut num_iters = 0;
    let mut coeff = hhbc::hhas_coeffects::HhasCoeffects::default();
    let mut is_memoize_wrapper = false;
    let mut is_memoize_wrapper_lsb = false;
    // For now we don't parse params, so will just have decl_vars (might need to move this later)
    token_iter.expect(Token::into_open_curly)?;
    // In body, before instructions, are 5 possible constructs:
    // only .declvars can be declared more than once
    while token_iter.peek_if(Token::is_decl) {
        if token_iter.peek_if_str(Token::is_decl, ".doc") {
            doc_comment = assemble_doc_comment(alloc, token_iter)?;
        } else if token_iter.peek_if_str(Token::is_decl, ".ismemoizewrapperlsb") {
            token_iter.expect_is_str(Token::into_decl, ".ismemoizewrapperlsb")?;
            token_iter.expect(Token::into_semicolon)?;
            is_memoize_wrapper_lsb = true;
        } else if token_iter.peek_if_str(Token::is_decl, ".ismemoizewrapper") {
            token_iter.expect_is_str(Token::into_decl, ".ismemoizewrapper")?;
            token_iter.expect(Token::into_semicolon)?;
            is_memoize_wrapper = true;
        } else if token_iter.peek_if_str(Token::is_decl, ".numiters") {
            ensure!(
                num_iters == 0,
                "Cannot have more than one .numiters per function body"
            ); // Because only printed once in print.rs

            num_iters = assemble_numiters(token_iter)?;
        } else if token_iter.peek_if_str(Token::is_decl, ".declvars") {
            ensure!(
                decl_vars.is_empty(),
                "Cannot have more than one .declvars per function body"
            );

            decl_vars = assemble_decl_vars(alloc, token_iter, decl_map)?;
        } else if token_iter.peek_if_str_starts(Token::is_decl, ".coeffects") {
            coeff = assemble_coeffects(alloc, token_iter)?;
        } else {
            break;
        }
    }
    // tcb_count tells how many TryCatchBegins there are that are still unclosed
    // we only stop parsing instructions once we see a is_close_curly and tcb_count is 0
    let mut tcb_count = 0;
    while tcb_count > 0 || !token_iter.peek_if(Token::is_close_curly) {
        instrs.push(assemble_instr(alloc, token_iter, decl_map, &mut tcb_count)?);
    }
    token_iter.expect(Token::into_close_curly)?;
    let tr = hhbc::hhas_body::HhasBody {
        body_instrs: Slice::from_vec(alloc, instrs),
        decl_vars,
        num_iters,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        doc_comment,
        ..Default::default()
    };
    Ok((tr, coeff))
}

/// Printed in this order (if exists)
/// static and unenforced static coeffects (.coeffects_static ... );
/// .coeffects_fun_param ..;
/// .coeffects_cc_param ..;
/// .coeffects_cc_this;
/// .coeffects_cc_reified;
/// .coeffects_closure_parent_scope;
/// .coeffects_generator_this;
/// .coeffects_caller;
fn assemble_coeffects<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::hhas_coeffects::HhasCoeffects<'arena>> {
    let mut scs = Vec::new();
    let mut uscs = Vec::new();
    let mut fun_param = Vec::new();
    let mut cc_param = Vec::new();
    let mut cc_this = Vec::new();
    let mut cc_reified = Vec::new();
    let mut closure_parent_scope = false;
    let mut generator_this = false;
    let mut caller = false;
    while token_iter.peek_if_str_starts(Token::is_decl, ".coeffects") {
        if token_iter.peek_if_str(Token::is_decl, ".coeffects_static") {
            assemble_static_coeffects(alloc, token_iter, &mut scs, &mut uscs)?;
        }
        if token_iter.peek_if_str(Token::is_decl, ".coeffects_fun_param") {
            assemble_coeffects_fun_param(token_iter, &mut fun_param)?;
        }
        if token_iter.peek_if_str(Token::is_decl, ".coeffects_cc_param") {
            assemble_coeffects_cc_param(alloc, token_iter, &mut cc_param)?;
        }
        if token_iter.peek_if_str(Token::is_decl, ".coeffects_cc_this") {
            assemble_coeffects_cc_this(alloc, token_iter, &mut cc_this)?;
        }
        if token_iter.peek_if_str(Token::is_decl, ".coeffects_cc_reified") {
            assemble_coeffects_cc_reified(alloc, token_iter, &mut cc_reified)?;
        }
        closure_parent_scope =
            token_iter.next_if_str(Token::is_decl, ".coeffects_closure_parent_scope");
        if closure_parent_scope {
            token_iter.expect(Token::into_semicolon)?;
        }
        generator_this = token_iter.next_if_str(Token::is_decl, ".coeffects_generator_this");
        if generator_this {
            token_iter.expect(Token::into_semicolon)?;
        }
        caller = token_iter.next_if_str(Token::is_decl, ".coeffects_caller");
        if caller {
            token_iter.expect(Token::into_semicolon)?;
        }
    }

    Ok(hhbc::hhas_coeffects::HhasCoeffects::new(
        Slice::from_vec(alloc, scs),
        Slice::from_vec(alloc, uscs),
        Slice::from_vec(alloc, fun_param),
        Slice::from_vec(alloc, cc_param),
        Slice::from_vec(alloc, cc_this),
        Slice::from_vec(alloc, cc_reified),
        closure_parent_scope,
        generator_this,
        caller,
    ))
}

/// Ex: .coeffects_static pure
fn assemble_static_coeffects<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    scs: &mut Vec<Ctx>,
    uscs: &mut Vec<Str<'arena>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".coeffects_static")?;
    while !token_iter.peek_if(Token::is_semicolon) {
        match token_iter.expect(Token::into_identifier)? {
            b"pure" => scs.push(Ctx::Pure),
            b"defaults" => scs.push(Ctx::Defaults),
            b"rx" => scs.push(Ctx::Rx),
            b"zoned" => scs.push(Ctx::Zoned),
            b"write_props" => scs.push(Ctx::WriteProps),
            b"rx_local" => scs.push(Ctx::RxLocal),
            b"zoned_with" => scs.push(Ctx::ZonedWith),
            b"zoned_local" => scs.push(Ctx::ZonedLocal),
            b"zoned_shallow" => scs.push(Ctx::ZonedShallow),
            b"leak_safe_local" => scs.push(Ctx::LeakSafeLocal),
            b"leak_safe_shallow" => scs.push(Ctx::LeakSafeShallow),
            b"leak_safe" => scs.push(Ctx::LeakSafe),
            b"read_globals" => scs.push(Ctx::ReadGlobals),
            b"globals" => scs.push(Ctx::Globals),
            b"write_this_props" => scs.push(Ctx::WriteThisProps),
            b"rx_shallow" => scs.push(Ctx::RxShallow),
            d => uscs.push(Str::new_slice(alloc, d)), //If unknown Ctx, is a unenforce_static_coeffect (ex: output)
        }
    }
    token_iter.expect(Token::into_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_fun_param 0;
fn assemble_coeffects_fun_param(
    token_iter: &mut Lexer<'_>,
    fun_param: &mut Vec<usize>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".coeffects_fun_param")?;
    while !token_iter.peek_if(Token::is_semicolon) {
        fun_param.push(token_iter.expect_and_get_number()?);
    }
    token_iter.expect(Token::into_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_cc_param 0 C 0 Cdebug;
fn assemble_coeffects_cc_param<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    cc_param: &mut Vec<Pair<usize, Str<'arena>>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".coeffects_cc_param")?;
    while !token_iter.peek_if(Token::is_semicolon) {
        let num: usize = token_iter.expect_and_get_number()?;
        let st = token_iter.expect_identifier_into_ffi_str(alloc)?;
        cc_param.push(Pair(num, st));
    }
    token_iter.expect(Token::into_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_cc_this Cdebug;
fn assemble_coeffects_cc_this<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    cc_this: &mut Vec<Slice<'arena, Str<'arena>>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".coeffects_cc_this")?;
    let mut params = Vec::new();
    while !token_iter.peek_if(Token::is_semicolon) {
        params.push(token_iter.expect_identifier_into_ffi_str(alloc)?);
    }
    token_iter.expect(Token::into_semicolon)?;
    cc_this.push(Slice::from_vec(alloc, params));
    Ok(())
}

/// .coeffects_cc_reified 0 C;
fn assemble_coeffects_cc_reified<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    cc_reified: &mut Vec<ffi::Triple<bool, usize, Slice<'arena, Str<'arena>>>>,
) -> Result<()> {
    token_iter.expect_is_str(Token::into_decl, ".coeffects_cc_reified")?;
    let isc = token_iter.next_if_str(Token::is_identifier, "isClass");
    let us = token_iter.expect_and_get_number()?;
    let mut sl = Vec::new();
    while !token_iter.peek_if(Token::is_semicolon) {
        sl.push(token_iter.expect_identifier_into_ffi_str(alloc)?);
    }
    token_iter.expect(Token::into_semicolon)?;
    cc_reified.push(ffi::Triple(isc, us, Slice::from_vec(alloc, sl)));
    Ok(())
}

/// Expects .numiters #+;
fn assemble_numiters(token_iter: &mut Lexer<'_>) -> Result<usize> {
    token_iter.expect_is_str(Token::into_decl, ".numiters")?;
    let num = token_iter.expect_and_get_number()?;
    token_iter.expect(Token::into_semicolon)?;
    Ok(num)
}

/// Expects .declvars ($x)+;
fn assemble_decl_vars<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
) -> Result<Slice<'arena, Str<'arena>>> {
    token_iter.expect_is_str(Token::into_decl, ".declvars")?;
    let mut var_names = Vec::new();
    while !token_iter.peek_if(Token::is_semicolon) {
        let var_nm = token_iter.expect_var()?;
        var_names.push(Str::new_slice(alloc, &var_nm[..]));
        decl_map.insert(var_nm, decl_map.len().try_into().unwrap());
    }
    token_iter.expect(Token::into_semicolon)?;
    Ok(Slice::from_vec(alloc, var_names))
}

/// Opcodes and Pseudos
fn assemble_instr<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
    tcb_count: &mut usize, // Increase this when get TryCatchBegin, decrease when TryCatchEnd
) -> Result<hhbc::Instruct<'arena>> {
    let label_reg = regex!(r"^((DV|L)[0-9]+)$").clone();
    if let Some(mut sl_lexer) = token_iter.fetch_until_newline() {
        if sl_lexer.peek_if(Token::is_decl) {
            // Not all pseudos are decls, but all instruction decls are pseudos
            if sl_lexer.peek_if_str(Token::is_decl, ".srcloc") {
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::SrcLoc(
                    assemble_srcloc(&mut sl_lexer)?,
                )))
            } else if sl_lexer.next_if_str(Token::is_decl, ".try") {
                sl_lexer.expect(Token::into_open_curly)?;
                *tcb_count += 1;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchBegin))
            } else {
                todo!("{}", sl_lexer.next().unwrap());
            }
        } else if sl_lexer.next_if(Token::is_close_curly) {
            if sl_lexer.next_if_str(Token::is_decl, ".catch") {
                // Is a TCM
                sl_lexer.expect(Token::into_open_curly)?;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchMiddle))
            } else {
                // Is a TCE
                debug_assert!(*tcb_count > 0);
                *tcb_count -= 1;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchEnd))
            }
        } else if sl_lexer.peek_if(Token::is_identifier) {
            if let Some(tok) = sl_lexer.peek() {
                match tok.as_bytes() {
                    label if label_reg.is_match(label) => Ok(hhbc::Instruct::Pseudo(
                        hhbc::Pseudo::Label(assemble_label(&mut sl_lexer, true)?),
                    )),
                    b"Add" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Add, "Add")
                    }
                    b"Sub" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Sub, "Sub")
                    }
                    b"Mul" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Mul, "Mul")
                    }
                    b"AddO" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::AddO, "AddO")
                    }
                    b"SubO" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::SubO, "SubO")
                    }
                    b"MulO" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::MulO, "MulO")
                    }
                    b"Pow" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Pow, "Pow")
                    }
                    b"Not" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Not, "Not")
                    }
                    b"NSame" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::NSame, "NSame")
                    }
                    b"Eq" => assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Eq, "Eq"),
                    b"Neq" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Neq, "Neq")
                    }
                    b"Lte" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Lte, "Lte")
                    }
                    b"Gt" => assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Gt, "Gt"),
                    b"Gte" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Gte, "Gte")
                    }
                    b"Cmp" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Cmp, "Cmp")
                    }
                    b"BitAnd" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::BitAnd,
                        "BitAnd",
                    ),
                    b"BitOr" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::BitOr, "BitOr")
                    }
                    b"BitXor" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::BitXor,
                        "BitXor",
                    ),
                    b"BitNot" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::BitNot,
                        "BitNot",
                    ),
                    b"Shl" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Shl, "Shl")
                    }
                    b"Shr" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Shr, "Shr")
                    }
                    b"CastBool" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastBool,
                        "CastBool",
                    ),
                    b"CastInt" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastInt,
                        "CastInt",
                    ),
                    b"CastDouble" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastDouble,
                        "CastDouble",
                    ),
                    b"CastString" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastString,
                        "CastString",
                    ),
                    b"CastDict" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastDict,
                        "CastDict",
                    ),
                    b"CastKeyset" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastKeyset,
                        "CastKeyset",
                    ),
                    b"CastVec" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CastVec,
                        "CastVec",
                    ),
                    b"DblAsBits" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::DblAsBits,
                        "DblAsBits",
                    ),
                    b"InstanceOf" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::InstanceOf,
                        "InstanceOf",
                    ),

                    b"Print" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Print, "Print")
                    }
                    b"Div" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Div, "Div")
                    }
                    b"Dir" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Dir, "Dir")
                    }
                    b"PopC" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::PopC, "PopC")
                    }
                    b"Concat" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::Concat,
                        "Concat",
                    ),
                    b"Null" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Null, "Null")
                    }
                    b"RetC" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::RetC, "RetC")
                    }
                    b"Lt" => assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Lt, "Lt"),
                    b"Mod" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Mod, "Mod")
                    }
                    b"Exit" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Exit, "Exit")
                    }
                    b"Same" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Same, "Same")
                    }
                    b"NullUninit" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::NullUninit,
                        "NullUninit",
                    ),
                    b"String" => assemble_string_opcode(alloc, &mut sl_lexer),
                    b"Int" => assemble_int_opcode(&mut sl_lexer),
                    b"Double" => assemble_double_opcode(&mut sl_lexer),
                    b"False" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::False, "False")
                    }
                    b"Dict" => assemble_adata_id_carrying_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::Dict,
                        "Dict",
                    ),
                    b"Vec" => assemble_adata_id_carrying_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::Vec,
                        "Vec",
                    ),
                    b"Keyset" => assemble_adata_id_carrying_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::Keyset,
                        "Keyset",
                    ),
                    b"True" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::True, "True")
                    }
                    b"VerifyOutType" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::VerifyOutType,
                        "VerifyOutType",
                    ),
                    b"VerifyParamType" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::VerifyParamType,
                        "VerifyParamType",
                    ),
                    b"VerifyParamTypeTS" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::VerifyParamTypeTS,
                        "VerifyParamTypeTS",
                    ),

                    b"PopL" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::PopL,
                        "PopL",
                    ),
                    b"PushL" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::PushL,
                        "PushL",
                    ),
                    b"GetMemoKeyL" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::GetMemoKeyL,
                        "GetMemoKeyL",
                    ),
                    b"IssetL" => assemble_local_carrying_opcode_instr(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::IssetL,
                        "IssetL",
                    ),
                    b"JmpZ" => {
                        assemble_jump_opcode_instr(&mut sl_lexer, hhbc::Opcode::JmpZ, "JmpZ")
                    }
                    b"Jmp" => assemble_jump_opcode_instr(&mut sl_lexer, hhbc::Opcode::Jmp, "Jmp"),
                    b"JmpNZ" => {
                        assemble_jump_opcode_instr(&mut sl_lexer, hhbc::Opcode::JmpNZ, "JmpNZ")
                    }
                    b"Enter" => {
                        assemble_jump_opcode_instr(&mut sl_lexer, hhbc::Opcode::Enter, "Enter")
                    }
                    b"FCallFuncD" | b"FCallCtor" | b"FCallClsMethod" | b"FCallClsMethodM"
                    | b"FCallClsMethodD" | b"FCallClsMethodS" | b"FCallClsMethodSD"
                    | b"FCallFunc" | b"FCallObjMethod" | b"FCallObjMethodD" => {
                        assemble_fcall(alloc, &mut sl_lexer)
                    }
                    b"IncDecL" => assemble_incdecl_opcode(&mut sl_lexer, decl_map),
                    b"IncDecG" => {
                        assemble_incdec_opcode(&mut sl_lexer, hhbc::Opcode::IncDecG, "IncDecG")
                    }
                    b"IncDecS" => {
                        assemble_incdec_opcode(&mut sl_lexer, hhbc::Opcode::IncDecS, "IncDecS")
                    }
                    b"IsTypeStructC" => assemble_is_type_struct_c(&mut sl_lexer),
                    b"IsTypeC" => assemble_is_type_c(&mut sl_lexer),
                    b"IsTypeL" => assemble_is_type_l(&mut sl_lexer, decl_map),
                    b"IterFree" => assemble_iter_free(&mut sl_lexer),
                    b"IterInit" => assemble_iter_init_iter_next(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::IterInit,
                        "IterInit",
                    ),
                    b"IterNext" => assemble_iter_init_iter_next(
                        &mut sl_lexer,
                        decl_map,
                        hhbc::Opcode::IterNext,
                        "IterNext",
                    ),
                    b"BaseGC" => {
                        assemble_base_gc_or_c(&mut sl_lexer, hhbc::Opcode::BaseGC, "BaseGC")
                    }
                    b"BaseGL" => assemble_base_gl(&mut sl_lexer, decl_map),
                    b"BaseSC" => assemble_base_sc(&mut sl_lexer),
                    b"BaseL" => assemble_base_l(&mut sl_lexer, decl_map),
                    b"BaseC" => assemble_base_gc_or_c(&mut sl_lexer, hhbc::Opcode::BaseC, "BaseC"),
                    b"BaseH" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::BaseH, "BaseH")
                    }
                    b"Dim" => assemble_dim(alloc, &mut sl_lexer, decl_map),
                    b"QueryM" => assemble_query_m(alloc, &mut sl_lexer, decl_map),

                    b"IncDecM" => assemble_inc_dec_m(alloc, &mut sl_lexer, decl_map),

                    b"RetM" => assemble_retm_opcode_instr(&mut sl_lexer),
                    b"NewObj" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::NewObj,
                        "NewObj",
                    ),
                    b"NewObjR" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::NewObjR,
                        "NewObjR",
                    ),
                    b"NewObjD" => assemble_obj_class_name_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::NewObjD,
                        "NewObjD",
                    ),
                    b"NewObjRD" => assemble_obj_class_name_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::NewObjRD,
                        "NewObjRD",
                    ),
                    b"NewObjS" => {
                        sl_lexer.next();
                        Ok(hhbc::Instruct::Opcode(hhbc::Opcode::NewObjS(
                            assemble_special_class_ref(&mut sl_lexer)?,
                        )))
                    }
                    b"LockObj" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::LockObj,
                        "LockObj",
                    ),
                    b"Dup" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Dup, "Dup")
                    }
                    b"Throw" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Throw, "Throw")
                    }
                    b"InstanceOfD" => assemble_obj_class_name_instr(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::InstanceOfD,
                        "InstanceOfD",
                    ),
                    b"CreateCl" => assemble_create_cl(&mut sl_lexer),
                    b"ResolveFunc" => assemble_resolve_func(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::ResolveFunc,
                        "ResolveFunc",
                    ),
                    b"ResolveMethCaller" => assemble_resolve_func(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::ResolveMethCaller,
                        "ResolveMethCaller",
                    ),
                    b"ResolveRFunc" => assemble_resolve_func(
                        alloc,
                        &mut sl_lexer,
                        hhbc::Opcode::ResolveRFunc,
                        "ResolveRFunc",
                    ),
                    b"ResolveClsMethod" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveClsMethodD" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveClsMethodS" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveRClsMethod" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveRClsMethodD" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveRClsMethodS" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"ResolveClass" => assemble_resolve_class(alloc, &mut sl_lexer),
                    b"NewDictArray" => assemble_u32_carrying_opcode(
                        &mut sl_lexer,
                        hhbc::Opcode::NewDictArray,
                        "NewDictArray",
                    ),
                    b"NewVec" => {
                        assemble_u32_carrying_opcode(&mut sl_lexer, hhbc::Opcode::NewVec, "NewVec")
                    }
                    b"NewKeysetArray" => assemble_u32_carrying_opcode(
                        &mut sl_lexer,
                        hhbc::Opcode::NewKeysetArray,
                        "NewKeysetArray",
                    ),
                    b"ConcatN" => assemble_u32_carrying_opcode(
                        &mut sl_lexer,
                        hhbc::Opcode::ConcatN,
                        "ConcatN",
                    ),
                    b"CombineAndResolveTypeStruct" => assemble_u32_carrying_opcode(
                        &mut sl_lexer,
                        hhbc::Opcode::CombineAndResolveTypeStruct,
                        "CombineAndResolveTypeStruct",
                    ),
                    b"SetL"
                    | b"SetG"
                    | b"SetS"
                    | b"SetOpL"
                    | b"SetOpG"
                    | b"SetOpS"
                    | b"SetImplicitContextByValue"
                    | b"SetM"
                    | b"SetRangeM"
                    | b"SetOpM"
                    | b"UnsetL"
                    | b"IsUnsetL"
                    | b"UnsetG"
                    | b"UnsetM" => assemble_set_instruct(alloc, &mut sl_lexer, decl_map),
                    b"CnsE" | b"ClsCns" | b"ClsCnsD" | b"ClsCnsL" => {
                        assemble_cns(alloc, &mut sl_lexer, decl_map)
                    }
                    b"VerifyRetTypeC" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::VerifyRetTypeC,
                        "VerifyRetTypeC",
                    ),
                    b"VerifyRetTypeTS" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::VerifyRetTypeTS,
                        "VerifyRetTypeTS",
                    ),
                    b"VerifyRetNonNullC" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::VerifyRetNonNullC,
                        "VerifyRetNonNullC",
                    ),
                    b"SelfCls" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::SelfCls,
                        "SelfCls",
                    ),
                    b"ParentCls" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ParentCls,
                        "ParentCls",
                    ),
                    b"LateBoundCls" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::LateBoundCls,
                        "LateBoundCls",
                    ),
                    b"RecordReifiedGeneric" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::RecordReifiedGeneric,
                        "RecordReifiedGeneric",
                    ),
                    b"CheckClsReifiedGenericMismatch" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CheckClsReifiedGenericMismatch,
                        "CheckClsReifiedGenericMismatch",
                    ),
                    b"ClassHasReifiedGenerics" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ClassHasReifiedGenerics,
                        "ClassHasReifiedGenerics",
                    ),
                    b"HasReifiedParent" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::HasReifiedParent,
                        "HasReifiedParent",
                    ),
                    b"NativeImpl" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::NativeImpl,
                        "NativeImpl",
                    ),
                    b"CGetCUNop" | b"CUGetCUNop" | b"CGetL" | b"CGetQuietL" | b"CUGetL"
                    | b"CGetL2" | b"CGetG" | b"CGetS" | b"ClassGetC" | b"ClassGetTS" => {
                        assemble_cget(&mut sl_lexer, decl_map)
                    }
                    b"Fatal" => assemble_fatal_opcode(&mut sl_lexer),
                    b"CheckThis" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CheckThis,
                        "CheckThis",
                    ),
                    b"This" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::This, "This")
                    }
                    b"CreateCont" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CreateCont,
                        "CreateCont",
                    ),
                    b"Yield" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Yield, "Yield")
                    }
                    b"File" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::File, "File")
                    }
                    b"Incl" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Incl, "Incl")
                    }
                    b"AddElemC" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::AddElemC,
                        "AddElemC",
                    ),
                    b"ClassName" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ClassName,
                        "ClassName",
                    ),
                    b"Req" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Req, "Req")
                    }
                    b"ReqDoc" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ReqDoc,
                        "ReqDoc",
                    ),
                    b"ReqOnce" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ReqOnce,
                        "ReqOnce",
                    ),
                    b"BareThis" => assemble_bare_this_opcode(&mut sl_lexer),
                    b"ColFromArray" => assemble_col_from_array(&mut sl_lexer),
                    b"NewCol" => assemble_new_col(&mut sl_lexer),
                    b"NewStructDict" => assemble_new_struct_dict(alloc, &mut sl_lexer),
                    b"IssetG" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::IssetG,
                        "IssetG",
                    ),
                    b"IssetS" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::IssetS,
                        "IssetS",
                    ),
                    b"NewPair" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::NewPair,
                        "NewPair",
                    ),
                    b"Clone" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Clone, "Clone")
                    }
                    b"Idx" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Idx, "Idx")
                    }
                    b"YieldK" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::YieldK,
                        "YieldK",
                    ),
                    b"ArrayUnmarkLegacy" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ArrayUnmarkLegacy,
                        "ArrayUnmarkLegacy",
                    ),
                    b"ArrayMarkLegacy" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ArrayMarkLegacy,
                        "ArrayMarkLegacy",
                    ),
                    b"AKExists" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::AKExists,
                        "AKExists",
                    ),
                    b"Await" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Await, "Await")
                    }
                    b"InclOnce" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::InclOnce,
                        "InclOnce",
                    ),
                    b"OODeclExists" => assemble_oodecl_exists(&mut sl_lexer),
                    b"Silence" => assemble_silence(&mut sl_lexer, decl_map),
                    b"ThrowNonExhaustiveSwitch" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ThrowNonExhaustiveSwitch,
                        "ThrowNonExhaustiveSwitch",
                    ),
                    b"ArrayIdx" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ArrayIdx,
                        "ArrayIdx",
                    ),
                    b"ChainFaults" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ChainFaults,
                        "ChainFaults",
                    ),
                    b"CheckProp" => assemble_check_prop(alloc, &mut sl_lexer),
                    b"InitProp" => assemble_init_prop(alloc, &mut sl_lexer),
                    b"VerifyImplicitContextState" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::VerifyImplicitContextState,
                        "VerifyImplicitContextState",
                    ),
                    b"MemoGet" => assemble_memo_get(&mut sl_lexer),
                    b"MemoSet" => assemble_memo_set(&mut sl_lexer),
                    b"Switch" => assemble_switch(alloc, &mut sl_lexer),
                    b"SSwitch" => assemble_sswitch(alloc, &mut sl_lexer),
                    b"Eval" => {
                        assemble_single_opcode_instr(&mut sl_lexer, || hhbc::Opcode::Eval, "Eval")
                    }
                    b"ThrowAsTypeStructException" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::ThrowAsTypeStructException,
                        "ThrowAsTypeStructException",
                    ),
                    b"AwaitAll" => assemble_await_all(&mut sl_lexer),
                    b"WHResult" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::WHResult,
                        "WHResult",
                    ),
                    b"AddNewElemC" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::AddNewElemC,
                        "AddNewElemC",
                    ),
                    b"IsLateBoundCls" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::IsLateBoundCls,
                        "IsLateBoundCls",
                    ),
                    b"FuncCred" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::FuncCred,
                        "FuncCred",
                    ),
                    b"MemoGetEager" => assemble_memo_get_eager(&mut sl_lexer),
                    b"MemoSetEager" => assemble_memo_set_eager(&mut sl_lexer),
                    b"RetCSuspended" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::RetCSuspended,
                        "RetCSuspended",
                    ),
                    b"CheckClsRGSoft" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::CheckClsRGSoft,
                        "CheckClsRGSoft",
                    ),
                    b"GetClsRGProp" => assemble_single_opcode_instr(
                        &mut sl_lexer,
                        || hhbc::Opcode::GetClsRGProp,
                        "GetClsRGProp",
                    ),
                    _ => todo!("assembling instrs: {}", tok),
                }
            } else {
                bail!("Something wrong in assemble_instr") // Shouldn't happen, we peek_if so there must be something
            }
        } else {
            bail!(
                "Function body line that's neither decl or identifier: {}",
                sl_lexer.next().unwrap() // Know there is something here because fetch_until_new_line won't return empty
            );
        }
    } else {
        bail!("Expected an additional instruction in body, reached EOF");
    }
}

/// .srcloc #:#,#:#;
fn assemble_srcloc(token_iter: &mut Lexer<'_>) -> Result<hhbc::SrcLoc> {
    token_iter.expect_is_str(Token::into_decl, ".srcloc")?;
    let tr = hhbc::SrcLoc {
        line_begin: token_iter.expect_and_get_number()?,
        col_begin: {
            token_iter.expect(Token::into_colon)?;
            token_iter.expect_and_get_number()?
        },
        line_end: {
            token_iter.expect(Token::into_comma)?;
            token_iter.expect_and_get_number()?
        },
        col_end: {
            token_iter.expect(Token::into_colon)?;
            token_iter.expect_and_get_number()?
        },
    };
    token_iter.expect(Token::into_semicolon)?;
    token_iter.expect_end()?;
    Ok(tr)
}

/// <(fcallargflag)*>
fn assemble_fcallargsflags(token_iter: &mut Lexer<'_>) -> Result<hhbc::FCallArgsFlags> {
    let mut flags = hhbc::FCallArgsFlags::FCANone;
    token_iter.expect(Token::into_lt)?;
    while !token_iter.peek_if(Token::is_gt) {
        let f = token_iter.expect(Token::into_identifier)?;
        match f {
            b"Unpack" => flags.add(hhbc::FCallArgsFlags::HasUnpack),
            b"Generics" => flags.add(hhbc::FCallArgsFlags::HasGenerics),
            b"LockWhileUnwinding" => flags.add(hhbc::FCallArgsFlags::LockWhileUnwinding),
            b"SkipRepack" => flags.add(hhbc::FCallArgsFlags::SkipRepack),
            b"SkipCoeffectsCheck" => flags.add(hhbc::FCallArgsFlags::SkipCoeffectsCheck),
            b"EnforceMutableReturn" => flags.add(hhbc::FCallArgsFlags::EnforceMutableReturn),
            b"EnforceReadonlyThis" => flags.add(hhbc::FCallArgsFlags::EnforceReadonlyThis),
            b"ExplicitContext" => flags.add(hhbc::FCallArgsFlags::ExplicitContext),
            b"HasInOut" => flags.add(hhbc::FCallArgsFlags::HasInOut),
            b"EnforceInOut" => flags.add(hhbc::FCallArgsFlags::EnforceInOut),
            b"EnforceReadonly" => flags.add(hhbc::FCallArgsFlags::EnforceReadonly),
            b"HasAsyncEagerOffset" => flags.add(hhbc::FCallArgsFlags::HasAsyncEagerOffset),
            b"NumArgsStart" => flags.add(hhbc::FCallArgsFlags::NumArgsStart),
            f => bail!("Unrecognized FCallArgsFlags: {:?}", f),
        }
    }
    token_iter.expect(Token::into_gt)?;
    Ok(flags)
}

/// "(0|1)*"
fn assemble_inouts_or_readonly<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Slice<'arena, bool>> {
    let literal = token_iter.expect(Token::into_str_literal)?; //
    debug_assert!(literal[0] == b'"' && literal[literal.len() - 1] == b'"');
    let tr: Result<Vec<bool>, _> = literal[1..literal.len() - 1] //trims the outer "", which are guaranteed b/c of str token
        .iter()
        .map(|c| {
            Ok(if *c == b'0' {
                false
            } else if *c == b'1' {
                true
            } else {
                bail!("Non 0/1 character in inouts/readonlys")
            })
        })
        .collect();
    Ok(Slice::from_vec(alloc, tr?))
}

/// - or a label
fn assemble_async_eager_target(token_iter: &mut Lexer<'_>) -> Result<Option<hhbc::Label>> {
    if token_iter.next_if(Token::is_dash) {
        Ok(None)
    } else {
        Ok(Some(assemble_label(token_iter, false)?))
    }
}

/// Just a string literal
fn assemble_fcall_context<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Str<'arena>> {
    let st = token_iter.expect(Token::into_str_literal)?;
    debug_assert!(st[0] == b'"' && st[st.len() - 1] == b'"');
    Ok(Str::new_slice(alloc, &st[1..st.len() - 1])) // if not hugged by "", won't pass into_str_literal
}

fn assemble_is_log_as_dynamic_call_op(
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::IsLogAsDynamicCallOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"LogAsDynamicCall" => Ok(hhbc::IsLogAsDynamicCallOp::LogAsDynamicCall),
        b"DontLogAsDynamicCall" => Ok(hhbc::IsLogAsDynamicCallOp::DontLogAsDynamicCall),
        b => bail!("Unknown IsLogAsDynamicCallOp: {:?}", b),
    }
}

fn assemble_special_class_ref(token_iter: &mut Lexer<'_>) -> Result<hhbc::SpecialClsRef> {
    match token_iter.expect(Token::into_identifier)? {
        b"SelfCls" => Ok(hhbc::SpecialClsRef::SelfCls),
        b"ParentCls" => Ok(hhbc::SpecialClsRef::ParentCls),
        b"LateBoundCls" => Ok(hhbc::SpecialClsRef::LateBoundCls),
        b => bail!("Unknown SpecialClassRef: {:?}", b),
    }
}

fn assemble_obj_method_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::ObjMethodOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"NullThrows" => Ok(hhbc::ObjMethodOp::NullThrows),
        b"NullSafe" => Ok(hhbc::ObjMethodOp::NullSafe),
        b => bail!("Unknown ObjMethodOp: {:?}", b),
    }
}

/// <(fcargflags)*> numargs numrets inouts readonly async_eager_target context
fn assemble_fcall_args<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::FCallArgs<'arena>> {
    let fcargflags = assemble_fcallargsflags(token_iter)?;
    let num_args = token_iter.expect_and_get_number()?;
    let num_rets = token_iter.expect_and_get_number()?;
    let inouts = assemble_inouts_or_readonly(alloc, token_iter)?;
    let readonly = assemble_inouts_or_readonly(alloc, token_iter)?;
    let async_eager_target = assemble_async_eager_target(token_iter)?;
    let context = assemble_fcall_context(alloc, token_iter)?;
    let fcargs = hhbc::FCallArgs::new(
        fcargflags,
        num_rets,
        num_args,
        inouts,
        readonly,
        async_eager_target,
        None,
    );
    Ok(hhbc::FCallArgs { context, ..fcargs })
}

fn assemble_unescaped_unquoted_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Str<'arena>> {
    let st = escaper::unescape_literal_bytes_into_vec_bytes(escaper::unquote_slice(
        token_iter.expect(Token::into_str_literal)?,
    ))?;
    Ok(Str::new_slice(alloc, &st))
}

fn assemble_unescaped_unquoted_triple_str<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<Str<'arena>> {
    let st = escaper::unquote_slice(escaper::unquote_slice(escaper::unquote_slice(
        token_iter.expect(Token::into_triple_str_literal)?,
    )));
    let st = escaper::unescape_literal_bytes_into_vec_bytes(st)?;
    Ok(Str::new_slice(alloc, &st))
}

/// Ex:
/// FCallClsMethodM <EnforceMutableReturn> 0 1 "" "" - "" "" DontLogAsDynamicCall "foo"
/// Opcode fargs str is_log_as_dynamic_call method_name_in_quotes
fn assemble_fcall<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    use hhbc::Opcode;
    let tok = token_iter
        .next()
        .ok_or_else(|| anyhow!("Expected an additional instruction in body, reached EOF"))?;
    let args = assemble_fcall_args(alloc, token_iter)?;
    let fcall = match tok.as_bytes() {
        b"FCallFuncD" => {
            Opcode::FCallFuncD(args, assemble_function_name_from_str(alloc, token_iter)?)
        }
        b"FCallCtor" => {
            Opcode::FCallCtor(args, assemble_unescaped_unquoted_str(alloc, token_iter)?)
        }
        b"FCallClsMethodM" => Opcode::FCallClsMethodM(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_is_log_as_dynamic_call_op(token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"FCallClsMethodSD" => Opcode::FCallClsMethodSD(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_special_class_ref(token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"FCallObjMethodD" => Opcode::FCallObjMethodD(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_obj_method_op(token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"FCallClsMethod" => Opcode::FCallClsMethod(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_is_log_as_dynamic_call_op(token_iter)?,
        ),
        b"FCallClsMethodS" => Opcode::FCallClsMethodS(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_special_class_ref(token_iter)?,
        ),
        b"FCallObjMethod" => Opcode::FCallObjMethod(
            args,
            assemble_unescaped_unquoted_str(alloc, token_iter)?,
            assemble_obj_method_op(token_iter)?,
        ),
        b"FCallClsMethodD" => Opcode::FCallClsMethodD(
            args,
            assemble_class_name_from_str(alloc, token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"FCallFunc" => Opcode::FCallFunc(args),
        _ => bail!("Unknown fcall: {}", tok),
    };
    Ok(hhbc::Instruct::Opcode(fcall))
}

fn assemble_is_type_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::IsTypeOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"Null" => Ok(hhbc::IsTypeOp::Null),
        b"Bool" => Ok(hhbc::IsTypeOp::Bool),
        b"Int" => Ok(hhbc::IsTypeOp::Int),
        b"Dbl" => Ok(hhbc::IsTypeOp::Dbl),
        b"Str" => Ok(hhbc::IsTypeOp::Str),
        b"Obj" => Ok(hhbc::IsTypeOp::Obj),
        b"Res" => Ok(hhbc::IsTypeOp::Res),
        b"Scalar" => Ok(hhbc::IsTypeOp::Scalar),
        b"Keyset" => Ok(hhbc::IsTypeOp::Keyset),
        b"Dict" => Ok(hhbc::IsTypeOp::Dict),
        b"Vec" => Ok(hhbc::IsTypeOp::Vec),
        b"ArrLike" => Ok(hhbc::IsTypeOp::ArrLike),
        b"ClsMeth" => Ok(hhbc::IsTypeOp::ClsMeth),
        b"Func" => Ok(hhbc::IsTypeOp::Func),
        b"LegacyArrLike" => Ok(hhbc::IsTypeOp::LegacyArrLike),
        b"Class" => Ok(hhbc::IsTypeOp::Class),
        ito => bail!("Unknown IsTypeOp: {:?}", ito),
    }
}

/// NewObjD "Foo" or NewObjRD "Foo"
fn assemble_obj_class_name_instr<
    'arena,
    F: FnOnce(hhbc::ClassName<'arena>) -> hhbc::Opcode<'arena>,
>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    Ok(hhbc::Instruct::Opcode(op_con(
        assemble_class_name_from_str(alloc, token_iter)?,
    )))
}

/// Ex: IterFree 0
fn assemble_iter_free<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IterFree")?;
    let iter_id = hhbc::IterId {
        idx: token_iter.expect_and_get_number()?,
    };
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IterFree(iter_id)))
}

/// Ex:
/// IterInit: IterInit 0 NK V:$v L0 (IterInit IterArgs Label)
/// IterNext: IterNext 0 NK V:$v L1 (IterInit IterArgs Label)
fn assemble_iter_init_iter_next<
    'arena,
    F: FnOnce(hhbc::IterArgs, hhbc::Label) -> hhbc::Opcode<'arena>,
>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    let iter_args = assemble_iter_args(token_iter, decl_map)?;
    let lbl = assemble_label(token_iter, false)?;
    Ok(hhbc::Instruct::Opcode(op_con(iter_args, lbl)))
}

/// IterArg { iter_id: IterId (~u32), key_id: Local, val_id: Local}
/// Ex: 0 NK V:$v
fn assemble_iter_args(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::IterArgs> {
    let idx: u32 = token_iter.expect_and_get_number()?;
    let key_id: hhbc::Local = match token_iter.expect(Token::into_identifier)? {
        b"NK" => hhbc::Local::INVALID,
        b"K" => {
            token_iter.expect(Token::into_colon)?;
            assemble_local(token_iter, decl_map)?
        }
        o => bail!("Invalid key_id given as iter args to IterArg: {:?}", o),
    };
    token_iter.expect_is_str(Token::into_identifier, "V")?;
    token_iter.expect(Token::into_colon)?;
    Ok(hhbc::IterArgs {
        iter_id: hhbc::IterId { idx },
        key_id,
        val_id: assemble_local(token_iter, decl_map)?,
    })
}

/// Ex: IsTypeL $a Obj
fn assemble_is_type_l<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IsTypeL")?;
    let lcl = assemble_local(token_iter, decl_map)?;
    let type_op = assemble_is_type_op(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IsTypeL(lcl, type_op)))
}

/// Ex: IsTypeC Obj
fn assemble_is_type_c<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IsTypeC")?;
    let type_op = assemble_is_type_op(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IsTypeC(type_op)))
}

/// IsTypeStructC (Resolve|DontResolve)
fn assemble_is_type_struct_c<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IsTypeStructC")?;
    let res_op = token_iter.expect(Token::into_identifier)?;
    match res_op {
        b"Resolve" => Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IsTypeStructC(
            hhbc::TypeStructResolveOp::Resolve,
        ))),
        b"DontResolve" => Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IsTypeStructC(
            hhbc::TypeStructResolveOp::DontResolve,
        ))),
        _ => bail!(
            "Unknown TypeStructResolveOp passed to TypeStructC instr: {:?}",
            res_op
        ),
    }
}

fn assemble_switch_kind(token_iter: &mut Lexer<'_>) -> Result<hhbc::SwitchKind> {
    let sk = token_iter.expect(Token::into_identifier)?;
    match sk {
        b"Unbounded" => Ok(hhbc::SwitchKind::Unbounded),
        b"Bounded" => Ok(hhbc::SwitchKind::Bounded),
        sk => bail!("Unknown switch kind: {:?}", sk),
    }
}

/// Switch(SwitchKind, i64, BumpSliceMut<'arena, Label>),
/// Switch SwitchKind i64 < label* >
fn assemble_switch<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Switch")?;
    let sk = assemble_switch_kind(token_iter)?;
    let int = token_iter.expect_and_get_number()?;
    let mut labels = Vec::new();
    token_iter.expect(Token::into_lt)?;
    while !token_iter.peek_if(Token::is_gt) {
        labels.push(assemble_label(token_iter, false)?)
    }
    token_iter.expect(Token::into_gt)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Switch(
        sk,
        int,
        Slice::from_vec(alloc, labels),
    )))
}

/// Ex:
/// SSwitch <"ARRAY7":L0 "ARRAY8":L1 "ARRAY9":L2 -:L3>
fn assemble_sswitch<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    let mut cases = Vec::new(); // Of Str<'arena>
    let mut targets = Vec::new(); // Of Labels
    token_iter.expect_is_str(Token::into_identifier, "SSwitch")?;
    token_iter.expect(Token::into_lt)?;
    // The last case is printed as '-' but interior it is "default"
    while !token_iter.peek_if(Token::is_gt) {
        if token_iter.peek_if(Token::is_dash) {
            // Last item; printed as '-' but is "default"
            token_iter.next();
            cases.push(Str::new_str(alloc, "default"));
        } else {
            cases.push(assemble_unescaped_unquoted_str(alloc, token_iter)?);
        }
        token_iter.expect(Token::into_colon)?;
        targets.push(assemble_label(token_iter, false)?);
    }
    token_iter.expect(Token::into_gt)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::SSwitch {
        cases: Slice::from_vec(alloc, cases),
        targets: Slice::from_vec(alloc, targets),
        _0: hhbc::Dummy::DEFAULT,
    }))
}

fn assemble_await_all<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "AwaitAll")?;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::AwaitAll(lcl_range)))
}

/// Ex:
/// MemoGetEager L0 L1 l:0+0
fn assemble_memo_get_eager<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "MemoGetEager")?;
    let lbl_slice = [
        assemble_label(token_iter, false)?,
        assemble_label(token_iter, false)?,
    ];
    let dum = hhbc::Dummy::DEFAULT;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::MemoGetEager(
        lbl_slice, dum, lcl_range,
    )))
}

/// Ex:
/// MemoSetEager L:0+0
fn assemble_memo_set_eager<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "MemoSetEager")?;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::MemoSetEager(
        lcl_range,
    )))
}

/// Ex:
/// MemoGet L0 L:0+0
fn assemble_memo_get<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "MemoGet")?;
    let lbl = assemble_label(token_iter, false)?;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::MemoGet(
        lbl, lcl_range,
    )))
}

/// Ex:
/// MemoSet L:0+0
fn assemble_memo_set<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "MemoSet")?;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::MemoSet(lcl_range)))
}

/// Ex:
/// MemoGet L0 L:0+0
/// last item is a local range
fn assemble_local_range(token_iter: &mut Lexer<'_>) -> Result<hhbc::LocalRange> {
    token_iter.expect_is_str(Token::into_identifier, "L")?;
    token_iter.expect(Token::into_colon)?;
    let start = hhbc::Local {
        idx: token_iter.expect_and_get_number()?,
    };
    //token_iter.expect(Token::into_plus)?; // Not sure if this exists yet
    let len = token_iter.expect_and_get_number()?;
    Ok(hhbc::LocalRange { start, len })
}

/// StackIndex : u32
fn assemble_stack_index(token_iter: &mut Lexer<'_>) -> Result<hhbc::StackIndex> {
    token_iter.expect_and_get_number()
}

fn assemble_mop_mode(token_iter: &mut Lexer<'_>) -> Result<hhbc::MOpMode> {
    match token_iter.expect(Token::into_identifier)? {
        b"None" => Ok(hhbc::MOpMode::None),
        b"Warn" => Ok(hhbc::MOpMode::Warn),
        b"Define" => Ok(hhbc::MOpMode::Define),
        b"Unset" => Ok(hhbc::MOpMode::Unset),
        b"InOut" => Ok(hhbc::MOpMode::InOut),
        mop => bail!("Expected a MOpMode but got: {:?}", mop),
    }
}

fn assemble_readonly_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::ReadonlyOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"Any" => Ok(hhbc::ReadonlyOp::Any),
        b"Readonly" => Ok(hhbc::ReadonlyOp::Readonly),
        b"Mutable" => Ok(hhbc::ReadonlyOp::Mutable),
        b"CheckROCOW" => Ok(hhbc::ReadonlyOp::CheckROCOW),
        b"CheckMutROCOW" => Ok(hhbc::ReadonlyOp::CheckMutROCOW),
        rop => {
            bail!("Expected a ReadonlyOp but got: {:?}", rop)
        }
    }
}

/// EC: stackIndex readOnlyOp | EL: local readOnlyOp | ET: string readOnlyOp | EI: int readOnlyOp
/// PC: stackIndex readOnlyOp | PL: local readOnlyOp | PT: propName readOnlyOp | QT: propName readOnlyOp
fn assemble_member_key<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::MemberKey<'arena>> {
    match token_iter.expect(Token::into_identifier)? {
        b"EC" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::EC(
                assemble_stack_index(token_iter)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"EL" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::EL(
                assemble_local(token_iter, decl_map)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"ET" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::ET(
                Str::new_slice(
                    alloc,
                    &escaper::unescape_literal_bytes_into_vec_bytes(
                        // In bp, print_quoted_str also escapes the string
                        escaper::unquote_slice(token_iter.expect(Token::into_str_literal)?),
                    )?,
                ),
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"EI" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::EI(
                token_iter.expect_and_get_number()?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"PC" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::PC(
                assemble_stack_index(token_iter)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"PL" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::PL(
                assemble_local(token_iter, decl_map)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"PT" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::PT(
                assemble_prop_name_from_str(alloc, token_iter)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"QT" => {
            token_iter.expect(Token::into_colon)?;
            Ok(hhbc::MemberKey::QT(
                assemble_prop_name_from_str(alloc, token_iter)?,
                assemble_readonly_op(token_iter)?,
            ))
        }
        b"W" => Ok(hhbc::MemberKey::W),
        mk => {
            bail!("Expected a MemberKey but got: {:?}", mk)
        }
    }
}

/// Dim MOpMode MemberKey
fn assemble_dim<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Dim")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Dim(
        assemble_mop_mode(token_iter)?,
        assemble_member_key(alloc, token_iter, decl_map)?,
    )))
}

fn assemble_query_mop(token_iter: &mut Lexer<'_>) -> Result<hhbc::QueryMOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"CGet" => Ok(hhbc::QueryMOp::CGet),
        b"CGetQuiet" => Ok(hhbc::QueryMOp::CGetQuiet),
        b"Isset" => Ok(hhbc::QueryMOp::Isset),
        b"InOut" => Ok(hhbc::QueryMOp::InOut),
        q => bail!("Unexpected QueryMOp: {:?}", q),
    }
}

/// Ex: QueryM 1 CGet EI:1 Any
/// QueryM <stackIndex> <QueryMOp> <MemberKey>
fn assemble_query_m<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "QueryM")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::QueryM(
        assemble_stack_index(token_iter)?,
        assemble_query_mop(token_iter)?,
        assemble_member_key(alloc, token_iter, decl_map)?,
    )))
}

/// IncDecM stackIndex IncDecOp MemberKey
fn assemble_inc_dec_m<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IncDecM")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IncDecM(
        assemble_stack_index(token_iter)?,
        assemble_inc_dec_op(token_iter)?,
        assemble_member_key(alloc, token_iter, decl_map)?,
    )))
}

/// BaseGL $var MOpMode
fn assemble_base_gl<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "BaseGL")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::BaseGL(
        assemble_local(token_iter, decl_map)?,
        assemble_mop_mode(token_iter)?,
    )))
}

/// BaseSC stackIndex stackIndex MOpMode ReadOnlyOp
fn assemble_base_sc<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "BaseSC")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::BaseSC(
        assemble_stack_index(token_iter)?,
        assemble_stack_index(token_iter)?,
        assemble_mop_mode(token_iter)?,
        assemble_readonly_op(token_iter)?,
    )))
}

/// BaseL $var MOpMode ReadOnlyOp
fn assemble_base_l<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "BaseL")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::BaseL(
        assemble_local(token_iter, decl_map)?,
        assemble_mop_mode(token_iter)?,
        assemble_readonly_op(token_iter)?,
    )))
}

/// Ex: BaseC 0 Warn
/// BaseGC 0 Warn
fn assemble_base_gc_or_c<
    'arena,
    F: FnOnce(hhbc::StackIndex, hhbc::MOpMode) -> hhbc::Opcode<'arena>,
>(
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    Ok(hhbc::Instruct::Opcode(op_con(
        assemble_stack_index(token_iter)?,
        assemble_mop_mode(token_iter)?,
    )))
}

fn assemble_set_range_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::SetRangeOp> {
    let sro = match token_iter.expect(Token::into_identifier)? {
        b"Forward" => hhbc::SetRangeOp::Forward,
        b"Reverse" => hhbc::SetRangeOp::Reverse,
        sro => bail!("Unknown SetRangeOp: {:?}", sro),
    };
    Ok(sro)
}

/// Ex:
/// CnsE "E_NOTICE"
fn assemble_cns<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    let op = match token_iter.expect(Token::into_identifier)? {
        b"CnsE" => hhbc::Opcode::CnsE(assemble_const_name_from_str(alloc, token_iter)?),
        b"ClsCns" => hhbc::Opcode::ClsCns(assemble_const_name_from_str(alloc, token_iter)?),
        b"ClsCnsD" => hhbc::Opcode::ClsCnsD(
            assemble_const_name_from_str(alloc, token_iter)?,
            assemble_class_name_from_str(alloc, token_iter)?,
        ),
        b"ClsCnsL" => hhbc::Opcode::ClsCnsL(assemble_local(token_iter, decl_map)?),
        cns => bail!("Unknown cns opcode: {:?}", cns),
    };
    Ok(hhbc::Instruct::Opcode(op))
}

/// Ex:
/// SetL $value_from_func
/// SetG
/// SetS Any
/// SetOpL $ret ConcatEqual
/// SetOpS PlusEqual
/// SetImplicitContextByValue
/// UnsetL _4
fn assemble_set_instruct<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    let op = match token_iter.expect(Token::into_identifier)? {
        b"SetL" => hhbc::Opcode::SetL(assemble_local(token_iter, decl_map)?),
        b"SetG" => hhbc::Opcode::SetG,
        b"SetS" => hhbc::Opcode::SetS(assemble_readonly_op(token_iter)?),
        b"SetOpL" => hhbc::Opcode::SetOpL(
            assemble_local(token_iter, decl_map)?,
            assemble_set_op_op(token_iter)?,
        ),
        b"SetOpG" => hhbc::Opcode::SetOpG(assemble_set_op_op(token_iter)?),
        b"SetOpS" => hhbc::Opcode::SetOpS(assemble_set_op_op(token_iter)?),
        b"SetImplicitContextByValue" => hhbc::Opcode::SetImplicitContextByValue,
        b"SetM" => hhbc::Opcode::SetM(
            assemble_stack_index(token_iter)?,
            assemble_member_key(alloc, token_iter, decl_map)?,
        ),
        b"SetRangeM" => hhbc::Opcode::SetRangeM(
            assemble_stack_index(token_iter)?,
            token_iter.expect_and_get_number()?,
            assemble_set_range_op(token_iter)?,
        ),
        b"SetOpM" => hhbc::Opcode::SetOpM(
            assemble_stack_index(token_iter)?,
            assemble_set_op_op(token_iter)?,
            assemble_member_key(alloc, token_iter, decl_map)?,
        ),
        b"UnsetL" => hhbc::Opcode::UnsetL(assemble_local(token_iter, decl_map)?),
        b"IsUnsetL" => hhbc::Opcode::IsUnsetL(assemble_local(token_iter, decl_map)?),
        b"UnsetG" => hhbc::Opcode::UnsetG,
        b"UnsetM" => hhbc::Opcode::UnsetM(
            assemble_stack_index(token_iter)?,
            assemble_member_key(alloc, token_iter, decl_map)?,
        ),
        _ => todo!(),
    };
    Ok(hhbc::Instruct::Opcode(op))
}

fn assemble_set_op_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::SetOpOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"PlusEqual" => Ok(hhbc::SetOpOp::PlusEqual),
        b"MinusEqual" => Ok(hhbc::SetOpOp::MinusEqual),
        b"MulEqual" => Ok(hhbc::SetOpOp::MulEqual),
        b"ConcatEqual" => Ok(hhbc::SetOpOp::ConcatEqual),
        b"DivEqual" => Ok(hhbc::SetOpOp::DivEqual),
        b"PowEqual" => Ok(hhbc::SetOpOp::PowEqual),
        b"ModEqual" => Ok(hhbc::SetOpOp::ModEqual),
        b"AndEqual" => Ok(hhbc::SetOpOp::AndEqual),
        b"OrEqual" => Ok(hhbc::SetOpOp::OrEqual),
        b"XorEqual" => Ok(hhbc::SetOpOp::XorEqual),
        b"SlEqual" => Ok(hhbc::SetOpOp::SlEqual),
        b"SrEqual" => Ok(hhbc::SetOpOp::SrEqual),
        b"PlusEqualO" => Ok(hhbc::SetOpOp::PlusEqualO),
        b"MinusEqualO" => Ok(hhbc::SetOpOp::MinusEqualO),
        b"MulEqualO" => Ok(hhbc::SetOpOp::MulEqualO),
        sop => bail!("Expected a SetOpOp but got: {:?}", sop),
    }
}

fn assemble_inc_dec_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::IncDecOp> {
    match token_iter.expect(Token::into_identifier)? {
        b"PreInc" => Ok(hhbc::IncDecOp::PreInc),
        b"PostInc" => Ok(hhbc::IncDecOp::PostInc),
        b"PreDec" => Ok(hhbc::IncDecOp::PreDec),
        b"PostDec" => Ok(hhbc::IncDecOp::PostDec),
        b"PreIncO" => Ok(hhbc::IncDecOp::PreIncO),
        b"PostIncO" => Ok(hhbc::IncDecOp::PostIncO),
        b"PreDecO" => Ok(hhbc::IncDecOp::PreDecO),
        b"PostDecO" => Ok(hhbc::IncDecOp::PostDecO),

        ido => bail!("Expected a IncDecOp but got: {:?}", ido),
    }
}

fn assemble_silence_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::SilenceOp> {
    let so = match token_iter.expect(Token::into_identifier)? {
        b"Start" => hhbc::SilenceOp::Start,
        b"End" => hhbc::SilenceOp::End,
        so => bail!("Expected a SilenceOp but got: {:?}", so),
    };
    Ok(so)
}

/// Ex:
/// Silence _2 End
fn assemble_silence<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &mut HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Silence")?;
    let lcl = assemble_local(token_iter, decl_map)?;
    let sop = assemble_silence_op(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Silence(lcl, sop)))
}

fn assemble_oodecl_exists_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::OODeclExistsOp> {
    let oo = match token_iter.expect(Token::into_identifier)? {
        b"Class" => hhbc::OODeclExistsOp::Class,
        b"Interface" => hhbc::OODeclExistsOp::Interface,
        b"Trait" => hhbc::OODeclExistsOp::Trait,
        oo => bail!("Expected a OODeclExistsOp but got: {:?}", oo),
    };
    Ok(oo)
}

fn assemble_oodecl_exists<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "OODeclExists")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::OODeclExists(
        assemble_oodecl_exists_op(token_iter)?,
    )))
}

fn assemble_fatal_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::FatalOp> {
    let fop = match token_iter.expect(Token::into_identifier)? {
        b"Runtime" => hhbc::FatalOp::Runtime,
        b"Parse" => hhbc::FatalOp::Parse,
        b"RuntimeOmitFrame" => hhbc::FatalOp::RuntimeOmitFrame,
        f => bail!("Expected a FatalOp, got: {:?}", f),
    };
    Ok(fop)
}

fn assemble_fatal_opcode<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Fatal")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Fatal(
        assemble_fatal_op(token_iter)?,
    )))
}

fn assemble_bare_this_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::BareThisOp> {
    let bop = match token_iter.expect(Token::into_identifier)? {
        b"Notice" => hhbc::BareThisOp::Notice,
        b"NoNotice" => hhbc::BareThisOp::NoNotice,
        b"NeverNull" => hhbc::BareThisOp::NeverNull,
        f => bail!("Expected a FatalOp, got: {:?}", f),
    };
    Ok(bop)
}

fn assemble_bare_this_opcode<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "BareThis")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::BareThis(
        assemble_bare_this_op(token_iter)?,
    )))
}

fn assemble_collection_type(token_iter: &mut Lexer<'_>) -> Result<hhbc::CollectionType> {
    let ct = match token_iter.expect(Token::into_identifier)? {
        b"Vector" => hhbc::CollectionType::Vector,
        b"Map" => hhbc::CollectionType::Map,
        b"Set" => hhbc::CollectionType::Set,
        b"Pair" => hhbc::CollectionType::Pair,
        b"ImmVector" => hhbc::CollectionType::ImmVector,
        b"ImmMap" => hhbc::CollectionType::ImmMap,
        b"ImmSet" => hhbc::CollectionType::ImmSet,
        f => bail!("Expected a FatalOp, got: {:?}", f),
    };
    Ok(ct)
}

fn assemble_init_prop_op(token_iter: &mut Lexer<'_>) -> Result<hhbc::InitPropOp> {
    let ipo = match token_iter.expect(Token::into_identifier)? {
        b"Static" => hhbc::InitPropOp::Static,
        b"NonStatic" => hhbc::InitPropOp::NonStatic,
        ipo => bail!("Expected a InitPropOp, got: {:?}", ipo),
    };
    Ok(ipo)
}

/// Ex:
/// InitProp "nonScalarTraitProperty" NonStatic
fn assemble_init_prop<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "InitProp")?;
    let pn = assemble_prop_name_from_str(alloc, token_iter)?;
    let ipo = assemble_init_prop_op(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::InitProp(pn, ipo)))
}

/// Ex:
/// CheckProp "profilePicPrepared"
fn assemble_check_prop<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "CheckProp")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::CheckProp(
        assemble_prop_name_from_str(alloc, token_iter)?,
    )))
}

fn assemble_col_from_array<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "ColFromArray")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::ColFromArray(
        assemble_collection_type(token_iter)?,
    )))
}

fn assemble_new_col<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "NewCol")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::NewCol(
        assemble_collection_type(token_iter)?,
    )))
}

/// Ex:
/// CUGetL $v
/// CGetS Mutable
fn assemble_cget<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    let cg = match token_iter.expect(Token::into_identifier)? {
        b"CGetCUNop" => hhbc::Opcode::CGetCUNop,
        b"UGetCUNop" => hhbc::Opcode::UGetCUNop,
        b"CGetL" => hhbc::Opcode::CGetL(assemble_local(token_iter, decl_map)?),
        b"CGetQuietL" => hhbc::Opcode::CGetQuietL(assemble_local(token_iter, decl_map)?),
        b"CUGetL" => hhbc::Opcode::CUGetL(assemble_local(token_iter, decl_map)?),
        b"CGetL2" => hhbc::Opcode::CGetL2(assemble_local(token_iter, decl_map)?),
        b"CGetG" => hhbc::Opcode::CGetG,
        b"CGetS" => hhbc::Opcode::CGetS(assemble_readonly_op(token_iter)?),
        b"ClassGetC" => hhbc::Opcode::ClassGetC,
        b"ClassGetTS" => hhbc::Opcode::ClassGetTS,
        cg => bail!("Unknown cget: {:?}", cg),
    };
    Ok(hhbc::Instruct::Opcode(cg))
}

/// IncDecL $var IncDecOp
fn assemble_incdecl_opcode<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "IncDecL")?;
    let lcl = token_iter.expect(Token::into_variable)?;
    if let Some(idx) = decl_map.get(lcl) {
        let ido = assemble_inc_dec_op(token_iter)?;
        token_iter.expect_end()?;
        Ok(hhbc::Instruct::Opcode(hhbc::Opcode::IncDecL(
            hhbc::Local { idx: *idx },
            ido,
        )))
    } else {
        bail!("Unknown local var given to IncDecL instr");
    }
}

/// IncDecS IncDecOp
fn assemble_incdec_opcode<'arena, F: FnOnce(hhbc::IncDecOp) -> hhbc::Opcode<'arena>>(
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    let ido = assemble_inc_dec_op(token_iter)?;
    Ok(hhbc::Instruct::Opcode(op_con(ido)))
}
/// Ex:
/// NewStructDict <"sc" "l_o" "l_b_t">
fn assemble_new_struct_dict<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "NewStructDict")?;
    token_iter.expect(Token::into_lt)?;
    let mut d = Vec::new();
    while !token_iter.peek_if(Token::is_gt) {
        d.push(assemble_unescaped_unquoted_str(alloc, token_iter)?);
    }
    token_iter.expect(Token::into_gt)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::NewStructDict(
        Slice::from_vec(alloc, d),
    )))
}

/// L#: or DV#: if needs_colon else L# or DV#
fn assemble_label(token_iter: &mut Lexer<'_>, needs_colon: bool) -> Result<hhbc::Label> {
    let mut lcl = token_iter.expect(Token::into_identifier)?;
    if needs_colon {
        token_iter.expect(Token::into_colon)?;
    }
    let label_reg = regex!(r"^((DV|L)[0-9]+)$").clone();
    if label_reg.is_match(lcl) {
        if lcl[0] == b'D' {
            lcl = &lcl[2..];
        } else {
            lcl = &lcl[1..];
        }
        let lcl = std::str::from_utf8(lcl)?.parse::<u32>()?;
        Ok(hhbc::Label(lcl))
    } else {
        bail!("Unknown label");
    }
}

/// Ex:
/// ResolveFunc "my_func"
fn assemble_resolve_func<'arena, F: FnOnce(hhbc::FunctionName<'arena>) -> hhbc::Opcode<'arena>>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    Ok(hhbc::Instruct::Opcode(op_con(
        assemble_function_name_from_str(alloc, token_iter)?,
    )))
}

fn assemble_resolve_class<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    let op = match token_iter.expect(Token::into_identifier)? {
        b"ResolveClsMethod" => {
            hhbc::Opcode::ResolveClsMethod(assemble_method_name_from_str(alloc, token_iter)?)
        }
        b"ResolveClsMethodD" => hhbc::Opcode::ResolveClsMethodD(
            assemble_class_name_from_str(alloc, token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"ResolveClsMethodS" => hhbc::Opcode::ResolveClsMethodS(
            assemble_special_class_ref(token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"ResolveRClsMethod" => {
            hhbc::Opcode::ResolveRClsMethod(assemble_method_name_from_str(alloc, token_iter)?)
        }
        b"ResolveRClsMethodD" => hhbc::Opcode::ResolveRClsMethodD(
            assemble_class_name_from_str(alloc, token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"ResolveRClsMethodS" => hhbc::Opcode::ResolveRClsMethodS(
            assemble_special_class_ref(token_iter)?,
            assemble_method_name_from_str(alloc, token_iter)?,
        ),
        b"ResolveClass" => {
            hhbc::Opcode::ResolveClass(assemble_class_name_from_str(alloc, token_iter)?)
        }
        b => bail!("Unknown resolve class opcode: {:?}", b),
    };
    Ok(hhbc::Instruct::Opcode(op))
}

/// Ex:
/// CreateCl 0 1
fn assemble_create_cl<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "CreateCl")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::CreateCl(
        token_iter.expect_and_get_number()?,
        token_iter.expect_and_get_number()?,
    )))
}

/// Assembles one of Enter/Jmp/JmpZ/JmpNZ
fn assemble_jump_opcode_instr<'arena, F: FnOnce(hhbc::Label) -> hhbc::Opcode<'arena>>(
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    let lbl = assemble_label(token_iter, false)
        .map_err(|_| anyhow!("Unknown label given to {} instr", op_str))?;
    Ok(hhbc::Instruct::Opcode(op_con(lbl)))
}

/// Returns the local (u32 idx) a var or unnamed corresponds to.
/// This information is based on the position of the var in parameters of a function/.declvars
/// or, if an unnamed, just the idx referenced (_1 -> idx 1)
/// $a -> idx where $a is stored in hcu body
/// _3 -> 3
fn assemble_local(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
) -> Result<hhbc::Local> {
    match token_iter.next() {
        Some(Token::Variable(v, p)) => {
            if let Some(idx) = decl_map.get(v) {
                Ok(hhbc::Local { idx: *idx })
            } else {
                bail!("Unknown local var: {:?} at {:?}", v, p);
            }
        }
        Some(Token::Identifier(i, _)) => {
            debug_assert!(i[0] == b'_');
            Ok(hhbc::Local {
                idx: std::str::from_utf8(&i[1..i.len()])?.parse()?,
            })
        }
        None => bail!("Expected local, found None"),
        b => bail!("Unknown local: {}", b.unwrap()),
    }
}

/// Ex: RetM 2
fn assemble_retm_opcode_instr<'arena>(
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "RetM")?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::RetM(
        token_iter.expect_and_get_number()?,
    )))
}

/// Assembles one of CGetL/Push/SetL/etc...
fn assemble_local_carrying_opcode_instr<'arena, F: FnOnce(hhbc::Local) -> hhbc::Opcode<'arena>>(
    token_iter: &mut Lexer<'_>,
    decl_map: &HashMap<Vec<u8>, u32>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    let lcl = assemble_local(token_iter, decl_map)?;
    Ok(hhbc::Instruct::Opcode(op_con(lcl)))
}

fn assemble_single_opcode_instr<'arena, F: FnOnce() -> hhbc::Opcode<'arena>>(
    token_iter: &mut Lexer<'_>,
    op_con: F, // Because of the trait bound, guaranteed only trying to build opcodes that take no arguments
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    token_iter.expect_end()?;
    Ok(hhbc::Instruct::Opcode(op_con()))
}

/// Assembles one of Dict/Keyset/Vec opcodes. Note that
/// when printing, the printer adds a `@` at the front of the AdataId; it's not part of the name
fn assemble_adata_id_carrying_instr<
    'arena,
    F: FnOnce(hhbc::AdataId<'arena>) -> hhbc::Opcode<'arena>,
>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    let adata_id = token_iter.expect(Token::into_global)?;
    debug_assert!(adata_id[0] == b'@');
    let adata_id: hhbc::AdataId<'arena> = Str::new_slice(alloc, &adata_id[1..]);
    Ok(hhbc::Instruct::Opcode(op_con(adata_id)))
}

fn assemble_string_opcode<'arena>(
    alloc: &'arena Bump,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "String")?;
    let st_data = token_iter.expect(Token::into_str_literal)?;
    let st_data =
        escaper::unescape_literal_bytes_into_vec_bytes(escaper::unquote_slice(st_data.as_bytes()))?;
    token_iter.expect_end()?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::String(
        Str::new_slice(alloc, &st_data),
    )))
}

fn assemble_int_opcode<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Int")?;
    let num: i64 = token_iter.expect_and_get_number()?;
    token_iter.expect_end()?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Int(num)))
}

fn assemble_u32_carrying_opcode<'arena, F: FnOnce(u32) -> hhbc::Opcode<'arena>>(
    token_iter: &mut Lexer<'_>,
    op_con: F,
    op_str: &str,
) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, op_str)?;
    Ok(hhbc::Instruct::Opcode(op_con(
        token_iter.expect_and_get_number()?,
    )))
}

fn assemble_double_opcode<'arena>(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct<'arena>> {
    token_iter.expect_is_str(Token::into_identifier, "Double")?;
    let num: f64 = token_iter.expect_and_get_number()?;
    token_iter.expect_end()?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::Double(
        hhbc::FloatBits(num),
    )))
}

type Line = usize;

#[derive(Debug, PartialEq, Eq, Copy, Clone)]
pub enum Token<'a> {
    // See below in Lexer::from_slice for regex definitions
    Global(&'a [u8], Line),
    Variable(&'a [u8], Line),
    TripleStrLiteral(&'a [u8], Line),
    Decl(&'a [u8], Line),
    StrLiteral(&'a [u8], Line),
    Variadic(Line),
    Semicolon(Line),
    Dash(Line),
    OpenCurly(Line),
    OpenBracket(Line),
    OpenParen(Line),
    CloseParen(Line),
    CloseBracket(Line),
    CloseCurly(Line),
    Equal(Line),
    Number(&'a [u8], Line),
    Comma(Line),
    Lt(Line),
    Gt(Line),
    Colon(Line),
    Identifier(&'a [u8], Line),
    Newline(Line),
    Error(&'a [u8], Line),
}

impl<'a> Token<'a> {
    fn as_bytes(&self) -> &'a [u8] {
        match self {
            Token::Global(u, _)
            | Token::Variable(u, _)
            | Token::TripleStrLiteral(u, _)
            | Token::Decl(u, _)
            | Token::StrLiteral(u, _)
            | Token::Number(u, _)
            | Token::Identifier(u, _)
            | Token::Error(u, _) => u,
            Token::Semicolon(_) => b";",
            Token::Dash(_) => b"-",
            Token::OpenCurly(_) => b"{",
            Token::OpenBracket(_) => b"[",
            Token::OpenParen(_) => b"(",
            Token::CloseParen(_) => b")",
            Token::CloseBracket(_) => b"]",
            Token::CloseCurly(_) => b"}",
            Token::Equal(_) => b"=",
            Token::Comma(_) => b",",
            Token::Lt(_) => b"<",
            Token::Gt(_) => b">",
            Token::Colon(_) => b":",
            Token::Variadic(_) => b"...",
            Token::Newline(_) => b"\n",
        }
    }

    /// Only str_literal and triple_str_literal can be parsed into a new tokenizer.
    /// To create a new tokenizer that still has accurate error reporting, we want to pass the line
    /// So `into_str_literal_and_line` and `into_triple_str_literal_and_line` return a Result of bytes rep and line # or bail
    fn into_triple_str_literal_and_line(self) -> Result<(&'a [u8], usize)> {
        match self {
            Token::TripleStrLiteral(vec_u8, pos) => Ok((vec_u8, pos)),

            _ => bail!("Expected a triple str literal, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_str_literal_and_line(self) -> Result<(&'a [u8], usize)> {
        match self {
            Token::StrLiteral(vec_u8, pos) => Ok((vec_u8, pos)),
            _ => bail!("Expected a str literal, got: {}", self),
        }
    }

    /// The "into" series of methods both check that [self] is the correct
    /// variant of Token and return a Result of [self]'s string rep
    #[allow(dead_code)]
    fn into_newline(self) -> Result<&'a [u8]> {
        match self {
            Token::Newline(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a newline, got: {}", self),
        }
    }
    #[allow(dead_code)]
    fn into_global(self) -> Result<&'a [u8]> {
        match self {
            Token::Global(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a global, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_variable(self) -> Result<&'a [u8]> {
        match self {
            Token::Variable(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a variable, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_triple_str_literal(self) -> Result<&'a [u8]> {
        match self {
            Token::TripleStrLiteral(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a triple str literal, got: {}", self),
        }
    }

    fn into_decl(self) -> Result<&'a [u8]> {
        match self {
            Token::Decl(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a decl, got: {}", self),
        }
    }

    fn into_str_literal(self) -> Result<&'a [u8]> {
        match self {
            Token::StrLiteral(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a str literal, got: {}", self),
        }
    }

    fn into_number(self) -> Result<&'a [u8]> {
        match self {
            Token::Number(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a number, got: {}", self),
        }
    }

    fn into_identifier(self) -> Result<&'a [u8]> {
        match self {
            Token::Identifier(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected an identifier, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_error(self) -> Result<&'a [u8]> {
        match self {
            Token::Error(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected an error, got: {}", self),
        }
    }

    fn into_semicolon(self) -> Result<&'a [u8]> {
        match self {
            Token::Semicolon(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a semicolon, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_dash(self) -> Result<&'a [u8]> {
        match self {
            Token::Dash(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a dash, got: {}", self),
        }
    }

    fn into_open_curly(self) -> Result<&'a [u8]> {
        match self {
            Token::OpenCurly(_) => Ok(self.as_bytes()),
            _ => bail!("Expected an open curly, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_open_bracket(self) -> Result<&'a [u8]> {
        match self {
            Token::OpenBracket(_) => Ok(self.as_bytes()),
            _ => bail!("Expected an open bracket, got: {}", self),
        }
    }

    fn into_open_paren(self) -> Result<&'a [u8]> {
        match self {
            Token::OpenParen(_) => Ok(self.as_bytes()),
            _ => bail!("Expected an open paren, got: {}", self),
        }
    }

    fn into_close_paren(self) -> Result<&'a [u8]> {
        match self {
            Token::CloseParen(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a close paren, got: {}", self),
        }
    }

    fn into_close_bracket(self) -> Result<&'a [u8]> {
        match self {
            Token::CloseBracket(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a close bracket, got: {}", self),
        }
    }

    fn into_close_curly(self) -> Result<&'a [u8]> {
        match self {
            Token::CloseCurly(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a close curly, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_equal(self) -> Result<&'a [u8]> {
        match self {
            Token::Equal(_) => Ok(self.as_bytes()),
            _ => bail!("Expected an equal, got: {}", self),
        }
    }

    fn into_comma(self) -> Result<&'a [u8]> {
        match self {
            Token::Comma(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a comma, got: {}", self),
        }
    }

    fn into_lt(self) -> Result<&'a [u8]> {
        match self {
            Token::Lt(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a lt (<), got: {}", self),
        }
    }

    fn into_gt(self) -> Result<&'a [u8]> {
        match self {
            Token::Gt(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a gt (>), got: {}", self),
        }
    }

    fn into_colon(self) -> Result<&'a [u8]> {
        match self {
            Token::Colon(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a colon, got: {}", self),
        }
    }

    #[allow(dead_code)]
    fn into_variadic(self) -> Result<&'a [u8]> {
        match self {
            Token::Variadic(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a variadic, got: {}", self),
        }
    }

    //is
    #[allow(dead_code)]
    fn is_newline(&self) -> bool {
        matches!(self, Token::Newline(..))
    }
    #[allow(dead_code)]
    fn is_global(&self) -> bool {
        matches!(self, Token::Global(..))
    }

    #[allow(dead_code)]
    fn is_variable(&self) -> bool {
        matches!(self, Token::Variable(..))
    }

    #[allow(dead_code)]
    fn is_triple_str_literal(&self) -> bool {
        matches!(self, Token::TripleStrLiteral(..))
    }

    #[allow(dead_code)]
    fn is_decl(&self) -> bool {
        matches!(self, Token::Decl(..))
    }

    #[allow(dead_code)]
    fn is_str_literal(&self) -> bool {
        matches!(self, Token::StrLiteral(..))
    }

    #[allow(dead_code)]
    fn is_number(&self) -> bool {
        matches!(self, Token::Number(..))
    }

    #[allow(dead_code)]
    fn is_identifier(&self) -> bool {
        matches!(self, Token::Identifier(..))
    }

    #[allow(dead_code)]
    fn is_error(&self) -> bool {
        matches!(self, Token::Error(..))
    }

    #[allow(dead_code)]
    fn is_semicolon(&self) -> bool {
        matches!(self, Token::Semicolon(_))
    }

    #[allow(dead_code)]
    fn is_dash(&self) -> bool {
        matches!(self, Token::Dash(_))
    }

    #[allow(dead_code)]
    fn is_open_curly(&self) -> bool {
        matches!(self, Token::OpenCurly(_))
    }

    #[allow(dead_code)]
    fn is_open_bracket(&self) -> bool {
        matches!(self, Token::OpenBracket(_))
    }

    #[allow(dead_code)]
    fn is_open_paren(&self) -> bool {
        matches!(self, Token::OpenParen(_))
    }

    #[allow(dead_code)]
    fn is_close_paren(&self) -> bool {
        matches!(self, Token::CloseParen(_))
    }

    #[allow(dead_code)]
    fn is_close_bracket(&self) -> bool {
        matches!(self, Token::CloseBracket(_))
    }

    #[allow(dead_code)]
    fn is_close_curly(&self) -> bool {
        matches!(self, Token::CloseCurly(_))
    }

    #[allow(dead_code)]
    fn is_equal(&self) -> bool {
        matches!(self, Token::Equal(_))
    }

    #[allow(dead_code)]
    fn is_comma(&self) -> bool {
        matches!(self, Token::Comma(_))
    }

    #[allow(dead_code)]
    fn is_lt(&self) -> bool {
        matches!(self, Token::Lt(_))
    }

    #[allow(dead_code)]
    fn is_gt(&self) -> bool {
        matches!(self, Token::Gt(_))
    }

    #[allow(dead_code)]
    fn is_colon(&self) -> bool {
        matches!(self, Token::Colon(_))
    }

    #[allow(dead_code)]
    fn is_variadic(&self) -> bool {
        matches!(self, Token::Variadic(_))
    }
}

impl<'a> fmt::Display for Token<'a> {
    /// Purpose of this fmt: so that vec of u8 (internal str representation of each token) is printed as a string rather than bytes
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let text = std::str::from_utf8(self.as_bytes()).map_err(|_| fmt::Error)?;
        match self {
            Token::Global(_, pos) => write!(f, "Global(val: \"{text}\", line: {pos:?})"),
            Token::Variable(_, pos) => write!(f, "Variable(val: \"{text}\", line: {pos:?})"),
            Token::TripleStrLiteral(_, pos) => {
                write!(f, "TripleStrLiteral(val: \"{text}\", line: {pos:?})")
            }
            Token::Decl(_, pos) => write!(f, "Decl(val: \"{text}\", line: {pos:?})"),
            Token::StrLiteral(_, pos) => write!(f, "StrLiteral(val: \"{text}\", line: {pos:?})"),
            Token::Number(_, pos) => write!(f, "Number(val: \"{text}\", line: {pos:?})"),
            Token::Identifier(_, pos) => write!(f, "Identifier(val: \"{text}\", line: {pos:?})"),
            Token::Error(_, pos) => write!(f, "Error(val: \"{text}\", line: {pos:?})"),
            Token::Semicolon(pos) => write!(f, "Semicolon(val: \"{text}\", line: {pos:?})"),
            Token::Dash(pos) => write!(f, "Dash(val: \"{text}\", line: {pos:?})"),
            Token::OpenCurly(pos) => write!(f, "OpenCurly(val: \"{text}\", line: {pos:?})"),
            Token::OpenBracket(pos) => write!(f, "OpenBracket(val: \"{text}\", line: {pos:?})"),
            Token::OpenParen(pos) => write!(f, "OpenParen(val: \"{text}\", line: {pos:?})"),
            Token::CloseParen(pos) => write!(f, "CloseParen(val: \"{text}\", line: {pos:?})"),
            Token::CloseBracket(pos) => write!(f, "CloseBracket(val: \"{text}\", line: {pos:?})"),
            Token::CloseCurly(pos) => write!(f, "CloseCurly(val: \"{text}\", line: {pos:?})"),
            Token::Equal(pos) => write!(f, "Equal(val: \"{text}\", line: {pos:?})"),
            Token::Comma(pos) => write!(f, "Comma(val: \"{text}\", line: {pos:?})"),
            Token::Lt(pos) => write!(f, "Lt(val: \"{text}\", line: {pos:?})"),
            Token::Gt(pos) => write!(f, "Gt(val: \"{text}\", line: {pos:?})"),
            Token::Colon(pos) => write!(f, "Colon(val: \"{text}\", line: {pos:?})"),
            Token::Variadic(pos) => write!(f, "Variadic(val: \"{text}\", line: {pos:?})"),
            Token::Newline(pos) => write!(f, "Newline(val: \"{text}\", line: {pos:?})"),
        }
    }
}

// We initially planned on using Logos, a crate for tokenizing really fast.
// We chose not to use Logos because it doesn't support all regexes -- for instance, it can't
// tokenize based on the regex "\"\"\".*\"\"\"". Here's the git issue:
// https://github.com/maciejhirsz/logos/issues/246
pub struct Lexer<'a> {
    pending: Option<Token<'a>>, // Cached next (non-newline)
    tokens: VecDeque<Token<'a>>,
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token<'a>;

    /// Returns the next token, never returning a newline and instead advancing the lexer past them.
    fn next(&mut self) -> Option<Self::Item> {
        self.fill(); // If `pending` already has a token this does nothing
        self.pending.take()
    }
}

impl<'a> Lexer<'a> {
    /// Counts tokens in this `Lexer`, including newlines. Because of this it only makes sense to call
    /// `size` on `Lexer`s spawned from `fetch_until_newline`
    fn _size(&self) -> usize {
        if self.pending.is_some() {
            1 + self.tokens.len()
        } else {
            self.tokens.len()
        }
    }

    /// If `pending` is none, fills with the next non-newline token, or None of one does not exist.
    fn fill(&mut self) {
        if self.pending.is_none() {
            self.pending = self.get_next_no_newlines();
        }
    }

    /// Returns the next non-newline token (or None), advancing the lexer past any leading newlines.
    fn get_next_no_newlines(&mut self) -> Option<Token<'a>> {
        while let Some(t) = self.tokens.pop_front() {
            if !t.is_newline() {
                return Some(t);
            }
        }
        None
    }

    pub fn is_empty(&mut self) -> bool {
        self.fill();
        if self.pending.is_none() {
            debug_assert!(self.tokens.is_empty()); // VD should never have \n \n \n and empty pending
            true
        } else {
            false
        }
    }

    /// Advances the lexer passed its first non-leading newline, returning a mini-lexer of the tokens up until that newline.
    pub fn fetch_until_newline(&mut self) -> Option<Lexer<'a>> {
        self.fill();
        if let Some(t) = self.pending {
            let mut first_toks = VecDeque::new();
            // Add what was in `pending`
            first_toks.push_back(t);
            // While we haven't reached the new line
            while let Some(to_push) = self.tokens.pop_front() {
                if to_push.is_newline() {
                    break;
                }
                // Add token
                first_toks.push_back(to_push);
            }
            self.pending = None;
            Some(Lexer {
                tokens: first_toks,
                pending: None,
            })
        } else {
            None
        }
    }

    /// Similarly to `Lexer::next`, will not return a peek to a newline. Instead
    /// this `peek` returns a view to the first token that's not a newline, and modifies the lexer in that it strips
    /// it of leading newlines.
    pub fn peek(&mut self) -> Option<&Token<'a>> {
        self.fill();
        self.pending.as_ref()
    }

    /// Returns f applied to the top of the iterator, but does not advance iterator (unlike next_if)
    fn peek_if<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek().map_or(false, f)
    }

    /// Applies f to top of iterator, and if true advances iterator and returns true else doesn't advance and returns false
    fn next_if<F>(&mut self, f: F) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_if(f);
        if tr {
            self.next();
        }
        tr
    }

    /// Applies f to top of iterator. If true, compares inner rep to passed &str and returns result. Else just false
    fn peek_if_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek_if(|t| f(t) && t.as_bytes() == s.as_bytes())
    }

    /// Applies f to top of iterator. If true, checks if starts with passed &str and returns result. Else just false
    /// Ex use: peek_if_str_starts(Token::is_decl, ".coeffects")
    fn peek_if_str_starts<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        self.peek_if(|t| f(t) && t.as_bytes().starts_with(s.as_bytes()))
    }

    /// Applies f to top of iterator. If true, compares inner representation to passed &str and if true consumes. Else doesn't modify iterator and returns false.
    fn next_if_str<F>(&mut self, f: F, s: &str) -> bool
    where
        F: Fn(&Token<'a>) -> bool,
    {
        let tr = self.peek_if_str(f, s);
        if tr {
            self.next();
        }
        tr
    }

    /// Applies f to the top of token_iter. In the most likely use, f bails if token_iter is not the
    /// expected token (expected token specified by f)
    fn expect<T, F>(&mut self, f: F) -> Result<T>
    where
        F: FnOnce(Token<'a>) -> Result<T>,
    {
        self.next()
            .map_or_else(|| bail!("End of token stream sooner than expected"), f)
    }

    /// Like `expect` in that bails if incorrect token passed, and does return the `into` of the passed token
    fn expect_is_str<F>(&mut self, f: F, s: &str) -> Result<&[u8]>
    where
        F: FnOnce(Token<'a>) -> Result<&[u8]>,
    {
        self.next().map_or_else(
            || bail!("End of token stream sooner than expected"),
            |t| {
                let tr = f(t)?;
                if tr != s.as_bytes() {
                    bail!("Expected {} got {}", s, t)
                } else {
                    Ok(tr)
                }
            },
        )
    }

    /// Similar to `expect_and_get_number` but puts identifier into a Str<'arena>
    fn expect_identifier_into_ffi_str<'arena>(
        &mut self,
        alloc: &'arena Bump,
    ) -> Result<Str<'arena>> {
        let st = self.expect(Token::into_identifier)?;
        Ok(Str::new_slice(alloc, st))
    }

    /// Similar to `expect_and_get_number` but puts decl into a Str<'arena>
    fn expect_decl_into_ffi_str<'arena>(&mut self, alloc: &'arena Bump) -> Result<Str<'arena>> {
        let st = self.expect(Token::into_decl)?;
        Ok(Str::new_slice(alloc, st))
    }

    /// Similar to `expect` but instead of returning a Result that usually contains a slice of u8,
    /// applies f to the `from_utf8 str` of the top token, bailing if the top token is not a number.
    fn expect_and_get_number<T: FromStr>(&mut self) -> Result<T> {
        let num = if self.peek_if(Token::is_dash) {
            self.expect(Token::into_dash)? // -INF
        } else if self.peek_if(Token::is_identifier) {
            self.expect(Token::into_identifier)? // NAN, INF, etc will parse as float constants
        } else {
            self.expect(Token::into_number)?
        };
        // If num is a dash we expect an identifier to follow
        if num == b"-" {
            let mut num = num.to_vec();
            num.extend_from_slice(self.expect(Token::into_identifier)?);
            FromStr::from_str(std::str::from_utf8(&num)?).map_err(|_| {
                anyhow!("Number-looking token in tokenizer that cannot be parsed into number")
            })
        } else {
            FromStr::from_str(std::str::from_utf8(num)?).map_err(|_| {
                anyhow!("Number-looking token in tokenizer that cannot be parsed into number")
            }) // This std::str::from_utf8 will never bail; if it should bail, the above `expect` bails first.
        }
    }

    /// A var can be written in HHAS as $abc or "$abc". Only valid if a $ preceeds
    fn expect_var(&mut self) -> Result<Vec<u8>> {
        if self.peek_if(Token::is_str_literal) {
            let s = self.expect(Token::into_str_literal)?;
            if s.starts_with(b"\"$") {
                // Remove the "" b/c that's not part of the var name
                // also unescape ("$\340\260" etc is literal bytes)
                let s = escaper::unquote_slice(s);
                let s = escaper::unescape_literal_bytes_into_vec_bytes(s)?;
                Ok(s)
            } else {
                bail!("Var does not start with $: {:?}", s)
            }
        } else {
            Ok(self.expect(Token::into_variable)?.to_vec())
        }
    }

    /// Bails if lexer is not empty, message contains the next token
    fn expect_end(&mut self) -> Result<()> {
        ensure!(
            self.is_empty(),
            "Expected end of token stream, see: {}",
            self.next().unwrap()
        );
        Ok(())
    }

    fn make_regex() -> Regex {
        // First create the regex that matches any token. Done this way for
        // readability
        let v = [
            r#""""([^"\\]|\\.)*?""""#, // Triple str literal
            "#.*",                     // Comment
            r"(?-u)[\.@][_a-z/A-Z\x80-\xff][_/a-zA-Z/0-9\x80-\xff\-\.]*", // Decl, global. (?-u) turns off utf8 check
            r"(?-u)\$[_a-zA-Z0-9$\x80-\xff][_/a-zA-Z0-9$\x80-\xff]*",     // Var.
            r#""((\\.)|[^\\"])*""#,                                       // Str literal
            r"[-+]?[0-9]+\.?[0-9]*([eE][-+]?[0-9]+\.?[0-9]*)?",           // Number
            r"(?-u)[_/a-zA-Z\x80-\xff]([_/\\a-zA-Z0-9\x80-\xff\.\$#\-]|::)*", // Identifier
            ";",
            "-",
            "=",
            r"\{",
            r"\[",
            r"\(",
            r"\)",
            r"\]",
            r"\}",
            ",",
            "<",
            ">",
            ":",
            r"\.\.\.", // Variadic
            "\n",
            r"[ \t\r\f]+",
        ];
        let big_regex = format!("^(({}))", v.join(")|("));
        Regex::new(&big_regex).unwrap()
    }

    fn from_slice(s: &'a [u8], start_line: usize) -> Self {
        // According to
        // https://github.com/rust-lang/regex/blob/master/PERFORMANCE.md (and
        // directly observed) Regex keeps mutable internal state which needs to
        // be synchronized - so using it from multiple threads will be a big
        // performance regression.
        static REGEX: OnceCell<Regex> = OnceCell::new();
        let big_regex = REGEX.get_or_init(Self::make_regex).clone();

        let mut cur_line = start_line;
        let mut tokens = VecDeque::new();
        let mut source = s;
        while !source.is_empty() {
            source = build_tokens_helper(source, &mut cur_line, &mut tokens, &big_regex);
        }
        Lexer {
            pending: None,
            tokens,
        }
    }
}

fn build_tokens_helper<'a>(
    s: &'a [u8],
    cur_line: &mut usize,
    tokens: &mut VecDeque<Token<'a>>,
    big_regex: &Regex,
) -> &'a [u8] {
    if let Some(mat) = big_regex.find(s) {
        let mut chars = s.iter(); // Implicit assumption: matched to the start (^), so we iter from the start
        debug_assert!(mat.start() == 0);
        match chars.next().unwrap() {
            // Get first character
            // Note these don't match what prints out on a printer, but not sure how to generalize
            b'\x0C' => {
                // form feed

                &s[mat.end()..]
            }
            b'\r' => &s[mat.end()..],
            b'\t' => &s[mat.end()..],
            b'#' => {
                &s[mat.end()..] // Don't advance the line; the newline at the end of the comment will advance the line
            }
            b' ' => &s[mat.end()..], // Don't add whitespace as tokens, just increase line and col
            o => {
                let end = mat.end();
                let tok = match o {
                    b'\n' => {
                        let old_pos = *cur_line;
                        *cur_line += 1;

                        Token::Newline(old_pos)
                    }
                    b'@' => Token::Global(&s[..end], *cur_line), // Global
                    b'$' => Token::Variable(&s[..end], *cur_line), // Var
                    b'.' => {
                        if *(chars.next().unwrap()) == b'.' && *(chars.next().unwrap()) == b'.' {
                            // Variadic
                            Token::Variadic(*cur_line)
                        } else {
                            Token::Decl(&s[..end], *cur_line) // Decl
                        }
                    }
                    b';' => Token::Semicolon(*cur_line), // Semicolon
                    b'{' => Token::OpenCurly(*cur_line), // Opencurly
                    b'[' => Token::OpenBracket(*cur_line),
                    b'(' => Token::OpenParen(*cur_line),
                    b')' => Token::CloseParen(*cur_line),
                    b']' => Token::CloseBracket(*cur_line),
                    b'}' => Token::CloseCurly(*cur_line),
                    b',' => Token::Comma(*cur_line),
                    b'<' => Token::Lt(*cur_line),    //<
                    b'>' => Token::Gt(*cur_line),    //>
                    b'=' => Token::Equal(*cur_line), //=
                    b'-' => {
                        if chars.next().unwrap().is_ascii_digit() {
                            // Negative number
                            Token::Number(&s[..end], *cur_line)
                        } else {
                            Token::Dash(*cur_line)
                        }
                    }
                    b':' => Token::Colon(*cur_line),
                    b'"' => {
                        if *(chars.next().unwrap()) == b'"' && *(chars.next().unwrap()) == b'"' {
                            // Triple string literal
                            Token::TripleStrLiteral(&s[..end], *cur_line)
                        } else {
                            // Single string literal
                            Token::StrLiteral(&s[..end], *cur_line)
                        }
                    }
                    dig_or_id => {
                        if dig_or_id.is_ascii_digit()
                            || (*dig_or_id as char == '+'
                                && (chars.next().unwrap()).is_ascii_digit())
                        // Positive numbers denoted with +
                        {
                            Token::Number(&s[..end], *cur_line)
                        } else {
                            Token::Identifier(&s[..end], *cur_line)
                        }
                    }
                };
                tokens.push_back(tok);
                &s[end..]
            }
        }
    } else {
        // Couldn't tokenize the string, so add the rest of it as an error
        tokens.push_back(Token::Error(s, *cur_line));
        // Done advancing col and line cuz at end
        &[]
    }
}

/// Tokenizes input string using a Lexer. Prints all tokens in the Lexer
fn print_tokens<'a>(s: &'a [u8]) {
    let lex: Lexer<'a> = Lexer::from_slice(s, 1);
    for tok in lex {
        println!("{}", tok);
    }
}

fn _print_from_lexer<'a>(l: Lexer<'a>) {
    for tok in l {
        println!("{}", tok);
    }
}

#[cfg(test)]
mod test {
    use super::*;
    #[test]
    fn str_into_test() -> Result<()> {
        // Want to test that only tokens surrounded by "" are str_literals
        // Want to confirm the assumption that after any token_iter.expect(Token::into_str_literal) call, you can safely remove the first and last element in slice
        let s = r#"abc "abc" """abc""""#;
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, 1);
        assert!(lex.next_if(Token::is_identifier));
        let sl = lex.expect(Token::into_str_literal)?;
        assert!(sl[0] == b'"' && sl[sl.len() - 1] == b'"');
        let tsl = lex.expect(Token::into_triple_str_literal)?;
        assert!(
            tsl[0..3] == [b'"', b'"', b'"'] && tsl[tsl.len() - 3..tsl.len()] == [b'"', b'"', b'"']
        );
        Ok(())
    }
    #[test]
    fn just_nl_is_empty() {
        let s = "\n \n \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, 1);
        assert!(lex.fetch_until_newline().is_none());
        assert!(lex.is_empty());
    }
    #[test]
    fn splits_mult_newlines_go_away() {
        // Point of this test: want to make sure that 3 mini-lexers are spawned (multiple new lines don't do anything)
        let s = "\n \n a \n \n \n b \n \n c \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, 1);
        let vc_of_lexers = vec![
            lex.fetch_until_newline(),
            lex.fetch_until_newline(),
            lex.fetch_until_newline(),
        ];
        assert!(lex.is_empty());
        assert!(lex.next().is_none());
        assert_eq!(vc_of_lexers.len(), 3);
    }
    #[test]
    fn no_trailing_newlines() {
        let s = "a \n \n \n";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, 1);
        assert!(lex.next().is_some());
        assert!(lex.is_empty());
    }
    #[test]
    fn splitting_multiple_lines() {
        let s = ".try { \n .srloc 3:7, 3:22 \n String \"I'm in the try\n\" \n Print \n PopC \n } .catch { \n Dup \n L1: \n Throw \n }";
        let s = s.as_bytes();
        let mut lex = Lexer::from_slice(s, 1);
        let mut vc_of_lexers = Vec::new();
        while !lex.is_empty() {
            vc_of_lexers.push(lex.fetch_until_newline());
        }
        assert_eq!(vc_of_lexers.len(), 10)
    }
    #[test]
    fn peek_next_on_newlines() {
        let s = "\n\na\n\n";
        let mut lex = Lexer::from_slice(s.as_bytes(), 1);
        assert!(lex.peek().is_some());
        assert!(lex.next().is_some());
        assert!(lex.fetch_until_newline().is_none()); // Have consumed the a here -- "\n\n" was left and that's been consumed.
    }
    #[test]
    #[should_panic]
    fn no_top_level_shouldnt_parse() {
        // Is there a better way, maybe to verify the string in the bail?
        let s = ".srloc 3:7,3:22";
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble_from_bytes(&alloc, s, false), Ok(_)))
    }
    #[test]
    #[should_panic]
    fn no_fpath_semicolon_shouldnt_parse() {
        let s = r#".filepath "aaaa""#;
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble_from_bytes(&alloc, s, false), Ok(_)))
    }
    #[test]
    #[should_panic]
    fn fpath_wo_file_shouldnt_parse() {
        let s = r#".filepath aaa"#;
        let s = s.as_bytes();
        let alloc = Bump::default();
        assert!(matches!(assemble_from_bytes(&alloc, s, false), Ok(_)))
    }
    #[test]
    fn difficult_strings() {
        let s = r#""\"0\""
        "12345\\:2\\"
        "class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class";
        "#;
        let s = s.as_bytes();
        let l: Lexer<'_> = Lexer::from_slice(s, 1);
        let mut l = l.into_iter();
        // Expecting 3 string tokens
        let _st1 = l.next().unwrap();
        let _by1 = str::as_bytes(r#""\"0\"""#);
        assert!(matches!(_st1, Token::StrLiteral(_by1, _)));
        let _st2 = l.next().unwrap();
        let _by2 = str::as_bytes(r#""12345\\:2\\""#);
        assert!(matches!(_st1, Token::StrLiteral(_by2, _)));
        let _st3 = l.next().unwrap();
        let _by3 = str::as_bytes(
            r#""class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class""#,
        );
        assert!(matches!(_st1, Token::StrLiteral(_by3, _)));
    }
    #[test]
    fn odd_unicode_test() {
        let s: &[u8] = b".\xA9\xEF\xB8\x8E $0\xC5\xA3\xB1\xC3 \xE2\x98\xBA\xE2\x98\xBA\xE2\x98\xBA @\xE2\x99\xA1\xE2\x99\xA4$";
        let l: Lexer<'_> = Lexer::from_slice(s, 1);
        // We are expecting an decl, a var, an identifier a global, and an error on the last empty variable
        let mut l = l.into_iter();
        let decl = l.next().unwrap();
        assert!(matches!(decl, Token::Decl(..)));
        let var = l.next().unwrap();
        assert!(matches!(var, Token::Variable(..)));
        let iden = l.next().unwrap();
        assert!(matches!(iden, Token::Identifier(..)));
        let glob = l.next().unwrap();
        assert!(matches!(glob, Token::Global(..)));
        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)))
    }

    #[test]
    fn every_token_test() {
        let s = r#"@_global $0Var """tripleStrLiteral:)""" #hashtagComment
        .Decl "str!Literal" ...
        ;-{[( )]} =98 -98 +101. 43.2 , < > : _/identifier/ /filepath ."#;
        // Expect glob var tsl decl strlit semicolon dash open_curly open_brack open_paren close_paren close_bracket
        // close_curly equal number number number number , < > : identifier identifier ERROR on the last .
        let s = s.as_bytes();
        let l: Lexer<'_> = Lexer::from_slice(s, 1);
        let mut l = l.into_iter();
        let glob = l.next().unwrap();
        assert!(
            matches!(glob, Token::Global(..)),
            "failed to match {}",
            glob
        );
        let var = l.next().unwrap();
        assert!(
            matches!(var, Token::Variable(..)),
            "failed to match {}",
            var
        );
        let tsl = l.next().unwrap();
        assert!(
            matches!(tsl, Token::TripleStrLiteral(..)),
            "failed to match {}",
            tsl
        );
        let decl = l.next().unwrap();
        assert!(matches!(decl, Token::Decl(..)), "failed to match {}", decl);
        let strlit = l.next().unwrap();
        assert!(
            matches!(strlit, Token::StrLiteral(..)),
            "failed to match {}",
            strlit
        );
        let variadic = l.next().unwrap();
        assert!(
            matches!(variadic, Token::Variadic(..)),
            "failed to match {}",
            variadic
        );
        let semicolon = l.next().unwrap();
        assert!(
            matches!(semicolon, Token::Semicolon(..)),
            "failed to match {}",
            semicolon
        );
        let dash = l.next().unwrap();
        assert!(matches!(dash, Token::Dash(..)), "failed to match {}", dash);
        let oc = l.next().unwrap();
        assert!(matches!(oc, Token::OpenCurly(..)), "failed to match {}", oc);
        let ob = l.next().unwrap();
        assert!(
            matches!(ob, Token::OpenBracket(..)),
            "failed to match {}",
            ob
        );
        let op = l.next().unwrap();
        assert!(matches!(op, Token::OpenParen(..)), "failed to match {}", op);
        let cp = l.next().unwrap();
        assert!(
            matches!(cp, Token::CloseParen(..)),
            "failed to match {}",
            cp
        );
        let cb = l.next().unwrap();
        assert!(
            matches!(cb, Token::CloseBracket(..)),
            "failed to match {}",
            cb
        );
        let cc = l.next().unwrap();
        assert!(
            matches!(cc, Token::CloseCurly(..)),
            "failed to match {}",
            cc
        );
        let eq = l.next().unwrap();
        assert!(matches!(eq, Token::Equal(..)), "failed to match {}", eq);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let num = l.next().unwrap();
        assert!(matches!(num, Token::Number(..)), "failed to match {}", num);
        let comma = l.next().unwrap();
        assert!(
            matches!(comma, Token::Comma(..)),
            "failed to match {}",
            comma
        );
        let lt = l.next().unwrap();
        assert!(matches!(lt, Token::Lt(..)), "failed to match {}", lt);
        let gt = l.next().unwrap();
        assert!(matches!(gt, Token::Gt(..)), "failed to match {}", gt);
        let colon = l.next().unwrap();
        assert!(
            matches!(colon, Token::Colon(..)),
            "failed to match {}",
            colon
        );
        let iden = l.next().unwrap();
        assert!(
            matches!(iden, Token::Identifier(..)),
            "failed to match {}",
            iden
        );
        let iden = l.next().unwrap();
        assert!(
            matches!(iden, Token::Identifier(..)),
            "failed to match {}",
            iden
        );
        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)), "failed to match {}", err);
    }
}

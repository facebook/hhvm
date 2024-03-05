// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::OsStr;
use std::fs;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::path::PathBuf;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Context;
use anyhow::Result;
use ffi::Maybe;
use ffi::Vector;
use hhbc::BytesId;
use hhbc::ModuleName;
use hhbc::StringId;
use hhbc::StringIdMap;
use hhvm_types_ffi::Attr;
use log::trace;
use naming_special_names_rust::coeffects::Ctx;
use parse_macro::parse;

use crate::assemble_imm::AssembleImm;
use crate::lexer::Lexer;
use crate::token::Line;
use crate::token::Token;

pub(crate) type DeclMap = StringIdMap<u32>;

/// Assembles the hhas within f to a hhbc::Unit
pub fn assemble(f: &Path) -> Result<(hhbc::Unit, PathBuf)> {
    let s: Vec<u8> = fs::read(f)?;
    assemble_from_bytes(&s)
}

/// Assembles the hhas represented by the slice of bytes input
pub fn assemble_from_bytes(s: &[u8]) -> Result<(hhbc::Unit, PathBuf)> {
    let mut lex = Lexer::from_slice(s, Line(1));
    let unit = assemble_from_toks(&mut lex)?;
    trace!("ASM UNIT: {unit:?}");
    Ok(unit)
}

/// Assembles a single bytecode. This is useful for dynamic analysis,
/// where we want to assemble each bytecode as it is being executed.
pub fn assemble_single_instruction(decl_map: &mut DeclMap, s: &[u8]) -> Result<hhbc::Instruct> {
    let mut lex = Lexer::from_slice(s, Line(1));
    let mut tcb_count = 0;
    assemble_instr(&mut lex, decl_map, &mut tcb_count)
}

/// Assembles the HCU. Parses over the top level of the .hhas file
/// File is either empty OR looks like:
/// .filepath <str_literal>
/// (.function <...>)+
/// (.function_refs <...>)+
fn assemble_from_toks(token_iter: &mut Lexer<'_>) -> Result<(hhbc::Unit, PathBuf)> {
    // First token should be the filepath
    let fp = assemble_filepath(token_iter)?;

    let mut unit = UnitBuilder::default();
    while !token_iter.is_empty() {
        unit.assemble_decl(token_iter)?;
    }

    Ok((unit.into_unit(), fp))
}

#[derive(Default)]
struct UnitBuilder {
    adatas: Vec<hhbc::Adata>,
    class_refs: Option<Vec<hhbc::ClassName>>,
    classes: Vec<hhbc::Class>,
    constant_refs: Option<Vec<hhbc::ConstName>>,
    constants: Vec<hhbc::Constant>,
    fatal: Option<hhbc::Fatal>,
    file_attributes: Vec<hhbc::Attribute>,
    func_refs: Option<Vec<hhbc::FunctionName>>,
    funcs: Vec<hhbc::Function>,
    include_refs: Option<Vec<hhbc::IncludePath>>,
    module_use: Option<ModuleName>,
    modules: Vec<hhbc::Module>,
    type_constants: Vec<hhbc::TypeConstant>,
    typedefs: Vec<hhbc::Typedef>,
}

impl UnitBuilder {
    fn into_unit(self) -> hhbc::Unit {
        hhbc::Unit {
            adata: self.adatas.into(),
            functions: self.funcs.into(),
            classes: self.classes.into(),
            typedefs: self.typedefs.into(),
            file_attributes: self.file_attributes.into(),
            modules: self.modules.into(),
            module_use: self.module_use.into(),
            symbol_refs: hhbc::SymbolRefs {
                functions: self.func_refs.unwrap_or_default().into(),
                classes: self.class_refs.unwrap_or_default().into(),
                constants: self.constant_refs.unwrap_or_default().into(),
                includes: self.include_refs.unwrap_or_default().into(),
            },
            constants: self.constants.into(),
            fatal: self.fatal.into(),
            missing_symbols: Default::default(),
            error_symbols: Default::default(),
            valid_utf8: true,
            invalid_utf8_offset: 0,
        }
    }

    fn assemble_decl(&mut self, token_iter: &mut Lexer<'_>) -> Result<()> {
        let tok = token_iter.peek().unwrap();

        if !tok.is_decl() {
            return Err(tok.error("Declaration expected"))?;
        }

        match tok.as_bytes() {
            b".fatal" => {
                self.fatal = Some(assemble_fatal(token_iter)?);
            }
            b".adata" => {
                self.adatas.push(assemble_adata(token_iter)?);
            }
            b".function" => {
                self.funcs.push(assemble_function(token_iter)?);
            }
            b".class" => {
                self.classes.push(assemble_class(token_iter)?);
            }
            b".function_refs" => {
                ensure_single_defn(&self.func_refs, tok)?;
                self.func_refs = Some(assemble_refs(token_iter, ".function_refs", |t| {
                    assemble_function_name(t)
                })?);
            }
            b".class_refs" => {
                ensure_single_defn(&self.class_refs, tok)?;
                self.class_refs = Some(assemble_refs(token_iter, ".class_refs", |t| {
                    assemble_class_name(t)
                })?);
            }
            b".constant_refs" => {
                ensure_single_defn(&self.constant_refs, tok)?;
                self.constant_refs = Some(assemble_refs(token_iter, ".constant_refs", |t| {
                    assemble_const_name(t)
                })?);
            }
            b".includes" => {
                ensure_single_defn(&self.include_refs, tok)?;
                self.include_refs = Some(assemble_refs(token_iter, ".includes", |t| {
                    let path_str = if t.peek_is(Token::is_decl) {
                        t.expect(Token::is_decl)?.as_bytes()
                    } else {
                        t.expect(Token::is_identifier)?.as_bytes()
                    };
                    Ok(hhbc::IncludePath::Absolute(hhbc::intern_bytes(path_str)))
                })?)
            }
            b".alias" => {
                self.typedefs.push(assemble_typedef(token_iter, false)?);
            }
            b".case_type" => {
                self.typedefs.push(assemble_typedef(token_iter, true)?);
            }
            b".const" => {
                assemble_const_or_type_const(
                    token_iter,
                    &mut self.constants,
                    &mut self.type_constants,
                )?;

                ensure!(
                    self.type_constants.is_empty(),
                    "Type constants defined outside of a class"
                );
            }
            b".file_attributes" => {
                assemble_file_attributes(token_iter, &mut self.file_attributes)?;
            }
            b".module_use" => {
                self.module_use = Some(assemble_module_use(token_iter)?);
            }
            b".module" => {
                self.modules.push(assemble_module(token_iter)?);
            }
            _ => {
                return Err(tok.error("Unknown top level declarationr"))?;
            }
        }

        Ok(())
    }
}

fn ensure_single_defn<T>(v: &Option<T>, tok: &Token<'_>) -> Result<()> {
    if v.is_none() {
        Ok(())
    } else {
        let what = std::str::from_utf8(tok.as_bytes()).unwrap();
        Err(tok.error(format!("'{what} defined multiple times")))
    }
}

/// Ex:
/// .module m0 (64, 64) {
/// }
fn assemble_module(token_iter: &mut Lexer<'_>) -> Result<hhbc::Module> {
    parse!(token_iter,
           ".module"
           <attr:assemble_special_and_user_attrs()>
           <name:assemble_module_name()>
           <span:assemble_span>
           "{"
           <doc_comment:assemble_doc_comment>
           "}");
    let (attr, attributes) = attr;

    ensure!(
        attr == hhvm_types_ffi::ffi::Attr::AttrNone,
        "Unexpected HHVM attrs in module definition"
    );

    Ok(hhbc::Module {
        attributes: attributes.into(),
        name,
        span,
        doc_comment,
        exports: Maybe::Nothing, // TODO: Add parsing
        imports: Maybe::Nothing, // TODO: Add parsing
    })
}

/// Ex:
/// .module_use "(module_use)";
fn assemble_module_use(token_iter: &mut Lexer<'_>) -> Result<ModuleName> {
    parse!(token_iter, ".module_use" <name:string> ";");
    let name = ModuleName::intern(std::str::from_utf8(escaper::unquote_slice(
        name.as_bytes(),
    ))?);
    Ok(name)
}

/// Ex:
/// .file_attributes ["__EnableUnstableFeatures"("""v:1:{s:8:\"readonly\";}""")] ;
fn assemble_file_attributes(
    token_iter: &mut Lexer<'_>,
    file_attributes: &mut Vec<hhbc::Attribute>,
) -> Result<()> {
    parse!(token_iter, ".file_attributes" "[" <attrs:assemble_user_attr(),*> "]" ";");
    file_attributes.extend(attrs);
    Ok(())
}

/// Ex:
/// .alias ShapeKeyEscaping = <"HH\\darray"> (3,6) """D:2:{s:4:\"kind\";i:14;s:6:\"fields\";D:2:{s:11:\"Whomst'd've\";D:1:{s:5:\"value\";D:1:{s:4:\"kind\";i:1;}}s:25:\"Whomst\\u{0027}d\\u{0027}ve\";D:1:{s:5:\"value\";D:1:{s:4:\"kind\";i:4;}}}}""";
/// Note that in BCP, TypeDef's typeinfo's user_type is not printed. What's between <> is the typeinfo's constraint's name.
fn assemble_typedef(token_iter: &mut Lexer<'_>, case_type: bool) -> Result<hhbc::Typedef> {
    if case_type {
        parse!(token_iter, ".case_type");
    } else {
        parse!(token_iter, ".alias");
    }
    parse!(token_iter,
           <attrs:assemble_special_and_user_attrs()>
           <name:assemble_class_name()>
           "="
           <type_info_union:assemble_type_info_union()>
           <span:assemble_span>
           <type_structure:assemble_triple_quoted_typed_value()>
           ";");
    let (attrs, attributes) = attrs;
    Ok(hhbc::Typedef {
        name,
        attributes: attributes.into(),
        type_info_union: type_info_union.into(),
        type_structure,
        span,
        attrs,
        case_type,
    })
}

fn assemble_class(token_iter: &mut Lexer<'_>) -> Result<hhbc::Class> {
    parse!(token_iter, ".class"
       <upper_bounds:assemble_upper_bounds()>
       <attr:assemble_special_and_user_attrs()>
       <name:assemble_class_name()>
       <span:assemble_span>
       <base:assemble_base()>
       <implements:assemble_imp_or_enum_includes("implements")>
       <enum_includes:assemble_imp_or_enum_includes("enum_includes")>
       "{"
       <doc_comment:assemble_doc_comment>
       <uses:assemble_uses()>
       <enum_type:assemble_enum_ty()>
    );
    let (flags, attributes) = attr;

    let mut requirements = Vec::new();
    let mut ctx_constants = Vec::new();
    let mut properties = Vec::new();
    let mut methods = Vec::new();
    let mut constants = Vec::new();
    let mut type_constants = Vec::new();
    while let Some(tok @ Token::Decl(txt, _)) = token_iter.peek() {
        match *txt {
            b".require" => requirements.push(assemble_requirement(token_iter)?),
            b".ctx" => ctx_constants.push(assemble_ctx_constant(token_iter)?),
            b".property" => properties.push(assemble_property(token_iter)?),
            b".method" => methods.push(assemble_method(token_iter)?),
            b".const" => {
                assemble_const_or_type_const(token_iter, &mut constants, &mut type_constants)?
            }
            _ => Err(tok.error("Unknown class-level identifier"))?,
        }
    }

    parse!(token_iter, "}");

    let hhas_class = hhbc::Class {
        attributes: attributes.into(),
        base,
        implements: implements.into(),
        enum_includes: enum_includes.into(),
        name,
        span,
        uses: uses.into(),
        enum_type,
        methods: methods.into(),
        properties: properties.into(),
        constants: constants.into(),
        type_constants: type_constants.into(),
        ctx_constants: ctx_constants.into(),
        requirements: requirements.into(),
        upper_bounds: upper_bounds.into(),
        doc_comment,
        flags,
    };
    Ok(hhas_class)
}

/// Defined in 'hack/src/naming/naming_special_names.rs`
fn is_enforced_static_coeffect(d: &str) -> bool {
    match d {
        "pure" | "defaults" | "rx" | "zoned" | "write_props" | "rx_local" | "zoned_with"
        | "zoned_local" | "zoned_shallow" | "leak_safe_local" | "leak_safe_shallow"
        | "leak_safe" | "read_globals" | "globals" | "write_this_props" | "rx_shallow" => true,
        _ => false,
    }
}

/// Ex:
/// .ctx C isAbstract;
/// .ctx Clazy pure;
/// .ctx Cdebug isAbstract;
fn assemble_ctx_constant(token_iter: &mut Lexer<'_>) -> Result<hhbc::CtxConstant> {
    parse!(token_iter, ".ctx" <name:id> <is_abstract:"isAbstract"?> <tokens:id*> ";");

    // .ctx has slice of recognized and unrecognized constants.
    // Making an assumption that recognized ~~ static coeffects and
    // unrecognized ~~ unenforced static coeffects
    let ids = tokens
        .into_iter()
        .map(|tok| Ok(hhbc::intern(tok.as_str()?)))
        .collect::<Result<Vec<_>>>()?;
    let (r, u): (Vec<_>, Vec<_>) = ids
        .into_iter()
        .partition(|s| is_enforced_static_coeffect(s.as_str()));

    Ok(hhbc::CtxConstant {
        name: hhbc::intern(name.as_str()?),
        recognized: r.into(),
        unrecognized: u.into(),
        is_abstract,
    })
}

/// Ex:
/// .require extends <C>;
fn assemble_requirement(token_iter: &mut Lexer<'_>) -> Result<hhbc::Requirement> {
    parse!(token_iter, ".require" <tok:id> "<" <name:assemble_class_name()> ">" ";");
    let kind = match tok.into_identifier()? {
        b"extends" => hhbc::TraitReqKind::MustExtend,
        b"implements" => hhbc::TraitReqKind::MustImplement,
        b"class" => hhbc::TraitReqKind::MustBeClass,
        _ => return Err(tok.error("Expected TraitReqKind")),
    };
    Ok(hhbc::Requirement { name, kind })
}

fn assemble_const_or_type_const<'arena>(
    token_iter: &mut Lexer<'_>,
    consts: &mut Vec<hhbc::Constant>,
    type_consts: &mut Vec<hhbc::TypeConstant>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".const")?;
    let mut attrs = Attr::AttrNone;
    if token_iter.peek_is(Token::is_open_bracket) {
        attrs = assemble_special_and_user_attrs(token_iter)?.0;
    }
    let name = token_iter.expect(Token::is_identifier)?;
    let name = name.as_str()?;
    if token_iter.next_is_str(Token::is_identifier, "isType") {
        //type const
        let is_abstract = token_iter.next_is_str(Token::is_identifier, "isAbstract");
        let initializer = if token_iter.next_is(Token::is_equal) {
            Maybe::Just(assemble_triple_quoted_typed_value(token_iter)?)
        } else {
            Maybe::Nothing
        };
        type_consts.push(hhbc::TypeConstant {
            name: hhbc::intern(name),
            initializer,
            is_abstract,
        });
    } else {
        //const
        let name = hhbc::ConstName::intern(name);
        let value = if token_iter.next_is(Token::is_equal) {
            if token_iter.next_is_str(Token::is_identifier, "uninit") {
                Maybe::Just(hhbc::TypedValue::Uninit)
            } else {
                Maybe::Just(assemble_triple_quoted_typed_value(token_iter)?)
            }
        } else {
            Maybe::Nothing
        };
        consts.push(hhbc::Constant { name, value, attrs });
    }
    token_iter.expect(Token::is_semicolon)?;
    Ok(())
}

/// Ex:
/// .method {}{} [public abstract] (15,15) <"" N > a(<"?HH\\varray" "HH\\varray" nullable extended_hint display_nullable> $a1 = DV1("""NULL"""), <"HH\\varray" "HH\\varray" > $a2 = DV2("""varray[]""")) {
///    ...
/// }
fn assemble_method(token_iter: &mut Lexer<'_>) -> Result<hhbc::Method> {
    let method_tok = token_iter.peek().copied();
    token_iter.expect_str(Token::is_decl, ".method")?;
    let shadowed_tparams = assemble_shadowed_tparams(token_iter)?;
    let upper_bounds = assemble_upper_bounds(token_iter)?;
    let (attrs, attributes) = assemble_special_and_user_attrs(token_iter)?;
    let span = assemble_span(token_iter)?;
    let return_type_info = assemble_type_info_opt(token_iter, TypeInfoKind::NotEnumOrTypeDef)?;
    let name = assemble_method_name(token_iter)?;
    let mut decl_map = DeclMap::default();
    let params = assemble_params(token_iter, &mut decl_map)?;
    let flags = assemble_method_flags(token_iter)?;
    let (body, coeffects) = assemble_body(
        token_iter,
        &mut decl_map,
        params,
        return_type_info,
        shadowed_tparams,
        upper_bounds,
    )?;
    // the visibility is printed in the attrs
    // confusion: Visibility::Internal is a mix of AttrInternal and AttrPublic?
    let visibility =
        determine_visibility(&attrs).map_err(|e| method_tok.unwrap().error(e.to_string()))?;
    let met = hhbc::Method {
        attributes: attributes.into(),
        visibility,
        name,
        body,
        span,
        coeffects,
        flags,
        attrs,
    };
    Ok(met)
}

fn assemble_shadowed_tparams(token_iter: &mut Lexer<'_>) -> Result<Vec<StringId>> {
    token_iter.expect(Token::is_open_curly)?;
    let mut stp = Vec::new();
    while token_iter.peek_is(Token::is_identifier) {
        stp.push(hhbc::intern(
            token_iter.expect(Token::is_identifier)?.as_str()?,
        ));
        if !token_iter.peek_is(Token::is_close_curly) {
            token_iter.expect(Token::is_comma)?;
        }
    }
    token_iter.expect(Token::is_close_curly)?;
    Ok(stp)
}

fn assemble_method_flags(token_iter: &mut Lexer<'_>) -> Result<hhbc::MethodFlags> {
    let mut flag = hhbc::MethodFlags::empty();
    while token_iter.peek_is(Token::is_identifier) {
        let tok = token_iter.expect_token()?;
        match tok.into_identifier()? {
            b"isPairGenerator" => flag |= hhbc::MethodFlags::IS_PAIR_GENERATOR,
            b"isAsync" => flag |= hhbc::MethodFlags::IS_ASYNC,
            b"isGenerator" => flag |= hhbc::MethodFlags::IS_GENERATOR,
            b"isClosureBody" => flag |= hhbc::MethodFlags::IS_CLOSURE_BODY,
            _ => return Err(tok.error("Unknown function flag")),
        }
    }
    Ok(flag)
}

fn assemble_property(token_iter: &mut Lexer<'_>) -> Result<hhbc::Property> {
    let prop_tok = token_iter.peek().copied();
    // A doc comment is just a triple string literal : """{}"""
    let mut doc_comment = Maybe::Nothing;
    parse!(token_iter,
           ".property"
           <attrs:assemble_special_and_user_attrs()>
           [triple_string: <dc:assemble_unescaped_unquoted_triple_vec> { doc_comment = Maybe::Just(dc); } ;
            else: { } ;
           ]
           <type_info:assemble_type_info(TypeInfoKind::NotEnumOrTypeDef)>
           <name:assemble_prop_name()>
           "="
           <initial_value:assemble_property_initial_value()>
           ";"
    );
    let (flags, attributes) = attrs;
    let visibility =
        determine_visibility(&flags).map_err(|e| prop_tok.unwrap().error(e.to_string()))?;
    Ok(hhbc::Property {
        name,
        flags,
        attributes: attributes.into(),
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
fn assemble_property_initial_value(token_iter: &mut Lexer<'_>) -> Result<Maybe<hhbc::TypedValue>> {
    Ok(parse!(token_iter,
    [
        "uninit": "uninit" { Maybe::Just(hhbc::TypedValue::Uninit) } ;
        "\"\"\"N;\"\"\"": "\"\"\"N;\"\"\"" { Maybe::Nothing } ;
        else: <v:assemble_triple_quoted_typed_value()> { Maybe::Just(v) } ;
    ]))
}

fn assemble_class_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::ClassName> {
    parse!(token_iter, <id:id>);
    Ok(hhbc::ClassName::intern(id.as_str()?))
}

fn assemble_module_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::ModuleName> {
    parse!(token_iter, <id:id>);
    Ok(hhbc::ModuleName::intern(id.as_str()?))
}

pub(crate) fn assemble_prop_name_from_str(token_iter: &mut Lexer<'_>) -> Result<hhbc::PropName> {
    Ok(hhbc::PropName::new(assemble_unescaped_unquoted_intern_str(
        token_iter,
    )?))
}

fn assemble_prop_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::PropName> {
    // Only properties that can start with #s start with 0 or
    // 86 and are compiler added ones
    let name = if token_iter.peek_is_str(Token::is_number, "86")
        || token_iter.peek_is_str(Token::is_number, "0")
    {
        let num_prefix = token_iter.expect_with(Token::into_number)?;
        let name = token_iter.expect_with(Token::into_identifier)?;
        let mut num_prefix = num_prefix.to_vec();
        num_prefix.extend_from_slice(name);
        hhbc::intern(std::str::from_utf8(&num_prefix)?)
    } else {
        hhbc::intern(token_iter.expect(Token::is_identifier)?.as_str()?)
    };
    Ok(hhbc::PropName::new(name))
}

fn assemble_method_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::MethodName> {
    let name = if token_iter.peek_is_str(Token::is_number, "86") {
        // Only methods that can start with #s start with
        // 86 and are compiler added ones
        let under86 = token_iter.expect_with(Token::into_number)?;
        let name = token_iter.expect_with(Token::into_identifier)?;
        let mut under86 = under86.to_vec();
        under86.extend_from_slice(name);
        hhbc::intern(std::str::from_utf8(&under86)?)
    } else {
        hhbc::intern(token_iter.expect(Token::is_identifier)?.as_str()?)
    };
    Ok(hhbc::MethodName::new(name))
}

fn assemble_const_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::ConstName> {
    parse!(token_iter, <id:id>);
    Ok(hhbc::ConstName::intern(id.as_str()?))
}

fn assemble_function_name(token_iter: &mut Lexer<'_>) -> Result<hhbc::FunctionName> {
    let name = if token_iter.peek_is_str(Token::is_number, "86") {
        // Only functions that can start with #s start with
        // 86 and are compiler added ones
        let under86 = token_iter.expect_with(Token::into_number)?;
        let name = token_iter.expect_with(Token::into_identifier)?;
        let mut under86 = under86.to_vec();
        under86.extend_from_slice(name);
        hhbc::intern(std::str::from_utf8(&under86)?)
    } else {
        hhbc::intern(token_iter.expect(Token::is_identifier)?.as_str()?)
    };
    Ok(hhbc::FunctionName::new(name))
}

/// Ex:
/// extends C
/// There is only one base per Class
fn assemble_base(token_iter: &mut Lexer<'_>) -> Result<Maybe<hhbc::ClassName>> {
    Ok(parse!(token_iter, [
      "extends": "extends" <cn:assemble_class_name()> { Maybe::Just(cn) } ;
      else: { Maybe::Nothing } ;
    ]))
}

/// Ex:
/// implements (Fooable Barable)
/// enum_includes (Fooable Barable)
fn assemble_imp_or_enum_includes(
    token_iter: &mut Lexer<'_>,
    imp_or_inc: &str,
) -> Result<Vec<hhbc::ClassName>> {
    let mut classes = Vec::new();
    if token_iter.next_is_str(Token::is_identifier, imp_or_inc) {
        token_iter.expect(Token::is_open_paren)?;
        while !token_iter.peek_is(Token::is_close_paren) {
            classes.push(assemble_class_name(token_iter)?);
        }
        token_iter.expect(Token::is_close_paren)?;
    }
    Ok(classes)
}

/// Ex: .doc """doc""";
fn assemble_doc_comment(token_iter: &mut Lexer<'_>) -> Result<Maybe<Vector<u8>>> {
    Ok(parse!(token_iter, [
        ".doc": ".doc" <com:assemble_unescaped_unquoted_triple_vec> ";" { Maybe::Just(com) } ;
        else: { Maybe::Nothing } ;
    ]))
}

/// Ex: .use TNonScalar;
fn assemble_uses(token_iter: &mut Lexer<'_>) -> Result<Vec<hhbc::ClassName>> {
    let classes = parse!(token_iter, [
        ".use": ".use" <classes:assemble_class_name()*> ";" { classes } ;
        else: { Vec::new() } ;
    ]);
    Ok(classes)
}

/// Ex: .enum_ty <type_info>
fn assemble_enum_ty(token_iter: &mut Lexer<'_>) -> Result<Maybe<hhbc::TypeInfo>> {
    parse!(token_iter, [
        ".enum_ty": ".enum_ty" <ti:assemble_type_info_opt(TypeInfoKind::Enum)> ";" { Ok(ti) } ;
        else: { Ok(Maybe::Nothing) } ;
    ])
}

/// Ex: .fatal 2:63,2:63 Parse "A right parenthesis `)` is expected here.";
fn assemble_fatal(token_iter: &mut Lexer<'_>) -> Result<hhbc::Fatal> {
    parse!(token_iter, ".fatal"
           <line_begin:numeric> ":" <col_begin:numeric> ","
           <line_end:numeric> ":" <col_end:numeric>
           <op:id>
           <msg:token>
           ";"
    );
    let loc = hhbc::SrcLoc {
        line_begin,
        col_begin,
        line_end,
        col_end,
    };
    let op = match op.as_bytes() {
        b"Parse" => hhbc::FatalOp::Parse,
        b"Runtime" => hhbc::FatalOp::Runtime,
        b"RuntimeOmitFrame" => hhbc::FatalOp::RuntimeOmitFrame,
        _ => return Err(op.error("Unknown fatal op")),
    };
    let msg = escaper::unescape_literal_bytes_into_vec_bytes(msg.into_unquoted_str_literal()?)?;
    Ok(hhbc::Fatal {
        op,
        loc,
        message: msg.into(),
    })
}

/// A line of adata looks like:
/// .adata id = """<tv>"""
/// with tv being a typed value; see `assemble_typed_value` doc for what <tv> looks like.
fn assemble_adata(token_iter: &mut Lexer<'_>) -> Result<hhbc::Adata> {
    parse!(token_iter, ".adata" <id:id> "=" <value:assemble_triple_quoted_typed_value()> ";");
    let id = hhbc::AdataId::parse(std::str::from_utf8(id.as_bytes())?)?;
    Ok(hhbc::Adata { id, value })
}

/// For use by initial value
fn assemble_triple_quoted_typed_value(token_iter: &mut Lexer<'_>) -> Result<hhbc::TypedValue> {
    let (st, line) = token_iter.expect_with(Token::into_triple_str_literal_and_line)?;
    // Guaranteed st is encased by """ """
    let st = &st[3..st.len() - 3];
    let st = escaper::unescape_literal_bytes_into_vec_bytes(st)?;
    assemble_typed_value(&st, line)
}

/// tv can look like:
/// uninit | N; | s:s.len():"(escaped s)"; | l:s.len():"(escaped s)"; | d:#; | i:#; | b:0; | b:1; | D:dict.len():{((tv1);(tv2);)*}
/// | v:vec.len():{(tv;)*} | k:keyset.len():{(tv;)*}
fn assemble_typed_value(src: &[u8], line: Line) -> Result<hhbc::TypedValue> {
    fn deserialize(src: &[u8], line: Line) -> Result<hhbc::TypedValue> {
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
        fn deserialize_int(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue)> {
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
        fn deserialize_float(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue)> {
            let src = expect(src, b"d")?;
            let src = expect(src, b":")?;
            let (num, src) = read_until(src, |c| c != b';', b";")?;
            let num = std::str::from_utf8(num)?.parse()?;
            Ok((src, hhbc::TypedValue::Float(hhbc::FloatBits(num))))
        }

        /// b:0;
        fn deserialize_bool(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue)> {
            let src = expect(src, b"b")?;
            let src = expect(src, b":")?;
            let val = src[0] == b'1';
            let src = expect(&src[1..], b";")?;
            Ok((src, hhbc::TypedValue::Bool(val)))
        }

        /// N;
        fn deserialize_null(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue)> {
            let src = expect(src, b"N")?;
            let src = expect(src, b";")?;
            Ok((src, hhbc::TypedValue::Null))
        }

        /// uninit
        fn deserialize_uninit(src: &[u8]) -> Result<(&[u8], hhbc::TypedValue)> {
            Ok((
                src.strip_prefix(b"uninit")
                    .ok_or_else(|| anyhow!("Expected uninit in src {:?}", src))?,
                hhbc::TypedValue::Uninit,
            ))
        }

        /// s:s.len():"(escaped s)"; or l:s.len():"(escaped s)";
        fn deserialize_string_or_lazyclass<'a>(
            src: &'a [u8],
            s_or_l: StringOrLazyClass,
        ) -> Result<(&'a [u8], hhbc::TypedValue)> {
            let src = expect(src, s_or_l.prefix())?;
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
            Ok((src, s_or_l.build(s)?))
        }

        #[derive(PartialEq)]
        pub enum StringOrLazyClass {
            String,
            LazyClass,
        }

        impl StringOrLazyClass {
            pub fn prefix(&self) -> &[u8] {
                match self {
                    StringOrLazyClass::String => b"s",
                    StringOrLazyClass::LazyClass => b"l",
                }
            }

            pub fn build(&self, content: &[u8]) -> Result<hhbc::TypedValue, std::str::Utf8Error> {
                match self {
                    StringOrLazyClass::String => Ok(hhbc::TypedValue::intern_string(content)),
                    StringOrLazyClass::LazyClass => Ok(hhbc::TypedValue::intern_lazy_class(
                        std::str::from_utf8(content)?,
                    )),
                }
            }
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

            pub fn build(&self, content: Vec<hhbc::TypedValue>) -> hhbc::TypedValue {
                match self {
                    VecOrKeyset::Vec => hhbc::TypedValue::Vec(content.into()),
                    VecOrKeyset::Keyset => hhbc::TypedValue::Keyset(content.into()),
                }
            }
        }
        /// v:vec.len():{(tv;)*} or k:keyset.len():{(tv;)*}
        fn deserialize_vec_or_keyset<'arena, 'a>(
            src: &'a [u8],
            v_or_k: VecOrKeyset,
        ) -> Result<(&'a [u8], hhbc::TypedValue)> {
            let src = expect(src, v_or_k.prefix())?;
            let src = expect(src, b":")?;
            let (len, src) = expect_usize(src)?;
            let src = expect(src, b":")?;
            let mut src = expect(src, b"{")?;
            let mut tv;
            let mut tv_vec = Vec::new();
            for _ in 0..len {
                (src, tv) = deserialize_tv(src)?;
                tv_vec.push(tv);
            }
            let src = expect(src, b"}")?;
            Ok((src, v_or_k.build(tv_vec)))
        }

        /// D:(D.len):{p1_0; p1_1; ...; pD.len_0; pD.len_1}
        fn deserialize_dict<'a>(src: &'a [u8]) -> Result<(&'a [u8], hhbc::TypedValue)> {
            let src = expect(src, b"D")?;
            let src = expect(src, b":")?;
            let (len, src) = expect_usize(src)?;
            let src = expect(src, b":")?;
            let mut src = expect(src, b"{")?;
            let mut key;
            let mut value;
            let mut tv_vec = Vec::new();
            for _ in 0..len {
                (src, key) = deserialize_tv(src)?;
                (src, value) = deserialize_tv(src)?;
                tv_vec.push(hhbc::DictEntry { key, value })
            }
            let src = expect(src, b"}")?;
            Ok((src, hhbc::TypedValue::Dict(tv_vec.into())))
        }

        fn deserialize_tv<'a>(src: &'a [u8]) -> Result<(&'a [u8], hhbc::TypedValue)> {
            let (src, tr) = match src[0] {
                b'i' => deserialize_int(src).context("Assembling a TV int")?,
                b'b' => deserialize_bool(src).context("Assembling a TV bool")?,
                b'd' => deserialize_float(src).context("Assembling a TV float")?,
                b'N' => deserialize_null(src).context("Assembling a TV Null")?,
                b'u' => deserialize_uninit(src).context("Assembling a uninit")?,
                b's' => deserialize_string_or_lazyclass(src, StringOrLazyClass::String)
                    .context("Assembling a TV string")?,
                b'v' => deserialize_vec_or_keyset(src, VecOrKeyset::Vec)
                    .context("Assembling a TV vec")?,
                b'k' => deserialize_vec_or_keyset(src, VecOrKeyset::Keyset)
                    .context("Assembling a TV keyset")?,
                b'D' => deserialize_dict(src).context("Assembling a TV dict")?,
                b'l' => deserialize_string_or_lazyclass(src, StringOrLazyClass::LazyClass)
                    .context("Assembling a LazyClass")?,
                _ => bail!("Unknown tv: {}", src[0]),
            };
            Ok((src, tr))
        }

        ensure!(!src.is_empty(), "Empty typed value. Line {}", line);

        let (src, tv) = deserialize_tv(src).context(format!("Line {}", line))?;

        ensure!(
            src.is_empty(),
            "Unassemble-able TV. Unassembled: {:?}. Line {}",
            src,
            line
        );

        Ok(tv)
    }
    deserialize(src, line)
}

/// Class refs look like:
/// .class_refs {
///    (identifier)+
/// }
/// Function ref looks like:
/// .functionrefs {
///    (identifier)+
/// }
fn assemble_refs<'arena, 'a, T: 'arena, F>(
    token_iter: &mut Lexer<'_>,
    name_str: &str,
    assemble_name: F,
) -> Result<Vec<T>>
where
    F: Fn(&mut Lexer<'_>) -> Result<T>,
{
    token_iter.expect_str(Token::is_decl, name_str)?;
    token_iter.expect(Token::is_open_curly)?;
    let mut names = Vec::new();
    while !token_iter.peek_is(Token::is_close_curly) {
        names.push(assemble_name(token_iter)?);
    }
    token_iter.expect(Token::is_close_curly)?;
    Ok(names)
}

/// A function def is composed of the following:
/// .function {upper bounds} [special_and_user_attrs] (span) <type_info> name (params) flags? {body}
fn assemble_function(token_iter: &mut Lexer<'_>) -> Result<hhbc::Function> {
    token_iter.expect_str(Token::is_decl, ".function")?;
    let upper_bounds = assemble_upper_bounds(token_iter)?;
    // Special and user attrs may or may not be specified. If not specified, no [] printed
    let (attr, attributes) = assemble_special_and_user_attrs(token_iter)?;
    let span = assemble_span(token_iter)?;
    // Body may not have return type info, so check if next token is a < or not
    // Specifically if body doesn't have a return type info bytecode printer doesn't print anything
    // (doesn't print <>)
    let return_type_info = assemble_type_info_opt(token_iter, TypeInfoKind::NotEnumOrTypeDef)?;
    // Assemble_name

    let name = assemble_function_name(token_iter)?;
    // Will store decls in this order: params, decl_vars, unnamed
    let mut decl_map = DeclMap::default();
    let params = assemble_params(token_iter, &mut decl_map)?;
    let flags = assemble_function_flags(name, token_iter)?;
    let shadowed_tparams = Default::default();
    let (body, coeffects) = assemble_body(
        token_iter,
        &mut decl_map,
        params,
        return_type_info,
        shadowed_tparams,
        upper_bounds,
    )?;
    let hhas_func = hhbc::Function {
        attributes: attributes.into(),
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
    name: hhbc::FunctionName,
    token_iter: &mut Lexer<'_>,
) -> Result<hhbc::FunctionFlags> {
    let mut flag = hhbc::FunctionFlags::empty();
    while token_iter.peek_is(Token::is_identifier) {
        let tok = token_iter.expect_token()?;
        match tok.into_identifier()? {
            b"isPairGenerator" => flag |= hhbc::FunctionFlags::PAIR_GENERATOR,
            b"isAsync" => flag |= hhbc::FunctionFlags::ASYNC,
            b"isGenerator" => flag |= hhbc::FunctionFlags::GENERATOR,
            _ => return Err(tok.error("Unknown function flag")),
        }
    }
    if name.as_bstr().ends_with(b"$memoize_impl") {
        flag |= hhbc::FunctionFlags::MEMOIZE_IMPL;
    }
    Ok(flag)
}

/// Parses over filepath. Note that HCU doesn't hold the filepath; filepath is in the context passed to the
/// bytecode printer. So we don't store the filepath in our HCU or anywhere, but still parse over it
/// Filepath: .filepath strliteral semicolon (.filepath already consumed in `assemble_from_toks)
fn assemble_filepath(token_iter: &mut Lexer<'_>) -> Result<PathBuf> {
    // Filepath is .filepath strliteral and semicolon
    parse!(token_iter, ".filepath" <fp:string> ";");
    let fp = Path::new(OsStr::from_bytes(fp.into_unquoted_str_literal()?));
    Ok(fp.to_path_buf())
}

/// Span ex: (2, 4)
fn assemble_span(token_iter: &mut Lexer<'_>) -> Result<hhbc::Span> {
    parse!(token_iter, "(" <line_begin:numeric> "," <line_end:numeric> ")");
    Ok(hhbc::Span {
        line_begin,
        line_end,
    })
}

/// Ex: {(T as <"HH\\int" "HH\\int" upper_bound>)}
/// {(id as type_info, type_info ... ), (id as type_info, type_info ...)*}
fn assemble_upper_bounds(token_iter: &mut Lexer<'_>) -> Result<Vec<hhbc::UpperBound>> {
    parse!(token_iter, "{" <ubs:assemble_upper_bound(),*> "}");
    Ok(ubs)
}

/// Ex: (T as <"HH\\int" "HH\\int" upper_bound>)
fn assemble_upper_bound(token_iter: &mut Lexer<'_>) -> Result<hhbc::UpperBound> {
    parse!(token_iter, "(" <id:id> "as" <tis:assemble_type_info(TypeInfoKind::NotEnumOrTypeDef),*> ")");
    Ok(hhbc::UpperBound {
        name: hhbc::intern(std::str::from_utf8(id.as_bytes())?),
        bounds: tis.into(),
    })
}

/// Ex: [ "__EntryPoint"("""v:0:{}""")]. This example lacks Attrs
/// Ex: [abstract final] This example lacks Attributes
fn assemble_special_and_user_attrs(
    token_iter: &mut Lexer<'_>,
) -> Result<(hhvm_types_ffi::ffi::Attr, Vec<hhbc::Attribute>)> {
    let mut user_atts = Vec::new();
    let mut tr = hhvm_types_ffi::ffi::Attr::AttrNone;
    if token_iter.next_is(Token::is_open_bracket) {
        while !token_iter.peek_is(Token::is_close_bracket) {
            if token_iter.peek_is(Token::is_str_literal) {
                user_atts.push(assemble_user_attr(token_iter)?)
            } else if token_iter.peek_is(Token::is_identifier) {
                tr.add(assemble_hhvm_attr(token_iter)?);
            } else {
                return Err(token_iter.error("Unknown token in special and user attrs"));
            }
        }
        token_iter.expect(Token::is_close_bracket)?;
    }
    // If no special and user attrs then no [] printed
    Ok((tr, user_atts))
}

fn assemble_hhvm_attr(token_iter: &mut Lexer<'_>) -> Result<hhvm_types_ffi::ffi::Attr> {
    let tok = token_iter.expect_token()?;
    let flag = match tok.into_identifier()? {
        b"abstract" => Attr::AttrAbstract,
        b"bad_redeclare" => Attr::AttrNoBadRedeclare,
        b"builtin" => Attr::AttrBuiltin,
        b"deep_init" => Attr::AttrDeepInit,
        b"dyn_callable" => Attr::AttrDynamicallyCallable,
        b"dyn_constructible" => Attr::AttrDynamicallyConstructible,
        b"dyn_referenced" => Attr::AttrDynamicallyReferenced,
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
        b"unused_max_attr" => Attr::AttrUnusedMaxAttr,
        b"variadic_param" => Attr::AttrVariadicParam,
        _ => return Err(tok.error("Unknown attr")),
    };
    Ok(flag)
}

/// Attributes are printed as follows:
/// "name"("""v:args.len:{args}""") where args are typed values.
fn assemble_user_attr(token_iter: &mut Lexer<'_>) -> Result<hhbc::Attribute> {
    let nm = escaper::unescape_literal_bytes_into_vec_bytes(
        token_iter.expect_with(Token::into_unquoted_str_literal)?,
    )?;
    token_iter.expect(Token::is_open_paren)?;
    let arguments = assemble_user_attr_args(token_iter)?;
    token_iter.expect(Token::is_close_paren)?;
    Ok(hhbc::Attribute::new(std::str::from_utf8(&nm)?, arguments))
}

/// Printed as follows (print_attributes in bcp)
/// "v:args.len:{args}" where args are typed values
fn assemble_user_attr_args(token_iter: &mut Lexer<'_>) -> Result<Vec<hhbc::TypedValue>> {
    let tok = token_iter.peek().copied();
    if let hhbc::TypedValue::Vec(vals) = assemble_triple_quoted_typed_value(token_iter)? {
        Ok(vals.into())
    } else {
        Err(tok
            .unwrap()
            .error("Malformed user_attr_args -- should be a vec"))
    }
}

#[derive(PartialEq)]
pub enum TypeInfoKind {
    TypeDef,
    Enum,
    NotEnumOrTypeDef,
}

fn assemble_type_info(token_iter: &mut Lexer<'_>, tik: TypeInfoKind) -> Result<hhbc::TypeInfo> {
    let loc = token_iter.peek().copied();
    if let Maybe::Just(ti) = assemble_type_info_opt(token_iter, tik)? {
        Ok(ti)
    } else if let Some(loc) = loc {
        Err(loc.error("TypeInfo expected"))
    } else {
        Err(anyhow!("TypeInfo expected at end"))
    }
}

fn assemble_type_info_union(token_iter: &mut Lexer<'_>) -> Result<Vec<hhbc::TypeInfo>> {
    parse!(token_iter, <tis:assemble_type_info(TypeInfoKind::TypeDef),*>);
    Ok(tis)
}

/// Ex: <"HH\\void" N >
/// < "user_type" "type_constraint.name" type_constraint.flags >
/// if 'is_enum', no  type_constraint name printed
/// if 'is_typedef', no user_type name printed
fn assemble_type_info_opt(
    token_iter: &mut Lexer<'_>,
    tik: TypeInfoKind,
) -> Result<Maybe<hhbc::TypeInfo>> {
    if token_iter.next_is(Token::is_lt) {
        let first = token_iter.expect_with(Token::into_unquoted_str_literal)?;
        let first = escaper::unescape_literal_bytes_into_vec_bytes(first)?;
        let type_cons_name = if tik == TypeInfoKind::TypeDef {
            if first.is_empty() {
                Maybe::Nothing
            } else {
                Maybe::Just(hhbc::intern(std::str::from_utf8(&first[..])?))
            }
        } else if tik == TypeInfoKind::Enum || token_iter.next_is_str(Token::is_identifier, "N") {
            Maybe::Nothing
        } else {
            Maybe::Just(hhbc::intern(std::str::from_utf8(
                &escaper::unescape_literal_bytes_into_vec_bytes(
                    token_iter.expect_with(Token::into_unquoted_str_literal)?,
                )?,
            )?))
        };
        let user_type = if tik != TypeInfoKind::TypeDef {
            Maybe::Just(hhbc::intern(std::str::from_utf8(&first)?))
        } else {
            Maybe::Nothing
        };
        let mut tcflags = hhvm_types_ffi::ffi::TypeConstraintFlags::NoFlags;
        while !token_iter.peek_is(Token::is_gt) {
            tcflags = tcflags | assemble_type_constraint(token_iter)?;
        }
        token_iter.expect(Token::is_gt)?;
        let cons = hhbc::Constraint::new(type_cons_name, tcflags);
        Ok(Maybe::Just(hhbc::TypeInfo::new(user_type, cons)))
    } else {
        Ok(Maybe::Nothing)
    }
}

fn assemble_type_constraint(
    token_iter: &mut Lexer<'_>,
) -> Result<hhvm_types_ffi::ffi::TypeConstraintFlags> {
    use hhvm_types_ffi::ffi::TypeConstraintFlags;
    let tok = token_iter.expect_token()?;
    match tok.into_identifier()? {
        b"upper_bound" => Ok(TypeConstraintFlags::UpperBound),
        b"display_nullable" => Ok(TypeConstraintFlags::DisplayNullable),
        //no mock objects
        //resolved
        b"type_constant" => Ok(TypeConstraintFlags::TypeConstant),
        b"soft" => Ok(TypeConstraintFlags::Soft),
        b"type_var" => Ok(TypeConstraintFlags::TypeVar),
        b"extended_hint" => Ok(TypeConstraintFlags::ExtendedHint),
        b"nullable" => Ok(TypeConstraintFlags::Nullable),
        _ => Err(tok.error("Unknown type constraint flag")),
    }
}

/// ((a, )*a) | () where a is a param
fn assemble_params(token_iter: &mut Lexer<'_>, decl_map: &mut DeclMap) -> Result<Vec<hhbc::Param>> {
    token_iter.expect(Token::is_open_paren)?;
    let mut params = Vec::new();
    while !token_iter.peek_is(Token::is_close_paren) {
        params.push(assemble_param(token_iter, decl_map)?);
        if !token_iter.peek_is(Token::is_close_paren) {
            token_iter.expect(Token::is_comma)?;
        }
    }
    token_iter.expect(Token::is_close_paren)?;
    Ok(params)
}

/// a: [user_attributes]? inout? readonly? ...?<type_info>?name (= default_value)?
fn assemble_param(token_iter: &mut Lexer<'_>, decl_map: &mut DeclMap) -> Result<hhbc::Param> {
    let mut ua_vec = Vec::new();
    if token_iter.peek_is(Token::is_open_bracket) {
        token_iter.expect(Token::is_open_bracket)?;
        while !token_iter.peek_is(Token::is_close_bracket) {
            ua_vec.push(assemble_user_attr(token_iter)?);
        }
        token_iter.expect(Token::is_close_bracket)?;
    }
    let is_inout = token_iter.next_is_str(Token::is_identifier, "inout");
    let is_readonly = token_iter.next_is_str(Token::is_identifier, "readonly");
    let is_variadic = token_iter.next_is(Token::is_variadic);
    let type_info = assemble_type_info_opt(token_iter, TypeInfoKind::NotEnumOrTypeDef)?;
    let name = token_iter.expect_with(Token::into_variable)?;
    let name = hhbc::intern(std::str::from_utf8(name)?);
    decl_map.insert(name, decl_map.len() as u32);
    let default_value = assemble_default_value(token_iter)?;
    Ok(hhbc::Param {
        name,
        is_variadic,
        is_inout,
        is_readonly,
        user_attributes: ua_vec.into(),
        type_info,
        default_value,
    })
}

/// Ex: $skip_top_libcore = DV13("""true""")
/// Parsing after the variable name
fn assemble_default_value(token_iter: &mut Lexer<'_>) -> Result<Maybe<hhbc::DefaultValue>> {
    let tr = if token_iter.next_is(Token::is_equal) {
        let label = assemble_label(token_iter)?;
        token_iter.expect(Token::is_open_paren)?;
        let expr = assemble_unescaped_unquoted_triple_vec(token_iter)?;
        token_iter.expect(Token::is_close_paren)?;
        Maybe::Just(hhbc::DefaultValue { label, expr })
    } else {
        Maybe::Nothing
    };
    Ok(tr)
}

/// { (.doc ...)? (.ismemoizewrapper)? (.ismemoizewrapperlsb)? (.numiters)? (.declvars)? (instructions)* }
fn assemble_body<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &mut DeclMap,
    params: Vec<hhbc::Param>,
    return_type_info: Maybe<hhbc::TypeInfo>,
    shadowed_tparams: Vec<StringId>,
    upper_bounds: Vec<hhbc::UpperBound>,
) -> Result<(hhbc::Body, hhbc::Coeffects)> {
    let mut doc_comment = Maybe::Nothing;
    let mut instrs = Vec::new();
    let mut decl_vars = Vec::new();
    let mut num_iters = 0;
    let mut coeff = hhbc::Coeffects::default();
    let mut is_memoize_wrapper = false;
    let mut is_memoize_wrapper_lsb = false;
    // For now we don't parse params, so will just have decl_vars (might need to move this later)
    token_iter.expect(Token::is_open_curly)?;
    // In body, before instructions, are 5 possible constructs:
    // only .declvars can be declared more than once
    while token_iter.peek_is(Token::is_decl) {
        if token_iter.peek_is_str(Token::is_decl, ".doc") {
            doc_comment = assemble_doc_comment(token_iter)?;
        } else if token_iter.peek_is_str(Token::is_decl, ".ismemoizewrapperlsb") {
            token_iter.expect_str(Token::is_decl, ".ismemoizewrapperlsb")?;
            token_iter.expect(Token::is_semicolon)?;
            is_memoize_wrapper_lsb = true;
        } else if token_iter.peek_is_str(Token::is_decl, ".ismemoizewrapper") {
            token_iter.expect_str(Token::is_decl, ".ismemoizewrapper")?;
            token_iter.expect(Token::is_semicolon)?;
            is_memoize_wrapper = true;
        } else if token_iter.peek_is_str(Token::is_decl, ".numiters") {
            if num_iters != 0 {
                token_iter.error("Cannot have more than one .numiters per function body");
            }

            num_iters = assemble_numiters(token_iter)?;
        } else if token_iter.peek_is_str(Token::is_decl, ".declvars") {
            if !decl_vars.is_empty() {
                return Err(
                    token_iter.error("Cannot have more than one .declvars per function body")
                );
            }
            decl_vars = assemble_decl_vars(token_iter, decl_map)?;
        } else if token_iter.peek_is(is_coeffects_decl) {
            coeff = assemble_coeffects(token_iter)?;
        } else {
            break;
        }
    }
    // tcb_count tells how many TryCatchBegins there are that are still unclosed
    // we only stop parsing instructions once we see a is_close_curly and tcb_count is 0
    let mut tcb_count = 0;
    while tcb_count > 0 || !token_iter.peek_is(Token::is_close_curly) {
        instrs.push(assemble_instr(token_iter, decl_map, &mut tcb_count)?);
    }
    token_iter.expect(Token::is_close_curly)?;
    let stack_depth = stack_depth::compute_stack_depth(&params, &instrs)?;
    let tr = hhbc::Body {
        body_instrs: instrs.into(),
        decl_vars: decl_vars.into(),
        num_iters,
        is_memoize_wrapper,
        is_memoize_wrapper_lsb,
        doc_comment,
        params: params.into(),
        return_type_info,
        shadowed_tparams: shadowed_tparams.into(),
        stack_depth,
        upper_bounds: upper_bounds.into(),
    };
    Ok((tr, coeff))
}

fn is_coeffects_decl(tok: &Token<'_>) -> bool {
    match tok {
        Token::Decl(id, _) => id.starts_with(b".coeffects"),
        _ => false,
    }
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
fn assemble_coeffects(token_iter: &mut Lexer<'_>) -> Result<hhbc::Coeffects> {
    let mut scs = Vec::new();
    let mut uscs = Vec::new();
    let mut fun_param = Vec::new();
    let mut cc_param = Vec::new();
    let mut cc_this = Vec::new();
    let mut cc_reified = Vec::new();
    let mut closure_parent_scope = false;
    let mut generator_this = false;
    let mut caller = false;
    while token_iter.peek_is(is_coeffects_decl) {
        if token_iter.peek_is_str(Token::is_decl, ".coeffects_static") {
            assemble_static_coeffects(token_iter, &mut scs, &mut uscs)?;
        }
        if token_iter.peek_is_str(Token::is_decl, ".coeffects_fun_param") {
            assemble_coeffects_fun_param(token_iter, &mut fun_param)?;
        }
        if token_iter.peek_is_str(Token::is_decl, ".coeffects_cc_param") {
            assemble_coeffects_cc_param(token_iter, &mut cc_param)?;
        }
        if token_iter.peek_is_str(Token::is_decl, ".coeffects_cc_this") {
            assemble_coeffects_cc_this(token_iter, &mut cc_this)?;
        }
        if token_iter.peek_is_str(Token::is_decl, ".coeffects_cc_reified") {
            assemble_coeffects_cc_reified(token_iter, &mut cc_reified)?;
        }
        closure_parent_scope =
            token_iter.next_is_str(Token::is_decl, ".coeffects_closure_parent_scope");
        if closure_parent_scope {
            token_iter.expect(Token::is_semicolon)?;
        }
        generator_this = token_iter.next_is_str(Token::is_decl, ".coeffects_generator_this");
        if generator_this {
            token_iter.expect(Token::is_semicolon)?;
        }
        caller = token_iter.next_is_str(Token::is_decl, ".coeffects_caller");
        if caller {
            token_iter.expect(Token::is_semicolon)?;
        }
    }

    Ok(hhbc::Coeffects::new(
        scs,
        uscs,
        fun_param,
        cc_param,
        cc_this,
        cc_reified,
        closure_parent_scope,
        generator_this,
        caller,
    ))
}

/// Ex: .coeffects_static pure
fn assemble_static_coeffects(
    token_iter: &mut Lexer<'_>,
    scs: &mut Vec<Ctx>,
    uscs: &mut Vec<StringId>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".coeffects_static")?;
    while !token_iter.peek_is(Token::is_semicolon) {
        match token_iter.expect_with(Token::into_identifier)? {
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
            d => uscs.push(hhbc::intern(std::str::from_utf8(d)?)), //If unknown Ctx, is a unenforce_static_coeffect (ex: output)
        }
    }
    token_iter.expect(Token::is_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_fun_param 0;
fn assemble_coeffects_fun_param(
    token_iter: &mut Lexer<'_>,
    fun_param: &mut Vec<u32>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".coeffects_fun_param")?;
    while !token_iter.peek_is(Token::is_semicolon) {
        fun_param.push(token_iter.expect_and_get_number()?);
    }
    token_iter.expect(Token::is_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_cc_param 0 C 0 Cdebug;
fn assemble_coeffects_cc_param(
    token_iter: &mut Lexer<'_>,
    cc_param: &mut Vec<hhbc::CcParam>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".coeffects_cc_param")?;
    while !token_iter.peek_is(Token::is_semicolon) {
        let index = token_iter.expect_and_get_number()?;
        let ctx_name = hhbc::intern(token_iter.expect(Token::is_identifier)?.as_str()?);
        cc_param.push(hhbc::CcParam { index, ctx_name });
    }
    token_iter.expect(Token::is_semicolon)?;
    Ok(())
}

/// Ex:
/// .coeffects_cc_this Cdebug;
fn assemble_coeffects_cc_this(
    token_iter: &mut Lexer<'_>,
    cc_this: &mut Vec<hhbc::CcThis>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".coeffects_cc_this")?;
    let mut params = Vec::new();
    while !token_iter.peek_is(Token::is_semicolon) {
        params.push(hhbc::intern(
            token_iter.expect(Token::is_identifier)?.as_str()?,
        ));
    }
    token_iter.expect(Token::is_semicolon)?;
    cc_this.push(hhbc::CcThis {
        types: params.into(),
    });
    Ok(())
}

/// .coeffects_cc_reified 0 C;
fn assemble_coeffects_cc_reified(
    token_iter: &mut Lexer<'_>,
    cc_reified: &mut Vec<hhbc::CcReified>,
) -> Result<()> {
    token_iter.expect_str(Token::is_decl, ".coeffects_cc_reified")?;
    let is_class = token_iter.next_is_str(Token::is_identifier, "isClass");
    let index = token_iter.expect_and_get_number()?;
    let mut types = Vec::new();
    while !token_iter.peek_is(Token::is_semicolon) {
        types.push(hhbc::intern(
            token_iter.expect(Token::is_identifier)?.as_str()?,
        ));
    }
    token_iter.expect(Token::is_semicolon)?;
    cc_reified.push(hhbc::CcReified {
        is_class,
        index,
        types: types.into(),
    });
    Ok(())
}

/// Expects .numiters #+;
fn assemble_numiters(token_iter: &mut Lexer<'_>) -> Result<usize> {
    token_iter.expect_str(Token::is_decl, ".numiters")?;
    let num = token_iter.expect_and_get_number()?;
    token_iter.expect(Token::is_semicolon)?;
    Ok(num)
}

/// Expects .declvars ($x)+;
fn assemble_decl_vars(token_iter: &mut Lexer<'_>, decl_map: &mut DeclMap) -> Result<Vec<StringId>> {
    token_iter.expect_str(Token::is_decl, ".declvars")?;
    let mut var_names = Vec::new();
    while !token_iter.peek_is(Token::is_semicolon) {
        let var_nm = token_iter.expect_var()?;
        let var_nm = hhbc::intern(std::str::from_utf8(&var_nm)?);
        var_names.push(var_nm);
        decl_map.insert(var_nm, decl_map.len().try_into().unwrap());
    }
    token_iter.expect(Token::is_semicolon)?;
    Ok(var_names)
}

#[assemble_opcode_macro::assemble_opcode]
fn assemble_opcode(
    tok: &'_ [u8],
    token_iter: &mut Lexer<'_>,
    decl_map: &mut DeclMap,
) -> Result<hhbc::Instruct> {
    // This is filled in by the macro.
}

/// Opcodes and Pseudos
#[allow(clippy::todo)]
fn assemble_instr<'arena>(
    token_iter: &mut Lexer<'_>,
    decl_map: &mut DeclMap,
    tcb_count: &mut usize, // Increase this when get TryCatchBegin, decrease when TryCatchEnd
) -> Result<hhbc::Instruct> {
    if let Some(mut sl_lexer) = token_iter.split_at_newline() {
        if sl_lexer.peek_is(Token::is_decl) {
            // Not all pseudos are decls, but all instruction decls are pseudos
            if sl_lexer.peek_is_str(Token::is_decl, ".srcloc") {
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::SrcLoc(
                    assemble_srcloc(&mut sl_lexer)?,
                )))
            } else if sl_lexer.next_is_str(Token::is_decl, ".try") {
                sl_lexer.expect(Token::is_open_curly)?;
                *tcb_count += 1;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchBegin))
            } else {
                todo!("{}", sl_lexer.next().unwrap());
            }
        } else if sl_lexer.next_is(Token::is_close_curly) {
            if sl_lexer.next_is_str(Token::is_decl, ".catch") {
                // Is a TCM
                sl_lexer.expect(Token::is_open_curly)?;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchMiddle))
            } else {
                // Is a TCE
                debug_assert!(*tcb_count > 0);
                *tcb_count -= 1;
                Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::TryCatchEnd))
            }
        } else if sl_lexer.peek_is(Token::is_identifier) {
            let tb = sl_lexer.peek().unwrap().as_bytes();

            if sl_lexer.peek1().map_or(false, |tok| tok.is_colon()) {
                // It's actually a label.
                // DV123: or L123:
                let label = assemble_label(&mut sl_lexer)?;
                sl_lexer.expect(Token::is_colon)?;
                return Ok(hhbc::Instruct::Pseudo(hhbc::Pseudo::Label(label)));
            }

            match tb {
                b"MemoGetEager" => assemble_memo_get_eager(&mut sl_lexer),
                b"SSwitch" => assemble_sswitch(&mut sl_lexer),
                tb => assemble_opcode(tb, &mut sl_lexer, decl_map),
            }
        } else {
            Err(sl_lexer.error("Function body line that's neither decl or identifier."))
        }
    } else {
        bail!("Expected an additional instruction in body, reached EOF");
    }
}

/// .srcloc #:#,#:#;
fn assemble_srcloc(token_iter: &mut Lexer<'_>) -> Result<hhbc::SrcLoc> {
    token_iter.expect_str(Token::is_decl, ".srcloc")?;
    let tr = hhbc::SrcLoc {
        line_begin: token_iter.expect_and_get_number()?,
        col_begin: {
            token_iter.expect(Token::is_colon)?;
            token_iter.expect_and_get_number()?
        },
        line_end: {
            token_iter.expect(Token::is_comma)?;
            token_iter.expect_and_get_number()?
        },
        col_end: {
            token_iter.expect(Token::is_colon)?;
            token_iter.expect_and_get_number()?
        },
    };
    token_iter.expect(Token::is_semicolon)?;
    token_iter.expect_end()?;
    Ok(tr)
}

/// <(fcallargflag)*>
pub(crate) fn assemble_fcallargsflags(token_iter: &mut Lexer<'_>) -> Result<hhbc::FCallArgsFlags> {
    let mut flags = hhbc::FCallArgsFlags::FCANone;
    token_iter.expect(Token::is_lt)?;
    while !token_iter.peek_is(Token::is_gt) {
        let tok = token_iter.expect_token()?;
        match tok.into_identifier()? {
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
            _ => return Err(tok.error("Unrecognized FCallArgsFlags")),
        }
    }
    token_iter.expect(Token::is_gt)?;
    Ok(flags)
}

/// "(0|1)*"
pub(crate) fn assemble_inouts_or_readonly(token_iter: &mut Lexer<'_>) -> Result<Vec<bool>> {
    let tok = token_iter.expect_token()?;
    let literal = tok.into_str_literal()?;
    debug_assert!(literal[0] == b'"' && literal[literal.len() - 1] == b'"');
    // trim the outer "", which are guaranteed b/c of str token
    let tr = literal[1..literal.len() - 1]
        .iter()
        .map(|c| match *c {
            b'0' => Ok(false),
            b'1' => Ok(true),
            _ => Err(tok.error("Non 0/1 character in inouts/readonlys")),
        })
        .collect::<Result<_>>()?;
    Ok(tr)
}

/// - or a label
pub(crate) fn assemble_async_eager_target(
    token_iter: &mut Lexer<'_>,
) -> Result<Option<hhbc::Label>> {
    if token_iter.next_is(Token::is_dash) {
        Ok(None)
    } else {
        Ok(Some(assemble_label(token_iter)?))
    }
}

/// Just a string literal
pub(crate) fn assemble_fcall_context(token_iter: &mut Lexer<'_>) -> Result<StringId> {
    let st = token_iter.expect_with(Token::into_str_literal)?;
    debug_assert!(st[0] == b'"' && st[st.len() - 1] == b'"');
    Ok(hhbc::intern(std::str::from_utf8(&st[1..st.len() - 1])?)) // if not hugged by "", won't pass into_str_literal
}

pub(crate) fn assemble_unescaped_unquoted_intern_str(
    token_iter: &mut Lexer<'_>,
) -> Result<StringId> {
    let st = escaper::unescape_literal_bytes_into_vec_bytes(
        token_iter.expect_with(Token::into_unquoted_str_literal)?,
    )?;
    Ok(hhbc::intern(std::str::from_utf8(&st)?))
}

pub(crate) fn assemble_unescaped_unquoted_intern_bytes(
    token_iter: &mut Lexer<'_>,
) -> Result<BytesId> {
    let st = escaper::unescape_literal_bytes_into_vec_bytes(
        token_iter.expect_with(Token::into_unquoted_str_literal)?,
    )?;
    Ok(hhbc::intern_bytes(st.as_ref()))
}

fn assemble_unescaped_unquoted_triple_vec(token_iter: &mut Lexer<'_>) -> Result<Vector<u8>> {
    let st = escaper::unquote_slice(escaper::unquote_slice(escaper::unquote_slice(
        token_iter.expect_with(Token::into_triple_str_literal)?,
    )));
    Ok(escaper::unescape_literal_bytes_into_vec_bytes(st)?.into())
}

/// Ex:
/// SSwitch <"ARRAY7":L0 "ARRAY8":L1 "ARRAY9":L2 -:L3>
fn assemble_sswitch(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct> {
    let mut cases = Vec::new(); // Of Str<'arena>
    let mut targets = Vec::new(); // Of Labels
    token_iter.expect_str(Token::is_identifier, "SSwitch")?;
    token_iter.expect(Token::is_lt)?;
    // The last case is printed as '-' but interior it is "default"
    while !token_iter.peek_is(Token::is_gt) {
        if token_iter.peek_is(Token::is_dash) {
            // Last item; printed as '-' but is "default"
            token_iter.next();
            cases.push(hhbc::intern_bytes("default".as_bytes()));
        } else {
            cases.push(assemble_unescaped_unquoted_intern_bytes(token_iter)?);
        }
        token_iter.expect(Token::is_colon)?;
        targets.push(assemble_label(token_iter)?);
    }
    token_iter.expect(Token::is_gt)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::SSwitch(
        cases.into(),
        targets.into(),
    )))
}

/// Ex:
/// MemoGetEager L0 L1 l:0+0
fn assemble_memo_get_eager(token_iter: &mut Lexer<'_>) -> Result<hhbc::Instruct> {
    token_iter.expect_str(Token::is_identifier, "MemoGetEager")?;
    let lbl_slice = [assemble_label(token_iter)?, assemble_label(token_iter)?];
    let dum = hhbc::Dummy::DEFAULT;
    let lcl_range = assemble_local_range(token_iter)?;
    Ok(hhbc::Instruct::Opcode(hhbc::Opcode::MemoGetEager(
        lbl_slice, dum, lcl_range,
    )))
}

/// Ex:
/// MemoGet L0 L:0+0
/// last item is a local range
fn assemble_local_range(token_iter: &mut Lexer<'_>) -> Result<hhbc::LocalRange> {
    token_iter.expect_str(Token::is_identifier, "L")?;
    token_iter.expect(Token::is_colon)?;
    let start = hhbc::Local {
        idx: token_iter.expect_and_get_number()?,
    };
    //token_iter.expect(Token::is_plus)?; // Not sure if this exists yet
    let len = token_iter.expect_and_get_number()?;
    Ok(hhbc::LocalRange { start, len })
}

/// L# or DV#
pub(crate) fn assemble_label(token_iter: &mut Lexer<'_>) -> Result<hhbc::Label> {
    let tok = token_iter.expect_token()?;
    let lcl = tok.into_identifier()?;

    let rest = if lcl.starts_with(b"DV") {
        Some(&lcl[2..])
    } else if lcl.starts_with(b"L") {
        Some(&lcl[1..])
    } else {
        None
    };

    if let Some(rest) = rest {
        let value = std::str::from_utf8(rest)?.parse::<u32>()?;
        Ok(hhbc::Label(value))
    } else {
        Err(tok.error("Unknown label"))
    }
}

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::io::Result;
use std::io::Write;
use std::write;

use ast_scope::Scope;
use bstr::BString;
use bstr::ByteSlice;
use core_utils_rust::add_ns;
use error::ErrorKind;
use hhbc::ClassName;
use hhbc_string_utils::integer;
use hhbc_string_utils::is_class;
use hhbc_string_utils::is_parent;
use hhbc_string_utils::is_self;
use hhbc_string_utils::is_static;
use hhbc_string_utils::is_xhp;
use hhbc_string_utils::lstrip;
use hhbc_string_utils::lstrip_bslice;
use hhbc_string_utils::mangle;
use hhbc_string_utils::strip_global_ns;
use hhbc_string_utils::strip_ns;
use hhbc_string_utils::types;
use lazy_static::__Deref;
use lazy_static::lazy_static;
use naming_special_names_rust::classes;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::ast_defs::ParamKind;
use oxidized::local_id;
use regex::Regex;
use write_bytes::write_bytes;

use crate::context::Context;
use crate::write;
use crate::write::Error;

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct HhasBodyEnv<'a, 'arena> {
    pub is_namespaced: bool,
    pub scope: &'a Scope<'a, 'arena>,
}

pub type ExprEnv<'arena, 'e> = Option<&'e HhasBodyEnv<'e, 'arena>>;

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

fn print_field(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    field: &ast::Field,
) -> Result<()> {
    print_key_value(ctx, w, env, &field.0, &field.1)
}

fn print_afield(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    afield: &ast::Afield,
) -> Result<()> {
    use ast::Afield as A;
    match afield {
        A::AFvalue(e) => print_expr(ctx, w, env, e),
        A::AFkvalue(k, v) => print_key_value(ctx, w, env, k, v),
    }
}

fn print_afields(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    afields: impl AsRef<[ast::Afield]>,
) -> Result<()> {
    write::concat_by(w, ", ", afields, |w, i| print_afield(ctx, w, env, i))
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
    write::concat_by(w, ", ", kvs, |w, (k, v)| {
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
    write::wrap_by_(w, "darray[", "]", |w| {
        print_key_values_(ctx, w, env, kp, kvs)
    })
}

fn print_expr_varray(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    varray: &[ast::Expr],
) -> Result<()> {
    write::wrap_by_(w, "varray[", "]", |w| {
        write::concat_by(w, ", ", varray, |w, e| print_expr(ctx, w, env, e))
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
            b'\n' => Some(Cow::Borrowed(b"\\n")),
            b'\r' => Some(Cow::Borrowed(b"\\r")),
            b'\t' => Some(Cow::Borrowed(b"\\t")),
            b'\\' => Some(Cow::Borrowed(b"\\\\")),
            b'"' => Some(Cow::Borrowed(b"\\\"")),
            b'$' => Some(Cow::Borrowed(b"\\$")),
            c if escaper::is_lit_printable(c) => None,
            c => {
                let mut buf = vec![];
                write!(buf, "\\{:03o}", c).unwrap();
                Some(buf.into())
            }
        }
    }

    write::wrap_by(w, "\"", |w| {
        w.write_all(&escaper::escape_bstr_by(s.as_bstr().into(), escape_char))
    })
}

fn print_expr_to_string(
    ctx: &Context<'_>,
    env: &ExprEnv<'_, '_>,
    expr: &ast::Expr,
) -> Result<BString> {
    let mut buf = Vec::new();
    print_expr(ctx, &mut buf, env, expr).map_err(|e| match write::into_error(e) {
        Error::NotImpl(m) => Error::NotImpl(m),
        e => Error::Fail(format!("Failed: {}", e)),
    })?;
    Ok(buf.into())
}

fn print_expr(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    ast::Expr(_, _, expr): &ast::Expr,
) -> Result<()> {
    fn adjust_id<'a>(env: &ExprEnv<'_, '_>, id: &'a str) -> Cow<'a, str> {
        match env {
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
        }
    }
    fn print_expr_id<'a>(w: &mut dyn Write, env: &ExprEnv<'_, '_>, s: &'a str) -> Result<()> {
        w.write_all(adjust_id(env, s).as_bytes())
    }
    fn fmt_class_name<'a>(is_class_constant: bool, id: Cow<'a, str>) -> Cow<'a, str> {
        let stripped = strip_global_ns(&id);
        let mangled = if is_xhp(&id) {
            mangle(stripped.to_string())
        } else {
            stripped.to_string()
        };
        if is_class_constant {
            format!("\\{}", mangled).into()
        } else {
            mangled.into()
        }
    }
    fn get_class_name_from_id<'e>(
        ctx: &Context<'_>,
        env: &ExprEnv<'_, 'e>,
        should_format: bool,
        is_class_constant: bool,
        id: &'e str,
    ) -> Cow<'e, str> {
        if id == classes::SELF || id == classes::PARENT || id == classes::STATIC {
            let name = ctx.special_class_resolver.resolve(env.map(|e| e.scope), id);
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
            let class_id = ClassName::from_ast_name_and_mangle(&alloc, id);
            let id = class_id.unsafe_as_str();
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
        e_: &ast::Expr_,
    ) -> Result<Option<()>> {
        match e_.as_class_const() {
            Some((
                ast::ClassId(_, _, ast::ClassId_::CIexpr(ast::Expr(_, _, ast::Expr_::Id(id)))),
                (_, s2),
            )) if is_class(s2) && !(is_self(&id.1) || is_parent(&id.1) || is_static(&id.1)) => {
                Ok(Some({
                    let s1 = get_class_name_from_id(ctx, env, false, false, &id.1);
                    print_expr_string(w, s1.as_bytes())?
                }))
            }
            _ => Ok(None),
        }
    }
    use ast::Expr_;
    match expr {
        Expr_::Id(id) => print_expr_id(w, env, id.1.as_ref()),
        Expr_::Lvar(lid) => w.write_all((lid.1).1.as_bytes()),
        Expr_::Float(f) => {
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
        Expr_::Int(i) => print_expr_int(w, i.as_ref()),
        Expr_::String(s) => print_expr_string(w, s),
        Expr_::Null => w.write_all(b"NULL"),
        Expr_::True => w.write_all(b"true"),
        Expr_::False => w.write_all(b"false"),
        Expr_::ValCollection(vc) if matches!(vc.0.1, ast::VcKind::Vec) => {
            write::wrap_by_(w, "vec[", "]", |w| {
                write::concat_by(w, ", ", &vc.2, |w, e| print_expr(ctx, w, env, e))
            })
        }
        Expr_::ValCollection(vc) if matches!(vc.0.1, ast::VcKind::Keyset) => {
            write::wrap_by_(w, "keyset[", "]", |w| {
                write::concat_by(w, ", ", &vc.2, |w, e| print_expr(ctx, w, env, e))
            })
        }
        Expr_::KeyValCollection(kvc) if matches!(kvc.0.1, ast::KvcKind::Dict) => {
            write::wrap_by_(w, "dict[", "]", |w| {
                write::concat_by(w, ", ", &kvc.2, |w, f| print_field(ctx, w, env, f))
            })
        }
        Expr_::Collection(c) => {
            let name = strip_ns((c.0).1.as_str());
            let name = types::fix_casing(name);
            match name {
                "Set" | "Pair" | "Vector" | "Map" | "ImmSet" | "ImmVector" | "ImmMap" => {
                    w.write_all(b"HH\\")?;
                    w.write_all(name.as_bytes())?;
                    write::wrap_by_(w, " {", "}", |w| {
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
        Expr_::Shape(fl) => print_expr_darray(ctx, w, env, print_shape_field_name, fl),
        Expr_::Binop(x) => {
            let ast::Binop { bop, lhs, rhs } = &**x;
            print_expr(ctx, w, env, lhs)?;
            w.write_all(b" ")?;
            print_bop(w, bop)?;
            w.write_all(b" ")?;
            print_expr(ctx, w, env, rhs)
        }
        Expr_::Call(c) => {
            let ast::CallExpr {
                func,
                args,
                unpacked_arg,
                ..
            } = &**c;
            match func.as_id() {
                Some(ast_defs::Id(_, call_id)) => {
                    w.write_all(lstrip(adjust_id(env, call_id).as_ref(), "\\").as_bytes())?
                }
                None => {
                    let buf = print_expr_to_string(ctx, env, func)?;
                    w.write_all(lstrip_bslice(&buf, br"\"))?
                }
            };
            write::paren(w, |w| {
                write::concat_by(w, ", ", args, |w, (pk, e)| match pk {
                    ParamKind::Pnormal => print_expr(ctx, w, env, e),
                    ParamKind::Pinout(_) => Err(Error::fail("illegal default value").into()),
                })?;
                match unpacked_arg {
                    None => Ok(()),
                    Some(e) => {
                        if !args.is_empty() {
                            w.write_all(b", ")?;
                        }
                        // TODO: Should probably have ... also
                        print_expr(ctx, w, env, e)
                    }
                }
            })
        }
        Expr_::New(x) => {
            let (cid, _, es, unpacked_element, _) = &**x;
            match cid.2.as_ciexpr() {
                Some(ci_expr) => {
                    w.write_all(b"new ")?;
                    match ci_expr.2.as_id() {
                        Some(ast_defs::Id(_, cname)) => w.write_all(
                            lstrip(
                                &adjust_id(
                                    env,
                                    ClassName::from_ast_name_and_mangle(
                                        &bumpalo::Bump::new(),
                                        cname,
                                    )
                                    .unsafe_as_str(),
                                ),
                                "\\",
                            )
                            .as_bytes(),
                        )?,
                        None => {
                            let buf = print_expr_to_string(ctx, env, ci_expr)?;
                            w.write_all(lstrip_bslice(&buf, br"\"))?
                        }
                    }
                    write::paren(w, |w| {
                        write::concat_by(w, ", ", es, |w, e| print_expr(ctx, w, env, e))?;
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
                        None => Err(Error::NotImpl(format!("{}:{}", file!(), line!())).into()),
                    }
                }
            }
        }
        Expr_::ClassGet(cg) => {
            match &(cg.0).2 {
                ast::ClassId_::CIexpr(e) => match e.as_id() {
                    Some(id) => w.write_all(
                        get_class_name_from_id(
                            ctx, env, true,  /* should_format */
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
                ast::ClassGetExpr::CGstring((_, litstr)) => w.write_all(litstr.as_bytes()),
                ast::ClassGetExpr::CGexpr(e) => print_expr(ctx, w, env, e),
            }
        }
        Expr_::Nameof(box cid) => {
            w.write_all(b"nameof ")?;
            if let Some(e1) = cid.as_ciexpr() {
                if let Some(ast_defs::Id(_, s1)) = e1.2.as_id() {
                    let s1 = get_class_name_from_id(ctx, env, true, false, s1);
                    return w.write_all(s1.as_ref().as_bytes());
                }
            }
            Err(Error::fail("TODO: Only expected CIexpr of Id in Nid").into())
        }
        Expr_::ClassConst(cc) => {
            if let Some(e1) = (cc.0).2.as_ciexpr() {
                handle_possible_colon_colon_class_expr(ctx, w, env, expr)?.map_or_else(
                    || {
                        let s2 = &(cc.1).1;
                        match e1.2.as_id() {
                            Some(ast_defs::Id(_, s1)) => {
                                let s1 = get_class_name_from_id(ctx, env, true, true, s1);
                                write::concat_str_by(w, "::", [&s1.into(), s2])
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
        Expr_::Unop(u) => match u.0 {
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
        Expr_::ObjGet(og) => {
            print_expr(ctx, w, env, &og.0)?;
            w.write_all(match og.2 {
                ast::OgNullFlavor::OGNullthrows => b"->",
                ast::OgNullFlavor::OGNullsafe => b"?->",
            })?;
            print_expr(ctx, w, env, &og.1)
        }
        Expr_::Clone(e) => {
            w.write_all(b"clone ")?;
            print_expr(ctx, w, env, e)
        }
        Expr_::ArrayGet(ag) => {
            print_expr(ctx, w, env, &ag.0)?;
            write::square(w, |w| {
                write::option(w, &ag.1, |w, e: &ast::Expr| print_expr(ctx, w, env, e))
            })
        }
        Expr_::String2(ss) => write::concat_by(w, " . ", ss, |w, s| print_expr(ctx, w, env, s)),
        Expr_::PrefixedString(s) => {
            w.write_all(s.0.as_bytes())?;
            w.write_all(b" . ")?;
            print_expr(ctx, w, env, &s.1)
        }
        Expr_::Eif(eif) => {
            print_expr(ctx, w, env, &eif.0)?;
            w.write_all(b" ? ")?;
            write::option(w, &eif.1, |w, etrue| print_expr(ctx, w, env, etrue))?;
            w.write_all(b" : ")?;
            print_expr(ctx, w, env, &eif.2)
        }
        Expr_::Cast(c) => {
            write::paren(w, |w| print_hint(w, false, &c.0))?;
            print_expr(ctx, w, env, &c.1)
        }
        Expr_::Pipe(p) => {
            print_expr(ctx, w, env, &p.1)?;
            w.write_all(b" |> ")?;
            print_expr(ctx, w, env, &p.2)
        }
        Expr_::Is(i) => {
            print_expr(ctx, w, env, &i.0)?;
            w.write_all(b" is ")?;
            print_hint(w, true, &i.1)
        }
        Expr_::As(a) => {
            print_expr(ctx, w, env, &a.0)?;
            w.write_all(if a.2 { b" ?as " } else { b" as " })?;
            print_hint(w, true, &a.1)
        }
        Expr_::Varray(va) => print_expr_varray(ctx, w, env, &va.1),
        Expr_::Darray(da) => print_expr_darray(ctx, w, env, print_expr, &da.1),
        Expr_::Tuple(t) => write::wrap_by_(w, "varray[", "]", |w| {
            // A tuple is represented by a varray when using reflection.
            write::concat_by(w, ", ", t, |w, i| print_expr(ctx, w, env, i))
        }),
        Expr_::List(l) => write::wrap_by_(w, "list(", ")", |w| {
            write::concat_by(w, ", ", l, |w, i| print_expr(ctx, w, env, i))
        }),
        Expr_::Yield(y) => {
            w.write_all(b"yield ")?;
            print_afield(ctx, w, env, y)
        }
        Expr_::Await(e) => {
            w.write_all(b"await ")?;
            print_expr(ctx, w, env, e)
        }
        Expr_::Import(i) => {
            print_import_flavor(w, &i.0)?;
            w.write_all(b" ")?;
            print_expr(ctx, w, env, &i.1)
        }
        Expr_::Xml(_) => {
            Err(Error::fail("expected Xml to be converted to New during rewriting").into())
        }
        Expr_::Efun(f) => print_efun(ctx, w, env, f),
        Expr_::FunctionPointer(fp) => {
            let (fp_id, targs) = &**fp;
            match fp_id {
                ast::FunctionPtrId::FPId(ast::Id(_, sid)) => {
                    w.write_all(lstrip(adjust_id(env, sid).as_ref(), "\\").as_bytes())?
                }
                ast::FunctionPtrId::FPClassConst(ast::ClassId(_, _, class_id), (_, meth_name)) => {
                    match class_id {
                        ast::ClassId_::CIexpr(e) => match e.as_id() {
                            Some(id) => w.write_all(
                                get_class_name_from_id(
                                    ctx, env, true,  /* should_format */
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
            write::wrap_by_(w, "<", ">", |w| {
                write::concat_by(w, ", ", targs, |w, _targ| w.write_all(b"_"))
            })
        }
        Expr_::Omitted => Ok(()),
        Expr_::Lfun(lfun) => {
            if ctx.dump_lambdas {
                let fun_ = &lfun.0;
                write::paren(w, |w| {
                    write::paren(w, |w| {
                        write::concat_by(w, ", ", &fun_.params, |w, param| {
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
        Expr_::ETSplice(splice) => {
            w.write_all(b"${")?;
            print_expr(ctx, w, env, splice)?;
            w.write_all(b"}")
        }
        Expr_::EnumClassLabel(ecl) => match &ecl.0 {
            Some(ast_defs::Id(_, s1)) => {
                let s1 = get_class_name_from_id(ctx, env, true, true, s1);
                write::concat_str_by(w, "#", [&s1.into(), &ecl.1])
            }
            None => {
                w.write_all(b"#")?;
                w.write_all(ecl.1.as_bytes())
            }
        },
        Expr_::ReadonlyExpr(expr) => {
            w.write_all(b"readonly(")?;
            print_expr(ctx, w, env, expr)?;
            w.write_all(b")")
        }
        Expr_::Invalid(expr_opt) => match &expr_opt.deref() {
            Some(expr) => print_expr(ctx, w, env, expr),
            _ => Ok(()),
        },
        Expr_::Package(_)
        | Expr_::Dollardollar(_)
        | Expr_::ExpressionTree(_)
        | Expr_::Hole(_)
        | Expr_::KeyValCollection(_)
        | Expr_::Lplaceholder(_)
        | Expr_::MethodCaller(_)
        | Expr_::Pair(_)
        | Expr_::This
        | Expr_::Upcast(_)
        | Expr_::ValCollection(_) => {
            todo!("T117477443: Unimplemented: Cannot print: {:?}", expr)
        }
    }
}

fn print_xml(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    env: &ExprEnv<'_, '_>,
    id: &str,
    es: &[ast::Expr],
) -> Result<()> {
    use ast::Expr;
    use ast::Expr_;

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
            (Expr(_, _, Expr_::Shape(attrs)), Expr(_, _, Expr_::Varray(children))) => {
                (attrs, &children.1)
            }
            _ => return Err(syntax_error().into()),
        }
    };
    write!(w, "new {}", mangle(id.into()))?;
    write::paren(w, |w| {
        write::wrap_by_(w, "darray[", "]", |w| {
            write::concat_by(w, ", ", attrs, |w, attr| print_xhp_attr(ctx, w, &env, attr))
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
    efun: &ast::Efun,
) -> Result<()> {
    let f = &efun.fun;
    if f.fun_kind.is_fasync() || f.fun_kind.is_fasync_generator() {
        write!(w, "async ",)?;
    }
    w.write_all(b"function ")?;
    write::paren(w, |w| {
        write::concat_by(w, ", ", &f.params, |w, p| print_fparam(ctx, w, env, p))
    })?;
    w.write_all(b" ")?;
    let use_list = &efun.use_;
    if !use_list.is_empty() {
        w.write_all(b"use ")?;
        write::paren(w, |w| {
            write::concat_by(
                w,
                ", ",
                use_list,
                |w: &mut dyn Write, ast::CaptureLid(_, ast::Lid(_, id))| {
                    w.write_all(local_id::get_name(id).as_bytes())
                },
            )
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
    match block {
        [] | [ast::Stmt(_, ast::Stmt_::Noop)] => Ok(()),
        [ast::Stmt(_, ast::Stmt_::Block(box (_, b)))] if b.len() == 1 => {
            print_block_(ctx, w, env, b)
        }
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
    write::wrap_by_(w, "{\n", "}\n", |w| {
        write::concat(w, block, |w, stmt| {
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
        S_::Return(expr) => write::wrap_by_(w, "return", ";\n", |w| {
            write::option(w, &**expr, |w, e| {
                w.write_all(b" ")?;
                print_expr(ctx, w, env, e)
            })
        }),
        S_::Expr(expr) => {
            print_expr(ctx, w, env, expr)?;
            w.write_all(b";\n")
        }
        S_::Throw(expr) => write::wrap_by_(w, "throw ", ";\n", |w| print_expr(ctx, w, env, expr)),
        S_::Break => w.write_all(b"break;\n"),
        S_::Continue => w.write_all(b"continue;\n"),
        S_::While(x) => {
            let (cond, block) = &**x;
            write::wrap_by_(w, "while (", ") ", |w| print_expr(ctx, w, env, cond))?;
            print_block(ctx, w, env, block.as_ref())
        }
        S_::If(x) => {
            let (cond, if_block, else_block) = &**x;
            write::wrap_by_(w, "if (", ") ", |w| print_expr(ctx, w, env, cond))?;
            print_block(ctx, w, env, if_block)?;
            let mut buf = Vec::new();
            print_block(ctx, &mut buf, env, else_block).map_err(|e| {
                match write::into_error(e) {
                    e @ Error::NotImpl(_) => e,
                    e => Error::Fail(format!("Failed: {}", e)),
                }
            })?;
            if !buf.is_empty() {
                write_bytes!(w, " else {}", BString::from(buf))?;
            };
            Ok(())
        }
        S_::Block(box (_, block)) => print_block_(ctx, w, env, block),
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
    write::option(w, &(param.type_hint).1, |w, h| {
        print_hint(w, true, h)?;
        w.write_all(b" ")
    })?;
    w.write_all(param.name.as_bytes())?;
    write::option(w, &param.expr, |w, e| {
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
    let h = emit_type_hint::fmt_hint(&alloc, &[], false, hint).map_err(|e| match e.kind() {
        ErrorKind::Unrecoverable(s) => Error::fail(s),
        _ => Error::fail("Error printing hint"),
    })?;
    let stripped = if ns { &h } else { strip_ns(&h) };
    w.write_all(stripped.as_bytes())
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

    let env = None;
    let mut escaped_src = Vec::new();
    print_expr(&ctx, &mut escaped_src, &env, expr).expect("Printing failed");

    let bs = escaper::unescape_double(unsafe { std::str::from_utf8_unchecked(&escaped_src) })
        .expect("Unescaping failed");
    let s = String::from_utf8_lossy(&bs);
    s.to_string()
}

pub fn external_print_expr(
    ctx: &Context<'_>,
    w: &mut dyn std::io::Write,
    env: &ExprEnv<'_, '_>,
    expr: &ast::Expr,
) -> std::result::Result<(), Error> {
    print_expr(ctx, w, env, expr).map_err(write::into_error)?;
    w.flush().map_err(write::into_error)?;
    Ok(())
}

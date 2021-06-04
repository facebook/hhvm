// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_emit_method as emit_method;
use hhbc_by_ref_emit_property as emit_property;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_method::HhasMethod;
use hhbc_by_ref_hhas_property::HhasProperty;
use hhbc_by_ref_hhas_xhp_attribute::HhasXhpAttribute;
use hhbc_by_ref_hhbc_id::{class, Id};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{unrecoverable, Result};
use oxidized::{ast::*, ast_defs, local_id, pos::Pos};

pub fn properties_for_cache<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    class: &'a Class_,
    class_is_const: bool,
) -> Result<Option<HhasProperty<'arena>>> {
    let initial_value = Some(Expr(Pos::make_none(), Expr_::mk_null()));
    let property = emit_property::from_ast(
        alloc,
        emitter,
        class,
        &[],
        class_is_const,
        emit_property::FromAstArgs {
            initial_value: &initial_value,
            visibility: Visibility::Private,
            is_static: true,
            is_abstract: false,
            is_readonly: false,
            typehint: None,
            doc_comment: None,
            user_attributes: &[],
            id: &ast_defs::Id(Pos::make_none(), "__xhpAttributeDeclarationCache".into()),
        },
    )?;
    Ok(Some(property))
}

pub fn from_attribute_declaration<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    class: &'a Class_,
    xal: &[HhasXhpAttribute],
    xual: &[Hint],
) -> Result<HhasMethod<'arena>> {
    let id_from_str = |s: &str| Expr_::mk_id(ast_defs::Id(Pos::make_none(), s.into()));

    let mk_var_r = || {
        mk_expr(Expr_::mk_lvar(Lid(
            Pos::make_none(),
            local_id::make_unscoped("$r"),
        )))
    };
    let mk_cache = || {
        let self_ = mk_expr(id_from_str("self"));
        mk_expr(Expr_::mk_class_get(
            ClassId(Pos::make_none(), ClassId_::CIexpr(self_)),
            ClassGetExpr::CGstring((Pos::make_none(), "$__xhpAttributeDeclarationCache".into())),
            false,
        ))
    };
    let token1 = mk_stmt(Stmt_::mk_expr(mk_expr(Expr_::mk_binop(
        Bop::Eq(None),
        mk_var_r(),
        mk_cache(),
    ))));
    // if ($r === null) {
    //   self::$__xhpAttributeDeclarationCache =
    //       __SystemLib\\merge_xhp_attr_declarations(
    //          parent::__xhpAttributeDeclaration(),
    //          attributes
    //        );
    //   $r = self::$__xhpAttributeDeclarationCache;
    // }
    let cond = mk_expr(Expr_::mk_binop(
        Bop::Eqeqeq,
        mk_var_r(),
        mk_expr(Expr_::mk_null()),
    ));
    let mut args = vec![mk_expr(Expr_::mk_call(
        mk_expr(Expr_::mk_class_const(
            ClassId(
                Pos::make_none(),
                ClassId_::CIexpr(mk_expr(id_from_str("parent"))),
            ),
            (Pos::make_none(), "__xhpAttributeDeclaration".into()),
        )),
        vec![],
        vec![],
        None,
    ))];
    for xua in xual.iter() {
        match xua.1.as_happly() {
            Some((ast_defs::Id(_, s), hints)) if hints.is_empty() => {
                let s = string_utils::mangle(string_utils::strip_global_ns(s).into());
                let arg = mk_expr(Expr_::mk_call(
                    mk_expr(Expr_::mk_class_const(
                        ClassId(
                            Pos::make_none(),
                            ClassId_::CIexpr(mk_expr(Expr_::mk_id(ast_defs::Id(
                                Pos::make_none(),
                                s,
                            )))),
                        ),
                        (Pos::make_none(), "__xhpAttributeDeclaration".into()),
                    )),
                    vec![],
                    vec![],
                    None,
                ));
                args.push(arg);
            }
            _ => return Err(unrecoverable("Xhp use attribute - unexpected attribute")),
        }
    }
    args.push(emit_xhp_attribute_array(alloc, xal)?);
    let array_merge_call = mk_expr(Expr_::mk_call(
        mk_expr(id_from_str("__SystemLib\\merge_xhp_attr_declarations")),
        vec![],
        args,
        None,
    ));
    let set_cache = mk_stmt(Stmt_::mk_expr(mk_expr(Expr_::mk_binop(
        Bop::Eq(None),
        mk_cache(),
        array_merge_call,
    ))));
    let set_r = mk_stmt(Stmt_::mk_expr(mk_expr(Expr_::mk_binop(
        Bop::Eq(None),
        mk_var_r(),
        mk_cache(),
    ))));
    let token2 = mk_stmt(Stmt_::mk_if(cond, vec![set_cache, set_r], vec![]));
    let token3 = mk_stmt(Stmt_::mk_return(Some(mk_var_r()))); // return $r;
    let body = vec![token1, token2, token3];
    from_xhp_attribute_declaration_method(
        alloc,
        emitter,
        class,
        None,
        "__xhpAttributeDeclaration",
        false,
        false,
        true,
        Visibility::Protected,
        body,
    )
}

pub fn from_children_declaration<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    ast_class: &'a Class_,
    (pos, children): &(&ast_defs::Pos, Vec<&XhpChild>),
) -> Result<HhasMethod<'arena>> {
    let children_arr = mk_expr(emit_xhp_children_array(children)?);
    let body = vec![Stmt((*pos).clone(), Stmt_::mk_return(Some(children_arr)))];
    from_xhp_attribute_declaration_method(
        alloc,
        emitter,
        ast_class,
        Some((*pos).clone()),
        "__xhpChildrenDeclaration",
        false,
        false,
        false,
        Visibility::Protected,
        body,
    )
}

pub fn from_category_declaration<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    ast_class: &'a Class_,
    (pos, categories): &(&ast_defs::Pos, Vec<&String>),
) -> Result<HhasMethod<'arena>> {
    let category_arr = mk_expr(get_category_array(categories));
    let body = vec![mk_stmt(Stmt_::mk_return(Some(category_arr)))];
    from_xhp_attribute_declaration_method(
        alloc,
        emitter,
        ast_class,
        Some((*pos).clone()),
        "__xhpCategoryDeclaration",
        false,
        false,
        false,
        Visibility::Protected,
        body,
    )
}

fn get_category_array(categories: &[&String]) -> Expr_ {
    // TODO: is this always 1?
    Expr_::mk_darray(
        None,
        categories
            .iter()
            .map(|&s| {
                (
                    mk_expr(Expr_::String(s.clone().into())),
                    mk_expr(Expr_::Int("1".into())),
                )
            })
            .collect(),
    )
}

fn emit_xhp_children_array(children: &[&XhpChild]) -> Result<Expr_> {
    match children {
        [] => Ok(Expr_::mk_int("0".into())),
        [c] => match c.as_child_name() {
            Some(ast_defs::Id(_, n)) => {
                if n.eq_ignore_ascii_case("empty") {
                    Ok(Expr_::mk_int("0".into()))
                } else if n.eq_ignore_ascii_case("any") {
                    Ok(Expr_::mk_int("1".into()))
                } else {
                    emit_xhp_children_paren_expr(c)
                }
            }
            None => emit_xhp_children_paren_expr(c),
        },
        _ => Err(unrecoverable(
            "HHVM does not support multiple children declarations",
        )),
    }
}

fn emit_xhp_children_paren_expr(child: &XhpChild) -> Result<Expr_> {
    let (children, op_num) = if let Some(l) = child.as_child_list() {
        (l, xhp_child_op_to_int(None))
    } else if let Some((Some(l), op)) = child
        .as_child_unary()
        .map(|(c, op)| (c.as_child_list(), op))
    {
        (l, xhp_child_op_to_int(Some(op)))
    } else {
        return Err(unrecoverable(concat!(
            "Xhp children declarations cannot be plain id, ",
            "plain binary or unary without an inside list"
        )));
    };
    let arr = emit_xhp_children_decl_expr("0", children)?;
    get_array3(Expr_::Int(op_num.to_string()), Expr_::Int("5".into()), arr)
}

fn emit_xhp_children_decl_expr(unary: &str, children: &[XhpChild]) -> Result<Expr_> {
    match children {
        [] => Err(unrecoverable("xhp children: unexpected empty list")),
        [c] => emit_xhp_child_decl(unary, c),
        [c1, c2, cs @ ..] => {
            let first_two = get_array3(
                Expr_::Int("4".into()),
                emit_xhp_child_decl(unary, c1)?,
                emit_xhp_child_decl(unary, c2)?,
            );
            cs.iter().fold(first_two, |acc, c| {
                get_array3(Expr_::Int("4".into()), acc?, emit_xhp_child_decl(unary, c)?)
            })
        }
    }
}

fn emit_xhp_child_decl(unary: &str, child: &XhpChild) -> Result<Expr_> {
    match child {
        XhpChild::ChildList(l) => get_array3(
            Expr_::Int(unary.into()),
            Expr_::Int("5".into()),
            emit_xhp_children_decl_expr("0", l)?,
        ),
        XhpChild::ChildName(ast_defs::Id(_, name)) => {
            if name.eq_ignore_ascii_case("any") {
                get_array3(
                    Expr_::Int(unary.into()),
                    Expr_::Int("1".into()),
                    Expr_::Null,
                )
            } else if name.eq_ignore_ascii_case("pcdata") {
                get_array3(
                    Expr_::Int(unary.into()),
                    Expr_::Int("2".into()),
                    Expr_::Null,
                )
            } else if let Some(end) = name.strip_prefix('%') {
                get_array3(
                    Expr_::Int(unary.into()),
                    Expr_::Int("4".into()),
                    Expr_::String(string_utils::mangle(end.into()).into()),
                )
            } else {
                get_array3(
                    Expr_::Int(unary.into()),
                    Expr_::Int("3".into()),
                    Expr_::String(string_utils::mangle(name.into()).into()),
                )
            }
        }
        XhpChild::ChildUnary(c, op) => {
            emit_xhp_child_decl(xhp_child_op_to_int(Some(op)).to_string().as_str(), &**c)
        }
        XhpChild::ChildBinary(c1, c2) => get_array3(
            Expr_::Int("5".into()),
            emit_xhp_child_decl(unary, &**c1)?,
            emit_xhp_child_decl(unary, &**c2)?,
        ),
    }
}

fn get_array3(i0: Expr_, i1: Expr_, i2: Expr_) -> Result<Expr_> {
    Ok(Expr_::mk_varray(
        None,
        vec![mk_expr(i0), mk_expr(i1), mk_expr(i2)],
    ))
}

fn xhp_child_op_to_int(op: Option<&XhpChildOp>) -> usize {
    match op {
        None => 0,
        Some(XhpChildOp::ChildStar) => 1,
        Some(XhpChildOp::ChildQuestion) => 2,
        Some(XhpChildOp::ChildPlus) => 3,
    }
}

fn emit_xhp_attribute_array<'arena>(
    alloc: &'arena bumpalo::Bump,
    xal: &[HhasXhpAttribute],
) -> Result<Expr> {
    fn hint_to_num(id: &str) -> usize {
        match id {
            "HH\\string" => 1,
            "HH\\bool" => 2,
            "HH\\int" => 3,
            "array" => 4,
            "var" | "HH\\mixed" => 6,
            "enum" => 7,
            "HH\\float" => 8,
            "callable" => 9,
            // Regular class names are type 5
            _ => 5,
        }
    }
    fn get_enum_attributes(enum_opt: Option<&Vec<Expr>>) -> Result<Expr> {
        match enum_opt {
            None => Err(unrecoverable(
                "Xhp attribute that's supposed to be an enum but not really",
            )),
            Some(es) => Ok(mk_expr(Expr_::mk_varray(None, es.to_vec()))),
        }
    }
    fn get_attribute_array_values<'arena>(
        alloc: &'arena bumpalo::Bump,
        id: &str,
        enum_opt: Option<&Vec<Expr>>,
    ) -> Result<(Expr, Expr)> {
        let id = class::Type::from_ast_name_and_mangle(alloc, id).to_raw_string();
        let type_ = hint_to_num(id);
        let type_ident = mk_expr(Expr_::Int(type_.to_string()));
        let class_name = match type_ {
            5 => mk_expr(Expr_::String(id.into())),
            7 => get_enum_attributes(enum_opt)?,
            _ => mk_expr(Expr_::Null),
        };
        Ok((class_name, type_ident))
    }
    fn extract_from_hint<'arena>(
        alloc: &'arena bumpalo::Bump,
        hint: &Hint,
        enum_opt: Option<&Vec<Expr>>,
    ) -> Result<(Expr, Expr)> {
        use naming_special_names_rust::fb;
        match &*(hint.1) {
            Hint_::Happly(ast_defs::Id(_, inc), hs) if inc == fb::INCORRECT_TYPE => match &hs[..] {
                [h] => extract_from_hint(alloc, h, enum_opt),
                _ => get_attribute_array_values(alloc, inc, enum_opt),
            },
            Hint_::Hlike(h) | Hint_::Hoption(h) => extract_from_hint(alloc, h, enum_opt),
            Hint_::Happly(ast_defs::Id(_, id), _) => {
                get_attribute_array_values(alloc, id, enum_opt)
            }
            _ => Err(unrecoverable(
                "There are no other possible xhp attribute hints",
            )),
        }
    }
    fn inner_array<'arena>(
        alloc: &'arena bumpalo::Bump,
        xa: &HhasXhpAttribute,
    ) -> Result<Vec<Expr>> {
        let enum_opt = xa.maybe_enum.map(|(_, es)| es);
        let expr = match &(xa.class_var).expr {
            Some(e) => e.clone(),
            None => mk_expr(Expr_::Null),
        };
        let (class_name, hint) = match &xa.type_ {
            // attribute declared with the var identifier - we treat it as mixed
            None if enum_opt.is_none() => {
                get_attribute_array_values(alloc, "\\HH\\mixed", enum_opt)
            }
            None => get_attribute_array_values(alloc, "enum", enum_opt),
            // As it turns out, if there is a type list, HHVM discards it
            Some(h) => extract_from_hint(alloc, h, enum_opt),
        }?;
        let is_required = mk_expr(Expr_::Int(
            (if xa.is_required() { "1" } else { "0" }).into(),
        ));
        Ok(vec![hint, class_name, expr, is_required])
    }
    fn emit_xhp_attribute<'arena>(
        alloc: &'arena bumpalo::Bump,
        xa: &HhasXhpAttribute,
    ) -> Result<(Expr, Expr)> {
        let k = mk_expr(Expr_::String(
            string_utils::clean(&((xa.class_var).id).1).into(),
        ));
        let v = mk_expr(Expr_::mk_varray(None, inner_array(alloc, xa)?));
        Ok((k, v))
    }
    let xal_arr = xal
        .iter()
        .map(|x| emit_xhp_attribute(alloc, x))
        .collect::<Result<Vec<_>>>()?;
    Ok(mk_expr(Expr_::mk_darray(None, xal_arr)))
}

fn from_xhp_attribute_declaration_method<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    class: &'a Class_,
    pos: Option<Pos>,
    name: &str,
    final_: bool,
    abstract_: bool,
    static_: bool,
    visibility: Visibility,
    ast: Block,
) -> Result<HhasMethod<'arena>> {
    let meth = Method_ {
        span: pos.unwrap_or_else(Pos::make_none),
        annotation: (),
        final_,
        abstract_,
        static_,
        readonly_this: false, // TODO readonly emitter
        visibility,
        name: ast_defs::Id(Pos::make_none(), name.into()),
        tparams: vec![],
        where_constraints: vec![],
        variadic: FunVariadicity::FVnonVariadic,
        params: vec![],
        ctxs: None,        // TODO(T70095684)
        unsafe_ctxs: None, // TODO(T70095684)
        body: FuncBody {
            ast,
            annotation: (),
        },
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![],
        readonly_ret: None, // TODO readonly emitter
        ret: TypeHint((), None),
        external: false,
        doc_comment: None,
    };
    emit_method::from_ast(alloc, emitter, class, meth)
}

fn mk_expr(expr_: Expr_) -> Expr {
    Expr(Pos::make_none(), expr_)
}

fn mk_stmt(stmt_: Stmt_) -> Stmt {
    Stmt(Pos::make_none(), stmt_)
}

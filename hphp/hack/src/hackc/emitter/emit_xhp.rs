// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use emit_property::PropAndInit;
use env::emitter::Emitter;
use error::Error;
use error::Result;
use hack_macros::hack_expr;
use hack_macros::hack_stmts;
use hhbc::Method;
use hhbc_string_utils as string_utils;
use oxidized::ast::*;
use oxidized::ast_defs;
use oxidized::pos::Pos;

use crate::emit_method;
use crate::emit_property;
use crate::xhp_attribute::XhpAttribute;

pub fn properties_for_cache<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    class: &'a Class_,
    class_is_const: bool,
    class_is_closure: bool,
) -> Result<PropAndInit> {
    let initial_value = Some(Expr((), Pos::NONE, Expr_::mk_null()));
    emit_property::from_ast(
        emitter,
        class,
        &[],
        class_is_const,
        class_is_closure,
        emit_property::FromAstArgs {
            initial_value: &initial_value,
            visibility: Visibility::Private,
            is_static: true,
            is_abstract: false,
            is_readonly: false,
            typehint: None,
            doc_comment: None,
            user_attributes: &[],
            id: &ast_defs::Id(Pos::NONE, "__xhpAttributeDeclarationCache".into()),
        },
    )
}

pub fn from_attribute_declaration<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    class: &'a Class_,
    xal: &[XhpAttribute<'_>],
    xual: &[Hint],
) -> Result<Method<'arena>> {
    let mut args = vec![(
        ParamKind::Pnormal,
        hack_expr!("parent::__xhpAttributeDeclaration()"),
    )];

    for xua in xual.iter() {
        match xua.1.as_happly() {
            Some((ast_defs::Id(_, s), hints)) if hints.is_empty() => {
                let s = string_utils::mangle(string_utils::strip_global_ns(s).into());
                let arg = hack_expr!("#{id(s)}::__xhpAttributeDeclaration()");
                args.push((ParamKind::Pnormal, arg));
            }
            _ => {
                return Err(Error::unrecoverable(
                    "Xhp use attribute - unexpected attribute",
                ));
            }
        }
    }
    args.push((
        ParamKind::Pnormal,
        emit_xhp_attribute_array(emitter.alloc, xal)?,
    ));

    let body = Block(hack_stmts!(
        r#"
            $r = self::$__xhpAttributeDeclarationCache;
            if ($r === null) {
                self::$__xhpAttributeDeclarationCache = __SystemLib\merge_xhp_attr_declarations(#{args*});
                $r = self::$__xhpAttributeDeclarationCache;
            }
            return $r;
    "#
    ));
    from_xhp_attribute_declaration_method(
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

pub fn from_children_declaration<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    ast_class: &'a Class_,
    (pos, children): &(&ast_defs::Pos, Vec<&XhpChild>),
) -> Result<Method<'arena>> {
    let children_arr = mk_expr(emit_xhp_children_array(children)?);
    let body = Block(vec![Stmt(
        (*pos).clone(),
        Stmt_::mk_return(Some(children_arr)),
    )]);
    from_xhp_attribute_declaration_method(
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

pub fn from_category_declaration<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    ast_class: &'a Class_,
    (pos, categories): &(&ast_defs::Pos, Vec<&String>),
) -> Result<Method<'arena>> {
    let category_arr = mk_expr(get_category_array(categories));
    let body = Block(vec![mk_stmt(Stmt_::mk_return(Some(category_arr)))]);
    from_xhp_attribute_declaration_method(
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
    Expr_::KeyValCollection(Box::new((
        (Pos::NONE, KvcKind::Dict),
        None,
        categories
            .iter()
            .map(|&s| {
                Field(
                    mk_expr(Expr_::String(s.clone().into())),
                    mk_expr(Expr_::Int("1".into())),
                )
            })
            .collect(),
    )))
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
        _ => Err(Error::unrecoverable(
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
        return Err(Error::unrecoverable(concat!(
            "Xhp children declarations cannot be plain id, ",
            "plain binary or unary without an inside list"
        )));
    };
    let arr = emit_xhp_children_decl_expr("0", children)?;
    get_array3(Expr_::Int(op_num.to_string()), Expr_::Int("5".into()), arr)
}

fn emit_xhp_children_decl_expr(unary: &str, children: &[XhpChild]) -> Result<Expr_> {
    match children {
        [] => Err(Error::unrecoverable("xhp children: unexpected empty list")),
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
            emit_xhp_child_decl(xhp_child_op_to_int(Some(op)).to_string().as_str(), c)
        }
        XhpChild::ChildBinary(c1, c2) => get_array3(
            Expr_::Int("5".into()),
            emit_xhp_child_decl(unary, c1)?,
            emit_xhp_child_decl(unary, c2)?,
        ),
    }
}

fn get_array3(i0: Expr_, i1: Expr_, i2: Expr_) -> Result<Expr_> {
    Ok(Expr_::ValCollection(Box::new((
        (Pos::NONE, VcKind::Vec),
        None,
        vec![mk_expr(i0), mk_expr(i1), mk_expr(i2)],
    ))))
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
    xal: &[XhpAttribute<'_>],
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
            None => Err(Error::unrecoverable(
                "Xhp attribute that's supposed to be an enum but not really",
            )),
            Some(es) => Ok(mk_expr(Expr_::ValCollection(Box::new((
                (Pos::NONE, VcKind::Vec),
                None,
                es.to_vec(),
            ))))),
        }
    }
    fn get_attribute_array_values<'arena>(
        id: &str,
        enum_opt: Option<&Vec<Expr>>,
    ) -> Result<(Expr, Expr)> {
        let id = hhbc::ClassName::from_ast_name_and_mangle(id).as_str();
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
        match &*(hint.1) {
            Hint_::Hlike(h) | Hint_::Hoption(h) => extract_from_hint(alloc, h, enum_opt),
            Hint_::Happly(ast_defs::Id(_, id), _) => get_attribute_array_values(id, enum_opt),
            _ => Err(Error::unrecoverable(
                "There are no other possible xhp attribute hints",
            )),
        }
    }
    fn inner_array<'arena>(
        alloc: &'arena bumpalo::Bump,
        xa: &XhpAttribute<'_>,
    ) -> Result<Vec<Expr>> {
        let enum_opt = xa.maybe_enum.map(|(_, es)| es);
        let expr = match &(xa.class_var).expr {
            Some(e) => e.clone(),
            None => mk_expr(Expr_::Null),
        };
        let (class_name, hint) = match &xa.type_ {
            // attribute declared with the var identifier - we treat it as mixed
            None if enum_opt.is_none() => get_attribute_array_values("\\HH\\mixed", enum_opt),
            None => get_attribute_array_values("enum", enum_opt),
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
        xa: &XhpAttribute<'_>,
    ) -> Result<Field> {
        let k = mk_expr(Expr_::String(
            string_utils::clean(&((xa.class_var).id).1).into(),
        ));
        let v = mk_expr(Expr_::ValCollection(Box::new((
            (Pos::NONE, VcKind::Vec),
            None,
            inner_array(alloc, xa)?,
        ))));
        Ok(Field(k, v))
    }
    let xal_arr = xal
        .iter()
        .map(|x| emit_xhp_attribute(alloc, x))
        .collect::<Result<Vec<_>>>()?;
    Ok(mk_expr(Expr_::KeyValCollection(Box::new((
        (Pos::NONE, KvcKind::Dict),
        None,
        xal_arr,
    )))))
}

fn from_xhp_attribute_declaration_method<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    class: &'a Class_,
    pos: Option<Pos>,
    name: &str,
    final_: bool,
    abstract_: bool,
    static_: bool,
    visibility: Visibility,
    fb_ast: Block,
) -> Result<Method<'arena>> {
    let meth = Method_ {
        span: pos.clone().unwrap_or(Pos::NONE),
        annotation: (),
        final_,
        abstract_,
        static_,
        readonly_this: false, // TODO readonly emitter
        visibility,
        name: ast_defs::Id(Pos::NONE, name.into()),
        tparams: vec![],
        where_constraints: vec![],
        params: vec![],
        ctxs: Some(Contexts(pos.unwrap_or(Pos::NONE), vec![])),
        unsafe_ctxs: None,
        body: FuncBody { fb_ast },
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: Default::default(),
        readonly_ret: None, // TODO readonly emitter
        ret: TypeHint((), None),
        external: false,
        doc_comment: None,
    };
    emit_method::from_ast(emitter, class, meth)
}

fn mk_expr(expr_: Expr_) -> Expr {
    Expr((), Pos::NONE, expr_)
}

fn mk_stmt(stmt_: Stmt_) -> Stmt {
    Stmt(Pos::NONE, stmt_)
}

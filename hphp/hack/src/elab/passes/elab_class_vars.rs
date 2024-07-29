// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassVar;
use nast::Class_;
use nast::ClassishKind;
use nast::Expr;
use nast::Expr_;
use nast::Hint;
use nast::Hint_;
use nast::Id;
use nast::Tprim;
use nast::TypeHint;
use nast::UserAttribute;
use nast::XhpAttr;
use nast::XhpAttrInfo;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ElabClassVarsPass;

impl Pass for ElabClassVarsPass {
    // TODO[mjt] split out elaboration of `XhpAttr`s?
    fn on_ty_class__top_down(&mut self, env: &Env, elem: &mut Class_) -> ControlFlow<()> {
        let const_user_attr_opt = elem
            .user_attributes
            .iter()
            .find(|ua| ua.name.1 == sn::user_attributes::CONST);

        // Modify static and instance props
        elem.vars.iter_mut().for_each(|var| {
            // apply to both static and instance props
            let Id(_, nm) = &var.id;
            if nm.starts_with(':') {
                var.xhp_attr = Some(XhpAttrInfo {
                    like: None,
                    tag: None,
                    enum_values: vec![],
                })
            } else {
                var.xhp_attr = None
            }
            // For instance props only, add CONST user attr const is the class is has it
            if !var.is_static {
                if let Some(ua) = const_user_attr_opt
                    && !var
                        .user_attributes
                        .iter()
                        .any(|ua| ua.name.1 == sn::user_attributes::CONST)
                {
                    var.user_attributes.push(UserAttribute {
                        name: ua.name.clone(),
                        params: vec![],
                    });
                }
            }
        });

        // Represent xhp_attrs as vars
        elem.xhp_attrs
            .drain(0..)
            .for_each(|xhp_attr| elem.vars.push(class_var_of_xhp_attr(xhp_attr, env)));

        // If this is an interface mark all methods as abstract
        if matches!(elem.kind, ClassishKind::Cinterface) {
            elem.methods.iter_mut().for_each(|m| m.abstract_ = true)
        }

        Continue(())
    }
}

// Convert an `XhpAttr` into a `ClassVar`
// The `XhpAttr` is represented by a 4-tuple of
// 1) a `TypeHint` which is a position paired with an optional `Hint`
// 2) a `ClassVar`
// 3) an `XhpAttrTag` which is either `Required`  or `LateInit`
// 4) a position paired with a vector of `Expr`, which corresponds to an
//    attribute declared with `enum {...}` rather than an explicit hint
// To represent as a `ClassVar`, we use the provided `ClassVar` then
// - clear the user attributes
// - update the `ClassVar` `XhpAttrInfo` `tag` field with the `XhpAttrTag`
// - update the `ClassVar` `type_` field using either the provided `TypeHint`
//   (when it is defined) or a hint inferred from the `enum` `Expr`s when it is
//   not
//
//  TODO[mjt] This elaboration step is quite involved and seemingly has no docs
//  The representation of `XhpAttr` doesn't help here:
//  i) we can represent an attribute having neither an explicit hint or an enum declaration
//  ii) the list of `Expr` can actually only be int or string literals
//  iii) `ClassVar` `XhpAttrInfo` `enum_values` already contains a validated and restricted
//       representation of the `Expr`s
fn class_var_of_xhp_attr(xhp_attr: XhpAttr, env: &Env) -> ClassVar {
    let XhpAttr(type_hint, mut cv, xhp_attr_tag_opt, enum_opt) = xhp_attr;
    let is_required = xhp_attr_tag_opt.is_some();
    let has_default = if let Some(Expr(_, _, expr_)) = &cv.expr {
        !matches!(expr_, Expr_::Null)
    } else {
        false
    };
    if let Some(xhp_attr) = &mut cv.xhp_attr {
        xhp_attr.tag = xhp_attr_tag_opt
    }
    cv.user_attributes.clear();

    let TypeHint(type_hint_pos, type_hint_hint_opt) = type_hint;

    // If we have `enum_opt`, a list of expressions, try and build a hint based on the
    // occurrence of expression literals
    // If we dont, use the (optional) hint within `type_hint`
    let mut hint_opt: Option<Hint> = enum_opt
        .map(|(pos, items)| Hint(pos, Box::new(xhp_attr_hint(items))))
        .or(type_hint_hint_opt);

    // Now examine the hint, if we have it, removing any `Hlike`
    // - if we have an `Hoption` but `is_required` is true, raise and error
    // - if we have mixed, do nothing
    // - if we have `is_required` or `has_default` do nothing
    // - otherwise, wrap the hint in an `Hoption`
    if let Some(hint) = &mut hint_opt {
        match strip_like(&hint.1) {
            // If we have an `Hoption` but `is_required` is true, raise an
            // error
            Hint_::Hoption(_) if is_required => {
                let Id(_, attr_name) = &cv.id;
                env.emit_error(NamingError::XhpOptionalRequiredAttr {
                    pos: hint.0.clone(),
                    attr_name: attr_name.clone(),
                });
            }
            // If the hint is `Hmixed` or we have either `is_required` or
            // `has_default`, just put back the `Hint_`
            Hint_::Hmixed => (),
            _ if is_required || has_default => (),
            // Otherwise, wrap the hint in `Hoption`
            _ => {
                *hint.1 = Hint_::Hoption(hint.clone());
            }
        }
    }

    // Finally, map our optional hint and the optional position from `cv`s
    // `xhp_hint`
    // If both are present wrap the hint in an `Hlike` using the position;
    // raise an error if like hints aren't enabled
    if !env.like_type_hints_enabled() {
        if let Some((pos, Hint(_, hint_))) = cv
            .xhp_attr
            .as_ref()
            .and_then(|xai| xai.like.as_ref())
            .zip(hint_opt.as_ref())
        {
            if matches!(hint_ as &Hint_, Hint_::Hlike(_)) {
                env.emit_error(ExperimentalFeature::LikeType(pos.clone()))
            }
        }
    }

    cv.type_ = TypeHint(type_hint_pos, hint_opt);
    cv
}

#[derive(Copy, Clone)]
enum XhpHint {
    Neither,
    Int,
    String,
    Both,
}

// If we have a like `Hint_` return a reference to the inner `Hint_` otherwise
// return the reference to the outer hint
fn strip_like(hint_: &Hint_) -> &Hint_ {
    if let Hint_::Hlike(Hint(_, inner_hint_)) = hint_ {
        inner_hint_
    } else {
        hint_
    }
}

impl XhpHint {
    pub fn combine(self, other: Self) -> ControlFlow<Self, Self> {
        match (self, other) {
            (Self::Both, _) => Break(self),
            (_, Self::Both) => Break(other),
            (Self::String, Self::Int) | (Self::Int, Self::String) => Break(Self::Both),
            (Self::Neither, _) => Continue(other),
            (_, Self::Neither) => Continue(self),
            (Self::Int, Self::Int) | (Self::String, Self::String) => Continue(self),
        }
    }

    pub fn to_hint_(self) -> Hint_ {
        match self {
            XhpHint::Int => Hint_::Hprim(Tprim::Tint),
            XhpHint::String => Hint_::Hprim(Tprim::Tstring),
            _ => Hint_::Hmixed,
        }
    }
}
// TODO[mjt] This function seems a little odd; we are folding over the
// expressions and short-circuiting when have seen both an `Int` and
// `String` / `String2` expression. At that point we say the hint should
// be `Hmixed`. It isn't clear why (1) we are doing a limited version of
// inference and (2) why we aren't inferring `Hprim Tarraykey` in this case
fn xhp_attr_hint(items: Vec<Expr>) -> Hint_ {
    match items
        .into_iter()
        .try_fold(XhpHint::Neither, |acc, Expr(_, _, expr_)| match expr_ {
            Expr_::Int(_) => acc.combine(XhpHint::Int),
            Expr_::String(_) | Expr_::String2(_) => acc.combine(XhpHint::String),
            _ => Continue(acc),
        }) {
        Continue(xhp_hint) | Break(xhp_hint) => xhp_hint.to_hint_(),
    }
}

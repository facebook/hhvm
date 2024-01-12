// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized_by_ref as obr;
use pos::Pos;

use crate::decl;
use crate::decl::folded;
use crate::decl::shallow;
use crate::decl::ty;
use crate::decl::Ty;
use crate::decl::Ty_;
use crate::reason::Reason;

#[inline]
fn slice<T: Copy + Into<U>, U>(items: &[T]) -> Box<[U]> {
    items.iter().copied().map(Into::into).collect()
}

#[inline]
fn map<'a, K1, V1, K2, V2, M>(items: impl Iterator<Item = (&'a K1, &'a V1)>) -> M
where
    K1: Copy + Into<K2> + 'a,
    V1: Copy + Into<V2> + 'a,
    M: FromIterator<(K2, V2)>,
{
    items.map(|(&k, &v)| (k.into(), v.into())).collect()
}

impl From<obr::ast_defs::XhpEnumValue<'_>> for ty::XhpEnumValue {
    fn from(x: obr::ast_defs::XhpEnumValue<'_>) -> Self {
        use obr::ast_defs::XhpEnumValue as Obr;
        match x {
            Obr::XEVInt(i) => Self::XEVInt(i),
            Obr::XEVString(s) => Self::XEVString(s.into()),
        }
    }
}

impl From<obr::typing_defs::CeVisibility<'_>> for ty::CeVisibility {
    fn from(x: obr::typing_defs::CeVisibility<'_>) -> Self {
        use obr::typing_defs::CeVisibility as Obr;
        match x {
            Obr::Vpublic => Self::Public,
            Obr::Vprivate(s) => Self::Private(s.into()),
            Obr::Vprotected(s) => Self::Protected(s.into()),
            Obr::Vinternal(s) => Self::Internal(s.into()),
        }
    }
}

fn tshape_field_name_from_decl<P: Pos>(
    x: obr::typing_defs::TshapeFieldName<'_>,
) -> (ty::ShapeFieldNamePos<P>, ty::TshapeFieldName) {
    use obr::typing_defs_core::TshapeFieldName as Obr;
    use ty::ShapeFieldNamePos as SfnPos;
    use ty::TshapeFieldName;
    match x {
        Obr::TSFlitInt(&pos_id) => (
            SfnPos::Simple(pos_id.0.into()),
            TshapeFieldName::TSFlitInt(pos_id.1.into()),
        ),
        Obr::TSFlitStr(&pos_bytes) => (
            SfnPos::Simple(pos_bytes.0.into()),
            TshapeFieldName::TSFlitStr(pos_bytes.1.into()),
        ),
        Obr::TSFclassConst(&(pos_id1, pos_id2)) => (
            SfnPos::ClassConst(pos_id1.0.into(), pos_id2.0.into()),
            TshapeFieldName::TSFclassConst(pos_id1.1.into(), pos_id2.1.into()),
        ),
    }
}

impl From<&obr::typing_defs::UserAttributeParam<'_>> for ty::UserAttributeParam {
    fn from(attr: &obr::typing_defs::UserAttributeParam<'_>) -> Self {
        use obr::typing_defs::UserAttributeParam as UAP;
        match *attr {
            UAP::Classname(cn) => Self::Classname(cn.into()),
            UAP::EnumClassLabel(l) => Self::EnumClassLabel(l.into()),
            UAP::String(s) => Self::String(s.into()),
            UAP::Int(i) => Self::Int(i.into()),
        }
    }
}

impl<P: Pos> From<&obr::typing_defs::UserAttribute<'_>> for ty::UserAttribute<P> {
    fn from(attr: &obr::typing_defs::UserAttribute<'_>) -> Self {
        Self {
            name: attr.name.into(),
            params: (attr.params.iter()).map(Into::into).collect(),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::Tparam<'_>> for ty::Tparam<R, Ty<R>> {
    fn from(tparam: &obr::typing_defs::Tparam<'_>) -> Self {
        Self {
            variance: tparam.variance,
            name: tparam.name.into(),
            tparams: slice(tparam.tparams),
            constraints: (tparam.constraints.iter())
                .map(|(kind, ty)| (*kind, (*ty).into()))
                .collect(),
            reified: tparam.reified,
            user_attributes: slice(tparam.user_attributes),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::WhereConstraint<'_>> for ty::WhereConstraint<Ty<R>> {
    fn from(x: &obr::typing_defs::WhereConstraint<'_>) -> Self {
        Self(x.0.into(), x.1, x.2.into())
    }
}

fn decl_shape_field_type<R: Reason>(
    field_name_pos: ty::ShapeFieldNamePos<R::Pos>,
    sft: &obr::typing_defs::ShapeFieldType<'_>,
) -> ty::ShapeFieldType<R> {
    ty::ShapeFieldType {
        field_name_pos,
        optional: sft.optional,
        ty: sft.ty.into(),
    }
}

impl<R: Reason> From<&obr::typing_defs::Ty<'_>> for Ty<R> {
    fn from(ty: &obr::typing_defs::Ty<'_>) -> Self {
        use obr::typing_defs_core;
        use Ty_::*;
        let reason = R::from(*ty.0);
        let ty_ = match ty.1 {
            typing_defs_core::Ty_::Tthis => Tthis,
            typing_defs_core::Ty_::Tapply(&(pos_id, tys)) => {
                Tapply(Box::new((pos_id.into(), slice(tys))))
            }
            typing_defs_core::Ty_::Tmixed => Tmixed,
            typing_defs_core::Ty_::Twildcard => Twildcard,
            typing_defs_core::Ty_::Tlike(ty) => Tlike(ty.into()),
            typing_defs_core::Ty_::Tany(_) => Tany,
            typing_defs_core::Ty_::Tnonnull => Tnonnull,
            typing_defs_core::Ty_::Tdynamic => Tdynamic,
            typing_defs_core::Ty_::Toption(ty) => Toption(ty.into()),
            typing_defs_core::Ty_::Tprim(prim) => Tprim(*prim),
            typing_defs_core::Ty_::Tfun(ft) => Tfun(Box::new(ft.into())),
            typing_defs_core::Ty_::Ttuple(tys) => Ttuple(slice(tys)),
            typing_defs_core::Ty_::Tshape(&typing_defs_core::ShapeType {
                origin: _,
                unknown_value: kind,
                fields,
            }) => Tshape(Box::new(ty::ShapeType(
                kind.into(),
                fields
                    .iter()
                    .map(|(name, ty)| {
                        let (field_name_pos, name) = tshape_field_name_from_decl(name.0);
                        (name, decl_shape_field_type(field_name_pos, ty))
                    })
                    .collect(),
            ))),
            typing_defs_core::Ty_::Trefinement(&(ty, cr)) => {
                Trefinement(Box::new(decl::TrefinementType {
                    ty: ty.into(),
                    refinement: ty::ClassRefinement {
                        consts: (cr.cr_consts.iter())
                            .map(|(k, v)| ((*k).into(), (*v).into()))
                            .collect(),
                    },
                }))
            }
            typing_defs_core::Ty_::Tgeneric(&(pos_id, tys)) => {
                Tgeneric(Box::new((pos_id.into(), slice(tys))))
            }
            typing_defs_core::Ty_::Tunion(tys) => Tunion(slice(tys)),
            typing_defs_core::Ty_::Tintersection(tys) => Tintersection(slice(tys)),
            typing_defs_core::Ty_::TvecOrDict(&(ty1, ty2)) => {
                TvecOrDict(Box::new((ty1.into(), ty2.into())))
            }
            typing_defs_core::Ty_::Taccess(taccess_type) => Taccess(Box::new(taccess_type.into())),
            typing_defs_core::Ty_::TunappliedAlias(_)
            | typing_defs_core::Ty_::Tnewtype(_)
            | typing_defs_core::Ty_::Tdependent(_)
            | typing_defs_core::Ty_::Tclass(_)
            | typing_defs_core::Ty_::Tneg(_)
            | typing_defs_core::Ty_::Tvar(_) => {
                unreachable!("Not used in decl tys")
            }
        };
        Ty::new(reason, ty_)
    }
}

impl<R: Reason> From<obr::typing_defs::RefinedConst<'_>> for ty::RefinedConst<Ty<R>> {
    fn from(rc: obr::typing_defs::RefinedConst<'_>) -> Self {
        Self {
            bound: rc.bound.into(),
            is_ctx: rc.is_ctx,
        }
    }
}

impl<R: Reason> From<obr::typing_defs::RefinedConstBound<'_>> for ty::RefinedConstBound<Ty<R>> {
    fn from(ctr: obr::typing_defs::RefinedConstBound<'_>) -> Self {
        use obr::typing_defs::RefinedConstBound::*;
        match ctr {
            TRexact(ty) => Self::Exact(ty.into()),
            TRloose(bounds) => Self::Loose(bounds.into()),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::RefinedConstBounds<'_>> for ty::RefinedConstBounds<Ty<R>> {
    fn from(bounds: &obr::typing_defs::RefinedConstBounds<'_>) -> Self {
        Self {
            lower: slice(bounds.lower),
            upper: slice(bounds.upper),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::TaccessType<'_>> for ty::TaccessType<R, Ty<R>> {
    fn from(taccess_type: &obr::typing_defs::TaccessType<'_>) -> Self {
        Self {
            ty: taccess_type.0.into(),
            type_const: taccess_type.1.into(),
        }
    }
}

impl<R: Reason> From<obr::typing_defs::Capability<'_>> for ty::Capability<R, Ty<R>> {
    fn from(cap: obr::typing_defs::Capability<'_>) -> Self {
        use obr::typing_defs_core::Capability as Obr;
        match cap {
            Obr::CapDefaults(pos) => Self::CapDefaults(pos.into()),
            Obr::CapTy(ty) => Self::CapTy(ty.into()),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::FunImplicitParams<'_>> for ty::FunImplicitParams<R, Ty<R>> {
    fn from(x: &obr::typing_defs::FunImplicitParams<'_>) -> Self {
        Self {
            capability: x.capability.into(),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::FunType<'_>> for ty::FunType<R, Ty<R>> {
    fn from(ft: &obr::typing_defs::FunType<'_>) -> Self {
        Self {
            tparams: slice(ft.tparams),
            where_constraints: slice(ft.where_constraints),
            params: slice(ft.params),
            implicit_params: ft.implicit_params.into(),
            ret: ft.ret.into(),
            flags: ft.flags,
            cross_package: ft.cross_package.as_ref().map(|s| (*s).into()),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs_core::FunParam<'_>> for ty::FunParam<R, Ty<R>> {
    fn from(fp: &obr::typing_defs_core::FunParam<'_>) -> Self {
        Self {
            pos: fp.pos.into(),
            name: fp.name.map(Into::into),
            ty: fp.type_.into(),
            flags: fp.flags,
            def_value: fp.def_value.map(Into::into),
        }
    }
}

impl From<obr::typing_defs::ClassConstFrom<'_>> for ty::ClassConstFrom {
    fn from(x: obr::typing_defs::ClassConstFrom<'_>) -> Self {
        use obr::typing_defs::ClassConstFrom as Obr;
        match x {
            Obr::Self_ => Self::Self_,
            Obr::From(s) => Self::From(s.into()),
        }
    }
}

impl From<obr::typing_defs::ClassConstRef<'_>> for ty::ClassConstRef {
    fn from(x: obr::typing_defs::ClassConstRef<'_>) -> Self {
        Self(x.0.into(), x.1.into())
    }
}

impl<R: Reason> From<&obr::typing_defs::AbstractTypeconst<'_>> for ty::AbstractTypeconst<R> {
    fn from(x: &obr::typing_defs::AbstractTypeconst<'_>) -> Self {
        Self {
            as_constraint: x.as_constraint.map(Into::into),
            super_constraint: x.super_constraint.map(Into::into),
            default: x.default.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::ConcreteTypeconst<'_>> for ty::ConcreteTypeconst<R> {
    fn from(x: &obr::typing_defs::ConcreteTypeconst<'_>) -> Self {
        Self {
            ty: x.tc_type.into(),
        }
    }
}

impl<R: Reason> From<obr::typing_defs::Typeconst<'_>> for ty::Typeconst<R> {
    fn from(x: obr::typing_defs::Typeconst<'_>) -> Self {
        use obr::typing_defs::Typeconst as Obr;
        match x {
            Obr::TCAbstract(atc) => Self::TCAbstract(atc.into()),
            Obr::TCConcrete(ctc) => Self::TCConcrete(ctc.into()),
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::EnumType<'_>> for ty::EnumType<R> {
    fn from(x: &obr::typing_defs::EnumType<'_>) -> Self {
        Self {
            base: x.base.into(),
            constraint: x.constraint.map(Into::into),
            includes: slice(x.includes),
        }
    }
}

impl<P: Pos> From<(&obr::pos::Pos<'_>, bool)> for ty::Enforceable<P> {
    fn from((pos, is_enforceable): (&obr::pos::Pos<'_>, bool)) -> Self {
        if is_enforceable {
            Self(Some(pos.into()))
        } else {
            Self(None)
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ShallowClassConst<'_>>
    for shallow::ShallowClassConst<R>
{
    fn from(scc: &obr::shallow_decl_defs::ShallowClassConst<'_>) -> Self {
        Self {
            kind: scc.abstract_,
            name: scc.name.into(),
            ty: scc.type_.into(),
            refs: slice(scc.refs),
            value: scc.value.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ShallowTypeconst<'_>>
    for shallow::ShallowTypeconst<R>
{
    fn from(stc: &obr::shallow_decl_defs::ShallowTypeconst<'_>) -> Self {
        Self {
            name: stc.name.into(),
            kind: stc.kind.into(),
            enforceable: <ty::Enforceable<R::Pos>>::from(stc.enforceable),
            reifiable: stc.reifiable.map(Into::into),
            is_ctx: stc.is_ctx,
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ShallowMethod<'_>> for shallow::ShallowMethod<R> {
    fn from(sm: &obr::shallow_decl_defs::ShallowMethod<'_>) -> Self {
        Self {
            name: sm.name.into(),
            ty: sm.type_.into(),
            visibility: sm.visibility,
            deprecated: sm.deprecated.map(Into::into),
            attributes: slice(sm.attributes),
            flags: sm.flags,
            sort_text: sm.sort_text.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ShallowProp<'_>> for shallow::ShallowProp<R> {
    fn from(sp: &obr::shallow_decl_defs::ShallowProp<'_>) -> Self {
        Self {
            name: sp.name.into(),
            xhp_attr: sp.xhp_attr,
            ty: sp.type_.into(),
            visibility: sp.visibility,
            flags: sp.flags,
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ClassDecl<'_>> for shallow::ShallowClass<R> {
    fn from(sc: &obr::shallow_decl_defs::ClassDecl<'_>) -> Self {
        // Destructure to help ensure we convert every field.
        let obr::shallow_decl_defs::ClassDecl {
            mode,
            final_,
            abstract_,
            is_xhp,
            internal,
            has_xhp_keyword,
            kind,
            module,
            name,
            tparams,
            where_constraints,
            extends,
            uses,
            xhp_attr_uses,
            xhp_enum_values,
            xhp_marked_empty,
            req_extends,
            req_implements,
            req_class,
            implements,
            support_dynamic_type,
            consts,
            typeconsts,
            props,
            sprops,
            constructor,
            static_methods,
            methods,
            user_attributes,
            enum_type,
            docs_url,
        } = sc;
        Self {
            mode: *mode,
            is_final: *final_,
            is_abstract: *abstract_,
            is_internal: *internal,
            is_xhp: *is_xhp,
            has_xhp_keyword: *has_xhp_keyword,
            kind: *kind,
            module: module.map(Into::into),
            name: (*name).into(),
            tparams: slice(tparams),
            where_constraints: slice(where_constraints),
            extends: slice(extends),
            uses: slice(uses),
            xhp_attr_uses: slice(xhp_attr_uses),
            xhp_enum_values: (xhp_enum_values.iter())
                .map(|(&k, v)| (k.into(), slice(v)))
                .collect(),
            xhp_marked_empty: *xhp_marked_empty,
            req_extends: slice(req_extends),
            req_implements: slice(req_implements),
            req_class: slice(req_class),
            implements: slice(implements),
            support_dynamic_type: *support_dynamic_type,
            consts: slice(consts),
            typeconsts: slice(typeconsts),
            props: slice(props),
            static_props: slice(sprops),
            constructor: constructor.map(Into::into),
            static_methods: slice(static_methods),
            methods: slice(methods),
            user_attributes: slice(user_attributes),
            enum_type: enum_type.map(Into::into),
            docs_url: docs_url.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::FunDecl<'_>> for shallow::FunDecl<R> {
    fn from(sf: &obr::shallow_decl_defs::FunDecl<'_>) -> Self {
        Self {
            pos: sf.pos.into(),
            ty: sf.type_.into(),
            deprecated: sf.deprecated.map(Into::into),
            module: sf.module.map(Into::into),
            internal: sf.internal,
            php_std_lib: sf.php_std_lib,
            support_dynamic_type: sf.support_dynamic_type,
            no_auto_dynamic: sf.no_auto_dynamic,
            no_auto_likes: sf.no_auto_likes,
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::TypedefDecl<'_>> for shallow::TypedefDecl<R> {
    fn from(x: &obr::shallow_decl_defs::TypedefDecl<'_>) -> Self {
        Self {
            module: x.module.map(Into::into),
            pos: x.pos.into(),
            vis: x.vis,
            tparams: slice(x.tparams),
            as_constraint: x.as_constraint.map(Into::into),
            super_constraint: x.super_constraint.map(Into::into),
            ty: x.type_.into(),
            is_ctx: x.is_ctx,
            attributes: slice(x.attributes),
            internal: x.internal,
            docs_url: x.docs_url.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ConstDecl<'_>> for shallow::ConstDecl<R> {
    fn from(x: &obr::shallow_decl_defs::ConstDecl<'_>) -> Self {
        Self {
            pos: x.pos.into(),
            ty: x.type_.into(),
            value: x.value.map(Into::into),
        }
    }
}

impl From<obr::typing_defs::ModuleReference<'_>> for ty::ModuleReference {
    fn from(x: obr::typing_defs::ModuleReference<'_>) -> Self {
        use obr::typing_defs::ModuleReference as Obr;
        match x {
            Obr::MRGlobal => Self::MRGlobal,
            Obr::MRPrefix(m) => Self::MRPrefix(m.into()),
            Obr::MRExact(m) => Self::MRExact(m.into()),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::ModuleDefType<'_>> for shallow::ModuleDecl<R> {
    fn from(x: &obr::shallow_decl_defs::ModuleDefType<'_>) -> Self {
        Self {
            pos: x.pos.into(),
            exports: x.exports.map(slice),
            imports: x.imports.map(slice),
        }
    }
}

impl<R: Reason> From<&obr::shallow_decl_defs::Decl<'_>> for shallow::Decl<R> {
    fn from(decl: &obr::shallow_decl_defs::Decl<'_>) -> Self {
        use obr::shallow_decl_defs::Decl as Obr;
        match *decl {
            Obr::Class(x) => Self::Class(x.into()),
            Obr::Fun(x) => Self::Fun(x.into()),
            Obr::Typedef(x) => Self::Typedef(x.into()),
            Obr::Const(x) => Self::Const(x.into()),
            Obr::Module(x) => Self::Module(x.into()),
        }
    }
}

impl<R: Reason> From<&(&str, obr::shallow_decl_defs::Decl<'_>)> for shallow::NamedDecl<R> {
    fn from(decl: &(&str, obr::shallow_decl_defs::Decl<'_>)) -> Self {
        use obr::shallow_decl_defs::Decl as Obr;
        match *decl {
            (name, Obr::Class(x)) => Self::Class(name.into(), x.into()),
            (name, Obr::Fun(x)) => Self::Fun(name.into(), x.into()),
            (name, Obr::Typedef(x)) => Self::Typedef(name.into(), x.into()),
            (name, Obr::Const(x)) => Self::Const(name.into(), x.into()),
            (name, Obr::Module(x)) => Self::Module(name.into(), x.into()),
        }
    }
}

impl From<&obr::decl_defs::Element<'_>> for folded::FoldedElement {
    fn from(x: &obr::decl_defs::Element<'_>) -> Self {
        Self {
            flags: x.flags,
            origin: x.origin.into(),
            visibility: x.visibility.into(),
            deprecated: x.deprecated.map(Into::into),
            sort_text: x.sort_text.map(Into::into),
        }
    }
}

impl<R: Reason> From<&obr::decl_defs::SubstContext<'_>> for folded::SubstContext<R> {
    fn from(x: &obr::decl_defs::SubstContext<'_>) -> Self {
        Self {
            subst: folded::Subst(map(x.subst.iter())),
            class_context: x.class_context.into(),
            from_req_extends: x.from_req_extends,
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::TypeconstType<'_>> for folded::TypeConst<R> {
    fn from(x: &obr::typing_defs::TypeconstType<'_>) -> Self {
        Self {
            is_synthesized: x.synthesized,
            name: x.name.into(),
            kind: x.kind.into(),
            origin: x.origin.into(),
            enforceable: x.enforceable.into(),
            reifiable: x.reifiable.map(Into::into),
            is_concretized: x.concretized,
            is_ctx: x.is_ctx,
        }
    }
}

impl<R: Reason> From<&obr::typing_defs::ClassConst<'_>> for folded::ClassConst<R> {
    fn from(x: &obr::typing_defs::ClassConst<'_>) -> Self {
        Self {
            is_synthesized: x.synthesized,
            kind: x.abstract_,
            pos: x.pos.into(),
            ty: x.type_.into(),
            origin: x.origin.into(),
            refs: slice(x.refs),
        }
    }
}

impl<R: Reason> From<&obr::decl_defs::Requirement<'_>> for folded::Requirement<R> {
    fn from(req: &obr::decl_defs::Requirement<'_>) -> Self {
        Self {
            pos: req.0.into(),
            ty: req.1.into(),
        }
    }
}

impl From<(Option<&obr::decl_defs::Element<'_>>, ty::ConsistentKind)> for folded::Constructor {
    fn from(construct: (Option<&obr::decl_defs::Element<'_>>, ty::ConsistentKind)) -> Self {
        Self::new(construct.0.map(Into::into), construct.1)
    }
}

impl<P> From<obr::decl_defs::DeclError<'_>> for crate::decl_error::DeclError<P>
where
    P: for<'a> From<&'a obr::pos::Pos<'a>>,
{
    fn from(decl_error: obr::decl_defs::DeclError<'_>) -> Self {
        use obr::decl_defs::DeclError as Obr;
        match decl_error {
            Obr::WrongExtendKind {
                pos,
                kind,
                name,
                parent_pos,
                parent_kind,
                parent_name,
            } => Self::WrongExtendKind {
                pos: pos.into(),
                kind,
                name: name.into(),
                parent_pos: parent_pos.into(),
                parent_kind,
                parent_name: parent_name.into(),
            },
            Obr::WrongUseKind {
                pos,
                name,
                parent_pos,
                parent_name,
            } => Self::WrongUseKind {
                pos: pos.into(),
                name: name.into(),
                parent_pos: parent_pos.into(),
                parent_name: parent_name.into(),
            },
            Obr::CyclicClassDef { pos, stack } => {
                Self::CyclicClassDef(pos.into(), stack.iter().copied().map(Into::into).collect())
            }
        }
    }
}

impl<R: Reason> From<&obr::decl_defs::DeclClassType<'_>> for folded::FoldedClass<R> {
    fn from(cls: &obr::decl_defs::DeclClassType<'_>) -> Self {
        // Destructure to help ensure we convert every field. A couple fields
        // are ignored because they're redundant with other fields (and
        // `folded::FoldedClass` just omits the redundant fields).
        let obr::decl_defs::DeclClassType {
            name,
            pos,
            kind,
            abstract_: _, // `Self::is_abstract()` just reads the `kind` field
            final_,
            const_,
            internal,
            is_xhp,
            has_xhp_keyword,
            support_dynamic_type,
            module,
            is_module_level_trait,
            tparams,
            where_constraints,
            substs,
            ancestors,
            props,
            sprops,
            methods,
            smethods,
            consts,
            typeconsts,
            xhp_enum_values,
            xhp_marked_empty,
            construct,
            need_init: _, // `Self::has_concrete_constructor()` reads the `constructor` field
            deferred_init_members,
            req_ancestors,
            req_ancestors_extends,
            req_class_ancestors,
            extends,
            sealed_whitelist,
            xhp_attr_deps,
            enum_type,
            decl_errors,
            docs_url,
            allow_multiple_instantiations,
        } = cls;
        Self {
            name: (*name).into(),
            pos: (*pos).into(),
            kind: *kind,
            is_final: *final_,
            is_const: *const_,
            is_internal: *internal,
            is_xhp: *is_xhp,
            has_xhp_keyword: *has_xhp_keyword,
            support_dynamic_type: *support_dynamic_type,
            enum_type: enum_type.map(Into::into),
            module: module.map(Into::into),
            is_module_level_trait: *is_module_level_trait,
            tparams: slice(tparams),
            where_constraints: slice(where_constraints),
            substs: map(substs.iter()),
            ancestors: map(ancestors.iter()),
            props: map(props.iter()),
            static_props: map(sprops.iter()),
            methods: map(methods.iter()),
            static_methods: map(smethods.iter()),
            constructor: (*construct).into(),
            consts: map(consts.iter()),
            type_consts: map(typeconsts.iter()),
            xhp_enum_values: (xhp_enum_values.iter())
                .map(|(&s, &evs)| (s.into(), slice(evs)))
                .collect(),
            xhp_marked_empty: *xhp_marked_empty,
            extends: extends.iter().copied().map(Into::into).collect(),
            xhp_attr_deps: xhp_attr_deps.iter().copied().map(Into::into).collect(),
            req_ancestors: req_ancestors.iter().copied().map(Into::into).collect(),
            req_ancestors_extends: (req_ancestors_extends.iter())
                .copied()
                .map(Into::into)
                .collect(),
            req_class_ancestors: (req_class_ancestors.iter())
                .copied()
                .map(Into::into)
                .collect(),
            sealed_whitelist: (sealed_whitelist)
                .map(|l| l.iter().copied().map(Into::into).collect()),
            deferred_init_members: (deferred_init_members.iter())
                .copied()
                .map(Into::into)
                .collect(),
            decl_errors: slice(decl_errors),
            docs_url: docs_url.map(Into::into),
            allow_multiple_instantiations: *allow_multiple_instantiations,
        }
    }
}

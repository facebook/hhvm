// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{self, shallow, ty, DeclTy, DeclTy_, FunParam, FunType, UserAttribute};
use crate::reason::Reason;
use pos::{ConstName, FunName, Pos, Symbol, TypeName};

use oxidized_by_ref as obr;

#[inline]
fn slice<T: Copy, U>(items: &[T], f: impl Fn(T) -> U) -> Box<[U]> {
    items
        .iter()
        .copied()
        .map(f)
        .collect::<Vec<_>>()
        .into_boxed_slice()
}

fn xhp_enum_value(x: obr::ast_defs::XhpEnumValue<'_>) -> ty::XhpEnumValue {
    use obr::ast_defs::XhpEnumValue as Obr;
    use ty::XhpEnumValue;
    match x {
        Obr::XEVInt(i) => XhpEnumValue::XEVInt(i),
        Obr::XEVString(s) => XhpEnumValue::XEVString(Symbol::new(s)),
    }
}

fn decl_ifc_fun_decl(x: obr::typing_defs::IfcFunDecl<'_>) -> ty::IfcFunDecl {
    use obr::typing_defs_core::IfcFunDecl as Obr;
    use ty::IfcFunDecl;
    match x {
        Obr::FDPolicied(s) => IfcFunDecl::FDPolicied(s.map(Symbol::new)),
        Obr::FDInferFlows => IfcFunDecl::FDInferFlows,
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
            TshapeFieldName::TSFlitInt(Symbol::new(pos_id.1)),
        ),
        Obr::TSFlitStr(&pos_bytes) => (
            SfnPos::Simple(pos_bytes.0.into()),
            TshapeFieldName::TSFlitStr(pos_bytes.1.into()),
        ),
        Obr::TSFclassConst(&(pos_id1, pos_id2)) => (
            SfnPos::ClassConst(pos_id1.0.into(), pos_id2.0.into()),
            TshapeFieldName::TSFclassConst(
                TypeName(Symbol::new(pos_id1.1)),
                Symbol::new(pos_id2.1),
            ),
        ),
    }
}

fn user_attribute_from_decl<P: Pos>(
    attr: &obr::typing_defs::UserAttribute<'_>,
) -> UserAttribute<P> {
    UserAttribute {
        name: attr.name.into(),
        classname_params: attr
            .classname_params
            .iter()
            .copied()
            .map(TypeName::new)
            .collect(),
    }
}

fn decl_tparam<R: Reason>(tparam: &obr::typing_defs::Tparam<'_>) -> ty::Tparam<R, DeclTy<R>> {
    ty::Tparam {
        variance: tparam.variance,
        name: tparam.name.into(),
        tparams: slice(tparam.tparams, decl_tparam),
        constraints: slice(tparam.constraints, |(kind, ty)| (kind, ty_from_decl(ty))),
        reified: tparam.reified,
        user_attributes: slice(tparam.user_attributes, user_attribute_from_decl),
    }
}

fn decl_where_constraint<R: Reason>(
    x: &obr::typing_defs::WhereConstraint<'_>,
) -> ty::WhereConstraint<DeclTy<R>> {
    ty::WhereConstraint(ty_from_decl(x.0), x.1, ty_from_decl(x.2))
}

fn decl_shape_field_type<R: Reason>(
    field_name_pos: ty::ShapeFieldNamePos<R::Pos>,
    sft: &obr::typing_defs::ShapeFieldType<'_>,
) -> ty::ShapeFieldType<R> {
    ty::ShapeFieldType {
        field_name_pos,
        optional: sft.optional,
        ty: ty_from_decl(sft.ty),
    }
}

fn ty_from_decl<R: Reason>(ty: &obr::typing_defs::Ty<'_>) -> DeclTy<R> {
    use obr::typing_defs_core::Ty_::*;
    use DeclTy_::*;
    let reason = R::from(*ty.0);
    let ty_ = match ty.1 {
        Tthis => DTthis,
        Tapply(&(pos_id, tys)) => DTapply(Box::new((pos_id.into(), slice(tys, ty_from_decl)))),
        Tmixed => DTmixed,
        Tlike(ty) => DTlike(ty_from_decl(ty)),
        Tany(_) => DTany,
        Terr => DTerr,
        Tnonnull => DTnonnull,
        Tdynamic => DTdynamic,
        Toption(ty) => DToption(ty_from_decl(ty)),
        Tprim(prim) => DTprim(*prim),
        Tfun(ft) => DTfun(Box::new(decl_fun_type(ft))),
        Ttuple(tys) => DTtuple(slice(tys, ty_from_decl)),
        Tshape(&(kind, fields)) => DTshape(Box::new((
            kind,
            fields
                .iter()
                .map(|(name, ty)| {
                    let (field_name_pos, name) = tshape_field_name_from_decl(name.0);
                    (name, decl_shape_field_type(field_name_pos, ty))
                })
                .collect(),
        ))),
        Tvar(ident) => DTvar(ident.into()),
        Tgeneric(&(pos_id, tys)) => {
            DTgeneric(Box::new((TypeName::new(pos_id), slice(tys, ty_from_decl))))
        }
        Tunion(tys) => DTunion(slice(tys, ty_from_decl)),
        Tintersection(tys) => DTintersection(slice(tys, ty_from_decl)),
        TvecOrDict((ty1, ty2)) => DTvecOrDict(Box::new((ty_from_decl(ty1), ty_from_decl(ty2)))),
        Taccess(taccess_type) => DTaccess(Box::new(decl_taccess_type(taccess_type))),
        TunappliedAlias(_) | Tnewtype(_) | Tdependent(_) | Tclass(_) | Tneg(_) => {
            unreachable!("Not used in decl tys")
        }
    };
    DeclTy::new(reason, ty_)
}

fn decl_taccess_type<R: Reason>(
    taccess_type: &obr::typing_defs::TaccessType<'_>,
) -> ty::TaccessType<R, DeclTy<R>> {
    ty::TaccessType {
        ty: ty_from_decl(taccess_type.0),
        type_const: taccess_type.1.into(),
    }
}

fn decl_capability<R: Reason>(
    cap: obr::typing_defs::Capability<'_>,
) -> ty::Capability<R, DeclTy<R>> {
    use obr::typing_defs_core::Capability as Obr;
    use ty::Capability;
    match cap {
        Obr::CapDefaults(pos) => Capability::CapDefaults(pos.into()),
        Obr::CapTy(ty) => Capability::CapTy(ty_from_decl(ty)),
    }
}

fn decl_fun_implicit_params<R: Reason>(
    x: &obr::typing_defs::FunImplicitParams<'_>,
) -> ty::FunImplicitParams<R, DeclTy<R>> {
    ty::FunImplicitParams {
        capability: decl_capability(x.capability),
    }
}

fn decl_fun_type<R: Reason>(ft: &obr::typing_defs::FunType<'_>) -> FunType<R, DeclTy<R>> {
    FunType {
        tparams: slice(ft.tparams, decl_tparam),
        where_constraints: slice(ft.where_constraints, decl_where_constraint),
        params: slice(ft.params, decl_fun_param),
        implicit_params: decl_fun_implicit_params(ft.implicit_params),
        ret: decl_possibly_enforced_ty(ft.ret),
        flags: ft.flags,
        ifc_decl: decl_ifc_fun_decl(ft.ifc_decl),
    }
}

fn decl_possibly_enforced_ty<R: Reason>(
    ty: &obr::typing_defs_core::PossiblyEnforcedTy<'_>,
) -> decl_defs::ty::PossiblyEnforcedTy<DeclTy<R>> {
    decl_defs::ty::PossiblyEnforcedTy {
        ty: ty_from_decl(ty.type_),
        enforced: ty.enforced,
    }
}

fn decl_fun_param<R: Reason>(fp: &obr::typing_defs_core::FunParam<'_>) -> FunParam<R, DeclTy<R>> {
    FunParam {
        pos: fp.pos.into(),
        name: fp.name.map(Symbol::new),
        ty: decl_possibly_enforced_ty(fp.type_),
        flags: fp.flags,
    }
}

fn class_const_from(x: obr::typing_defs::ClassConstFrom<'_>) -> ty::ClassConstFrom {
    use obr::typing_defs::ClassConstFrom as Obr;
    use ty::ClassConstFrom;
    match x {
        Obr::Self_ => ClassConstFrom::Self_,
        Obr::From(s) => ClassConstFrom::From(TypeName::new(s)),
    }
}

fn class_const_ref(x: obr::typing_defs::ClassConstRef<'_>) -> ty::ClassConstRef {
    ty::ClassConstRef(class_const_from(x.0), Symbol::new(x.1))
}

fn abstract_typeconst<R: Reason>(
    x: &obr::typing_defs::AbstractTypeconst<'_>,
) -> ty::AbstractTypeconst<R> {
    ty::AbstractTypeconst {
        as_constraint: x.as_constraint.as_ref().map(|ty| ty_from_decl(ty)),
        super_constraint: x.super_constraint.as_ref().map(|ty| ty_from_decl(ty)),
        default: x.default.as_ref().map(|ty| ty_from_decl(ty)),
    }
}

fn concrete_typeconst<R: Reason>(
    x: &obr::typing_defs::ConcreteTypeconst<'_>,
) -> ty::ConcreteTypeconst<R> {
    ty::ConcreteTypeconst {
        ty: ty_from_decl(x.tc_type),
    }
}

fn typeconst<R: Reason>(x: obr::typing_defs::Typeconst<'_>) -> ty::Typeconst<R> {
    use obr::typing_defs::Typeconst as Obr;
    use ty::Typeconst;
    match x {
        Obr::TCAbstract(atc) => Typeconst::TCAbstract(abstract_typeconst(atc)),
        Obr::TCConcrete(ctc) => Typeconst::TCConcrete(concrete_typeconst(ctc)),
    }
}

fn enum_type<R: Reason>(x: &obr::typing_defs::EnumType<'_>) -> ty::EnumType<R> {
    ty::EnumType {
        base: ty_from_decl(x.base),
        constraint: x.constraint.as_ref().map(|ty| ty_from_decl(ty)),
        includes: slice(x.includes, ty_from_decl),
    }
}

fn shallow_class_const<R: Reason>(
    scc: &obr::shallow_decl_defs::ShallowClassConst<'_>,
) -> shallow::ShallowClassConst<R> {
    shallow::ShallowClassConst {
        kind: scc.abstract_,
        name: scc.name.into(),
        ty: ty_from_decl(scc.type_),
        refs: slice(scc.refs, class_const_ref),
    }
}

fn shallow_typeconst<R: Reason>(
    stc: &obr::shallow_decl_defs::ShallowTypeconst<'_>,
) -> shallow::ShallowTypeconst<R> {
    shallow::ShallowTypeconst {
        name: stc.name.into(),
        kind: typeconst(stc.kind),
        enforceable: if stc.enforceable.1 {
            Some(stc.enforceable.0.into())
        } else {
            None
        },
        reifiable: stc.reifiable.map(Into::into),
        is_ctx: stc.is_ctx,
    }
}

fn shallow_method<R: Reason>(
    sm: &obr::shallow_decl_defs::ShallowMethod<'_>,
) -> shallow::ShallowMethod<R> {
    shallow::ShallowMethod {
        name: sm.name.into(),
        ty: ty_from_decl(sm.type_),
        visibility: sm.visibility,
        deprecated: sm.deprecated.map(Into::into),
        attributes: slice(sm.attributes, user_attribute_from_decl),
        flags: shallow::MethodFlags::empty(),
    }
}

fn shallow_prop<R: Reason>(
    sp: &obr::shallow_decl_defs::ShallowProp<'_>,
) -> shallow::ShallowProp<R> {
    shallow::ShallowProp {
        name: sp.name.into(),
        xhp_attr: sp.xhp_attr,
        ty: sp.type_.as_ref().map(|ty| ty_from_decl(ty)),
        visibility: sp.visibility,
        flags: sp.flags,
    }
}

fn shallow_class<R: Reason>(
    sc: &obr::shallow_decl_defs::ClassDecl<'_>,
) -> shallow::ShallowClass<R> {
    shallow::ShallowClass {
        mode: sc.mode,
        is_final: sc.final_,
        is_abstract: sc.abstract_,
        is_xhp: sc.is_xhp,
        has_xhp_keyword: sc.has_xhp_keyword,
        kind: sc.kind,
        module: sc.module.map(Into::into),
        name: sc.name.into(),
        tparams: slice(sc.tparams, decl_tparam),
        where_constraints: slice(sc.where_constraints, decl_where_constraint),
        extends: slice(sc.extends, ty_from_decl),
        uses: slice(sc.uses, ty_from_decl),
        xhp_attr_uses: slice(sc.xhp_attr_uses, ty_from_decl),
        xhp_enum_values: sc
            .xhp_enum_values
            .iter()
            .map(|(&k, v)| (Symbol::new(k), slice(v, xhp_enum_value)))
            .collect(),
        req_extends: slice(sc.req_extends, ty_from_decl),
        req_implements: slice(sc.req_implements, ty_from_decl),
        implements: slice(sc.implements, ty_from_decl),
        support_dynamic_type: sc.support_dynamic_type,
        consts: slice(sc.consts, shallow_class_const),
        typeconsts: slice(sc.typeconsts, shallow_typeconst),
        props: slice(sc.props, shallow_prop),
        static_props: slice(sc.sprops, shallow_prop),
        constructor: sc.constructor.map(|ctor| shallow_method(ctor)),
        static_methods: slice(sc.static_methods, shallow_method),
        methods: slice(sc.methods, shallow_method),
        user_attributes: slice(sc.user_attributes, user_attribute_from_decl),
        enum_type: sc.enum_type.as_ref().map(|et| enum_type(et)),
    }
}

fn fun_decl<R: Reason>(sf: &obr::shallow_decl_defs::FunDecl<'_>) -> shallow::FunDecl<R> {
    shallow::FunDecl {
        pos: sf.pos.into(),
        ty: ty_from_decl(sf.type_),
        deprecated: sf.deprecated.map(Into::into),
        module: sf.module.map(Into::into),
        internal: sf.internal,
        php_std_lib: sf.php_std_lib,
        support_dynamic_type: sf.support_dynamic_type,
    }
}

fn typedef_decl<R: Reason>(x: &obr::shallow_decl_defs::TypedefDecl<'_>) -> shallow::TypedefDecl<R> {
    ty::TypedefType {
        module: x.module.map(Into::into),
        pos: x.pos.into(),
        vis: x.vis,
        tparams: slice(x.tparams, decl_tparam),
        constraint: x.constraint.map(|ty| ty_from_decl(ty)),
        ty: ty_from_decl(x.type_),
        is_ctx: x.is_ctx,
        attributes: slice(x.attributes, user_attribute_from_decl),
    }
}

fn const_decl<R: Reason>(x: &obr::shallow_decl_defs::ConstDecl<'_>) -> shallow::ConstDecl<R> {
    ty::ConstDecl {
        pos: x.pos.into(),
        ty: ty_from_decl(x.type_),
    }
}

impl<R: Reason> From<(&str, obr::shallow_decl_defs::Decl<'_>)> for shallow::Decl<R> {
    fn from(decl: (&str, obr::shallow_decl_defs::Decl<'_>)) -> shallow::Decl<R> {
        use obr::shallow_decl_defs::Decl as Obr;
        use shallow::Decl;
        match decl {
            (name, Obr::Class(x)) => Decl::Class(TypeName::new(name), shallow_class(x)),
            (name, Obr::Fun(x)) => Decl::Fun(FunName::new(name), fun_decl(x)),
            (name, Obr::Typedef(x)) => Decl::Typedef(TypeName(Symbol::new(name)), typedef_decl(x)),
            (name, Obr::Const(x)) => Decl::Const(ConstName(Symbol::new(name)), const_decl(x)),
        }
    }
}

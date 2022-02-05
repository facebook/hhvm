// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Allocator;
use crate::decl_defs::{self, shallow, ty, DeclTy, DeclTy_, FunParam, FunType, UserAttribute};
use crate::reason::Reason;
use oxidized_by_ref as obr;
use pos::TypeName;

impl<R: Reason> Allocator<R> {
    #[inline]
    fn vec<T: Copy, U>(&self, items: &[T], f: impl Fn(&Self, T) -> U) -> Vec<U> {
        items.iter().copied().map(|x| f(self, x)).collect()
    }

    fn xhp_enum_value(&self, x: obr::ast_defs::XhpEnumValue<'_>) -> ty::XhpEnumValue {
        use obr::ast_defs::XhpEnumValue as Obr;
        use ty::XhpEnumValue;
        match x {
            Obr::XEVInt(i) => XhpEnumValue::XEVInt(i),
            Obr::XEVString(s) => XhpEnumValue::XEVString(self.symbol(s)),
        }
    }

    fn decl_ifc_fun_decl(&self, x: obr::typing_defs::IfcFunDecl<'_>) -> ty::IfcFunDecl {
        use obr::typing_defs_core::IfcFunDecl as Obr;
        use ty::IfcFunDecl;
        match x {
            Obr::FDPolicied(s) => IfcFunDecl::FDPolicied(s.map(|s| self.symbol(s))),
            Obr::FDInferFlows => IfcFunDecl::FDInferFlows,
        }
    }

    fn tshape_field_name_from_decl(
        &self,
        x: obr::typing_defs::TshapeFieldName<'_>,
    ) -> ty::TshapeFieldName {
        use obr::typing_defs_core::TshapeFieldName as Obr;
        use ty::TshapeFieldName;
        match x {
            Obr::TSFlitInt(&pos_id) => TshapeFieldName::TSFlitInt(self.symbol(pos_id.1)),
            Obr::TSFlitStr(&pos_bytes) => TshapeFieldName::TSFlitStr(self.bytes(pos_bytes.1)),
            Obr::TSFclassConst(&(pos_id1, pos_id2)) => TshapeFieldName::TSFclassConst(
                TypeName(self.symbol(pos_id1.1)),
                self.symbol(pos_id2.1),
            ),
        }
    }

    fn user_attribute_from_decl(
        &self,
        attr: &obr::typing_defs::UserAttribute<'_>,
    ) -> UserAttribute<R::Pos> {
        UserAttribute {
            name: self.pos_classname_from_decl(attr.name),
            classname_params: attr
                .classname_params
                .iter()
                .map(|param| TypeName(self.symbol(param)))
                .collect(),
        }
    }

    fn decl_tparam(&self, tparam: &obr::typing_defs::Tparam<'_>) -> ty::Tparam<R, DeclTy<R>> {
        ty::Tparam {
            variance: tparam.variance,
            name: self.pos_classname_from_decl(tparam.name),
            tparams: self.vec(tparam.tparams, Self::decl_tparam),
            constraints: self.vec(tparam.constraints, |alloc, (kind, ty)| {
                (kind, alloc.ty_from_decl(ty))
            }),
            reified: tparam.reified,
            user_attributes: self.vec(tparam.user_attributes, Self::user_attribute_from_decl),
        }
    }

    fn decl_where_constraint(
        &self,
        x: &obr::typing_defs::WhereConstraint<'_>,
    ) -> ty::WhereConstraint<DeclTy<R>> {
        ty::WhereConstraint(self.ty_from_decl(x.0), x.1, self.ty_from_decl(x.2))
    }

    fn decl_shape_field_type(
        &self,
        x: &obr::typing_defs::ShapeFieldType<'_>,
    ) -> ty::ShapeFieldType<R> {
        ty::ShapeFieldType {
            optional: x.optional,
            ty: self.ty_from_decl(x.ty),
        }
    }

    #[inline]
    pub fn decl_ty(&self, reason: R, ty: DeclTy_<R>) -> DeclTy<R> {
        DeclTy::new(reason, self.decl_tys.mk(ty))
    }

    fn ty_from_decl(&self, ty: &obr::typing_defs::Ty<'_>) -> DeclTy<R> {
        use obr::typing_defs_core::Ty_::*;
        use DeclTy_::*;
        let reason = self.reason(ty.0);
        let ty_ = match ty.1 {
            Tthis => DTthis,
            Tapply(&(pos_id, tys)) => DTapply(
                self.pos_classname_from_decl(pos_id),
                self.vec(tys, Self::ty_from_decl),
            ),
            Tmixed => DTmixed,
            Tlike(ty) => DTlike(self.ty_from_decl(ty)),
            Tany(_) => DTany,
            Terr => DTerr,
            Tnonnull => DTnonnull,
            Tdynamic => DTdynamic,
            Toption(ty) => DToption(self.ty_from_decl(ty)),
            Tprim(prim) => DTprim(*prim),
            Tfun(ft) => DTfun(self.decl_fun_type(ft)),
            Ttuple(tys) => DTtuple(self.vec(tys, Self::ty_from_decl)),
            Tshape(&(kind, fields)) => DTshape(
                kind,
                fields
                    .iter()
                    .map(|(name, ty)| {
                        (
                            self.tshape_field_name_from_decl(name.0),
                            self.decl_shape_field_type(ty),
                        )
                    })
                    .collect(),
            ),
            Tvar(ident) => DTvar(ident.into()),
            Tgeneric(&(pos_id, tys)) => DTgeneric(
                TypeName(self.symbol(pos_id)),
                self.vec(tys, Self::ty_from_decl),
            ),
            Tunion(tys) => DTunion(self.vec(tys, Self::ty_from_decl)),
            Tintersection(tys) => DTintersection(self.vec(tys, Self::ty_from_decl)),
            TvecOrDict((ty1, ty2)) => DTvecOrDict(self.ty_from_decl(ty1), self.ty_from_decl(ty2)),
            Taccess(taccess_type) => DTaccess(self.decl_taccess_type(taccess_type)),
            TunappliedAlias(_) | Tnewtype(_) | Tdependent(_) | Tclass(_) | Tneg(_) => {
                unreachable!("Not used in decl tys")
            }
        };
        self.decl_ty(reason, ty_)
    }

    fn decl_taccess_type(
        &self,
        taccess_type: &obr::typing_defs::TaccessType<'_>,
    ) -> ty::TaccessType<R, DeclTy<R>> {
        ty::TaccessType(
            self.ty_from_decl(taccess_type.0),
            self.pos_id_from_decl(taccess_type.1),
        )
    }

    fn decl_capability(
        &self,
        cap: obr::typing_defs::Capability<'_>,
    ) -> ty::Capability<R, DeclTy<R>> {
        use obr::typing_defs_core::Capability as Obr;
        use ty::Capability;
        match cap {
            Obr::CapDefaults(pos) => Capability::CapDefaults(self.pos_from_decl(pos)),
            Obr::CapTy(ty) => Capability::CapTy(self.ty_from_decl(ty)),
        }
    }

    fn decl_fun_implicit_params(
        &self,
        x: &obr::typing_defs::FunImplicitParams<'_>,
    ) -> ty::FunImplicitParams<R, DeclTy<R>> {
        ty::FunImplicitParams {
            capability: self.decl_capability(x.capability),
        }
    }

    fn decl_fun_type(&self, ft: &obr::typing_defs::FunType<'_>) -> FunType<R, DeclTy<R>> {
        FunType {
            arity: self.decl_fun_arity(ft.arity),
            tparams: self.vec(ft.tparams, Self::decl_tparam),
            where_constraints: self.vec(ft.where_constraints, Self::decl_where_constraint),
            params: self.vec(ft.params, Self::decl_fun_param),
            implicit_params: self.decl_fun_implicit_params(ft.implicit_params),
            ret: self.decl_possibly_enforced_ty(ft.ret),
            flags: ft.flags,
            ifc_decl: self.decl_ifc_fun_decl(ft.ifc_decl),
        }
    }

    fn decl_fun_arity(&self, x: obr::typing_defs::FunArity<'_>) -> ty::FunArity<R, DeclTy<R>> {
        use obr::typing_defs_core::FunArity as Obr;
        use ty::FunArity;
        match x {
            Obr::Fstandard => FunArity::Fstandard,
            Obr::Fvariadic(param) => FunArity::Fvariadic(self.decl_fun_param(param)),
        }
    }

    fn decl_possibly_enforced_ty(
        &self,
        ty: &obr::typing_defs_core::PossiblyEnforcedTy<'_>,
    ) -> decl_defs::ty::PossiblyEnforcedTy<DeclTy<R>> {
        decl_defs::ty::PossiblyEnforcedTy {
            ty: self.ty_from_decl(ty.type_),
            enforced: ty.enforced,
        }
    }

    fn decl_fun_param(&self, fp: &obr::typing_defs_core::FunParam<'_>) -> FunParam<R, DeclTy<R>> {
        FunParam {
            pos: self.pos_from_decl(fp.pos),
            name: fp.name.map(|name| self.symbol(name)),
            ty: self.decl_possibly_enforced_ty(fp.type_),
            flags: fp.flags,
        }
    }

    fn class_const_from(&self, x: obr::typing_defs::ClassConstFrom<'_>) -> ty::ClassConstFrom {
        use obr::typing_defs::ClassConstFrom as Obr;
        use ty::ClassConstFrom;
        match x {
            Obr::Self_ => ClassConstFrom::Self_,
            Obr::From(s) => ClassConstFrom::From(TypeName(self.symbol(s))),
        }
    }

    fn class_const_ref(&self, x: obr::typing_defs::ClassConstRef<'_>) -> ty::ClassConstRef {
        ty::ClassConstRef(self.class_const_from(x.0), self.symbol(x.1))
    }

    fn abstract_typeconst(
        &self,
        x: &obr::typing_defs::AbstractTypeconst<'_>,
    ) -> ty::AbstractTypeconst<R> {
        ty::AbstractTypeconst {
            as_constraint: x.as_constraint.as_ref().map(|ty| self.ty_from_decl(ty)),
            super_constraint: x.super_constraint.as_ref().map(|ty| self.ty_from_decl(ty)),
            default: x.default.as_ref().map(|ty| self.ty_from_decl(ty)),
        }
    }

    fn concrete_typeconst(
        &self,
        x: &obr::typing_defs::ConcreteTypeconst<'_>,
    ) -> ty::ConcreteTypeconst<R> {
        ty::ConcreteTypeconst {
            ty: self.ty_from_decl(x.tc_type),
        }
    }

    fn typeconst(&self, x: obr::typing_defs::Typeconst<'_>) -> ty::Typeconst<R> {
        use obr::typing_defs::Typeconst as Obr;
        use ty::Typeconst;
        match x {
            Obr::TCAbstract(atc) => Typeconst::TCAbstract(self.abstract_typeconst(atc)),
            Obr::TCConcrete(ctc) => Typeconst::TCConcrete(self.concrete_typeconst(ctc)),
        }
    }

    fn enum_type(&self, x: &obr::typing_defs::EnumType<'_>) -> ty::EnumType<R> {
        ty::EnumType {
            base: self.ty_from_decl(x.base),
            constraint: x.constraint.as_ref().map(|ty| self.ty_from_decl(ty)),
            includes: self.vec(x.includes, Self::ty_from_decl),
        }
    }

    fn shallow_class_const(
        &self,
        scc: &obr::shallow_decl_defs::ShallowClassConst<'_>,
    ) -> shallow::ShallowClassConst<R> {
        shallow::ShallowClassConst {
            is_abstract: scc.abstract_,
            name: self.pos_id_from_decl(scc.name),
            ty: self.ty_from_decl(scc.type_),
            refs: self.vec(scc.refs, Self::class_const_ref),
        }
    }

    fn shallow_typeconst(
        &self,
        stc: &obr::shallow_decl_defs::ShallowTypeconst<'_>,
    ) -> shallow::ShallowTypeconst<R> {
        shallow::ShallowTypeconst {
            name: self.pos_id_from_decl(stc.name),
            kind: self.typeconst(stc.kind),
            enforceable: (self.pos_from_decl(stc.enforceable.0), stc.enforceable.1),
            reifiable: stc.reifiable.map(|pos| self.pos_from_decl(pos)),
            is_ctx: stc.is_ctx,
        }
    }

    fn shallow_method(
        &self,
        sm: &obr::shallow_decl_defs::ShallowMethod<'_>,
    ) -> shallow::ShallowMethod<R> {
        shallow::ShallowMethod {
            name: self.pos_id_from_decl(sm.name),
            ty: self.ty_from_decl(sm.type_),
            visibility: sm.visibility,
            deprecated: sm.deprecated.map(|s| self.bytes(s)),
            attributes: self.vec(sm.attributes, Self::user_attribute_from_decl),
            flags: shallow::MethodFlags::empty(),
        }
    }

    fn shallow_prop(
        &self,
        sp: &obr::shallow_decl_defs::ShallowProp<'_>,
    ) -> shallow::ShallowProp<R> {
        shallow::ShallowProp {
            name: self.pos_id_from_decl(sp.name),
            xhp_attr: sp.xhp_attr,
            ty: sp.type_.as_ref().map(|ty| self.ty_from_decl(ty)),
            visibility: sp.visibility,
            flags: sp.flags,
        }
    }

    pub fn shallow_class(
        &self,
        sc: &obr::shallow_decl_defs::ClassDecl<'_>,
    ) -> shallow::ShallowClass<R> {
        shallow::ShallowClass {
            mode: sc.mode,
            is_final: sc.final_,
            is_abstract: sc.abstract_,
            is_xhp: sc.is_xhp,
            has_xhp_keyword: sc.has_xhp_keyword,
            kind: sc.kind,
            module: sc.module.as_ref().map(|id| self.pos_id_from_ast_ref(id)),
            name: self.pos_classname_from_decl(sc.name),
            tparams: self.vec(sc.tparams, Self::decl_tparam),
            where_constraints: self.vec(sc.where_constraints, Self::decl_where_constraint),
            extends: self.vec(sc.extends, Self::ty_from_decl),
            uses: self.vec(sc.uses, Self::ty_from_decl),
            xhp_attr_uses: self.vec(sc.xhp_attr_uses, Self::ty_from_decl),
            xhp_enum_values: sc
                .xhp_enum_values
                .iter()
                .map(|(k, v)| (self.symbol(k), self.vec(v, Self::xhp_enum_value)))
                .collect(),
            req_extends: self.vec(sc.req_extends, Self::ty_from_decl),
            req_implements: self.vec(sc.req_implements, Self::ty_from_decl),
            implements: self.vec(sc.implements, Self::ty_from_decl),
            support_dynamic_type: sc.support_dynamic_type,
            consts: self.vec(sc.consts, Self::shallow_class_const),
            typeconsts: self.vec(sc.typeconsts, Self::shallow_typeconst),
            props: self.vec(sc.props, Self::shallow_prop),
            static_props: self.vec(sc.sprops, Self::shallow_prop),
            constructor: sc.constructor.map(|ctor| self.shallow_method(ctor)),
            static_methods: self.vec(sc.static_methods, Self::shallow_method),
            methods: self.vec(sc.methods, Self::shallow_method),
            user_attributes: self.vec(sc.user_attributes, Self::user_attribute_from_decl),
            enum_type: sc.enum_type.as_ref().map(|et| self.enum_type(et)),
        }
    }

    pub fn fun_decl(&self, sf: &obr::shallow_decl_defs::FunDecl<'_>) -> shallow::FunDecl<R> {
        shallow::FunDecl {
            pos: self.pos_from_decl(sf.pos),
            ty: self.ty_from_decl(sf.type_),
            deprecated: sf.deprecated.map(|s| self.bytes(s)),
            module: sf.module.map(|pos_id| self.pos_id_from_ast_ref(&pos_id)),
            internal: sf.internal,
            php_std_lib: sf.php_std_lib,
            support_dynamic_type: sf.support_dynamic_type,
        }
    }
}

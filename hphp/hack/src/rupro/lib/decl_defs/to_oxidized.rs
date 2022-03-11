// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{folded, ty::*};
use crate::folded_decl_provider::Subst;
use crate::reason::Reason;
use oxidized_by_ref::{ast::Id, s_set::SSet};
use pos::{Pos, ToOxidized};

use oxidized_by_ref as obr;

impl<'a> ToOxidized<'a> for CeVisibility {
    type Output = obr::typing_defs::CeVisibility<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::typing_defs::CeVisibility as Obr;
        match self {
            CeVisibility::Public => Obr::Vpublic,
            CeVisibility::Private(v) => Obr::Vprivate(v.to_oxidized(arena)),
            CeVisibility::Protected(v) => Obr::Vprotected(v.to_oxidized(arena)),
            CeVisibility::Internal(v) => Obr::Vinternal(v.to_oxidized(arena)),
        }
    }
}

impl<'a> ToOxidized<'a> for IfcFunDecl {
    type Output = obr::typing_defs::IfcFunDecl<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::typing_defs::IfcFunDecl as Obr;
        match self {
            IfcFunDecl::FDPolicied(x) => Obr::FDPolicied(x.to_oxidized(arena)),
            IfcFunDecl::FDInferFlows => Obr::FDInferFlows,
        }
    }
}

impl<'a, P: Pos> ToOxidized<'a> for UserAttribute<P> {
    type Output = &'a obr::typing_defs::UserAttribute<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::UserAttribute {
            name: self.name.to_oxidized(arena),
            classname_params: self.classname_params.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for Tparam<R, DeclTy<R>> {
    type Output = &'a obr::typing_defs::Tparam<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::Tparam {
            variance: self.variance,
            name: self.name.to_oxidized(arena),
            tparams: self.tparams.to_oxidized(arena),
            constraints: arena.alloc_slice_fill_iter(
                self.constraints
                    .iter()
                    .map(|(x, y)| (*x, y.to_oxidized(arena))),
            ),
            reified: self.reified,
            user_attributes: self.user_attributes.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for WhereConstraint<DeclTy<R>> {
    type Output = &'a obr::typing_defs::WhereConstraint<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        let WhereConstraint(tvar_ty, kind, as_ty) = self;
        arena.alloc(obr::typing_defs::WhereConstraint(
            tvar_ty.to_oxidized(arena),
            *kind,
            as_ty.to_oxidized(arena),
        ))
    }
}

impl<'a, R: Reason> ToOxidized<'a> for DeclTy<R> {
    type Output = &'a obr::typing_defs::Ty<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::Ty(
            arena.alloc(self.reason().to_oxidized(arena)),
            self.node().to_oxidized(arena),
        ))
    }
}

impl<'a, R: Reason> ToOxidized<'a> for ShapeFieldType<R> {
    type Output = &'a obr::typing_defs::ShapeFieldType<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::ShapeFieldType {
            optional: self.optional,
            ty: self.ty.to_oxidized(arena),
        })
    }
}

fn oxidize_shape_field_name<'a, P: Pos>(
    arena: &'a bumpalo::Bump,
    name: TshapeFieldName,
    field_name_pos: &ShapeFieldNamePos<P>,
) -> obr::typing_defs::TshapeFieldName<'a> {
    use obr::typing_defs::TshapeFieldName as Obr;
    use ShapeFieldNamePos as SfnPos;
    let simple_pos = || match field_name_pos {
        SfnPos::Simple(p) => p.to_oxidized(arena),
        SfnPos::ClassConst(..) => panic!("expected ShapeFieldNamePos::Simple"),
    };
    match name {
        TshapeFieldName::TSFlitInt(x) => Obr::TSFlitInt(arena.alloc(obr::typing_defs::PosString(
            simple_pos(),
            x.to_oxidized(arena),
        ))),
        TshapeFieldName::TSFlitStr(x) => Obr::TSFlitStr(arena.alloc(
            obr::typing_defs::PosByteString(simple_pos(), x.to_oxidized(arena).into()),
        )),
        TshapeFieldName::TSFclassConst(cls, name) => {
            let (pos1, pos2) = match field_name_pos {
                SfnPos::ClassConst(p1, p2) => (p1.to_oxidized(arena), p2.to_oxidized(arena)),
                SfnPos::Simple(..) => panic!("expected ShapeFieldNamePos::ClassConst"),
            };
            Obr::TSFclassConst(arena.alloc((
                (pos1, cls.to_oxidized(arena)),
                obr::typing_defs::PosString(pos2, name.to_oxidized(arena)),
            )))
        }
    }
}

impl<'a, R: Reason> ToOxidized<'a> for DeclTy_<R> {
    type Output = obr::typing_defs::Ty_<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::t_shape_map::{TShapeField, TShapeMap};
        use obr::typing_defs::Ty_;
        match self {
            DeclTy_::DTthis => Ty_::Tthis,
            DeclTy_::DTapply(x) => Ty_::Tapply(x.to_oxidized(arena)),
            DeclTy_::DTmixed => Ty_::Tmixed,
            DeclTy_::DTlike(x) => Ty_::Tlike(x.to_oxidized(arena)),
            DeclTy_::DTany => Ty_::Tany(obr::tany_sentinel::TanySentinel),
            DeclTy_::DTerr => Ty_::Terr,
            DeclTy_::DTnonnull => Ty_::Tnonnull,
            DeclTy_::DTdynamic => Ty_::Tdynamic,
            DeclTy_::DToption(x) => Ty_::Toption(x.to_oxidized(arena)),
            DeclTy_::DTprim(x) => Ty_::Tprim(arena.alloc(*x)),
            DeclTy_::DTfun(x) => Ty_::Tfun(x.to_oxidized(arena)),
            DeclTy_::DTtuple(x) => Ty_::Ttuple(x.to_oxidized(arena)),
            DeclTy_::DTshape(shape) => {
                let mut shape_fields = arena_collections::AssocListMut::new_in(arena);
                let (shape_kind, shape_field_type_map): &(_, _) = shape;
                for (k, v) in shape_field_type_map.iter() {
                    let k = oxidize_shape_field_name(arena, *k, &v.field_name_pos);
                    shape_fields.insert_or_replace(TShapeField(k), v.to_oxidized(arena));
                }
                Ty_::Tshape(arena.alloc((*shape_kind, TShapeMap::from(shape_fields))))
            }
            DeclTy_::DTvar(ident) => Ty_::Tvar((*ident).into()),
            DeclTy_::DTgeneric(x) => Ty_::Tgeneric(x.to_oxidized(arena)),
            DeclTy_::DTunion(x) => Ty_::Tunion(x.to_oxidized(arena)),
            DeclTy_::DTintersection(x) => Ty_::Tintersection(x.to_oxidized(arena)),
            DeclTy_::DTvecOrDict(x) => Ty_::TvecOrDict(x.to_oxidized(arena)),
            DeclTy_::DTaccess(x) => Ty_::Taccess(x.to_oxidized(arena)),
        }
    }
}

impl<'a, R: Reason> ToOxidized<'a> for TaccessType<R, DeclTy<R>> {
    type Output = &'a obr::typing_defs::TaccessType<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::TaccessType(
            self.ty.to_oxidized(arena),
            self.type_const.to_oxidized(arena),
        ))
    }
}

impl<'a, R: Reason> ToOxidized<'a> for FunImplicitParams<R, DeclTy<R>> {
    type Output = &'a obr::typing_defs::FunImplicitParams<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::FunImplicitParams {
            capability: match &self.capability {
                Capability::CapDefaults(p) => {
                    obr::typing_defs::Capability::CapDefaults(p.to_oxidized(arena))
                }
                Capability::CapTy(ty) => obr::typing_defs::Capability::CapTy(ty.to_oxidized(arena)),
            },
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for FunType<R, DeclTy<R>> {
    type Output = &'a obr::typing_defs::FunType<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::FunType {
            tparams: self.tparams.to_oxidized(arena),
            where_constraints: self.where_constraints.to_oxidized(arena),
            params: self.params.to_oxidized(arena),
            implicit_params: self.implicit_params.to_oxidized(arena),
            ret: self.ret.to_oxidized(arena),
            flags: self.flags,
            ifc_decl: self.ifc_decl.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for PossiblyEnforcedTy<DeclTy<R>> {
    type Output = &'a obr::typing_defs::PossiblyEnforcedTy<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::PossiblyEnforcedTy {
            enforced: self.enforced,
            type_: self.ty.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for FunParam<R, DeclTy<R>> {
    type Output = &'a obr::typing_defs::FunParam<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::FunParam {
            pos: self.pos.to_oxidized(arena),
            name: self.name.to_oxidized(arena),
            type_: self.ty.to_oxidized(arena),
            flags: self.flags,
        })
    }
}

impl<'a> ToOxidized<'a> for XhpEnumValue {
    type Output = obr::ast_defs::XhpEnumValue<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::ast_defs::XhpEnumValue as Obr;
        match self {
            Self::XEVInt(i) => Obr::XEVInt(*i),
            Self::XEVString(s) => Obr::XEVString(s.to_oxidized(arena)),
        }
    }
}

impl<'a> ToOxidized<'a> for ClassConstFrom {
    type Output = obr::typing_defs::ClassConstFrom<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::typing_defs::ClassConstFrom as Obr;
        match self {
            Self::Self_ => Obr::Self_,
            Self::From(ty) => Obr::From(ty.to_oxidized(arena)),
        }
    }
}

impl<'a> ToOxidized<'a> for ClassConstRef {
    type Output = obr::typing_defs::ClassConstRef<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        let ClassConstRef(class, symbol) = self;
        obr::typing_defs::ClassConstRef(class.to_oxidized(arena), symbol.to_oxidized(arena))
    }
}

impl<'a, R: Reason> ToOxidized<'a> for AbstractTypeconst<R> {
    type Output = &'a obr::typing_defs::AbstractTypeconst<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::AbstractTypeconst {
            as_constraint: self.as_constraint.to_oxidized(arena),
            super_constraint: self.super_constraint.to_oxidized(arena),
            default: self.default.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for ConcreteTypeconst<R> {
    type Output = &'a obr::typing_defs::ConcreteTypeconst<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::ConcreteTypeconst {
            tc_type: self.ty.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for Typeconst<R> {
    type Output = obr::typing_defs::Typeconst<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        use obr::typing_defs::Typeconst as Obr;
        match self {
            Self::TCAbstract(x) => Obr::TCAbstract(x.to_oxidized(arena)),
            Self::TCConcrete(x) => Obr::TCConcrete(x.to_oxidized(arena)),
        }
    }
}

impl<'a, R: Reason> ToOxidized<'a> for folded::SubstContext<R> {
    type Output = &'a obr::decl_defs::SubstContext<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        let Subst(subst) = &self.subst;
        arena.alloc(obr::decl_defs::SubstContext {
            subst: subst.to_oxidized(arena),
            class_context: self.class_context.to_oxidized(arena),
            from_req_extends: self.from_req_extends,
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for folded::TypeConst<R> {
    type Output = &'a obr::typing_defs::TypeconstType<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::TypeconstType {
            synthesized: self.is_synthesized,
            concretized: self.is_concretized,
            is_ctx: self.is_ctx,
            enforceable: self.enforceable.as_ref().map_or_else(
                || (obr::pos::Pos::none(), false),
                |x| (x.to_oxidized(arena), true),
            ),
            reifiable: self.reifiable.as_ref().map(|x| x.to_oxidized(arena)),
            origin: self.origin.to_oxidized(arena),
            kind: self.kind.to_oxidized(arena),
            name: self.name.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for folded::ClassConst<R> {
    type Output = &'a obr::typing_defs::ClassConst<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::typing_defs::ClassConst {
            synthesized: self.is_synthesized,
            abstract_: self.kind,
            origin: self.origin.to_oxidized(arena),
            refs: self.refs.to_oxidized(arena),
            type_: self.ty.to_oxidized(arena),
            pos: self.pos.to_oxidized(arena),
        })
    }
}

impl<'a, R: Reason> ToOxidized<'a> for folded::FoldedClass<R> {
    type Output = obr::decl_defs::DeclClassType<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        obr::decl_defs::DeclClassType {
            name: self.name.to_oxidized(arena),
            pos: self.pos.to_oxidized(arena),
            kind: self.kind,
            abstract_: self.is_abstract(),
            final_: self.is_final,
            const_: self.is_const,
            internal: self.is_internal,
            is_xhp: self.is_xhp,
            has_xhp_keyword: self.has_xhp_keyword,
            support_dynamic_type: self.support_dynamic_type,
            module: self.module.as_ref().map(|m| {
                let (pos, id) = m.to_oxidized(arena);
                Id(pos, id)
            }),
            tparams: self.tparams.to_oxidized(arena),
            where_constraints: self.where_constraints.to_oxidized(arena),
            substs: self.substs.to_oxidized(arena),
            ancestors: self.ancestors.to_oxidized(arena),
            props: self.props.to_oxidized(arena),
            sprops: self.static_props.to_oxidized(arena),
            methods: self.methods.to_oxidized(arena),
            smethods: self.static_methods.to_oxidized(arena),
            consts: self.consts.to_oxidized(arena),
            typeconsts: self.type_consts.to_oxidized(arena),
            xhp_enum_values: self.xhp_enum_values.to_oxidized(arena),
            // TODO(milliechen): implement the rest
            construct: (None, obr::typing_defs::ConsistentKind::Inconsistent),
            need_init: false,
            deferred_init_members: SSet::empty(),
            req_ancestors: &[],
            req_ancestors_extends: SSet::empty(),
            extends: SSet::empty(),
            sealed_whitelist: None,
            xhp_attr_deps: SSet::empty(),
            enum_type: None,
            decl_errors: None,
        }
    }
}

impl<'a> ToOxidized<'a> for folded::FoldedElement {
    type Output = &'a obr::decl_defs::Element<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc(obr::decl_defs::Element {
            origin: self.origin.to_oxidized(arena),
            visibility: self.visibility.to_oxidized(arena),
            deprecated: self.deprecated.map(|x| {
                bumpalo::collections::String::from_utf8_lossy_in(x.as_bytes(), arena)
                    .into_bump_str()
            }),
            flags: self.flags,
        })
    }
}

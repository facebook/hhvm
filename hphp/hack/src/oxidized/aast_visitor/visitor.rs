// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<da149b32ff9f3823dbfc9870834f1aab>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node::Node;
use super::type_params::Params;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
pub fn visit<P: Params>(
    v: &mut impl Visitor<P = P>,
    c: &mut P::Context,
    p: &impl Node<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait Visitor {
    type P: Params;
    fn object(&mut self) -> &mut dyn Visitor<P = Self::P>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &<Self::P as Params>::Ex,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_fb(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &<Self::P as Params>::Fb,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &<Self::P as Params>::En,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_hi(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &<Self::P as Params>::Hi,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Afield<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &AsExpr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_assert_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &AssertExpr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_bop(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Bop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &CaField<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_type(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &CaType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_call_type(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &CallType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Case<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_catch(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Catch<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassAttr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassConst<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassGetExpr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassId<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassId_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_tparams(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassTparams<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassTypeconst<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_var(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ClassVar<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Class_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_collection_targ(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &CollectionTarg<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ConstraintKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Def<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_doc_comment(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &DocComment,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &EmitId,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Enum_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Expr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Expr_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_field(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Field<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_file_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FileAttribute<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FunKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FunParam<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_variadicity(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FunVariadicity<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Fun_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_body(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FuncBody<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_reactive(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &FuncReactive,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Gconst<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Hint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &HintFun,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Hint_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Id,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ImportFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &InsteadofAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &KvcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Lid,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_redeclaration(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &MethodRedeclaration<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Method_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_nast_shape_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &NastShapeInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &NsKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &OgNullFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ParamKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_mutability(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ParamMutability,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_enum(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &PuEnum<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_loc(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &PuLoc,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_member(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &PuMember<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_record_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &RecordDef<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_reify_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ReifyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ShapeFieldInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &ShapeFieldName,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Stmt<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Stmt_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_targ(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Targ<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Tparam<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tprim(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Tprim,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &TypeHint<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typeconst_abstract_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &TypeconstAbstractKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Typedef<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &TypedefVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Uop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &UseAsAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &UseAsVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &UserAttribute<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_using_stmt(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &UsingStmt<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_variance(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Variance,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &VcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &Visibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &WhereConstraint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpAttr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpAttrInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpAttrTag,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpAttribute<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpChild,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &XhpChildOp,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
}

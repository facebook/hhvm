// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<dbfe99e445fd5fd600be9f76851d3b53>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node_mut::NodeMut;
use super::type_params::Params;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
pub fn visit<P: Params>(
    v: &mut impl VisitorMut<P = P>,
    c: &mut P::Context,
    p: &mut impl NodeMut<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait VisitorMut {
    type P: Params;
    fn object(&mut self) -> &mut dyn VisitorMut<P = Self::P>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut <Self::P as Params>::Ex,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_fb(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut <Self::P as Params>::Fb,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut <Self::P as Params>::En,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_hi(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut <Self::P as Params>::Hi,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Afield<
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
        p: &mut AsExpr<
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
        p: &mut AssertExpr<
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
        p: &mut Bop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut CaField<
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
        p: &mut CaType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_call_type(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut CallType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Case<
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
        p: &mut Catch<
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
        p: &mut ClassAttr<
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
        p: &mut ClassConst<
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
        p: &mut ClassGetExpr<
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
        p: &mut ClassId<
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
        p: &mut ClassId_<
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
        p: &mut ClassKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_tparams(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ClassTparams<
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
        p: &mut ClassTypeconst<
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
        p: &mut ClassVar<
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
        p: &mut Class_<
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
        p: &mut CollectionTarg<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ConstraintKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Def<
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
        p: &mut DocComment,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut EmitId,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Enum_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Expr<
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
        p: &mut Expr_<
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
        p: &mut Field<
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
        p: &mut FileAttribute<
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
        p: &mut FunKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut FunParam<
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
        p: &mut FunVariadicity<
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
        p: &mut Fun_<
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
        p: &mut FuncBody<
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
        p: &mut FuncReactive,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Gconst<
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
        p: &mut Hint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut HintFun,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Hint_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Id,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ImportFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut InsteadofAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut KvcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Lid,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_redeclaration(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut MethodRedeclaration<
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
        p: &mut Method_<
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
        p: &mut NastShapeInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut NsKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut OgNullFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ParamKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_mutability(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ParamMutability,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_enum(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut PuEnum<
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
        p: &mut PuLoc,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_member(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut PuMember<
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
        p: &mut RecordDef<
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
        p: &mut ReifyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ShapeFieldInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut ShapeFieldName,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Stmt<
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
        p: &mut Stmt_<
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
        p: &mut Targ<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Tparam<
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
        p: &mut Tprim,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut TypeHint<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typeconst_abstract_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut TypeconstAbstractKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Typedef<
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
        p: &mut TypedefVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Uop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut UseAsAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut UseAsVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut UserAttribute<
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
        p: &mut UsingStmt<
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
        p: &mut Variance,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut VcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut Visibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut WhereConstraint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut XhpAttr<
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
        p: &mut XhpAttrInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut XhpAttrTag,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut XhpAttribute<
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
        p: &mut XhpChild,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &mut XhpChildOp,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
}

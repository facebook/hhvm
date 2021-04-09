// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ed7cc613e4f0c8aa427140921885a219>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::node_mut::NodeMut;
use super::type_params::Params;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
pub fn visit<'node, P: Params>(
    v: &mut impl VisitorMut<'node, P = P>,
    c: &mut P::Context,
    p: &'node mut impl NodeMut<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait VisitorMut<'node> {
    type P: Params;
    fn object(&mut self) -> &mut dyn VisitorMut<'node, P = Self::P>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut <Self::P as Params>::Ex,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_fb(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut <Self::P as Params>::Fb,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut <Self::P as Params>::En,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_hi(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut <Self::P as Params>::Hi,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Afield<
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
        p: &'node mut AsExpr<
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
        p: &'node mut Bop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut CaField<
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
        p: &'node mut CaType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Case<
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
        p: &'node mut Catch<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_abstract_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassAbstractTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassAttr<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_concrete_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassConcreteTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassConst<
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
        p: &'node mut ClassGetExpr<
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
        p: &'node mut ClassId<
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
        p: &'node mut ClassId_<
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
        p: &'node mut ClassKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_partially_abstract_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassPartiallyAbstractTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ClassTypeconstDef<
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
        p: &'node mut ClassVar<
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
        p: &'node mut Class_<
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
        p: &'node mut CollectionTarg<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ConstraintKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_contexts(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Contexts,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Def<
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
        p: &'node mut DocComment,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut EmitId,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Enum_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_env_annot(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut EnvAnnot,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Expr<
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
        p: &'node mut Expr_<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expression_tree(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ExpressionTree<
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
        p: &'node mut Field<
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
        p: &'node mut FileAttribute<
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
        p: &'node mut FunKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut FunParam<
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
        p: &'node mut FunVariadicity<
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
        p: &'node mut Fun_<
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
        p: &'node mut FuncBody<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_function_ptr_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut FunctionPtrId<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Gconst<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hf_param_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut HfParamInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Hint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut HintFun,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Hint_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hole_source(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut HoleSource,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Id,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ImportFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut InsteadofAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut KvcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Lid,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Method_<
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
        p: &'node mut NastShapeInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut NsKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut OgNullFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ParamKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_readonly_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ReadonlyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_record_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut RecordDef<
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
        p: &'node mut ReifyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ShapeFieldInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut ShapeFieldName,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Stmt<
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
        p: &'node mut Stmt_<
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
        p: &'node mut Targ<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Tparam<
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
        p: &'node mut Tprim,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut TypeHint<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Typedef<
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
        p: &'node mut TypedefVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Uop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut UseAsAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut UseAsVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut UserAttribute<
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
        p: &'node mut UsingStmt<
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
        p: &'node mut Variance,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut VcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut Visibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut WhereConstraintHint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut XhpAttr<
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
        p: &'node mut XhpAttrInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut XhpAttrTag,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut XhpAttribute<
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
        p: &'node mut XhpChild,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut XhpChildOp,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_simple(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node mut XhpSimple<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
}

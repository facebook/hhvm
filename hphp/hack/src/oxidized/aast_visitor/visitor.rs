// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a4707a93d20c378d9e734ff70ffcf204>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::node::Node;
use super::type_params::Params;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
pub fn visit<'node, P: Params>(
    v: &mut impl Visitor<'node, P = P>,
    c: &mut P::Context,
    p: &'node impl Node<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait Visitor<'node> {
    type P: Params;
    fn object(&mut self) -> &mut dyn Visitor<'node, P = Self::P>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node <Self::P as Params>::Ex,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_fb(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node <Self::P as Params>::Fb,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node <Self::P as Params>::En,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_hi(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node <Self::P as Params>::Hi,
    ) -> Result<(), <Self::P as Params>::Error> {
        Ok(())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Afield<
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
        p: &'node AsExpr<
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
        p: &'node Bop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node CaField<
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
        p: &'node CaType,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Case<
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
        p: &'node Catch<
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
        p: &'node ClassAbstractTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ClassAttr<
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
        p: &'node ClassConcreteTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ClassConst<
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
        p: &'node ClassGetExpr<
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
        p: &'node ClassId<
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
        p: &'node ClassId_<
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
        p: &'node ClassKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_partially_abstract_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ClassPartiallyAbstractTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ClassTypeconst,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ClassTypeconstDef<
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
        p: &'node ClassVar<
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
        p: &'node Class_<
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
        p: &'node CollectionTarg<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ConstraintKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_contexts(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Contexts,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Def<
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
        p: &'node DocComment,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node EmitId,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Enum_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_env_annot(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node EnvAnnot,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Expr<
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
        p: &'node Expr_<
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
        p: &'node ExpressionTree<
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
        p: &'node Field<
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
        p: &'node FileAttribute<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node FunDef<
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
        p: &'node FunKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node FunParam<
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
        p: &'node FunVariadicity<
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
        p: &'node Fun_<
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
        p: &'node FuncBody<
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
        p: &'node FunctionPtrId<
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
        p: &'node Gconst<
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
        p: &'node HfParamInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Hint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node HintFun,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Hint_,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hole_source(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node HoleSource,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Id,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ImportFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node InsteadofAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node KvcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Lid,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Method_<
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
        p: &'node NastShapeInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node NsKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node OgNullFlavor,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ParamKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_readonly_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ReadonlyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_record_def(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node RecordDef<
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
        p: &'node ReifyKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ShapeFieldInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node ShapeFieldName,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Stmt<
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
        p: &'node Stmt_<
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
        p: &'node Targ<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Tparam<
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
        p: &'node Tprim,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node TypeHint<<Self::P as Params>::Hi>,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Typedef<
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
        p: &'node TypedefVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Uop,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node UseAsAlias,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node UseAsVisibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node UserAttribute<
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
        p: &'node UsingStmt<
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
        p: &'node Variance,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node VcKind,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node Visibility,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint_hint(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node WhereConstraintHint,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpAttr<
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
        p: &'node XhpAttrInfo,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpAttrTag,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpAttribute<
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
        p: &'node XhpChild,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpChildOp,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_enum_value(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpEnumValue,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_simple(
        &mut self,
        c: &mut <Self::P as Params>::Context,
        p: &'node XhpSimple<
            <Self::P as Params>::Ex,
            <Self::P as Params>::Fb,
            <Self::P as Params>::En,
            <Self::P as Params>::Hi,
        >,
    ) -> Result<(), <Self::P as Params>::Error> {
        p.recurse(c, self.object())
    }
}

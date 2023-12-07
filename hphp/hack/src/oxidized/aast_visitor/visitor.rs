// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e19853c409f94c6985933f91505f7cc9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_imports)]
#![allow(unused_variables)]
use super::node::Node;
use super::type_params::Params;
use crate::aast_defs::*;
use crate::aast_defs::{self};
use crate::ast_defs::*;
use crate::ast_defs::{self};
use crate::*;
pub fn visit<'node, P: Params>(
    v: &mut impl Visitor<'node, Params = P>,
    c: &mut P::Context,
    p: &'node impl Node<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait Visitor<'node> {
    type Params: Params;
    fn object(&mut self) -> &mut dyn Visitor<'node, Params = Self::Params>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node <Self::Params as Params>::Ex,
    ) -> Result<(), <Self::Params as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node <Self::Params as Params>::En,
    ) -> Result<(), <Self::Params as Params>::Error> {
        Ok(())
    }
    fn visit_abstraction(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Abstraction,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Afield<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node AsExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node As_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_binop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Binop<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_block(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Block<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_bop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Bop,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_call_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node CallExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_capture_lid(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node CaptureLid<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Case<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_catch(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Catch<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_abstract_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassAbstractTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_concrete_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassConcreteTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassConst<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassConstKind<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassGetExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassId<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassId_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_req(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassReq,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassTypeconstDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_var(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassVar<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Class_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_classish_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ClassishKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_collection_targ(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node CollectionTarg<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ConstraintKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_contexts(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Contexts,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ctx_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node CtxRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ctx_refinement_bounds(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node CtxRefinementBounds,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Def<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_default_case(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node DefaultCase<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_efun(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Efun<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node EmitId,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Enum_,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Expr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Expr_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expression_tree(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ExpressionTree<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_field(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Field<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_file_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FileAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_finally_block(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FinallyBlock<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FunDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FunKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FunParam<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Fun_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_body(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FuncBody<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_function_ptr_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node FunctionPtrId<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Gconst<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hf_param_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node HfParamInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Hint,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node HintFun,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Hint_,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hole_source(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node HoleSource,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Id,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ImportFlavor,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node KvcKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Lid,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_md_name_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node MdNameKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Method_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_module_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ModuleDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_nast_shape_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node NastShapeInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node NsKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node OgNullFlavor,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ParamKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pat_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node PatRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pat_var(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node PatVar,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pattern(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Pattern,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_program(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Program<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_prop_or_method(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node PropOrMethod,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_readonly_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ReadonlyKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Refinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_reify_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ReifyKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_require_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node RequireKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ShapeFieldInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node ShapeFieldName,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Stmt<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_match(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node StmtMatch<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_match_arm(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node StmtMatchArm<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Stmt_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_targ(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Targ<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Tparam<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tprim(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Tprim,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node TypeHint<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node TypeRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_refinement_bounds(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node TypeRefinementBounds,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Typedef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef_visibility(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node TypedefVisibility,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Uop,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node UserAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attributes(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node UserAttributes<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_using_stmt(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node UsingStmt<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_variance(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Variance,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node VcKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node Visibility,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node WhereConstraintHint,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpAttr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpAttrInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpAttrTag,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpChild,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpChildOp,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_enum_value(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpEnumValue,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_simple(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node XhpSimple<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
}

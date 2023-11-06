// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ee2147344f15dcab61129c0b99023828>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_imports)]
#![allow(unused_variables)]
use super::node_mut::NodeMut;
use super::type_params::Params;
use crate::aast_defs::*;
use crate::aast_defs::{self};
use crate::ast_defs::*;
use crate::ast_defs::{self};
use crate::*;
pub fn visit<'node, P: Params>(
    v: &mut impl VisitorMut<'node, Params = P>,
    c: &mut P::Context,
    p: &'node mut impl NodeMut<P>,
) -> Result<(), P::Error> {
    p.accept(c, v)
}
pub trait VisitorMut<'node> {
    type Params: Params;
    fn object(&mut self) -> &mut dyn VisitorMut<'node, Params = Self::Params>;
    fn visit_ex(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut <Self::Params as Params>::Ex,
    ) -> Result<(), <Self::Params as Params>::Error> {
        Ok(())
    }
    fn visit_en(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut <Self::Params as Params>::En,
    ) -> Result<(), <Self::Params as Params>::Error> {
        Ok(())
    }
    fn visit_abstraction(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Abstraction,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_afield(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Afield<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut AsExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut As_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_binop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Binop<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_block(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Block<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_bop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Bop,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_call_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut CallExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_capture_lid(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut CaptureLid<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Case<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_catch(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Catch<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_abstract_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassAbstractTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_concrete_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassConcreteTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassConst<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassConstKind<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassGetExpr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassId<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassId_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_req(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassReq,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassTypeconst,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassTypeconstDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_var(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassVar<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Class_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_classish_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ClassishKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_collection_targ(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut CollectionTarg<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ConstraintKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_contexts(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Contexts,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ctx_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut CtxRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ctx_refinement_bounds(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut CtxRefinementBounds,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Def<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_default_case(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut DefaultCase<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_efun(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Efun<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_emit_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut EmitId,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Enum_,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_env_annot(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut EnvAnnot,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Expr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Expr_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expression_tree(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ExpressionTree<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_field(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Field<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_file_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FileAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_finally_block(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FinallyBlock<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FunDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FunKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FunParam<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Fun_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_body(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FuncBody<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_function_ptr_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut FunctionPtrId<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Gconst<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hf_param_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut HfParamInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Hint,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut HintFun,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Hint_,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hole_source(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut HoleSource,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Id,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ImportFlavor,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut KvcKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Lid,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_md_name_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut MdNameKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Method_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_module_def(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ModuleDef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_nast_shape_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut NastShapeInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut NsKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut OgNullFlavor,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ParamKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pat_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut PatRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pat_var(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut PatVar,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pattern(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Pattern,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_program(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Program<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_prop_or_method(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut PropOrMethod,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_readonly_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ReadonlyKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Refinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_reify_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ReifyKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_require_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut RequireKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ShapeFieldInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut ShapeFieldName,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Stmt<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_match(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut StmtMatch<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_match_arm(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut StmtMatchArm<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Stmt_<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_targ(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Targ<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Tparam<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tprim(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Tprim,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut TypeHint<<Self::Params as Params>::Ex>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_refinement(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut TypeRefinement,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_refinement_bounds(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut TypeRefinementBounds,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Typedef<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef_visibility(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut TypedefVisibility,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Uop,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut UserAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attributes(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut UserAttributes<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_using_stmt(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut UsingStmt<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_variance(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Variance,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut VcKind,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut Visibility,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint_hint(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut WhereConstraintHint,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpAttr<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_info(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpAttrInfo,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpAttrTag,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpAttribute<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpChild,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpChildOp,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_enum_value(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpEnumValue,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_simple(
        &mut self,
        c: &mut <Self::Params as Params>::Context,
        p: &'node mut XhpSimple<<Self::Params as Params>::Ex, <Self::Params as Params>::En>,
    ) -> Result<(), <Self::Params as Params>::Error> {
        p.recurse(c, self.object())
    }
}

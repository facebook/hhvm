use std::ops::ControlFlow;

use oxidized::aast_defs::Afield;
use oxidized::aast_defs::AsExpr;
use oxidized::aast_defs::Block;
use oxidized::aast_defs::Case;
use oxidized::aast_defs::Catch;
use oxidized::aast_defs::ClassAbstractTypeconst;
use oxidized::aast_defs::ClassConcreteTypeconst;
use oxidized::aast_defs::ClassConst;
use oxidized::aast_defs::ClassConstKind;
use oxidized::aast_defs::ClassGetExpr;
use oxidized::aast_defs::ClassHint;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::ClassTypeconst;
use oxidized::aast_defs::ClassTypeconstDef;
use oxidized::aast_defs::ClassVar;
use oxidized::aast_defs::Class_;
use oxidized::aast_defs::CollectionTarg;
use oxidized::aast_defs::Context;
use oxidized::aast_defs::Contexts;
use oxidized::aast_defs::CtxRefinement;
use oxidized::aast_defs::CtxRefinementBounds;
use oxidized::aast_defs::Def;
use oxidized::aast_defs::DefaultCase;
use oxidized::aast_defs::Efun;
use oxidized::aast_defs::Enum_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::ExpressionTree;
use oxidized::aast_defs::Field;
use oxidized::aast_defs::FileAttribute;
use oxidized::aast_defs::FunDef;
use oxidized::aast_defs::FunParam;
use oxidized::aast_defs::Fun_;
use oxidized::aast_defs::FuncBody;
use oxidized::aast_defs::FunctionPtrId;
use oxidized::aast_defs::Gconst;
use oxidized::aast_defs::GenCase;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::HintFun;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Method_;
use oxidized::aast_defs::ModuleDef;
use oxidized::aast_defs::NastShapeInfo;
use oxidized::aast_defs::Program;
use oxidized::aast_defs::Refinement;
use oxidized::aast_defs::RequireKind;
use oxidized::aast_defs::ShapeFieldInfo;
use oxidized::aast_defs::Stmt;
use oxidized::aast_defs::Stmt_;
use oxidized::aast_defs::Targ;
use oxidized::aast_defs::Tparam;
use oxidized::aast_defs::TraitHint;
use oxidized::aast_defs::TypeHint;
use oxidized::aast_defs::TypeHint_;
use oxidized::aast_defs::TypeRefinement;
use oxidized::aast_defs::TypeRefinementBounds;
use oxidized::aast_defs::Typedef;
use oxidized::aast_defs::UserAttribute;
use oxidized::aast_defs::UserAttributes;
use oxidized::aast_defs::UsingStmt;
use oxidized::aast_defs::VariadicHint;
use oxidized::aast_defs::WhereConstraintHint;
use oxidized::aast_defs::XhpAttr;
use oxidized::aast_defs::XhpAttrHint;
use oxidized::aast_defs::XhpAttribute;
use oxidized::aast_defs::XhpSimple;

pub trait Pass {
    type Ctx: Clone;
    type Err;

    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        _elem: &mut Program<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Def ------------------------------------------------------------------

    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        _elem: &mut Def<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- TypeDef --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        _elem: &mut Typedef<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FunDef ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        _elem: &mut FunDef<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ModuleDef ------------------------------------------------------------

    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        _elem: &mut ModuleDef<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Gconst ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        _elem: &mut Gconst<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_gconst_type_(
        &self,
        _elem: &mut Option<Hint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        _elem: &mut Expr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Stmt -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        _elem: &mut Stmt<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        _elem: &mut Stmt_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        _elem: &mut Block<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        _elem: &mut UsingStmt<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_gen_case<Ex, En>(
        &self,
        _elem: &mut GenCase<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        _elem: &mut Case<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        _elem: &mut DefaultCase<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        _elem: &mut Catch<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        _elem: &mut AsExpr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassId --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        _elem: &mut ClassId<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        _elem: &mut ClassId_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Expr -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        _elem: &mut Expr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        _elem: &mut Expr_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        _elem: &mut CollectionTarg<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_funcion_ptr_id<Ex, En>(
        &self,
        _elem: &mut FunctionPtrId<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        _elem: &mut ExpressionTree<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        _elem: &mut ClassGetExpr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        _elem: &mut Field<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        _elem: &mut Afield<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        _elem: &mut XhpSimple<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        _elem: &mut XhpAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        _elem: &mut XhpAttr<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
    // -- Fun_ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        _elem: &mut Fun_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_fun__ret<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Method_ --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        _elem: &mut Method_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_method__ret<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FunParam -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        _elem: &mut FunParam<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Efun -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        _elem: &mut Efun<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FuncBody -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        _elem: &mut FuncBody<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Class_ ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        _elem: &mut Class_<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        _elem: &mut Vec<Tparam<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__extends(
        &self,
        _elem: &mut Vec<ClassHint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__uses(
        &self,
        _elem: &mut Vec<TraitHint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        _elem: &mut Vec<XhpAttrHint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__reqs(
        &self,
        _elem: &mut Vec<(ClassHint, RequireKind)>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__implements(
        &self,
        _elem: &mut Vec<ClassHint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__where_constraints(
        &self,
        _elem: &mut Vec<WhereConstraintHint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        _elem: &mut Vec<ClassConst<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__typeconsts<Ex, En>(
        &self,
        _elem: &mut Vec<ClassTypeconstDef<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__vars<Ex, En>(
        &self,
        _elem: &mut Vec<ClassVar<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__methods<Ex, En>(
        &self,
        _elem: &mut Vec<Method_<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        _elem: &mut Vec<XhpAttr<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        _elem: &mut UserAttributes<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__enum_(
        &self,
        _elem: &mut Option<Enum_>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassVar -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        _elem: &mut ClassVar<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_class_var_expr<Ex, En>(
        &self,
        _elem: &mut Option<Expr<Ex, En>>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassConst -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        _elem: &mut ClassConst<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        _elem: &mut ClassConstKind<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassTypeconstDef ----------------------------------------------------
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        _elem: &mut ClassTypeconstDef<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        _elem: &mut ClassTypeconst,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        _elem: &mut ClassAbstractTypeconst,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        _elem: &mut ClassConcreteTypeconst,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Context(s) -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_context(
        &self,
        _elem: &mut Context,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- WhereConstraintHint --------------------------------------------------

    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        _elem: &mut WhereConstraintHint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Enum -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_enum_(
        &self,
        _elem: &mut Enum_,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- TypeHint<Ex> ---------------------------------------------------------

    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_hint_(
        &self,
        _elem: &mut TypeHint_,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Targ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_targ<Ex>(
        &self,
        _elem: &mut Targ<Ex>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Tparam ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        _elem: &mut Tparam<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- UserAttribute(s) -----------------------------------------------------

    #[inline(always)]
    fn on_ty_user_attributes<Ex, En>(
        &self,
        _elem: &mut UserAttributes<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        _elem: &mut UserAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FileAttribute --------------------------------------------------------

    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        _elem: &mut FileAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Hints ----------------------------------------------------------------
    #[inline(always)]
    fn on_ty_hint(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_hint_(
        &self,
        _elem: &mut Hint_,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_hint_fun(
        &self,
        _elem: &mut HintFun,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_param_tys(
        &self,
        _elem: &mut Vec<Hint>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_variadic_ty(
        &self,
        _elem: &mut VariadicHint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_ctxs(
        &self,
        _elem: &mut Option<Contexts>,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_refinement(
        &self,
        _elem: &mut Refinement,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        _elem: &mut TypeRefinement,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        _elem: &mut TypeRefinementBounds,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        _elem: &mut CtxRefinement,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        _elem: &mut CtxRefinementBounds,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_contexts(
        &self,
        _elem: &mut Contexts,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        _elem: &mut NastShapeInfo,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        _elem: &mut ShapeFieldInfo,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_hint(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_trait_hint(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attr_hint(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
        _errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
}

struct Passes<Ctx, Err, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx, Err = Err>,
    Q: Pass<Ctx = Ctx, Err = Err>,
{
    fst: P,
    snd: Q,
}

impl<Ctx, Err, P, Q> Pass for Passes<Ctx, Err, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx, Err = Err>,
    Q: Pass<Ctx = Ctx, Err = Err>,
{
    type Ctx = Ctx;
    type Err = Err;

    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        elem: &mut Program<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_program(elem, ctx, errs)?;
        self.snd.on_ty_program(elem, ctx, errs)
    }

    // -- Def ------------------------------------------------------------------

    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        elem: &mut Def<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_def(elem, ctx, errs)?;
        self.snd.on_ty_def(elem, ctx, errs)
    }

    // -- TypeDef --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        elem: &mut Typedef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_typedef(elem, ctx, errs)?;
        self.snd.on_ty_typedef(elem, ctx, errs)
    }

    // -- FunDef ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        elem: &mut FunDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_def(elem, ctx, errs)?;
        self.snd.on_ty_fun_def(elem, ctx, errs)
    }

    // -- ModuleDef ------------------------------------------------------------

    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        elem: &mut ModuleDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_module_def(elem, ctx, errs)?;
        self.snd.on_ty_module_def(elem, ctx, errs)
    }

    // -- Gconst ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        elem: &mut Gconst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_gconst(elem, ctx, errs)?;
        self.snd.on_ty_gconst(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_gconst_type_(
        &self,
        elem: &mut Option<Hint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_gconst_type_(elem, ctx, errs)?;
        self.snd.on_fld_gconst_type_(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_gconst_value(elem, ctx, errs)?;
        self.snd.on_fld_gconst_value(elem, ctx, errs)
    }

    // -- Stmt -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        elem: &mut Stmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_stmt(elem, ctx, errs)?;
        self.snd.on_ty_stmt(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        elem: &mut Stmt_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_stmt_(elem, ctx, errs)?;
        self.snd.on_ty_stmt_(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        elem: &mut Block<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_block(elem, ctx, errs)?;
        self.snd.on_ty_block(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        elem: &mut UsingStmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_using_stmt(elem, ctx, errs)?;
        self.snd.on_ty_using_stmt(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_gen_case<Ex, En>(
        &self,
        elem: &mut GenCase<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_gen_case(elem, ctx, errs)?;
        self.snd.on_ty_gen_case(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        elem: &mut Case<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_case(elem, ctx, errs)?;
        self.snd.on_ty_case(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        elem: &mut DefaultCase<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_default_case(elem, ctx, errs)?;
        self.snd.on_ty_default_case(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        elem: &mut Catch<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_catch(elem, ctx, errs)?;
        self.snd.on_ty_catch(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        elem: &mut AsExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_as_expr(elem, ctx, errs)?;
        self.snd.on_ty_as_expr(elem, ctx, errs)
    }

    // -- ClassId --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        elem: &mut ClassId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_id(elem, ctx, errs)?;
        self.snd.on_ty_class_id(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        elem: &mut ClassId_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_id_(elem, ctx, errs)?;
        self.snd.on_ty_class_id_(elem, ctx, errs)
    }

    // -- Expr -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expr(elem, ctx, errs)?;
        self.snd.on_ty_expr(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        elem: &mut Expr_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expr_(elem, ctx, errs)?;
        self.snd.on_ty_expr_(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        elem: &mut CollectionTarg<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_collection_targ(elem, ctx, errs)?;
        self.snd.on_ty_collection_targ(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_funcion_ptr_id<Ex, En>(
        &self,
        elem: &mut FunctionPtrId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_funcion_ptr_id(elem, ctx, errs)?;
        self.snd.on_ty_funcion_ptr_id(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        elem: &mut ExpressionTree<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expression_tree(elem, ctx, errs)?;
        self.snd.on_ty_expression_tree(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        elem: &mut ClassGetExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_get_expr(elem, ctx, errs)?;
        self.snd.on_ty_class_get_expr(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        elem: &mut Field<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_field(elem, ctx, errs)?;
        self.snd.on_ty_field(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        elem: &mut Afield<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_afield(elem, ctx, errs)?;
        self.snd.on_ty_afield(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        elem: &mut XhpSimple<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_simple(elem, ctx, errs)?;
        self.snd.on_ty_xhp_simple(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        elem: &mut XhpAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attribute(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attribute(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        elem: &mut XhpAttr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attr(elem, ctx, errs)
    }
    // -- Fun_ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        elem: &mut Fun_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_(elem, ctx, errs)?;
        self.snd.on_ty_fun_(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_fun__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_fun__ret(elem, ctx, errs)?;
        self.snd.on_fld_fun__ret(elem, ctx, errs)
    }

    // -- Method_ --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        elem: &mut Method_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_method_(elem, ctx, errs)?;
        self.snd.on_ty_method_(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_method__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_method__ret(elem, ctx, errs)?;
        self.snd.on_fld_method__ret(elem, ctx, errs)
    }

    // -- FunParam -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        elem: &mut FunParam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_param(elem, ctx, errs)?;
        self.snd.on_ty_fun_param(elem, ctx, errs)
    }

    // -- Efun -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        elem: &mut Efun<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_efun(elem, ctx, errs)?;
        self.snd.on_ty_efun(elem, ctx, errs)
    }

    // -- FuncBody -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        elem: &mut FuncBody<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_func_body(elem, ctx, errs)?;
        self.snd.on_ty_func_body(elem, ctx, errs)
    }

    // -- Class_ ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        elem: &mut Class_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_(elem, ctx, errs)?;
        self.snd.on_ty_class_(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        elem: &mut Vec<Tparam<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__tparams(elem, ctx, errs)?;
        self.snd.on_fld_class__tparams(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__extends(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__extends(elem, ctx, errs)?;
        self.snd.on_fld_class__extends(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__uses(
        &self,
        elem: &mut Vec<TraitHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__uses(elem, ctx, errs)?;
        self.snd.on_fld_class__uses(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        elem: &mut Vec<XhpAttrHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__xhp_attr_uses(elem, ctx, errs)?;
        self.snd.on_fld_class__xhp_attr_uses(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__reqs(
        &self,
        elem: &mut Vec<(ClassHint, RequireKind)>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__reqs(elem, ctx, errs)?;
        self.snd.on_fld_class__reqs(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__implements(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__implements(elem, ctx, errs)?;
        self.snd.on_fld_class__implements(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__where_constraints(
        &self,
        elem: &mut Vec<WhereConstraintHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__where_constraints(elem, ctx, errs)?;
        self.snd.on_fld_class__where_constraints(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__consts(elem, ctx, errs)?;
        self.snd.on_fld_class__consts(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__typeconsts<Ex, En>(
        &self,
        elem: &mut Vec<ClassTypeconstDef<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__typeconsts(elem, ctx, errs)?;
        self.snd.on_fld_class__typeconsts(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__vars<Ex, En>(
        &self,
        elem: &mut Vec<ClassVar<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__vars(elem, ctx, errs)?;
        self.snd.on_fld_class__vars(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__methods<Ex, En>(
        &self,
        elem: &mut Vec<Method_<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__methods(elem, ctx, errs)?;
        self.snd.on_fld_class__methods(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__xhp_attrs(elem, ctx, errs)?;
        self.snd.on_fld_class__xhp_attrs(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__user_attributes(elem, ctx, errs)?;
        self.snd.on_fld_class__user_attributes(elem, ctx, errs)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__enum_(
        &self,
        elem: &mut Option<Enum_>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__enum_(elem, ctx, errs)?;
        self.snd.on_fld_class__enum_(elem, ctx, errs)
    }

    // -- ClassVar -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        elem: &mut ClassVar<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_var(elem, ctx, errs)?;
        self.snd.on_ty_class_var(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class_var_type_(elem, ctx, errs)?;
        self.snd.on_fld_class_var_type_(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_class_var_expr<Ex, En>(
        &self,
        elem: &mut Option<Expr<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class_var_expr(elem, ctx, errs)?;
        self.snd.on_fld_class_var_expr(elem, ctx, errs)
    }

    // -- ClassConst -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        elem: &mut ClassConst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_const(elem, ctx, errs)?;
        self.snd.on_ty_class_const(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        elem: &mut ClassConstKind<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_const_kind(elem, ctx, errs)?;
        self.snd.on_ty_class_const_kind(elem, ctx, errs)
    }

    // -- ClassTypeconstDef ----------------------------------------------------
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_typeconst_def(elem, ctx, errs)?;
        self.snd.on_ty_class_typeconst_def(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        elem: &mut ClassTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_typeconst(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        elem: &mut ClassAbstractTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_abstract_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_abstract_typeconst(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        elem: &mut ClassConcreteTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_concrete_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_concrete_typeconst(elem, ctx, errs)
    }

    // -- Context(s) -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_contexts(
        &self,
        elem: &mut Contexts,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_contexts(elem, ctx, errs)?;
        self.snd.on_ty_contexts(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_context(
        &self,
        elem: &mut Context,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_context(elem, ctx, errs)?;
        self.snd.on_ty_context(elem, ctx, errs)
    }

    // -- WhereConstraintHint --------------------------------------------------

    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        elem: &mut WhereConstraintHint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_where_constraint_hint(elem, ctx, errs)?;
        self.snd.on_ty_where_constraint_hint(elem, ctx, errs)
    }

    // -- Enum -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_enum_(
        &self,
        elem: &mut Enum_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_enum_(elem, ctx, errs)?;
        self.snd.on_ty_enum_(elem, ctx, errs)
    }

    // -- TypeHint<Ex> ---------------------------------------------------------

    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_hint(elem, ctx, errs)?;
        self.snd.on_ty_type_hint(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_type_hint_(
        &self,
        elem: &mut TypeHint_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_hint_(elem, ctx, errs)?;
        self.snd.on_ty_type_hint_(elem, ctx, errs)
    }

    // -- Targ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_targ<Ex>(
        &self,
        elem: &mut Targ<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_targ(elem, ctx, errs)?;
        self.snd.on_ty_targ(elem, ctx, errs)
    }

    // -- Tparam ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        elem: &mut Tparam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_tparam(elem, ctx, errs)?;
        self.snd.on_ty_tparam(elem, ctx, errs)
    }

    // -- UserAttribute(s) -----------------------------------------------------

    #[inline(always)]
    fn on_ty_user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_user_attributes(elem, ctx, errs)?;
        self.snd.on_ty_user_attributes(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        elem: &mut UserAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_user_attribute(elem, ctx, errs)?;
        self.snd.on_ty_user_attribute(elem, ctx, errs)
    }

    // -- FileAttribute --------------------------------------------------------

    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        elem: &mut FileAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_file_attribute(elem, ctx, errs)?;
        self.snd.on_ty_file_attribute(elem, ctx, errs)
    }

    // -- Hints ----------------------------------------------------------------
    #[inline(always)]
    fn on_ty_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint(elem, ctx, errs)?;
        self.snd.on_ty_hint(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_hint_(
        &self,
        elem: &mut Hint_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_(elem, ctx, errs)?;
        self.snd.on_ty_hint_(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_hint_fun(
        &self,
        elem: &mut HintFun,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_fun(elem, ctx, errs)?;
        self.snd.on_ty_hint_fun(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_hint_fun_param_tys(
        &self,
        elem: &mut Vec<Hint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_param_tys(elem, ctx, errs)?;
        self.snd.on_fld_hint_fun_param_tys(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_hint_fun_variadic_ty(
        &self,
        elem: &mut VariadicHint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_variadic_ty(elem, ctx, errs)?;
        self.snd.on_fld_hint_fun_variadic_ty(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_hint_fun_ctxs(
        &self,
        elem: &mut Option<Contexts>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_ctxs(elem, ctx, errs)?;
        self.snd.on_fld_hint_fun_ctxs(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_return_ty(elem, ctx, errs)?;
        self.snd.on_fld_hint_fun_return_ty(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_refinement(
        &self,
        elem: &mut Refinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_refinement(elem, ctx, errs)?;
        self.snd.on_ty_refinement(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        elem: &mut TypeRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement(elem, ctx, errs)?;
        self.snd.on_ty_type_refinement(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        elem: &mut TypeRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement_bounds(elem, ctx, errs)?;
        self.snd.on_ty_type_refinement_bounds(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        elem: &mut CtxRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement(elem, ctx, errs)?;
        self.snd.on_ty_ctx_refinement(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        elem: &mut CtxRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement_bounds(elem, ctx, errs)?;
        self.snd.on_ty_ctx_refinement_bounds(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        elem: &mut NastShapeInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_nast_shape_info(elem, ctx, errs)?;
        self.snd.on_ty_nast_shape_info(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        elem: &mut ShapeFieldInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_shape_field_info(elem, ctx, errs)?;
        self.snd.on_ty_shape_field_info(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_class_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_hint(elem, ctx, errs)?;
        self.snd.on_ty_class_hint(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_trait_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_trait_hint(elem, ctx, errs)?;
        self.snd.on_ty_trait_hint(elem, ctx, errs)
    }

    #[inline(always)]
    fn on_ty_xhp_attr_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr_hint(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attr_hint(elem, ctx, errs)
    }
}

// Used to combine multiple types implementing `Pass` into nested `Passes` types
// without requiring them to hand write it so :
// `!passes (p1,p2,p3)` => `Passes(p1,Passes(p2,p3))`
#[macro_export]
macro_rules! passes{
    ( $p:expr $(,$ps:expr)+ $(,)? ) => {
        $crate::transform::Passes($p, passes!($($ps),*))
    };
    ( $p:expr $(,)? ) => {
        $p
    };
}

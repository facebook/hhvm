class virtual ['b] reduce :
  object ('b)
    constraint 'b =
      < on_ADef : 'c -> Ast_visitors_ancestors.def -> 'd;
        on_AExpr : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_AFkvalue : 'c ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.expr -> 'd;
        on_AFvalue : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_AHint : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_AMpamp : 'c -> 'd;
        on_AProgram : 'c -> Ast_visitors_ancestors.program -> 'd;
        on_AStmt : 'c -> Ast_visitors_ancestors.stmt -> 'd;
        on_AbsConst : 'c ->
                      Ast_visitors_ancestors.hint option ->
                      Ast_visitors_ancestors.id -> 'd;
        on_Abstract : 'c -> 'd;
        on_Alias : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_Amp : 'c -> 'd;
        on_Array : 'c -> Ast_visitors_ancestors.afield list -> 'd;
        on_Darray : 'c ->
                    (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr)
                    list ->
                    'd;
        on_Varray : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_Array_get : 'c ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr option -> 'd;
        on_As_kv : 'c ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr -> 'd;
        on_As_v : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Attributes : 'c -> Ast_visitors_ancestors.class_attr list -> 'd;
        on_Await : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_BArbar : 'c -> 'd; on_Bar : 'c -> 'd;
        on_Binop : 'c ->
                   Ast_visitors_ancestors.bop ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr -> 'd;
        on_Block : 'c -> Ast_visitors_ancestors.block -> 'd;
        on_BracedExpr : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Break : 'c -> Ast_visitors_ancestors.pos_t -> int option -> 'd;
        on_CA_enum : 'c -> string list -> 'd;
        on_CA_field : 'c -> Ast_visitors_ancestors.ca_field -> 'd;
        on_CA_hint : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_CA_name : 'c -> Ast_visitors_ancestors.id -> 'd;
        on_Cabstract : 'c -> 'd;
        on_Call : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr list -> 'd;
        on_Case : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.block -> 'd;
        on_Cast : 'c ->
                  Ast_visitors_ancestors.hint ->
                  Ast_visitors_ancestors.expr -> 'd;
        on_Cenum : 'c -> 'd; on_Cinterface : 'c -> 'd;
        on_Class : 'c -> Ast_visitors_ancestors.class_ -> 'd;
        on_ClassTraitRequire : 'c ->
                               Ast_visitors_ancestors.trait_req_kind ->
                               Ast_visitors_ancestors.hint -> 'd;
        on_ClassUse : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_ClassUseAlias : 'c ->
                            Ast_visitors_ancestors.id option ->
                            Ast_visitors_ancestors.pstring ->
                            Ast_visitors_ancestors.id option ->
                            Ast_visitors_ancestors.kind option ->
                           'd;
        on_ClassUsePrecedence : 'c ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring ->
                           Ast_visitors_ancestors.id list ->
                          'd;
        on_ClassVars : 'c ->
                       Ast_visitors_ancestors.kind list ->
                       Ast_visitors_ancestors.hint option ->
                       Ast_visitors_ancestors.class_var list ->
                       string option -> 'd;
        on_Class_const : 'c ->
                         Ast_visitors_ancestors.id ->
                         Ast_visitors_ancestors.pstring -> 'd;
        on_Class_get : 'c ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.expr -> 'd;
        on_Clone : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Cnormal : 'c -> 'd;
        on_Collection : 'c ->
                        Ast_visitors_ancestors.id ->
                        Ast_visitors_ancestors.afield list -> 'd;
        on_Const : 'c ->
                   Ast_visitors_ancestors.hint option ->
                   (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                   list -> 'd;
        on_Constant : 'c -> Ast_visitors_ancestors.gconst -> 'd;
        on_Constraint_as : 'c -> 'd; on_Constraint_eq : 'c -> 'd;
        on_Constraint_super : 'c -> 'd;
        on_Continue : 'c -> Ast_visitors_ancestors.pos_t -> int option -> 'd;
        on_Contravariant : 'c -> 'd; on_Covariant : 'c -> 'd;
        on_Cst_const : 'c -> 'd; on_Cst_define : 'c -> 'd;
        on_Ctrait : 'c -> 'd;
        on_Def_inline : 'c -> Ast_visitors_ancestors.def -> 'd;
        on_Default : 'c -> Ast_visitors_ancestors.block -> 'd;
        on_Diff : 'c -> 'd; on_Diff2 : 'c -> 'd;
        on_Do : 'c ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.expr -> 'd;
        on_Efun : 'c ->
                  Ast_visitors_ancestors.fun_ ->
                  (Ast_visitors_ancestors.id *
                   Ast_visitors_ancestors.is_reference)
                  list -> 'd;
        on_Eif : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr option ->
                 Ast_visitors_ancestors.expr -> 'd;
        on_Eq : 'c -> Ast_visitors_ancestors.bop option -> 'd;
        on_Eqeq : 'c -> 'd;
        on_Omitted: 'c -> 'd;
        on_Expr : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Expr_list : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_FAsync : 'c -> 'd; on_FAsyncGenerator : 'c -> 'd;
        on_FGenerator : 'c -> 'd; on_FSync : 'c -> 'd;
        on_Fallthrough : 'c -> 'd; on_False : 'c -> 'd; on_Final : 'c -> 'd;
        on_Float : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_For : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.block -> 'd;
        on_Foreach : 'c ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.pos_t option ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.block -> 'd;
        on_Fun : 'c -> Ast_visitors_ancestors.fun_ -> 'd; on_Gt : 'c -> 'd;
        on_GotoLabel : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_Goto : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_Markup : 'c ->
                    Ast_visitors_ancestors.pstring ->
                    Ast_visitors_ancestors.expr option -> 'd;
        on_Gte : 'c -> 'd; on_Gtgt : 'c -> 'd;
        on_Haccess : 'c ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id list -> 'd;
        on_Happly : 'c ->
                    Ast_visitors_ancestors.id ->
                    Ast_visitors_ancestors.hint list -> 'd;
        on_Hfun : 'c ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.is_reference ->
                  Ast_visitors_ancestors.hint -> 'd;
        on_Hoption : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_Hshape : 'c -> Ast_visitors_ancestors.shape_info -> 'd;
        on_Htuple : 'c -> Ast_visitors_ancestors.hint list -> 'd;
        on_Hsoft : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_Id : 'c -> Ast_visitors_ancestors.id -> 'd;
        on_Id_type_arguments : 'c ->
                               Ast_visitors_ancestors.id ->
                               Ast_visitors_ancestors.hint list -> 'd;
        on_If : 'c ->
                Ast_visitors_ancestors.expr ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.block -> 'd;
        on_Import : 'c ->
                    Ast_visitors_ancestors.import_flavor ->
                    Ast_visitors_ancestors.expr -> 'd;
        on_Include : 'c -> 'd; on_IncludeOnce : 'c -> 'd;
        on_InstanceOf : 'c ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr -> 'd;
        on_Int : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_Invariant : 'c -> 'd;
        on_Lfun : 'c -> Ast_visitors_ancestors.fun_ -> 'd;
        on_List : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_Cmp : 'c -> 'd;
        on_Lt : 'c -> 'd; on_Lte : 'c -> 'd; on_Ltlt : 'c -> 'd;
        on_Lvar : 'c -> Ast_visitors_ancestors.id -> 'd;
        on_Lvarvar : 'c -> int -> Ast_visitors_ancestors.id -> 'd;
        on_Method : 'c -> Ast_visitors_ancestors.method_ -> 'd;
        on_Minus : 'c -> 'd; on_MustExtend : 'c -> 'd;
        on_MustImplement : 'c -> 'd; on_NSNamespace : 'c -> 'd;
        on_NSClass : 'c -> 'd; on_NSClassAndNamespace : 'c -> 'd;
        on_NSConst : 'c -> 'd; on_NSFun : 'c -> 'd;
        on_Namespace : 'c ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.program -> 'd;
        on_SetNamespaceEnv : 'c ->
                       Ast_visitors_ancestors.nsenv -> 'd;
        on_NamespaceUse : 'c ->
                          (Ast_visitors_ancestors.ns_kind *
                           Ast_visitors_ancestors.id *
                           Ast_visitors_ancestors.id)
                          list -> 'd;
        on_New : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr list -> 'd;
        on_NewType : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_Noop : 'c -> 'd; on_Null : 'c -> 'd;
        on_NullCoalesce : 'c ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr -> 'd;
        on_OG_nullsafe : 'c -> 'd; on_OG_nullthrows : 'c -> 'd;
        on_Obj_get : 'c ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.og_null_flavor -> 'd;
        on_Percent : 'c -> 'd;
        on_Pipe : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.expr -> 'd;
        on_Plus : 'c -> 'd; on_Private : 'c -> 'd; on_Protected : 'c -> 'd;
        on_Public : 'c -> 'd; on_Require : 'c -> 'd;
        on_RequireOnce : 'c -> 'd;
        on_Return : 'c ->
                    Ast_visitors_ancestors.pos_t ->
                    Ast_visitors_ancestors.expr option -> 'd;
        on_SFclass_const : 'c ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring -> 'd;
        on_SFlit : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_Shape : 'c ->
                   (Ast_visitors_ancestors.shape_field_name *
                    Ast_visitors_ancestors.expr)
                   list -> 'd;
        on_Slash : 'c -> 'd; on_Star : 'c -> 'd; on_Starstar : 'c -> 'd;
        on_Static : 'c -> 'd;
        on_Static_var : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_Global_var : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_Stmt : 'c -> Ast_visitors_ancestors.stmt -> 'd;
        on_String : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_String2 : 'c -> Ast_visitors_ancestors.expr list -> 'd;
        on_Switch : 'c ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.case list -> 'd;
        on_Throw : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_True : 'c -> 'd;
        on_Try : 'c ->
                 Ast_visitors_ancestors.block ->
                 Ast_visitors_ancestors.catch list ->
                 Ast_visitors_ancestors.block -> 'd;
        on_TypeConst : 'c -> Ast_visitors_ancestors.typeconst -> 'd;
        on_Typedef : 'c -> Ast_visitors_ancestors.typedef -> 'd;
        on_Udecr : 'c -> 'd; on_Uincr : 'c -> 'd; on_Uminus : 'c -> 'd;
        on_Unop : 'c ->
                  Ast_visitors_ancestors.uop ->
                  Ast_visitors_ancestors.expr -> 'd;
        on_Unot : 'c -> 'd; on_Unsafe : 'c -> 'd;
        on_Unsafeexpr : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Updecr : 'c -> 'd; on_Upincr : 'c -> 'd; on_Uplus : 'c -> 'd;
        on_Uref : 'c -> 'd; on_Usplat : 'c -> 'd; on_Utild : 'c -> 'd;
        on_Usilence : 'c -> 'd;
        on_While : 'c ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.block -> 'd;
        on_XhpAttr : 'c ->
                     Ast_visitors_ancestors.hint option ->
                     Ast_visitors_ancestors.class_var ->
                     Ast_visitors_ancestors.is_reference ->
                     (Ast_visitors_ancestors.pos_t *
                      Ast_visitors_ancestors.expr list)
                     option -> 'd;
        on_XhpAttrUse : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_XhpCategory : 'c -> Ast_visitors_ancestors.pstring list -> 'd;


        on_XhpChild : 'c -> Ast_visitors_ancestors.xhp_child -> 'd;
        on_xhp_child : 'c -> Ast_visitors_ancestors.xhp_child -> 'd;
        on_ChildName : 'c -> Ast_visitors_ancestors.id -> 'd;
        on_ChildList : 'c -> Ast_visitors_ancestors.xhp_child list -> 'd;
        on_ChildUnary : 'c -> Ast_visitors_ancestors.xhp_child ->
          Ast_visitors_ancestors.xhp_child_op -> 'd;
        on_ChildBinary : 'c -> Ast_visitors_ancestors.xhp_child ->
          Ast_visitors_ancestors.xhp_child -> 'd;
        on_xhp_child_op : 'c -> Ast_visitors_ancestors.xhp_child_op -> 'd;
        on_ChildStar : 'c -> 'd;
        on_ChildPlus : 'c -> 'd;
        on_ChildQuestion : 'c -> 'd;

        on_Xml : 'c ->
                 Ast_visitors_ancestors.id ->
                 (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                 list -> Ast_visitors_ancestors.expr list -> 'd;
        on_LogXor : 'c -> 'd;
        on_Xor : 'c -> 'd;
        on_Yield : 'c -> Ast_visitors_ancestors.afield -> 'd;
        on_Yield_from : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_Yield_break : 'c -> 'd;
        on_afield : 'c -> Ast_visitors_ancestors.afield -> 'd;
        on_any : 'c -> Ast_visitors_ancestors.any -> 'd;
        on_as_expr : 'c -> Ast_visitors_ancestors.as_expr -> 'd;
        on_attr : 'c ->
                  Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
                  'd;
        on_block : 'c -> Ast_visitors_ancestors.block -> 'd;
        on_bop : 'c -> Ast_visitors_ancestors.bop -> 'd;
        on_ca_field : 'c -> Ast_visitors_ancestors.ca_field -> 'd;
        on_ca_type : 'c -> Ast_visitors_ancestors.ca_type -> 'd;
        on_case : 'c -> Ast_visitors_ancestors.case -> 'd;
        on_catch : 'c -> Ast_visitors_ancestors.catch -> 'd;
        on_class_ : 'c -> Ast_visitors_ancestors.class_ -> 'd;
        on_class_attr : 'c -> Ast_visitors_ancestors.class_attr -> 'd;
        on_class_elt : 'c -> Ast_visitors_ancestors.class_elt -> 'd;
        on_class_kind : 'c -> Ast_visitors_ancestors.class_kind -> 'd;
        on_class_var : 'c -> Ast_visitors_ancestors.class_var -> 'd;
        on_constraint_kind : 'c ->
                             Ast_visitors_ancestors.constraint_kind -> 'd;
        on_cst_kind : 'c -> Ast_visitors_ancestors.cst_kind -> 'd;
        on_def : 'c -> Ast_visitors_ancestors.def -> 'd;
        on_enum_ : 'c -> Ast_visitors_ancestors.enum_ -> 'd;
        on_expr : 'c -> Ast_visitors_ancestors.expr -> 'd;
        on_expr_ : 'c -> Ast_visitors_ancestors.expr_ -> 'd;
        on_field : 'c ->
                   Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
                   'd;
        on_fun_ : 'c -> Ast_visitors_ancestors.fun_ -> 'd;
        on_fun_kind : 'c -> Ast_visitors_ancestors.fun_kind -> 'd;
        on_fun_param : 'c -> Ast_visitors_ancestors.fun_param -> 'd;
        on_gconst : 'c -> Ast_visitors_ancestors.gconst -> 'd;
        on_hint : 'c -> Ast_visitors_ancestors.hint -> 'd;
        on_hint_ : 'c -> Ast_visitors_ancestors.hint_ -> 'd;
        on_id : 'c -> Ast_visitors_ancestors.id -> 'd;
        on_import_flavor : 'c -> Ast_visitors_ancestors.import_flavor -> 'd;
        on_is_reference : 'c -> Ast_visitors_ancestors.is_reference -> 'd;
        on_is_variadic : 'c -> Ast_visitors_ancestors.is_reference -> 'd;
        on_kind : 'c -> Ast_visitors_ancestors.kind -> 'd;
        on_method_ : 'c -> Ast_visitors_ancestors.method_ -> 'd;
        on_ns_kind : 'c -> Ast_visitors_ancestors.ns_kind -> 'd;
        on_og_null_flavor : 'c -> Ast_visitors_ancestors.og_null_flavor -> 'd;
        on_program : 'c -> Ast_visitors_ancestors.program -> 'd;
        on_pstring : 'c -> Ast_visitors_ancestors.pstring -> 'd;
        on_shape_field : 'c -> Ast_visitors_ancestors.shape_field -> 'd;
        on_shape_field_name : 'c ->
                              Ast_visitors_ancestors.shape_field_name -> 'd;
        on_stmt : 'c -> Ast_visitors_ancestors.stmt -> 'd;
        on_tconstraint : 'c -> Ast_visitors_ancestors.tconstraint -> 'd;
        on_tparam : 'c -> Ast_visitors_ancestors.tparam -> 'd;
        on_trait_req_kind : 'c -> Ast_visitors_ancestors.trait_req_kind -> 'd;
        on_typeconst : 'c -> Ast_visitors_ancestors.typeconst -> 'd;
        on_typedef : 'c -> Ast_visitors_ancestors.typedef -> 'd;
        on_typedef_kind : 'c -> Ast_visitors_ancestors.typedef_kind -> 'd;
        on_uop : 'c -> Ast_visitors_ancestors.uop -> 'd;
        on_user_attribute : 'c -> Ast_visitors_ancestors.user_attribute -> 'd;
        on_variance : 'c -> Ast_visitors_ancestors.variance -> 'd; .. >
    method private virtual add : 'd -> 'd -> 'd
    method private virtual e : 'd
    method on_ADef : 'c -> Ast_visitors_ancestors.def -> 'd
    method on_AExpr : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_AFkvalue :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_AFvalue : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_AHint : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_AMpamp : 'c -> 'd
    method on_AProgram : 'c -> Ast_visitors_ancestors.program -> 'd
    method on_AStmt : 'c -> Ast_visitors_ancestors.stmt -> 'd
    method on_AbsConst :
      'c ->
      Ast_visitors_ancestors.hint option -> Ast_visitors_ancestors.id -> 'd
    method on_Abstract : 'c -> 'd
    method on_Alias : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_Amp : 'c -> 'd
    method on_Array : 'c -> Ast_visitors_ancestors.afield list -> 'd
    method on_Darray :
      'c ->
      (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr) list ->
      'd
    method on_Varray : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_Array_get :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr option -> 'd
    method on_As_kv :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_As_v : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Attributes : 'c -> Ast_visitors_ancestors.class_attr list -> 'd
    method on_Await : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_BArbar : 'c -> 'd
    method on_Bar : 'c -> 'd
    method on_Binop :
      'c ->
      Ast_visitors_ancestors.bop ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_Block : 'c -> Ast_visitors_ancestors.block -> 'd
    method on_BracedExpr : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Break : 'c -> Ast_visitors_ancestors.pos_t -> int option -> 'd
    method on_CA_enum : 'c -> string list -> 'd
    method on_CA_field : 'c -> Ast_visitors_ancestors.ca_field -> 'd
    method on_CA_hint : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_CA_name : 'c -> Ast_visitors_ancestors.id -> 'd
    method on_Cabstract : 'c -> 'd
    method on_Call :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> 'd
    method on_Case :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> 'd
    method on_Cast :
      'c -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.expr -> 'd
    method on_Cenum : 'c -> 'd
    method on_Cinterface : 'c -> 'd
    method on_Class : 'c -> Ast_visitors_ancestors.class_ -> 'd
    method on_ClassTraitRequire :
      'c ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.hint -> 'd
    method on_ClassUse : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_ClassUseAlias :
      'c ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.kind option ->
      'd
    method on_ClassUsePrecedence :
      'c ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id list ->
      'd
    method on_ClassVars :
      'c ->
      Ast_visitors_ancestors.kind list ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var list ->
      string option -> 'd
    method on_Class_const :
      'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.pstring -> 'd
    method on_Class_get :
      'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr -> 'd
    method on_Clone : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Cmp : 'c -> 'd
    method on_Cnormal : 'c -> 'd
    method on_Collection :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.afield list -> 'd
    method on_Const :
      'c ->
      Ast_visitors_ancestors.hint option ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list -> 'd
    method on_Constant : 'c -> Ast_visitors_ancestors.gconst -> 'd
    method on_Constraint_as : 'c -> 'd
    method on_Constraint_eq : 'c -> 'd
    method on_Constraint_super : 'c -> 'd
    method on_Continue : 'c -> Ast_visitors_ancestors.pos_t -> int option -> 'd
    method on_Contravariant : 'c -> 'd
    method on_Covariant : 'c -> 'd
    method on_Cst_const : 'c -> 'd
    method on_Cst_define : 'c -> 'd
    method on_Ctrait : 'c -> 'd
    method on_Def_inline : 'c -> Ast_visitors_ancestors.def -> 'd
    method on_Default : 'c -> Ast_visitors_ancestors.block -> 'd
    method on_Diff : 'c -> 'd
    method on_Diff2 : 'c -> 'd
    method on_Do :
      'c -> Ast_visitors_ancestors.block -> Ast_visitors_ancestors.expr -> 'd
    method on_Dot : 'c -> 'd
    method on_EQeqeq : 'c -> 'd
    method on_Efun :
      'c ->
      Ast_visitors_ancestors.fun_ ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.is_reference) list ->
      'd
    method on_Eif :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option -> Ast_visitors_ancestors.expr -> 'd
    method on_Eq : 'c -> Ast_visitors_ancestors.bop option -> 'd
    method on_Eqeq : 'c -> 'd
    method on_Omitted: 'c -> 'd
    method on_Expr : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Expr_list : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_FAsync : 'c -> 'd
    method on_FAsyncGenerator : 'c -> 'd
    method on_FGenerator : 'c -> 'd
    method on_FSync : 'c -> 'd
    method on_Fallthrough : 'c -> 'd
    method on_False : 'c -> 'd
    method private on_FileInfo_mode :
      'c -> Ast_visitors_ancestors.fimode -> 'd
    method on_Final : 'c -> 'd
    method on_Float : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_For :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> 'd
    method on_Foreach :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.pos_t option ->
      Ast_visitors_ancestors.as_expr -> Ast_visitors_ancestors.block -> 'd
    method on_Fun : 'c -> Ast_visitors_ancestors.fun_ -> 'd
    method on_GotoLabel : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_Goto : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_Gt : 'c -> 'd
    method on_Gte : 'c -> 'd
    method on_Gtgt : 'c -> 'd
    method on_Haccess :
      'c ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.id list -> 'd
    method on_Happly :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.hint list -> 'd
    method on_Hfun :
      'c ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.hint -> 'd
    method on_Hoption : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_Hshape : 'c -> Ast_visitors_ancestors.shape_info -> 'd
    method on_Htuple : 'c -> Ast_visitors_ancestors.hint list -> 'd
    method on_Hsoft : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_Id : 'c -> Ast_visitors_ancestors.id -> 'd
    method on_Id_type_arguments :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.hint list -> 'd
    method on_If :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.block -> 'd
    method on_Import :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.expr -> 'd
    method on_Include : 'c -> 'd
    method on_IncludeOnce : 'c -> 'd
    method on_InstanceOf :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_Int : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_Invariant : 'c -> 'd
    method on_Lfun : 'c -> Ast_visitors_ancestors.fun_ -> 'd
    method on_List : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_Lt : 'c -> 'd
    method on_Lte : 'c -> 'd
    method on_Ltlt : 'c -> 'd
    method on_Lvar : 'c -> Ast_visitors_ancestors.id -> 'd
    method on_Lvarvar : 'c -> int -> Ast_visitors_ancestors.id -> 'd
    method on_Method : 'c -> Ast_visitors_ancestors.method_ -> 'd
    method on_Minus : 'c -> 'd
    method on_MustExtend : 'c -> 'd
    method on_MustImplement : 'c -> 'd
    method on_NSClass : 'c -> 'd
    method on_NSNamespace : 'c -> 'd
    method on_NSClassAndNamespace : 'c -> 'd
    method on_NSConst : 'c -> 'd
    method on_NSFun : 'c -> 'd
    method on_Namespace :
      'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.program -> 'd
    method on_SetNamespaceEnv :
      'c ->
      Ast_visitors_ancestors.nsenv -> 'd
    method on_NamespaceUse :
      'c ->
      (Ast_visitors_ancestors.ns_kind * Ast_visitors_ancestors.id *
       Ast_visitors_ancestors.id)
      list -> 'd
    method private on_Namespace_env :
      'c -> Ast_visitors_ancestors.nsenv -> 'd
    method on_New :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> 'd
    method on_NewType : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_Noop : 'c -> 'd
    method on_Null : 'c -> 'd
    method on_NullCoalesce :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_OG_nullsafe : 'c -> 'd
    method on_OG_nullthrows : 'c -> 'd
    method on_Obj_get :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.og_null_flavor -> 'd
    method on_Percent : 'c -> 'd
    method on_Pipe :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> 'd
    method on_Plus : 'c -> 'd
    method private on_Pos_t : 'c -> Ast_visitors_ancestors.pos_t -> 'd
    method on_Private : 'c -> 'd
    method on_Protected : 'c -> 'd
    method on_Public : 'c -> 'd
    method on_Require : 'c -> 'd
    method on_RequireOnce : 'c -> 'd
    method on_Return :
      'c ->
      Ast_visitors_ancestors.pos_t ->
      Ast_visitors_ancestors.expr option -> 'd
    method on_SFclass_const :
      'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.pstring -> 'd
    method on_SFlit : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_Shape :
      'c ->
      (Ast_visitors_ancestors.shape_field_name * Ast_visitors_ancestors.expr)
      list -> 'd
    method on_Slash : 'c -> 'd
    method on_Star : 'c -> 'd
    method on_Starstar : 'c -> 'd
    method on_Static : 'c -> 'd
    method on_Static_var : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_Global_var : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_Stmt : 'c -> Ast_visitors_ancestors.stmt -> 'd
    method on_String : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_String2 : 'c -> Ast_visitors_ancestors.expr list -> 'd
    method on_Switch :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.case list -> 'd
    method on_Throw : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_True : 'c -> 'd
    method on_Try :
      'c ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.catch list -> Ast_visitors_ancestors.block -> 'd
    method on_TypeConst : 'c -> Ast_visitors_ancestors.typeconst -> 'd
    method on_Typedef : 'c -> Ast_visitors_ancestors.typedef -> 'd
    method on_Udecr : 'c -> 'd
    method on_Uincr : 'c -> 'd
    method on_Uminus : 'c -> 'd
    method on_Unop :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.expr -> 'd
    method on_Unot : 'c -> 'd
    method on_Unsafe : 'c -> 'd
    method on_Unsafeexpr : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Updecr : 'c -> 'd
    method on_Upincr : 'c -> 'd
    method on_Uplus : 'c -> 'd
    method on_Uref : 'c -> 'd
    method on_Usplat : 'c -> 'd
    method on_Usilence : 'c -> 'd
    method on_Utild : 'c -> 'd
    method on_While :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> 'd
    method on_XhpAttr :
      'c ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var ->
      Ast_visitors_ancestors.is_reference ->
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr list)
      option -> 'd
    method on_XhpAttrUse : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_XhpCategory : 'c -> Ast_visitors_ancestors.pstring list -> 'd
    method on_XhpChild : 'c -> Ast_visitors_ancestors.xhp_child -> 'd
    method on_xhp_child : 'c -> Ast_visitors_ancestors.xhp_child -> 'd
    method on_ChildName : 'c -> Ast_visitors_ancestors.id -> 'd
    method on_ChildList : 'c -> Ast_visitors_ancestors.xhp_child list -> 'd
    method on_ChildUnary : 'c -> Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child_op -> 'd
    method on_ChildBinary : 'c -> Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child -> 'd
    method on_xhp_child_op : 'c -> Ast_visitors_ancestors.xhp_child_op -> 'd
    method on_ChildStar : 'c -> 'd
    method on_ChildPlus : 'c -> 'd
    method on_ChildQuestion : 'c -> 'd
    method on_Xml :
      'c ->
      Ast_visitors_ancestors.id ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr list -> 'd
    method on_LogXor : 'c -> 'd
    method on_Xor : 'c -> 'd
    method on_Yield : 'c -> Ast_visitors_ancestors.afield -> 'd
    method on_Yield_from : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_Yield_break : 'c -> 'd
    method on_afield : 'c -> Ast_visitors_ancestors.afield -> 'd
    method on_any : 'c -> Ast_visitors_ancestors.any -> 'd
    method on_as_expr : 'c -> Ast_visitors_ancestors.as_expr -> 'd
    method on_attr :
      'c -> Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr -> 'd
    method on_block : 'c -> Ast_visitors_ancestors.block -> 'd
    method private on_bool : 'c -> Ast_visitors_ancestors.is_reference -> 'd
    method on_bop : 'c -> Ast_visitors_ancestors.bop -> 'd
    method on_ca_field : 'c -> Ast_visitors_ancestors.ca_field -> 'd
    method on_ca_type : 'c -> Ast_visitors_ancestors.ca_type -> 'd
    method on_case : 'c -> Ast_visitors_ancestors.case -> 'd
    method on_catch : 'c -> Ast_visitors_ancestors.catch -> 'd
    method on_class_ : 'c -> Ast_visitors_ancestors.class_ -> 'd
    method on_class_attr : 'c -> Ast_visitors_ancestors.class_attr -> 'd
    method on_class_elt : 'c -> Ast_visitors_ancestors.class_elt -> 'd
    method on_class_kind : 'c -> Ast_visitors_ancestors.class_kind -> 'd
    method on_class_var : 'c -> Ast_visitors_ancestors.class_var -> 'd
    method on_constraint_kind :
      'c -> Ast_visitors_ancestors.constraint_kind -> 'd
    method on_cst_kind : 'c -> Ast_visitors_ancestors.cst_kind -> 'd
    method on_def : 'c -> Ast_visitors_ancestors.def -> 'd
    method on_enum_ : 'c -> Ast_visitors_ancestors.enum_ -> 'd
    method on_expr : 'c -> Ast_visitors_ancestors.expr -> 'd
    method on_expr_ : 'c -> Ast_visitors_ancestors.expr_ -> 'd
    method on_field :
      'c -> Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr -> 'd
    method on_fun_ : 'c -> Ast_visitors_ancestors.fun_ -> 'd
    method on_fun_kind : 'c -> Ast_visitors_ancestors.fun_kind -> 'd
    method on_fun_param : 'c -> Ast_visitors_ancestors.fun_param -> 'd
    method on_gconst : 'c -> Ast_visitors_ancestors.gconst -> 'd
    method on_hint : 'c -> Ast_visitors_ancestors.hint -> 'd
    method on_hint_ : 'c -> Ast_visitors_ancestors.hint_ -> 'd
    method on_id : 'c -> Ast_visitors_ancestors.id -> 'd
    method on_import_flavor :
      'c -> Ast_visitors_ancestors.import_flavor -> 'd
    method private on_int : 'c -> int -> 'd
    method on_is_reference : 'c -> Ast_visitors_ancestors.is_reference -> 'd
    method on_is_variadic : 'c -> Ast_visitors_ancestors.is_reference -> 'd
    method on_kind : 'c -> Ast_visitors_ancestors.kind -> 'd
    method private on_list :
      'env 'a. ('env -> 'a -> 'd) -> 'env -> 'a list -> 'd
    method on_method_ : 'c -> Ast_visitors_ancestors.method_ -> 'd
    method on_ns_kind : 'c -> Ast_visitors_ancestors.ns_kind -> 'd
    method on_og_null_flavor :
      'c -> Ast_visitors_ancestors.og_null_flavor -> 'd
    method private on_option :
      'env 'a. ('env -> 'a -> 'd) -> 'env -> 'a option -> 'd
    method on_program : 'c -> Ast_visitors_ancestors.program -> 'd
    method on_pstring : 'c -> Ast_visitors_ancestors.pstring -> 'd
    method on_shape_field : 'c -> Ast_visitors_ancestors.shape_field -> 'd
    method on_shape_field_name :
      'c -> Ast_visitors_ancestors.shape_field_name -> 'd
    method on_stmt : 'c -> Ast_visitors_ancestors.stmt -> 'd
    method private on_string : 'c -> string -> 'd
    method on_tconstraint : 'c -> Ast_visitors_ancestors.tconstraint -> 'd
    method on_tparam : 'c -> Ast_visitors_ancestors.tparam -> 'd
    method on_trait_req_kind :
      'c -> Ast_visitors_ancestors.trait_req_kind -> 'd
    method on_typeconst : 'c -> Ast_visitors_ancestors.typeconst -> 'd
    method on_typedef : 'c -> Ast_visitors_ancestors.typedef -> 'd
    method on_typedef_kind : 'c -> Ast_visitors_ancestors.typedef_kind -> 'd
    method on_uop : 'c -> Ast_visitors_ancestors.uop -> 'd
    method on_user_attribute :
      'c -> Ast_visitors_ancestors.user_attribute -> 'd
    method on_variance : 'c -> Ast_visitors_ancestors.variance -> 'd
  end

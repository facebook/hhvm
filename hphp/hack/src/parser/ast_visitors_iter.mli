class virtual ['b] iter :
  object ('b)
    constraint 'b =
      < on_ADef : 'c -> Ast_visitors_ancestors.def -> unit;
        on_AExpr : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_AFkvalue : 'c ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.expr -> unit;
        on_AFvalue : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_AHint : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_AMpamp : 'c -> unit;
        on_AProgram : 'c -> Ast_visitors_ancestors.program -> unit;
        on_AStmt : 'c -> Ast_visitors_ancestors.stmt -> unit;
        on_AbsConst : 'c ->
                      Ast_visitors_ancestors.hint option ->
                      Ast_visitors_ancestors.id -> unit;
        on_Abstract : 'c -> unit;
        on_Alias : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_Amp : 'c -> unit;
        on_Array : 'c -> Ast_visitors_ancestors.afield list -> unit;
        on_Darray : 'c ->
                    (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr)
                    list ->
                    unit;
        on_Varray : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_Array_get : 'c ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr option -> unit;
        on_As_kv : 'c ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr -> unit;
        on_As_v : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Attributes : 'c -> Ast_visitors_ancestors.class_attr list -> unit;
        on_Await : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_BArbar : 'c -> unit; on_Bar : 'c -> unit;
        on_Binop : 'c ->
                   Ast_visitors_ancestors.bop ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr -> unit;
        on_Block : 'c -> Ast_visitors_ancestors.block -> unit;
        on_BracedExpr : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Break : 'c -> Ast_visitors_ancestors.pos_t -> int option -> unit;
        on_CA_enum : 'c -> string list -> unit;
        on_CA_field : 'c -> Ast_visitors_ancestors.ca_field -> unit;
        on_CA_hint : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_CA_name : 'c -> Ast_visitors_ancestors.id -> unit;
        on_Cabstract : 'c -> unit;
        on_Call : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr list -> unit;
        on_Case : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.block -> unit;
        on_Cast : 'c ->
                  Ast_visitors_ancestors.hint ->
                  Ast_visitors_ancestors.expr -> unit;
        on_Cenum : 'c -> unit; on_Cinterface : 'c -> unit;
        on_Class : 'c -> Ast_visitors_ancestors.class_ -> unit;
        on_ClassTraitRequire : 'c ->
                               Ast_visitors_ancestors.trait_req_kind ->
                               Ast_visitors_ancestors.hint -> unit;
        on_ClassUse : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_ClassUseAlias : 'c ->
                           Ast_visitors_ancestors.id option ->
                           Ast_visitors_ancestors.pstring ->
                           Ast_visitors_ancestors.id option ->
                           Ast_visitors_ancestors.kind option ->
                           unit;
        on_ClassUsePrecedence : 'c ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring ->
                           Ast_visitors_ancestors.id list ->
                           unit;
        on_ClassVars : 'c ->
                       Ast_visitors_ancestors.kind list ->
                       Ast_visitors_ancestors.hint option ->
                       Ast_visitors_ancestors.class_var list -> unit;
        on_Class_const : 'c ->
                         Ast_visitors_ancestors.id ->
                         Ast_visitors_ancestors.pstring -> unit;
        on_Class_get : 'c ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.expr -> unit;
        on_Clone : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Cnormal : 'c -> unit;
        on_Collection : 'c ->
                        Ast_visitors_ancestors.id ->
                        Ast_visitors_ancestors.afield list -> unit;
        on_Const : 'c ->
                   Ast_visitors_ancestors.hint option ->
                   (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                   list -> unit;
        on_Constant : 'c -> Ast_visitors_ancestors.gconst -> unit;
        on_Constraint_as : 'c -> unit; on_Constraint_eq : 'c -> unit;
        on_Constraint_super : 'c -> unit;
        on_Continue : 'c -> Ast_visitors_ancestors.pos_t -> int option -> unit;
        on_Contravariant : 'c -> unit; on_Covariant : 'c -> unit;
        on_Cst_const : 'c -> unit; on_Cst_define : 'c -> unit;
        on_Ctrait : 'c -> unit;
        on_Def_inline : 'c ->
                                                Ast_visitors_ancestors.def ->
                                                unit;
        on_Default : 'c -> Ast_visitors_ancestors.block -> unit;
        on_Diff : 'c -> unit; on_Diff2 : 'c -> unit;
        on_Do : 'c ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.expr -> unit;
        on_EQeqeq : 'c -> unit;
        on_Efun : 'c ->
                  Ast_visitors_ancestors.fun_ ->
                  (Ast_visitors_ancestors.id *
                   Ast_visitors_ancestors.is_reference)
                  list -> unit;
        on_Eif : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr option ->
                 Ast_visitors_ancestors.expr -> unit;
        on_Eq : 'c -> Ast_visitors_ancestors.bop option -> unit;
        on_Eqeq : 'c -> unit;
        on_Expr : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Omitted : 'c -> unit;
        on_Expr_list : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_FAsync : 'c -> unit; on_FAsyncGenerator : 'c -> unit;
        on_FGenerator : 'c -> unit; on_FSync : 'c -> unit;
        on_Fallthrough : 'c -> unit; on_False : 'c -> unit;
        on_Final : 'c -> unit;
        on_Float : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_For : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.block -> unit;
        on_Foreach : 'c ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.pos_t option ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.block -> unit;
        on_Fun : 'c -> Ast_visitors_ancestors.fun_ -> unit;
        on_GotoLabel : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_Goto : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_Markup : 'c ->
                    Ast_visitors_ancestors.pstring ->
                    Ast_visitors_ancestors.expr option -> unit;
        on_Gt : 'c -> unit; on_Gte : 'c -> unit; on_Gtgt : 'c -> unit;
        on_Haccess : 'c ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id list -> unit;
        on_Happly : 'c ->
                    Ast_visitors_ancestors.id ->
                    Ast_visitors_ancestors.hint list -> unit;
        on_Hfun : 'c ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.is_reference ->
                  Ast_visitors_ancestors.hint -> unit;
        on_Hoption : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_Hshape : 'c -> Ast_visitors_ancestors.shape_info -> unit;
        on_Htuple : 'c -> Ast_visitors_ancestors.hint list -> unit;
        on_Hsoft : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_Id : 'c -> Ast_visitors_ancestors.id -> unit;
        on_Id_type_arguments : 'c ->
                               Ast_visitors_ancestors.id ->
                               Ast_visitors_ancestors.hint list -> unit;
        on_If : 'c ->
                Ast_visitors_ancestors.expr ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.block -> unit;
        on_Import : 'c ->
                    Ast_visitors_ancestors.import_flavor ->
                    Ast_visitors_ancestors.expr -> unit;
        on_Include : 'c -> unit; on_IncludeOnce : 'c -> unit;
        on_InstanceOf : 'c ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr -> unit;
        on_Int : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_Invariant : 'c -> unit;
        on_Lfun : 'c -> Ast_visitors_ancestors.fun_ -> unit;
        on_List : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_Lt : 'c -> unit; on_Lte : 'c -> unit; on_Ltlt : 'c -> unit;
        on_Cmp : 'c -> unit;
        on_Lvar : 'c -> Ast_visitors_ancestors.id -> unit;
        on_Lvarvar : 'c -> int -> Ast_visitors_ancestors.id -> unit;
        on_Method : 'c -> Ast_visitors_ancestors.method_ -> unit;
        on_Minus : 'c -> unit; on_MustExtend : 'c -> unit;
        on_MustImplement : 'c -> unit; on_NSNamespace: 'c -> unit;
        on_NSClass: 'c -> unit; on_NSClassAndNamespace : 'c -> unit;
        on_NSConst : 'c -> unit; on_NSFun : 'c -> unit;
        on_Namespace : 'c ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.program -> unit;
        on_SetNamespaceEnv : 'c ->
                             Ast_visitors_ancestors.nsenv -> unit;
        on_NamespaceUse : 'c ->
                          (Ast_visitors_ancestors.ns_kind *
                           Ast_visitors_ancestors.id *
                           Ast_visitors_ancestors.id)
                          list -> unit;
        on_New : 'c ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr list -> unit;
        on_NewType : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_Noop : 'c -> unit; on_Null : 'c -> unit;
        on_NullCoalesce : 'c ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr -> unit;
        on_OG_nullsafe : 'c -> unit; on_OG_nullthrows : 'c -> unit;
        on_Obj_get : 'c ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.og_null_flavor -> unit;
        on_Percent : 'c -> unit;
        on_Pipe : 'c ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.expr -> unit;
        on_Plus : 'c -> unit; on_Private : 'c -> unit;
        on_Protected : 'c -> unit; on_Public : 'c -> unit;
        on_Require : 'c -> unit; on_RequireOnce : 'c -> unit;
        on_Return : 'c ->
                    Ast_visitors_ancestors.pos_t ->
                    Ast_visitors_ancestors.expr option -> unit;
        on_SFclass_const : 'c ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring -> unit;
        on_SFlit : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_Shape : 'c ->
                   (Ast_visitors_ancestors.shape_field_name *
                    Ast_visitors_ancestors.expr)
                   list -> unit;
        on_Slash : 'c -> unit; on_Star : 'c -> unit;
        on_Starstar : 'c -> unit; on_Static : 'c -> unit;
        on_Static_var : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_Global_var : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_Stmt : 'c -> Ast_visitors_ancestors.stmt -> unit;
        on_String : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_String2 : 'c -> Ast_visitors_ancestors.expr list -> unit;
        on_Switch : 'c ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.case list -> unit;
        on_Throw : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_True : 'c -> unit;
        on_Try : 'c ->
                 Ast_visitors_ancestors.block ->
                 Ast_visitors_ancestors.catch list ->
                 Ast_visitors_ancestors.block -> unit;
        on_TypeConst : 'c -> Ast_visitors_ancestors.typeconst -> unit;
        on_Typedef : 'c -> Ast_visitors_ancestors.typedef -> unit;
        on_Udecr : 'c -> unit; on_Uincr : 'c -> unit; on_Uminus : 'c -> unit;
        on_Unop : 'c ->
                  Ast_visitors_ancestors.uop ->
                  Ast_visitors_ancestors.expr -> unit;
        on_Unot : 'c -> unit; on_Unsafe : 'c -> unit;
        on_Unsafeexpr : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Updecr : 'c -> unit; on_Upincr : 'c -> unit;
        on_Uplus : 'c -> unit; on_Uref : 'c -> unit; on_Usplat : 'c -> unit;
        on_Usilence : 'c -> unit;
        on_Utild : 'c -> unit;
        on_While : 'c ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.block -> unit;
        on_XhpAttr : 'c ->
                     Ast_visitors_ancestors.hint option ->
                     Ast_visitors_ancestors.class_var ->
                     Ast_visitors_ancestors.is_reference ->
                     (Ast_visitors_ancestors.pos_t *
                      Ast_visitors_ancestors.expr list)
                     option -> unit;
        on_XhpAttrUse : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_XhpCategory : 'c -> Ast_visitors_ancestors.pstring list -> unit;
        on_XhpChild : 'c -> Ast_visitors_ancestors.xhp_child -> unit;
        on_xhp_child : 'c -> Ast_visitors_ancestors.xhp_child -> unit;
        on_ChildName : 'c -> Ast_visitors_ancestors.id -> unit;
        on_ChildList : 'c -> Ast_visitors_ancestors.xhp_child list -> unit;
        on_ChildUnary : 'c -> Ast_visitors_ancestors.xhp_child ->
          Ast_visitors_ancestors.xhp_child_op -> unit;
        on_ChildBinary : 'c -> Ast_visitors_ancestors.xhp_child ->
          Ast_visitors_ancestors.xhp_child -> unit;
        on_xhp_child_op : 'c -> Ast_visitors_ancestors.xhp_child_op -> unit;
        on_ChildStar : 'c -> unit;
        on_ChildPlus : 'c -> unit;
        on_ChildQuestion : 'c -> unit;

        on_Xml : 'c ->
                 Ast_visitors_ancestors.id ->
                 (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                 list -> Ast_visitors_ancestors.expr list -> unit;
        on_LogXor : 'c -> unit;
        on_Xor : 'c -> unit;
        on_Yield : 'c -> Ast_visitors_ancestors.afield -> unit;
        on_Yield_from : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_Yield_break : 'c -> unit;
        on_afield : 'c -> Ast_visitors_ancestors.afield -> unit;
        on_any : 'c -> Ast_visitors_ancestors.any -> unit;
        on_as_expr : 'c -> Ast_visitors_ancestors.as_expr -> unit;
        on_attr : 'c ->
                  Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
                  unit;
        on_block : 'c -> Ast_visitors_ancestors.block -> unit;
        on_bop : 'c -> Ast_visitors_ancestors.bop -> unit;
        on_ca_field : 'c -> Ast_visitors_ancestors.ca_field -> unit;
        on_ca_type : 'c -> Ast_visitors_ancestors.ca_type -> unit;
        on_case : 'c -> Ast_visitors_ancestors.case -> unit;
        on_catch : 'c -> Ast_visitors_ancestors.catch -> unit;
        on_class_ : 'c -> Ast_visitors_ancestors.class_ -> unit;
        on_class_attr : 'c -> Ast_visitors_ancestors.class_attr -> unit;
        on_class_elt : 'c -> Ast_visitors_ancestors.class_elt -> unit;
        on_class_kind : 'c -> Ast_visitors_ancestors.class_kind -> unit;
        on_class_var : 'c -> Ast_visitors_ancestors.class_var -> unit;
        on_constraint_kind : 'c ->
                             Ast_visitors_ancestors.constraint_kind -> unit;
        on_cst_kind : 'c -> Ast_visitors_ancestors.cst_kind -> unit;
        on_def : 'c -> Ast_visitors_ancestors.def -> unit;
        on_enum_ : 'c -> Ast_visitors_ancestors.enum_ -> unit;
        on_expr : 'c -> Ast_visitors_ancestors.expr -> unit;
        on_expr_ : 'c -> Ast_visitors_ancestors.expr_ -> unit;
        on_field : 'c ->
                   Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
                   unit;
        on_fun_ : 'c -> Ast_visitors_ancestors.fun_ -> unit;
        on_fun_kind : 'c -> Ast_visitors_ancestors.fun_kind -> unit;
        on_fun_param : 'c -> Ast_visitors_ancestors.fun_param -> unit;
        on_gconst : 'c -> Ast_visitors_ancestors.gconst -> unit;
        on_hint : 'c -> Ast_visitors_ancestors.hint -> unit;
        on_hint_ : 'c -> Ast_visitors_ancestors.hint_ -> unit;
        on_id : 'c -> Ast_visitors_ancestors.id -> unit;
        on_import_flavor : 'c -> Ast_visitors_ancestors.import_flavor -> unit;
        on_is_reference : 'c -> Ast_visitors_ancestors.is_reference -> unit;
        on_is_variadic : 'c -> Ast_visitors_ancestors.is_reference -> unit;
        on_kind : 'c -> Ast_visitors_ancestors.kind -> unit;
        on_method_ : 'c -> Ast_visitors_ancestors.method_ -> unit;
        on_ns_kind : 'c -> Ast_visitors_ancestors.ns_kind -> unit;
        on_og_null_flavor : 'c ->
                            Ast_visitors_ancestors.og_null_flavor -> unit;
        on_program : 'c -> Ast_visitors_ancestors.program -> unit;
        on_pstring : 'c -> Ast_visitors_ancestors.pstring -> unit;
        on_shape_field : 'c -> Ast_visitors_ancestors.shape_field -> unit;
        on_shape_field_name : 'c ->
                              Ast_visitors_ancestors.shape_field_name -> unit;
        on_stmt : 'c -> Ast_visitors_ancestors.stmt -> unit;
        on_tconstraint : 'c -> Ast_visitors_ancestors.tconstraint -> unit;
        on_tparam : 'c -> Ast_visitors_ancestors.tparam -> unit;
        on_trait_req_kind : 'c ->
                            Ast_visitors_ancestors.trait_req_kind -> unit;
        on_typeconst : 'c -> Ast_visitors_ancestors.typeconst -> unit;
        on_typedef : 'c -> Ast_visitors_ancestors.typedef -> unit;
        on_typedef_kind : 'c -> Ast_visitors_ancestors.typedef_kind -> unit;
        on_uop : 'c -> Ast_visitors_ancestors.uop -> unit;
        on_user_attribute : 'c ->
                            Ast_visitors_ancestors.user_attribute -> unit;
        on_variance : 'c -> Ast_visitors_ancestors.variance -> unit; .. >
    method on_ADef : 'c -> Ast_visitors_ancestors.def -> unit
    method on_AExpr : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_AFkvalue :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_AFvalue : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_AHint : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_AMpamp : 'c -> unit
    method on_AProgram : 'c -> Ast_visitors_ancestors.program -> unit
    method on_AStmt : 'c -> Ast_visitors_ancestors.stmt -> unit
    method on_AbsConst :
      'c ->
      Ast_visitors_ancestors.hint option -> Ast_visitors_ancestors.id -> unit
    method on_Abstract : 'c -> unit
    method on_Alias : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_Amp : 'c -> unit
    method on_Array : 'c -> Ast_visitors_ancestors.afield list -> unit
    method on_Darray :
      'c ->
      (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr) list ->
      unit
    method on_Varray : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_Array_get :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option -> unit
    method on_As_kv :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_As_v : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Attributes :
      'c -> Ast_visitors_ancestors.class_attr list -> unit
    method on_Await : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_BArbar : 'c -> unit
    method on_Bar : 'c -> unit
    method on_Binop :
      'c ->
      Ast_visitors_ancestors.bop ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_Block : 'c -> Ast_visitors_ancestors.block -> unit
    method on_BracedExpr : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Break : 'c -> Ast_visitors_ancestors.pos_t -> int option -> unit
    method on_CA_enum : 'c -> string list -> unit
    method on_CA_field : 'c -> Ast_visitors_ancestors.ca_field -> unit
    method on_CA_hint : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_CA_name : 'c -> Ast_visitors_ancestors.id -> unit
    method on_Cabstract : 'c -> unit
    method on_Call :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> unit
    method on_Case :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> unit
    method on_Cast :
      'c ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.expr -> unit
    method on_Cenum : 'c -> unit
    method on_Cinterface : 'c -> unit
    method on_Class : 'c -> Ast_visitors_ancestors.class_ -> unit
    method on_ClassTraitRequire :
      'c ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.hint -> unit
    method on_ClassUse : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_ClassUseAlias : 'c ->
                              Ast_visitors_ancestors.id option ->
                              Ast_visitors_ancestors.pstring ->
                              Ast_visitors_ancestors.id option ->
                              Ast_visitors_ancestors.kind option ->
                              unit
    method on_ClassUsePrecedence : 'c ->
                              Ast_visitors_ancestors.id ->
                              Ast_visitors_ancestors.pstring ->
                              Ast_visitors_ancestors.id list ->
                              unit
    method on_ClassVars :
      'c ->
      Ast_visitors_ancestors.kind list ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var list -> unit
    method on_Class_const :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.pstring -> unit
    method on_Class_get :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr -> unit
    method on_Clone : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Cmp : 'c -> unit
    method on_Cnormal : 'c -> unit
    method on_Collection :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.afield list -> unit
    method on_Const :
      'c ->
      Ast_visitors_ancestors.hint option ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list -> unit
    method on_Constant : 'c -> Ast_visitors_ancestors.gconst -> unit
    method on_Constraint_as : 'c -> unit
    method on_Constraint_eq : 'c -> unit
    method on_Constraint_super : 'c -> unit
    method on_Continue :
      'c ->
      Ast_visitors_ancestors.pos_t ->
      int option -> unit
    method on_Contravariant : 'c -> unit
    method on_Covariant : 'c -> unit
    method on_Cst_const : 'c -> unit
    method on_Cst_define : 'c -> unit
    method on_Ctrait : 'c -> unit
    method on_Def_inline :
      'c ->
      Ast_visitors_ancestors.def -> unit
    method on_Default : 'c -> Ast_visitors_ancestors.block -> unit
    method on_Diff : 'c -> unit
    method on_Diff2 : 'c -> unit
    method on_Do :
      'c ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.expr -> unit
    method on_Dot : 'c -> unit
    method on_EQeqeq : 'c -> unit
    method on_Efun :
      'c ->
      Ast_visitors_ancestors.fun_ ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.is_reference) list ->
      unit
    method on_Eif :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option ->
      Ast_visitors_ancestors.expr -> unit
    method on_Eq : 'c -> Ast_visitors_ancestors.bop option -> unit
    method on_Eqeq : 'c -> unit
    method on_Expr : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Expr_list : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_FAsync : 'c -> unit
    method on_FAsyncGenerator : 'c -> unit
    method on_FGenerator : 'c -> unit
    method on_FSync : 'c -> unit
    method on_Fallthrough : 'c -> unit
    method on_False : 'c -> unit
    method private on_FileInfo_mode :
      'c -> Ast_visitors_ancestors.fimode -> unit
    method on_Final : 'c -> unit
    method on_Float : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_For :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> unit
    method on_Foreach :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.pos_t option ->
      Ast_visitors_ancestors.as_expr -> Ast_visitors_ancestors.block -> unit
    method on_Fun : 'c -> Ast_visitors_ancestors.fun_ -> unit
    method on_GotoLabel : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_Goto : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_Gt : 'c -> unit
    method on_Gte : 'c -> unit
    method on_Gtgt : 'c -> unit
    method on_Haccess :
      'c ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.id list -> unit
    method on_Happly :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.hint list -> unit
    method on_Hfun :
      'c ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.hint -> unit
    method on_Hoption : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_Hshape : 'c -> Ast_visitors_ancestors.shape_info -> unit
    method on_Htuple : 'c -> Ast_visitors_ancestors.hint list -> unit
    method on_Id : 'c -> Ast_visitors_ancestors.id -> unit
    method on_Id_type_arguments :
      'c ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.hint list -> unit
    method on_If :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.block -> unit
    method on_Import :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.expr -> unit
    method on_Include : 'c -> unit
    method on_IncludeOnce : 'c -> unit
    method on_InstanceOf :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_Int : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_Invariant : 'c -> unit
    method on_Lfun : 'c -> Ast_visitors_ancestors.fun_ -> unit
    method on_List : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_Lt : 'c -> unit
    method on_Lte : 'c -> unit
    method on_Ltlt : 'c -> unit
    method on_Lvar : 'c -> Ast_visitors_ancestors.id -> unit
    method on_Lvarvar : 'c -> int -> Ast_visitors_ancestors.id -> unit
    method on_Method : 'c -> Ast_visitors_ancestors.method_ -> unit
    method on_Minus : 'c -> unit
    method on_MustExtend : 'c -> unit
    method on_MustImplement : 'c -> unit
    method on_NSClass : 'c -> unit
    method on_NSNamespace: 'c -> unit
    method on_NSClassAndNamespace : 'c -> unit
    method on_NSConst : 'c -> unit
    method on_NSFun : 'c -> unit
    method on_Namespace :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.program -> unit
    method on_SetNamespaceEnv :
      'c ->
      Ast_visitors_ancestors.nsenv -> unit
    method on_NamespaceUse :
      'c ->
      (Ast_visitors_ancestors.ns_kind * Ast_visitors_ancestors.id *
       Ast_visitors_ancestors.id)
      list -> unit
    method private on_Namespace_env :
      'c -> Ast_visitors_ancestors.nsenv -> unit
    method on_New :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> unit
    method on_NewType : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_Noop : 'c -> unit
    method on_Null : 'c -> unit
    method on_NullCoalesce :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_OG_nullsafe : 'c -> unit
    method on_OG_nullthrows : 'c -> unit
    method on_Obj_get :
      'c ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.og_null_flavor -> unit
    method on_Percent : 'c -> unit
    method on_Pipe :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr -> unit
    method on_Plus : 'c -> unit
    method private on_Pos_t : 'c -> Ast_visitors_ancestors.pos_t -> unit
    method on_Private : 'c -> unit
    method on_Protected : 'c -> unit
    method on_Public : 'c -> unit
    method on_Require : 'c -> unit
    method on_RequireOnce : 'c -> unit
    method on_Return :
      'c ->
      Ast_visitors_ancestors.pos_t ->
      Ast_visitors_ancestors.expr option -> unit
    method on_SFclass_const :
      'c ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.pstring -> unit
    method on_SFlit : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_Shape :
      'c ->
      (Ast_visitors_ancestors.shape_field_name * Ast_visitors_ancestors.expr)
      list -> unit
    method on_Slash : 'c -> unit
    method on_Star : 'c -> unit
    method on_Starstar : 'c -> unit
    method on_Static : 'c -> unit
    method on_Static_var : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_Global_var : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_Stmt : 'c -> Ast_visitors_ancestors.stmt -> unit
    method on_String : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_String2 : 'c -> Ast_visitors_ancestors.expr list -> unit
    method on_Switch :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.case list -> unit
    method on_Throw : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_True : 'c -> unit
    method on_Try :
      'c ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.catch list ->
      Ast_visitors_ancestors.block -> unit
    method on_TypeConst : 'c -> Ast_visitors_ancestors.typeconst -> unit
    method on_Typedef : 'c -> Ast_visitors_ancestors.typedef -> unit
    method on_Udecr : 'c -> unit
    method on_Uincr : 'c -> unit
    method on_Uminus : 'c -> unit
    method on_Unop :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.expr -> unit
    method on_Unot : 'c -> unit
    method on_Unsafe : 'c -> unit
    method on_Unsafeexpr : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Updecr : 'c -> unit
    method on_Upincr : 'c -> unit
    method on_Uplus : 'c -> unit
    method on_Uref : 'c -> unit
    method on_Usplat : 'c -> unit
    method on_Usilence : 'c -> unit
    method on_Utild : 'c -> unit
    method on_While :
      'c ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.block -> unit
    method on_XhpAttr :
      'c ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var ->
      Ast_visitors_ancestors.is_reference ->
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr list)
      option -> unit
    method on_XhpAttrUse : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_XhpCategory : 'c -> Ast_visitors_ancestors.pstring list -> unit
    method on_XhpChild : 'c -> Ast_visitors_ancestors.xhp_child -> unit
    method on_xhp_child : 'c -> Ast_visitors_ancestors.xhp_child -> unit
    method on_ChildName : 'c -> Ast_visitors_ancestors.id -> unit
    method on_ChildList : 'c -> Ast_visitors_ancestors.xhp_child list -> unit
    method on_ChildUnary : 'c -> Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child_op -> unit
    method on_ChildBinary : 'c -> Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child -> unit
    method on_xhp_child_op : 'c -> Ast_visitors_ancestors.xhp_child_op -> unit
    method on_ChildStar : 'c -> unit
    method on_ChildPlus : 'c -> unit
    method on_ChildQuestion : 'c -> unit

    method on_Xml :
      'c ->
      Ast_visitors_ancestors.id ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr list -> unit
    method on_LogXor : 'c -> unit
    method on_Xor : 'c -> unit
    method on_Yield : 'c -> Ast_visitors_ancestors.afield -> unit
    method on_Yield_from : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_Yield_break : 'c -> unit
    method on_afield : 'c -> Ast_visitors_ancestors.afield -> unit
    method on_any : 'c -> Ast_visitors_ancestors.any -> unit
    method on_as_expr : 'c -> Ast_visitors_ancestors.as_expr -> unit
    method on_attr :
      'c -> Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr -> unit
    method on_block : 'c -> Ast_visitors_ancestors.block -> unit
    method private on_bool :
      'c -> Ast_visitors_ancestors.is_reference -> unit
    method on_bop : 'c -> Ast_visitors_ancestors.bop -> unit
    method on_ca_field : 'c -> Ast_visitors_ancestors.ca_field -> unit
    method on_ca_type : 'c -> Ast_visitors_ancestors.ca_type -> unit
    method on_case : 'c -> Ast_visitors_ancestors.case -> unit
    method on_catch : 'c -> Ast_visitors_ancestors.catch -> unit
    method on_class_ : 'c -> Ast_visitors_ancestors.class_ -> unit
    method on_class_attr : 'c -> Ast_visitors_ancestors.class_attr -> unit
    method on_class_elt : 'c -> Ast_visitors_ancestors.class_elt -> unit
    method on_class_kind : 'c -> Ast_visitors_ancestors.class_kind -> unit
    method on_class_var : 'c -> Ast_visitors_ancestors.class_var -> unit
    method on_constraint_kind :
      'c -> Ast_visitors_ancestors.constraint_kind -> unit
    method on_cst_kind : 'c -> Ast_visitors_ancestors.cst_kind -> unit
    method on_def : 'c -> Ast_visitors_ancestors.def -> unit
    method on_enum_ : 'c -> Ast_visitors_ancestors.enum_ -> unit
    method on_expr : 'c -> Ast_visitors_ancestors.expr -> unit
    method on_expr_ : 'c -> Ast_visitors_ancestors.expr_ -> unit
    method on_field :
      'c -> Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr -> unit
    method on_fun_ : 'c -> Ast_visitors_ancestors.fun_ -> unit
    method on_fun_kind : 'c -> Ast_visitors_ancestors.fun_kind -> unit
    method on_fun_param : 'c -> Ast_visitors_ancestors.fun_param -> unit
    method on_gconst : 'c -> Ast_visitors_ancestors.gconst -> unit
    method on_hint : 'c -> Ast_visitors_ancestors.hint -> unit
    method on_hint_ : 'c -> Ast_visitors_ancestors.hint_ -> unit
    method on_id : 'c -> Ast_visitors_ancestors.id -> unit
    method on_import_flavor :
      'c -> Ast_visitors_ancestors.import_flavor -> unit
    method private on_int : 'c -> int -> unit
    method on_is_reference :
      'c -> Ast_visitors_ancestors.is_reference -> unit
    method on_is_variadic : 'c -> Ast_visitors_ancestors.is_reference -> unit
    method on_kind : 'c -> Ast_visitors_ancestors.kind -> unit
    method private on_list :
      'env 'a. ('env -> 'a -> unit) -> 'env -> 'a list -> unit
    method on_method_ : 'c -> Ast_visitors_ancestors.method_ -> unit
    method on_ns_kind : 'c -> Ast_visitors_ancestors.ns_kind -> unit
    method on_og_null_flavor :
      'c -> Ast_visitors_ancestors.og_null_flavor -> unit
    method private on_option :
      'env 'a. ('env -> 'a -> unit) -> 'env -> 'a option -> unit
    method on_program : 'c -> Ast_visitors_ancestors.program -> unit
    method on_pstring : 'c -> Ast_visitors_ancestors.pstring -> unit
    method on_shape_field : 'c -> Ast_visitors_ancestors.shape_field -> unit
    method on_shape_field_name :
      'c -> Ast_visitors_ancestors.shape_field_name -> unit
    method on_stmt : 'c -> Ast_visitors_ancestors.stmt -> unit
    method private on_string : 'c -> string -> unit
    method on_tconstraint : 'c -> Ast_visitors_ancestors.tconstraint -> unit
    method on_tparam : 'c -> Ast_visitors_ancestors.tparam -> unit
    method on_trait_req_kind :
      'c -> Ast_visitors_ancestors.trait_req_kind -> unit
    method on_typeconst : 'c -> Ast_visitors_ancestors.typeconst -> unit
    method on_typedef : 'c -> Ast_visitors_ancestors.typedef -> unit
    method on_typedef_kind :
      'c -> Ast_visitors_ancestors.typedef_kind -> unit
    method on_uop : 'c -> Ast_visitors_ancestors.uop -> unit
    method on_user_attribute :
      'c -> Ast_visitors_ancestors.user_attribute -> unit
    method on_variance : 'c -> Ast_visitors_ancestors.variance -> unit
  end

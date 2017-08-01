class virtual ['c] map :
  object ('c)
    constraint 'c =
      < on_ADef : 'd ->
                  Ast_visitors_ancestors.def -> Ast_visitors_ancestors.any;
        on_AExpr : 'd ->
                   Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.any;
        on_AFkvalue : 'd ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.afield;
        on_AFvalue : 'd ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.afield;
        on_AHint : 'd ->
                   Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.any;
        on_AMpamp : 'd -> Ast_visitors_ancestors.bop;
        on_AProgram : 'd ->
                      Ast_visitors_ancestors.program ->
                      Ast_visitors_ancestors.any;
        on_AStmt : 'd ->
                   Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.any;
        on_AbsConst : 'd ->
                      Ast_visitors_ancestors.hint option ->
                      Ast_visitors_ancestors.id ->
                      Ast_visitors_ancestors.class_elt;
        on_Abstract : 'd -> Ast_visitors_ancestors.kind;
        on_Alias : 'd ->
                   Ast_visitors_ancestors.hint ->
                   Ast_visitors_ancestors.typedef_kind;
        on_Amp : 'd -> Ast_visitors_ancestors.bop;
        on_Array : 'd ->
                   Ast_visitors_ancestors.afield list ->
                   Ast_visitors_ancestors.expr_;
        on_Darray : 'd ->
                   (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr)
                   list ->
                   Ast_visitors_ancestors.expr_;

        on_Varray : 'd ->
                   Ast_visitors_ancestors.expr list ->
                   Ast_visitors_ancestors.expr_;

        on_Array_get : 'd ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr option ->
                       Ast_visitors_ancestors.expr_;
        on_As_kv : 'd ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.as_expr;
        on_As_v : 'd ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.as_expr;
        on_Attributes : 'd ->
                        Ast_visitors_ancestors.class_attr list ->
                        Ast_visitors_ancestors.class_elt;
        on_Await : 'd ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
        on_BArbar : 'd -> Ast_visitors_ancestors.bop;
        on_Bar : 'd -> Ast_visitors_ancestors.bop;
        on_Binop : 'd ->
                   Ast_visitors_ancestors.bop ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
        on_Block : 'd ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.stmt;
       on_BracedExpr : 'd ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr_;
        on_Break : 'd ->
                   Ast_visitors_ancestors.pos_t ->
                   int option ->
                   Ast_visitors_ancestors.stmt;
        on_CA_enum : 'd -> string list -> Ast_visitors_ancestors.ca_type;
        on_CA_field : 'd ->
                      Ast_visitors_ancestors.ca_field ->
                      Ast_visitors_ancestors.class_attr;
        on_CA_hint : 'd ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.ca_type;
        on_CA_name : 'd ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.class_attr;
        on_Cabstract : 'd -> Ast_visitors_ancestors.class_kind;
        on_Call : 'd ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr_;
        on_Case : 'd ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case;
        on_Cast : 'd ->
                  Ast_visitors_ancestors.hint ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Cenum : 'd -> Ast_visitors_ancestors.class_kind;
        on_Cinterface : 'd -> Ast_visitors_ancestors.class_kind;
        on_Class : 'd ->
                   Ast_visitors_ancestors.class_ ->
                   Ast_visitors_ancestors.def;
        on_ClassTraitRequire : 'd ->
                               Ast_visitors_ancestors.trait_req_kind ->
                               Ast_visitors_ancestors.hint ->
                               Ast_visitors_ancestors.class_elt;
        on_ClassUse : 'd ->
                      Ast_visitors_ancestors.hint ->
                      Ast_visitors_ancestors.class_elt;
        on_ClassUseAlias : 'd ->
                            Ast_visitors_ancestors.id option ->
                            Ast_visitors_ancestors.pstring ->
                            Ast_visitors_ancestors.id option ->
                            Ast_visitors_ancestors.kind option ->
                            Ast_visitors_ancestors.class_elt;
        on_ClassUsePrecedence : 'd ->
                            Ast_visitors_ancestors.id ->
                            Ast_visitors_ancestors.pstring ->
                            Ast_visitors_ancestors.id list ->
                            Ast_visitors_ancestors.class_elt;
        on_ClassVars : 'd ->
                       Ast_visitors_ancestors.kind list ->
                       Ast_visitors_ancestors.hint option ->
                       Ast_visitors_ancestors.class_var list ->
                       Ast_visitors_ancestors.class_elt;
        on_Class_const : 'd ->
                         Ast_visitors_ancestors.id ->
                         Ast_visitors_ancestors.pstring ->
                         Ast_visitors_ancestors.expr_;
        on_Class_get : 'd ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr_;
        on_Clone : 'd ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
        on_Cmp : 'd -> Ast_visitors_ancestors.bop;
        on_Cnormal : 'd -> Ast_visitors_ancestors.class_kind;
        on_Collection : 'd ->
                        Ast_visitors_ancestors.id ->
                        Ast_visitors_ancestors.afield list ->
                        Ast_visitors_ancestors.expr_;
        on_Const : 'd ->
                   Ast_visitors_ancestors.hint option ->
                   (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                   list -> Ast_visitors_ancestors.class_elt;
        on_Constant : 'd ->
                      Ast_visitors_ancestors.gconst ->
                      Ast_visitors_ancestors.def;
        on_Constraint_as : 'd -> Ast_visitors_ancestors.constraint_kind;
        on_Constraint_eq : 'd -> Ast_visitors_ancestors.constraint_kind;
        on_Constraint_super : 'd -> Ast_visitors_ancestors.constraint_kind;
        on_Continue : 'd ->
                      Ast_visitors_ancestors.pos_t ->
                      int option ->
                      Ast_visitors_ancestors.stmt;
        on_Contravariant : 'd -> Ast_visitors_ancestors.variance;
        on_Covariant : 'd -> Ast_visitors_ancestors.variance;
        on_Cst_const : 'd -> Ast_visitors_ancestors.cst_kind;
        on_Cst_define : 'd -> Ast_visitors_ancestors.cst_kind;
        on_Ctrait : 'd -> Ast_visitors_ancestors.class_kind;
        on_Def_inline : 'd ->
                        Ast_visitors_ancestors.def ->
                        Ast_visitors_ancestors.stmt;
        on_Default : 'd ->
                     Ast_visitors_ancestors.block ->
                     Ast_visitors_ancestors.case;
        on_Diff : 'd -> Ast_visitors_ancestors.bop;
        on_Diff2 : 'd -> Ast_visitors_ancestors.bop;
        on_Do : 'd ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_Dot : 'd -> Ast_visitors_ancestors.bop;
        on_EQeqeq : 'd -> Ast_visitors_ancestors.bop;
        on_Efun : 'd ->
                  Ast_visitors_ancestors.fun_ ->
                  (Ast_visitors_ancestors.id *
                   Ast_visitors_ancestors.is_reference)
                  list -> Ast_visitors_ancestors.expr_;
        on_Eif : 'd ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr option ->
                 Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Eq : 'd ->
                Ast_visitors_ancestors.bop option ->
                Ast_visitors_ancestors.bop;
        on_Eqeq : 'd -> Ast_visitors_ancestors.bop;
        on_Expr : 'd ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_Omitted : 'd -> Ast_visitors_ancestors.expr_;
        on_Expr_list : 'd ->
                       Ast_visitors_ancestors.expr list ->
                       Ast_visitors_ancestors.expr_;
        on_FAsync : 'd -> Ast_visitors_ancestors.fun_kind;
        on_FAsyncGenerator : 'd -> Ast_visitors_ancestors.fun_kind;
        on_FGenerator : 'd -> Ast_visitors_ancestors.fun_kind;
        on_FSync : 'd -> Ast_visitors_ancestors.fun_kind;
        on_Fallthrough : 'd -> Ast_visitors_ancestors.stmt;
        on_False : 'd -> Ast_visitors_ancestors.expr_;
        on_Final : 'd -> Ast_visitors_ancestors.kind;
        on_Float : 'd ->
                   Ast_visitors_ancestors.pstring ->
                   Ast_visitors_ancestors.expr_;
        on_For : 'd ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_Foreach : 'd ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.pos_t option ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.block ->
                     Ast_visitors_ancestors.stmt;
        on_Fun : 'd ->
                 Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.def;
        on_GotoLabel : 'd ->
                       Ast_visitors_ancestors.pstring ->
                       Ast_visitors_ancestors.stmt;
        on_Goto : 'd ->
                  Ast_visitors_ancestors.pstring ->
                  Ast_visitors_ancestors.stmt;
        on_Markup : 'd ->
                  Ast_visitors_ancestors.pstring ->
                  Ast_visitors_ancestors.expr option ->
                  Ast_visitors_ancestors.stmt;
        on_Gt : 'd -> Ast_visitors_ancestors.bop;
        on_Gte : 'd -> Ast_visitors_ancestors.bop;
        on_Gtgt : 'd -> Ast_visitors_ancestors.bop;
        on_Haccess : 'd ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id list ->
                     Ast_visitors_ancestors.hint_;
        on_Happly : 'd ->
                    Ast_visitors_ancestors.id ->
                    Ast_visitors_ancestors.hint list ->
                    Ast_visitors_ancestors.hint_;
        on_Hfun : 'd ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.is_reference ->
                  Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_;
        on_Hoption : 'd ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.hint_;
        on_Hshape : 'd ->
                    Ast_visitors_ancestors.shape_info ->
                    Ast_visitors_ancestors.hint_;
        on_Htuple : 'd ->
                    Ast_visitors_ancestors.hint list ->
                    Ast_visitors_ancestors.hint_;
        on_Hsoft : 'd ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.hint_;
        on_Id : 'd ->
                     Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_;
        on_Id_type_arguments : 'd ->
                               Ast_visitors_ancestors.id ->
                               Ast_visitors_ancestors.hint list ->
                               Ast_visitors_ancestors.expr_;
        on_If : 'd ->
                Ast_visitors_ancestors.expr ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_Import : 'd ->
                    Ast_visitors_ancestors.import_flavor ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.expr_;
        on_Include : 'd -> Ast_visitors_ancestors.import_flavor;
        on_IncludeOnce : 'd -> Ast_visitors_ancestors.import_flavor;
        on_InstanceOf : 'd ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr_;
        on_Int : 'd ->
                 Ast_visitors_ancestors.pstring ->
                 Ast_visitors_ancestors.expr_;
        on_Invariant : 'd -> Ast_visitors_ancestors.variance;
        on_Lfun : 'd ->
                  Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.expr_;
        on_List : 'd ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr_;
        on_Lt : 'd -> Ast_visitors_ancestors.bop;
        on_Lte : 'd -> Ast_visitors_ancestors.bop;
        on_Ltlt : 'd -> Ast_visitors_ancestors.bop;
        on_Lvar : 'd ->
                  Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_;
        on_Lvarvar : 'd ->
                     int ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.expr_;
        on_Method : 'd ->
                    Ast_visitors_ancestors.method_ ->
                    Ast_visitors_ancestors.class_elt;
        on_Minus : 'd -> Ast_visitors_ancestors.bop;
        on_MustExtend : 'd -> Ast_visitors_ancestors.trait_req_kind;
        on_MustImplement : 'd -> Ast_visitors_ancestors.trait_req_kind;
        on_NSClass : 'd -> Ast_visitors_ancestors.ns_kind;
        on_NSNamespace : 'd -> Ast_visitors_ancestors.ns_kind;
        on_NSClassAndNamespace : 'd -> Ast_visitors_ancestors.ns_kind;
        on_NSConst : 'd -> Ast_visitors_ancestors.ns_kind;
        on_NSFun : 'd -> Ast_visitors_ancestors.ns_kind;
        on_Namespace : 'd ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.program ->
                       Ast_visitors_ancestors.def;
        on_SetNamespaceEnv : 'd ->
                            Ast_visitors_ancestors.nsenv ->
                            Ast_visitors_ancestors.def;
        on_NamespaceUse : 'd ->
                          (Ast_visitors_ancestors.ns_kind *
                           Ast_visitors_ancestors.id *
                           Ast_visitors_ancestors.id)
                          list -> Ast_visitors_ancestors.def;
        on_New : 'd ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr_;
        on_NewType : 'd ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.typedef_kind;
        on_Noop : 'd -> Ast_visitors_ancestors.stmt;
        on_Null : 'd -> Ast_visitors_ancestors.expr_;
        on_NullCoalesce : 'd ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr_;
        on_OG_nullsafe : 'd -> Ast_visitors_ancestors.og_null_flavor;
        on_OG_nullthrows : 'd -> Ast_visitors_ancestors.og_null_flavor;
        on_Obj_get : 'd ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.og_null_flavor ->
                     Ast_visitors_ancestors.expr_;
        on_Percent : 'd -> Ast_visitors_ancestors.bop;
        on_Pipe : 'd ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Plus : 'd -> Ast_visitors_ancestors.bop;
        on_Private : 'd -> Ast_visitors_ancestors.kind;
        on_Protected : 'd -> Ast_visitors_ancestors.kind;
        on_Public : 'd -> Ast_visitors_ancestors.kind;
        on_Require : 'd -> Ast_visitors_ancestors.import_flavor;
        on_RequireOnce : 'd -> Ast_visitors_ancestors.import_flavor;
        on_Return : 'd ->
                    Ast_visitors_ancestors.pos_t ->
                    Ast_visitors_ancestors.expr option ->
                    Ast_visitors_ancestors.stmt;
        on_SFclass_const : 'd ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring ->
                           Ast_visitors_ancestors.shape_field_name;
        on_SFlit : 'd ->
                   Ast_visitors_ancestors.pstring ->
                   Ast_visitors_ancestors.shape_field_name;
        on_Shape : 'd ->
                   (Ast_visitors_ancestors.shape_field_name *
                    Ast_visitors_ancestors.expr)
                   list -> Ast_visitors_ancestors.expr_;
        on_Slash : 'd -> Ast_visitors_ancestors.bop;
        on_Star : 'd -> Ast_visitors_ancestors.bop;
        on_Starstar : 'd -> Ast_visitors_ancestors.bop;
        on_Static : 'd -> Ast_visitors_ancestors.kind;
        on_Static_var : 'd ->
                        Ast_visitors_ancestors.expr list ->
                        Ast_visitors_ancestors.stmt;
        on_Global_var : 'd ->
                        Ast_visitors_ancestors.expr list ->
                        Ast_visitors_ancestors.stmt;
        on_Stmt : 'd ->
                  Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.def;
        on_String : 'd ->
                    Ast_visitors_ancestors.pstring ->
                    Ast_visitors_ancestors.expr_;
        on_String2 : 'd ->
                     Ast_visitors_ancestors.expr list ->
                     Ast_visitors_ancestors.expr_;
        on_Switch : 'd ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.case list ->
                    Ast_visitors_ancestors.stmt;
        on_Throw : 'd ->
                   Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_True : 'd -> Ast_visitors_ancestors.expr_;
        on_Try : 'd ->
                 Ast_visitors_ancestors.block ->
                 Ast_visitors_ancestors.catch list ->
                 Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_TypeConst : 'd ->
                       Ast_visitors_ancestors.typeconst ->
                       Ast_visitors_ancestors.class_elt;
        on_Typedef : 'd ->
                     Ast_visitors_ancestors.typedef ->
                     Ast_visitors_ancestors.def;
        on_Udecr : 'd -> Ast_visitors_ancestors.uop;
        on_Uincr : 'd -> Ast_visitors_ancestors.uop;
        on_Uminus : 'd -> Ast_visitors_ancestors.uop;
        on_Unop : 'd ->
                  Ast_visitors_ancestors.uop ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Unot : 'd -> Ast_visitors_ancestors.uop;
        on_Unsafe : 'd -> Ast_visitors_ancestors.stmt;
        on_Unsafeexpr : 'd ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr_;
        on_Updecr : 'd -> Ast_visitors_ancestors.uop;
        on_Upincr : 'd -> Ast_visitors_ancestors.uop;
        on_Uplus : 'd -> Ast_visitors_ancestors.uop;
        on_Uref : 'd -> Ast_visitors_ancestors.uop;
        on_Usplat : 'd -> Ast_visitors_ancestors.uop;
        on_Usilence : 'd -> Ast_visitors_ancestors.uop;
        on_Utild : 'd -> Ast_visitors_ancestors.uop;
        on_While : 'd ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.stmt;
        on_XhpAttr : 'd ->
                     Ast_visitors_ancestors.hint option ->
                     Ast_visitors_ancestors.class_var ->
                     Ast_visitors_ancestors.is_reference ->
                     (Ast_visitors_ancestors.pos_t *
                      Ast_visitors_ancestors.expr list)
                     option -> Ast_visitors_ancestors.class_elt;
        on_XhpAttrUse : 'd ->
                        Ast_visitors_ancestors.hint ->
                        Ast_visitors_ancestors.class_elt;
        on_XhpCategory : 'd ->
                         Ast_visitors_ancestors.pstring list ->
                         Ast_visitors_ancestors.class_elt;
         on_XhpChild : 'd ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.class_elt;
          on_xhp_child : 'd ->
                           Ast_visitors_ancestors.xhp_child ->
                           Ast_visitors_ancestors.xhp_child;
          on_ChildName : 'd ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.id ->
                          Ast_visitors_ancestors.xhp_child;

          on_ChildList : 'd ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child list ->
                          Ast_visitors_ancestors.xhp_child;

          on_ChildUnary : 'd ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child_op ->
                          Ast_visitors_ancestors.xhp_child;

          on_ChildBinary : 'd ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child ->
                          Ast_visitors_ancestors.xhp_child;

          on_xhp_child_op : 'd ->
            Ast_visitors_ancestors.xhp_child_op ->
            Ast_visitors_ancestors.xhp_child_op;

          on_ChildStar : 'd ->
            Ast_visitors_ancestors.xhp_child_op ->
            Ast_visitors_ancestors.xhp_child_op;

          on_ChildPlus : 'd ->
            Ast_visitors_ancestors.xhp_child_op ->
            Ast_visitors_ancestors.xhp_child_op;

          on_ChildQuestion : 'd ->
            Ast_visitors_ancestors.xhp_child_op ->
            Ast_visitors_ancestors.xhp_child_op;




        on_Xml : 'd ->
                 Ast_visitors_ancestors.id ->
                 (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                 list ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr_;
        on_LogXor : 'd -> Ast_visitors_ancestors.bop;
        on_Xor : 'd -> Ast_visitors_ancestors.bop;
        on_Yield : 'd ->
                   Ast_visitors_ancestors.afield ->
                   Ast_visitors_ancestors.expr_;
        on_Yield_from : 'd ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.expr_;
        on_Yield_break : 'd -> Ast_visitors_ancestors.expr_;
        on_afield : 'd ->
                    Ast_visitors_ancestors.afield ->
                    Ast_visitors_ancestors.afield;
        on_any : 'd ->
                 Ast_visitors_ancestors.any -> Ast_visitors_ancestors.any;
        on_as_expr : 'd ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.as_expr;
        on_attr : 'd ->
                  Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
                  (Ast_visitors_ancestors.pos_t * string) *
                  (Ast_visitors_ancestors.pos_t *
                   Ast_visitors_ancestors.expr_);
        on_block : 'd ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.block;
        on_bop : 'd ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_ca_field : 'd ->
                      Ast_visitors_ancestors.ca_field ->
                      Ast_visitors_ancestors.ca_field;
        on_ca_type : 'd ->
                     Ast_visitors_ancestors.ca_type ->
                     Ast_visitors_ancestors.ca_type;
        on_case : 'd ->
                  Ast_visitors_ancestors.case -> Ast_visitors_ancestors.case;
        on_catch : 'd ->
                   Ast_visitors_ancestors.catch ->
                   Ast_visitors_ancestors.id * Ast_visitors_ancestors.id *
                   Ast_visitors_ancestors.block;
        on_class_ : 'd ->
                    Ast_visitors_ancestors.class_ ->
                    Ast_visitors_ancestors.class_;
        on_class_attr : 'd ->
                        Ast_visitors_ancestors.class_attr ->
                        Ast_visitors_ancestors.class_attr;
        on_class_elt : 'd ->
                       Ast_visitors_ancestors.class_elt ->
                       Ast_visitors_ancestors.class_elt;
        on_class_kind : 'd ->
                        Ast_visitors_ancestors.class_kind ->
                        Ast_visitors_ancestors.class_kind;
        on_class_var : 'd ->
                       Ast_visitors_ancestors.class_var ->
                       Ast_visitors_ancestors.pos_t *
                       Ast_visitors_ancestors.id *
                       Ast_visitors_ancestors.expr option;
        on_constraint_kind : 'd ->
                             Ast_visitors_ancestors.constraint_kind ->
                             Ast_visitors_ancestors.constraint_kind;
        on_cst_kind : 'd ->
                      Ast_visitors_ancestors.cst_kind ->
                      Ast_visitors_ancestors.cst_kind;
        on_def : 'd ->
                 Ast_visitors_ancestors.def -> Ast_visitors_ancestors.def;
        on_enum_ : 'd ->
                   Ast_visitors_ancestors.enum_ ->
                   Ast_visitors_ancestors.enum_;
        on_expr : 'd ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr_;
        on_expr_ : 'd ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr_;
        on_field : 'd ->
                   Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
                   (Ast_visitors_ancestors.pos_t *
                    Ast_visitors_ancestors.expr_) *
                   (Ast_visitors_ancestors.pos_t *
                    Ast_visitors_ancestors.expr_);
        on_fun_ : 'd ->
                  Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.fun_;
        on_fun_kind : 'd ->
                      Ast_visitors_ancestors.fun_kind ->
                      Ast_visitors_ancestors.fun_kind;
        on_fun_param : 'd ->
                       Ast_visitors_ancestors.fun_param ->
                       Ast_visitors_ancestors.fun_param;
        on_gconst : 'd ->
                    Ast_visitors_ancestors.gconst ->
                    Ast_visitors_ancestors.gconst;
        on_hint : 'd ->
                  Ast_visitors_ancestors.hint ->
                  Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.hint_;
        on_hint_ : 'd ->
                   Ast_visitors_ancestors.hint_ ->
                   Ast_visitors_ancestors.hint_;
        on_id : 'd ->
                Ast_visitors_ancestors.id ->
                Ast_visitors_ancestors.pos_t * string;
        on_import_flavor : 'd ->
                           Ast_visitors_ancestors.import_flavor ->
                           Ast_visitors_ancestors.import_flavor;
        on_is_reference : 'd ->
                          Ast_visitors_ancestors.is_reference ->
                          Ast_visitors_ancestors.is_reference;
        on_is_variadic : 'd ->
                         Ast_visitors_ancestors.is_reference ->
                         Ast_visitors_ancestors.is_reference;
        on_kind : 'd ->
                  Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind;
        on_method_ : 'd ->
                     Ast_visitors_ancestors.method_ ->
                     Ast_visitors_ancestors.method_;
        on_ns_kind : 'd ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_og_null_flavor : 'd ->
                            Ast_visitors_ancestors.og_null_flavor ->
                            Ast_visitors_ancestors.og_null_flavor;
        on_program : 'd ->
                     Ast_visitors_ancestors.program ->
                     Ast_visitors_ancestors.program;
        on_pstring : 'd ->
                     Ast_visitors_ancestors.pstring ->
                     Ast_visitors_ancestors.pos_t * string;
        on_shape_field : 'd ->
                         Ast_visitors_ancestors.shape_field ->
                         Ast_visitors_ancestors.shape_field;
        on_shape_field_name : 'd ->
                              Ast_visitors_ancestors.shape_field_name ->
                              Ast_visitors_ancestors.shape_field_name;
        on_stmt : 'd ->
                  Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt;
        on_tconstraint : 'd ->
                         Ast_visitors_ancestors.tconstraint ->
                         Ast_visitors_ancestors.tconstraint;
        on_tparam : 'd ->
                    Ast_visitors_ancestors.tparam ->
                    Ast_visitors_ancestors.variance *
                    Ast_visitors_ancestors.id *
                    (Ast_visitors_ancestors.constraint_kind *
                     Ast_visitors_ancestors.hint)
                    list;
        on_trait_req_kind : 'd ->
                            Ast_visitors_ancestors.trait_req_kind ->
                            Ast_visitors_ancestors.trait_req_kind;
        on_typeconst : 'd ->
                       Ast_visitors_ancestors.typeconst ->
                       Ast_visitors_ancestors.typeconst;
        on_typedef : 'd ->
                     Ast_visitors_ancestors.typedef ->
                     Ast_visitors_ancestors.typedef;
        on_typedef_kind : 'd ->
                          Ast_visitors_ancestors.typedef_kind ->
                          Ast_visitors_ancestors.typedef_kind;
        on_uop : 'd ->
                 Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_user_attribute : 'd ->
                            Ast_visitors_ancestors.user_attribute ->
                            Ast_visitors_ancestors.user_attribute;
        on_variance : 'd ->
                      Ast_visitors_ancestors.variance ->
                      Ast_visitors_ancestors.variance;
        .. >
    method on_ADef :
      'd -> Ast_visitors_ancestors.def -> Ast_visitors_ancestors.any
    method on_AExpr :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.any
    method on_AFkvalue :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.afield
    method on_AFvalue :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.afield
    method on_AHint :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.any
    method on_AMpamp : 'd -> Ast_visitors_ancestors.bop
    method on_AProgram :
      'd -> Ast_visitors_ancestors.program -> Ast_visitors_ancestors.any
    method on_AStmt :
      'd -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.any
    method on_AbsConst :
      'd ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.class_elt
    method on_Abstract : 'd -> Ast_visitors_ancestors.kind
    method on_Alias :
      'd ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.typedef_kind
    method on_Amp : 'd -> Ast_visitors_ancestors.bop
    method on_Array :
      'd ->
      Ast_visitors_ancestors.afield list -> Ast_visitors_ancestors.expr_
    method on_Darray :
      'd ->
      (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr_
    method on_Varray :
      'd ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Array_get :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option -> Ast_visitors_ancestors.expr_
    method on_As_kv :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.as_expr
    method on_As_v :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.as_expr
    method on_Attributes :
      'd ->
      Ast_visitors_ancestors.class_attr list ->
      Ast_visitors_ancestors.class_elt
    method on_Await :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_BArbar : 'd -> Ast_visitors_ancestors.bop
    method on_Bar : 'd -> Ast_visitors_ancestors.bop
    method on_Binop :
      'd ->
      Ast_visitors_ancestors.bop ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Block :
      'd -> Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_BracedExpr :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Break :
      'd -> Ast_visitors_ancestors.pos_t -> int option -> Ast_visitors_ancestors.stmt
    method on_CA_enum : 'd -> string list -> Ast_visitors_ancestors.ca_type
    method on_CA_field :
      'd ->
      Ast_visitors_ancestors.ca_field -> Ast_visitors_ancestors.class_attr
    method on_CA_hint :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.ca_type
    method on_CA_name :
      'd -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.class_attr
    method on_Cabstract : 'd -> Ast_visitors_ancestors.class_kind
    method on_Call :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Case :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case
    method on_Cast :
      'd ->
      Ast_visitors_ancestors.hint ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Cenum : 'd -> Ast_visitors_ancestors.class_kind
    method on_Cinterface : 'd -> Ast_visitors_ancestors.class_kind
    method on_Class :
      'd -> Ast_visitors_ancestors.class_ -> Ast_visitors_ancestors.def
    method on_ClassTraitRequire :
      'd ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_ClassUse :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_ClassUseAlias :
      'd ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.kind option -> Ast_visitors_ancestors.class_elt
    method on_ClassUsePrecedence :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id list -> Ast_visitors_ancestors.class_elt
    method on_ClassVars :
      'd ->
      Ast_visitors_ancestors.kind list ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var list ->
      Ast_visitors_ancestors.class_elt
    method on_Class_const :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_Class_get :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Clone :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Cmp : 'd -> Ast_visitors_ancestors.bop
    method on_Cnormal : 'd -> Ast_visitors_ancestors.class_kind
    method on_Collection :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.afield list -> Ast_visitors_ancestors.expr_
    method on_Const :
      'd ->
      Ast_visitors_ancestors.hint option ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.class_elt
    method on_Constant :
      'd -> Ast_visitors_ancestors.gconst -> Ast_visitors_ancestors.def
    method on_Constraint_as : 'd -> Ast_visitors_ancestors.constraint_kind
    method on_Constraint_eq : 'd -> Ast_visitors_ancestors.constraint_kind
    method on_Constraint_super : 'd -> Ast_visitors_ancestors.constraint_kind
    method on_Continue :
      'd -> Ast_visitors_ancestors.pos_t -> int option -> Ast_visitors_ancestors.stmt
    method on_Contravariant : 'd -> Ast_visitors_ancestors.variance
    method on_Covariant : 'd -> Ast_visitors_ancestors.variance
    method on_Cst_const : 'd -> Ast_visitors_ancestors.cst_kind
    method on_Cst_define : 'd -> Ast_visitors_ancestors.cst_kind
    method on_Ctrait : 'd -> Ast_visitors_ancestors.class_kind
    method on_Default :
      'd -> Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case
    method on_Diff : 'd -> Ast_visitors_ancestors.bop
    method on_Diff2 : 'd -> Ast_visitors_ancestors.bop
    method on_Do :
      'd ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
    method on_Dot : 'd -> Ast_visitors_ancestors.bop
    method on_EQeqeq : 'd -> Ast_visitors_ancestors.bop
    method on_Efun :
      'd ->
      Ast_visitors_ancestors.fun_ ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.is_reference) list ->
      Ast_visitors_ancestors.expr_
    method on_Eif :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Eq :
      'd -> Ast_visitors_ancestors.bop option -> Ast_visitors_ancestors.bop
    method on_Eqeq : 'd -> Ast_visitors_ancestors.bop
    method on_Expr :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
    method on_Omitted: 'd -> Ast_visitors_ancestors.expr_
    method on_Expr_list :
      'd -> Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_FAsync : 'd -> Ast_visitors_ancestors.fun_kind
    method on_FAsyncGenerator : 'd -> Ast_visitors_ancestors.fun_kind
    method on_FGenerator : 'd -> Ast_visitors_ancestors.fun_kind
    method on_FSync : 'd -> Ast_visitors_ancestors.fun_kind
    method on_Fallthrough : 'd -> Ast_visitors_ancestors.stmt
    method on_False : 'd -> Ast_visitors_ancestors.expr_
    method private on_FileInfo_mode :
      'd -> Ast_visitors_ancestors.fimode -> Ast_visitors_ancestors.fimode
    method on_Final : 'd -> Ast_visitors_ancestors.kind
    method on_Float :
      'd -> Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_For :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Foreach :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.pos_t option ->
      Ast_visitors_ancestors.as_expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Fun :
      'd -> Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.def
    method on_GotoLabel : 'd ->
                   Ast_visitors_ancestors.pstring ->
                   Ast_visitors_ancestors.stmt
    method on_Goto : 'd ->
              Ast_visitors_ancestors.pstring ->
              Ast_visitors_ancestors.stmt
    method on_Gt : 'd -> Ast_visitors_ancestors.bop
    method on_Gte : 'd -> Ast_visitors_ancestors.bop
    method on_Gtgt : 'd -> Ast_visitors_ancestors.bop
    method on_Haccess :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id list -> Ast_visitors_ancestors.hint_
    method on_Happly :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.hint_
    method on_Hfun :
      'd ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Hoption :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Hshape :
      'd ->
      Ast_visitors_ancestors.shape_info -> Ast_visitors_ancestors.hint_
    method on_Htuple :
      'd -> Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.hint_
    method on_Hsoft :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Id :
      'd -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Id_type_arguments :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.expr_
    method on_If :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Import :
      'd ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Include : 'd -> Ast_visitors_ancestors.import_flavor
    method on_IncludeOnce : 'd -> Ast_visitors_ancestors.import_flavor
    method on_InstanceOf :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Int :
      'd -> Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_Invariant : 'd -> Ast_visitors_ancestors.variance
    method on_Lfun :
      'd -> Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.expr_
    method on_List :
      'd -> Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Lt : 'd -> Ast_visitors_ancestors.bop
    method on_Lte : 'd -> Ast_visitors_ancestors.bop
    method on_Ltlt : 'd -> Ast_visitors_ancestors.bop
    method on_Lvar :
      'd -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Lvarvar :
      'd -> int -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Method :
      'd ->
      Ast_visitors_ancestors.method_ -> Ast_visitors_ancestors.class_elt
    method on_Minus : 'd -> Ast_visitors_ancestors.bop
    method on_MustExtend : 'd -> Ast_visitors_ancestors.trait_req_kind
    method on_MustImplement : 'd -> Ast_visitors_ancestors.trait_req_kind
    method on_NSClass : 'd -> Ast_visitors_ancestors.ns_kind
    method on_NSNamespace : 'd -> Ast_visitors_ancestors.ns_kind
    method on_NSClassAndNamespace : 'd -> Ast_visitors_ancestors.ns_kind
    method on_NSConst : 'd -> Ast_visitors_ancestors.ns_kind
    method on_NSFun : 'd -> Ast_visitors_ancestors.ns_kind
    method on_Namespace :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.program -> Ast_visitors_ancestors.def
    method on_SetNamespaceEnv :
      'd ->
      Ast_visitors_ancestors.nsenv ->
      Ast_visitors_ancestors.def
    method on_NamespaceUse :
      'd ->
      (Ast_visitors_ancestors.ns_kind * Ast_visitors_ancestors.id *
       Ast_visitors_ancestors.id)
      list -> Ast_visitors_ancestors.def
    method private on_Namespace_env :
      'd -> Ast_visitors_ancestors.nsenv -> Ast_visitors_ancestors.nsenv
    method on_New :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_NewType :
      'd ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.typedef_kind
    method on_Noop : 'd -> Ast_visitors_ancestors.stmt
    method on_Null : 'd -> Ast_visitors_ancestors.expr_
    method on_NullCoalesce :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_OG_nullsafe : 'd -> Ast_visitors_ancestors.og_null_flavor
    method on_OG_nullthrows : 'd -> Ast_visitors_ancestors.og_null_flavor
    method on_Obj_get :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.og_null_flavor -> Ast_visitors_ancestors.expr_
    method on_Percent : 'd -> Ast_visitors_ancestors.bop
    method on_Pipe :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Plus : 'd -> Ast_visitors_ancestors.bop
    method private on_Pos_t :
      'd -> Ast_visitors_ancestors.pos_t -> Ast_visitors_ancestors.pos_t
    method on_Private : 'd -> Ast_visitors_ancestors.kind
    method on_Protected : 'd -> Ast_visitors_ancestors.kind
    method on_Public : 'd -> Ast_visitors_ancestors.kind
    method on_Require : 'd -> Ast_visitors_ancestors.import_flavor
    method on_RequireOnce : 'd -> Ast_visitors_ancestors.import_flavor
    method on_Return :
      'd ->
      Ast_visitors_ancestors.pos_t ->
      Ast_visitors_ancestors.expr option -> Ast_visitors_ancestors.stmt
    method on_SFclass_const :
      'd ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.shape_field_name
    method on_SFlit :
      'd ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.shape_field_name
    method on_Shape :
      'd ->
      (Ast_visitors_ancestors.shape_field_name * Ast_visitors_ancestors.expr)
      list -> Ast_visitors_ancestors.expr_
    method on_Slash : 'd -> Ast_visitors_ancestors.bop
    method on_Star : 'd -> Ast_visitors_ancestors.bop
    method on_Starstar : 'd -> Ast_visitors_ancestors.bop
    method on_Static : 'd -> Ast_visitors_ancestors.kind
    method on_Static_var :
      'd -> Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.stmt
    method on_Global_var :
      'd -> Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.stmt
    method on_Stmt :
      'd -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.def
    method on_String :
      'd -> Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_String2 :
      'd -> Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Switch :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.case list -> Ast_visitors_ancestors.stmt
    method on_Throw :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
    method on_True : 'd -> Ast_visitors_ancestors.expr_
    method on_Try :
      'd ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.catch list ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_TypeConst :
      'd ->
      Ast_visitors_ancestors.typeconst -> Ast_visitors_ancestors.class_elt
    method on_Typedef :
      'd -> Ast_visitors_ancestors.typedef -> Ast_visitors_ancestors.def
    method on_Udecr : 'd -> Ast_visitors_ancestors.uop
    method on_Uincr : 'd -> Ast_visitors_ancestors.uop
    method on_Uminus : 'd -> Ast_visitors_ancestors.uop
    method on_Unop :
      'd ->
      Ast_visitors_ancestors.uop ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Unot : 'd -> Ast_visitors_ancestors.uop
    method on_Unsafe : 'd -> Ast_visitors_ancestors.stmt
    method on_Unsafeexpr :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Updecr : 'd -> Ast_visitors_ancestors.uop
    method on_Upincr : 'd -> Ast_visitors_ancestors.uop
    method on_Uplus : 'd -> Ast_visitors_ancestors.uop
    method on_Uref : 'd -> Ast_visitors_ancestors.uop
    method on_Usplat : 'd -> Ast_visitors_ancestors.uop
    method on_Usilence : 'd -> Ast_visitors_ancestors.uop
    method on_Utild : 'd -> Ast_visitors_ancestors.uop
    method on_While :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_XhpAttr :
      'd ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var ->
      Ast_visitors_ancestors.is_reference ->
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr list)
      option -> Ast_visitors_ancestors.class_elt
    method on_XhpAttrUse :
      'd -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_XhpCategory :
      'd ->
      Ast_visitors_ancestors.pstring list -> Ast_visitors_ancestors.class_elt
    method on_XhpChild :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.class_elt
    method on_xhp_child :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildName :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildList :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child list ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildUnary :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child_op ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildBinary :
      'd ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child

    method on_xhp_child_op : 'd ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildStar : 'd ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildPlus : 'd ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildQuestion : 'd ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op


    method on_Xml :
      'd ->
      Ast_visitors_ancestors.id ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_LogXor : 'd -> Ast_visitors_ancestors.bop
    method on_Xor : 'd -> Ast_visitors_ancestors.bop
    method on_Yield :
      'd -> Ast_visitors_ancestors.afield -> Ast_visitors_ancestors.expr_
    method on_Yield_from :
      'd -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Yield_break : 'd -> Ast_visitors_ancestors.expr_
    method on_afield :
      'd -> Ast_visitors_ancestors.afield -> Ast_visitors_ancestors.afield
    method on_any :
      'd -> Ast_visitors_ancestors.any -> Ast_visitors_ancestors.any
    method on_as_expr :
      'd -> Ast_visitors_ancestors.as_expr -> Ast_visitors_ancestors.as_expr
    method on_attr :
      'd ->
      Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
      (Ast_visitors_ancestors.pos_t * string) *
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr_)
    method on_block :
      'd -> Ast_visitors_ancestors.block -> Ast_visitors_ancestors.block
    method private on_bool :
      'd ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_bop :
      'd -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_ca_field :
      'd ->
      Ast_visitors_ancestors.ca_field -> Ast_visitors_ancestors.ca_field
    method on_ca_type :
      'd -> Ast_visitors_ancestors.ca_type -> Ast_visitors_ancestors.ca_type
    method on_case :
      'd -> Ast_visitors_ancestors.case -> Ast_visitors_ancestors.case
    method on_catch :
      'd ->
      Ast_visitors_ancestors.catch ->
      Ast_visitors_ancestors.id * Ast_visitors_ancestors.id *
      Ast_visitors_ancestors.block
    method on_class_ :
      'd -> Ast_visitors_ancestors.class_ -> Ast_visitors_ancestors.class_
    method on_class_attr :
      'd ->
      Ast_visitors_ancestors.class_attr -> Ast_visitors_ancestors.class_attr
    method on_class_elt :
      'd ->
      Ast_visitors_ancestors.class_elt -> Ast_visitors_ancestors.class_elt
    method on_class_kind :
      'd ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_class_var :
      'd ->
      Ast_visitors_ancestors.class_var ->
      Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.id *
      Ast_visitors_ancestors.expr option
    method on_constraint_kind :
      'd ->
      Ast_visitors_ancestors.constraint_kind ->
      Ast_visitors_ancestors.constraint_kind
    method on_cst_kind :
      'd ->
      Ast_visitors_ancestors.cst_kind -> Ast_visitors_ancestors.cst_kind
    method on_def :
      'd -> Ast_visitors_ancestors.def -> Ast_visitors_ancestors.def
    method on_enum_ :
      'd -> Ast_visitors_ancestors.enum_ -> Ast_visitors_ancestors.enum_
    method on_expr :
      'd ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr_
    method on_expr_ :
      'd -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_field :
      'd ->
      Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr_) *
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr_)
    method on_fun_ :
      'd -> Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.fun_
    method on_fun_kind :
      'd ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_fun_param :
      'd ->
      Ast_visitors_ancestors.fun_param -> Ast_visitors_ancestors.fun_param
    method on_gconst :
      'd -> Ast_visitors_ancestors.gconst -> Ast_visitors_ancestors.gconst
    method on_hint :
      'd ->
      Ast_visitors_ancestors.hint ->
      Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.hint_
    method on_hint_ :
      'd -> Ast_visitors_ancestors.hint_ -> Ast_visitors_ancestors.hint_
    method on_id :
      'd ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.pos_t * string
    method on_import_flavor :
      'd ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method private on_int : 'd -> int -> int
    method on_is_reference :
      'd ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_is_variadic :
      'd ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_kind :
      'd -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method private on_list :
      'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a list -> 'b list
    method on_method_ :
      'd -> Ast_visitors_ancestors.method_ -> Ast_visitors_ancestors.method_
    method on_ns_kind :
      'd -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_og_null_flavor :
      'd ->
      Ast_visitors_ancestors.og_null_flavor ->
      Ast_visitors_ancestors.og_null_flavor
    method private on_option :
      'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a option -> 'b option
    method on_program :
      'd -> Ast_visitors_ancestors.program -> Ast_visitors_ancestors.program
    method on_pstring :
      'd ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.pos_t * string
    method on_shape_field :
      'd ->
      Ast_visitors_ancestors.shape_field ->
      Ast_visitors_ancestors.shape_field
    method on_shape_field_name :
      'd ->
      Ast_visitors_ancestors.shape_field_name ->
      Ast_visitors_ancestors.shape_field_name
    method on_stmt :
      'd -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt
    method private on_string : 'd -> string -> string
    method on_tconstraint :
      'd ->
      Ast_visitors_ancestors.tconstraint ->
      Ast_visitors_ancestors.tconstraint
    method on_tparam :
      'd ->
      Ast_visitors_ancestors.tparam ->
      Ast_visitors_ancestors.variance * Ast_visitors_ancestors.id *
      (Ast_visitors_ancestors.constraint_kind * Ast_visitors_ancestors.hint)
      list
    method on_trait_req_kind :
      'd ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.trait_req_kind
    method on_typeconst :
      'd ->
      Ast_visitors_ancestors.typeconst -> Ast_visitors_ancestors.typeconst
    method on_typedef :
      'd -> Ast_visitors_ancestors.typedef -> Ast_visitors_ancestors.typedef
    method on_typedef_kind :
      'd ->
      Ast_visitors_ancestors.typedef_kind ->
      Ast_visitors_ancestors.typedef_kind
    method on_uop :
      'd -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_user_attribute :
      'd ->
      Ast_visitors_ancestors.user_attribute ->
      Ast_visitors_ancestors.user_attribute
    method on_variance :
      'd ->
      Ast_visitors_ancestors.variance -> Ast_visitors_ancestors.variance
  end

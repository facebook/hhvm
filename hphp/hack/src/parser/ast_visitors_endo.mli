class virtual ['b] endo :
  object ('b)
    constraint 'b =
      < on_ADef : 'c ->
                  Ast_visitors_ancestors.any ->
                  Ast_visitors_ancestors.def -> Ast_visitors_ancestors.any;
        on_AExpr : 'c ->
                   Ast_visitors_ancestors.any ->
                   Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.any;
        on_AFkvalue : 'c ->
                      Ast_visitors_ancestors.afield ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.expr ->
                      Ast_visitors_ancestors.afield;
        on_AFvalue : 'c ->
                     Ast_visitors_ancestors.afield ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.afield;
        on_AHint : 'c ->
                   Ast_visitors_ancestors.any ->
                   Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.any;
        on_AMpamp : 'c ->
                    Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_AProgram : 'c ->
                      Ast_visitors_ancestors.any ->
                      Ast_visitors_ancestors.program ->
                      Ast_visitors_ancestors.any;
        on_AStmt : 'c ->
                   Ast_visitors_ancestors.any ->
                   Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.any;
        on_AbsConst : 'c ->
                      Ast_visitors_ancestors.class_elt ->
                      Ast_visitors_ancestors.hint option ->
                      Ast_visitors_ancestors.id ->
                      Ast_visitors_ancestors.class_elt;
        on_Abstract : 'c ->
                      Ast_visitors_ancestors.kind ->
                      Ast_visitors_ancestors.kind;
        on_Alias : 'c ->
                   Ast_visitors_ancestors.typedef_kind ->
                   Ast_visitors_ancestors.hint ->
                   Ast_visitors_ancestors.typedef_kind;
        on_Amp : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Array : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.afield list ->
                   Ast_visitors_ancestors.expr_;
        on_Darray : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr)
                   list ->
                   Ast_visitors_ancestors.expr_;
        on_Varray : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr list ->
                   Ast_visitors_ancestors.expr_;
        on_Array_get : 'c ->
                       Ast_visitors_ancestors.expr_ ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr option ->
                       Ast_visitors_ancestors.expr_;
        on_As_kv : 'c ->
                   Ast_visitors_ancestors.as_expr ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.as_expr;
        on_As_v : 'c ->
                  Ast_visitors_ancestors.as_expr ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.as_expr;
        on_Attributes : 'c ->
                        Ast_visitors_ancestors.class_elt ->
                        Ast_visitors_ancestors.class_attr list ->
                        Ast_visitors_ancestors.class_elt;
        on_Await : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
        on_BArbar : 'c ->
                    Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Bar : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Binop : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.bop ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
        on_Block : 'c ->
                   Ast_visitors_ancestors.stmt ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.stmt;
       on_BracedExpr : 'c ->
                       Ast_visitors_ancestors.expr_ ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr_;
        on_Break : 'c ->
                   Ast_visitors_ancestors.stmt ->
                   Ast_visitors_ancestors.pos_t ->
                   int option ->
                   Ast_visitors_ancestors.stmt;
        on_CA_enum : 'c ->
                     Ast_visitors_ancestors.ca_type ->
                     string list -> Ast_visitors_ancestors.ca_type;
        on_CA_field : 'c ->
                      Ast_visitors_ancestors.class_attr ->
                      Ast_visitors_ancestors.ca_field ->
                      Ast_visitors_ancestors.class_attr;
        on_CA_hint : 'c ->
                     Ast_visitors_ancestors.ca_type ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.ca_type;
        on_CA_name : 'c ->
                     Ast_visitors_ancestors.class_attr ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.class_attr;
        on_Cabstract : 'c ->
                       Ast_visitors_ancestors.class_kind ->
                       Ast_visitors_ancestors.class_kind;
        on_Call : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr_;
        on_Case : 'c ->
                  Ast_visitors_ancestors.case ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case;
        on_Cast : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.hint ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Cenum : 'c ->
                   Ast_visitors_ancestors.class_kind ->
                   Ast_visitors_ancestors.class_kind;
        on_Cinterface : 'c ->
                        Ast_visitors_ancestors.class_kind ->
                        Ast_visitors_ancestors.class_kind;
        on_Class : 'c ->
                   Ast_visitors_ancestors.def ->
                   Ast_visitors_ancestors.class_ ->
                   Ast_visitors_ancestors.def;
        on_ClassTraitRequire : 'c ->
                               Ast_visitors_ancestors.class_elt ->
                               Ast_visitors_ancestors.trait_req_kind ->
                               Ast_visitors_ancestors.hint ->
                               Ast_visitors_ancestors.class_elt;
        on_ClassUse : 'c ->
                      Ast_visitors_ancestors.class_elt ->
                      Ast_visitors_ancestors.hint ->
                      Ast_visitors_ancestors.class_elt;
        on_ClassUseAlias : 'c ->
                      Ast_visitors_ancestors.class_elt ->
                      Ast_visitors_ancestors.id option ->
                      Ast_visitors_ancestors.pstring ->
                      Ast_visitors_ancestors.id option ->
                      Ast_visitors_ancestors.kind option ->
                      Ast_visitors_ancestors.class_elt;
        on_ClassUsePrecedence : 'c ->
                      Ast_visitors_ancestors.class_elt ->
                      Ast_visitors_ancestors.id ->
                      Ast_visitors_ancestors.pstring ->
                      Ast_visitors_ancestors.id list ->
                      Ast_visitors_ancestors.class_elt;
        on_ClassVars : 'c ->
                       Ast_visitors_ancestors.class_elt ->
                       Ast_visitors_ancestors.kind list ->
                       Ast_visitors_ancestors.hint option ->
                       Ast_visitors_ancestors.class_var list ->
                       string option ->
                       Ast_visitors_ancestors.class_elt;
        on_Class_const : 'c ->
                         Ast_visitors_ancestors.expr_ ->
                         Ast_visitors_ancestors.id ->
                         Ast_visitors_ancestors.pstring ->
                         Ast_visitors_ancestors.expr_;
        on_Class_get : 'c ->
                       Ast_visitors_ancestors.expr_ ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.expr ->
                       Ast_visitors_ancestors.expr_;
        on_Clone : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr_;
       on_Cmp : 'c ->
                Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
       on_Cnormal : 'c ->
                     Ast_visitors_ancestors.class_kind ->
                     Ast_visitors_ancestors.class_kind;
        on_Collection : 'c ->
                        Ast_visitors_ancestors.expr_ ->
                        Ast_visitors_ancestors.id ->
                        Ast_visitors_ancestors.afield list ->
                        Ast_visitors_ancestors.expr_;
        on_Const : 'c ->
                   Ast_visitors_ancestors.class_elt ->
                   Ast_visitors_ancestors.hint option ->
                   (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                   list -> Ast_visitors_ancestors.class_elt;
        on_Constant : 'c ->
                      Ast_visitors_ancestors.def ->
                      Ast_visitors_ancestors.gconst ->
                      Ast_visitors_ancestors.def;
        on_Constraint_as : 'c ->
                           Ast_visitors_ancestors.constraint_kind ->
                           Ast_visitors_ancestors.constraint_kind;
        on_Constraint_eq : 'c ->
                           Ast_visitors_ancestors.constraint_kind ->
                           Ast_visitors_ancestors.constraint_kind;
        on_Constraint_super : 'c ->
                              Ast_visitors_ancestors.constraint_kind ->
                              Ast_visitors_ancestors.constraint_kind;
        on_Continue : 'c ->
                      Ast_visitors_ancestors.stmt ->
                      Ast_visitors_ancestors.pos_t ->
                      int option ->
                      Ast_visitors_ancestors.stmt;
        on_Contravariant : 'c ->
                           Ast_visitors_ancestors.variance ->
                           Ast_visitors_ancestors.variance;
        on_Covariant : 'c ->
                       Ast_visitors_ancestors.variance ->
                       Ast_visitors_ancestors.variance;
        on_Cst_const : 'c ->
                       Ast_visitors_ancestors.cst_kind ->
                       Ast_visitors_ancestors.cst_kind;
        on_Cst_define : 'c ->
                        Ast_visitors_ancestors.cst_kind ->
                        Ast_visitors_ancestors.cst_kind;
        on_Ctrait : 'c ->
                    Ast_visitors_ancestors.class_kind ->
                    Ast_visitors_ancestors.class_kind;
        on_Def_inline : 'c ->
                        Ast_visitors_ancestors.stmt ->
                        Ast_visitors_ancestors.def ->
                        Ast_visitors_ancestors.stmt;
        on_Default : 'c ->
                     Ast_visitors_ancestors.case ->
                     Ast_visitors_ancestors.block ->
                     Ast_visitors_ancestors.case;
        on_Diff : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Diff2 : 'c ->
                   Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Do : 'c ->
                Ast_visitors_ancestors.stmt ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_Dot : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_EQeqeq : 'c ->
                    Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Efun : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.fun_ ->
                  (Ast_visitors_ancestors.id *
                   Ast_visitors_ancestors.is_reference)
                  list -> Ast_visitors_ancestors.expr_;
        on_Eif : 'c ->
                 Ast_visitors_ancestors.expr_ ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr option ->
                 Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Eq : 'c ->
                Ast_visitors_ancestors.bop ->
                Ast_visitors_ancestors.bop option ->
                Ast_visitors_ancestors.bop;
        on_Eqeq : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Expr : 'c ->
                  Ast_visitors_ancestors.stmt ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_Omitted: 'a ->
                  Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_;
        on_Expr_list : 'c ->
                       Ast_visitors_ancestors.expr_ ->
                       Ast_visitors_ancestors.expr list ->
                       Ast_visitors_ancestors.expr_;
        on_FAsync : 'c ->
                    Ast_visitors_ancestors.fun_kind ->
                    Ast_visitors_ancestors.fun_kind;
        on_FAsyncGenerator : 'c ->
                             Ast_visitors_ancestors.fun_kind ->
                             Ast_visitors_ancestors.fun_kind;
        on_FGenerator : 'c ->
                        Ast_visitors_ancestors.fun_kind ->
                        Ast_visitors_ancestors.fun_kind;
        on_FSync : 'c ->
                   Ast_visitors_ancestors.fun_kind ->
                   Ast_visitors_ancestors.fun_kind;
        on_Fallthrough : 'c ->
                         Ast_visitors_ancestors.stmt ->
                         Ast_visitors_ancestors.stmt;
        on_False : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr_;
        on_Final : 'c ->
                   Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind;
        on_Float : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.pstring ->
                   Ast_visitors_ancestors.expr_;
        on_For : 'c ->
                 Ast_visitors_ancestors.stmt ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_Foreach : 'c ->
                     Ast_visitors_ancestors.stmt ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.pos_t option ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.block ->
                     Ast_visitors_ancestors.stmt;
        on_Fun : 'c ->
                 Ast_visitors_ancestors.def ->
                 Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.def;
        on_GotoLabel : 'c ->
                Ast_visitors_ancestors.stmt ->
                Ast_visitors_ancestors.pstring ->
                Ast_visitors_ancestors.stmt;
        on_Goto : 'c ->
                Ast_visitors_ancestors.stmt ->
                Ast_visitors_ancestors.pstring ->
                Ast_visitors_ancestors.stmt;
        on_Markup : 'c ->
                Ast_visitors_ancestors.stmt ->
                Ast_visitors_ancestors.pstring ->
                Ast_visitors_ancestors.expr option ->
                Ast_visitors_ancestors.stmt;
        on_Gt : 'c ->
                Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Gte : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Gtgt : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Haccess : 'c ->
                     Ast_visitors_ancestors.hint_ ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.id list ->
                     Ast_visitors_ancestors.hint_;
        on_Happly : 'c ->
                    Ast_visitors_ancestors.hint_ ->
                    Ast_visitors_ancestors.id ->
                    Ast_visitors_ancestors.hint list ->
                    Ast_visitors_ancestors.hint_;
        on_Hfun : 'c ->
                  Ast_visitors_ancestors.hint_ ->
                  Ast_visitors_ancestors.hint list ->
                  Ast_visitors_ancestors.is_reference ->
                  Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_;
        on_Hoption : 'c ->
                     Ast_visitors_ancestors.hint_ ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.hint_;
        on_Hshape : 'c ->
                    Ast_visitors_ancestors.hint_ ->
                    Ast_visitors_ancestors.shape_info ->
                    Ast_visitors_ancestors.hint_;
        on_Htuple : 'c ->
                    Ast_visitors_ancestors.hint_ ->
                    Ast_visitors_ancestors.hint list ->
                    Ast_visitors_ancestors.hint_;
        on_Hsoft : 'c ->
                     Ast_visitors_ancestors.hint_ ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.hint_;
        on_Id : 'c ->
                Ast_visitors_ancestors.expr_ ->
                Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_;
        on_Id_type_arguments : 'c ->
                               Ast_visitors_ancestors.expr_ ->
                               Ast_visitors_ancestors.id ->
                               Ast_visitors_ancestors.hint list ->
                               Ast_visitors_ancestors.expr_;
        on_If : 'c ->
                Ast_visitors_ancestors.stmt ->
                Ast_visitors_ancestors.expr ->
                Ast_visitors_ancestors.block ->
                Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_Import : 'c ->
                    Ast_visitors_ancestors.expr_ ->
                    Ast_visitors_ancestors.import_flavor ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.expr_;
        on_Include : 'c ->
                     Ast_visitors_ancestors.import_flavor ->
                     Ast_visitors_ancestors.import_flavor;
        on_IncludeOnce : 'c ->
                         Ast_visitors_ancestors.import_flavor ->
                         Ast_visitors_ancestors.import_flavor;
        on_InstanceOf : 'c ->
                        Ast_visitors_ancestors.expr_ ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr_;
        on_Int : 'c ->
                 Ast_visitors_ancestors.expr_ ->
                 Ast_visitors_ancestors.pstring ->
                 Ast_visitors_ancestors.expr_;
        on_Invariant : 'c ->
                       Ast_visitors_ancestors.variance ->
                       Ast_visitors_ancestors.variance;
        on_Lfun : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.expr_;
        on_List : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.expr list ->
                  Ast_visitors_ancestors.expr_;
        on_Lt : 'c ->
                Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Lte : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Ltlt : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Lvar : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_;
        on_Lvarvar : 'c ->
                     Ast_visitors_ancestors.expr_ ->
                     int ->
                     Ast_visitors_ancestors.id ->
                     Ast_visitors_ancestors.expr_;
        on_Method : 'c ->
                    Ast_visitors_ancestors.class_elt ->
                    Ast_visitors_ancestors.method_ ->
                    Ast_visitors_ancestors.class_elt;
        on_Minus : 'c ->
                   Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_MustExtend : 'c ->
                        Ast_visitors_ancestors.trait_req_kind ->
                        Ast_visitors_ancestors.trait_req_kind;
        on_MustImplement : 'c ->
                           Ast_visitors_ancestors.trait_req_kind ->
                           Ast_visitors_ancestors.trait_req_kind;
        on_NSClass : 'c ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_NSNamespace:  'c ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_NSClassAndNamespace : 'c ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_NSConst : 'c ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_NSFun : 'c ->
                   Ast_visitors_ancestors.ns_kind ->
                   Ast_visitors_ancestors.ns_kind;
        on_Namespace : 'c ->
                       Ast_visitors_ancestors.def ->
                       Ast_visitors_ancestors.id ->
                       Ast_visitors_ancestors.program ->
                       Ast_visitors_ancestors.def;
        on_SetNamespaceEnv : 'c ->
                      Ast_visitors_ancestors.def ->
                      Ast_visitors_ancestors.nsenv ->
                      Ast_visitors_ancestors.def;
        on_NamespaceUse : 'c ->
                      Ast_visitors_ancestors.def ->
                      (Ast_visitors_ancestors.ns_kind *
                       Ast_visitors_ancestors.id *
                       Ast_visitors_ancestors.id)
                      list -> Ast_visitors_ancestors.def;
        on_New : 'c ->
                 Ast_visitors_ancestors.expr_ ->
                 Ast_visitors_ancestors.expr ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr_;
        on_NewType : 'c ->
                     Ast_visitors_ancestors.typedef_kind ->
                     Ast_visitors_ancestors.hint ->
                     Ast_visitors_ancestors.typedef_kind;
        on_Noop : 'c ->
                  Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt;
        on_Null : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.expr_;
        on_NullCoalesce : 'c ->
                          Ast_visitors_ancestors.expr_ ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr ->
                          Ast_visitors_ancestors.expr_;
        on_OG_nullsafe : 'c ->
                         Ast_visitors_ancestors.og_null_flavor ->
                         Ast_visitors_ancestors.og_null_flavor;
        on_OG_nullthrows : 'c ->
                           Ast_visitors_ancestors.og_null_flavor ->
                           Ast_visitors_ancestors.og_null_flavor;
        on_Obj_get : 'c ->
                     Ast_visitors_ancestors.expr_ ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.expr ->
                     Ast_visitors_ancestors.og_null_flavor ->
                     Ast_visitors_ancestors.expr_;
        on_Percent : 'c ->
                     Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Pipe : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Plus : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Private : 'c ->
                     Ast_visitors_ancestors.kind ->
                     Ast_visitors_ancestors.kind;
        on_Protected : 'c ->
                       Ast_visitors_ancestors.kind ->
                       Ast_visitors_ancestors.kind;
        on_Public : 'c ->
                    Ast_visitors_ancestors.kind ->
                    Ast_visitors_ancestors.kind;
        on_Require : 'c ->
                     Ast_visitors_ancestors.import_flavor ->
                     Ast_visitors_ancestors.import_flavor;
        on_RequireOnce : 'c ->
                         Ast_visitors_ancestors.import_flavor ->
                         Ast_visitors_ancestors.import_flavor;
        on_Return : 'c ->
                    Ast_visitors_ancestors.stmt ->
                    Ast_visitors_ancestors.pos_t ->
                    Ast_visitors_ancestors.expr option ->
                    Ast_visitors_ancestors.stmt;
        on_SFclass_const : 'c ->
                           Ast_visitors_ancestors.shape_field_name ->
                           Ast_visitors_ancestors.id ->
                           Ast_visitors_ancestors.pstring ->
                           Ast_visitors_ancestors.shape_field_name;
        on_SFlit : 'c ->
                   Ast_visitors_ancestors.shape_field_name ->
                   Ast_visitors_ancestors.pstring ->
                   Ast_visitors_ancestors.shape_field_name;
        on_Shape : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   (Ast_visitors_ancestors.shape_field_name *
                    Ast_visitors_ancestors.expr)
                   list -> Ast_visitors_ancestors.expr_;
        on_Slash : 'c ->
                   Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Star : 'c ->
                  Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Starstar : 'c ->
                      Ast_visitors_ancestors.bop ->
                      Ast_visitors_ancestors.bop;
        on_Static : 'c ->
                    Ast_visitors_ancestors.kind ->
                    Ast_visitors_ancestors.kind;
        on_Static_var : 'c ->
                        Ast_visitors_ancestors.stmt ->
                        Ast_visitors_ancestors.expr list ->
                        Ast_visitors_ancestors.stmt;
        on_Global_var : 'c ->
                        Ast_visitors_ancestors.stmt ->
                        Ast_visitors_ancestors.expr list ->
                        Ast_visitors_ancestors.stmt;
        on_Stmt : 'c ->
                  Ast_visitors_ancestors.def ->
                  Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.def;
        on_String : 'c ->
                    Ast_visitors_ancestors.expr_ ->
                    Ast_visitors_ancestors.pstring ->
                    Ast_visitors_ancestors.expr_;
        on_String2 : 'c ->
                     Ast_visitors_ancestors.expr_ ->
                     Ast_visitors_ancestors.expr list ->
                     Ast_visitors_ancestors.expr_;
        on_Switch : 'c ->
                    Ast_visitors_ancestors.stmt ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.case list ->
                    Ast_visitors_ancestors.stmt;
        on_Throw : 'c ->
                   Ast_visitors_ancestors.stmt ->
                   Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt;
        on_True : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.expr_;
        on_Try : 'c ->
                 Ast_visitors_ancestors.stmt ->
                 Ast_visitors_ancestors.block ->
                 Ast_visitors_ancestors.catch list ->
                 Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt;
        on_TypeConst : 'c ->
                       Ast_visitors_ancestors.class_elt ->
                       Ast_visitors_ancestors.typeconst ->
                       Ast_visitors_ancestors.class_elt;
        on_Typedef : 'c ->
                     Ast_visitors_ancestors.def ->
                     Ast_visitors_ancestors.typedef ->
                     Ast_visitors_ancestors.def;
        on_Udecr : 'c ->
                   Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Uincr : 'c ->
                   Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Uminus : 'c ->
                    Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Unop : 'c ->
                  Ast_visitors_ancestors.expr_ ->
                  Ast_visitors_ancestors.uop ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_;
        on_Unot : 'c ->
                  Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Unsafe : 'c ->
                    Ast_visitors_ancestors.stmt ->
                    Ast_visitors_ancestors.stmt;
        on_Unsafeexpr : 'c ->
                        Ast_visitors_ancestors.expr_ ->
                        Ast_visitors_ancestors.expr ->
                        Ast_visitors_ancestors.expr_;
        on_Updecr : 'c ->
                    Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Upincr : 'c ->
                    Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Uplus : 'c ->
                   Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Uref : 'c ->
                  Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Usplat : 'c ->
                    Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Usilence : 'c ->
                    Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_Utild : 'c ->
                   Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_While : 'c ->
                   Ast_visitors_ancestors.stmt ->
                   Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.stmt;
        on_XhpAttr : 'c ->
                     Ast_visitors_ancestors.class_elt ->
                     Ast_visitors_ancestors.hint option ->
                     Ast_visitors_ancestors.class_var ->
                     Ast_visitors_ancestors.is_reference ->
                     (Ast_visitors_ancestors.pos_t *
                      Ast_visitors_ancestors.expr list)
                     option -> Ast_visitors_ancestors.class_elt;
        on_XhpAttrUse : 'c ->
                        Ast_visitors_ancestors.class_elt ->
                        Ast_visitors_ancestors.hint ->
                        Ast_visitors_ancestors.class_elt;
        on_XhpCategory : 'c ->
                         Ast_visitors_ancestors.class_elt ->
                         Ast_visitors_ancestors.pstring list ->
                         Ast_visitors_ancestors.class_elt;
       on_XhpChild : 'c ->
                        Ast_visitors_ancestors.class_elt ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.class_elt;
        on_xhp_child : 'c ->
                         Ast_visitors_ancestors.xhp_child ->
                         Ast_visitors_ancestors.xhp_child;
        on_ChildName : 'c ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.id ->
                        Ast_visitors_ancestors.xhp_child;

        on_ChildList : 'c ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child list ->
                        Ast_visitors_ancestors.xhp_child;

        on_ChildUnary : 'c ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child_op ->
                        Ast_visitors_ancestors.xhp_child;

        on_ChildBinary : 'c ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child ->
                        Ast_visitors_ancestors.xhp_child;

        on_xhp_child_op : 'c ->
          Ast_visitors_ancestors.xhp_child_op ->
          Ast_visitors_ancestors.xhp_child_op;

        on_ChildStar : 'c ->
          Ast_visitors_ancestors.xhp_child_op ->
          Ast_visitors_ancestors.xhp_child_op;

        on_ChildPlus : 'c ->
          Ast_visitors_ancestors.xhp_child_op ->
          Ast_visitors_ancestors.xhp_child_op;

        on_ChildQuestion : 'c ->
          Ast_visitors_ancestors.xhp_child_op ->
          Ast_visitors_ancestors.xhp_child_op;

        on_Xml : 'c ->
                 Ast_visitors_ancestors.expr_ ->
                 Ast_visitors_ancestors.id ->
                 (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr)
                 list ->
                 Ast_visitors_ancestors.expr list ->
                 Ast_visitors_ancestors.expr_;
        on_LogXor: 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Xor : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_Yield : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.afield ->
                   Ast_visitors_ancestors.expr_;
        on_Yield_from : 'c ->
                    Ast_visitors_ancestors.expr_ ->
                    Ast_visitors_ancestors.expr ->
                    Ast_visitors_ancestors.expr_;
        on_Yield_break : 'c ->
                         Ast_visitors_ancestors.expr_ ->
                         Ast_visitors_ancestors.expr_;
        on_afield : 'c ->
                    Ast_visitors_ancestors.afield ->
                    Ast_visitors_ancestors.afield;
        on_any : 'c ->
                 Ast_visitors_ancestors.any -> Ast_visitors_ancestors.any;
        on_as_expr : 'c ->
                     Ast_visitors_ancestors.as_expr ->
                     Ast_visitors_ancestors.as_expr;
        on_attr : 'c ->
                  Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
                  Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr;
        on_block : 'c ->
                   Ast_visitors_ancestors.block ->
                   Ast_visitors_ancestors.block;
        on_bop : 'c ->
                 Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop;
        on_ca_field : 'c ->
                      Ast_visitors_ancestors.ca_field ->
                      Ast_visitors_ancestors.ca_field;
        on_ca_type : 'c ->
                     Ast_visitors_ancestors.ca_type ->
                     Ast_visitors_ancestors.ca_type;
        on_case : 'c ->
                  Ast_visitors_ancestors.case -> Ast_visitors_ancestors.case;
        on_catch : 'c ->
                   Ast_visitors_ancestors.catch ->
                   Ast_visitors_ancestors.catch;
        on_class_ : 'c ->
                    Ast_visitors_ancestors.class_ ->
                    Ast_visitors_ancestors.class_;
        on_class_attr : 'c ->
                        Ast_visitors_ancestors.class_attr ->
                        Ast_visitors_ancestors.class_attr;
        on_class_elt : 'c ->
                       Ast_visitors_ancestors.class_elt ->
                       Ast_visitors_ancestors.class_elt;
        on_class_kind : 'c ->
                        Ast_visitors_ancestors.class_kind ->
                        Ast_visitors_ancestors.class_kind;
        on_class_var : 'c ->
                       Ast_visitors_ancestors.class_var ->
                       Ast_visitors_ancestors.class_var;
        on_constraint_kind : 'c ->
                             Ast_visitors_ancestors.constraint_kind ->
                             Ast_visitors_ancestors.constraint_kind;
        on_cst_kind : 'c ->
                      Ast_visitors_ancestors.cst_kind ->
                      Ast_visitors_ancestors.cst_kind;
        on_def : 'c ->
                 Ast_visitors_ancestors.def -> Ast_visitors_ancestors.def;
        on_enum_ : 'c ->
                   Ast_visitors_ancestors.enum_ ->
                   Ast_visitors_ancestors.enum_;
        on_expr : 'c ->
                  Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr;
        on_expr_ : 'c ->
                   Ast_visitors_ancestors.expr_ ->
                   Ast_visitors_ancestors.expr_;
        on_field : 'c ->
                   Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
                   Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr;
        on_fun_ : 'c ->
                  Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.fun_;
        on_fun_kind : 'c ->
                      Ast_visitors_ancestors.fun_kind ->
                      Ast_visitors_ancestors.fun_kind;
        on_fun_param : 'c ->
                       Ast_visitors_ancestors.fun_param ->
                       Ast_visitors_ancestors.fun_param;
        on_gconst : 'c ->
                    Ast_visitors_ancestors.gconst ->
                    Ast_visitors_ancestors.gconst;
        on_hint : 'c ->
                  Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint;
        on_hint_ : 'c ->
                   Ast_visitors_ancestors.hint_ ->
                   Ast_visitors_ancestors.hint_;
        on_id : 'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.id;
        on_import_flavor : 'c ->
                           Ast_visitors_ancestors.import_flavor ->
                           Ast_visitors_ancestors.import_flavor;
        on_is_reference : 'c ->
                          Ast_visitors_ancestors.is_reference ->
                          Ast_visitors_ancestors.is_reference;
        on_is_variadic : 'c ->
                         Ast_visitors_ancestors.is_reference ->
                         Ast_visitors_ancestors.is_reference;
        on_kind : 'c ->
                  Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind;
        on_method_ : 'c ->
                     Ast_visitors_ancestors.method_ ->
                     Ast_visitors_ancestors.method_;
        on_ns_kind : 'c ->
                     Ast_visitors_ancestors.ns_kind ->
                     Ast_visitors_ancestors.ns_kind;
        on_og_null_flavor : 'c ->
                            Ast_visitors_ancestors.og_null_flavor ->
                            Ast_visitors_ancestors.og_null_flavor;
        on_program : 'c ->
                     Ast_visitors_ancestors.program ->
                     Ast_visitors_ancestors.program;
        on_pstring : 'c ->
                     Ast_visitors_ancestors.pstring ->
                     Ast_visitors_ancestors.pstring;
        on_shape_field : 'c ->
                         Ast_visitors_ancestors.shape_field ->
                         Ast_visitors_ancestors.shape_field;
        on_shape_field_name : 'c ->
                              Ast_visitors_ancestors.shape_field_name ->
                              Ast_visitors_ancestors.shape_field_name;
        on_stmt : 'c ->
                  Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt;
        on_tconstraint : 'c ->
                         Ast_visitors_ancestors.tconstraint ->
                         Ast_visitors_ancestors.tconstraint;
        on_tparam : 'c ->
                    Ast_visitors_ancestors.tparam ->
                    Ast_visitors_ancestors.tparam;
        on_trait_req_kind : 'c ->
                            Ast_visitors_ancestors.trait_req_kind ->
                            Ast_visitors_ancestors.trait_req_kind;
        on_typeconst : 'c ->
                       Ast_visitors_ancestors.typeconst ->
                       Ast_visitors_ancestors.typeconst;
        on_typedef : 'c ->
                     Ast_visitors_ancestors.typedef ->
                     Ast_visitors_ancestors.typedef;
        on_typedef_kind : 'c ->
                          Ast_visitors_ancestors.typedef_kind ->
                          Ast_visitors_ancestors.typedef_kind;
        on_uop : 'c ->
                 Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop;
        on_user_attribute : 'c ->
                            Ast_visitors_ancestors.user_attribute ->
                            Ast_visitors_ancestors.user_attribute;
        on_variance : 'c ->
                      Ast_visitors_ancestors.variance ->
                      Ast_visitors_ancestors.variance;
        .. >
    method on_ADef :
      'c ->
      Ast_visitors_ancestors.any ->
      Ast_visitors_ancestors.def -> Ast_visitors_ancestors.any
    method on_AExpr :
      'c ->
      Ast_visitors_ancestors.any ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.any
    method on_AFkvalue :
      'c ->
      Ast_visitors_ancestors.afield ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.afield
    method on_AFvalue :
      'c ->
      Ast_visitors_ancestors.afield ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.afield
    method on_AHint :
      'c ->
      Ast_visitors_ancestors.any ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.any
    method on_AMpamp :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_AProgram :
      'c ->
      Ast_visitors_ancestors.any ->
      Ast_visitors_ancestors.program -> Ast_visitors_ancestors.any
    method on_AStmt :
      'c ->
      Ast_visitors_ancestors.any ->
      Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.any
    method on_AbsConst :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.class_elt
    method on_Abstract :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Alias :
      'c ->
      Ast_visitors_ancestors.typedef_kind ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.typedef_kind
    method on_Amp :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Array :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.afield list -> Ast_visitors_ancestors.expr_
    method on_Darray :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      (Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr_
    method on_Varray :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Array_get :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option -> Ast_visitors_ancestors.expr_
    method on_As_kv :
      'c ->
      Ast_visitors_ancestors.as_expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.as_expr
    method on_As_v :
      'c ->
      Ast_visitors_ancestors.as_expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.as_expr
    method on_Attributes :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.class_attr list ->
      Ast_visitors_ancestors.class_elt
    method on_Await :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_BArbar :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Bar :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Binop :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.bop ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Block :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_BracedExpr :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Break :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.pos_t ->
      int option -> Ast_visitors_ancestors.stmt
    method on_CA_enum :
      'c ->
      Ast_visitors_ancestors.ca_type ->
      string list -> Ast_visitors_ancestors.ca_type
    method on_CA_field :
      'c ->
      Ast_visitors_ancestors.class_attr ->
      Ast_visitors_ancestors.ca_field -> Ast_visitors_ancestors.class_attr
    method on_CA_hint :
      'c ->
      Ast_visitors_ancestors.ca_type ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.ca_type
    method on_CA_name :
      'c ->
      Ast_visitors_ancestors.class_attr ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.class_attr
    method on_Cabstract :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_Call :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr_
    method on_Case :
      'c ->
      Ast_visitors_ancestors.case ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case
    method on_Cast :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.hint ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Cenum :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_Cinterface :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_Class :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.class_ -> Ast_visitors_ancestors.def
    method on_ClassTraitRequire :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_ClassUse :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_ClassUseAlias :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id option ->
      Ast_visitors_ancestors.kind option ->
      Ast_visitors_ancestors.class_elt
    method on_ClassUsePrecedence :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.id list ->
      Ast_visitors_ancestors.class_elt
    method on_ClassVars :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.kind list ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var list ->
      string option ->
      Ast_visitors_ancestors.class_elt
    method on_Class_const :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_Class_get :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Clone :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Cmp :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Cnormal :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_Collection :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.afield list -> Ast_visitors_ancestors.expr_
    method on_Const :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.hint option ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.class_elt
    method on_Constant :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.gconst -> Ast_visitors_ancestors.def
    method on_Constraint_as :
      'c ->
      Ast_visitors_ancestors.constraint_kind ->
      Ast_visitors_ancestors.constraint_kind
    method on_Constraint_eq :
      'c ->
      Ast_visitors_ancestors.constraint_kind ->
      Ast_visitors_ancestors.constraint_kind
    method on_Constraint_super :
      'c ->
      Ast_visitors_ancestors.constraint_kind ->
      Ast_visitors_ancestors.constraint_kind
    method on_Continue :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.pos_t ->
      int option -> Ast_visitors_ancestors.stmt
    method on_Contravariant :
      'c ->
      Ast_visitors_ancestors.variance -> Ast_visitors_ancestors.variance
    method on_Covariant :
      'c ->
      Ast_visitors_ancestors.variance -> Ast_visitors_ancestors.variance
    method on_Cst_const :
      'c ->
      Ast_visitors_ancestors.cst_kind -> Ast_visitors_ancestors.cst_kind
    method on_Cst_define :
      'c ->
      Ast_visitors_ancestors.cst_kind -> Ast_visitors_ancestors.cst_kind
    method on_Ctrait :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_Def_inline :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.def -> Ast_visitors_ancestors.stmt
    method on_Default :
      'c ->
      Ast_visitors_ancestors.case ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.case
    method on_Diff :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Diff2 :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Do :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
    method on_Dot :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_EQeqeq :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Efun :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.fun_ ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.is_reference) list ->
      Ast_visitors_ancestors.expr_
    method on_Eif :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr option ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Eq :
      'c ->
      Ast_visitors_ancestors.bop ->
      Ast_visitors_ancestors.bop option -> Ast_visitors_ancestors.bop
    method on_Eqeq :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Expr :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
      method on_Omitted :
        'c ->
        Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_Expr_list :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_FAsync :
      'c ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_FAsyncGenerator :
      'c ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_FGenerator :
      'c ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_FSync :
      'c ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_Fallthrough :
      'c -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt
    method on_False :
      'c -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method private on_FileInfo_mode :
      'c -> Ast_visitors_ancestors.fimode -> Ast_visitors_ancestors.fimode
    method on_Final :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Float :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_For :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Foreach :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.pos_t option ->
      Ast_visitors_ancestors.as_expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Fun :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.def
    method on_GotoLabel : 'c ->
            Ast_visitors_ancestors.stmt ->
            Ast_visitors_ancestors.pstring ->
            Ast_visitors_ancestors.stmt
    method on_Goto : 'c ->
            Ast_visitors_ancestors.stmt ->
            Ast_visitors_ancestors.pstring ->
            Ast_visitors_ancestors.stmt
    method on_Gt :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Gte :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Gtgt :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Haccess :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.id list -> Ast_visitors_ancestors.hint_
    method on_Happly :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.hint_
    method on_Hfun :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.hint list ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Hoption :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Hshape :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.shape_info -> Ast_visitors_ancestors.hint_
    method on_Htuple :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.hint_
    method on_Hsoft :
      'c ->
      Ast_visitors_ancestors.hint_ ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint_
    method on_Id :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Id_type_arguments :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.hint list -> Ast_visitors_ancestors.expr_
    method on_If :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_Import :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Include :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method on_IncludeOnce :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method on_InstanceOf :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Int :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_Invariant :
      'c ->
      Ast_visitors_ancestors.variance -> Ast_visitors_ancestors.variance
    method on_Lfun :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.expr_
    method on_List :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Lt :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Lte :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Ltlt :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Lvar :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Lvarvar :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      int -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.expr_
    method on_Method :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.method_ -> Ast_visitors_ancestors.class_elt
    method on_Minus :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_MustExtend :
      'c ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.trait_req_kind
    method on_MustImplement :
      'c ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.trait_req_kind
    method on_NSClass:
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_NSNamespace :
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_NSClassAndNamespace :
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_NSConst :
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_NSFun :
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_Namespace :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.program -> Ast_visitors_ancestors.def
    method on_SetNamespaceEnv :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.nsenv ->
      Ast_visitors_ancestors.def
    method on_NamespaceUse :
      'c ->
      Ast_visitors_ancestors.def ->
      (Ast_visitors_ancestors.ns_kind * Ast_visitors_ancestors.id *
       Ast_visitors_ancestors.id)
      list -> Ast_visitors_ancestors.def
    method private on_Namespace_env :
      'c -> Ast_visitors_ancestors.nsenv -> Ast_visitors_ancestors.nsenv
    method on_New :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr list ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_NewType :
      'c ->
      Ast_visitors_ancestors.typedef_kind ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.typedef_kind
    method on_Noop :
      'c -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt
    method on_Null :
      'c -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_NullCoalesce :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_OG_nullsafe :
      'c ->
      Ast_visitors_ancestors.og_null_flavor ->
      Ast_visitors_ancestors.og_null_flavor
    method on_OG_nullthrows :
      'c ->
      Ast_visitors_ancestors.og_null_flavor ->
      Ast_visitors_ancestors.og_null_flavor
    method on_Obj_get :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.og_null_flavor -> Ast_visitors_ancestors.expr_
    method on_Percent :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Pipe :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Plus :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method private on_Pos_t :
      'c -> Ast_visitors_ancestors.pos_t -> Ast_visitors_ancestors.pos_t
    method on_Private :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Protected :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Public :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Require :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method on_RequireOnce :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method on_Return :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.pos_t ->
      Ast_visitors_ancestors.expr option -> Ast_visitors_ancestors.stmt
    method on_SFclass_const :
      'c ->
      Ast_visitors_ancestors.shape_field_name ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.shape_field_name
    method on_SFlit :
      'c ->
      Ast_visitors_ancestors.shape_field_name ->
      Ast_visitors_ancestors.pstring ->
      Ast_visitors_ancestors.shape_field_name
    method on_Shape :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      (Ast_visitors_ancestors.shape_field_name * Ast_visitors_ancestors.expr)
      list -> Ast_visitors_ancestors.expr_
    method on_Slash :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Star :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Starstar :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Static :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method on_Static_var :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.stmt
    method on_Global_var :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.stmt
    method on_Stmt :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.def
    method on_String :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.expr_
    method on_String2 :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Switch :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.case list -> Ast_visitors_ancestors.stmt
    method on_Throw :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.stmt
    method on_True :
      'c -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_Try :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.block ->
      Ast_visitors_ancestors.catch list ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_TypeConst :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.typeconst -> Ast_visitors_ancestors.class_elt
    method on_Typedef :
      'c ->
      Ast_visitors_ancestors.def ->
      Ast_visitors_ancestors.typedef -> Ast_visitors_ancestors.def
    method on_Udecr :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Uincr :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Uminus :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Unop :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.uop ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Unot :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Unsafe :
      'c -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt
    method on_Unsafeexpr :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Updecr :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Upincr :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Uplus :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Uref :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Usplat :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Usilence :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_Utild :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_While :
      'c ->
      Ast_visitors_ancestors.stmt ->
      Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.block -> Ast_visitors_ancestors.stmt
    method on_XhpAttr :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.hint option ->
      Ast_visitors_ancestors.class_var ->
      Ast_visitors_ancestors.is_reference ->
      (Ast_visitors_ancestors.pos_t * Ast_visitors_ancestors.expr list)
      option -> Ast_visitors_ancestors.class_elt
    method on_XhpAttrUse :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.class_elt
    method on_XhpCategory :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.pstring list -> Ast_visitors_ancestors.class_elt

    method on_XhpChild :
      'c ->
      Ast_visitors_ancestors.class_elt ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.class_elt

    method on_xhp_child :
      'c ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildName :
      'c ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.id ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildList :
      'c ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child list ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildUnary :
      'c ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child_op ->
      Ast_visitors_ancestors.xhp_child

    method on_ChildBinary :
      'c ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child ->
      Ast_visitors_ancestors.xhp_child

    method on_xhp_child_op : 'c ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildStar : 'c ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildPlus : 'c ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_ChildQuestion : 'c ->
        Ast_visitors_ancestors.xhp_child_op ->
        Ast_visitors_ancestors.xhp_child_op

    method on_Xml :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.id ->
      (Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr) list ->
      Ast_visitors_ancestors.expr list -> Ast_visitors_ancestors.expr_
    method on_Xor :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_LogXor :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_Yield :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.afield -> Ast_visitors_ancestors.expr_
    method on_Yield_from :
      'c ->
      Ast_visitors_ancestors.expr_ ->
      Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr_
    method on_Yield_break :
      'c -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_afield :
      'c -> Ast_visitors_ancestors.afield -> Ast_visitors_ancestors.afield
    method on_any :
      'c -> Ast_visitors_ancestors.any -> Ast_visitors_ancestors.any
    method on_as_expr :
      'c -> Ast_visitors_ancestors.as_expr -> Ast_visitors_ancestors.as_expr
    method on_attr :
      'c ->
      Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.id * Ast_visitors_ancestors.expr
    method on_block :
      'c -> Ast_visitors_ancestors.block -> Ast_visitors_ancestors.block
    method private on_bool :
      'c ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_bop :
      'c -> Ast_visitors_ancestors.bop -> Ast_visitors_ancestors.bop
    method on_ca_field :
      'c ->
      Ast_visitors_ancestors.ca_field -> Ast_visitors_ancestors.ca_field
    method on_ca_type :
      'c -> Ast_visitors_ancestors.ca_type -> Ast_visitors_ancestors.ca_type
    method on_case :
      'c -> Ast_visitors_ancestors.case -> Ast_visitors_ancestors.case
    method on_catch :
      'c -> Ast_visitors_ancestors.catch -> Ast_visitors_ancestors.catch
    method on_class_ :
      'c -> Ast_visitors_ancestors.class_ -> Ast_visitors_ancestors.class_
    method on_class_attr :
      'c ->
      Ast_visitors_ancestors.class_attr -> Ast_visitors_ancestors.class_attr
    method on_class_elt :
      'c ->
      Ast_visitors_ancestors.class_elt -> Ast_visitors_ancestors.class_elt
    method on_class_kind :
      'c ->
      Ast_visitors_ancestors.class_kind -> Ast_visitors_ancestors.class_kind
    method on_class_var :
      'c ->
      Ast_visitors_ancestors.class_var -> Ast_visitors_ancestors.class_var
    method on_constraint_kind :
      'c ->
      Ast_visitors_ancestors.constraint_kind ->
      Ast_visitors_ancestors.constraint_kind
    method on_cst_kind :
      'c ->
      Ast_visitors_ancestors.cst_kind -> Ast_visitors_ancestors.cst_kind
    method on_def :
      'c -> Ast_visitors_ancestors.def -> Ast_visitors_ancestors.def
    method on_enum_ :
      'c -> Ast_visitors_ancestors.enum_ -> Ast_visitors_ancestors.enum_
    method on_expr :
      'c -> Ast_visitors_ancestors.expr -> Ast_visitors_ancestors.expr
    method on_expr_ :
      'c -> Ast_visitors_ancestors.expr_ -> Ast_visitors_ancestors.expr_
    method on_field :
      'c ->
      Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr ->
      Ast_visitors_ancestors.expr * Ast_visitors_ancestors.expr
    method on_fun_ :
      'c -> Ast_visitors_ancestors.fun_ -> Ast_visitors_ancestors.fun_
    method on_fun_kind :
      'c ->
      Ast_visitors_ancestors.fun_kind -> Ast_visitors_ancestors.fun_kind
    method on_fun_param :
      'c ->
      Ast_visitors_ancestors.fun_param -> Ast_visitors_ancestors.fun_param
    method on_gconst :
      'c -> Ast_visitors_ancestors.gconst -> Ast_visitors_ancestors.gconst
    method on_hint :
      'c -> Ast_visitors_ancestors.hint -> Ast_visitors_ancestors.hint
    method on_hint_ :
      'c -> Ast_visitors_ancestors.hint_ -> Ast_visitors_ancestors.hint_
    method on_id :
      'c -> Ast_visitors_ancestors.id -> Ast_visitors_ancestors.id
    method on_import_flavor :
      'c ->
      Ast_visitors_ancestors.import_flavor ->
      Ast_visitors_ancestors.import_flavor
    method private on_int : 'c -> int -> int
    method on_is_reference :
      'c ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_is_variadic :
      'c ->
      Ast_visitors_ancestors.is_reference ->
      Ast_visitors_ancestors.is_reference
    method on_kind :
      'c -> Ast_visitors_ancestors.kind -> Ast_visitors_ancestors.kind
    method private on_list :
      'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a list -> 'a list
    method on_method_ :
      'c -> Ast_visitors_ancestors.method_ -> Ast_visitors_ancestors.method_
    method on_ns_kind :
      'c -> Ast_visitors_ancestors.ns_kind -> Ast_visitors_ancestors.ns_kind
    method on_og_null_flavor :
      'c ->
      Ast_visitors_ancestors.og_null_flavor ->
      Ast_visitors_ancestors.og_null_flavor
    method private on_option :
      'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a option -> 'a option
    method on_program :
      'c -> Ast_visitors_ancestors.program -> Ast_visitors_ancestors.program
    method on_pstring :
      'c -> Ast_visitors_ancestors.pstring -> Ast_visitors_ancestors.pstring
    method on_shape_field :
      'c ->
      Ast_visitors_ancestors.shape_field ->
      Ast_visitors_ancestors.shape_field
    method on_shape_field_name :
      'c ->
      Ast_visitors_ancestors.shape_field_name ->
      Ast_visitors_ancestors.shape_field_name
    method on_stmt :
      'c -> Ast_visitors_ancestors.stmt -> Ast_visitors_ancestors.stmt
    method private on_string : 'c -> string -> string
    method on_tconstraint :
      'c ->
      Ast_visitors_ancestors.tconstraint ->
      Ast_visitors_ancestors.tconstraint
    method on_tparam :
      'c -> Ast_visitors_ancestors.tparam -> Ast_visitors_ancestors.tparam
    method on_trait_req_kind :
      'c ->
      Ast_visitors_ancestors.trait_req_kind ->
      Ast_visitors_ancestors.trait_req_kind
    method on_typeconst :
      'c ->
      Ast_visitors_ancestors.typeconst -> Ast_visitors_ancestors.typeconst
    method on_typedef :
      'c -> Ast_visitors_ancestors.typedef -> Ast_visitors_ancestors.typedef
    method on_typedef_kind :
      'c ->
      Ast_visitors_ancestors.typedef_kind ->
      Ast_visitors_ancestors.typedef_kind
    method on_uop :
      'c -> Ast_visitors_ancestors.uop -> Ast_visitors_ancestors.uop
    method on_user_attribute :
      'c ->
      Ast_visitors_ancestors.user_attribute ->
      Ast_visitors_ancestors.user_attribute
    method on_variance :
      'c ->
      Ast_visitors_ancestors.variance -> Ast_visitors_ancestors.variance
  end

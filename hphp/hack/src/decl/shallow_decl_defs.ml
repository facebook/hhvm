(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type shallow_method = {
  sm_final             : bool;
  sm_abstract          : bool;
  sm_visibility        : Aast.visibility;
  sm_name              : Aast.sid;
  sm_tparams           : Aast.tparam list;
  sm_where_constraints : Aast.where_constraint list;
  sm_variadic          : Nast.fun_variadicity;
  sm_params            : Nast.fun_param list;
  sm_fun_kind          : Ast.fun_kind;
  sm_user_attributes   : Nast.user_attribute list;
  sm_ret               : Aast.hint option;
  sm_ret_by_ref        : bool;
} [@@deriving show]

type shallow_class = {
  sc_mode            : FileInfo.mode;
  sc_final           : bool;
  sc_is_xhp          : bool;
  sc_kind            : Ast.class_kind;
  sc_name            : Aast.sid;
  sc_tparams :
    Aast.tparam list *
    ((Ast.constraint_kind * Ast.hint) list SMap.t);
  sc_extends         : Aast.hint list;
  sc_uses            : Aast.hint list;
  sc_xhp_attr_uses   : Aast.hint list;
  sc_req_extends     : Aast.hint list;
  sc_req_implements  : Aast.hint list;
  sc_implements      : Aast.hint list;
  sc_consts          : Nast.class_const list;
  sc_typeconsts      : Nast.class_typeconst list;
  sc_static_vars     : Nast.static_var list;
  sc_vars            : Nast.class_var list;
  sc_constructor     : shallow_method option;
  sc_static_methods  : shallow_method list;
  sc_methods         : shallow_method list;
  sc_user_attributes : Nast.user_attribute list;
  sc_enum            : Nast.enum_ option;
} [@@deriving show]

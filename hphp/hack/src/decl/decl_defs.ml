(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

(* The following classes are used to make sure we make no typing
 * mistake when interacting with the database. The database knows
 * how to associate a string to a string. We need to deserialize
 * the string and make sure the type is correct. By using these
 * modules, the places where there could be a typing mistake are
 * very well isolated.
*)

type decl_class_type = {
  dc_need_init           : bool;
  dc_members_fully_known : bool;
  dc_abstract            : bool;
  dc_final               : bool;
  dc_deferred_init_members : SSet.t;
  dc_kind                : Ast.class_kind;
  dc_name                : string ;
  dc_pos                 : Pos.t ;
  dc_tparams             : decl tparam list ;
  dc_consts              : class_const SMap.t;
  dc_typeconsts          : typeconst_type SMap.t;
  dc_props               : class_elt SMap.t;
  dc_sprops              : class_elt SMap.t;
  dc_methods             : class_elt SMap.t;
  dc_smethods            : class_elt SMap.t;
  dc_construct           : class_elt option * bool;
  dc_ancestors           : decl ty SMap.t ;
  dc_req_ancestors       : requirement list;
  dc_req_ancestors_extends : SSet.t;
  dc_extends             : SSet.t;
  dc_enum_type           : enum_type option;
}

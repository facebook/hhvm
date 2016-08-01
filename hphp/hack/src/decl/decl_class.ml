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
open Decl_defs

let to_class_type {
  dc_need_init;
  dc_members_fully_known;
  dc_abstract;
  dc_final;
  dc_deferred_init_members;
  dc_kind;
  dc_name;
  dc_pos;
  dc_tparams;
  dc_consts;
  dc_typeconsts;
  dc_props;
  dc_sprops;
  dc_methods;
  dc_smethods;
  dc_construct;
  dc_ancestors;
  dc_req_ancestors;
  dc_req_ancestors_extends;
  dc_extends;
  dc_enum_type;
} = {
  tc_need_init = dc_need_init;
  tc_members_fully_known = dc_members_fully_known;
  tc_abstract = dc_abstract;
  tc_final = dc_final;
  tc_deferred_init_members = dc_deferred_init_members;
  tc_kind = dc_kind;
  tc_name = dc_name;
  tc_pos = dc_pos;
  tc_tparams = dc_tparams;
  tc_consts = dc_consts;
  tc_typeconsts = dc_typeconsts;
  tc_props = dc_props;
  tc_sprops = dc_sprops;
  tc_methods = dc_methods;
  tc_smethods = dc_smethods;
  tc_construct = dc_construct;
  tc_ancestors = dc_ancestors;
  tc_req_ancestors = dc_req_ancestors;
  tc_req_ancestors_extends = dc_req_ancestors_extends;
  tc_extends = dc_extends;
  tc_enum_type = dc_enum_type;
}

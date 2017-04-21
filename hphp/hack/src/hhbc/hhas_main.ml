(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  main_body          : Hhbc_ast.instruct list;
  main_decl_vars     : string list; (* Actually local_id list *)
  main_num_iters     : int;
  main_num_cls_ref_slots : int;
}

let make
  main_body
  main_decl_vars
  main_num_iters
  main_num_cls_ref_slots =
  {
    main_body;
    main_decl_vars;
    main_num_iters;
    main_num_cls_ref_slots;
  }

let body m = m.main_body
let decl_vars m = m.main_decl_vars
let num_iters m = m.main_num_iters
let num_cls_ref_slots m = m.main_num_cls_ref_slots

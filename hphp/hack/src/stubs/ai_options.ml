(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  analyses: string list;
  compute_folded_class_decls_with_hh_distc: bool;
  compute_type_infos_with_hh_distc: bool;
  run_hh_distc_workers_locally: bool;
  unittest_hack_root: Path.t option;
}

let prepare ~server:_ _ =
  {
    analyses = [];
    compute_folded_class_decls_with_hh_distc = false;
    compute_type_infos_with_hh_distc = false;
    run_hh_distc_workers_locally = false;
    unittest_hack_root = None;
  }

let merge_for_unit_tests (options : t) (_ : t) = options

let modify_shared_mem _options config = config

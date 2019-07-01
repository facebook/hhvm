(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json

type decls = {
  funs: Tast.fun_def list;
  typedefs: Tast.typedef list;
  classes: Tast.class_ list;
  consts: Tast.gconst list;
}

let default_decls = { funs = []; classes = []; typedefs = []; consts = []}

let build_json tcopt decls =
  (* to avoid typing errors *)
  let _ = tcopt in
  let _ = decls in

  (* TODO: build json data based on decls for Glean schema *)
  JSON_Array([])

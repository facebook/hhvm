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
  hhas_fun     : Hhas_function.t list;
  hhas_classes : Hhas_class.t list;
}

let make hhas_fun hhas_classes =
  { hhas_fun; hhas_classes }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let from_ast
  (parsed_functions,
  parsed_classes,
  _parsed_typedefs,
  _parsed_consts) =
  let compiled_funs = Emit_function.from_asts parsed_functions in
  let compiled_classes = Emit_class.from_asts parsed_classes in
  let _compiled_typedefs = [] in (* TODO *)
  let _compiled_consts = [] in (* TODO *)
  make compiled_funs compiled_classes

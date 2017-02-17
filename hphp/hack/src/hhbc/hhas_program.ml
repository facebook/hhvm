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
  hhas_fun     : Hhbc_ast.fun_def list;
  hhas_classes : Hhas_class.t list;
}

let make hhas_fun hhas_classes =
  { hhas_fun; hhas_classes }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module A = Ast_php
module PI = Parse_info
module V = Visitor_php

let changed = ref false

let harden_param (k, _) param = begin
  match param.A.p_soft_type with
    | Some tok -> (tok.PI.transfo <- PI.Remove; changed := true)
    | None -> ()
  end;
  k param

let harden_func (k, _) func = begin
  match func.A.f_return_type with
    | Some (_, Some tok, _) -> (tok.PI.transfo <- PI.Remove; changed := true)
    | _ -> ()
  end;
  k func

(* Removes all the "@" sigils in front of every typehint. *)
let harden_visitor =
  V.mk_visitor { V.default_visitor with
    V.kparameter = harden_param;
    V.kfunc_def = harden_func;
    V.kmethod_def = harden_func;
  }

let go fn =
  changed := false;
  let (ast, toks) = Parse_php.ast_and_tokens fn in
  harden_visitor (A.Program ast);
  if !changed then begin
    let s =
      Unparse_php.string_of_program_with_comments_using_transfo (ast, toks) in
    Common.write_file fn s
  end

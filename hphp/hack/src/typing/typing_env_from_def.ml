(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Env = Typing_env

(*****************************************************************************)
(* Construct a Typing_env from an AAST toplevel definition.
 * Functorizing these env construction functions allows them to be used with
 * both the NAST and the TAST. *)
(*****************************************************************************)

module EnvFromDef(ASTAnnotations: Aast.ASTAnnotationTypes) = struct
  module AnnotatedAST = Aast.AnnotatedAST(ASTAnnotations)
  open AnnotatedAST

  let fun_env tcopt f =
    let file = Pos.filename (fst f.f_name) in
    let droot = Some (Typing_deps.Dep.Fun (snd f.f_name)) in
    let env = Env.empty tcopt file ~droot in
    env

  let class_env tcopt c =
    let file = Pos.filename (fst c.c_name) in
    let droot = Some (Typing_deps.Dep.Class (snd c.c_name)) in
    let env = Env.empty tcopt file ~droot in
    env

  let typedef_env tcopt t =
    let file = Pos.filename (fst t.t_kind) in
    let droot = Some (Typing_deps.Dep.Class (snd t.t_name)) in
    let env = Env.empty tcopt file ~droot in
    env

  let gconst_env tcopt cst =
    let file = Pos.filename (fst cst.cst_name) in
    let droot = Some (Typing_deps.Dep.GConst (snd cst.cst_name)) in
    let env = Env.empty tcopt file ~droot in
    env
end

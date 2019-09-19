(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 * This module contains smart constructors implementation that can be used to
 * build AST.
 
 *)

module type SC_S = SmartConstructors.SmartConstructors_S

module SK = Full_fidelity_syntax_kind

module type SyntaxKind_S = sig
  include SC_S

  type original_sc_r [@@deriving show]
end

module SyntaxKind (SC : SC_S) :
  SyntaxKind_S
    with module Token = SC.Token
     and type original_sc_r = SC.r
     and type t = SC.t = struct
  module Token = SC.Token

  type original_sc_r = SC.r [@@deriving show]

  type t = SC.t [@@deriving show]

  type r = SK.t * SC.r [@@deriving show]

  let compose : SK.t -> t * SC.r -> t * r =
   (fun kind (state, res) -> (state, (kind, res)))

  let rust_parse text env =
    let (state, res, errors, pointer) = SC.rust_parse text env in
    let (state, res) = compose SK.Script (state, res) in
    (state, res, errors, pointer)

  let initial_state = SC.initial_state
end

(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO: Make these not default to positioned_syntax *)
module PositionedSyntax = Full_fidelity_positioned_syntax
module CoroutineSC = Coroutine_smart_constructor.WithSyntax (PositionedSyntax)
module PositionedSyntaxTree_ =
  Full_fidelity_syntax_tree.WithSyntax (PositionedSyntax)
module PositionedSyntaxTree =
  PositionedSyntaxTree_.WithSmartConstructors (CoroutineSC)

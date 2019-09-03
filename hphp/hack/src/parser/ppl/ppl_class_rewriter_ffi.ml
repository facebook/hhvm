(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text
module EditablePositionedSyntax = Full_fidelity_editable_positioned_syntax

external parse_and_rewrite_ppl_classes :
  SourceText.t -> EditablePositionedSyntax.t = "parse_and_rewrite_ppl_classes"

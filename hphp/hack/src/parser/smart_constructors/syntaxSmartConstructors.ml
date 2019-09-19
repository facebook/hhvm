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

module ParserEnv = Full_fidelity_parser_env

module type State_S = sig
  type r [@@deriving show]

  type t [@@deriving show]

  val initial : ParserEnv.t -> t

  val next : t -> r list -> t
end

module type RustParser_S = sig
  type t

  type r

  val rust_parse :
    Full_fidelity_source_text.t ->
    ParserEnv.t ->
    t * r * Full_fidelity_syntax_error.t list * Rust_pointer.t option
end

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithState (State : State_S with type r = Syntax.t) = struct
    module WithRustParser
        (RustParser : RustParser_S with type t = State.t with type r = Syntax.t) =
    struct
      module Token = Syntax.Token

      type t = State.t [@@deriving show]

      type r = Syntax.t [@@deriving show]

      let rust_parse = RustParser.rust_parse

      let initial_state = State.initial
    end
  end

  include WithState (struct
    type r = Syntax.t [@@deriving show]

    type t = unit [@@deriving show]

    let initial _ = ()

    let next () _ = ()
  end)

  include WithRustParser (struct
    type r = Syntax.t

    type t = unit

    let rust_parse = Syntax.rust_parse
  end)
end

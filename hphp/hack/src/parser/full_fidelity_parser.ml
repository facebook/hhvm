(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Full_fidelity_parser_env

module type SC_S = SmartConstructors.SmartConstructors_S

module type SCWithToken_S = SmartConstructorsWrappers.SyntaxKind_S

[@@@ocaml.warning "-60"] (* https://caml.inria.fr/mantis/view.php?id=7522 *)

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithSmartConstructors (SCI : SC_S with module Token = Syntax.Token) : sig
    type t

    val make : Env.t -> Full_fidelity_source_text.t -> t

    val errors : t -> Full_fidelity_syntax_error.t list

    val env : t -> Env.t

    val sc_state : t -> SCI.t

    val parse_script : t -> t * SCI.r * Rust_pointer.t option
  end = struct
    module SCWithToken = SmartConstructorsWrappers.SyntaxKind (SCI)
    module SourceText = Full_fidelity_source_text
    module SyntaxError = Full_fidelity_syntax_error

    type t = {
      text: SourceText.t;
      errors: SyntaxError.t list;
      env: Env.t;
      sc_state: SCWithToken.t;
    }

    let make (env : Env.t) text =
      { text; errors = []; env; sc_state = SCWithToken.initial_state env }

    let errors parser = parser.errors

    let env parser = parser.env

    let sc_state parser = parser.sc_state

    let rust_parse_script parser =
      let (sc_state, root, errors, rust_tree) =
        SCI.rust_parse parser.text parser.env
      in
      (match rust_tree with
      | Some tree -> Rust_pointer.register_leaked_pointer tree
      | None -> ());
      ({ parser with errors; sc_state }, root, rust_tree)

    let parse_script parser = rust_parse_script parser
  end

  module SC = SyntaxSmartConstructors.WithSyntax (Syntax)
  include WithSmartConstructors (SC)
end

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_minimal_syntax
module Parser = WithSyntax (Syntax)

let parse_mode = Rust_parser_ffi.parse_mode

let () = Rust_parser_ffi.init ()

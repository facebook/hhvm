(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module WithSyntax (Syntax : Syntax_sig.Syntax_S) : sig
  module WithSmartConstructors
      (SmartConstructors : SmartConstructors.SmartConstructors_S
                             with type r = Syntax.t
                             with module Token = Syntax.Token) : sig
    type t [@@deriving show]

    val make :
      ?env:Full_fidelity_parser_env.t -> Full_fidelity_source_text.t -> t

    val create :
      Full_fidelity_source_text.t ->
      Syntax.t ->
      Rust_pointer.t option ->
      Full_fidelity_syntax_error.t list ->
      FileInfo.mode option ->
      SmartConstructors.t ->
      t

    val build :
      Full_fidelity_source_text.t ->
      Syntax.t ->
      Rust_pointer.t option ->
      Full_fidelity_syntax_error.t list ->
      FileInfo.mode option ->
      SmartConstructors.t ->
      t

    val root : t -> Syntax.t

    val rust_tree : t -> Rust_pointer.t option

    val text : t -> Full_fidelity_source_text.t

    val sc_state : t -> SmartConstructors.t

    val all_errors : t -> Full_fidelity_syntax_error.t list

    val errors : t -> Full_fidelity_syntax_error.t list

    val mode : t -> FileInfo.mode option

    val is_strict : t -> bool

    val is_hhi : t -> bool

    val to_json : ?with_value:bool -> ?ignore_missing:bool -> t -> Hh_json.json

    val parse_tree_to_json :
      ?with_value:bool -> ?ignore_missing:bool -> t -> Hh_json.json
  end

  include module type of
      WithSmartConstructors (SyntaxSmartConstructors.WithSyntax (Syntax))

  val create :
    Full_fidelity_source_text.t ->
    Syntax.t ->
    Full_fidelity_syntax_error.t list ->
    FileInfo.mode option ->
    t

  val build :
    Full_fidelity_source_text.t ->
    Syntax.t ->
    Full_fidelity_syntax_error.t list ->
    FileInfo.mode option ->
    t
end

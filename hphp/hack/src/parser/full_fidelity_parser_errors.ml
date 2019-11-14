(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithSmartConstructors
      (SCI : SmartConstructors.SmartConstructors_S
               with type r = Syntax.t
               with module Token = Syntax.Token) =
  struct
    module SyntaxTree_ = Full_fidelity_syntax_tree.WithSyntax (Syntax)
    module SyntaxTree = SyntaxTree_.WithSmartConstructors (SCI)
    module SyntaxError = Full_fidelity_syntax_error

    type error_level =
      | Minimum
      | Typical
      | Maximum

    type hhvm_compat_mode =
      | NoCompat
      | HHVMCompat

    type env = {
      syntax_tree: SyntaxTree.t;
      level: error_level;
      hhvm_compat_mode: hhvm_compat_mode;
      codegen: bool;
      hhi_mode: bool;
      parser_options: ParserOptions.t;
    }

    let make_env
        ?(level = Typical)
        ?(hhvm_compat_mode = NoCompat)
        ?(hhi_mode = false)
        ~(parser_options : ParserOptions.t)
        (syntax_tree : SyntaxTree.t)
        ~(codegen : bool) : env =
      {
        syntax_tree;
        level;
        hhvm_compat_mode;
        codegen;
        hhi_mode;
        parser_options;
      }

    let start_offset n =
      let s = Syntax.position Relative_path.default n in
      let (s, _) = Pos.info_raw (Option.value ~default:Pos.none s) in
      s

    let end_offset n =
      let e = Syntax.position Relative_path.default n in
      let (_, e) = Pos.info_raw (Option.value ~default:Pos.none e) in
      e

    let make_error_from_nodes
        ?(error_type = SyntaxError.ParseError) start_node end_node error =
      let s = start_offset start_node in
      let e = end_offset end_node in
      SyntaxError.make ~error_type s e error

    let make_error_from_node ?(error_type = SyntaxError.ParseError) node error =
      make_error_from_nodes ~error_type node node error

    let find_syntax_errors env =
      match SyntaxTree.rust_tree env.syntax_tree with
      | Some rust_tree ->
        Rust_pointer.unregister_leaked_pointer rust_tree;
        Syntax.rust_parser_errors
          (SyntaxTree.text env.syntax_tree)
          rust_tree
          (ParserOptions.to_rust_ffi_t
             env.parser_options
             ~hhvm_compat_mode:(env.hhvm_compat_mode <> NoCompat)
             ~hhi_mode:env.hhi_mode
             ~codegen:env.codegen)
      | None ->
        failwith
          "expected to find Rust tree. ~leak_rust_tree was not set correctly somwhere earlier"

    let parse_errors env =
      (*
  Minimum: suppress cascading errors; no second-pass errors if there are
  any first-pass errors.
  Typical: suppress cascading errors; give second pass errors always.
  Maximum: all errors
  *)
      try
        let errors1 =
          match env.level with
          | Maximum -> SyntaxTree.all_errors env.syntax_tree
          | _ -> SyntaxTree.errors env.syntax_tree
        in
        let errors2 =
          if env.level = Minimum && errors1 <> [] then
            []
          else
            find_syntax_errors env
        in
        List.sort SyntaxError.compare (List.append errors1 errors2)
      with e ->
        let error_msg = "UNEXPECTED_ERROR: " ^ Exn.to_string e in
        [make_error_from_node (SyntaxTree.root env.syntax_tree) error_msg]
  end

  include WithSmartConstructors (SyntaxSmartConstructors.WithSyntax (Syntax))
end

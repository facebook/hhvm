(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

type t = {
  extern_uses: SSet.t;
  (* names of types (or derive macros) to import from other Rust crates *)
  uses: SSet.t;
  (* names of referenced modules (need to be explicitly imported in Rust) *)
  glob_uses: SSet.t;
  (* names of opened modules (to convert to glob-imports in Rust) *)
  aliases: (string * string) list;
  (* (module_name, alias) pairs *)
  includes: SSet.t;
  (* names of directly-included modules *)
  ty_uses: (string * string) list;
  (* (module_name, type_name) pairs *)
  decls: (string * string) list;
      (* (name, rust_syntax_for_entire_declaration) *)
}
(** This type is mostly strings for the sake of making conversion easy, but we
    retain some structure for the postprocessing and formatting we do in
    {!Stringify}. *)

let empty =
  {
    extern_uses = SSet.empty;
    uses = SSet.empty;
    glob_uses = SSet.empty;
    aliases = [];
    includes = SSet.empty;
    ty_uses = [];
    decls = [];
  }

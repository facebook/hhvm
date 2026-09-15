(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This type is mostly strings for the sake of making conversion easy, but we
    retain some structure for the postprocessing and formatting we do in
    {!Stringify}. *)
type t = {
  extern_uses: S_set.t;
  (* names of types (or derive macros) to import from other Rust crates *)
  glob_uses: S_set.t;
  (* names of opened modules (to convert to glob-imports in Rust) *)
  aliases: (string * string) list;
  (* (module_name, alias) pairs *)
  includes: S_set.t;
  (* names of directly-included modules *)
  ty_reexports: string list;
  (* fully-qualified type names to be re-exported *)
  decls: (string * string) list; (* (name, rust_syntax_for_entire_declaration) *)
}

let empty =
  {
    extern_uses = S_set.empty;
    glob_uses = S_set.empty;
    aliases = [];
    includes = S_set.empty;
    ty_reexports = [];
    decls = [];
  }

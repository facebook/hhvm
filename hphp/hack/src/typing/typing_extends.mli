(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [check_implements_extends_uses env ~implements ~parents type_]
    checks that a classish type implements its interfaces,
    extends its base class, and uses its traits.

  This actually only checks member compatibility. Other hierarchy
  checks like requirements (e.g. `require extends`) are checked
  in Typing_class.

  @param implements   the list of interfaces the classish type directly implements.
  @param parents      the list of direct ancestors, including interfaces and
                      traits, the class directly uses.
  @param type_        the classish type to check.
  *)
val check_implements_extends_uses :
  Typing_env_types.env ->
  implements:Typing_defs.decl_ty list ->
  parents:(Aast.hint * Typing_defs.decl_ty) list ->
  Nast.class_ * Decl_provider.Class.t ->
  Typing_env_types.env

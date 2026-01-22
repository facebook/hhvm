(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Produce warnings for "needs concrete" violations.
  * https://fburl.com/hack_needs_concrete
*)
val check_class_get :
  Typing_env_types.env ->
  Pos.t ->
  Pos_or_decl.t ->
  string ->
  string ->
  Typing_defs.class_elt ->
  ('ex, 'en) Aast_defs.class_id_ ->
  bool ->
  unit

(** Produce warnings for "needs concrete" violations.
 *  https://fburl.com/hack_needs_concrete
 *)
val check_instantiation :
  Typing_env_types.env -> Pos.t -> ('ex, 'en) Aast_defs.class_id_ -> unit

(** Validate `<<__NeedsConcrete>>` usage on class members.
 *  - Errors on instance methods and constructors.
 *  - Errors on static methods in final classes.
 *)
val check_class_def :
  Typing_env_types.env -> Nast.class_ -> Decl_provider.class_decl -> unit

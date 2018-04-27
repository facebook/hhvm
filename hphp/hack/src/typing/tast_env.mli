(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env
type t = env

val print_ty : env -> 'a Typing_defs.ty -> string
(** Return a string representation of the given type using Hack-like syntax. *)

val print_ty_with_identity :
  env ->
  'a Typing_defs.ty ->
  'b SymbolOccurrence.t ->
  'b SymbolDefinition.t option ->
  string
(** Return a string representation of the given type using Hack-like syntax,
    formatted with limited width and line breaks, including additional
    information from the {!SymbolOccurrence.t} and (if provided)
    {!SymbolDefinition.t}. *)

val ty_to_json : env -> 'a Typing_defs.ty -> Hh_json.json
(** Return a JSON representation of the given type. *)

val get_self_id : env -> string
(** Return the name of the enclosing class definition.
    When not in a class definition, return the empty string. *)

val get_self : env -> Tast.ty
(** Return the type of the enclosing class definition.
    When not in a class definition, return a {!Typing_defs.Tany}) type. *)

val is_static : env -> bool
(** Return {true} when in the definition of a static property or method. *)

val get_tcopt : env -> TypecheckerOptions.t
(** Return the {!TypecheckerOptions.t} with which this TAST was checked. *)

val expand_type : env -> Tast.ty -> env * Tast.ty
(** Expand a type variable ({!Typing_defs.Tvar}) to the type it refers to. *)

val fully_expand : env -> Tast.ty -> Tast.ty
(** Eliminate type variables ({!Typing_defs.Tvar}) in the given type by
    recursively replacing them with the type they refer to. *)

val get_class_ids : env -> Tast.ty -> string list
(** Given some class type or unresolved union of class types, return the
    identifiers of all classes the type may represent. *)

val is_visible :
  env ->
  Typing_defs.visibility ->
  Nast.class_id_ option ->
  Typing_defs.class_type ->
  bool
(** Return {true} if the given {class_type} (referred to by the given
    {class_id_}, if provided) allows the current class (the one returned by
    {!get_self}) to access its members with the given {visibility}. *)

val hint_to_ty : env -> Aast.hint -> Typing_defs.decl Typing_defs.ty
(** Return the declaration-phase type the given hint represents. *)

val localize_with_self : env -> Typing_defs.decl Typing_defs.ty -> env * Tast.ty
(** Transforms a declaration phase type ({!Typing_defs.decl Typing_defs.ty})
    into a localized type ({!Typing_defs.locl Typing_defs.ty} = {!Tast.ty}).
    Performs no substitutions of generics and initializes the late static bound
    type ({!Typing_defs.Tthis}) to the current class type (the type returned by
    {!get_self}).

    This is mostly provided as legacy support for {!AutocompleteService}, and
    should not be considered a general mechanism for transforming a {decl ty} to
    a {!Tast.ty}. *)

val referenced_typeconsts :
  env -> Aast.hint -> Aast.sid list -> (string * string * Pos.t) list
(** Returns (class_name, tconst_name, tconst_reference_position) for each type
    constant referenced in the type access path. *)

val set_static : env -> env
(** Return an {!env} for which {!is_static} will return {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)

val save : env -> Tast.saved_env
(** Return the subset of this {!env} which is persisted in a TAST.
    It should usually not be necessary to invoke this. *)

val empty : TypecheckerOptions.t -> env
(** Construct an empty {!env}. Unlikely to be the best choice; prefer using
    {!Tast_visitor} or constructing an {!env} from a {!Tast.def}. *)

(** Construct an {!env} from a toplevel definition. *)

val fun_env     : Tast.fun_    -> env
val class_env   : Tast.class_  -> env
val typedef_env : Tast.typedef -> env
val gconst_env  : Tast.gconst  -> env

val restore_method_env : env -> Tast.method_ -> env
(** Construct an {!env} from a method definition and the {!env} of the context
    it appears in. *)

val restore_fun_env : env -> Tast.fun_ -> env
(** Construct an {!env} from a lambda definition and the {!env} of the context
    it appears in. *)

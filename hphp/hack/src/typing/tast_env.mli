(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env [@@deriving show]
type t = env [@@deriving show]

exception Not_in_class

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

val get_self_id_exn : env -> string
(** Return the name of the enclosing class definition.
    When not in a class definition, raise {!Not_in_class}. *)

val get_self_id : env -> string option
(** Return the name of the enclosing class definition.
    When not in a class definition, return {!None}. *)

val get_self_exn : env -> Tast.ty
(** Return the type of the enclosing class definition.
    When not in a class definition, raise {!Not_in_class}. *)

val get_self : env -> Tast.ty option
(** Return the type of the enclosing class definition.
    When not in a class definition, return {!None}. *)

val is_static : env -> bool
(** Return {true} when in the definition of a static property or method. *)

val is_strict : env -> bool
(** Return {true} if the containing file was checked in strict mode. *)

val in_loop : env -> bool
(** Return {true} when inside any For, Do, While, or Foreach statement. *)

val get_tcopt : env -> TypecheckerOptions.t
(** Return the {!TypecheckerOptions.t} with which this TAST was checked. *)

val forward_compat_ge : env -> int -> bool
(** Return {true} if the forward compatibility level is newer than the given ISO
    date (represented with an int; e.g. 2018_06_14) *)

val error_if_forward_compat_ge : env -> int -> (unit -> unit) -> unit
(** Call the func if the forward compatibility level is new enough *)

val get_file : env -> Relative_path.t
(* Return the {!Relative_path.t} of the file the env is from *)

val expand_type : env -> Tast.ty -> env * Tast.ty
(** Expand a type variable ({!Typing_defs.Tvar}) to the type it refers to. *)

val fully_expand : env -> Tast.ty -> Tast.ty
(** Eliminate type variables ({!Typing_defs.Tvar}) in the given type by
    recursively replacing them with the type they refer to. *)

val get_class_ids : env -> Tast.ty -> string list
(** Given some class type or unresolved union of class types, return the
    identifiers of all classes the type may represent. *)

val fold_unresolved : env -> Tast.ty -> env * Tast.ty
(** Try to unify all the types in an unresolved union. *)

val flatten_unresolved : env -> Tast.ty -> Tast.ty list -> env * Tast.ty list
(** Flatten nested unresolved unions, turning ((A | B) | C) to (A | B | C). *)

val push_option_out : env -> Tast.ty -> env * Tast.ty
(** Strip away all Toptions that we possibly can in a type, expanding type
    variables along the way, turning ?T -> T. *)

val get_concrete_supertypes : env -> Tast.ty -> env * Tast.ty list
(** Get the "as" constraints from an abstract type or generic parameter, or
    return the type itself if there is no "as" constraint. In the case of a
    generic parameter whose "as" constraint is another generic parameter, repeat
    the process until a type is reached that is not a generic parameter. Don't
    loop on cycles. (For example, function foo<Tu as Tv, Tv as Tu>(...)) *)

val is_visible :
  env ->
  Typing_defs.visibility ->
  Nast.class_id_ option ->
  Typing_defs.class_type ->
  bool
(** Return {true} if the given {class_type} (referred to by the given
    {class_id_}, if provided) allows the current class (the one returned by
    {!get_self}) to access its members with the given {visibility}. *)

val assert_nontrivial : Pos.t -> Ast.bop -> env -> Tast.ty -> Tast.ty -> unit
(** Assert that the types of values involved in a strict (non-)equality
    comparison are compatible; e.g., that the types are not statically
    known to be disjoint, in which case the comparison will always return
    true or false. *)

val assert_nullable : Pos.t -> Ast.bop -> env -> Tast.ty -> unit
(** Assert that the type of a value involved in a strict (non-)equality
    comparsion to null is nullable (otherwise it is known to always
    return true or false). *)

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

val localize_with_dty_validator:
  env ->
  Typing_defs.decl Typing_defs.ty ->
  (Typing_defs.decl Typing_defs.ty -> unit) ->
  env * Tast.ty
(** Identical to localize_with_self, but also takes a validator that is applied
    to every expanded decl type on the way to becoming a locl type. *)

val get_upper_bounds: env -> string -> Type_parameter_env.tparam_bounds
(** Get the upper bounds of the type parameter with the given name. *)

val is_fresh_generic_parameter: string -> bool
(** Return whether the type parameter with the given name was implicity created
    as part of an `instamceof`, `is`, or `as` expression (instead of being
    explicitly declared in code by the user). *)

val subtype: env -> Tast.ty -> Tast.ty -> env * bool
(** Return {true} when the first type is a subtype of the second type. *)

val referenced_typeconsts :
  env -> Aast.hint -> Aast.sid list -> (string * string * Pos.t) list
(** Returns (class_name, tconst_name, tconst_reference_position) for each type
    constant referenced in the type access path. *)

val set_static : env -> env
(** Return an {!env} for which {!is_static} will return {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)

val set_inside_constructor : env -> env
(** Returns an {!env} for which {!inside_constructor} is set to {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)

val set_in_loop : env -> env
(** Return an {!env} for which {!in_loop} will return {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)

val get_inside_constructor : env -> bool
(** Returns whether or not the typing environment is inside the
    constructor of a class *)

val get_decl_env : env -> Decl_env.env
(** Returns a {!Decl_env.env} *)

val get_inside_ppl_class : env -> bool
(** Returns whether or not the typing environment is
    inside a <<__PPL>> annotated class. *)

val save : env -> Tast.saved_env
(** Return the subset of this {!env} which is persisted in a TAST.
    It should usually not be necessary to invoke this. *)

val empty : TypecheckerOptions.t -> env
(** Construct an empty {!env}. Unlikely to be the best choice; prefer using
    {!Tast_visitor} or constructing an {!env} from a {!Tast.def}. *)

val def_env : Tast.def -> env
(** Construct an {!env} from a toplevel definition. *)

val restore_method_env : env -> Tast.method_ -> env
(** Construct an {!env} from a method definition and the {!env} of the context
    it appears in. *)

val restore_fun_env : env -> Tast.fun_ -> env
(** Construct an {!env} from a lambda definition and the {!env} of the context
    it appears in. *)

val set_ppl_lambda : env -> env
(** Construct an {!env} where inside_ppl_class is {false}. Due to rewriting
    limitations, we are unable to rewrite lambdas inside <<__PPL>> classes.
    If you are using {!Tast_visitor}, you should have no need of this. *)

val get_anonymous_lambda_types : env -> int -> Tast.ty list

val typing_env_as_tast_env : Typing_env.env -> env

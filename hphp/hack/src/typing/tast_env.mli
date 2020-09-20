(*
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

(** Return a string representation of the given type using Hack-like syntax. *)
val print_ty : env -> Typing_defs.locl_ty -> string

val print_decl_ty : env -> Typing_defs.decl_ty -> string

val print_error_ty :
  ?ignore_dynamic:bool -> env -> Typing_defs.locl_ty -> string

(** Return a string representation of the given type using Hack-like syntax,
    formatted with limited width and line breaks, including additional
    information from the {!SymbolOccurrence.t} and (if provided)
    {!SymbolDefinition.t}. *)
val print_ty_with_identity :
  env ->
  Typing_defs.phase_ty ->
  'b SymbolOccurrence.t ->
  'b SymbolDefinition.t option ->
  string

(** Return a JSON representation of the given type. *)
val ty_to_json : env -> Typing_defs.locl_ty -> Hh_json.json

(** Convert a JSON representation of a type back into a locl-phase type. *)
val json_to_locl_ty :
  ?keytrace:Hh_json.Access.keytrace ->
  Provider_context.t ->
  Hh_json.json ->
  (Typing_defs.locl_ty, Typing_defs.deserialization_error) result

(** Return the name of the enclosing class definition.
    When not in a class definition, return {!None}. *)
val get_self_id : env -> string option

(** Return the type of the enclosing class definition.
    When not in a class definition, return {!None}. *)
val get_self_ty : env -> Tast.ty option

(** Return the type of the enclosing class definition.
    When not in a class definition, raise {!Not_in_class}. *)
val get_self_ty_exn : env -> Tast.ty

(** Return the info of the given class from the typing heap. *)
val get_class :
  env -> Decl_provider.class_key -> Decl_provider.class_decl option

(** Return {true} when in the definition of a static property or method. *)
val is_static : env -> bool

(** Return {true} if the containing file was checked in strict mode. *)
val is_strict : env -> bool

(** Return the mode of the containing file *)
val get_mode : env -> FileInfo.mode

(** Return the {!TypecheckerOptions.t} with which this TAST was checked. *)
val get_tcopt : env -> TypecheckerOptions.t

(** Return the {!Provider_context.t} with which this TAST was checked. *)
val get_ctx : env -> Provider_context.t

val get_file : env -> Relative_path.t

(* Return the {!Relative_path.t} of the file the env is from *)

(** Expand a type variable ({!Typing_defs.Tvar}) to the type it refers to. *)
val expand_type : env -> Tast.ty -> env * Tast.ty

(** Eliminate type variables ({!Typing_defs.Tvar}) in the given type by
    recursively replacing them with the type they refer to. *)
val fully_expand : env -> Tast.ty -> Tast.ty

(** Given some class type or unresolved union of class types, return the
    identifiers of all classes the type may represent. *)
val get_class_ids : env -> Tast.ty -> string list

(** Strip away all Toptions that we possibly can in a type, expanding type
    variables along the way, turning ?T -> T. *)
val non_null : env -> Pos.t -> Tast.ty -> env * Tast.ty

(** Get the "as" constraints from an abstract type or generic parameter, or
    return the type itself if there is no "as" constraint. In the case of a
    generic parameter whose "as" constraint is another generic parameter, repeat
    the process until a type is reached that is not a generic parameter. Don't
    loop on cycles. (For example, function foo<Tu as Tv, Tv as Tu>(...)) *)
val get_concrete_supertypes : env -> Tast.ty -> env * Tast.ty list

(** Return {true} if the given {Decl_provider.class_decl} (referred to by the given
    {class_id_}, if provided) allows the current class (the one returned by
    {!get_self}) to access its members with the given {visibility}. *)
val is_visible :
  env ->
  Typing_defs.visibility * bool ->
  Nast.class_id_ option ->
  Decl_provider.class_decl ->
  bool

(** Assert that the types of values involved in a strict (non-)equality
    comparison are compatible; e.g., that the types are not statically
    known to be disjoint, in which case the comparison will always return
    true or false. *)
val assert_nontrivial :
  Pos.t -> Ast_defs.bop -> env -> Tast.ty -> Tast.ty -> unit

(** Assert that the type of a value involved in a strict (non-)equality
    comparsion to null is nullable (otherwise it is known to always
    return true or false). *)
val assert_nullable : Pos.t -> Ast_defs.bop -> env -> Tast.ty -> unit

(** Return the declaration-phase type the given hint represents. *)
val hint_to_ty : env -> Aast.hint -> Typing_defs.decl_ty

val localize :
  env -> Typing_defs.expand_env -> Typing_defs.decl_ty -> env * Tast.ty

(** Transforms a declaration phase type ({!Typing_defs.decl_ty})
    into a localized type ({!Typing_defs.locl_ty} = {!Tast.ty}).
    Performs no substitutions of generics and initializes the late static bound
    type ({!Typing_defs.Tthis}) to the current class type (the type returned by
    {!get_self}).

    This is mostly provided as legacy support for {!AutocompleteService}, and
    should not be considered a general mechanism for transforming a {decl_ty} to
    a {!Tast.ty}.

    {!quiet} silences certain errors because those errors have already fired
    and/or are not appropriate at the time we call localize.
    *)
val localize_with_self :
  env ->
  ?pos:Pos.t ->
  ?quiet:bool ->
  ?report_cycle:Pos.t * string ->
  Typing_defs.decl_ty ->
  env * Tast.ty

(** Get the upper bounds of the type parameter with the given name.
  FIXME: This function cannot return correct bounds at this time, because
  during TAST checks, the Next continuation in the typing environment (which stores
  information about type parameters) is gone.
 *)
val get_upper_bounds :
  env -> string -> Typing_defs.locl_ty list -> Type_parameter_env.tparam_bounds

(** Get the reification of the type parameter with the given name. *)
val get_reified : env -> string -> Aast.reify_kind

(** Get whether the type parameter supports testing with is/as. *)
val get_enforceable : env -> string -> bool

(** Indicates whether the type parameter with the given name is <<__Newable>>. *)
val get_newable : env -> string -> bool

(** Return whether the type parameter with the given name was implicity created
    as part of an `instanceof`, `is`, or `as` expression (instead of being
    explicitly declared in code by the user). *)
val is_fresh_generic_parameter : string -> bool

(** Assert that one type is a subtype of another, resolving unbound type
    variables in both types (if any), with {!env} reflecting the new state of
    these type variables. Produce an error if they cannot be subtypes. *)
val assert_subtype :
  Pos.t ->
  Typing_reason.ureason ->
  env ->
  Tast.ty ->
  Tast.ty ->
  Errors.typing_error_callback ->
  env

(** Return {true} when the first type is a subtype of the second type
    regardless of the values of unbound type variables in both types (if any). *)
val is_sub_type : env -> Tast.ty -> Tast.ty -> bool

(** Return {true} when the first type can be considered a subtype of the second
    type after resolving unbound type variables in both types (if any). *)
val can_subtype : env -> Tast.ty -> Tast.ty -> bool

(** Return {true} when the first type is a subtype of the second type. There is
    no type T such that for all T', T <: T' and T' <: T (which is the case for Tany
    and Terr in `can_subtype`) *)
val is_sub_type_for_union : env -> Tast.ty -> Tast.ty -> bool

(** Simplify unions in a type. *)
val simplify_unions : env -> Tast.ty -> env * Tast.ty

(** Union a list of types. *)
val union_list : env -> Typing_reason.t -> Tast.ty list -> env * Tast.ty

(** Returns (class_name, tconst_name, tconst_reference_position) for each type
    constant referenced in the type access path. *)
val referenced_typeconsts :
  env -> Aast.hint -> Aast.sid list -> (string * string * Pos.t) list

(** Return an {!env} for which {!is_static} will return {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)
val set_static : env -> env

(** Return an {!env} for which {!val_kind} is set to the second argument. *)
val set_val_kind : env -> Typing_defs.val_kind -> env

(** Returns the val_kind of the typing environment *)
val get_val_kind : env -> Typing_defs.val_kind

(** Returns an {!env} for which {!inside_constructor} is set to {true}.
    If you are using {!Tast_visitor}, you should have no need of this. *)
val set_inside_constructor : env -> env

(** Returns whether or not the typing environment is inside the
    constructor of a class *)
val get_inside_constructor : env -> bool

(** Returns a {!Decl_env.env} *)
val get_decl_env : env -> Decl_env.env

(** Construct an empty {!env}. Unlikely to be the best choice; prefer using
    {!Tast_visitor} or constructing an {!env} from a {!Tast.def}. *)
val empty : Provider_context.t -> env

(** Construct an {!env} from a toplevel definition. *)
val def_env : Provider_context.t -> Tast.def -> env

(** Construct an {!env} from a method definition and the {!env} of the context
    it appears in. *)
val restore_method_env : env -> Tast.method_ -> env

(** Construct an {!env} from a lambda definition and the {!env} of the context
    it appears in. *)
val restore_fun_env : env -> Tast.fun_ -> env

(** Construct an {!env} from a pocket universe definition and the {!env} of the context
    it appears in. *)
val restore_pu_enum_env : env -> Tast.pu_enum -> env

val typing_env_as_tast_env : Typing_env_types.env -> env

val tast_env_as_typing_env : env -> Typing_env_types.env

(** Verify that an XHP body expression is legal. *)
val is_xhp_child : env -> Pos.t -> Tast.ty -> bool

val get_enum : env -> string -> Decl_provider.class_decl option

val is_typedef : env -> string -> bool

val get_typedef : env -> string -> Decl_provider.typedef_decl option

val is_enum : env -> string -> bool

val env_reactivity : env -> Typing_defs.reactivity

val function_is_mutable : env -> Tast.type_param_mutability option

val local_is_mutable : include_borrowed:bool -> env -> Local_id.t -> bool

val get_env_mutability : env -> Typing_mutability_env.mutability_env

val get_fun : env -> Decl_provider.fun_key -> Decl_provider.fun_decl option

val set_env_reactive : env -> Typing_defs.reactivity -> env

val set_allow_wildcards : env -> env

val get_allow_wildcards : env -> bool

val condition_type_matches : is_self:bool -> env -> Tast.ty -> Tast.ty -> bool

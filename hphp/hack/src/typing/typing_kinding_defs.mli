open Typing_defs
module TySet = Typing_set

type tparam_bounds = TySet.t

(** The main representation of kinds in Hack. A record r of OCaml type kind can both represent
  fully-applied Hack types (e.g., of kind * ) and higher-kinded types.
  If r.parameters = [], we have a fully applied type.
  Otherwise, if, for example r.parameters = [(n1, r1); (n2;r2)], then
  r represents a type constructor with two (curried) arguments
  (i.e., r stands for kind * -> * -> * ).
  Here, n1 and n2 are the names of those parameters. The names are then bound
  and can be used in the involved constraints (unless the name is the wildcarf name _ ).
  In the example above, this means that n1 and n2 are bound in the constraints of r1 and r2, as well as
  r.upper_bounds and r.lower_bounds.
  *)
type kind = {
  lower_bounds: tparam_bounds;
  upper_bounds: tparam_bounds;
  reified: Aast.reify_kind;
      (** = Reified if generic parameter is marked `reify`, = Erased otherwise *)
  enforceable: bool;
      (** Set if generic parameter has attribute <<__Enforceable>> *)
  newable: bool;  (** Set if generic parameter has attribute <<__Newable>> *)
  require_dynamic: bool;
      (** Set if class is marked <<__SupportDynamicType>> and
          generic parameter does *not* have attribute <<__NoRequireDynamic>> *)
  parameters: named_kind list;
}

and named_kind = pos_id * kind [@@deriving hash, show]

(** This can be used in situations where we don't have a name for a type
  parameter at hand. All error functions must be aware of this and not print the dummy name. *)
val dummy_name : pos_id

(** Simple kinds are used in situations where we want to check well-kindedness, but
  ignore type constraints. Most importantly, this is used in Typing_phase.
  In other words, simple kinds can be seen as kinds that are just built from * and ->,
  whereas full kinds *additionally* also carry constraints.
  For technical reasons, simple kinds must, however, carry some degree of information about
  bounds: During localiazation, we may come across a type like T<_>, where the user provided
  a wildcard for the type argument and T is either a (higher-kinded) type parameter or a
  class/type name. In this case, we introduce a fresh type parameter in the environment, which
  is used as an abstract type standing in for the type argument. We then add those constraints
  to the fresh parameter that T imposes on its type parameter.
  To this end, we need to know the constraints T imposes on its parameters, while only having the
  kind of T at hand. To this end, simple kinds carry around constraints at the toplevel.
  However, the nested kinds (i.e., the kinds of the type parameters) always have empty constraints.

  Implementation note: Internally, simple and full kinds are represented similarly. Using two
  different types for them is mostly a matter of hygiene, in order to prevent mixing up the two
  sorts of kinds. Therefore, we keep the type of simple kinds abstract here.
  *)
module Simple : sig
  (* Gives us access to the non-simple kind after shadowing it below *)
  type full_kind = kind

  type kind

  type named_kind = pos_id * kind

  type bounds_for_wildcard =
    | NonLocalized of (Ast_defs.constraint_kind * decl_ty) list
    | Localized of {
        wc_lower: tparam_bounds;
        wc_upper: tparam_bounds;
      }

  (** Creates a general description of a kind, built from "Type" and "->". *)
  val string_of_kind : kind -> string

  (** Creates a human-readable representation of a kind, suitable for error messages. *)
  val description_of_kind : kind -> string

  val get_arity : kind -> int

  val fully_applied_type :
    ?reified:Aast.reify_kind ->
    ?enforceable:bool ->
    ?newable:bool ->
    unit ->
    kind

  val named_kind_of_decl_tparam : decl_tparam -> named_kind

  val named_kinds_of_decl_tparams : decl_tparam list -> named_kind list

  val get_wilcard_bounds : kind -> bounds_for_wildcard

  val to_full_kind_without_bounds : kind -> full_kind

  val type_with_params_to_simple_kind :
    ?reified:Aast.reify_kind ->
    ?enforceable:bool ->
    ?newable:bool ->
    decl_tparam list ->
    kind

  val get_named_parameter_kinds : kind -> named_kind list

  val from_full_kind : full_kind -> kind

  val with_dummy_name : kind -> named_kind
end

val force_lazy_values : kind -> kind

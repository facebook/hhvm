open Typing_defs
module TySet = Typing_set

type tparam_bounds = TySet.t

(** The kind of a type, used to collect information about type paramters. *)
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
  rank: int;
}

and named_kind = pos_id * kind [@@deriving hash, show]

(** This can be used in situations where we don't have a name for a type
  parameter at hand. All error functions must be aware of this and not print the dummy name. *)
val dummy_name : pos_id

(** Turns a decl_tparam into a kind, throwing away all bounds in the process *)
val kind_of_decl_tparam_no_bounds :
  ?rank:int -> ?require_dynamic:bool -> decl_tparam -> kind

val force_lazy_values : kind -> kind

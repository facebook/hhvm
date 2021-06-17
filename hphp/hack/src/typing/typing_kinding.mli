open Typing_defs
module KindDefs = Typing_kinding_defs

module Locl_Inst : sig
  val instantiate : locl_ty SMap.t -> locl_ty -> locl_ty
end

(** Simple well-kindedness checks do not take constraints into account. *)
module Simple : sig
  (** Check that the given type is a well-kinded, fully-applied type. In other
    words, check that the given decl_ty has kind *. Otherwise, reports errors.
    Check that classes mentioned in types are accessible from the current
    module, and accessible also from outside if in_signature=true. *)
  val check_well_kinded_type :
    in_signature:bool -> Typing_env_types.env -> decl_ty -> unit

  (** Check that the given type is a well-kinded type whose kind matches the provided one.
    Otherwise, reports errors.
    Check that classes mentioned in types are accessible from the current
    module, and accessible also from outside if in_signature=true. *)
  val check_well_kinded :
    in_signature:bool ->
    Typing_env_types.env ->
    decl_ty ->
    KindDefs.Simple.named_kind ->
    unit

  (** Runs check_well_kinded_type after translating hint to decl type *)
  val check_well_kinded_hint :
    in_signature:bool -> Typing_env_types.env -> Aast.hint -> unit

  val is_subkind :
    Typing_env_types.env ->
    sub:KindDefs.Simple.kind ->
    sup:KindDefs.Simple.kind ->
    bool
end

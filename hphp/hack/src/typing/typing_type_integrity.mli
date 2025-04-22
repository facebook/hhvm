open Typing_defs
module KindDefs = Typing_kinding_defs

module Locl_Inst : sig
  val instantiate : locl_ty SMap.t -> locl_ty -> locl_ty
end

(** Simple well-kindedness checks do not take constraints into account. *)
module Simple : sig
  (** Check that the given type is a well-kinded type whose kind matches the provided one.
    Otherwise, reports errors.

    Also check that classes mentioned in types are accessible from the current
    module, and accessible also from outside if in_signature=true.

    Finally, check that classes and type aliases mentioned in types are accessible
    from the current package. We however ignore package errors when
    `should_check_package_boundary` is `No. This is usually the case when
    we are inside the following (to model the behavior of SymbolRefs)
      - type consts
      - type hints
      - type params
      - type param constraints
  *)
  val check_well_kinded :
    in_signature:bool ->
    should_check_package_boundary:Typing_packages.check_reason ->
    Typing_env_types.env ->
    decl_ty ->
    KindDefs.Simple.named_kind ->
    unit

  (** Traverse a type and for each encountered type argument of a type X,
      check that it complies with the corresponding type parameter of X (arity and kind, but not constraints),
      fetched from the decl of X.

      Also check that classes mentioned in hints are accessible from the current
      module, and accessible also from outside if `in_signature` is true.

      Finally, check that classes and type aliases mentioned in types are accessible
      from the current package. We however ignore package errors when
      `should_check_package_boundary` is `No. This is usually the case when
      we are inside the following (to model the behavior of SymbolRefs)
        - type consts
        - type hints
        - type params
        - type param constraints
  *)
  val check_well_kinded_hint :
    in_signature:bool ->
    should_check_package_boundary:Typing_packages.check_reason ->
    Typing_env_types.env ->
    Aast.hint ->
    unit

  (** Traverse a type and for each encountered type argument of a type X,
  check that it complies with the corresponding type parameter of X (arity, kind, etc.),
  fetched from the decl of X.

  Also check that classes mentioned in hints are accessible from the current
  module, and accessible also from outside if `in_signature` is true.
  Package visibility checks are ignored. *)
  val check_well_kinded_context_hint :
    in_signature:bool -> Typing_env_types.env -> Aast.hint -> unit

  val is_subkind :
    Typing_env_types.env ->
    sub:KindDefs.Simple.kind ->
    sup:KindDefs.Simple.kind ->
    bool
end

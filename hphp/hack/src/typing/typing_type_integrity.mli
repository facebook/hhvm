open Typing_defs
module KindDefs = Typing_kinding_defs

module Locl_Inst : sig
  val instantiate : locl_ty SMap.t -> locl_ty -> locl_ty
end

(** Check that the given type is a well-formed. This includes recursively checking that whenever
    a paramterized type is used, we provide the right number of arguments.
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
val check_type_integrity :
  in_signature:bool ->
  should_check_package_boundary:Typing_packages.check_reason ->
  Typing_env_types.env ->
  decl_ty ->
  unit

(** Check that the given type hint is a well-formed. This includes recursively checking that
      whenever a paramterized type is used, we provide the right number of arguments.
      Otherwise, reports errors.

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
val check_hint_integrity :
  in_signature:bool ->
  should_check_package_boundary:Typing_packages.check_reason ->
  Typing_env_types.env ->
  Aast.hint ->
  unit

(** Shortcut for check_well_formed_hint with
    ~should_check_package_boundary:`No *)
val check_context_hint_integrity :
  in_signature:bool -> Typing_env_types.env -> Aast.hint -> unit

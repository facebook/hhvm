open Typing_defs
open Typing_unify_recursive

(* Initialize Typing_utils.sub_type_ref, which is referenced by
   Typing_unify.unify_unwrapped through Typing_utils.sub_type. *)
module Typing_subtype = Typing_subtype

let global_options =
  GlobalOptions.make
    ~tco_assume_php:true
    ~tco_safe_array:true
    ~tco_safe_vector_array:true
    ~po_deregister_php_stdlib:false
    ~po_use_full_fidelity:true
    ~tco_user_attrs:None
    ~tco_experimental_features:TypecheckerOptions.experimental_all
    ~tco_migration_flags:SSet.empty
    ~tco_dynamic_view:false
    ~tco_disallow_array_as_tuple:false
    ~po_auto_namespace_map:[]
    ~tco_disallow_ambiguous_lambda:false
    ~tco_disallow_array_typehint:false
    ~tco_disallow_array_literal:false
    ~tco_disallow_return_by_ref:false
    ~tco_disallow_array_cell_pass_by_ref:false
    ~tco_language_feature_logging:false
    ~tco_unsafe_rx:false
    ~tco_disallow_implicit_returns_in_non_void_functions:true
    ~ignored_fixme_codes:ISet.empty
    ~forward_compatibility_level:ForwardCompatibilityLevel.default

let empty_env = Typing_env.empty global_options Relative_path.default None

let tyl_equal tyl1 tyl2 =
  Typing_defs.tyl_compare
    (List.sort Typing_defs.ty_compare tyl1)
    (List.sort Typing_defs.ty_compare tyl2) = 0

let occurs_result_equal result1 result2 = match result1, result2 with
  | DoesOccurUnderOptions, DoesOccurUnderOptions
  | DoesOccur, DoesOccur
  | DoesNotOccur, DoesNotOccur
  | DoesOccurAtTop, DoesOccurAtTop -> true

  | DoesOccurUnderUnresolvedOptions tyl1,
    DoesOccurUnderUnresolvedOptions tyl2 -> tyl_equal tyl1 tyl2

  | DoesOccurUnderUnresolved tyl1,
    DoesOccurUnderUnresolved tyl2 -> tyl_equal tyl1 tyl2

  | _ -> false

let test_occurs env n ty ~expected = fun () ->
  occurs_result_equal expected (occursTop env n ty)

let test_occurs_under_options =
  let n = Ident.tmp () in
  let ty = (Reason.none, Toption (Reason.none, Toption (Reason.none, Tvar n))) in
  test_occurs empty_env n ty ~expected:DoesOccurUnderOptions

let test_occurs_under_unresolved_options =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tunresolved [
    Reason.none, Tprim Nast.Tint;
    Reason.none, Tprim Nast.Tstring;
    Reason.none, Toption (Reason.none, Tvar n);
  ]) in
  test_occurs empty_env n ty
    ~expected:(DoesOccurUnderUnresolvedOptions [
      Reason.none, Tprim Nast.Tint;
      Reason.none, Tprim Nast.Tstring;
    ])

let test_occurs_under_unresolved =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tunresolved [
    Reason.none, Tprim Nast.Tint;
    Reason.none, Tprim Nast.Tstring;
    Reason.none, Tvar n;
  ]) in
  test_occurs empty_env n ty
    ~expected:(DoesOccurUnderUnresolved [
      Reason.none, Tprim Nast.Tint;
      Reason.none, Tprim Nast.Tstring;
    ])

let test_occurs_at_top =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tvar n) in
  test_occurs empty_env n ty ~expected:DoesOccurAtTop

let test_occurs_as_generic_parameter =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tarraykind (AKvec (Reason.none, Tvar n))) in
  test_occurs empty_env n ty ~expected:DoesOccur

let test_does_not_occur =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tarraykind (AKvec (Reason.none, Tprim Nast.Tint))) in
  test_occurs empty_env n ty ~expected:DoesNotOccur

let test_occurs_under_singleton_unresolved =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tunresolved [Reason.none, Tvar n]) in
  test_occurs empty_env n ty ~expected:(DoesOccurUnderUnresolved [])

let test_occurs_under_option_and_singleton_unresolved =
  let n = Ident.tmp () in
  let ty = (Reason.none, Toption (Reason.none, Tunresolved [Reason.none, Tvar n])) in
  test_occurs empty_env n ty ~expected:DoesOccurUnderOptions

let test_occurs_under_singleton_unresolved_and_option =
  let n = Ident.tmp () in
  let ty = (Reason.none, Tunresolved [Reason.none, Toption (Reason.none, Tvar n)]) in
  test_occurs empty_env n ty ~expected:(DoesOccurUnderUnresolvedOptions [])

(* The unification of #1(int) and #2(#1(int)) should be (int) modulo
   intervening type variables. *)
let test_unify_tvars () =
  let env = empty_env in
  let n1 = Ident.tmp () in
  let n2 = Ident.tmp () in
  let env = Typing_env.add env n1
    (Reason.none, Tunresolved [Reason.none, Tprim Nast.Tint]) in
  let env = Typing_env.add env n2
    (Reason.none, Tunresolved [Reason.none, Tvar n1]) in
  let env, ty = Typing_unify.unify env
    (Reason.none, Tvar n1) (Reason.none, Tvar n2) in
  Typing_defs.ty_equal
    (Typing_expand.fully_expand env ty)
    (Reason.none, Tunresolved [Reason.none, Tprim Nast.Tint])

let tests =
  [
    "test_occurs_under_options",
    test_occurs_under_options;
    "test_occurs_under_unresolved_options",
    test_occurs_under_unresolved_options;
    "test_occurs_under_unresolved",
    test_occurs_under_unresolved;
    "test_occurs_at_top",
    test_occurs_at_top;
    "test_occurs_as_generic_parameter",
    test_occurs_as_generic_parameter;
    "test_does_not_occur",
    test_does_not_occur;
    "test_occurs_under_singleton_unresolved",
    test_occurs_under_singleton_unresolved;
    "test_unify_tvars", test_unify_tvars;
  ]

let () = Unit_test.run_all tests

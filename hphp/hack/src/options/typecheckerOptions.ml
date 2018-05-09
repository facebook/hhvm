(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = GlobalOptions.t
let assume_php = GlobalOptions.tco_assume_php
let safe_array = GlobalOptions.tco_safe_array
let safe_vector_array = GlobalOptions.tco_safe_vector_array
let allowed_attribute = GlobalOptions.tco_allowed_attribute
let disallow_ambiguous_lambda = GlobalOptions.tco_disallow_ambiguous_lambda
let disallow_array_typehint = GlobalOptions.tco_disallow_array_typehint
let disallow_array_literal = GlobalOptions.tco_disallow_array_literal
let experimental_feature_enabled =
  GlobalOptions.tco_experimental_feature_enabled
let migration_flag_enabled =
  GlobalOptions.tco_migration_flag_enabled
let default = GlobalOptions.default
let make_permissive = GlobalOptions.make_permissive
let experimental_instanceof = GlobalOptions.tco_experimental_instanceof
let experimental_isarray = GlobalOptions.tco_experimental_isarray
let experimental_darray_and_varray =
  GlobalOptions.tco_experimental_darray_and_varray
let experimental_goto = GlobalOptions.tco_experimental_goto
let experimental_tconst_on_generics =
  GlobalOptions.tco_experimental_tconst_on_generics
let experimental_disable_shape_and_tuple_arrays =
  GlobalOptions.tco_experimental_disable_shape_and_tuple_arrays
let experimental_stronger_shape_idx_ret =
  GlobalOptions.tco_experimental_stronger_shape_idx_ret
let experimental_annotate_function_calls =
  GlobalOptions.tco_experimental_annotate_function_calls
let experimental_unresolved_fix =
  GlobalOptions.tco_experimental_unresolved_fix
let experimental_generics_arity =
  GlobalOptions.tco_experimental_generics_arity
let experimental_forbid_nullable_cast =
  GlobalOptions.tco_experimental_forbid_nullable_cast
let experimental_coroutines =
  GlobalOptions.tco_experimental_coroutines
let experimental_disallow_static_memoized =
  GlobalOptions.tco_experimental_disallow_static_memoized
let experimental_disable_optional_and_unknown_shape_fields =
  GlobalOptions.tco_experimental_disable_optional_and_unknown_shape_fields
let experimental_no_trait_reuse = GlobalOptions.tco_experimental_no_trait_reuse
let experimental_is_expression = GlobalOptions.tco_experimental_is_expression
let experimental_as_expression = GlobalOptions.tco_experimental_as_expression
let experimental_decl_override_require_hint = GlobalOptions.tco_decl_override_require_hint
let experimental_sealed_classes = GlobalOptions.tco_sealed_classes
let experimental_void_is_type_of_null =
  GlobalOptions.tco_experimental_void_is_type_of_null

let experimental_all = GlobalOptions.tco_experimental_all
let migration_flags_all = GlobalOptions.tco_migration_flags_all
let dynamic_view = GlobalOptions.tco_dynamic_view
let disallow_unsafe_comparisons = GlobalOptions.tco_disallow_unsafe_comparisons

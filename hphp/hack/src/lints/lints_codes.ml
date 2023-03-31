(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Codes = struct
  (* 5501 - 5541 reserved for FB. *)

  let deprecated = 5542

  (* 5543 - 5548 reserved. *)

  let include_use = 5549

  (* 5550 - 5561 reserved. *)

  let clone_use = 5562

  (* 5563 - 5567 reserved. *)

  let loop_variable_shadows_local_variable = 5568

  (* 5569 - 5574 reserved. *)

  let duplicate_key = 5575

  (* 5576 - 5580 reserved. *)

  let if_literal = 5581

  (* 5582 reserved. *)

  let await_in_loop = 5583

  (* 5584 - 5607 reserved. *)

  let non_equatable_comparison = 5607

  let invalid_contains_check = 5608

  let is_always_true = 5609

  let is_always_false = 5610

  let invalid_null_check = 5611

  let invalid_switch_case_value_type = 5614

  (* 5615 reserved. *)

  let missing_override_attribute = 5616

  let sketchy_null_check = 5618

  let invalid_truthiness_test = 5622

  let sketchy_truthiness_test = 5623

  let redundant_generic = 5624

  (* let deprecated_pocket_universes_reserved_syntax = 5625 *)

  let as_invalid_type = 5626

  let class_overrides_all_trait_methods = 5627

  let as_always_succeeds = 5628

  let as_always_fails = 5629

  let redundant_nonnull_assertion = 5630

  let bool_method_return_hint = 5631

  (* let deprecated_missing_via_label_attribute = 5632 *)

  let nullsafe_not_needed = 5633

  let duplicate_property_enum_init = 5634

  let duplicate_property = 5635

  let loose_unsafe_cast_lower_bound = 5636

  let loose_unsafe_cast_upper_bound = 5637

  let invalid_disjointness_check = 5638

  let inferred_variance = 5639 (* DEPRECATED *)

  let switch_nonexhaustive = 5640

  let bad_xhp_enum_attribute_value = 5641

  let unreachable_method_in_trait = 5642

  let comparing_booleans = 5643

  let pointless_booleans_expression = 5644

  let unconditional_recursion = 5645

  let branch_return_same_value = 5646

  let redundant_unsafe_cast = 5647

  let redundant_cast = 5648

  let internal_classname = 5649

  let async_lambda = 5650

  let awaitable_awaitable = 5651
end

(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  tco_assume_php : bool;
  tco_safe_array : bool;
  tco_safe_vector_array : bool;
  tco_user_attrs : SSet.t option;
  tco_experimental_features : SSet.t;
  tco_migration_flags : SSet.t;
  po_auto_namespace_map : (string * string) list;
  ignored_fixme_codes : ISet.t;
}

let tco_experimental_instanceof = "instanceof"

(* Whether darray and varray are enabled. *)
let tco_experimental_darray_and_varray = "darray_and_varray"

let tco_experimental_goto = "goto"

(* Whether Shapes::idx should allow accessing a field that does not exist in
  a partial shape and is not explicitly unset. *)
let tco_experimental_shape_idx_relaxed =
  "shape_idx_relaxed"

let tco_experimental_all =
 SSet.empty |> List.fold_right SSet.add
   [
     tco_experimental_instanceof;
     tco_experimental_darray_and_varray;
     tco_experimental_goto;
     tco_experimental_shape_idx_relaxed;
   ]

let tco_migration_flags_all =
  SSet.empty |> List.fold_right SSet.add
    [
      "array_cast"
    ]

let default = {
 tco_assume_php = true;
 tco_safe_array = false;
 tco_safe_vector_array = false;
 tco_user_attrs = None;
 (** Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
 tco_experimental_features = tco_experimental_all;
 tco_migration_flags = SSet.empty;
 po_auto_namespace_map = [];
 ignored_fixme_codes = ISet.empty;
}

(* Use this instead of default when you don't have access to a project
* .hhconfig and it's acceptable to omit some errors. IDE integration is one
* example, assuming the IDE also talks to hh_client to get the full list with
* higher latency.
*
* Use 'default' if it's fine to be potentially stricter than the rest of the
* project requires, eg hh_single_typecheck *)
let make_permissive tcopt =
  { tcopt with
    tco_assume_php = true;
    tco_user_attrs = None;
  }

let make ~tco_assume_php
         ~tco_safe_array
         ~tco_safe_vector_array
         ~tco_user_attrs
         ~tco_experimental_features
         ~tco_migration_flags
         ~po_auto_namespace_map
         ~ignored_fixme_codes = {
                   tco_assume_php;
                   tco_safe_array;
                   tco_safe_vector_array;
                   tco_user_attrs;
                   tco_experimental_features;
                   tco_migration_flags;
                   po_auto_namespace_map;
                   ignored_fixme_codes;
        }
let tco_assume_php t = t.tco_assume_php
let tco_safe_array t = t.tco_safe_array
let tco_safe_vector_array t = t.tco_safe_vector_array
let tco_user_attrs t = t.tco_user_attrs
let tco_allowed_attribute t name = match t.tco_user_attrs with
 | None -> true
 | Some attr_names -> SSet.mem name attr_names
let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features
let tco_migration_flag_enabled t s =
  SSet.mem s t.tco_migration_flags
let po_auto_namespace_map t = t.po_auto_namespace_map
let ignored_fixme_codes t = t.ignored_fixme_codes

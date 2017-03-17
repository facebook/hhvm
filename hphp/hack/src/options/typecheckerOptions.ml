(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = GlobalOptions.t
let assume_php = GlobalOptions.tco_assume_php
let unsafe_xhp = GlobalOptions.tco_unsafe_xhp
let safe_array = GlobalOptions.tco_safe_array
let safe_vector_array = GlobalOptions.tco_safe_vector_array
let user_attrs = GlobalOptions.tco_user_attrs
let allowed_attribute = GlobalOptions.tco_allowed_attribute
let experimental_feature_enabled =
  GlobalOptions.tco_experimental_feature_enabled
let default = GlobalOptions.default
let make_permissive = GlobalOptions.make_permissive
let experimental_instanceof = GlobalOptions.tco_experimental_instanceof
let experimental_optional_shape_field =
  GlobalOptions.tco_experimental_optional_shape_field
let experimental_all = GlobalOptions.tco_experimental_all

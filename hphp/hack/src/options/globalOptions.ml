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
 (* When we encounter an unknown class|function|constant name outside
  * of strict mode, is that an error? *)
 tco_assume_php : bool;
 (* For somewhat silly historical reasons having to do with the lack
  * of .hhi's for fairly core XHP classes, we unfortunately mark all
  * XHP classes as not having their members fully known at Facebook.
  *
  * We've also historically not typechecked attributes, and using string
  * literals for things that aren't strings (eg colspan="3") is common.
  *
  * If set to true, this option disables the new, stricter behavior. Please
  * don't use for new code, but it's useful for migrating existing XHP
  * codebases. *)
 tco_unsafe_xhp : bool;

 (* List of <<UserAttribute>> names expected in the codebase *)
 tco_user_attrs : SSet.t option;

 (* Set of experimental features, in lowercase. *)
 tco_experimental_features : SSet.t;

 (* Namespace aliasing map *)
 po_auto_namespace_map : (string * string) list;
}

let tco_experimental_dict = "dict"
let tco_experimental_instanceof = "instanceof"
let tco_experimental_all =
 List.fold_left
   (fun acc x -> SSet.add x acc) SSet.empty
   [tco_experimental_dict; tco_experimental_instanceof]

let default = {
 tco_assume_php = true;
 tco_unsafe_xhp = false;
 tco_user_attrs = None;
 (** Default all features for testing. Actual options are set by reading
  * from hhconfig, which defaults to empty. *)
 tco_experimental_features = tco_experimental_all;
 po_auto_namespace_map = [];
}

(* Use this instead of default when you don't have access to a project
* .hhconfig and it's acceptable to omit some errors. IDE integration is one
* example, assuming the IDE also talks to hh_client to get the full list with
* higher latency.
*
* Use 'default' if it's fine to be potentially stricter than the rest of the
* project requires, eg hh_single_typecheck *)
let permissive = { default with tco_unsafe_xhp = true; }
let make ~tco_assume_php
         ~tco_unsafe_xhp
         ~tco_user_attrs
         ~tco_experimental_features
         ~po_auto_namespace_map = {
                   tco_assume_php;
                   tco_unsafe_xhp;
                   tco_user_attrs;
                   tco_experimental_features;
                   po_auto_namespace_map;
        }
let tco_assume_php t = t.tco_assume_php
let tco_unsafe_xhp t = t.tco_unsafe_xhp
let tco_user_attrs t = t.tco_user_attrs
let tco_allowed_attribute t name = match t.tco_user_attrs with
 | None -> true
 | Some attr_names -> SSet.mem name attr_names
let tco_experimental_feature_enabled t s =
  SSet.mem s t.tco_experimental_features
let po_auto_namespace_map t = t.po_auto_namespace_map

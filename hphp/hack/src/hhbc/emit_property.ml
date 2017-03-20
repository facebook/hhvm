(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let from_ast cv_kind_list _type_hint (_, (_, cv_name), initial_value) =
  (* TODO: Deal with type hint *)
  (* TODO: Deal with initializer *)
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let property_name = Litstr.to_string @@ cv_name in
  let property_is_private = Core.List.mem cv_kind_list Ast.Private in
  let property_is_protected = Core.List.mem cv_kind_list Ast.Protected in
  let property_is_public = Core.List.mem cv_kind_list Ast.Public in
  let property_is_static = Core.List.mem cv_kind_list Ast.Static in
  let property_initial_value = match initial_value with
    | None -> None
    | Some expr -> Some (Hhbc_from_nast.literal_from_expr expr) in
  Hhas_property.make
    property_is_private
    property_is_protected
    property_is_public
    property_is_static
    property_name
    property_initial_value

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let is_typedef ctx x =
  match Naming_provider.get_type_kind ctx x with
  | Some Naming_types.TTypedef -> true
  | _ -> false

module Provider :
  Decl_enforceability.Provider
    with type t = Provider_context.t
     and type class_t = Shallow_decl_defs.shallow_class = struct
  type t = Provider_context.t

  type class_t = Shallow_decl_defs.shallow_class

  let get_tcopt = Provider_context.get_tcopt

  let get_class = Decl_provider_internals.get_shallow_class

  let get_class_or_typedef ctx x =
    if is_typedef ctx x then
      match
        Decl_provider_internals.get_typedef_without_pessimise ctx x
        |> Decl_entry.to_option
      with
      | None -> None
      | Some td -> Some (Decl_enforceability.TypedefResult td)
    else
      match get_class ctx x with
      | None -> None
      | Some cd -> Some (Decl_enforceability.ClassResult cd)
end

module ShallowContextAccess = Decl_enforceability.ShallowContextAccess (Provider)
include Decl_enforceability.Pessimize (ShallowContextAccess)

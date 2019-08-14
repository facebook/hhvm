(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open Oxidized_module

let output = ref Oxidized_module.empty

let with_output_context ~module_name f =
  State.with_module_name module_name (fun () ->
      output := Oxidized_module.empty;
      let () = f () in
      let oxidized_module = !output in
      output := Oxidized_module.empty;
      oxidized_module)

let add_extern_use ty =
  output := { !output with extern_uses = SSet.add !output.extern_uses ty }

let add_mod_use mod_name =
  output := { !output with uses = SSet.add !output.uses mod_name }

let add_glob_use mod_name =
  output := { !output with glob_uses = SSet.add !output.glob_uses mod_name }

let add_alias mod_name alias =
  output := { !output with aliases = (mod_name, alias) :: !output.aliases }

let add_include mod_name =
  output := { !output with includes = SSet.add !output.includes mod_name }

let add_ty_use qualified_ty ty =
  output := { !output with ty_uses = (qualified_ty, ty) :: !output.ty_uses }

let add_decl name decl =
  output := { !output with decls = (name, decl) :: !output.decls }

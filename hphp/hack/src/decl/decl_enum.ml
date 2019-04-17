(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Typing_defs

module SN = Naming_special_names

(* Figures out if a class needs to be treated like an enum and if so returns
 * Some(base, type, constraint), where base is the underlying type of the
 * enum, type is the actual type of enum elements, and constraint is
 * an optional subtyping constraint. For subclasses of Enum<T>, both
 * base and type these are T.
 * For first-class enums, we distinguish between these. *)
let enum_kind name enum get_ancestor =
  match enum with
    | None ->
      (match get_ancestor SN.FB.cEnum with
        | Some (_, (Tapply ((_, enum), [ty_exp]))) when enum = SN.FB.cEnum ->
          (* If the class is a subclass of UncheckedEnum, ignore it. *)
          if Option.is_some (get_ancestor SN.FB.cUncheckedEnum) then None
          else Some (ty_exp, ty_exp, None)
        | _ -> None)
    | Some enum ->
      Some (enum.te_base, (fst enum.te_base, Tapply (name, [])),
            enum.te_constraint)

(* If a class is an Enum, we give all of the constants in the class the type
 * of the Enum. We don't do this for Enum<mixed> and Enum<arraykey>, since
 * that could *lose* type information.
 *)
let rewrite_class name enum get_ancestor consts =
  match enum_kind name enum get_ancestor with
    | None
    | Some (_, (_, (Tmixed | Tprim Tarraykey)), _) -> consts
    | Some (_, ty, _) ->
    (* A special constant called "class" gets added, and we don't
     * want to rewrite its type. *)
    SMap.mapi (fun k c ->
               if k = SN.Members.mClass then c else {c with cc_type = ty})
      consts

(* Same as above, but for use when shallow_class_decl is enabled *)
let rewrite_class_consts enum_kind =
  Sequence.map ~f:begin fun (k, c as pair) ->
    match Lazy.force enum_kind with
    | None
    | Some (_, (_, (Tmixed | Tprim Tarraykey)), _) -> pair
    | Some (_, ty, _) ->
    (* A special constant called "class" gets added, and we don't
     * want to rewrite its type. *)
      if k = SN.Members.mClass then pair else k, {c with cc_type = ty}
  end

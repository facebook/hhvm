(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Module used to enforce that Enum subclasses are used reasonably.
 * Exports the Enum type as the type of all constants, checks that constants
 * have the proper type, and restricts what types can be used for enums.
 *)
(*****************************************************************************)
open Nast
open Utils
open Typing_defs

(* Checks if a class is a subclass of Enum<T> and, if so, returns Some(T)
 * If the class is a subclass of UncheckedEnum, ignore it. *)
let is_enum ancestors =
  match SMap.get "\\Enum" ancestors with
    | Some (_, (Tapply ((_, "\\Enum"), [ty_exp]))) ->
      if SMap.mem "\\UncheckedEnum" ancestors then None
      else Some ty_exp
    | _ -> None

(* Check that a type is something that can be used as an array index
 * (int or string), blowing through typedefs to do it. Takes a function
 * to call to report the error if it isn't. *)
let check_valid_array_key_type f_fail ~allow_any:allow_any env p t =
  let env, (r, t'), trail = Typing_tdef.force_expand_typedef env t in
  (match t' with
    | Tprim Tint | Tprim Tstring -> ()
    | Tany when allow_any -> ()
    | _ -> f_fail p (Reason.to_pos r) (Typing_print.error t') trail);
  env

let enum_check_const ty_exp env (_, (p, _), _) t =
  (* Constants need to be subtypes of the enum type *)
  let env = Typing_ops.sub_type p Reason.URenum env ty_exp t in
  (* Make sure the underlying type of the constant is an int
   * or a string. This matters because we need to only allow
   * int and string constants (since only they can be array
   * indexes). *)
  (* Need to allow Tany, since we might not have the types *)
  check_valid_array_key_type
    Errors.enum_constant_type_bad
    ~allow_any:true env p t

(* If a class is a subclass of Enum<T>, check that the types of all of
 * the constants are compatible with T.
 * Also make sure that T is either int, string, or mixed (or an
 * abstract type that is one of those under the hood), that all
 * constants are ints or strings when T is mixed, and that any type
 * hints are compatible with the type. *)
let enum_class_check env tc consts const_types =
  match is_enum tc.tc_ancestors with
    | Some ty_exp ->
        let env, (r, ty_exp'), trail =
          Typing_tdef.force_expand_typedef env ty_exp in
        (match ty_exp' with
          (* We disallow typedefs that point to mixed *)
          | Tmixed -> if snd ty_exp <> Tmixed then
              Errors.enum_type_typedef_mixed (Reason.to_pos r)
          | Tprim Tint | Tprim Tstring -> ()
          (* Don't tell anyone, but we allow type params too, since there are
           * Enum subclasses that need to do that *)
          | Tgeneric _ -> ()
          | _ -> Errors.enum_type_bad (Reason.to_pos r)
                   (Typing_print.error ty_exp') trail);

        List.fold_left2 (enum_check_const ty_exp) env consts const_types

    | None -> env

(* If a class is an Enum, we give all of the constants in the class
 * the type of the Enum. We don't do this for Enum<mixed>, since that
 * could *lose* type information.
 *)
let enum_class_decl_rewrite env ancestors consts =
  if Typing_env.is_decl env then consts else
    match is_enum ancestors with
      | None
      | Some (_, Tmixed) -> consts
      | Some ty ->
      (* A special constant called "class" gets added, and we don't
       * want to rewrite its type. *)
      SMap.mapi (function k -> function c ->
                 if k = "class" then c else {c with ce_type = ty})
        consts

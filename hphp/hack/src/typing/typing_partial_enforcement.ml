(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Cls = Decl_provider.Class
module Env = Typing_env
module MakeType = Typing_make_type

(* For a decl type ty, return the type ety that the runtime will enforce.
 * It's possible that ty </: ety because the runtime breaks through abstraction
 * boundaries. For example, for enum E : int as arraykey
 * we have get_enforced_type(E) = int, but E </: int.
 *
 * If ty=ety then we say that the type is "fully enforced"
 * Resolve type parameters wrt to class_def_opt, if present
 *)
let rec get_enforced_type env class_def_opt ty =
  let default () = MakeType.mixed Reason.Rnone in
  match get_node ty with
  (* An enum type is enforced at its underlying type *)
  | Tapply ((_, name), _) when Env.is_enum env name -> begin
    match Env.get_class env name with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      default ()
    | Decl_entry.Found tc ->
      (match Cls.enum_type tc with
      | None -> default ()
      | Some e -> get_enforced_type env None e.te_base)
  end
  | Tapply ((_pos, name), tyargs) -> begin
    match Env.get_class_or_typedef env name with
    | Decl_entry.Found (Env.ClassResult _class_info) ->
      (* Non-generic types are fully enforced *)
      if List.is_empty tyargs then
        ty
      (* Generic types are not enforced at their type arguments *)
      else
        default ()
    | Decl_entry.Found (Env.TypedefResult typedef_info) ->
      (* Enforcement "sees through" type aliases and newtype, but does not instantiate generic arguments *)
      (* The same is true of newtypes *)
      get_enforced_type env None typedef_info.td_type
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      default ()
  end
  | Tgeneric (name, []) -> begin
    match class_def_opt with
    | None -> default ()
    | Some cd ->
      let tparams = Cls.tparams cd in
      begin
        match
          List.find tparams ~f:(fun tp -> String.equal (snd tp.tp_name) name)
        with
        | None -> default ()
        | Some tp ->
          let bounds =
            List.filter_map tp.tp_constraints ~f:(fun (cstr, ty) ->
                match cstr with
                | Ast_defs.Constraint_as
                | Ast_defs.Constraint_eq -> begin
                  (* Do not follow bounds that are themselves generic parameters
                   * as HHVM doesn't enforce these *)
                  match get_node ty with
                  | Tgeneric _ -> None
                  | _ -> Some (get_enforced_type env class_def_opt ty)
                end
                | Ast_defs.Constraint_super -> None)
          in
          begin
            match bounds with
            | [] -> default ()
            | [t] -> t
            | ts -> MakeType.intersection Reason.Rnone ts
          end
      end
    (* This is enforced but only at its class, which we can't express
     * in Hack types. So we use the enclosing class as an approximation.
     *)
  end
  | Tthis -> begin
    match class_def_opt with
    | None -> default ()
    | Some cd ->
      mk
        ( Reason.Rnone,
          Tapply
            ( (Cls.pos cd, Cls.name cd),
              List.map (Cls.tparams cd) ~f:(fun _ ->
                  (* TODO akenn *)
                  mk (Reason.Rnone, Tunion [])) ) )
  end
  | Toption t ->
    let ety = get_enforced_type env class_def_opt t in
    MakeType.nullable Reason.Rnone ety
  | Tlike t -> get_enforced_type env class_def_opt t
  (* Special case for intersections, as these are used to describe partial enforcement *)
  | Tintersection tys ->
    let tys = List.map tys ~f:(get_enforced_type env class_def_opt) in
    MakeType.intersection (get_reason ty) tys
  | Tshape _
  | Ttuple _ ->
    MakeType.nonnull Reason.Rnone
  | Tprim _
  | Tnonnull ->
    ty
  | _ -> default ()

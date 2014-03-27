(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open Typing_defs
open Autocomplete

module Reason = Typing_reason
module Env = Typing_env

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)

let not_implemented _ = failwith "Function not implemented"

type expand_typedef = 
    SSet.t -> Env.env -> Reason.t -> string -> ty list -> Env.env * ty

let (expand_typedef_ref : expand_typedef ref) = ref not_implemented
let expand_typedef x = !expand_typedef_ref x

type unify = Env.env -> ty -> ty -> Env.env * ty
let (unify_ref: unify ref) = ref not_implemented
let unify x = !unify_ref x

type sub_type = Env.env -> ty -> ty -> Env.env
let (sub_type_ref: sub_type ref) = ref not_implemented
let sub_type x = !sub_type_ref x

(*****************************************************************************)
(* Returns true if a type is optional *)
(*****************************************************************************)

let rec is_option env ty =
  let _, ety = Env.expand_type env ty in
  match ety with
  | _, Toption _ -> true
  | _, Tunresolved tyl ->
      List.exists (is_option env) tyl
  | _ -> false

(*****************************************************************************)
(* Unification error *)
(*****************************************************************************)
let uerror r1 ty1 r2 ty2 =
  let p1 = Reason.to_pos r1 in
  let p2 = Reason.to_pos r2 in
  let ty1 = Typing_print.error ty1 in
  let ty2 = Typing_print.error ty2 in
  error_l [p1, "This is "^ ty1 ^ Reason.to_string r1;
           p2, "It is incompatible with " ^ ty2 ^
           Reason.to_string r2]

(*****************************************************************************)
(* Adding results to auto-completion  *)
(*****************************************************************************)

let add_auto_result env class_members =
  Autocomplete.auto_complete_result :=
    SMap.fold begin fun x class_elt acc ->
      let ty = class_elt.ce_type in
      let type_ = Typing_print.full env ty in
      let pos = Reason.to_pos (fst ty) in
      let sig_ = x^" "^type_ in
      SMap.add sig_ (Autocomplete.make_result x pos type_) acc
    end class_members SMap.empty

let handle_class_type completion_type c =
  match completion_type, c.Typing_defs.tc_kind with
  | Some Autocomplete.Acid, Ast.Cnormal
  | Some Autocomplete.Acid, Ast.Cabstract
  | Some Autocomplete.Acnew, Ast.Cnormal
  | Some Autocomplete.Actype, _ -> true
  | _ -> false

let should_complete_fun completion_type name =
  match (Typing_env.Funs.get name) with
  | Some _ when completion_type = (Some Autocomplete.Acid) -> true
  | _ -> false

let should_complete_class completion_type name =
  match (Typing_env.Classes.get name) with
  | Some c when handle_class_type completion_type c -> true
  | _ -> false

let is_argument_info_target p =
  match !argument_info_target with
  | None -> false
  | Some (line, char_pos) ->
      let start_line, start_col, end_col = Pos.info_pos p in
      start_line = line && start_col <= char_pos && char_pos - 1 <= end_col

let process_arg_info fun_args used_args env =
  if !argument_info_target <> None && !argument_info_expected = None then
    let _, result = List.fold_left begin fun (index, result) arg ->
      let result =
        if is_argument_info_target (fst arg) then Some index else result in
      index + 1, result
    end (0, None) used_args in
    if result <> None then (
      argument_info_expected := Some (List.map begin
          fun (x,y) -> x, Typing_print.full env y end fun_args);
      argument_info_position := result
    );
  ()
  
let process_static_find_ref cid mid =
  match cid with
  | Nast.CI c -> Find_refs.process_class_ref (fst c) (snd c) (Some (snd mid))
  | _ -> ()

(*****************************************************************************)
(* Adding an infered type *)
(*****************************************************************************)

(* Remember (when we care) the type found at a position *)
let save_infer env pos ty =
  match !infer_target with
  | None -> ()
  | Some (line, char_pos) ->
      let l, start, end_ = Pos.info_pos pos in
      if l = line && start <= char_pos && char_pos <= end_ && !infer_type = None
      then begin
        infer_type := Some (Typing_print.full env ty);
        infer_pos := Some (Reason.to_pos (fst ty));
      end
      else ()

(* Find the first defined position in a list of types *)
let rec find_pos tyl =
  match tyl with
  | [] -> Pos.none
  | (r, _) :: rl ->
      let p = Reason.to_pos r in
      if p = Pos.none
      then find_pos rl
      else p

(*****************************************************************************)
(* Applies a function to 2 shapes simultaneously, raises an error if
 * the second argument has less fields than the first.
 *)
(*****************************************************************************)

let apply_shape ~f env (r1, fdm1) (r2, fdm2) =
  SMap.fold begin fun name ty1 env ->
    match SMap.get name fdm2 with
    | None when is_option env ty1 -> env
    | None ->
        let pos1 = Reason.to_pos r1 in
        let pos2 = Reason.to_pos r2 in
        error_l [pos2, "The field '"^name^"' is missing";
                 pos1, "The field '"^name^"' is defined"]
    | Some ty2 ->
        f env ty1 ty2
  end fdm1 env

(*****************************************************************************)
(* Try to unify all the types in a intersection *)
(*****************************************************************************)

let rec member_inter env ty tyl acc =
  match tyl with
  | [] -> env, ty :: acc
  | x :: rl ->
      try
        let env, ty = unify env x ty in
        env, List.rev_append acc (ty :: rl)
      with Error _ ->
        member_inter env ty rl (x :: acc)

and normalize_inter env tyl1 tyl2 =
  match tyl1 with
  | [] -> env, tyl2
  | x :: rl ->
      let env, tyl2 = member_inter env x tyl2 [] in
      normalize_inter env rl tyl2

(*****************************************************************************)
(* *)
(*****************************************************************************)

let in_var env ty =
  let res = Env.fresh_type() in
  let env, res = unify env ty res in
  env, res

(*****************************************************************************)
(*****************************************************************************)

(* Try to unify all the types in a intersection *)
let fold_unresolved env ty =
  let env, ety = Env.expand_type env ty in
  match ety with
  | r, Tunresolved [] -> env, (r, Tany)
  | _, Tunresolved [x] -> env, x
  | _, Tunresolved (x :: rl) ->
      (try
        let env, acc =
          List.fold_left begin fun (env, acc) ty ->
            try unify env acc ty
            with Error _ ->
              raise Exit
          end (env, x) rl in
        env, acc
      with Exit ->
        env, ty
      )
  | _ -> env, ty

(*****************************************************************************)
(* *)
(*****************************************************************************)

let string_of_visibility = function
  | Vpublic  -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"

let unresolved env ty =
  let env, ety = Env.expand_type env ty in
  match ety with
  | _, Tunresolved _ -> in_var env ety
  | _ -> in_var env (fst ty, Tunresolved [ty])

(*****************************************************************************)
(* Function checking if an array is used as a tuple *)
(*****************************************************************************)

let is_array_as_tuple env ty =
  let env, ety = Env.expand_type env ty in
  let env, ty = fold_unresolved env ty in
  match ety with
  | r, Tunresolved [_, Tarray (_, Some elt_type, None)]
  | r, Tarray (_, Some elt_type, None) ->
      let env, normalized_elt_ty = Env.expand_type env elt_type in
      let env, normalized_elt_ty = fold_unresolved env normalized_elt_ty in
      (match normalized_elt_ty with
      | _, Tunresolved _ -> true
      | _ -> false
      )
  | _ -> false

(*****************************************************************************)
(* Retrieves the type of "self" *)
(*****************************************************************************)

let get_self_class env error =
  let self = Env.get_self env in
  match self with
  | _, Tapply ((_, self), _) ->
      (match snd (Env.get_class env self) with
      | None -> assert false
      | Some tc -> tc)
  | _ ->
      error()

(*****************************************************************************)
(* Adds a new field to all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec grow_shape pos lvalue field_name ty env shape =
  let _, shape = Env.expand_type env shape in
  match shape with
  | _, Tshape fields ->
      let fields = SMap.add field_name ty fields in
      let result = Reason.Rwitness pos, Tshape fields in
      env, result
  | _, Tunresolved tyl ->
      let env, tyl = lfold (grow_shape pos lvalue field_name ty) env tyl in
      let result = Reason.Rwitness pos, Tunresolved tyl in
      env, result
  | x ->
      env, x

(*****************************************************************************)
(* Keep the most restrictive visibility (private < protected < public).
 * This is useful when dealing with unresolved types.
 * When there are several candidates for a given visibility we need to be
 * conservative and consider the most restrictive one.
 *)
(*****************************************************************************)

let min_vis vis1 vis2 =
  match vis1, vis2 with
  | x, Vpublic | Vpublic, x -> x
  | Vprotected _, x | x, Vprotected _ -> x
  | Vprivate _ as vis, Vprivate _ -> vis

let min_vis_opt vis_opt1 vis_opt2 =
  match vis_opt1, vis_opt2 with
  | None, x | x, None -> x
  | Some (pos1, x), Some (pos2, y) ->
      let pos = if pos1 = Pos.none then pos2 else pos1 in
      Some (pos, min_vis x y)

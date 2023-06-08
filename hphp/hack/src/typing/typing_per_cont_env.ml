(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Common
module C = Typing_continuations
module CMap = C.Map

type per_cont_entry = {
  (* Local types per continuation. For example, the local types of the
   * break continuation correspond to the local types that there were at the
   * last encountered break in the current scope. These are kept to be merged
   * at the appropriate merge points. *)
  local_types: Typing_local_types.t;
  (* Fake members are used when we want member variables to be treated like
   * locals. We want to handle the following:
   * if($this->x) {
   *   ... $this->x ...
   * }
   * The trick consists in replacing $this->x with a "fake" local. So that
   * all the logic that normally applies to locals is applied in cases like
   * this. Hence the name: FakeMembers.
   * All the fake members are thrown away at the first call.
   * We keep the invalidated fake members for better error messages.
   *)
  fake_members: Typing_fake_members.t;
  (* Type parameter environment
   * Lower and upper bounds on generic type parameters and abstract types
   * For constraints of the form Tu <: Tv where both Tu and Tv are type
   * parameters, we store an upper bound for Tu and a lower bound for Tv.
   * Contrasting with tenv and subst, bounds are *assumptions* for type
   * inference, not conclusions.
   *)
  tpenv: Type_parameter_env.t;
}

type t = per_cont_entry C.Map.t

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)

let empty_entry =
  {
    local_types = Typing_local_types.empty;
    fake_members = Typing_fake_members.empty;
    tpenv = Type_parameter_env.empty;
  }

let initial_locals entry = CMap.add C.Next entry CMap.empty

let get_cont_option = CMap.find_opt

let all_continuations : t -> C.t list = C.Map.keys

(** Continuations used to typecheck the `finally` block. *)
let continuations_for_finally =
  [C.Break; C.Continue; C.Catch; C.Exit; C.Finally]

(* Update an entry if it exists *)
let update_cont_entry name m f =
  match CMap.find_opt name m with
  | None -> m
  | Some entry -> CMap.add name (f entry) m

let add_to_cont name key value m =
  match CMap.find_opt name m with
  | None -> m
  | Some cont ->
    let cont =
      { cont with local_types = Local_id.Map.add key value cont.local_types }
    in
    CMap.add name cont m

let remove_from_cont name key m =
  match CMap.find_opt name m with
  | None -> m
  | Some c ->
    CMap.add
      name
      { c with local_types = Local_id.Map.remove key c.local_types }
      m

let drop_cont = CMap.remove

let drop_conts conts map =
  List.fold ~f:(fun map cont -> drop_cont cont map) ~init:map conts

let replace_cont key valueopt map =
  match valueopt with
  | None -> drop_cont key map
  | Some value -> CMap.add key value map

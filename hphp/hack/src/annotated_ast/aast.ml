(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

let is_erased = function
  | Erased -> true
  | SoftReified
  | Reified ->
    false

let is_reified = function
  | Reified -> true
  | Erased
  | SoftReified ->
    false

let is_soft_reified = function
  | SoftReified -> true
  | Erased
  | Reified ->
    false

(* Splits the methods on a class into the constructor, statics, dynamics *)

(**
 * Methods, properties, and requirements are order dependent in bytecode
 * emission, which is observable in user code via `ReflectionClass`.
 *)
let split_methods c_methods =
  let (constr, statics, res) =
    List.fold_left
      (fun (constr, statics, rest) m ->
        if snd m.m_name = "__construct" then
          (Some m, statics, rest)
        else if m.m_static then
          (constr, m :: statics, rest)
        else
          (constr, statics, m :: rest))
      (None, [], [])
      c_methods
  in
  (constr, List.rev statics, List.rev res)

(* Splits class properties into statics, dynamics *)
let split_vars c_vars =
  let (statics, res) =
    List.fold_left
      (fun (statics, rest) v ->
        if v.cv_is_static then
          (v :: statics, rest)
        else
          (statics, v :: rest))
      ([], [])
      c_vars
  in
  (List.rev statics, List.rev res)

(* Splits `require`s into extends, implements, class *)
let split_reqs c_reqs =
  let (extends, implements, class_) =
    List.fold_left
      (fun (extends, implements, class_) (h, require_kind) ->
        match require_kind with
        | RequireExtends -> (h :: extends, implements, class_)
        | RequireImplements -> (extends, h :: implements, class_)
        | RequireClass -> (extends, implements, h :: class_))
      ([], [], [])
      c_reqs
  in
  (List.rev extends, List.rev implements, List.rev class_)

let partition_map_require_kind ~f trait_reqs =
  let rec partition req_extends req_implements req_class c_reqs =
    match c_reqs with
    | [] -> (List.rev req_extends, List.rev req_implements, List.rev req_class)
    | ((_, RequireExtends) as req) :: tl ->
      partition (f req :: req_extends) req_implements req_class tl
    | ((_, RequireImplements) as req) :: tl ->
      partition req_extends (f req :: req_implements) req_class tl
    | ((_, RequireClass) as req) :: tl ->
      partition req_extends req_implements (f req :: req_class) tl
  in
  partition [] [] [] trait_reqs

(* extract the hint from a type annotation *)
let hint_of_type_hint : 'ex. 'ex type_hint -> type_hint_ = snd

(* extract the type from a type annotation *)
let type_of_type_hint : 'ex. 'ex type_hint -> 'ex = fst

(* map a function over the second part of a type annotation *)
let type_hint_option_map ~f ta =
  let mapped_hint =
    match hint_of_type_hint ta with
    | Some hint -> Some (f hint)
    | None -> None
  in
  (type_of_type_hint ta, mapped_hint)

(* helper function to access the list of enums included by an enum *)
let enum_includes_map ?(default = []) ~f includes =
  match includes with
  | None -> default
  | Some includes -> f includes

let is_enum_class c = Ast_defs.is_c_enum_class c.c_kind

(* Combinators for folding / iterating over all of a switch statement *)
module GenCase : sig
  val map :
    f:(('ex, 'en) gen_case -> 'a) ->
    ('ex, 'en) case list ->
    ('ex, 'en) default_case option ->
    'a list

  val fold_left :
    init:'state ->
    f:('state -> ('ex, 'en) gen_case -> 'state) ->
    ('ex, 'en) case list ->
    ('ex, 'en) default_case option ->
    'state

  val fold_right :
    init:'state ->
    f:(('ex, 'en) gen_case -> 'state -> 'state) ->
    ('ex, 'en) case list ->
    ('ex, 'en) default_case option ->
    'state

  val fold_left_mem :
    init:'state ->
    f:('state -> ('ex, 'en) gen_case -> 'state) ->
    ('ex, 'en) case list ->
    ('ex, 'en) default_case option ->
    'state list
end = struct
  let map ~f cases odfl =
    let rec doit cases =
      match cases with
      | [] -> Option.to_list (Option.map (fun x -> f (Default x)) odfl)
      | case :: cases -> f (Case case) :: doit cases
    in
    doit cases

  let fold_left ~init:state ~f cases odfl =
    let state =
      List.fold_left (fun state case -> f state (Case case)) state cases
    in
    let state =
      let some dfl = f state (Default dfl) in
      Option.fold ~none:state ~some odfl
    in
    state

  let fold_right ~init:state ~f cases odfl =
    let state =
      let some dfl = f (Default dfl) state in
      Option.fold ~none:state ~some odfl
    in
    let state =
      List.fold_right (fun case state -> f (Case case) state) cases state
    in
    state

  let fold_left_mem ~init ~f cases odfl =
    let f (state, acc) case =
      let state = f state case in
      (state, state :: acc)
    in
    snd (fold_left ~init:(init, []) ~f cases odfl)
end

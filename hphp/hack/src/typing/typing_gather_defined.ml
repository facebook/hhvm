(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This modules allows to traverse an AST and gather all local variables
that are defined by simple assignments like

  $x = ...

This is useful to gather variables that are defined in unsafe blocks
while not performing any typechecking.

We gather defined variables in a control-flow sensitive way:
variables that are only assigned in some code paths are ignored.
E.g. in the following if statement:

  if ($mybool) {
    $x = 1;
  } else {}

we consider that no variable is defined after the `if` since there is at least
one control-flow path (the second branch) where no variable is defined.

Similarly, there is no need to gather other created variables, like the ones
created in a `foreach` or a `catch`, because those won't survive outside
the block.
*)

open Core_kernel
open Nast

module C = Typing_continuations
module LEnv = Typing_lenv
module LEnvC = Typing_lenv_cont
module Reason = Typing_reason
module Utils = Typing_utils

module LocalIdsPerCont = struct
  type t = Typing_env.local_types

  let drop = C.Map.remove
  let drop_list contl m =
    List.fold contl ~init:m ~f:(fun m k -> drop k m)

  let add = C.Map.add

  let set k vopt = match vopt with
    | None -> drop k
    | Some v -> add k v

  let get = C.Map.find_opt

  let get_list contl m =
    List.map contl ~f:(fun cont -> get cont m)

  let empty = C.Map.empty

  let union env l1 l2 =
    let _env, locals = LEnvC.union_by_cont env LEnv.union l1 l2 in
    locals

  let union_list env ml =
    List.fold ml ~init:empty ~f:(union env)

  let union_cont_list env contl m =
    let lid_sets = get_list contl m in
    let union l1 l2 = let _env, l = LEnv.union_contextopts env l1 l2 in l in
    List.fold lid_sets ~init:None ~f:union

  let add_local env lid m =
    match get C.Next m with
    | None -> m
    | Some lid_set ->
      let tany = ((Reason.none, Utils.tany env), Ident.tmp ()) in
      add C.Next (Local_id.Map.add lid tany lid_set) m

end

module L = LocalIdsPerCont

class gatherer env = object (self) inherit [_] Nast.reduce as parent
  val mutable gamma = L.get C.Next (LEnv.get_all_locals env)

  method union = L.union env
  method union_list = L.union_list env
  method union_cont_list = L.union_cont_list env
  method add_local = L.add_local env

  method zero = L.set C.Next gamma L.empty

  method plus delta1 delta2 =
    let delta1 = L.drop C.Next delta1 in
    let delta = self#union delta1 delta2 in
    gamma <- L.get C.Next delta;
    delta

  method update_gamma delta =
    self#plus self#zero delta

  method! on_expr_ () e =
    let delta = self#update_gamma (parent#on_expr_ () e) in
    match e with
      | Obj_get _
      | Array_get _
      | Class_get _
      | Class_const _
      | Call _
      | Binop _
      | Unop _
      | New _
      | Cast _ ->
        self#might_throw delta
      | _ ->
        delta

  method! on_stmt () s =
    self#update_gamma (parent#on_stmt () s)

  method! on_Binop () bop e1 e2 =
    let delta = parent#on_Binop () bop e1 e2 in
    match bop with
    | Ast.Eq None ->
      let (_, e1) = e1 in
      begin match e1 with
      | Lvar (_, id) -> self#add_local id delta
      | _ -> delta
      end
    | Ast.Barbar | Ast.Ampamp -> self#on_expr () e1
    | _ -> delta

  method might_throw delta =
    L.set C.Catch gamma delta

  method! on_Break () =
    L.set C.Break gamma L.empty

  method! on_Continue () =
    L.set C.Continue gamma L.empty

  method! on_Throw () x e =
    self#plus
      (parent#on_Throw () x e)
      (L.set C.Catch gamma L.empty)

  method! on_Return () e =
    self#plus
      (parent#on_Return () e)
      (L.set C.Exit gamma L.empty)

  method! on_Yield () a =
    self#plus
      (parent#on_Yield () a)
      (L.set C.Exit gamma L.empty)

  method! on_Yield_from () e =
    self#plus
      (parent#on_Yield_from () e)
      (L.set C.Exit gamma L.empty)

  method on_branch b =
    let gamma_start = gamma in
    let delta = self#on_block () b in
    gamma <- gamma_start;
    delta

  method! on_If () e b1 b2 =
    match e with
    (* Matching on true and false here also allows to handle infinite loops
     * properly. *)
    | (_, True) -> self#on_block () b1
    | (_, False) -> self#on_block () b2
    | _ ->
      self#plus
       (self#on_expr () e)
       (self#union
         (self#on_branch b1)
         (self#on_branch b2))

  method! on_case () c =
    let b = match c with
      | Default b -> b
      | Case ((pos, _ as e), b) -> (pos, Expr e) :: b in
    self#on_branch b

  method! on_Switch () e cl =
    self#plus
      (self#on_expr () e)
      (self#union_list
        (List.map cl ~f:(self#on_case ())))

  method on_While_True b =
    let delta = self#on_block () b in
    let delta = L.set C.Next (L.get C.Break delta) delta in
    L.drop_list [C.Continue; C.Break] delta

  method! on_While () (p, _ as e) b =
    self#on_While_True ((p, If (e, [p, Break], [])) :: b)

  method! on_Do () b (p, _ as e) =
    self#on_While_True (
      b @ [
      p, If (e, [
        p, Break], [])])

  method! on_For () (p1, _ as e1) e2 (p3, _ as e3) b =
    self#on_block () (
      (p1, Expr e1) :: [
      Pos.none, While (e2,
        b @ [
        (p3, Expr e3)])])

  method! on_Foreach () e _ _ =
    (* if the iterable is empty, the block is not executed *)
    self#on_expr () e

  method! on_Try () b _cl fb =
    let delta = self#on_block () b in
    let delta = L.set C.Next (self#union_cont_list C.all delta) delta in
    let delta = L.drop C.Catch delta in
    (* The catch list might never be executed, so we ignore it *)
    self#plus delta (self#on_block () fb)

  method! on_fun_ () _ = self#on_Noop ()

end

let block env b =
  let gatherer = new gatherer env in
  gatherer#on_block () b

let expr env e =
  let gatherer = new gatherer env in
  gatherer#on_expr () e

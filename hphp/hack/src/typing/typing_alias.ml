(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module determining how long the longest "aliasing chain" of a given
 * block is.
 *
 * The problem:
 *   The type-inference algorithm needs to find a fix point when dealing with
 *   loops.
 *
 * Normally, a fix-point should be reached after 2 iterations. However,
 * there are pathological cases where this is not true.
 * It happens when the code is aliasing locals, because the type of locals
 * can change, a 'chain' of aliases can delay the moment where we reach a
 * fix point.
 *
 * Example:
*    // $x is an int
 *   $x = $y;
 *   $y = $z;
 *   $z = 'hello';
 *   // $x will only become an int after 3 iterations.
 *
 *)
(*****************************************************************************)

open Hh_prelude
open Common
open Aast
module Fake = Typing_fake_members

(*****************************************************************************)
(* Module computing all the aliased locals.
 *
 * if one writes '$y = $z', then the binding $y => $z is added to the map.
 * Note that object/class members are counted are counted as locals
 * (conservatively), because in some cases, they can behave like locals (cf
 * Typing_fake_members).
 *)
(*****************************************************************************)

module Dep = struct
  let add x1 x2 acc =
    let x2 = Local_id.to_string x2 in
    let prev = Option.value ~default:[] (SMap.find_opt x1 acc) in
    SMap.add x1 (x2 :: prev) acc

  let get key acc =
    match SMap.find_opt key acc with
    | None -> []
    | Some kl -> kl

  let visitor local =
    (object
       inherit [string list SMap.t] Nast.Visitor_DEPRECATED.visitor as parent

       method! on_expr acc ((_, _, e_) as e) =
         match e_ with
         | Lvar (_, x) -> add local x acc
         | Obj_get (((_, _, (This | Lvar _)) as x), (_, _, Id (_, y)), _, _) ->
           add local (Fake.make_id x y) acc
         | Class_get ((_, _, x), CGstring (_, y), _) ->
           add local (Fake.make_static_id x y) acc
         | _ -> parent#on_expr acc e
    end
    [@alert "-deprecated"])

  let expr local acc e = (visitor local)#on_expr acc e
end

module AliasMap : sig
  type t = string list SMap.t

  val get : string -> t -> string list

  val make : Nast.stmt -> t
end = struct
  type t = string list SMap.t

  let get = Dep.get

  let local_to_string = function
    | Lvar (_, x) -> Some (Local_id.to_string x)
    | Obj_get (((_, _, (This | Lvar _)) as x), (_, _, Id (_, y)), _, _) ->
      Some (Local_id.to_string (Fake.make_id x y))
    | Class_get ((_, _, x), CGstring (_, y), _) ->
      Some (Local_id.to_string (Fake.make_static_id x y))
    | _ -> None

  let visitor =
    (object (this)
       inherit [string list SMap.t] Nast.Visitor_DEPRECATED.visitor as parent

       method! on_expr acc ((_, _, e_) as e) =
         match e_ with
         | Binop Aast.{ bop = Ast_defs.Eq _; lhs = (_, p, List el); rhs = x2 }
           ->
           List.fold_left
             ~f:
               begin
                 fun acc e ->
                   this#on_expr
                     acc
                     ( (),
                       p,
                       Binop Aast.{ bop = Ast_defs.Eq None; lhs = e; rhs = x2 }
                     )
               end
             ~init:acc
             el
         | Binop Aast.{ bop = Ast_defs.Eq _; lhs; rhs } ->
           this#on_assign acc lhs rhs
         | _ -> parent#on_expr acc e

       method on_assign acc (_, _, e1) e2 =
         Option.value_map
           (local_to_string e1)
           ~f:
             begin
               (fun s -> Dep.expr s acc e2)
             end
           ~default:acc

       method! on_efun acc _ = acc

       method! on_lfun acc _ _ = acc
    end
    [@alert "-deprecated"])

  let make st = visitor#on_stmt SMap.empty st
end

(*****************************************************************************)
(* Given an alias map, returns the length of the longest possible
 * 'aliasing chain'.
 *
 * Example:
 *   $x = $y;
 *   $y = $z;
 *   $z = 'hello';
 *
 * RESULT=3 because of the chain $x => $y => $z
 *)
(*****************************************************************************)

module Depth : sig
  val get : AliasMap.t -> int
end = struct
  let rec fold aliases =
    SMap.fold
      begin
        fun k _ (visited, current_max) ->
          let (visited, n) = key aliases visited k in
          (visited, max n current_max)
      end
      aliases
      (SMap.empty, 0)

  and key aliases visited k =
    if SMap.mem k visited then
      (visited, SMap.find k visited)
    else
      let visited = SMap.add k 0 visited in
      let kl = AliasMap.get k aliases in
      let (visited, depth_l) = List.map_env visited kl ~f:(key aliases) in
      let my_depth = 1 + List.fold_left ~f:max ~init:0 depth_l in
      (SMap.add k my_depth visited, my_depth)

  let get aliases = snd (fold aliases)
end

(*****************************************************************************)
(* Entry point. *)
(*****************************************************************************)

let get_depth st =
  let aliases = AliasMap.make st in
  let result = Depth.get aliases in
  (* Needs to be at least 2 because of back edges. *)
  max 2 result

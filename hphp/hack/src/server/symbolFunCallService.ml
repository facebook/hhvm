(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SN = Naming_special_names

open Hh_core
open ServerCommandTypes.Symbol_info_service

module Result_set = Set.Make(struct
  type t = ServerCommandTypes.Symbol_info_service.symbol_fun_call
  let compare a b =
    (* Descending order, since SymbolInfoService.format_result uses rev_append
       and will reverse our sorted result list. *)
    let r = Pos.compare b.pos a.pos in if r <> 0 then r else
    let r = compare b.name a.name   in if r <> 0 then r else
    let r = compare b.type_ a.type_ in if r <> 0 then r else
    compare b.caller a.caller
end)

let combine_name cur_class cur_caller =
  match cur_class, cur_caller with
  | _, None -> "" (* Top-level function call *)
  | None, Some f -> f
  | Some c, Some f -> c ^ "::" ^ f

let is_pseudofunction name =
  List.mem SN.PseudoFunctions.[empty; isset; unset;] name

class visitor = object (self)
  inherit [_] Tast_visitor.reduce as super

  method zero = Result_set.empty
  method plus = Result_set.union

  val mutable cur_caller = None;

  method fun_call env target_type name pos =
    if is_pseudofunction name then self#zero else
    let name = Utils.strip_ns name in
    if name = SN.SpecialFunctions.echo then self#zero else
    let cur_class = Tast_env.get_self_id env |> Option.map ~f:Utils.strip_ns in
    Result_set.singleton {
      name;
      type_ = target_type;
      pos = Pos.to_relative_string pos;
      caller = combine_name cur_class cur_caller;
    }

  method method_call env target_type class_name method_id =
    let pos, method_name = method_id in
    let method_fullname = combine_name (Some class_name) (Some method_name) in
    self#fun_call env target_type method_fullname pos

  method! on_fun_ env f =
    let name = snd f.Tast.f_name in
    let is_anon = name = ";anonymous" in
    if not is_anon then cur_caller <- Some (Utils.strip_ns name);
    let acc = super#on_fun_ env f in
    if not is_anon then cur_caller <- None;
    acc

  method! on_method_ env m =
    cur_caller <- Some (snd m.Tast.m_name);
    let acc = super#on_method_ env m in
    cur_caller <- None;
    acc

  method! on_expr env (((pos, ty), expr_) as expr) =
    let acc =
      match expr_ with
      | Tast.New _ ->
        let mid = (pos, SN.Members.__construct) in
        Tast_env.get_class_ids env ty
        |> List.map ~f:(fun cid -> self#method_call env Constructor cid mid)
        |> List.fold ~init:self#zero ~f:self#plus
      | Tast.Fun_id (pos, name) ->
        self#fun_call env Function name pos
      | Tast.Method_id (((_, ty), _), mid) ->
        Tast_env.get_class_ids env ty
        |> List.map ~f:(fun cid -> self#method_call env Method cid mid)
        |> List.fold ~init:self#zero ~f:self#plus
      | Tast.Smethod_id ((_, cid), mid)
      | Tast.Method_caller ((_, cid), mid) ->
        self#method_call env Method cid mid
      | _ -> self#zero
    in
    let special_fun_acc =
      let special_fun id = self#fun_call env Function id pos in
      let module SF = SN.SpecialFunctions in
      match expr_ with
      | Tast.Fun_id _        -> special_fun SF.fun_
      | Tast.Method_id _     -> special_fun SF.inst_meth
      | Tast.Smethod_id _    -> special_fun SF.class_meth
      | Tast.Method_caller _ -> special_fun SF.meth_caller
      | _ -> self#zero
    in
    let (+) = self#plus in
    special_fun_acc + acc + super#on_expr env expr

  method! on_Call env ct e hl el uel =
    let acc =
      match snd e with
      | Tast.Id (pos, name) ->
        self#fun_call env Function name pos
      | Tast.Class_const (((_, ty), _), mid)
      | Tast.Obj_get (((_, ty), _), (_, Tast.Id mid), _) ->
        let target_type =
          if snd mid = SN.Members.__construct then Constructor else Method in
        Tast_env.get_class_ids env ty
        |> List.map ~f:(fun cid -> self#method_call env target_type cid mid)
        |> List.fold ~init:self#zero ~f:self#plus
      | _ -> self#zero
    in
    self#plus acc (super#on_Call env ct e hl el uel)
end

let find_fun_calls tasts =
  List.concat_map tasts ~f:(fun x -> new visitor#go x |> Result_set.elements)

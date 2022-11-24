(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module JSON = Hh_json

type result = {
  mixed_count: int;
  nonnull_count: int;
  dynamic_count: int;
}

let json_of_results results =
  let json_of_result (id, result) =
    JSON.JSON_Object
      [
        ("id", JSON.string_ id);
        ("mixed_count", JSON.int_ result.mixed_count);
        ("dynamic_count", JSON.int_ result.dynamic_count);
        ("nonnull_count", JSON.int_ result.nonnull_count);
      ]
  in
  SMap.bindings results |> JSON.array_ json_of_result

let bad_type_visitor_per_def =
  object (self)
    inherit [_] Tast_visitor.reduce

    method zero = { mixed_count = 0; nonnull_count = 0; dynamic_count = 0 }

    method plus r1 r2 =
      {
        mixed_count = r1.mixed_count + r2.mixed_count;
        nonnull_count = r1.nonnull_count + r2.nonnull_count;
        dynamic_count = r1.dynamic_count + r2.dynamic_count;
      }

    method! on_'ex _ ty =
      match Typing_defs.get_node ty with
      | Typing_defs.Tintersection [] -> { (self#zero) with mixed_count = 1 }
      | Typing_defs.Toption ty
        when Typing_defs.equal_locl_ty_
               (Typing_defs.get_node ty)
               Typing_defs.Tnonnull ->
        { (self#zero) with mixed_count = 1 }
      | Typing_defs.Tdynamic -> { (self#zero) with dynamic_count = 1 }
      | Typing_defs.Tnonnull -> { (self#zero) with nonnull_count = 1 }
      | _ -> self#zero
  end

let bad_type_visitor =
  object
    inherit [_] Tast_visitor.reduce

    method zero = SMap.empty

    method plus =
      SMap.union ~combine:(fun id _ _ -> failwith ("Clash at %s" ^ id))

    method! on_fun_ env (Aast_defs.{ f_name = (_, id); _ } as fun_def) =
      let result = bad_type_visitor_per_def#on_fun_ env fun_def in
      SMap.singleton id result

    method! on_method_ env (Aast_defs.{ m_name = (_, mid); _ } as method_def) =
      let result = bad_type_visitor_per_def#on_method_ env method_def in
      let cid = Tast_env.get_self_id env |> Option.value_exn in
      let id = cid ^ "::" ^ mid in
      SMap.singleton id result
  end

let count ctx tast = bad_type_visitor#go ctx tast

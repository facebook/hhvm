(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let id_if_string expr : 'a option =
  let (pos, expr_) = expr in
  match expr_ with
  | String s -> Some (pos, s)
  | _ -> None

let check_duplicates (ids : Aast.sid list) : unit =
  let _ =
    List.fold ids ~init:SMap.empty ~f:(fun acc (pos, name) ->
        (match SMap.find_opt name acc with
        | Some prev_pos ->
          Errors.repeated_record_field
            name
            pos
            (Pos_or_decl.of_raw_pos prev_pos)
        | None -> ());
        SMap.add name pos acc)
  in
  ()

let handler =
  object
    inherit Nast_visitor.handler_base as super

    (* Ban duplicate fields in declarations: record Foo { x: int, x: int } *)
    method! at_record_def _env rd =
      let field_names = List.map rd.rd_fields ~f:(fun (id, _, _) -> id) in
      check_duplicates field_names

    (* Ban duplicate fields in instantiations: Foo['x' => 1, 'x' => 2]; *)
    method! at_expr env (pos, e) =
      match e with
      | Aast.Record (_, fields) ->
        let field_names =
          List.map fields ~f:(fun (id, _) -> id_if_string id) |> List.filter_opt
        in
        check_duplicates field_names
      | _ -> super#at_expr env (pos, e)
  end

(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* In order to run recheck_typing, workers need access to the File_info for each
 * file to be typechecked, so a File_info is paired with each query.
 *
 * Note that this means that many queries on the same file result in needlessly
 * marshalling and unmarshalling the same File_info many times over. There are
 * probably ways we could avoid this, but it doesn't seem to be a major problem.
 *)

module T = Aast
module SN = Naming_special_names

module Results = Stdlib.Set.Make (struct
  type t = Relative_path.t Symbol_occurrence.t

  let compare = Symbol_occurrence.compare Relative_path.compare
end)

let process_method_cid n cid =
  Results.singleton
    {
      Symbol_occurrence.name = cid ^ "::" ^ snd n;
      type_ = Symbol_occurrence.Method (Symbol_occurrence.ClassName cid, snd n);
      is_declaration = None;
      pos = fst n;
      affects_prod_build = true;
    }

let process_method env ty n =
  Tast_env.get_class_ids env ty
  |> List.map ~f:(process_method_cid n)
  |> List.fold ~init:Results.empty ~f:Results.union

let process_function id =
  Results.singleton
    {
      Symbol_occurrence.name = snd id;
      type_ = Symbol_occurrence.Function;
      is_declaration = None;
      pos = fst id;
      affects_prod_build = true;
    }

let process_local id =
  Results.singleton
    {
      Symbol_occurrence.name = snd id;
      type_ = Symbol_occurrence.LocalVar;
      is_declaration = None;
      pos = fst id;
      affects_prod_build = true;
    }

let collect_in_decl =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method zero = Results.empty

    method plus a b = Results.union a b

    method! on_Call env call =
      let ( + ) = self#plus in
      let T.{ func = (_, _, expr_); _ } = call in
      let acc =
        match expr_ with
        | T.Obj_get ((ty, _, _), (_, _, T.Id mid), _, _) ->
          process_method env ty mid
        | T.Id id -> process_function id
        | T.Class_const ((ty, _, _), mid) -> process_method env ty mid
        | T.Lvar (pos, id) -> process_local (pos, Local_id.get_name id)
        | _ -> self#zero
      in
      acc + super#on_Call env call

    method! on_New env ((ty, p, _) as c) targs el unpacked_element ctor_annot =
      let ( + ) = self#plus in
      let acc = process_method env ty (p, SN.Members.__construct) in
      acc + super#on_New env c targs el unpacked_element ctor_annot

    method! on_expr env ((_, _, expr_) as expr) =
      let ( + ) = self#plus in
      let acc =
        match expr_ with
        | T.Method_caller ((p, cid), mid) ->
          process_function (p, SN.AutoimportedFunctions.meth_caller)
          + process_method_cid mid cid
        | T.FunctionPointer (T.FP_id id, _targs, _) -> process_function id
        | T.FunctionPointer (T.FP_class_const ((ty, _, _cid), mid), _targs, _)
          ->
          process_method env ty mid
        | _ -> self#zero
      in
      acc + super#on_expr env expr
  end

let result_to_string result (fn, line, char) =
  let obj =
    `Assoc
      [
        ("position", Server_rx_api_shared.pos_to_json fn line char);
        (match result with
        | Ok (Some refs) ->
          ( "deps",
            let l =
              List.map refs ~f:(fun def_opt ->
                  match def_opt with
                  | None -> `Null
                  | Some def ->
                    let module SD = Symbol_definition in
                    let props =
                      [
                        ("name", `String (SD.full_name def));
                        ("kind", `String (SD.string_of_kind def.SD.kind));
                        ("position", Pos.json (Pos.to_absolute def.SD.pos));
                      ]
                    in
                    `Assoc props)
            in
            `List l )
        | Ok None -> ("error", `String "Function/method not found")
        | Error e -> ("error", `String e));
      ]
  in
  Yojson.Safe.to_string obj

let remove_duplicates_except_none ~compare l =
  let rec loop l accum =
    match l with
    | [] -> accum
    | [x] -> x :: accum
    | x1 :: x2 :: tl ->
      if Option.is_some x1 && compare x1 x2 = 0 then
        loop (x2 :: tl) accum
      else
        loop (x2 :: tl) (x1 :: accum)
  in
  List.rev (loop l [])

let handlers :
    ( Results.t,
      Relative_path.t Symbol_definition.t option list,
      Nast.program )
    Server_rx_api_shared.handlers =
  let compare =
    Option.compare (Symbol_definition.compare Relative_path.compare)
  in
  {
    Server_rx_api_shared.result_to_string;
    walker =
      {
        Server_rx_api_shared.plus = collect_in_decl#plus;
        on_method = collect_in_decl#on_method_;
        on_fun_def = collect_in_decl#on_fun_def;
      };
    get_state = (fun ctx fn -> Ast_provider.get_ast ~full:true ctx fn);
    map_result =
      (fun ctx ast refs ->
        let ast = Some ast in
        Results.elements refs
        |> List.map ~f:(Server_symbol_definition.go ctx ast)
        |> List.sort ~compare
        |> remove_duplicates_except_none ~compare);
  }

(* Entry Point *)
let go :
    Multi_worker.worker list option ->
    (string * int * int) list ->
    Server_env.env ->
    _ =
 fun workers pos_list env ->
  Server_rx_api_shared.go workers pos_list env handlers

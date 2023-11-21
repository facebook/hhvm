(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* In order to run recheck_typing, workers need access to the FileInfo for each
 * file to be typechecked, so a FileInfo is paired with each query.
 *
 * Note that this means that many queries on the same file result in needlessly
 * marshalling and unmarshalling the same FileInfo many times over. There are
 * probably ways we could avoid this, but it doesn't seem to be a major problem.
 *)

module T = Aast
module S = ServerRxApiShared
module SN = Naming_special_names
open SymbolOccurrence

module Results = Stdlib.Set.Make (struct
  type t = Relative_path.t SymbolOccurrence.t

  let compare = SymbolOccurrence.compare Relative_path.compare
end)

let process_method_cid n cid =
  Results.singleton
    {
      name = cid ^ "::" ^ snd n;
      type_ = Method (ClassName cid, snd n);
      is_declaration = false;
      pos = fst n;
    }

let process_method env ty n =
  Tast_env.get_class_ids env ty
  |> List.map ~f:(process_method_cid n)
  |> List.fold ~init:Results.empty ~f:Results.union

let process_function id =
  Results.singleton
    { name = snd id; type_ = Function; is_declaration = false; pos = fst id }

let process_local id =
  Results.singleton
    { name = snd id; type_ = LocalVar; is_declaration = false; pos = fst id }

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
        | T.FunctionPointer (T.FP_id id, _targs) -> process_function id
        | T.FunctionPointer (T.FP_class_const ((ty, _, _cid), mid), _targs) ->
          process_method env ty mid
        | _ -> self#zero
      in
      acc + super#on_expr env expr
  end

let result_to_string result (fn, line, char) =
  Hh_json.(
    let obj =
      JSON_Object
        [
          ("position", S.pos_to_json fn line char);
          (match result with
          | Ok (Some refs) ->
            ( "deps",
              let l =
                List.map refs ~f:(fun def_opt ->
                    match def_opt with
                    | None -> JSON_Null
                    | Some def ->
                      let module SD = SymbolDefinition in
                      let props =
                        [
                          ("name", JSON_String def.SD.full_name);
                          ("kind", JSON_String (SD.string_of_kind def.SD.kind));
                          ("position", Pos.json (Pos.to_absolute def.SD.pos));
                        ]
                      in
                      JSON_Object props)
              in
              JSON_Array l )
          | Ok None -> ("error", JSON_String "Function/method not found")
          | Error e -> ("error", JSON_String e));
        ]
    in
    json_to_string obj)

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

let handlers =
  let compare =
    Option.compare (SymbolDefinition.compare Relative_path.compare)
  in
  {
    S.result_to_string;
    S.walker =
      {
        S.plus = collect_in_decl#plus;
        S.on_method = collect_in_decl#on_method_;
        S.on_fun_def = collect_in_decl#on_fun_def;
      };
    S.get_state =
      begin
        (fun ctx fn -> Ast_provider.get_ast ~full:true ctx fn)
      end;
    S.map_result =
      begin
        fun ctx ast refs ->
          let ast = Some ast in
          Results.elements refs
          |> List.map ~f:(ServerSymbolDefinition.go ctx ast)
          |> List.sort ~compare
          |> remove_duplicates_except_none ~compare
      end;
  }

(* Entry Point *)
let go :
    MultiWorker.worker list option ->
    (string * int * int) list ->
    ServerEnv.env ->
    _ =
 (fun workers pos_list env -> ServerRxApiShared.go workers pos_list env handlers)

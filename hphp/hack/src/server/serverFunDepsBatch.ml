(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* In order to run recheck_typing, workers need access to the FileInfo for each
 * file to be typechecked, so a FileInfo is paired with each query.
 *
 * Note that this means that many queries on the same file result in needlessly
 * marshalling and unmarshalling the same FileInfo many times over. There are
 * probably ways we could avoid this, but it doesn't seem to be a major problem.
 *)
type pos = Relative_path.t * int * int
type pos_info = pos * FileInfo.t

module T = Tast
module SN = Naming_special_names

open SymbolOccurrence

let recheck_typing tcopt (pos_infos : pos_info list) =
  let files_to_check =
    pos_infos
    |> List.map ~f:(fun ((filename,_, _), file_info) -> filename, file_info)
    |> List.remove_consecutive_duplicates ~equal:(fun (a,_) (b,_) -> a = b)
  in
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  ServerIdeUtils.recheck tcopt files_to_check

module Results = Caml.Set.Make(struct
  type t = Relative_path.t SymbolOccurrence.t
  let compare = Pervasives.compare
end)

let process_method_cid n cid =
  Results.singleton {
    name = (cid ^ "::" ^ (snd n));
    type_ = Method (cid, snd n);
    is_declaration = false;
    pos = fst n
  }

let process_method env ty n =
  Tast_env.get_class_ids env ty
  |> List.map ~f:(process_method_cid n)
  |> List.fold ~init:Results.empty ~f:Results.union

let process_function id =
  Results.singleton {
    name  = snd id;
    type_ = Function;
    is_declaration = false;
    pos   = fst id
  }

let process_local id =
  Results.singleton {
    name  = snd id;
    type_ = LocalVar;
    is_declaration = false;
    pos   = fst id
  }

let collect_in_decl = object(self)
  inherit [_] Tast_visitor.reduce as super
  method zero = Results.empty
  method plus a b = Results.union a b
  method! on_Call env ct e h el uel =
    let (+) = self#plus in
    let acc =
      match snd e with
      | T.Obj_get (((_, ty), _), (_, T.Id mid), _) ->
        process_method env ty mid
      | T.Id id ->
        process_function id
      | T.Class_const (((_, ty), _), mid) ->
        process_method env ty mid
      | T.Lvar (pos, id) ->
        process_local (pos, Local_id.get_name id)
      | _ ->  self#zero in
    acc + (super#on_Call env ct e h el uel)

  method! on_New env (((p, ty), _) as c) el uel =
    let (+) = self#plus in
    let acc = process_method env ty (p, SN.Members.__construct) in
    acc + super#on_New env c el uel

  method! on_expr env expr =
    let (+) = self#plus in
    let acc =
      match snd expr with
      | T.Fun_id id ->
        process_function (fst (fst expr), "\\"^SN.SpecialFunctions.fun_) +
        process_function id
      | T.Smethod_id ((p, cid), mid) ->
        process_function (p, "\\"^SN.SpecialFunctions.class_meth) +
        process_method_cid mid cid
      | T.Method_caller ((p, cid), mid) ->
        process_function (p, "\\"^SN.SpecialFunctions.meth_caller) +
        process_method_cid mid cid
      | T.Method_id (((p, ty), _), mid) ->
        process_function (p, "\\"^SN.SpecialFunctions.inst_meth) +
        process_method env ty mid
      | _ -> self#zero in
    acc + (super#on_expr env expr)
end

let pos_contains_line_char pos line char =
  let l, start, end_ = Pos.info_pos pos in
  l = line && start <= char && char - 1 <= end_

let collect line char = object(self)
  inherit [_] Tast_visitor.reduce
  inherit [_] Ast.option_monoid

  method merge = collect_in_decl#plus

  method! on_method_ env m =
    if pos_contains_line_char (fst m.Tast.m_name) line char
    then Some (collect_in_decl#on_method_ env m)
    else self#zero

  method! on_fun_ env f =
    if pos_contains_line_char (fst f.Tast.f_name) line char
    then Some (collect_in_decl#on_fun_ env f)
    else self#zero
end

let result_to_string result (fn, line, char) =
  let open Hh_json in
  let pos_to_json fn line char =
    JSON_Object [
        "file", JSON_String (Relative_path.to_absolute fn);
        "line", int_ line;
        "character", int_ char
    ]
  in
  let obj = JSON_Object [
    "position", pos_to_json fn line char;
    match result with
    | Ok (Some refs) -> "deps",
      begin
      let l =
        List.map refs ~f:(fun def_opt ->
          match def_opt with
          | None -> JSON_Null
          | Some def ->
            let module SD = SymbolDefinition in
            let props = [
                "name", JSON_String def.SD.full_name;
                "kind", JSON_String (SD.string_of_kind def.SD.kind);
                "position", Pos.json (Pos.to_absolute def.SD.pos)
              ] in
            let props =
              if def.SD.reactivity_attributes <> []
              then begin
                let l =
                  List.map def.SD.reactivity_attributes
                    ~f:(fun s -> JSON_String (SD.string_of_reactivity_attribute s))
                in
                props @ ["reactivity", JSON_Array l]
              end
              else props in
            JSON_Object props) in
      JSON_Array l
      end
    | Ok None -> "error", JSON_String "Function/method not found"
    | Error e -> "error", JSON_String e
  ] in
  json_to_string obj

let prepare_pos_infos pos_list files_info =
  let pos_info_results =
    pos_list
    (* Sort, so that many queries on the same file will (generally) be
     * dispatched to the same worker. *)
    |> List.sort ~compare
    (* Dedup identical queries *)
    |> List.remove_consecutive_duplicates ~equal:(=)
    (* Get the FileInfo for each query *)
    |> List.map ~f:begin fun (fn, line, char) ->
      let fn = Relative_path.create_detect_prefix fn in
      let pos = (fn, line, char) in
      match Relative_path.Map.get files_info fn with
      | Some fileinfo -> Ok (pos, fileinfo)
      | None -> Error pos
    end
  in
  let pos_infos = List.filter_map pos_info_results ~f:Result.ok in
  let failure_msgs =
    pos_info_results
    |> List.filter_map ~f:Result.error
    |> List.map ~f:(result_to_string (Error "No such file or directory")) in
  pos_infos, failure_msgs

let remove_duplicates_except_none l=
  let rec loop l accum =
    match l with
    | [] -> accum
    | [x] -> x::accum
    | x1::x2::tl ->
        if x1 <> None && (x1 = x2)
        then loop (x2 :: tl) accum
        else loop (x2 :: tl) (x1 :: accum)
  in
  List.rev (loop l [])

let helper tcopt acc pos_infos =
  let tasts =
    List.fold (recheck_typing tcopt pos_infos)
      ~init:Relative_path.Map.empty
      ~f:(fun map (key, data) -> Relative_path.Map.add map ~key ~data)
  in
  List.fold pos_infos ~init:acc ~f:begin fun acc (pos, _) ->
    let fn, line, char = pos in
    let (ast, _) = Parser_heap.ParserHeap.find_unsafe fn in
    let result =
      Relative_path.Map.get tasts fn
      |> Result.of_option ~error:"No such file or directory"
      |> Result.map ~f:begin fun tast ->
        (collect line char)#go tast
        |> Option.map ~f:begin fun refs ->
          Results.elements refs
          |> List.map ~f:(ServerSymbolDefinition.go tcopt ast)
          |> List.sort ~compare
          |> remove_duplicates_except_none
        end
      end
    in
    result_to_string result pos :: acc
  end

let parallel_helper workers tcopt pos_infos =
  MultiWorker.call
    workers
    ~job:(helper tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers pos_infos)

(* Entry Point *)
let go:
  MultiWorker.worker list option ->
  (string * int * int) list ->
  ServerEnv.env ->
  _ =
fun workers pos_list env ->
  let {ServerEnv.tcopt; files_info; _} = env in
  let pos_infos, failure_msgs = prepare_pos_infos pos_list files_info in
  let results =
    if (List.length pos_infos) < 10
    then helper tcopt [] pos_infos
    else parallel_helper workers tcopt pos_infos
  in
  failure_msgs @ results

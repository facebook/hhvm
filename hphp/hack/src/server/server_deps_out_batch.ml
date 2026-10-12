open Hh_prelude
open Server_deps_util

let build_json_def def =
  `Assoc
    [
      ( "kind",
        `String (Symbol_definition.string_of_kind def.Symbol_definition.kind) );
      ("name", `String (Symbol_definition.full_name def));
      ( "position",
        Pos.to_absolute def.Symbol_definition.pos |> Pos.multiline_json );
    ]

let rec build_json_entry
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(total_occ_list : Relative_path.t Symbol_occurrence.t list)
    ~(get_def :
       Relative_path.t Symbol_occurrence.t ->
       Relative_path.t Symbol_definition.t option)
    (occ : Relative_path.t Symbol_occurrence.t) : Yojson.Safe.t =
  let open Symbol_occurrence in
  let def_opt = get_def occ in
  let depends_json =
    match def_opt with
    | None -> `String "None"
    | Some def ->
      if Option.is_none occ.is_declaration then
        build_json_def def
      else
        let body_list = body_symbols ~ctx ~entry total_occ_list occ def in
        `List
          (List.map
             body_list
             ~f:(build_json_entry ~ctx ~entry ~total_occ_list ~get_def))
  in
  `Assoc
    [
      ("kind", `String (kind_to_string occ.type_));
      ("name", `String occ.name);
      ( "declaration",
        match occ.is_declaration with
        | None -> `Null
        | Some p -> Pos.to_absolute p |> Pos.multiline_json );
      ("position", Pos.to_absolute occ.pos |> Pos.multiline_json);
      ("depends_on", depends_json);
    ]

let interesting_occ (occ : Relative_path.t Symbol_occurrence.t) : bool =
  let open Symbol_occurrence in
  match occ.type_ with
  | Keyword _
  | LocalVar
  | BuiltInType _
  | BestEffortArgument _ ->
    false
  | _ -> true

let go_json :
    Provider_context.t -> (string * int * int) list -> Yojson.Safe.t list =
 fun server_ctx pos_list ->
  let json_of_symbols acc_ctx_in (file, line, column) =
    let (acc_ctx_out, entry, _, get_def) = get_def_setup acc_ctx_in file in
    let total_occ_list =
      Identify_symbol_service.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:interesting_occ
    in
    let symbols = List.filter total_occ_list ~f:(is_target line column) in
    let json =
      `List
        (List.map
           symbols
           ~f:
             (build_json_entry ~ctx:acc_ctx_out ~entry ~total_occ_list ~get_def))
    in
    (acc_ctx_out, json)
  in

  let (_, json_list) =
    List.fold_map pos_list ~init:server_ctx ~f:json_of_symbols
  in
  json_list

let go (ctx : Provider_context.t) (pos_list : (string * int * int) list) :
    string list =
  let jsons = go_json ctx pos_list in
  List.map jsons ~f:Yojson.Safe.pretty_to_string

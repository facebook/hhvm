open Hh_prelude
open ServerDepsUtil

let build_json_def def =
  let open SymbolDefinition in
  let open Hh_json in
  Hh_json.JSON_Object
    [
      ("kind", string_ (SymbolDefinition.string_of_kind def.kind));
      ("name", string_ def.full_name);
      ("position", Pos.to_absolute def.pos |> Pos.multiline_json);
    ]

let rec build_json_entry
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(total_occ_list : Relative_path.t SymbolOccurrence.t list)
    ~(get_def :
       Relative_path.t SymbolOccurrence.t ->
       Relative_path.t SymbolDefinition.t option)
    (occ : Relative_path.t SymbolOccurrence.t) : Hh_json.json =
  let open SymbolOccurrence in
  let open Hh_json in
  let def_opt = get_def occ in
  let depends_json =
    match def_opt with
    | None -> string_ "None"
    | Some def ->
      if not occ.is_declaration then
        build_json_def def
      else
        let body_list = body_symbols ~ctx ~entry total_occ_list occ def in
        Hh_json.array_
          (build_json_entry ~ctx ~entry ~total_occ_list ~get_def)
          body_list
  in
  Hh_json.JSON_Object
    [
      ("kind", kind_to_string occ.type_ |> string_);
      ("name", string_ occ.name);
      ("declaration", bool_ occ.is_declaration);
      ("position", Pos.to_absolute occ.pos |> Pos.multiline_json);
      ("depends_on", depends_json);
    ]

let interesting_occ (occ : Relative_path.t SymbolOccurrence.t) : bool =
  let open SymbolOccurrence in
  match occ.type_ with
  | Keyword _
  | LocalVar
  | BuiltInType _
  | BestEffortArgument _ ->
    false
  | _ -> true

let go_json :
    Provider_context.t -> (string * int * int) list -> Hh_json.json list =
 fun server_ctx pos_list ->
  let json_of_symbols acc_ctx_in (file, line, column) =
    let (acc_ctx_out, entry, _, get_def) = get_def_setup acc_ctx_in file in
    let total_occ_list =
      IdentifySymbolService.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:interesting_occ
    in
    let symbols = List.filter total_occ_list ~f:(is_target line column) in
    let json =
      Hh_json.JSON_Array
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
  List.map jsons ~f:(Hh_json.json_to_string ~pretty:true)

open Hh_prelude

let is_target target_line target_char (occ : Relative_path.t SymbolOccurrence.t)
    =
  let open SymbolOccurrence in
  let pos = occ.pos in
  let (l, start, end_) = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let build_json_def def =
  let open SymbolDefinition in
  let open Hh_json in
  Hh_json.JSON_Object
    [
      ("kind", string_ (SymbolDefinition.string_of_kind def.kind));
      ("name", string_ def.full_name);
      ("position", Pos.to_absolute def.pos |> Pos.multiline_json);
    ]

let body_symbols
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(total_occ_list : Relative_path.t SymbolOccurrence.t list)
    (occ : Relative_path.t SymbolOccurrence.t)
    (def : Relative_path.t SymbolDefinition.t) :
    Relative_path.t SymbolOccurrence.t list =
  let open SymbolOccurrence in
  let open SymbolDefinition in
  let node_opt =
    ServerSymbolDefinition.get_definition_cst_node_ctx
      ~ctx
      ~entry
      ~kind:def.kind
      ~pos:def.pos
  in
  match node_opt with
  | None -> []
  | Some node ->
    let span_pos_opt =
      Full_fidelity_positioned_syntax.position (Pos.filename def.pos) node
    in
    (match span_pos_opt with
    | None -> []
    | Some span_pos ->
      let pos_filter (o : Relative_path.t SymbolOccurrence.t) =
        (not (phys_equal o occ)) && Pos.contains span_pos o.pos
      in
      List.filter total_occ_list ~f:pos_filter)

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
        let body_list = body_symbols ~ctx ~entry ~total_occ_list occ def in
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
    let path = Relative_path.create_detect_prefix file in
    let (acc_ctx_out, entry) =
      Provider_context.add_entry_if_missing ~ctx:acc_ctx_in ~path
    in
    let ast =
      Ast_provider.compute_ast
        ~popt:(Provider_context.get_popt acc_ctx_out)
        ~entry
    in
    let total_occ_list =
      IdentifySymbolService.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:interesting_occ
    in
    let symbols = List.filter total_occ_list ~f:(is_target line column) in
    let get_def = ServerSymbolDefinition.go acc_ctx_out (Some ast) in
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

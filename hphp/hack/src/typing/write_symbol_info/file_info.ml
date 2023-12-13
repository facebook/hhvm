(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Class = Typing_classes_heap.Api

type symbol = {
  occ: Relative_path.t SymbolOccurrence.t;
  def: Sym_def.t option;
}
[@@deriving show]

type container = {
  name: string;
  kind: Ast_defs.classish_kind;
}
[@@deriving show]

type member = { name: string } [@@deriving show]

type member_cluster = {
  container: container;
  methods: member list;
  properties: member list;
  class_constants: member list;
  type_constants: member list;
}
[@@deriving show]

type t = {
  path: string;
  cst: Full_fidelity_positioned_syntax.t;
  tast: (Tast.def * member_cluster list) list;
  source_text: Full_fidelity_source_text.t;
  symbols: symbol list;
  sym_hash: Md5.t option;
  fanout: bool;
}

let concat_hash str hash = Md5.digest_string (Md5.to_binary hash ^ str)

(* TODO we hash the string representation of the symbol types. We
   should move a more robust scheme and make sure this is enough to
   identify files which need reindexing *)
let compute_sym_hash symbols init =
  let f cur { occ; def } =
    let full_name =
      match def with
      | None -> ""
      | Some def ->
        Sym_def.(def.full_name ^ SymbolDefinition.string_of_kind def.kind)
    in
    let str = SymbolOccurrence.(occ.name ^ show_kind occ.type_ ^ full_name) in
    concat_hash str cur
  in
  List.fold ~init ~f symbols

let compute_member_clusters_hash hash member_clusters =
  let hash_container hash (c : container) =
    concat_hash c.name @@ concat_hash (Ast_defs.show_classish_kind c.kind) hash
  in
  let hash_member_cluster hash (mc : member_cluster) =
    let members =
      mc.methods @ mc.properties @ mc.class_constants @ mc.type_constants
    in
    let init = hash_container hash mc.container in
    List.fold members ~init ~f:(fun hash m -> concat_hash m.name hash)
  in
  List.fold member_clusters ~init:hash ~f:hash_member_cluster

(* We fetch the class for things that are already found in the decl store. So
   it should never be the case that the decl doesn't exist. *)
let get_class_exn ctx class_name =
  Decl_provider.get_class ctx class_name
  |> Decl_entry.to_option
  |> Option.value_or_thunk ~default:(fun () ->
         failwith
         @@ "Impossible happened. "
         ^ class_name
         ^ " does not have a decl")

(* Use the folded decl of a container definition (class/interface/trait) to
   collect inherited members of that definition. Particularly, we collect
   methods, properties, class constants, and type constants. *)
let collect_inherited_members ctx def =
  let empty_member_cluster container =
    {
      container;
      methods = [];
      properties = [];
      type_constants = [];
      class_constants = [];
    }
  in
  let update_member_cluster add ~name ~origin (mcs : member_cluster SMap.t) =
    let origin_decl = get_class_exn ctx origin in
    let kind = Class.kind origin_decl in
    let container = { name = origin; kind } in
    SMap.update
      container.name
      (function
        | None -> Some (add { name } (empty_member_cluster container))
        | Some mc -> Some (add { name } mc))
      mcs
  in
  (* All enums/enum classes inherit the same members which CodeHub currently
     doesn't index (and is not particularly interesting), so we're only
     interested in container definitions: classes, interfaces, and traits. *)
  let has_proper_inherited_members c =
    let open Ast_defs in
    match c.Aast_defs.c_kind with
    | Cclass _
    | Cinterface
    | Ctrait ->
      true
    | Cenum
    | Cenum_class _ ->
      false
  in
  let inherited_member_clusters = function
    | Aast_defs.Class class_ when has_proper_inherited_members class_ ->
      let class_name = snd class_.Aast_defs.c_name in
      let class_decl = get_class_exn ctx class_name in
      (* Collect members indexed by their origin *)
      let mcs = SMap.empty in
      let mcs =
        let add_method m mc = { mc with methods = m :: mc.methods } in
        let update = update_member_cluster add_method in
        Class.methods class_decl @ Class.smethods class_decl
        |> List.fold ~init:mcs ~f:(fun mcs (name, elt) ->
               let origin = elt.Typing_defs.ce_origin in
               (* Skip if not inherited *)
               if String.equal origin class_name then
                 mcs
               else
                 update ~name ~origin mcs)
      in
      let mcs =
        let add_property p mc = { mc with properties = p :: mc.properties } in
        let update = update_member_cluster add_property in
        Class.props class_decl @ Class.sprops class_decl
        |> List.fold ~init:mcs ~f:(fun mcs (name, elt) ->
               let origin = elt.Typing_defs.ce_origin in
               (* Skip if not inherited *)
               if String.equal origin class_name then
                 mcs
               else
                 update ~name ~origin mcs)
      in
      let (mcs, type_constants) =
        let add_type_constant tc mc =
          { mc with type_constants = tc :: mc.type_constants }
        in
        let update = update_member_cluster add_type_constant in
        Class.typeconsts class_decl
        |> List.fold ~init:(mcs, SSet.empty) ~f:(fun (mcs, tcs) (name, elt) ->
               let origin = elt.Typing_defs.ttc_origin in
               (* Skip if not inherited *)
               if String.equal origin class_name then
                 (mcs, tcs)
               else
                 let mcs = update ~name ~origin mcs in
                 let tcs = SSet.add name tcs in
                 (mcs, tcs))
      in
      let mcs =
        let add_class_constant cc mc =
          { mc with class_constants = cc :: mc.class_constants }
        in
        let update = update_member_cluster add_class_constant in
        Class.consts class_decl
        |> List.fold ~init:mcs ~f:(fun mcs (name, elt) ->
               let origin = elt.Typing_defs.cc_origin in
               (* Skip if
                  - not inherited
                  - `::class` which is a "synthesised" member that exists in
                     every class
                  - is a type constant (we already collected those separately
               *)
               if
                 String.equal origin class_name
                 || String.equal name Naming_special_names.Members.mClass
                 || SSet.mem name type_constants
               then
                 mcs
               else
                 update ~name ~origin mcs)
      in
      SMap.values mcs
    | _ -> []
  in
  let index_inherited_members =
    (Provider_context.get_tcopt ctx)
      .GlobalOptions.symbol_write_index_inherited_members
  in
  let inherited_member_clusters =
    if index_inherited_members then
      inherited_member_clusters def
    else
      []
  in
  (def, inherited_member_clusters)

let create
    ctx
    Indexable.{ path; fanout }
    ~gen_sym_hash
    ~gen_references
    ~root_path
    ~hhi_path =
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let path_str =
    Relative_path.to_absolute_with_prefix
      ~www:(Path.make root_path)
      ~hhi:(Path.make hhi_path)
      path
  in
  let source_text = Ast_provider.compute_source_text ~entry in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_unquarantined ~ctx ~entry
  in
  let tast = tast.Tast_with_dynamic.under_normal_assumptions in
  let cst =
    Provider_context.PositionedSyntaxTree.root
      (Ast_provider.compute_cst ~ctx ~entry)
  in
  let symbol_occs = IdentifySymbolService.all_symbols ctx tast in
  let symbols =
    List.map symbol_occs ~f:(fun occ ->
        { occ; def = Sym_def.resolve ctx occ ~sym_path:gen_references })
  in
  let tast = List.map tast ~f:(collect_inherited_members ctx) in
  let sym_hash =
    if gen_sym_hash then
      let member_clusterss = List.map ~f:snd tast in
      Some
        ( Md5.digest_string path_str |> compute_sym_hash symbols |> fun init ->
          List.fold ~init ~f:compute_member_clusters_hash member_clusterss )
    else
      None
  in
  { path = path_str; tast; source_text; cst; symbols; sym_hash; fanout }

let referenced t =
  let path sym =
    match sym.def with
    | Some Sym_def.{ path = Some path; _ }
      when Relative_path.(is_root (prefix path)) ->
      Some (Relative_path.to_absolute path)
    | _ -> None
  in
  List.filter_map t.symbols ~f:path |> SSet.of_list

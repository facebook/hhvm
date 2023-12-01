(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

(* NB: Must keep in sync with Rust type
   oxidized_by_ref::direct_decl_parser::Decls *)
type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

(* NB: Must keep in sync with Rust type
   oxidized_by_ref::direct_decl_parser::ParsedFile *)
type parsed_file = {
  pf_mode: FileInfo.mode option;
  pf_file_attributes: Typing_defs.user_attribute list;
  pf_decls: decls;
  pf_has_first_pass_parse_errors: bool;
}

(* NB: Must keep in sync with ToOcamlRep impl of Rust type rust_decl_ffi::OcamlParsedFileWithHashes *)
type parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: FileInfo.pfh_hash;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

external parse_decls :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file
  = "hh_parse_decls_ffi"

external parse_and_hash_decls :
  DeclParserOptions.t ->
  bool ->
  Relative_path.t ->
  string ->
  parsed_file_with_hashes = "hh_parse_and_hash_decls_ffi"

(* NB: Must be manually kept in sync with Rust function
   `hackrs_provider_backend::FileInfo::from::<ParsedFileWithHashes>` *)
let decls_to_fileinfo fn (parsed_file : parsed_file_with_hashes) =
  let file_mode = parsed_file.pfh_mode in
  let position_free_decl_hash = Some parsed_file.pfh_hash in
  let ids =
    List.fold
      parsed_file.pfh_decls
      ~init:FileInfo.empty_ids
      ~f:(fun acc (name, decl, hash) ->
        let pos p = FileInfo.Full (Pos_or_decl.fill_in_filename fn p) in
        match decl with
        | Shallow_decl_defs.Class c ->
          let info = (pos (fst c.Shallow_decl_defs.sc_name), name, Some hash) in
          FileInfo.{ acc with classes = info :: acc.classes }
        | Shallow_decl_defs.Fun f ->
          let info = (pos f.Typing_defs.fe_pos, name, Some hash) in
          FileInfo.{ acc with funs = info :: acc.funs }
        | Shallow_decl_defs.Typedef tf ->
          let info = (pos tf.Typing_defs.td_pos, name, Some hash) in
          FileInfo.{ acc with typedefs = info :: acc.typedefs }
        | Shallow_decl_defs.Const c ->
          let info = (pos c.Typing_defs.cd_pos, name, Some hash) in
          FileInfo.{ acc with consts = info :: acc.consts }
        | Shallow_decl_defs.Module m ->
          let info = (pos m.Typing_defs.mdt_pos, name, Some hash) in
          FileInfo.{ acc with modules = info :: acc.modules })
  in
  { FileInfo.ids; position_free_decl_hash; file_mode; comments = None }

let decls_to_addenda (parsed_file : parsed_file_with_hashes) :
    FileInfo.si_addendum list =
  (* NB: Must be manually kept in sync with Rust function `si_addenum::get_si_addenda` *)
  List.filter_map parsed_file.pfh_decls ~f:(fun (name, decl, _hash) ->
      let sia_name = Utils.strip_ns name in
      let sia_kind =
        match decl with
        | Shallow_decl_defs.(Class { sc_kind = Ast_defs.Cclass _; _ }) ->
          Some FileInfo.SI_Class
        | Shallow_decl_defs.(Class { sc_kind = Ast_defs.Cinterface; _ }) ->
          Some FileInfo.SI_Interface
        | Shallow_decl_defs.(Class { sc_kind = Ast_defs.Ctrait; _ }) ->
          Some FileInfo.SI_Trait
        | Shallow_decl_defs.(
            Class { sc_kind = Ast_defs.(Cenum | Cenum_class _); _ }) ->
          Some FileInfo.SI_Enum
        | Shallow_decl_defs.Fun _ -> Some FileInfo.SI_Function
        | Shallow_decl_defs.Typedef _ -> Some FileInfo.SI_Typedef
        | Shallow_decl_defs.Const _ -> Some FileInfo.SI_GlobalConstant
        | Shallow_decl_defs.Module _ -> None
      in
      let (sia_is_abstract, sia_is_final) =
        match decl with
        | Shallow_decl_defs.Class c ->
          (c.Shallow_decl_defs.sc_abstract, c.Shallow_decl_defs.sc_final)
        | _ -> (false, false)
      in
      match sia_kind with
      | None -> None
      | Some sia_kind ->
        Some { FileInfo.sia_name; sia_kind; sia_is_abstract; sia_is_final })

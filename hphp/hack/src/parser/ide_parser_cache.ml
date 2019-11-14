(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let is_active_ref : [ `Disabled | `Active | `Inactive ] ref = ref `Disabled

let enable () =
  Hh_logger.log "Enabling Ide_parser_cache";
  is_active_ref := `Inactive

let is_enabled () = !is_active_ref <> `Disabled

let activate () = if is_enabled () then is_active_ref := `Active

let deactivate () = if is_enabled () then is_active_ref := `Inactive

let with_ide_cache f =
  Utils.try_finally
    ~f:
      begin
        fun () ->
        activate ();
        f ()
      end
    ~finally:deactivate

module IdeAstCache =
  SharedMem.FreqCache
    (StringKey)
    (struct
      type value = Parser_return.t * Fixme_provider.fixme_map * Errors.t

      let capacity = 10
    end)

module IdeCstCache =
  SharedMem.FreqCache
    (StringKey)
    (struct
      type value = SyntaxTree.t

      let capacity = 10
    end)

let get_digest path content =
  let string_path = Relative_path.to_absolute path in
  Md5.(digest_string (string_path ^ content) |> to_binary)

let get_cst source_text =
  assert (!is_active_ref = `Active);
  let digest =
    get_digest
      (Full_fidelity_source_text.file_path source_text)
      (Full_fidelity_source_text.text source_text)
  in
  match IdeCstCache.get digest with
  | Some cst -> cst
  | None ->
    let env = Full_fidelity_parser_env.default in
    let cst = SyntaxTree.make ~env source_text in
    IdeCstCache.add digest cst;
    cst

let get_ast tcopt path content =
  assert (!is_active_ref = `Active);
  let digest = get_digest path content in
  match IdeAstCache.get digest with
  | Some (ast, fixmes, errors) ->
    Errors.merge_into_current errors;
    Fixme_provider.provide_hh_fixmes path fixmes;
    ast
  | None ->
    let (errors, ast) =
      Errors.do_ @@ fun () ->
      Full_fidelity_ast.defensive_program
        ~fail_open:true
        ~keep_errors:true
        ~include_line_comments:true
        ~quick:false
        tcopt
        path
        content
    in
    Errors.merge_into_current errors;
    let fixmes =
      match Fixme_provider.get_hh_fixmes path with
      | Some fixmes -> fixmes
      | None ->
        failwith
          ("HH_FIXMEs not found for path " ^ Relative_path.to_absolute path)
    in
    IdeAstCache.add digest (ast, fixmes, errors);
    ast

let get_ast_if_active tcopt path content =
  if !is_active_ref <> `Active then
    None
  else
    Some (get_ast tcopt path content)

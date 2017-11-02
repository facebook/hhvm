(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*  Quick FileInfo parser that parses only the basics *)
open Hh_core
open FileInfo

open Lexer_hack

module L = Lexer_hack

(*****************************************************************************)
(* Environment *)
(*****************************************************************************)

type env = {
  file      : Relative_path.t;
  mode      : FileInfo.mode;
  priority  : int;
  lb        : Lexing.lexbuf;
  errors    : (Pos.t * string) list ref;
  popt      : ParserOptions.t;
  ns_env    : Namespace_env.env
}

let init_env file lb popt = {
  file     = file;
  mode     = FileInfo.Mpartial;
  priority = 0;
  lb       = lb;
  errors   = ref [];
  popt     = popt;
  ns_env   = Namespace_env.empty popt;
}

type parser_return = FileInfo.t

(*****************************************************************************)
(* Errors *)
(*****************************************************************************)

let error_at env pos msg =
  env.errors := (pos, msg) :: !(env.errors)

let error env msg =
  error_at env (Pos.make env.file env.lb) msg

let error_back env msg =
  let pos = Pos.make env.file env.lb in
  L.back env.lb;
  error_at env pos msg

let error_expect env expect =
  error_back env ("Expected "^expect)

let expect env x =
  if L.token env.file env.lb = x
  then ()
  else error_expect env (L.token_to_string x)

let expect_word env name =
  let tok = L.token env.file env.lb in
  let value = Lexing.lexeme env.lb in
  if tok <> Tword || value <> name
  then error_expect env ("'"^name^ "' (not '"^value^"')");
  ()

let identifier env =
  match L.token env.file env.lb with
  | Tword ->
      let pos = Pos.make env.file env.lb in
      let name = Lexing.lexeme env.lb in
      pos, name
  | _ ->
      error_expect env "identifier";
      Pos.make env.file env.lb, "*Unknown*"


let make_fileInfo mode : FileInfo.t =
  {
    FileInfo.empty_t with
    file_mode = Some mode;
  }

let rec program popt file content =
  let lb = Lexing.from_string content in
  let env = init_env file lb popt in
  let result = header env in
  Option.iter (List.last !(env.errors)) Errors.parsing_error;
  result

and header env =
  let file_type, head = get_header env in
  match file_type, head with
  | FileInfo.PhpFile, _ ->
      FileInfo.empty_t
  | _, Some mode ->
      let result = toplevel (make_fileInfo mode) { env with mode = mode } in
      expect env Teof;
      result
  | _ ->
      FileInfo.empty_t


and get_header env =
  match L.header env.file env.lb with
  | `error -> FileInfo.HhFile, None
  | `default_mode -> FileInfo.HhFile, Some FileInfo.Mpartial
  | `php_decl_mode -> FileInfo.PhpFile, Some FileInfo.Mdecl
  | `php_mode -> FileInfo.PhpFile, None
  | `explicit_mode ->
      let _token = L.token env.file env.lb in
      (match Lexing.lexeme env.lb with
      | "strict" when !(Ide.is_ide_mode) ->
          FileInfo.HhFile, Some FileInfo.Mpartial
      | "strict" -> FileInfo.HhFile, Some FileInfo.Mstrict
      | ("decl"|"only-headers") -> FileInfo.HhFile, Some FileInfo.Mdecl
      | "partial" -> FileInfo.HhFile, Some FileInfo.Mpartial
      | _ ->
          error env
 "Incorrect comment; possible values include strict, decl, partial or empty";
          FileInfo.HhFile, Some FileInfo.Mdecl
      )

and toplevel acc env =
  match L.token env.file env.lb with
    | Teof ->
      acc
    | Tlcb ->
        ignore_until_match env (fun x -> x = Trcb);

        toplevel acc env
    | Tlp ->
        ignore_until_match env (fun x -> x = Trp);
        toplevel acc env
    | Tdquote
    | Tquote as quote ->
        ignore_until_match env (fun x -> x = quote);
        toplevel acc env
    | Tword ->
      (match Lexing.lexeme env.lb with
      | "class" ->
          let cname = identifier env in
          let elaborated =
            Namespaces.elaborate_id
              (env.ns_env)
              Namespaces.ElaborateClass cname in
          let newacc = {
            acc with
              classes = (FileInfo.pos_full elaborated)::acc.classes
          } in
          toplevel newacc env
      | "function" ->
          let fname = identifier env in
          let elaborated =
            Namespaces.elaborate_id
              (env.ns_env)
              Namespaces.ElaborateFun fname in
          let newacc = {
            acc with
              funs = (FileInfo.pos_full elaborated)::acc.funs
          } in
          toplevel newacc env

      | _ -> toplevel acc env)
    | _ -> toplevel acc env

and ignore_until_match env terminate =
  match L.token env.file env.lb with
    | x when terminate x ||  x = Teof ->
      ()
    | Tlcb ->
      ignore_until_match env (fun x -> x = Trcb);
      ignore_until_match env terminate
    | Tdquote
    | Tquote as quote->
      ignore_until_match env (fun x -> x = quote);
      ignore_until_match env terminate
    | _ ->
      ignore_until_match env terminate



let from_file popt file =
  let content =
    try Sys_utils.cat (Relative_path.to_absolute file) with _ -> "" in
  program popt file content

let from_file_with_default_popt file =
  from_file ParserOptions.default file

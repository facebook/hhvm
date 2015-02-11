(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module A = Ast_php
module C = Common
module CT = Convert_ty
module DP = Docblock_parse
module HT = Hack_ty
module P = Parser_php
module PI = Parse_info
module TH = Token_helpers_php
module V = Visitor_php

let err errl func s =
  let tok = match func.A.f_name with
    | A.Name (_, tok) -> tok
    | A.XhpName (_, tok) -> tok in
  let loc = PI.token_location_of_info tok in
  let err = Printf.sprintf "%s:%d:%d function/method %s %s"
    loc.PI.file
    loc.PI.line
    loc.PI.column
    loc.PI.str
    s
  in
  errl := err :: !errl;
  ()

let rev_take_until tok toks =
  let rec loop acc = function
    | [] -> assert false
    | x :: _ when tok = TH.info_of_tok x -> acc
    | x :: l -> loop (x::acc) l in
  loop [] toks

let rec find_comment = function
  | [] -> None
  | P.TSpaces _ :: l
  | P.TNewline _ :: l -> find_comment l
  | P.T_DOC_COMMENT c :: _ -> Some ((PI.token_location_of_info c).PI.str)
  | _ -> None

let update_param mode (changed, errl) comment_map func param =
  let pname, pnametok = match param.A.p_name with
    | A.DName (pname, pnametok) -> "$" ^ pname, pnametok in
  let err s = err errl func ("param " ^ pname ^ " " ^ s) in
  match param.A.p_type with
    | Some _ -> err "already has a type"
    | None ->
  match Smap.find pname comment_map with
    | None -> err "has no type"
    | Some at ->
  match CT.convert mode at with
    | C.Right s -> err ("does not convert cleanly: " ^ s)
    | C.Left ht ->
  pnametok.PI.transfo <- PI.AddBefore (PI.AddStr ((HT.to_string ht) ^ " "));
  changed := true;
  ()

let update_return mode (changed, errl) comment_map func =
  let (_, _, rptok) = func.A.f_params in
  match func.A.f_return_type with
    | Some _ -> err errl func "already has a return type"
    | None ->
  match Smap.find DP.ret_key comment_map with
    | None -> err errl func "has no return type"
    | Some at ->
  match CT.convert mode at with
    | C.Right s -> err errl func ("return type does not convert cleanly: " ^ s)
    | C.Left ht ->
  rptok.PI.transfo <- PI.AddAfter (PI.AddStr (": " ^ (HT.to_string ht)));
  changed := true;
  ()

let update_func mode (changed, errl) comment func =
  let comment_map = DP.parse comment in
  let (_, params, _) = func.A.f_params in
  List.iter begin function
    | C.Right3 _ (* Comma *) -> ()
    | C.Middle3 _ (* Hack-style variadic *) ->
      err errl func "unexpected Hack variadic"
    | C.Left3 param ->
      update_param mode (changed, errl) comment_map func param
  end params;
  update_return mode (changed, errl) comment_map func;
  ()

let visit_func mode (changed, errl) toks _ func =
  let tok = match func.A.f_modifiers with
    | (_, x) :: _ -> x
    | [] -> func.A.f_tok in
  let comment = find_comment (rev_take_until tok toks) in
  match comment with
    | None -> err errl func "has no AT docblock"
    | Some comment ->
      begin try
        update_func mode (changed, errl) comment func
      with At_parse.Parse_error ->
        err errl func "has invalid AT docblock"
      end

let visitor mode (changed, errl) toks =
  V.mk_visitor { V.default_visitor with
    V.kfunc_def = visit_func mode (changed, errl) toks;
    V.kmethod_def = visit_func mode (changed, errl) toks;
  }

let convert mode fn =
  let changed = ref false in
  let errl = ref [] in
  let (ast, toks) = Parse_php.ast_and_tokens fn in
  visitor mode (changed, errl) toks (A.Program ast);
  let outopt = if !changed then
    Some (Unparse_php.string_of_program_with_comments_using_transfo (ast, toks))
  else
    None
  in
  outopt, List.rev !errl

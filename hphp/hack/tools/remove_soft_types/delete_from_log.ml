(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Common

module A = Ast_php
module PI = Parse_info
module V = Visitor_php

exception Already_mutated

type parsed_arg = Argument of int | Return
type parsed_line = {file_name: string; func: string; cl: string option; arg: parsed_arg}

let remove_all_tokens_visitor = V.mk_visitor { V.default_visitor with
  V.kinfo = begin fun (k, _) tok ->
    tok.PI.transfo <- PI.Remove;
    k tok
  end
}

let find_token tok toks =
  List.find (fun x -> tok = Token_helpers_php.info_of_tok x) toks

let rec find_token_before tok = function
  | a :: b :: rest when tok = Token_helpers_php.info_of_tok b -> a
  | a :: rest -> find_token_before tok rest
  | [] -> raise Impossible

(**
 * Removes a function's argument annotation
 *)
let remove_function_arg_visit toks parsed_line (k, _) meth =
  if A.str_of_ident meth.A.f_name = parsed_line.func then begin
    match parsed_line.arg with
    | Argument arg ->
      (* Find the arg'th parameter. The f_params list contains other tokens, so
       * we need to count the occurances of Left3, which represents an actual
       * argument definition. *)
      let (_, params, _) = meth.A.f_params in
      let rec find n = function
        | Left3 x :: xs when n = arg -> x
        | Left3 _ :: xs -> find (n+1) xs
        | _ :: xs -> find n xs
        | _ -> raise Already_mutated (* it's possible for an argument to have been removed *)
      in
      let param = find 1 params in

      (* Delete the @ sign. *)
      (match param.A.p_soft_type, param.A.p_type with
      | Some tok, _ ->
        tok.PI.transfo <- PI.Remove
      | None, Some (A.HintQuestion _) ->
        ()
      | None, _ ->
        (* This is the only place we want to raise Already_mutated -- everywhere
         * else should be impossible, i.e., it indicates that the log message
         * refers to a function or paramter which doesn't exist at all. Here,
         * the parameter exists, we just don't have a soft typehint marker
         * there. Notably, we should never have a typehint marker without a soft
         * typehint marker or vice versa, so raising Impossible below is
         * correct.
         *)
        raise Already_mutated
      );

      (* Delete the typehint itself. *)
      (match param.A.p_type with
      | Some hint -> remove_all_tokens_visitor (A.Hint2 hint)
      | None -> raise Impossible
      );

      (* The above can leave a single space of whitespace before the argument
       * (the space that used to separate the typehint from the argument name),
       * remove it if found. Note that this doesn't remove indentation since
       * that occurs before the typehint tokens themselves, which are only
       * marked for deletion via transfo at this point but still appear in the
       * toks list. *)
      let A.DName(_, name_tok) = param.A.p_name in
      let tok_before = find_token_before name_tok toks in
      (match tok_before with
      | Parser_php.TSpaces sp -> sp.PI.transfo <- PI.Remove
      | _ -> ()
      )
    | _ -> ()
  end

(**
 * Removes a function's return annotation
 *)
let remove_function_return_visit toks parsed_line (k, _) meth =
  if A.str_of_ident meth.A.f_name = parsed_line.func then begin
    match parsed_line.arg with
    | Return ->
      let param = meth.A.f_return_type in
      (match param with
      | Some (tok, atsignopt, hint) ->
        (match atsignopt with
        | Some atsign -> atsign.PI.transfo <- PI.Remove
        | _ -> ()
        );
        remove_all_tokens_visitor (A.Hint2 hint);
        let tok = find_token tok toks in
        (match tok with
        | Parser_php.TCOLON x -> x.PI.transfo <- PI.Remove
        | _ -> raise Impossible
        )
      | None -> ()
      )
    | _ -> ()
  end

let remove_function_arg_visitor toks parsed_line =
  V.mk_visitor { V.default_visitor with
    V.kfunc_def = remove_function_arg_visit toks parsed_line;
    V.kmethod_def = remove_function_arg_visit toks parsed_line;
  }

let remove_function_return_visitor toks parsed_line =
  V.mk_visitor { V.default_visitor with
    V.kfunc_def = remove_function_return_visit toks parsed_line;
    V.kmethod_def = remove_function_return_visit toks parsed_line;
  }

(* Locates the appropriate class and then hands the body off to
 * remove_function_visitor since they are otherwise the same. *)
let remove_member_function_visitor toks class_ parsed_line =
  V.mk_visitor { V.default_visitor with
    V.kclass_def = begin fun (k, _) def ->
      if A.str_of_ident def.A.c_name = class_ then begin
        let (_, body, _) = def.A.c_body in
        List.iter begin fun stmt ->
          remove_function_arg_visitor toks parsed_line (A.ClassStmt stmt) end body;
        List.iter begin fun stmt ->
          remove_function_return_visitor toks parsed_line (A.ClassStmt stmt) end body
      end;
      k def
    end
  }

(* it's ~20% faster to compute the regexp only once *)
let re1 = Str.regexp ".*Argument \\([0-9]+\\) \\(passed \\)?to \\(.+::\\)?\\(.+\\)() must be of type [@?].+, .+ given (?in \\([^ ]+\\) on line [0-9]+)?"
let re2 = Str.regexp ".*Value returned from \\(.+::\\)?\\(.+\\)() must be of type @.+, .+ given (?in \\(.+\\) on line [0-9]+)?"

(**
 * Parses both, argument and return type failures.
 *
 * Also handle both, the default HPHP error log format and the very slightly modified format that www uses.
 *)
let parse_logline tbl line =
  if Str.string_match re1 line 0
  then begin
    let fn, func, class_opt, arg =
      Str.matched_group 5 line,
      Str.matched_group 4 line,
      (try Some (Str.matched_group 3 line) with Not_found -> None),
      int_of_string (Str.matched_group 1 line)
    in
    Some {file_name=fn; func=func; cl=class_opt; arg=Argument arg}
  end
  else if Str.string_match re2 line 0
  then begin
    let fn, func, class_opt =
      Str.matched_group 3 line,
      Str.matched_group 2 line,
      (try Some (Str.matched_group 1 line) with Not_found -> None)
    in
    Some {file_name=fn; func=func; cl=class_opt; arg=Return}
  end
  else None

let file_regexps = [Str.regexp ".*/flib/error/data/data.php$";
                    Str.regexp ".*/flib/web/haste/css/CSS.php$";
                    Str.regexp ".*/XControllerURIBuilder.php$";
                    Str.regexp ".*/flib/core/preparable/";
                    Str.regexp ".*/flib/http/uri/URI.php$";
                    Str.regexp ".*/flib/utils/crypto"]

let should_skip_file (file_name : string) : bool =
  List.fold_left (fun r re -> r || (Str.string_match re file_name 0)) false file_regexps

let fix_xhp_regexp = Str.regexp "__"

let fix_xhp_class_name (cl : string) : string =
  let len = String.length cl in
  if (len > 6) && (String.sub cl 0 4 = "xhp_")
  then begin
    let s = String.sub cl 4 (len - 6) in
    (* replace __ with : *)
    let s = Str.global_replace fix_xhp_regexp ":" s in
    let s = String.map (fun s -> if s = '_' then '-' else s) s in
    let s = ":" ^ s ^ "::" in
    s
  end
  else cl

let handle_logline tbl line =
  let parsed_line = parse_logline tbl line in
  match parsed_line with
  | None -> ()
  | Some parsed_line ->
    (* skip some files *)
    if should_skip_file parsed_line.file_name
    then Printf.printf "skipping: %s\n" parsed_line.file_name
    else begin
      (try
         if Hashtbl.mem tbl parsed_line then raise Already_mutated;
         let (ast, toks) = Parse_php.ast_and_tokens parsed_line.file_name in
         (match parsed_line.cl with
         | None ->
           remove_function_arg_visitor toks parsed_line (A.Program ast);
           remove_function_return_visitor toks parsed_line (A.Program ast)
         | Some class_ -> begin
           let class_ = fix_xhp_class_name class_ in
           (* Remove double colon *)
           let class_ = String.sub class_ 0 (String.length class_ - 2) in
           remove_member_function_visitor toks class_ parsed_line (A.Program ast)
         end
         );
         try
           let s =
             Unparse_php.string_of_program_with_comments_using_transfo (ast, toks) in
           Common.write_file parsed_line.file_name s
         with
           Stack_overflow ->
             Printf.printf "caught a stack overflow: %s\n" line;
             ()
       with Already_mutated ->
         ()
      );
      Hashtbl.replace tbl parsed_line ()
    end

let go logfile =
  let ic = open_in logfile in
  let tbl = Hashtbl.create 23 in
  try
    while true do
      handle_logline tbl (input_line ic)
    done;
  with End_of_file -> ();
  close_in ic

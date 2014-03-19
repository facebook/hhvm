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

open Ast_php

module Ast = Ast_php
module T = Parser_php
module PI = Parse_info
module V = Visitor_php

(*****************************************************************************)
(* Prelude *)
(*****************************************************************************)
(* 
 * todo: see also refactoring_code_php.ml
 *)

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let hhclient www =
  let cmd = spf "hh_client %s" www in
  let (xs, _) = 
    Common2.cmd_to_list_and_status cmd
    (* ["No errors!"], ()*)
  in
  (match xs with
  | ["No errors!"] -> None
  | _ -> Some (Common.take_safe 2 xs +> Common.join "\n")
  )

let rec with_trying_modif ?(undo_when_error=true) ~www file = function
  | [] -> false
  | f::f_rest ->
    if (hhclient www) <> None
    then failwith "unstable www state before modification";

    let tmpfile = Common.new_temp_file "hackificator" ".php" in
    Common.write_file ~file:tmpfile (Common.read_file file);

    let res = f file in
    match res with
    | None -> false
    | Some new_content ->
      Common.write_file ~file new_content;
      (match hhclient www with
      | None ->
        pr2 (spf "HACKIFIED: %s" file);
        true
      | Some err when undo_when_error ->
        pr2 (spf "ERROR HACKIFYING, UNDO: %s (err = %s)" file err);
        Common.write_file ~file (Common.read_file tmpfile);
        with_trying_modif ~undo_when_error ~www file f_rest
      | Some err ->
        pr2 (spf "ERROR HACKIFYING, but keep on: %s (err = %s)" file err);
        true
      )


let string_of_class_var_modifier modifiers =
  match modifiers with
  | NoModifiers _ -> "var"
  | VModifiers xs -> xs +> List.map (fun (_modifier, tok) ->
      PI.str_of_info tok) +> Common.join " "

(* todo: factorize with what is in database_prolog_php.ml *)
let rec string_of_hint_type x =
  match x with
  (* TODO: figure out a reasonable respresentation for type args in prolog *)
  | Hint (c, _targsTODO) ->
    (match c with
    | XName [QI (c)] -> Ast.str_of_ident c
    | XName qu -> raise (TodoNamespace (Ast_php.info_of_qualified_ident qu))
    | Self _ -> "self"
    | Parent _ -> "parent"
    | LateStatic _ -> "")
  | HintArray _ -> "array"
  | HintQuestion (_, t) -> "?" ^ (string_of_hint_type t)
  | HintTuple v1 ->
    let elts = List.map
      (fun x -> string_of_hint_type x)
      (Ast.uncomma (Ast.unparen v1))
    in
    "(" ^ (String.concat ", " elts) ^ ")"
  | HintCallback _ -> "callback"
  | HintShape _ -> failwith "I don't think shapes can appear here?"

(*****************************************************************************)
(* Visitor *)
(*****************************************************************************)
let visit ast =
  let visitor = V.mk_visitor { V.default_visitor with
    V.kparameter = (fun (k, _) x ->
      match x with
      | { p_type = Some ((Hint _ | HintArray _) as hint);
          p_default = Some((_i_7, Id(XName[QI(Name(("null", _i_8)))])));
          _
        } ->
        let ii = List.hd (Lib_parsing_php.ii_of_any (Hint2 hint)) in
        ii.PI.transfo <- PI.AddBefore (PI.AddStr "?");
      | _ -> k x
    );

    V.kexpr = (fun (k, _) x ->
      match x with
      | New (_, id, None) ->
        let ii = List.hd (Lib_parsing_php.ii_of_any (Expr id)) in
        ii.PI.transfo <- PI.AddAfter (PI.AddStr "()");
      | _ -> k x
    );
  }
  in
  visitor (Program ast)

let blockize_if_needed st =
  match st with
  | Block _ ->
    ()
  | _ ->
    let xs = Lib_parsing_php.ii_of_any (Stmt2 st) in
    let (hd, _middle, tl) = Common2.head_middle_tail xs in
    hd.PI.transfo <- PI.AddBefore (PI.AddStr "{ ");
    tl.PI.transfo <- PI.AddAfter (PI.AddStr " }");
    ()

let visit_thrift ast =
  let visitor = V.mk_visitor { V.default_visitor with
    V.kstmt = (fun (k, _) x ->
      match x with
      | If(_t, (_lp, _e, _rp), st1, _elseifs, else_opt) ->
        blockize_if_needed st1;
        (match else_opt with
        | None -> ()
        | Some (_tok, st) ->
          blockize_if_needed st
        );
        k x
      | _ -> k x
    );

    V.kparameter = (fun (k, _) x ->
      match x with
      | { p_type = Some ((Hint _ | HintArray _) as hint);
          p_default = Some((_i_7, Id(XName[QI(Name(("null", _i_8)))])));
          _
        } ->
        let ii = List.hd (Lib_parsing_php.ii_of_any (Hint2 hint)) in
        ii.PI.transfo <- PI.AddBefore (PI.AddStr "?");
      | _ -> k x
    );

  }
  in
  visitor (Program ast)

let visit_split_vars ~print_lines (ast, toks) =
  let visitor = V.mk_visitor { V.default_visitor with
    (* mostly a copy paste of refactoring_code_php.ml *)
    V.kclass_stmt = (fun (k, _) x ->
      match x with
     (* private $x, $y; *)
      | ClassVariables (modifiers, _typ_opt, xs, _semicolon) ->
        (match xs with
        (* $x *)
        | Left (_dname, _affect_opt)::rest ->

          let rec aux rest =
            match rest with
            (* , $y -> ;\n private $y *)
            | Right comma::Left (dname, _affect_opt)::rest ->
              (* todo: look at col of modifiers? *)
              let indent = "  " in
              let tok = Ast.info_of_dname dname in
              let s = Ast.str_of_dname dname in

              (* Transform the variable into itself with the class modifiers
               * prepended. We transform the comma before it into a semicolon
               * below. *)
              let str_modifiers =
                spf "%s $%s"
                  (string_of_class_var_modifier modifiers) s
              in

              (* Deal with inserting newlines and reindentation. *)
              let str_modifiers =
                if PI.line_of_info comma = PI.line_of_info tok
                then begin
                  (* This variable was on the same line as the last variable.
                   * Insert a newline and reindent. *)
                  if print_lines then pr (string_of_int (PI.line_of_info tok));
                  "\n" ^ indent ^ str_modifiers
                end else begin
                  (* This variable was on the line below the last variable. We
                   * don't need a newline, but we need to remove the indentation
                   * that was likely here. We do this by searching the token
                   * stream for the variable's token, finding the whitespace we
                   * suspect is before it, and shortening that whitespace by
                   * two spaces. *)
                  let rec locate elt = function
                    | (_acc, []) -> None (* pad: bug? no acc? *)
                    | (acc, x :: rest)
                      when (Token_helpers_php.info_of_tok x) = elt ->
                        Some (acc, x :: rest)
                    | (acc, x :: rest) -> locate elt (x :: acc, rest)
                  in

                  begin match locate tok ([], toks) with
                    | Some (Parser_php.TSpaces sp :: _, _) ->
                      let sp_str = PI.str_of_info sp in
                      let len = String.length sp_str in
                      let indent_len = String.length indent in

                      if len > indent_len
                      then begin
                        let new_str = String.sub sp_str 0 (len - indent_len) in
                        sp.PI.transfo <- PI.Replace (PI.AddStr new_str);
                      end;
                    | _ -> ()
                  end;

                  str_modifiers
                end in

              comma.PI.transfo <- PI.Replace (PI.AddStr ";");
              tok.PI.transfo <- PI.Replace (PI.AddStr str_modifiers);
              aux rest
            | [] -> ()
            | _ -> raise Impossible
          in
          aux rest;
          k x
      | _ -> raise Impossible
    )
      | _ -> k x
    );

  }
  in
  visitor (Program ast)

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let hackify ~upgrade header file =
  (* Phabricator is not smart about looking for at-generated *)
  let pat = ".*@" ^ "generated" in 
  (* Skip generated files *)
  if Common.cat file +> List.exists (fun s -> s =~ pat)
  then 
    (pr2 (spf "SKIPPING GENERATED: %s" file); None)
  else try 
    let ((ast, toks), _stat) = Parse_php.parse file in
    match toks with
     | T.T_OPEN_TAG info::rest ->
       let s = PI.str_of_info info in
       let hh_header = s =$= "<?hh" in
       if (upgrade && not hh_header) || (not upgrade && hh_header)
       then None
       else begin
         (* Remove the <?foo header *)
         info.PI.transfo <- PI.Replace (PI.AddStr header);

         (* Remove a "<?hh // mode" mode comment *)
         let munge_mode spopt mode = begin
           let s = PI.str_of_info mode in
           if s =$= "// decl" || s=$= "//decl" || s =$= "// strict" then begin
             mode.PI.transfo <- PI.Remove;
             (* Remove spaces only if we removed a mode *)
             (match spopt with
               | Some sp -> sp.PI.transfo <- PI.Remove
               | None -> ()
             )
           end
         end in
         (match rest with
           | T.TSpaces sp :: T.T_COMMENT mode :: _ -> munge_mode (Some sp) mode
           | T.T_COMMENT mode :: _ -> munge_mode None mode
           | _ -> ()
         );

         (* Actually do the transform *)
         visit ast;
         let s = Unparse_php.string_of_program_with_comments_using_transfo
           (ast, toks) in
         Some s
       end
     | _ -> 
       pr2 (spf "no open tag, skipping %s" file);
       None
  with Parse_php.Parse_error _ ->
    pr2 (spf "Parse error: %s" file);
    None




(* The transformation here may respect less indentation, but because they
 * are applied on generated code (thrift code), we care less
 *)
let hackify_thrift file =
  try 
    let (ast, toks) = Parse_php.ast_and_tokens file in
    match ast, toks with
     (* ugly: empty files like <?php // xxx\n ?> are tokenized
      * with a fake SEMICOLON and so when we remove the ?> we also remove
      * the comment just before, and on empty generated thrift file,
      * this cause resign.php to fail, so hardcode here the recognition
      * of such empty thrift generated file
      *)
    | [StmtList([EmptyStmt(_); InlineHtml((_))]); FinalDef(_)], _ ->
      None

     | _, T.T_OPEN_TAG info::_rest ->
       let s = PI.str_of_info info in
       if s =$= "<?hh"
       then None
       else begin
         info.PI.transfo <- PI.Replace (PI.AddStr "<?hh");
         visit_thrift ast;
         visit_split_vars ~print_lines:false (ast, toks);
         toks +> List.iter (function
           (* ugly: we should have a T_CLOSE_TAG instead of abusing SEMICOLON *)
           | T.TSEMICOLON (ii) ->
             let s = PI.str_of_info ii in
             if s =$= "?>"
             then begin
               ii.PI.transfo <- PI.Remove;
             end
           | _ -> ()
         );
         let s = Unparse_php.string_of_program_with_comments_using_transfo 
           (ast, toks) in
         Some s
       end
     | _ -> 
       pr2 (spf "no open tag, skipping %s" file);
       None
  with Parse_php.Parse_error _ ->
    pr2 (spf "Parse error: %s" file);
    None

let split_vars file =
  try
    let (ast, toks) = Parse_php.ast_and_tokens file in
    pr "Split lines:";
    visit_split_vars ~print_lines:true (ast, toks);
    let s = Unparse_php.string_of_program_with_comments_using_transfo 
      (ast, toks) in
    Some s
  with Parse_php.Parse_error _ ->
    pr2 (spf "Parse error: %s" file);
    None

let adjust_type_hints_following_model m ~params_interface ~params_class =
  if List.length params_interface <> List.length params_class
  then pr2 
    (spf "CLOWNTOWN: method %s does not have the right number of parameters"
       m);

  Common2.zip_safe params_interface params_class +> List.iter (fun (p1, p2) ->
    let type_model = 
      match p1.p_type with
      | None -> failwith "no type for this parameter?"
      | Some ht -> ht
    in
    match p2.p_type with
    | None ->
      let var = p2.p_name in
      let tok = Ast.info_of_dname var in
      tok.PI.transfo <- PI.AddBefore 
        (PI.AddStr (string_of_hint_type type_model ^ " "));
    | Some ht2 ->
      if (string_of_hint_type ht2) =$= 
         (string_of_hint_type type_model)
      then ()
      else failwith "the two types are different"
      
  )

let hackify_class_of_typed_interface interface_file class_file =
  let ast_interface = Parse_php.parse_program interface_file in
  let (ast, toks) = Parse_php.ast_and_tokens class_file in
  
  let (name_interface: string), 
      (methods_interface: (string, Ast_php.parameter list) Common.assoc)
    = 
    match ast_interface with
    | ClassDef {c_type = Interface _; c_name = name; c_body = (_, body, _);_}
      ::_ 
      ->
      Ast.str_of_ident name,
      body +> Common.map_filter (function
      | Method def ->
        Some (Ast.str_of_ident def.f_name,
              def.f_params +> Ast.unparen +> Ast.uncomma_dots
        )
      | _ -> 
        None
      )
    | _ ->
     failwith (spf "could not find interface in %s" interface_file)

  in

  ast +> List.iter (function
  | ClassDef {c_implements = Some (_, interfaces); 
              c_body = (_, body, _);
              c_name = name;
              _
             } ->
    let xs = interfaces +> Ast.uncomma +> List.map Ast.str_of_class_name in
    if List.mem name_interface xs
    then begin
      pr2 (spf "Found %s implementing %s" 
             (Ast.str_of_ident name) name_interface);
      body +> List.iter (function
      | Method def ->
        (try 
          let params_interface = List.assoc (Ast.str_of_ident def.f_name)
            methods_interface in
          adjust_type_hints_following_model (Ast.str_of_ident def.f_name)
            ~params_interface
            ~params_class:(def.f_params +> Ast.unparen +> Ast.uncomma_dots)
        with Not_found ->
          ()
        )
      | _ -> ()
      );
    end
  | _ -> ()
  );

  let s = Unparse_php.string_of_program_with_comments_using_transfo
    (ast, toks) in
  s


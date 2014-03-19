(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils
  
let main_js s =
  Ast.is_js := true;
  Pos.file := "js" ;
  let lb = Lexing.from_string s in
  Utils.db := Some (); (* Dbm.opendbm db_path [Dbm.Dbm_rdwr; Dbm.Dbm_create] 0o755); *)
  let buf = Buffer.create 256 in
  try
    let builtins = Parser.program Lexer.token (Lexing.from_string Ast.builtins) in
    let ast = Parser.program Lexer.token lb in
    let ast = builtins @ ast in
    let nenv = Naming.make_env Naming.empty ast in
    let nast = Naming.program nenv ast in
    let tenv = Typing_env.empty() in
    let tenv = Typing.make_env tenv nenv SMap.empty (ref SSet.empty) ast in
    let _ = Typing.program tenv nast in
    JsGen.program (Buffer.add_string buf) nast;
    Buffer.contents buf
  with
  | Parsing.Parse_error -> Utils.pmsg_l [Pos.make lb, "Syntax error"]
  | Utils.Error l -> Utils.pmsg_l l

let (compile: (string, string -> string) Js.meth_callback) = Js.wrap_callback main_js
open Js.Unsafe
let () = eval_string "my_ocaml = { x: null, get: function () { return this.x; }, set: function (x) { this.x = x; }};"
let () = meth_call (eval_string "my_ocaml") "set" [|inject compile|]
let () = eval_string "compile_php_string = function(s) { return my_ocaml.get()(new MlString(s)).bytes; };"

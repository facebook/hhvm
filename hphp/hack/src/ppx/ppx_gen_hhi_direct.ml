(*
 * Copyright (c) 2013-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "flow" directory of this source tree.
 *
 *)

(* Ast_mapper based PPX, used by BUCK *)

open Asttypes
open Parsetree
open Ast_mapper
open Ast_helper

(* Turn the (name, contents) list into a PPX ast (string * string) array
 * expression *)
let contents hhi_dir =
  Ppx_gen_hhi_common.get_hhis hhi_dir
  |> List.map (fun (name, contents) ->
         Exp.tuple
           [
             Exp.constant (Const.string name);
             Exp.constant (Const.string contents);
           ])
  |> Exp.array

(* Whenever we see [%hhi_contents], replace it with all of the hhis *)
let ppx_gen_hhi_mapper hhi_dir =
  {
    default_mapper with
    expr =
      (fun mapper expr ->
        match expr with
        | {
         pexp_desc = Pexp_extension ({ txt = "hhi_contents"; _ }, PStr []);
         _;
        } ->
          contents hhi_dir
        | other -> default_mapper.expr mapper other);
  }

let register () =
  register "ppx_gen_hhi" (fun _argv ->
      let hhi_dir = Sys.argv.(1) in
      ppx_gen_hhi_mapper hhi_dir)

let () = register ()

(*
 * Copyright (c) 2013-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "flow" directory of this source tree.
 *
 *)

(* Ppxlib based PPX, used by DUNE *)
open Ppxlib

(* Turn the (name, contents) list into a PPX ast (string * string) array
 * expression *)
let contents hhi_dir hsl_dir =
  let open Ast_helper in
  let handwritten_hhis = Ppx_gen_hhi_common.get_hhis hhi_dir in
  let generated_hsl_hhis = Ppx_gen_hhi_common.get_hhis hsl_dir
    |> List.map (fun (name, contents) -> ("hsl_generated/" ^ name, contents))
  in
  handwritten_hhis @ generated_hsl_hhis
  |> List.map (fun (name, contents) ->
         Exp.tuple
           [
             Exp.constant (Const.string name);
             Exp.constant (Const.string contents);
           ])
  |> Exp.array

let hhi_dir : string option ref = ref None
let hsl_dir : string option ref = ref None

(* Whenever we see [%hhi_contents], replace it with all of the hhis *)
let expand_function ~loc:_ ~path:_ =
  let hhi_dir =
    match !hhi_dir with
    | None -> raise (Arg.Bad "-hhi-dir is mandatory")
    | Some dir -> dir
  in
  let hsl_dir =
    match !hsl_dir with
    | None -> raise (Arg.Bad "-hsl-dir is mandatory")
    | Some dir -> dir
  in
  contents hhi_dir hsl_dir

let extension =
  Extension.declare
    "hhi_contents"
    Extension.Context.expression
    Ast_pattern.(pstr nil)
    expand_function

let rule = Context_free.Rule.extension extension

let set_hhi_dir dir = hhi_dir := Some dir
let set_hsl_dir dir = hsl_dir := Some dir

let () =
  Driver.add_arg
    "-hhi-dir"
    (Arg.String set_hhi_dir)
    ~doc:"<dir> directory of the hhis sources";
  Driver.add_arg
    "-hsl-dir"
    (Arg.String set_hsl_dir)
    ~doc:"<dir> directory of the generated HHIs for the HSL";
  Driver.register_transformation ~rules:[rule] "ppx_gen_hhi"

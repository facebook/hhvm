(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

let check_defs_tast tcopt fn {FileInfo.funs; classes; typedefs; consts; _} =
  let open Option in
  let result, tast, _ = (Errors.do_ (fun () ->
    let fs = List.map funs begin fun (_, x) ->
      Typing_check_service.type_fun tcopt fn x
      >>| (fun (f, env) -> Tast.Fun f, env)
    end in
    let cs = List.map classes begin fun (_, x) ->
      Typing_check_service.type_class tcopt fn x
      >>| (fun (c, env) -> Tast.Class c, env)
    end in
    let ts = List.map typedefs begin fun (_, x) ->
      Typing_check_service.check_typedef tcopt fn x
      >>| (fun (td, env) -> Tast.Typedef td, env)
    end in
    let gcs = List.map consts begin fun (_, x) ->
      Typing_check_service.check_const tcopt fn x
      >>| (fun (c, env) -> Tast.Constant c, env)
    end in
    (fs @ cs @ ts @ gcs)
  )) in
  result, tast

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn fi = fst (check_defs_tast tcopt fn fi)

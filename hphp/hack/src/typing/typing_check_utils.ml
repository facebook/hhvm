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

let type_file tcopt fn {FileInfo.funs; classes; typedefs; consts; _} =
  let open Option.Monad_infix in
  let errors, tast, _ = Errors.do_with_context fn Errors.Typing begin fun () ->
    let fs = List.filter_map funs begin fun (_, x) ->
      Typing_check_service.type_fun tcopt fn x
      >>| fun f -> Tast.Fun f
    end in
    let cs = List.filter_map classes begin fun (_, x) ->
      Typing_check_service.type_class tcopt fn x
      >>| fun c -> Tast.Class c
    end in
    let ts = List.filter_map typedefs begin fun (_, x) ->
      Typing_check_service.check_typedef tcopt fn x
      >>| fun td -> Tast.Typedef td
    end in
    let gcs = List.filter_map consts begin fun (_, x) ->
      Typing_check_service.check_const tcopt fn x
      >>| fun c -> Tast.Constant c
    end in
    (fs @ cs @ ts @ gcs)
  end in
  tast, errors

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn fi = snd (type_file tcopt fn fi)

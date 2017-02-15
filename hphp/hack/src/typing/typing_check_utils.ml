(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn {FileInfo.funs; classes; typedefs; consts; _} =
  let result, _, _ = (Errors.do_ (fun () ->
    ignore(
      List.map funs begin fun (_, x) ->
        Typing_check_service.type_fun tcopt fn x
      end
    );
    ignore(
      List.map classes begin fun (_, x) ->
        Typing_check_service.type_class tcopt fn x;
      end
    );
    List.iter typedefs begin fun (_, x) ->
      Typing_check_service.check_typedef tcopt fn x
    end;
    List.iter consts begin fun (_, x) ->
      Typing_check_service.check_const tcopt fn x
    end;
  )) in
  result

let get_nast_from_fileinfo tcopt fn fileinfo =
  let funs = fileinfo.FileInfo.funs in
  let name_function (_, fun_) =
    Typing_check_service.type_fun tcopt fn fun_ in
  let named_functions = List.filter_map funs name_function in

  let classes = fileinfo.FileInfo.classes in
  let name_class (_, class_) =
    Typing_check_service.type_class tcopt fn class_ in
  let named_classes = List.filter_map classes name_class in

  let named_typedefs = [] in (* TODO typedefs *)
  let named_consts = [] in (* TODO consts *)
  (named_functions, named_classes, named_typedefs, named_consts)

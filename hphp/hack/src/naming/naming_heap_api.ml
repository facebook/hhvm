(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_class ctx id =
  match Naming_provider.get_class_path ctx id with
  | None -> None
  | Some fn ->
    (match Ast_provider.find_class_in_file ctx fn id with
    | None -> None
    | Some class_ -> Some (Errors.ignore_ (fun () -> Naming.class_ ctx class_)))

let get_fun ctx id =
  match Naming_provider.get_fun_path ctx id with
  | None -> None
  | Some fn ->
    (match Ast_provider.find_fun_in_file ctx fn id with
    | None -> None
    | Some fun_ -> Some (Naming.fun_ ctx fun_))

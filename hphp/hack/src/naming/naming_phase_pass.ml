(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Common

type 'ctx t =
  | Top_down of 'ctx Aast_defs.Pass.t
  | Bottom_up of 'ctx Aast_defs.Pass.t

let combine ts =
  let id = Aast_defs.Pass.identity () in
  List.fold_left ts ~init:(id, id) ~f:(fun (td_acc, bu_acc) t ->
      match t with
      | Top_down td -> (Aast_defs.Pass.combine td_acc td, bu_acc)
      | Bottom_up bu -> (td_acc, Aast_defs.Pass.combine bu_acc bu))

let top_down pass = Top_down pass

let bottom_up pass = Bottom_up pass

let mk_visitor passes =
  let (top_down, bottom_up) = combine passes in
  Aast_defs.
    ( (fun ctx elem -> transform_ty_program elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_class_ elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_fun_def elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_module_def elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_gconst elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_typedef elem ~ctx ~top_down ~bottom_up),
      (fun ctx elem -> transform_ty_stmt elem ~ctx ~top_down ~bottom_up) )

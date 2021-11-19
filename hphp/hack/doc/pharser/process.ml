(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open MenhirSdk

module G = Cmly_read.Read(struct let filename = Sys.argv.(1) end)

(*let rec list_filter_map f = function
  | [] -> []
  | x :: xs ->
    match f x with
    | None -> list_filter_map f xs
    | Some x' -> x' :: list_filter_map f xs

let reductions =
  G.Lr1.tabulate (fun lr1 ->
      lr1
      |> G.Lr1.reductions
      |> List.map snd
      |> List.flatten
      |> List.sort_uniq compare
    )

let items offset lr1 =
  list_filter_map (fun prod ->
      let rhs = G.Production.rhs prod in
      let dot = Array.length rhs - offset in
      if dot < 0
      then None
      else Some (prod, dot)
    ) (reductions lr1)

let rec all_reductions offset lr1 =
  G.Lr1.
  if offset < 5 then
    List.flatten @@
    reductions offset lr1 ::
    List.map
      (fun (_sym, lr1) -> all_reductions (offset + 1) lr1)
      (G.Lr1.transitions lr1)
  else
    []*)

let () =
  Printf.printf "let describe_state = function\n";
  G.Lr1.iter begin fun lr1 ->
    Printf.printf "  | %d -> %S\n"
      (G.Lr1.to_int lr1)
      (Format.asprintf "%a" G.Print.itemset
         (G.Lr0.items (G.Lr1.lr0 lr1)))
  end;
  Printf.printf "  | _ -> assert false\n"

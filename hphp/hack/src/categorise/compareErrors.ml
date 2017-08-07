(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


type token =
  | Any
  | AnyOptional
  | Token of string

let tokenise str = List.map (fun s -> Token s) @@ Pcre.split str

(* BINARY MATCHER *)
let rec template_of_two t1 t2 =
  match t1, t2 with
    | [], [] -> []
    | hd1 :: tl1, hd2 :: tl2 ->
      if hd1 = hd2
      then hd1 :: (template_of_two tl1 tl2)
      else Any :: (template_of_two tl1 tl2)
    | _ :: tl, [] | [], _ :: tl -> AnyOptional :: (template_of_two tl [])


(* GENERAL MATCHER *)
module HT = Hashtbl

type matcher =
  | End
  | MatchNode of (token, matcher) HT.t

let new_ht k v = let ht = HT.create 1 in HT.add ht k v; ht

let build_matcher tokens =
  let rec build_matcher_aux = function
    | [] -> End
    | hd :: tl ->
      MatchNode (new_ht hd (build_matcher_aux tl)) in
  build_matcher_aux tokens


let merge_into_matcher matcher tokens =
  let rec merge_into_matcher_aux current_node tokens =
    match current_node, tokens with
      | End, [] -> ()                 (* TODO *)
      | End, _ :: _ -> ()           (* TODO *)
      | MatchNode table, hd :: tl ->
        if HT.mem table hd
        then merge_into_matcher_aux (HT.find table hd) tl
        else
          let current_node_layer = HT.fold (fun _ v acc -> v :: acc) table [] in
          let new_branch = MatchNode (HT.create 1) in
          HT.add table hd new_branch;
          build_new_branch new_branch current_node_layer tl
      | MatchNode _, _ -> ()
  and build_new_branch _ _ _ =
    (* So now we have our new node and the rest of that node layer (list),
       as well as the token list (args in order) *)
    ()   (* TODO *)
  in
  merge_into_matcher_aux matcher tokens

(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open SearchUtils

(* Information about one leaf in the namespace tree *)
type nss_node = {

  (* The name of just this leaf *)
  nss_name: string;

  (* The full name including all parent trunks above this leaf *)
  nss_full_namespace: string;

  (* A hashtable of all leaf elements below this branch *)
  nss_children: (string, nss_node) Hashtbl.t;
}

(* This is the root namespace *)
let root_namespace = {
    nss_name = "\\";
    nss_full_namespace = "\\";
    nss_children = Hashtbl.create 0;
  }

(* Register a namespace *)
let register_namespace (namespace: string): unit =
  let elem_list = String.split_on_char '\\' namespace in
  let _ = Core_kernel.List.fold elem_list ~init:root_namespace
    ~f:(fun current_node leaf_name ->

    (* Ignore trailing backslashes *)
    if leaf_name <> "" then begin

      (* Check to see if this leaf exists within the current node *)
      match Hashtbl.find_opt current_node.nss_children leaf_name with
      | Some matching_leaf ->
        matching_leaf
      | None ->
        let new_node = {
          nss_name = leaf_name;
          nss_full_namespace = current_node.nss_full_namespace ^ "\\" ^ leaf_name;
          nss_children = Hashtbl.create 0;
        } in
        Hashtbl.add current_node.nss_children leaf_name new_node;
        new_node
    end else current_node
  ) in
  ()

(* Find all namespaces that match this prefix *)
let find_matching_namespaces (query_text: string): si_results =
  let elem_list = String.split_on_char '\\' query_text in
  let current_node = ref root_namespace in
  let reached_end = ref false in
  let matches = ref [] in
  Core_kernel.List.iter elem_list ~f:(fun leaf_name -> begin

        (* If we've already reached the end of matches, offer no more *)
        if !reached_end then begin
          matches := [];
        end else begin

          (* Check to see if this leaf exists within the current node *)
          match Hashtbl.find_opt !current_node.nss_children leaf_name with
          | Some matching_leaf ->
            current_node := matching_leaf;
          | None ->
            Hashtbl.iter (fun key _ ->
                if Core_kernel.String.is_substring key ~substring:leaf_name then begin
                  matches := {
                    si_name = key;
                    si_kind = SI_Namespace;
                    si_filehash = 0L;
                  } :: !matches;
                end
              ) !current_node.nss_children;
            reached_end := true;
        end
      end);

  (* Now take those matches and return them as a list *)
  !matches

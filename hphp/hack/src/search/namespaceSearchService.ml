(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open SearchUtils

(* Register a namespace *)
let register_namespace
    ~(sienv: si_env)
    ~(namespace: string): unit =
  let elem_list = String.split_on_char '\\' namespace in
  let _ = Core_kernel.List.fold elem_list ~init:sienv.nss_root_namespace
    ~f:(fun current_node leaf_name ->

    (* Ignore extra backslashes *)
    if leaf_name <> "" then begin

      (* Check to see if this leaf exists within the current node *)
      let leaf_lc = String.lowercase_ascii leaf_name in
      match Hashtbl.find_opt current_node.nss_children leaf_lc with
      | Some matching_leaf ->
        matching_leaf
      | None ->
        let new_node = {
          nss_name = leaf_name;
          nss_full_namespace =
            if current_node.nss_full_namespace = "\\" then
              "\\" ^ leaf_name
            else
              current_node.nss_full_namespace ^ "\\" ^ leaf_name;
          nss_children = Hashtbl.create 0;
        } in
        Hashtbl.add current_node.nss_children leaf_lc new_node;
        new_node
    end else current_node
  ) in
  ()

(* Find the namespace that matches this prefix *)
let find_exact_match
    ~(sienv: si_env)
    ~(namespace: string): nss_node =

  (* If we're at the root namespace the answer is easy *)
  if namespace = "" || namespace = "\\" then begin
    sienv.nss_root_namespace
  end else begin
    let elem_list = String.split_on_char '\\'
      (String.lowercase_ascii namespace) in
    Core_kernel.List.fold elem_list ~init:sienv.nss_root_namespace
      ~f:(fun current_node leaf_name ->

      (* Check to see if this leaf exists within the current node *)
      if leaf_name <> "" then begin
        match Hashtbl.find_opt current_node.nss_children leaf_name with
        | Some matching_leaf -> matching_leaf;
        | None -> raise Not_found
      end else current_node
    )
  end

(* Produce a list of results for the current node *)
let get_matches_for_node (node: nss_node): si_results =
  Hashtbl.fold (fun _key value acc ->
    {
      si_name = value.nss_name;
      si_kind = SI_Namespace;
      si_filehash = 0L;
    } :: acc
  ) node.nss_children []

(* Find all namespaces that match this prefix *)
let find_matching_namespaces
    ~(sienv: si_env)
    ~(query_text: string): si_results =

  (* Trivial case *)
  if query_text = "" then begin
    []

  (* Special case - just give root namespace only *)
  end else if query_text = "\\" then begin
    get_matches_for_node sienv.nss_root_namespace

  (* Normal case, search for matches *)
  end else begin
    let elem_list = String.split_on_char '\\'
      (String.lowercase_ascii query_text) in
    let current_node = ref sienv.nss_root_namespace in
    let reached_end = ref false in
    let matches = ref [] in
    Core_kernel.List.iter elem_list ~f:(fun leaf_name ->

      (* If we've already reached the end of matches, offer no more *)
      if !reached_end then begin
        matches := [];
      end else begin
        if leaf_name <> "" then begin

          (* Check to see if this leaf exists within the current node *)
          match Hashtbl.find_opt !current_node.nss_children leaf_name with
          | Some matching_leaf ->
            current_node := matching_leaf;
            matches := get_matches_for_node matching_leaf;
          | None ->
            matches := [];
            Hashtbl.iter (fun key _ ->
                if Core_kernel.String.is_substring key ~substring:leaf_name then begin
                  let node = Hashtbl.find !current_node.nss_children key in
                  matches := {
                    si_name = node.nss_name;
                    si_kind = SI_Namespace;
                    si_filehash = 0L;
                  } :: !matches;
                end
              ) !current_node.nss_children;
            reached_end := true;
        end
      end
    );

    (* Now take those matches and return them as a list *)
    !matches
  end

(*
 * Register a namespace alias, represented as a shortcut in the tree.
 * The alias will share the hashtbl of its target, so future namespaces
 * should appear in both places.
 *)
let register_alias
    ~(sienv: si_env)
    ~(alias: string)
    ~(target: string): unit =
  try

    (* First find the target and make sure there's only one *)
    register_namespace ~sienv ~namespace:target;
    let target_node = find_exact_match ~sienv ~namespace:target in

    (* Now assert that the alias cannot have a backslash in it *)
    let (source_ns, name) = Utils.split_ns_from_name alias in
    let source = find_exact_match ~sienv ~namespace:source_ns in

    (* Register this alias at the root *)
    let new_node = {
      nss_name = name;
      nss_full_namespace = target_node.nss_full_namespace;
      nss_children = target_node.nss_children;
    } in
    Hashtbl.add source.nss_children (String.lowercase_ascii name) new_node;
  with Not_found ->
    Hh_logger.log "Unable to register namespace map for [%s] -> [%s]"
      alias target;

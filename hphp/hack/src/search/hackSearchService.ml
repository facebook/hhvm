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
open Utils
open Reordered_argument_collections

let fuzzy = ref false
type search_result_type =
  | Class of Ast.class_kind option
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant

type symbol = (Pos.absolute, search_result_type) SearchUtils.term
type result = symbol list

module SS = SearchService.Make(struct
  type t = search_result_type
  let fuzzy_types = [Class (Some Ast.Cnormal); Function; Constant; Typedef]
  let type_num = function
    | Class _ -> 0
    | Function -> 1
    | Typedef -> 2
    | Constant -> 3
    | _ -> 4
  let compare_result_type a b =
    (type_num a) - (type_num b)
end)

module WorkerApi = struct
  (* cleans off namespace and colon at the start of xhp name because the
   * user will want to search for xhp classes without typing a : at
   * the start of every search *)
  let clean_key key =
    if (String.length key) > 0
    then
      let key = String.lowercase (Utils.strip_ns key) in
      if (String.length key) > 0 && key.[0] = ':'
      then String.sub key 1 (String.length key - 1)
      else key
    else key

  (* Unlike anything else, we need to look at the class body to extract it's
   * methods so that they can also be searched for *)
  let update_class c (acc : SS.Trie.SearchUpdates.t) =
    let prefix = (snd c.Ast.c_name)^"::" in
    List.fold_left c.Ast.c_body ~f:begin fun acc elt ->
      match elt with
        | Ast.Method m -> let id = m.Ast.m_name in
            let pos, name = FileInfo.pos_full id in
            let full_name = prefix^name in
            let is_static = List.mem m.Ast.m_kind Ast.Static in
            let type_ =
              Method (is_static, (Utils.strip_ns (snd c.Ast.c_name)))
            in
            let acc =
              SS.WorkerApi.process_trie_term (clean_key name) name pos type_ acc
            in
            SS.WorkerApi.process_trie_term
              (clean_key full_name) name pos type_ acc
        | _ -> acc
    end ~init:acc

  let add_trie_term id type_ acc =
    let (pos, name) = id in
    SS.WorkerApi.process_trie_term (strip_ns name) name pos type_ acc

  let add_fuzzy_term id type_ acc =
    let name = snd id in
    let key = strip_ns name in
    SS.WorkerApi.process_fuzzy_term key name (fst id) type_ acc

  let add_autocomplete_term id type_ acc =
    let pos, name = id in
    let key = strip_ns name in
    SS.WorkerApi.process_autocomplete_term key name pos type_ acc

  let update_defs id type_ fuzzy_defs trie_defs =
    if !fuzzy then
    add_fuzzy_term id type_ fuzzy_defs, trie_defs
    else
    fuzzy_defs, add_trie_term id type_ trie_defs


  (* We don't have full info here, so we give File positions and None for *)
  (* class kind. We then lazily populate class methods for search later *)
  let update_from_fast fn (names : FileInfo.names) =
    let { FileInfo.n_funs; n_types; n_consts; n_classes } = names in
    (* Fold all the functions *)
    let fuzzy, trie, auto = SSet.fold n_funs
        ~init:(SS.Fuzzy.TMap.empty, [], [])
        ~f:begin fun name (f, t, a)  ->
            let pos = FileInfo.File (FileInfo.Fun, fn) in
            let f, t = update_defs (pos, name) Function f t in
            f, t, add_autocomplete_term (pos, name) Function a
        end in
    let fuzzy, trie, auto = SSet.fold n_classes
        ~init:(fuzzy, trie, auto)
        ~f:begin fun name (f, t, a)  ->
            let pos = FileInfo.File (FileInfo.Class, fn) in
            let f, t = update_defs (pos, name) (Class None) f t in
            f, t, add_autocomplete_term (pos, name) (Class None) a
        end in
    let fuzzy, trie, auto = SSet.fold n_types
        ~init:(fuzzy, trie, auto)
        ~f:begin fun name (f, t, _a)  ->
          let pos = FileInfo.File (FileInfo.Typedef, fn) in
          let f, t = update_defs (pos, name) Typedef f t in
          f, t, _a
        end in
    let fuzzy, trie, auto = SSet.fold n_consts
        ~init:(fuzzy, trie, auto)
        ~f:begin fun name (f, t, _a)  ->
          let pos = FileInfo.File (FileInfo.Const, fn) in
          let f, t = update_defs (pos, name) Constant f t in
          f, t, _a
        end in
    SS.WorkerApi.update fn trie fuzzy auto

  (* Update from the full fileInfo object: get all data except class kind *)
  let update_from_fileinfo fn (info : FileInfo.t) =
    let { FileInfo.funs; typedefs; consts; classes; _} = info in
    let fuzzy, trie, auto = List.fold funs
        ~init:(SS.Fuzzy.TMap.empty, [], [])
        ~f:begin fun (f, t, a) id ->
            let f, t = update_defs id Function f t in
            f, t, add_autocomplete_term id Function a
        end in

    let fuzzy, trie, auto = List.fold classes
        ~init:(fuzzy, trie, auto)
        ~f:begin fun  (f, t, a) id ->
            let f, t = update_defs id (Class None) f t in
            f, t, add_autocomplete_term id (Class None) a
        end in
    let fuzzy, trie, auto = List.fold typedefs
        ~init:(fuzzy, trie, auto)
        ~f:begin fun  (f, t, _a) id  ->
          let f, t = update_defs id Typedef f t in
          f, t, _a
        end in
    let fuzzy, trie, auto = List.fold consts
        ~init:(fuzzy, trie, auto)
        ~f:begin fun (f, t, _a) id ->
          let f, t = update_defs id Constant f t in
          f, t, _a
        end in
    SS.WorkerApi.update fn trie fuzzy auto

  (* Called by a worker after the file is parsed *)
  let update fn ast =
    let fuzzy_defs, trie_defs =
      List.fold_left ast ~f:begin fun (fuzzy_defs, trie_defs) def ->
      match def with
      | Ast.Fun f ->
          update_defs
            (FileInfo.pos_full f.Ast.f_name)
            Function
            fuzzy_defs
            trie_defs
      | Ast.Class c ->
          (* Still index methods for trie search *)
          let trie_defs = update_class c trie_defs in
          update_defs (FileInfo.pos_full c.Ast.c_name)
                      (Class (Some c.Ast.c_kind))
                      fuzzy_defs
                      trie_defs
      | Ast.Typedef td ->
          update_defs (FileInfo.pos_full td.Ast.t_id)
            Typedef fuzzy_defs trie_defs
      | Ast.Constant cst ->
          update_defs (FileInfo.pos_full cst.Ast.cst_name)
            Constant fuzzy_defs trie_defs
      | _ -> fuzzy_defs, trie_defs
    end ~init:(SS.Fuzzy.TMap.empty, []) in
    let autocomplete_defs = List.fold_left ast ~f:begin fun acc def ->
      match def with
      | Ast.Fun f ->
        add_autocomplete_term (FileInfo.pos_full f.Ast.f_name) Function acc
      | Ast.Class c -> add_autocomplete_term
          (FileInfo.pos_full c.Ast.c_name)
          (Class (Some c.Ast.c_kind)) acc
      | _ -> acc
    end ~init:[] in
    SS.WorkerApi.update fn trie_defs fuzzy_defs autocomplete_defs
end

module MasterApi = struct
  let get_type = function
    | "class" -> Some (Class (Some Ast.Cnormal))
    | "function" -> Some Function
    | "constant" -> Some Constant
    | "typedef" -> Some Typedef
    | _ -> None

  let query popt workers input type_ ~fuzzy =
    let results = SS.MasterApi.query workers input (get_type type_) ~fuzzy in
    List.filter_map results
      (fun search_term ->
        let {
          SearchUtils.name;
          pos;
          result_type;
        } = search_term in
        begin
          match pos, result_type with
          (* Need both class kind and position *)
          | FileInfo.File (FileInfo.Class, fn), Class _ ->
            (match Parser_heap.find_class_in_file popt fn name with
            | Some c ->
                let pos, result_type =
                  (fst c.Ast.c_name, Class (Some c.Ast.c_kind)) in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
            | None -> None)
          | FileInfo.File (FileInfo.Fun, fn), Function ->
            (match Parser_heap.find_fun_in_file popt fn name with
            | Some c ->
                let pos = fst c.Ast.f_name in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
            | None -> None)
          | FileInfo.File (FileInfo.Typedef, fn), Typedef ->
            (match Parser_heap.find_typedef_in_file popt fn name with
            | Some c ->
                let pos = fst c.Ast.t_id in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
            | None -> None)
          | FileInfo.File (FileInfo.Const, fn), Constant ->
            (match Parser_heap.find_const_in_file popt fn name with
            | Some c ->
                let pos = fst c.Ast.cst_name in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
            | None -> None)
          | FileInfo.Full p, Class None ->
            let fn = Pos.filename p in
            (match Parser_heap.find_class_in_file popt fn name with
            | Some c ->
                let result_type = Class (Some c.Ast.c_kind) in
                Some {
                  SearchUtils.name;
                  pos = p;
                  result_type;
                }
            | None -> None)
        | FileInfo.Full p, _ ->
          Some {
            SearchUtils.name;
            pos=p;
            result_type;
          }
        | _ -> Utils.assert_false_log_backtrace(Some "Incorrect position")
      end
      )

  let query_autocomplete input ~limit ~filter_map =
    SS.AutocompleteTrie.MasterApi.query input ~limit ~filter_map

  let clear_shared_memory =
    SS.MasterApi.clear_shared_memory

  let update_search_index ~fuzzy files =
    SS.MasterApi.update_search_index ~fuzzy files
end

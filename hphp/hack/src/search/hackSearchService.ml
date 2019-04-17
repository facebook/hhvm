(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Utils
open Reordered_argument_collections
open SearchUtils

let fuzzy = ref false

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
        let is_static = List.mem ~equal:(=) m.Ast.m_kind Ast.Static in
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
end

module MasterApi = struct
  let get_type = function
    | "class" -> Some (Class (Some Ast.Cnormal))
    | "function" -> Some Function
    | "constant" -> Some Constant
    | "typedef" -> Some Typedef
    | _ -> None

  let query workers input type_ ~fuzzy =
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
             (match Parser_heap.find_class_in_file fn name with
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
             (match Parser_heap.find_fun_in_file fn name with
              | Some c ->
                let pos = fst c.Ast.f_name in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
              | None -> None)
           | FileInfo.File (FileInfo.Typedef, fn), Typedef ->
             (match Parser_heap.find_typedef_in_file fn name with
              | Some c ->
                let pos = fst c.Ast.t_id in
                Some {
                  SearchUtils.name;
                  pos;
                  result_type;
                }
              | None -> None)
           | FileInfo.File (FileInfo.Const, fn), Constant ->
             (match Parser_heap.find_const_in_file fn name with
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
             (match Parser_heap.find_class_in_file fn name with
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

  let query_autocomplete (input: string)
      ~(limit: int option)
      ~(filter_map: string -> string -> legacy_symbol -> 'a option)
    : 'a list Utils.With_complete_flag.t =
    SS.AutocompleteTrie.MasterApi.query input ~limit ~filter_map

  let clear_shared_memory =
    SS.MasterApi.clear_shared_memory

  let update_search_index ~fuzzy files =
    SS.MasterApi.update_search_index ~fuzzy files
end

module ClassMethods = struct
  let get_class_definition_file class_name =
    let class_def = Naming_table.Types.get_pos class_name in
    match class_def with
    | Some (pos, Naming_table.TClass) ->
      let file =
        match pos with
        | FileInfo.Full pos -> Pos.filename pos
        | FileInfo.File (_, file) -> file
      in
      Some file
    | _ -> None

  let query class_name method_query =
    let open Option.Monad_infix in
    let method_query = String.lowercase method_query in
    let matches_query method_name =
      let method_name = String.lowercase method_name in
      String_utils.is_substring method_query method_name
    in
    get_class_definition_file class_name
    >>= (fun file -> Parser_heap.find_class_in_file file class_name)
    >>| (fun class_ -> class_.Ast.c_body)
    >>| List.filter_map ~f:begin fun class_elt ->
      match class_elt with
      | Ast.Method Ast.{m_kind; m_name = (pos, name); _}
        when matches_query name ->
        let is_static = List.mem ~equal:(=) m_kind Ast.Static in
        Some SearchUtils. {
            name;
            pos;
            result_type = Method (is_static, class_name)
          }
      | _ -> None
    end
    |> Option.value ~default:[]
end

(* Differentiate between two types of data sets provided via either
 * the saved state or the typechecker *)
let index_symbols_in_file (fn, info) =
  match info with
  | Full infos -> WorkerApi.update_from_fileinfo fn infos
  | Fast names -> WorkerApi.update_from_fast fn names

(* Method for importing data from the typechecker or saved-state *)
let update_from_typechecker
    (worker_list_opt: MultiWorker.worker list option)
    (files: (Relative_path.t * info) list): unit =
  if List.length files < 100
  then List.iter files index_symbols_in_file
  else
    MultiWorker.call worker_list_opt
      ~job:(fun () files -> List.iter files index_symbols_in_file)
      ~neutral:()
      ~merge:(fun _ _ -> ())
      ~next:(MultiWorker.next worker_list_opt files);
  MasterApi.update_search_index
    ~fuzzy:!fuzzy
    (List.map files fst)

(* Method for retrieving data that matches the symbolindex search *)
let index_search
    (prefix: string)
    (max_results: int)
    (kind_opt: si_kind option): si_results =

  (* What kind of kind-matching are we doing? *)
  let kind_match (res_kind: si_kind): bool  =
    if kind_opt = None then begin
      true
    end else begin
      match kind_opt with
      | None -> false
      | Some kind -> kind = res_kind
    end in

  (* Raw query *)
  let results = MasterApi.query_autocomplete prefix
      ~limit:(Some max_results)
      ~filter_map:(fun _ _ result -> begin
            let name = result.SearchUtils.name in
            let kind = result_to_kind result.SearchUtils.result_type in
            if kind_match kind then begin
              Some {
                si_kind = kind;
                si_name = name;
              }
            end else begin
              None
            end
          end) in
  results.With_complete_flag.value

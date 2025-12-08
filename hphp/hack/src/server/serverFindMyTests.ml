(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

type member = Method of string

let findrefs_member_of_member = function
  | Method m -> SearchTypes.Find_refs.Method m

let supported_test_framework_classes = SSet.of_list ["WWWTest"]

let show_pos_strip_newline pos =
  let string =
    Pos.show_pos
      (fun fmt s -> Format.pp_print_string fmt s)
      (Pos.to_relative_string pos)
  in
  String.filter string ~f:(fun c -> not (Char.equal c '\n'))

let add_ns name =
  if Char.equal name.[0] '\\' then
    name
  else
    "\\" ^ name

let strip_ns results =
  List.map results ~f:(fun r ->
      SearchTypes.Find_refs.{ r with name = Utils.strip_ns r.name })

let log_info fmt = Hh_logger.info ~category:"FindMyTests" fmt

let log_debug fmt = Hh_logger.debug ~category:"FindMyTests" fmt

let get_class_decl_entry ctx class_name =
  match Decl_provider.get_class ctx (Utils.add_ns class_name) with
  | Decl_entry.DoesNotExist ->
    failwith
      (Printf.sprintf "Internal error: Could not find class %s" class_name)
  | NotYetAvailable ->
    failwith
      (Printf.sprintf
         "Internal error: Class %s is marked as not yet available"
         class_name)
  | Found class_ -> class_

let is_test_file ctx file_path file_nast =
  let is_test_class class_name =
    let class_ = get_class_decl_entry ctx class_name in
    let ancestors = Folded_class.all_ancestor_names class_ in
    List.exists ancestors ~f:(fun ancestor ->
        SSet.mem (Utils.strip_ns ancestor) supported_test_framework_classes)
  in

  match String.substr_index file_path ~pattern:"/__tests__/" with
  | None -> false
  | Some _ ->
    let classes_in_file =
      let visitor =
        object
          inherit [_] Aast.reduce

          method! on_class_ _ctx class_ =
            SSet.singleton (Ast_defs.get_id class_.c_name)

          method! on_method_ _ctx _ = SSet.empty

          method! on_fun_ _ctx _ = SSet.empty

          method plus = SSet.union

          method zero = SSet.empty
        end
      in
      visitor#on_program () file_nast
    in
    SSet.exists is_test_class classes_in_file

let search ctx target include_defs ~files genv =
  List.iter files ~f:(fun file ->
      log_debug
        "ServerFindMyTests.search file %s"
        (Relative_path.to_absolute file));
  (* Get all the references to the provided target in the files *)
  let res =
    FindRefsService.find_references
      ctx
      genv.workers
      target
      include_defs
      files
      ~stream_file:None
  in
  strip_ns res

let search_member
    ctx (class_name : string) (member : member) (genv : genv) (env : env) :
    SearchTypes.Find_refs.t list =
  let dep_member_of member =
    let open Typing_deps.Dep.Member in
    match member with
    | Method n -> [method_ n; smethod n]
  in
  let fr_member = findrefs_member_of_member member in

  let class_name = Utils.add_ns class_name in
  let origin_class_name =
    FindRefsService.get_origin_class_name ctx class_name fr_member
  in

  let (descendant_class_files, member_use_files) =
    FindRefsService
    .get_files_for_descendants_and_dependents_of_members_in_descendants
      ctx
      ~class_name
      (dep_member_of member)
  in
  let descendant_classes =
    FindRefsService.find_child_classes_in_files
      ctx
      origin_class_name
      env.naming_table
      descendant_class_files
  in
  let class_and_descendants = SSet.add origin_class_name descendant_classes in
  let files =
    Relative_path.Set.union descendant_class_files member_use_files
    |> Relative_path.Set.elements
  in
  let target =
    FindRefsService.IMember
      (FindRefsService.Class_set class_and_descendants, fr_member)
  in
  let include_defs = false in
  search ctx target include_defs ~files genv

let search_class ctx class_name genv =
  let class_name = add_ns class_name in
  let target =
    (* We use IExplicitClass here instead of IClass.
       The difference is that this excludes references to our target using static::, self:: and parent::, which we don't care about *)
    FindRefsService.IExplicitClass class_name
  in
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton class_name)
    |> Relative_path.Set.elements
  in
  let include_defs = false in
  search ctx target include_defs ~files genv

let is_enumish_kind classish_kind =
  Ast_defs.is_c_enum classish_kind || Ast_defs.is_c_enum_class classish_kind

(* Standalone module so we can pass it to Hash_set *)
module Symbol_def = struct
  type t =
    | Method of {
        class_name: string;  (** Does not have leading \*)
        method_name: string;
      }
    | Classish of {
        name: string;  (** Does not have leading \*)
        kind: Ast_defs.classish_kind;
      }
  [@@deriving show, ord, sexp, hash]

  let class_from_name ctx class_name =
    let decl = get_class_decl_entry ctx class_name in
    let kind = Folded_class.kind decl in
    Classish { name = class_name; kind }
end

open Symbol_def

(** A symbol we encounter during our BFS traversal *)
type symbol_node = {
  symbol_def: Symbol_def.t;
  distance: int;
}
[@@deriving show]

type error_msg = string

type result_entry = ServerCommandTypes.Find_my_tests.result_entry

(**
  Turns a method symbol occurrence (i.e., method use site) into the surrounding definition,
  if we are interested in it.

  Concretely, if method C1:m1 is used at some position p, check if p is inside some method C2:m2.
  If so, return the latter. Otherwise, return None.
*)
let process_method_occurrence
    (method_occurrence_pos : Pos.t) method_occurrence_nast : Symbol_def.t option
    =
  let visitor =
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.Aast.c_span in
        if Pos.contains class_span method_occurrence_pos then
          let ctx = Some class_.Aast.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_method_ ctx m =
        let span = m.Aast.m_span in
        if Pos.contains span method_occurrence_pos then
          (* We are seeing a method, so on_class_ must have set the class_name *)
          let class_name = snd (Option.value_exn ctx) in
          let method_name = snd m.Aast.m_name in
          Some (Method { class_name; method_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  visitor#on_program None method_occurrence_nast

(**
  Turns a class symbol occurrence (i.e., class name use site) into the surrounding definition,
  if we are interested in it.

  Concretely, if class C1 is used at some position p, check if p corresponds to `C1::class`
  or `new C1(...)`.
*)
let process_class_occurrence
    class_name kind (class_occurrence_pos : Pos.t) class_occurrence_nast :
    Symbol_def.t option =
  let is_in_class_or_new_expr block =
    let is_our_class ci =
      match ci with
      | Aast.CI class_id
      | CIexpr (_, _, Id class_id) ->
        String.equal (snd class_id) class_name
      | _ -> false
    in

    (* Enums should not be handled by this function *)
    assert (not (is_enumish_kind kind));

    (object (self)
       inherit [_] Aast.reduce as super

       method! on_stmt ctx (stmt_span, stmt_) =
         Pos.contains stmt_span class_occurrence_pos && self#on_stmt_ ctx stmt_

       method! on_expr_ ctx expr_ =
         (* TODO(frankemrich) This is pretty brittle and just handles the common cases.
             If we do this using TASTs instead of NASTs we can just inspect the types *)
         match expr_ with
         | New ((_, _, ci), _, _, _, _) -> is_our_class ci
         | Class_const ((_, _, ci), const_name) ->
           is_our_class ci && String.equal (snd const_name) "class"
         | _ -> super#on_expr_ ctx expr_

       method zero = false

       method plus = ( || )
    end)
      #on_block
      ()
      block
  in
  let method_visitor =
    let open Aast in
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.Aast.c_span in
        if Pos.contains class_span class_occurrence_pos then
          let ctx = Some class_.Aast.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_method_ ctx m =
        let span = m.m_span in

        let should_include_method method_body_block =
          if is_enumish_kind kind then
            (* If we are looking at an enum, we are very permissive:
               Any method that mentions the enum type will be included *)
            true
          else
            is_in_class_or_new_expr method_body_block
        in
        let body_block = m.m_body.fb_ast in
        if
          Pos.contains span class_occurrence_pos
          && should_include_method body_block
        then
          (* We are seeing a method, so on_class_ must have set the class_name *)
          let class_name = snd (Option.value_exn ctx) in
          let method_name = snd m.Aast.m_name in
          Some (Method { class_name; method_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  method_visitor#on_program None class_occurrence_nast

(**
  Find references to the given symbol.
  If the reference is from a test file, it's returned as part of the second list.
  Otherwise, the definition from where the reference happens is returned as part of the first list. *)
let get_method_references
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~class_name
    ~method_name : (Symbol_def.t list * string list, error_msg) Result.t =
  let search_results =
    search_member ctx class_name (Method method_name) genv env
  in
  let resolve_found_position
      (acc_symbol_defs, acc_files)
      SearchTypes.Find_refs.{ name = _; pos = referencing_pos } =
    let relative_path = Pos.filename referencing_pos in
    let path = Relative_path.suffix relative_path in

    log_debug
      "get_method_references: Resolving symbol occurence %s of method %s::%s"
      (show_pos_strip_newline referencing_pos)
      class_name
      method_name;

    (* We could implement all of our traversals without NASTs, but just CSTs and their
       Full_fidelity_positioned_syntax.parentage function.

       But those never seem to be cached, whereas the NAST may still be in cache after we just
       constructed the TAST in FindRefsService. *)
    let nast = Ast_provider.get_ast ~full:true ctx relative_path in

    if is_test_file ctx path nast then (
      log_debug "get_method_references: Reference is from a test file: %s" path;
      Result.Ok
        ( acc_symbol_defs,
          (Pos.to_absolute referencing_pos |> Pos.filename) :: acc_files )
    ) else
      match process_method_occurrence referencing_pos nast with
      | Some referencing_def ->
        log_debug
          "get_method_references: found referencing def %s"
          (Symbol_def.show referencing_def);
        Result.Ok (referencing_def :: acc_symbol_defs, acc_files)
      | None ->
        Result.Error
          (Printf.sprintf
             "Could not find definition enclosing %s"
             (show_pos_strip_newline referencing_pos))
  in
  List.fold_result search_results ~init:([], []) ~f:resolve_found_position

let get_class_references
    ~(ctx : Provider_context.t) ~(genv : ServerEnv.genv) ~name ~kind :
    (Symbol_def.t list * string list, error_msg) Result.t =
  let search_results = search_class ctx name genv in
  let resolve_found_position
      (acc_symbol_defs, acc_files)
      SearchTypes.Find_refs.{ name = _; pos = referencing_pos } =
    let relative_path = Pos.filename referencing_pos in
    let path = Relative_path.suffix relative_path in

    log_debug
      "get_class_references: Resolving symbol occurence %s"
      (show_pos_strip_newline referencing_pos);

    (* Same comment here regarding using NASTs to benefit from caching *)
    let nast = Ast_provider.get_ast ~full:true ctx relative_path in

    if is_test_file ctx path nast then (
      log_debug "get_class_references: Reference is from a test file: %s" path;
      Result.Ok
        ( acc_symbol_defs,
          (Pos.to_absolute referencing_pos |> Pos.filename) :: acc_files )
    ) else
      match process_class_occurrence name kind referencing_pos nast with
      | Some referencing_def ->
        log_debug
          "get_class_references: found referencing def %s"
          (Symbol_def.show referencing_def);
        Result.Ok (referencing_def :: acc_symbol_defs, acc_files)
      | None ->
        log_info
          "get_class_references: Could not find surrounding definition for use site %s of class %s, or it's not suitable"
          (show_pos_strip_newline referencing_pos)
          name;
        Result.Ok (acc_symbol_defs, acc_files)
  in
  List.fold_result search_results ~init:([], []) ~f:resolve_found_position

let get_references
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    symbol : (Symbol_def.t list * string list, error_msg) Result.t =
  match symbol with
  | Method { class_name; method_name } ->
    get_method_references ~ctx ~genv ~env ~class_name ~method_name
  | Classish { name; kind } -> get_class_references ~ctx ~genv ~name ~kind

let is_root_symbol_in_test_file ctx symbol_def =
  let check_class class_name =
    match Naming_provider.get_class_path ctx (Utils.add_ns class_name) with
    | None ->
      failwith
        (Printf.sprintf
           "Could not resolve class %s, even though it was given as part of a root"
           class_name)
    | Some path ->
      let nast = Ast_provider.get_ast ~full:true ctx path in
      let relative_path = Relative_path.suffix path in
      if is_test_file ctx relative_path nast then
        let absolute_path = Relative_path.to_absolute path in
        Some absolute_path
      else
        None
  in
  match symbol_def with
  | Method { class_name; method_name = _ } -> check_class class_name
  | Classish { name; kind = _ } -> check_class name

let search
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    max_distance
    (roots : Symbol_def.t list) : result_entry list =
  let queue = Queue.create () in
  let seen_symbols = Hash_set.create (module Symbol_def) in
  (* Maps test file paths to the minimum distance at which we have discovered them *)
  let test_files = Hashtbl.create (module String) in

  List.iter roots ~f:(fun root_symbol ->
      let seen =
        Result.is_error (Hash_set.strict_add seen_symbols root_symbol)
      in
      match is_root_symbol_in_test_file ctx root_symbol with
      | Some test_file -> ignore (Hashtbl.add test_files ~key:test_file ~data:0)
      | None ->
        if not seen then (
          log_debug "Adding root %s to queue" (Symbol_def.show root_symbol);
          Queue.enqueue queue { distance = 0; symbol_def = root_symbol }
        ));

  (* For each root, we add a node for the enclosing class.
     TODO(frankemrich): In the future we may want to consider doing this for any method we
     encounter, not just root methods *)
  List.iter roots ~f:(fun root_symbol ->
      match root_symbol with
      | Method { class_name; _ } ->
        let class_symbol = Symbol_def.class_from_name ctx class_name in
        let seen =
          Result.is_error (Hash_set.strict_add seen_symbols class_symbol)
        in
        (* We consider going from root (distance 0) to the enclosing class as a step *)
        let distance = 1 in
        if not seen then (
          log_debug
            "Adding enclosing class to queue for root %s"
            (Symbol_def.show root_symbol);
          Queue.enqueue queue { distance; symbol_def = class_symbol }
        )
      | _ -> ());

  let rec bfs () =
    match Queue.dequeue queue with
    | Some ({ distance; symbol_def } as node) when distance < max_distance ->
      log_info "processing node %s" (show_symbol_node node);
      let new_distance = distance + 1 in
      (match get_references ~ctx ~genv ~env symbol_def with
      | Result.Ok (referencing_defs, referencing_test_files) ->
        List.iter referencing_defs ~f:(fun referencing_def ->
            match Hash_set.strict_add seen_symbols referencing_def with
            | Result.Ok () ->
              let new_node =
                { distance = new_distance; symbol_def = referencing_def }
              in
              log_info "adding node %s to queue" (show_symbol_node new_node);
              Queue.enqueue queue new_node
            | Result.Error _ -> (* Already seen, nothing to do *) ());
        List.iter referencing_test_files ~f:(fun referencing_test_file ->
            match
              Hashtbl.add
                test_files
                ~key:referencing_test_file
                ~data:new_distance
            with
            | `Duplicate -> (* Already seen, nothing to do *) ()
            | `Ok -> log_info "adding new test %s" referencing_test_file);
        bfs ()
      | Result.Error msg ->
        log_info
          "Getting references for symbol %s failed: %s"
          (Symbol_def.show symbol_def)
          msg)
    | _ -> ()
  in

  bfs ();

  Hashtbl.fold test_files ~init:[] ~f:(fun ~key ~data acc ->
      let open ServerCommandTypes.Find_my_tests in
      { file_path = key; distance = data } :: acc)

let go
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~(max_distance : int)
    (actions : ServerCommandTypes.Find_my_tests.action list) :
    ServerCommandTypes.Find_my_tests.result =
  let open Result.Let_syntax in
  (match env.prechecked_files with
  | ServerEnv.Prechecked_files_disabled -> ()
  | _ ->
    (* Prechecked files influence what dependencies we are aware of, (see ServerFindRefs)
       We just choose not to support them, as they are being deprecated *)
    failwith
      "FindMyTests not supported by servers with prechecked optimisation enabled");

  let* roots =
    List.fold_result actions ~init:[] ~f:(fun acc action ->
        let root =
          match action with
          | Class { class_name } ->
            Symbol_def.class_from_name ctx (Utils.strip_ns class_name)
          | Method { class_name; member_name } ->
            Method
              {
                class_name = Utils.strip_ns class_name;
                method_name = member_name;
              }
        in

        Result.Ok (root :: acc))
  in
  Result.Ok (search ~ctx ~genv ~env max_distance roots)

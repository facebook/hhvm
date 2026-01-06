(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

[@@@alert "-dependencies"]
(* TODO: either use fanout-aware functions from typing env
 * or update this comment to say why we don't need them
 *)

type member =
  | Method of string
  | Typeconst of string

let findrefs_member_of_member = function
  | Method m -> SearchTypes.Find_refs.Method m
  | Typeconst t -> SearchTypes.Find_refs.Typeconst t

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
    | Typeconst t -> [const t]
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

let search_type ctx type_name genv =
  let type_name = add_ns type_name in
  let target =
    (* We use IExplicitClass here instead of IClass.
       The difference is that this excludes references to our target using static::, self:: and parent::, which we don't care about.
       Despite being named "class", this also works for typedefs *)
    FindRefsService.IExplicitClass type_name
  in
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton type_name)
    |> Relative_path.Set.elements
  in
  let include_defs = false in
  search ctx target include_defs ~files genv

let is_enumish_kind classish_kind =
  Ast_defs.is_c_enum classish_kind || Ast_defs.is_c_enum_class classish_kind

(* Standalone module so we can pass it to Hash_set *)
module Symbol_def = struct
  type member = {
    class_name: string;  (** Does not have leading \*)
    member_name: string;
  }
  [@@deriving show, ord, sexp, hash]

  type t =
    | Method of member
    | Typeconst of member
    | Classish of {
        name: string;  (** Does not have leading \*)
        kind: Ast_defs.classish_kind;
      }
    | Typedef of { name: string  (** Does not have leading \ *) }
        (** We do not categorize typedefs as classish because there is no corresponding Ast_defs.classish_kind for typedefs.
          This is different from ServerFindRefs, which uses the "Class" action for typedefs. *)
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
    ~referenced_name:_ ~referencing_pos ~referencing_nast : Symbol_def.t option
    =
  let visitor =
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.Aast.c_span in
        if Pos.contains class_span referencing_pos then
          let ctx = Some class_.Aast.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_method_ ctx m =
        let span = m.Aast.m_span in
        if Pos.contains span referencing_pos then
          (* We are seeing a method, so on_class_ must have set the class_name *)
          let class_name = snd (Option.value_exn ctx) in
          let method_name = snd m.Aast.m_name in
          Some (Method { class_name; member_name = method_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  visitor#on_program None referencing_nast

(**
  Turns a class symbol occurrence (i.e., class name use site) into the surrounding definition,
  if we are interested in it.

  Concretely, if class C1 is used at some position p, check if p corresponds to `C1::class`
  or `new C1(...)`.
*)
let process_class_occurrence
    ~kind ~referenced_name:class_name ~referencing_pos ~referencing_nast :
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
         Pos.contains stmt_span referencing_pos && self#on_stmt_ ctx stmt_

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
        if Pos.contains class_span referencing_pos then
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
        if Pos.contains span referencing_pos && should_include_method body_block
        then
          (* We are seeing a method, so on_class_ must have set the class_name *)
          let class_name = snd (Option.value_exn ctx) in
          let method_name = snd m.Aast.m_name in
          Some (Method { class_name; member_name = method_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  method_visitor#on_program None referencing_nast

let is_class_id_for class_id_ type_name =
  match class_id_ with
  | Aast.CI class_id
  | CIexpr (_, _, Id class_id) ->
    String.equal (snd class_id) type_name
  | _ -> false

let is_call_of_function expr_ function_name =
  match expr_ with
  | Aast.Call { func = (_, _, Aast.Id (_, name)); _ } ->
    String.equal name function_name
  | _ -> false

let is_nameof_for expr_ type_name =
  match expr_ with
  | Aast.Nameof (_, _, class_id_) -> is_class_id_for class_id_ type_name
  | _ -> false

(* Checks if this is TypeName::class *)
let is_class_const_for expr_ type_name =
  match expr_ with
  | Aast.Class_const ((_, _, ci), const_name) ->
    (match ci with
    | Aast.CI class_id
    | CIexpr (_, _, Id class_id) ->
      String.equal (snd class_id) type_name
      && String.equal (snd const_name) "class"
    | _ -> false)
  | _ -> false

(* Checks if this is type_structure(TypeName::class, 'typeconst_name') *)
let is_type_structure_for expr_ type_name typeconst_name =
  match expr_ with
  | Aast.Call { func = _; args = [Aast.Anormal arg1; Aast.Anormal arg2]; _ }
    when is_call_of_function expr_ "type_structure" ->
    let ((), _, arg1_) = arg1 in
    let ((), arg2_pos, _) = arg2 in
    is_class_const_for arg1_ type_name
    && Nast.equal_expr arg2 ((), arg2_pos, Aast.String typeconst_name)
  | _ -> false

(** Turns an occurrence of a typedef Ty into the surrounding definition,
  if we are interested in it.

  Concretely, if Ty is used at some position p, check if
  - if p is inside some other typedef or typeconst definition.
  - if p is inside a method we look for
    + Ty::class
    + nameof Ty
   *)
let process_typedef_occurrence
    ~referenced_name ~referencing_pos ~referencing_nast : Symbol_def.t option =
  let block_has_relevant_reference block =
    (object (self)
       inherit [_] Aast.reduce as super

       method! on_stmt ctx (stmt_span, stmt_) =
         Pos.contains stmt_span referencing_pos && self#on_stmt_ ctx stmt_

       method! on_expr ctx expr =
         let (_, expr_span, expr_) = expr in
         if Pos.contains expr_span referencing_pos then
           (* Ty::class *)
           is_class_const_for expr_ referenced_name
           (* nameof Ty *)
           || is_nameof_for expr_ referenced_name
           || super#on_expr ctx expr
         else
           false

       method zero = false

       method plus = ( || )
    end)
      #on_block
      ()
      block
  in
  let visitor =
    let open Aast in
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.c_span in
        if Pos.contains class_span referencing_pos then
          let ctx = Some class_.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_typedef _ctx td =
        let span = td.t_span in
        if Pos.contains span referencing_pos then
          Some (Symbol_def.Typedef { name = Utils.strip_ns (snd td.t_name) })
        else
          None

      method! on_method_ ctx m =
        let span = m.m_span in
        if Pos.contains span referencing_pos then
          (* Check if the reference is in a form we care about *)
          let body_block = m.m_body.fb_ast in
          if block_has_relevant_reference body_block then
            (* We are seeing a method, so on_class_ must have set the class_name *)
            let class_name = snd (Option.value_exn ctx) in
            let method_name = snd m.m_name in
            Some (Symbol_def.Method { class_name; member_name = method_name })
          else
            None
        else
          None

      method! on_class_typeconst_def ctx tc =
        let span = tc.c_tconst_span in
        if Pos.contains span referencing_pos then
          (* We are seeing a typeconst, so on_class_ must have set the class_name *)
          let class_name = Utils.strip_ns (snd (Option.value_exn ctx)) in
          let typeconst_name = snd tc.c_tconst_name in
          Some
            (Symbol_def.Typeconst { class_name; member_name = typeconst_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  visitor#on_program None referencing_nast

(** Turns an occurrence of a typeconst Foo::TBar into the surrounding definition,
  if we are interested in it.

  Concretely, if Foo::Bar is used at some position p, check if
  - if p is inside some other typedef or typeconst definition.


  Note that we have special logic elsewhere that looks for
  type_structure(Foo::class, 'TBar')

   *)
let process_typeconst_occurrence
    ~referenced_name:_ ~referencing_pos ~referencing_nast : Symbol_def.t option
    =
  let visitor =
    let open Aast in
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.c_span in
        if Pos.contains class_span referencing_pos then
          let ctx = Some class_.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_typedef _ctx _td =
        (* Hack does not allow aliasing a typeconst! *)
        None

      method! on_class_typeconst_def ctx tc =
        let span = tc.c_tconst_span in
        if Pos.contains span referencing_pos then
          (* We are seeing a typeconst, so on_class_ must have set the class_name *)
          let class_name = Utils.strip_ns (snd (Option.value_exn ctx)) in
          let typeconst_name = snd tc.c_tconst_name in
          Some
            (Symbol_def.Typeconst { class_name; member_name = typeconst_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  visitor#on_program None referencing_nast

(**
  Handles the most common case:
  Our `target_symbol` is referenced by a bunch of `search_results`.
  For each of these referencing positions, we check if they are from a test file.
  If so, we select the file immediately.

  Otherwise, we apply `on_non_test_file_reference` to check if the reference is from a symbol where
  we should continue searching. *)
let get_references_standard
    ctx target_symbol search_results on_non_test_file_reference =
  let resolve_found_position
      (acc_symbol_defs, acc_files)
      SearchTypes.Find_refs.{ name = referenced_name; pos = referencing_pos } =
    let relative_path = Pos.filename referencing_pos in
    let path = Relative_path.suffix relative_path in

    log_debug
      "get_references_standard: Resolving symbol occurence %s"
      (show_pos_strip_newline referencing_pos);

    (* We could implement all of our traversals without NASTs, but just CSTs and their
       Full_fidelity_positioned_syntax.parentage function. *)
    let referencing_nast = Ast_provider.get_ast ~full:true ctx relative_path in

    if is_test_file ctx path referencing_nast then (
      log_debug
        "get_references_standard: Reference is from a test file: %s"
        path;
      Result.Ok
        ( acc_symbol_defs,
          (Pos.to_absolute referencing_pos |> Pos.filename) :: acc_files )
    ) else
      match
        on_non_test_file_reference
          ~referenced_name
          ~referencing_pos
          ~referencing_nast
      with
      | Some referencing_def ->
        log_debug
          "get_references_standard: found referencing def %s"
          (Symbol_def.show referencing_def);
        Result.Ok (referencing_def :: acc_symbol_defs, acc_files)
      | None ->
        log_info
          "get_references_standard: Could not find surrounding definition for use site %s of symbol %s, or it's not suitable"
          (show_pos_strip_newline referencing_pos)
          (Symbol_def.show target_symbol);
        Result.Ok (acc_symbol_defs, acc_files)
  in
  List.fold_result search_results ~init:([], []) ~f:resolve_found_position

(** For type constants `Foo::TBar`, we need to do something slightly special:
  - We are particularly interested in `type_structure(Foo::class, 'TBar')`
    + In order to find these, we need to find all references to Foo,
      not just those to Foo::TBar.
  - We don't want to use the logic in `get_references_standard` that would blindly accept any
    reference from a test file to `Foo`.

  Therefore, we
  - Look search all references to `Foo` and check if they are actually a `type_structure` call for
    `Foo::TBar`
  - We search all references to `Foo::TBar` to do similar checks as process_typedef_occurrence:
    + Is it referenced from another typedef or typeconst?
    + Is it referenced from a test file?
   *)
let get_typeconst_references
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    class_name
    member_name =
  let open Result.Let_syntax in
  let typeconst_search_result =
    search_member ctx class_name (Typeconst member_name) genv env
  in
  let typeconst_symbol = Typeconst { class_name; member_name } in
  let* (typeconst_using_symbols, typeconst_using_test_files) =
    get_references_standard
      ctx
      typeconst_symbol
      typeconst_search_result
      process_typeconst_occurrence
  in

  let block_has_type_structure_for_typeconst referencing_pos block =
    (object (self)
       inherit [_] Aast.reduce as super

       method! on_stmt ctx (stmt_span, stmt_) =
         Pos.contains stmt_span referencing_pos && self#on_stmt_ ctx stmt_

       method! on_expr ctx expr =
         let (_, expr_span, expr_) = expr in
         if Pos.contains expr_span referencing_pos then
           is_type_structure_for expr_ class_name member_name
           || super#on_expr ctx expr
         else
           false

       method zero = false

       method plus = ( || )
    end)
      #on_block
      ()
      block
  in

  (* Helper to check if a reference to `class_name` is actually a type_structure call for our
     typeconst *)
  let get_symbols_with_type_structure_for_typeconst
      ~referencing_pos ~referencing_nast =
    let visitor =
      let open Aast in
      object
        inherit [_] Aast.reduce as super

        method! on_class_ _ctx class_ =
          let class_span = class_.c_span in
          if Pos.contains class_span referencing_pos then
            let ctx = Some class_.c_name in
            super#on_class_ ctx class_
          else
            None

        method! on_method_ ctx m =
          let span = m.m_span in
          if Pos.contains span referencing_pos then
            (* Check if the reference is in a form we care about *)
            let body_block = m.m_body.fb_ast in
            if block_has_type_structure_for_typeconst referencing_pos body_block
            then
              (* We are seeing a method, so on_class_ must have set the class_name *)
              let class_name = snd (Option.value_exn ctx) in
              let method_name = snd m.m_name in
              Some (Symbol_def.Method { class_name; member_name = method_name })
            else
              None
          else
            None

        method zero = None

        method plus r1 r2 =
          match (r1, r2) with
          | (None, _) -> r2
          | (Some _, _) -> r1
      end
    in
    visitor#on_program None referencing_nast
  in

  (* References to the class itself, used to find type_structure calls *)
  let class_search_results = search_type ctx class_name genv in

  (* We add the type structure usages to our previous results *)
  let (symbols, test_files) =
    List.fold_left
      class_search_results
      ~init:(typeconst_using_symbols, typeconst_using_test_files)
      ~f:(fun (acc_symbol_defs, acc_files) { name = _; pos = referencing_pos }
         ->
        let relative_path = Pos.filename referencing_pos in
        let path = Relative_path.suffix relative_path in

        log_debug
          "get_typeconst_references: Resolving symbol occurence %s"
          (show_pos_strip_newline referencing_pos);

        (* We could implement all of our traversals without NASTs, but just CSTs and their
           Full_fidelity_positioned_syntax.parentage function. *)
        let referencing_nast =
          Ast_provider.get_ast ~full:true ctx relative_path
        in

        match
          get_symbols_with_type_structure_for_typeconst
            ~referencing_pos
            ~referencing_nast
        with
        | Some type_structure_referencing_symbol ->
          if is_test_file ctx path referencing_nast then (
            log_debug
              "get_typeconst_references: Reference is from a test file: %s"
              path;
            ( acc_symbol_defs,
              (Pos.to_absolute referencing_pos |> Pos.filename) :: acc_files )
          ) else
            (type_structure_referencing_symbol :: acc_symbol_defs, acc_files)
        | None -> (acc_symbol_defs, acc_files))
  in

  Result.Ok (symbols, test_files)

let get_references
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    symbol : (Symbol_def.t list * string list, error_msg) Result.t =
  match symbol with
  | Method { class_name; member_name } ->
    let search_results =
      search_member ctx class_name (Method member_name) genv env
    in
    get_references_standard ctx symbol search_results process_method_occurrence
  | Classish { name; kind } ->
    let search_results = search_type ctx name genv in
    get_references_standard
      ctx
      symbol
      search_results
      (process_class_occurrence ~kind)
  | Typedef { name } ->
    let search_results = search_type ctx name genv in
    get_references_standard ctx symbol search_results process_typedef_occurrence
  | Typeconst { class_name; member_name } ->
    get_typeconst_references ~ctx ~genv ~env class_name member_name

let is_root_symbol_in_test_file ctx symbol_def =
  let type_name =
    match symbol_def with
    | Method { class_name; member_name = _ }
    | Typeconst { class_name; member_name = _ } ->
      class_name
    | Classish { name; kind = _ }
    | Typedef { name } ->
      name
  in
  match Naming_provider.get_type_path ctx (Utils.add_ns type_name) with
  | None ->
    failwith
      (Printf.sprintf
         "Could not resolve type %s, even though it was given as part of a root"
         type_name)
  | Some path ->
    let nast = Ast_provider.get_ast ~full:true ctx path in
    let relative_path = Relative_path.suffix path in
    if is_test_file ctx relative_path nast then
      let absolute_path = Relative_path.to_absolute path in
      Some absolute_path
    else
      None

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
            Method { class_name = Utils.strip_ns class_name; member_name }
          | Typedef { name } -> Typedef { name = Utils.strip_ns name }
          | Typeconst { class_name; member_name } ->
            Typeconst { class_name = Utils.strip_ns class_name; member_name }
        in

        Result.Ok (root :: acc))
  in
  Result.Ok (search ~ctx ~genv ~env max_distance roots)

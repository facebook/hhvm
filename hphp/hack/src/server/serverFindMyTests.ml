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

(** Reasons tell us why we are considering a particular symbol.
    They effectively act as labels on the edges in our graph, used for provenance tracking. *)
module Reason = struct
  (** We are adding the method in question to our graph because ... *)
  type method_ =
    | MRoot  (** ... it's a root *)
    | MRefMethod
        (** ... it references (e.g., calls) another method in our graph *)
    | MTypedefClassPtr
        (** ... it does Ty::class, where Ty is a typedef in our graph. *)
    | MTypedefNameof
        (** ... it does nameof Ty, where Ty is a typedef in our graph. *)
    | MClassClassPtr
        (** ... it does C::class, where C is a class in our graph. *)
    | MTypeconstClassPtr
        (** ... it does Ty::TBar::class, where Ty::TBar is a type constant in our graph. *)
    | MTypeconstStructure
        (** ... it does type_structure(Ty::class, 'TBar'), where Ty::TBar is a type constant in our graph. *)
    | MRefEnumType  (** ... it references some enum type in our graph *)
    | MNewExpr  (** It does new T(...), where T is a type in our graph *)
  [@@deriving show, ord, sexp, hash]

  (** We are adding the typedef in question to our graph because ... *)
  type typedef =
    | TdRoot  (** ... it's a root *)
    | TdRefTypedef
        (** ... its definition references another typedef in our graph *)
  [@@deriving show, ord, sexp, hash]

  (** We are adding the type constant in question to our graph because ... *)
  type typeconst =
    | TcRoot  (** ... it's a root *)
    | TcRefTypeconst
        (** ... its definition references another typeconst in our graph *)
    | TcRefTypedef  (** ... its definition references a typedef in our graph *)
  [@@deriving show, ord, sexp, hash]

  (** We are adding the class-ish definition to our graph because ... *)
  type classish =
    | CRoot  (** it's a root *)
    | CEnclosingRootMethod
      (* it's the class containing a method given as a root *)
  [@@deriving show, ord, sexp, hash]

  type t =
    | Method of method_
    | Typeconst of typeconst
    | Typedef of typedef
    | Classish of classish
  [@@deriving show, ord, sexp, hash]
end

(** We associate an "expansion strategy" with each symbol in our graph.
    It determines how to construct the successors for a node with that symbol in the graph.

    We can view `Expansion_strategy.t` as a coarser version of `Reason.t`:
    - There's a surjective function from the latter to the former
    - In our graph, a node for a symbol with a given expansion strategy `st` can have
      incoming edges annotated with different reasons `r`, as long as they all map to the
      same `st`.

  An alternative view is that `Expansion_strategy` defines an equivalence relation on `Reason.t`:
  If we encounter the same symbol `s` for two different reasons `r1` and `r2` such that
  `of_reason r1 == of_reason r2`, then they can share a node in our graph because they will have
  the same successor nodes.
*)
module Expansion_strategy = struct
  type method_ = MDefault [@@deriving show, ord, sexp, hash]

  type typedef = TdDefault [@@deriving show, ord, sexp, hash]

  type typeconst = TcDefault [@@deriving show, ord, sexp, hash]

  type classish = CDefault [@@deriving show, ord, sexp, hash]

  type t =
    | Method of method_
    | Typeconst of typeconst
    | Typedef of typedef
    | Classish of classish
  [@@deriving show, ord, sexp, hash]

  let of_reason = function
    | Reason.Method MRoot -> Method MDefault
    | Reason.Method MRefMethod -> Method MDefault
    | Reason.Method MTypedefClassPtr -> Method MDefault
    | Reason.Method MClassClassPtr -> Method MDefault
    | Reason.Method MTypeconstClassPtr -> Method MDefault
    | Reason.Method MTypedefNameof -> Method MDefault
    | Reason.Method MRefEnumType -> Method MDefault
    | Reason.Method MTypeconstStructure -> Method MDefault
    | Reason.Method MNewExpr -> Method MDefault
    | Reason.Typeconst TcRoot -> Typeconst TcDefault
    | Reason.Typeconst TcRefTypeconst -> Typeconst TcDefault
    | Reason.Typeconst TcRefTypedef -> Typeconst TcDefault
    | Reason.Typedef TdRoot -> Typedef TdDefault
    | Reason.Typedef TdRefTypedef -> Typedef TdDefault
    | Reason.Classish CRoot -> Classish CDefault
    | Reason.Classish CEnclosingRootMethod -> Classish CDefault
end

(* Standalone module so we can pass it to Hash_set *)
module Symbol_def = struct
  type member = {
    class_name: string;  (** Does not have leading \*)
    member_name: string;
  }
  [@@deriving show, ord, sexp, hash]

  (** We do not categorize typedefs as classish because there is no corresponding Ast_defs.classish_kind for typedefs.
      This is different from ServerFindRefs, which uses the "Class" action for typedefs. *)
  type t =
    | Method of member
    | Typeconst of member
    | Classish of {
        name: string;  (** Does not have leading \*)
        kind: Ast_defs.classish_kind;
      }
    | Typedef of { name: string  (** Does not have leading \ *) }
  [@@deriving show, ord, sexp, hash]

  type with_strategy = {
    symbol_def: t;
    strategy: Expansion_strategy.t;
        (** Invariant: The symbol's  constructor name matches that of the strategy:
            (e.g., a Symbol_def.Method is matched with a Expansion_strategy.Method) *)
  }
  [@@deriving show, ord, sexp, hash]

  type with_reason = {
    symbol_def: t;
    reason: Reason.t;
        (** Invariant: The symbol's  constructor name matches that of the reason:
            (e.g., a Symbol_def.Method is matched with a Reason.Method) *)
  }
  [@@deriving show, sexp, hash, ord]

  let class_from_name ctx class_name reason : with_strategy =
    let decl = get_class_decl_entry ctx class_name in
    let kind = Folded_class.kind decl in
    let strategy = Expansion_strategy.of_reason reason in
    { symbol_def = Classish { name = class_name; kind }; strategy }

  (* Just here for Hash_set *)
  module With_reason = struct
    type t = with_reason [@@deriving sexp, hash, ord]
  end
end

open Symbol_def

type error_msg = string

type result_entry = ServerCommandTypes.Find_my_tests.result_entry

(**
  Turns a method symbol occurrence (i.e., method use site) into the surrounding definition,
  if we are interested in it.

  Concretely, if method C1:m1 is used at some position p, check if p is inside some method C2:m2.
  If so, return the latter. Otherwise, return None.
*)
let process_method_occurrence
    ~referenced_name:_ ~referencing_pos ~referencing_nast :
    Symbol_def.with_reason option =
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
          let member = { class_name; member_name = method_name } in
          let reason = Reason.Method Reason.MRefMethod in
          Some { symbol_def = Method member; reason }
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
    Symbol_def.with_reason option =
  let reason_if_class_or_new_expr block =
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
         if Pos.contains stmt_span referencing_pos then
           self#on_stmt_ ctx stmt_
         else
           None

       method! on_expr_ ctx expr_ =
         (* TODO(frankemrich) This is pretty brittle and just handles the common cases.
             If we do this using TASTs instead of NASTs we can just inspect the types *)
         match expr_ with
         | New ((_, _, ci), _, _, _, _) ->
           if is_our_class ci then
             Some Reason.MNewExpr
           else
             None
         | Class_const ((_, _, ci), const_name) ->
           if is_our_class ci && String.equal (snd const_name) "class" then
             Some Reason.MClassClassPtr
           else
             None
         | _ -> super#on_expr_ ctx expr_

       method zero = None

       method plus r1 r2 =
         match (r1, r2) with
         | (None, _) -> r2
         | (Some _, _) -> r1
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

        let reason_for method_body_block =
          if is_enumish_kind kind then
            (* If we are looking at an enum, we are very permissive:
               Any method that mentions the enum type will be included *)
            Some Reason.MRefEnumType
          else
            reason_if_class_or_new_expr method_body_block
        in
        let body_block = m.m_body.fb_ast in
        if Pos.contains span referencing_pos then
          match reason_for body_block with
          | Some reason ->
            (* We are seeing a method, so on_class_ must have set the class_name *)
            let class_name = snd (Option.value_exn ctx) in
            let method_name = snd m.Aast.m_name in
            let member = { class_name; member_name = method_name } in
            let reason = Reason.Method reason in
            Some { symbol_def = Method member; reason }
          | None -> None
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
    ~referenced_name ~referencing_pos ~referencing_nast :
    Symbol_def.with_reason option =
  let reason_if_block_has_relevant_reference block =
    (object (self)
       inherit [_] Aast.reduce as super

       method! on_stmt ctx (stmt_span, stmt_) =
         if Pos.contains stmt_span referencing_pos then
           self#on_stmt_ ctx stmt_
         else
           None

       method! on_expr ctx expr =
         let (_, expr_span, expr_) = expr in
         if Pos.contains expr_span referencing_pos then
           (* Ty::class *)
           if is_class_const_for expr_ referenced_name then
             Some Reason.MTypedefClassPtr
           else if (* nameof Ty *)
                   is_nameof_for expr_ referenced_name then
             Some Reason.MTypedefNameof
           else
             super#on_expr ctx expr
         else
           None

       method zero = None

       method plus r1 r2 =
         match (r1, r2) with
         | (None, _) -> r2
         | (Some _, _) -> r1
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
          let reason = Reason.Typedef Reason.TdRefTypedef in
          Some
            {
              symbol_def =
                Symbol_def.Typedef { name = Utils.strip_ns (snd td.t_name) };
              reason;
            }
        else
          None

      method! on_method_ ctx m =
        let span = m.m_span in
        if Pos.contains span referencing_pos then
          (* Check if the reference is in a form we care about *)
          let body_block = m.m_body.fb_ast in
          Option.map
            (reason_if_block_has_relevant_reference body_block)
            ~f:(fun reason ->
              (* We are seeing a method, so on_class_ must have set the class_name *)
              let class_name = snd (Option.value_exn ctx) in
              let method_name = snd m.m_name in
              let member = { class_name; member_name = method_name } in
              let reason = Reason.Method reason in
              { symbol_def = Symbol_def.Method member; reason })
        else
          None

      method! on_class_typeconst_def ctx tc =
        let span = tc.c_tconst_span in
        if Pos.contains span referencing_pos then
          (* We are seeing a typeconst, so on_class_ must have set the class_name *)
          let class_name = Utils.strip_ns (snd (Option.value_exn ctx)) in
          let typeconst_name = snd tc.c_tconst_name in
          let member = { class_name; member_name = typeconst_name } in
          let reason = Reason.Typeconst Reason.TcRefTypedef in
          Some { symbol_def = Symbol_def.Typeconst member; reason }
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
    ~referenced_name:_ ~referencing_pos ~referencing_nast :
    Symbol_def.with_reason option =
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
          let member = { class_name; member_name = typeconst_name } in
          let reason = Reason.Typeconst Reason.TcRefTypeconst in
          Some { symbol_def = Symbol_def.Typeconst member; reason }
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
let get_successors_standard
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
      | Some referencing_def_r ->
        log_debug
          "get_references_standard: found referencing def %s"
          (Symbol_def.show_with_reason referencing_def_r);
        Result.Ok (referencing_def_r :: acc_symbol_defs, acc_files)
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
let get_typeconst_successors
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    class_name
    member_name
    _strategy =
  let open Result.Let_syntax in
  let typeconst_search_result =
    search_member ctx class_name (Typeconst member_name) genv env
  in
  let typeconst_symbol = Typeconst { class_name; member_name } in

  let* (typeconst_using_symbols, typeconst_using_test_files) =
    get_successors_standard
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
              let member = { class_name; member_name = method_name } in
              let reason = Reason.Method MTypeconstStructure in
              Some { symbol_def = Symbol_def.Method member; reason }
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

let get_successors
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    (symbol : Symbol_def.with_strategy) :
    (Symbol_def.with_reason list * string list, error_msg) Result.t =
  match (symbol.symbol_def, symbol.strategy) with
  | (Method { class_name; member_name }, Expansion_strategy.Method MDefault) ->
    let search_results =
      search_member ctx class_name (Method member_name) genv env
    in
    get_successors_standard
      ctx
      symbol.symbol_def
      search_results
      process_method_occurrence
  | (Classish { name; kind }, Expansion_strategy.Classish _) ->
    let search_results = search_type ctx name genv in
    get_successors_standard
      ctx
      symbol.symbol_def
      search_results
      (process_class_occurrence ~kind)
  | (Typedef { name }, Expansion_strategy.Typedef TdDefault) ->
    let search_results = search_type ctx name genv in
    get_successors_standard
      ctx
      symbol.symbol_def
      search_results
      process_typedef_occurrence
  | (Typeconst { class_name; member_name }, Expansion_strategy.Typeconst _) ->
    get_typeconst_successors
      ~ctx
      ~genv
      ~env
      class_name
      member_name
      symbol.strategy
  | _ ->
    failwith
      "Internal error: mismatched symbol_def and strategy in get_successors"

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

module Selection_graph = struct
  module Symbol_node = struct
    type t = {
      symbol_s: Symbol_def.with_strategy;
          (** The Symbol_def.with_strategy uniquely identifies each node in the graph. *)
      distance: int;
      incoming: (Reason.t, Symbol_def.with_strategy list) Hashtbl.t; [@opaque]
          (** Invariant: no duplicates *)
    }
    [@@deriving show]

    let make symbol_s distance =
      { symbol_s; distance; incoming = Hashtbl.create (module Reason) }

    let add_incoming ~predecessor ~target ~reason =
      Hashtbl.update target.incoming reason ~f:(fun existing ->
          let predecessor_symbol = predecessor.symbol_s in
          predecessor_symbol :: Option.value existing ~default:[])
  end

  (** Just here so we can create a Hashtbl with Symbol_def.with_strategy keys *)
  module With_strategy = struct
    type t = Symbol_def.with_strategy [@@deriving ord, sexp, hash]
  end

  let check_invariants
      (nodes : (Symbol_def.with_strategy, Symbol_node.t) Hashtbl.t) :
      (unit, error_msg) result =
    let open Result.Let_syntax in
    let check_symbol_strategy_match (ws : Symbol_def.with_strategy) :
        (unit, error_msg) result =
      let open Symbol_def in
      match (ws.symbol_def, ws.strategy) with
      | (Method _, Expansion_strategy.Method _)
      | (Typeconst _, Expansion_strategy.Typeconst _)
      | (Typedef _, Expansion_strategy.Typedef _)
      | (Classish _, Expansion_strategy.Classish _) ->
        Result.Ok ()
      | _ ->
        let msg =
          Printf.sprintf
            "Symbol/strategy mismatch: %s"
            (Symbol_def.show_with_strategy ws)
        in
        HackEventLogger.invariant_violation_bug msg;
        Result.Error msg
    in

    let check_no_duplicate_incoming
        (node : Symbol_node.t)
        ((reason, incoming) : Reason.t * Symbol_def.with_strategy list) :
        (unit, error_msg) result =
      let seen = Hash_set.create (module With_strategy) in
      List.fold_result incoming ~init:() ~f:(fun () incoming_symbol ->
          Result.map_error
            (Hash_set.strict_add seen incoming_symbol)
            ~f:(fun _ ->
              let msg =
                Printf.sprintf
                  "duplicate edge from %s to %s for reason %s"
                  (Symbol_def.show_with_strategy incoming_symbol)
                  (Symbol_def.show_with_strategy node.symbol_s)
                  (Reason.show reason)
              in
              HackEventLogger.invariant_violation_bug msg;
              msg))
    in

    Hashtbl.fold
      nodes
      ~init:(Result.Ok ())
      ~f:(fun ~key:symbol_s ~data:node acc ->
        let* () = acc in

        let* () = check_symbol_strategy_match symbol_s in

        let per_reason = Hashtbl.to_alist node.incoming in
        let checked =
          List.map per_reason ~f:(check_no_duplicate_incoming node)
        in
        let* () = Result.all_unit checked in
        Result.Ok ())

  let build
      ~(ctx : Provider_context.t)
      ~(genv : ServerEnv.genv)
      ~(env : ServerEnv.env)
      max_distance
      max_test_files
      (roots : Symbol_def.with_strategy list) :
      (result_entry list, error_msg) Result.t =
    let open Result.Let_syntax in
    (* Invariant: Deduplicated; does not contain the same Symbol_def.with_strategy twice *)
    let queue : Symbol_node.t Queue.t = Queue.create () in

    (* Invariant: Superset of nodes in `queue` *)
    let nodes : (Symbol_def.with_strategy, Symbol_node.t) Hashtbl.t =
      Hashtbl.create (module With_strategy)
    in
    (* Maps test file paths to the minimum distance at which we have discovered them *)
    let test_files = Hashtbl.create (module String) in

    let maybe_add_node
        (symbol : Symbol_def.with_strategy) distance predecessor_opt =
      let (seen, node) =
        match Hashtbl.find nodes symbol with
        | None ->
          let node = Symbol_node.make symbol distance in
          Hashtbl.add_exn nodes ~key:symbol ~data:node;
          Queue.enqueue queue node;
          (false, node)
        | Some node ->
          log_debug "Saw symbol %s again" (Symbol_def.show_with_strategy symbol);
          (true, node)
      in

      assert (node.distance <= distance);
      Option.iter predecessor_opt ~f:(fun (predecessor, reason) ->
          Symbol_node.add_incoming ~predecessor ~target:node ~reason);
      seen
    in

    List.iter roots ~f:(fun root_symbol ->
        match is_root_symbol_in_test_file ctx root_symbol.symbol_def with
        | Some test_file ->
          ignore (Hashtbl.add test_files ~key:test_file ~data:0)
        | None -> ignore (maybe_add_node root_symbol 0 None));

    (* For each root method, we add a node for the enclosing class.
       TODO(frankemrich): In the future we may want to consider doing this for any method we
       encounter, not just root methods *)
    let added_root_symbols =
      Hashtbl.fold nodes ~init:[] ~f:(fun ~key:_ ~data acc ->
          data.symbol_s :: acc)
    in
    List.iter added_root_symbols ~f:(fun root_symbol ->
        match root_symbol.symbol_def with
        | Method { class_name; _ } ->
          let reason = Reason.Classish CEnclosingRootMethod in

          let class_symbol = Symbol_def.class_from_name ctx class_name reason in

          (* We consider going from root (distance 0) to the enclosing class as a step *)
          let distance = 1 in

          (* Must exist, we added it during init above *)
          let root_node = Hashtbl.find_exn nodes root_symbol in
          let seen =
            maybe_add_node class_symbol distance (Some (root_node, reason))
          in

          if not seen then
            log_debug
              "Added enclosing class to queue for root %s"
              (Symbol_def.show_with_strategy root_symbol)
        | _ -> ());

    let at_test_file_limit () =
      match max_test_files with
      | None -> false
      | Some limit -> Hashtbl.length test_files >= limit
    in

    let rec bfs () =
      match Queue.dequeue queue with
      | Some ({ distance; symbol_s; incoming = _ } as node)
        when distance < max_distance && not (at_test_file_limit ()) ->
        log_info "processing node %s" (Symbol_node.show node);
        let new_distance = distance + 1 in
        (match get_successors ~ctx ~genv ~env symbol_s with
        | Result.Ok (referencing_defs, referencing_test_files) ->
          (* referencing_defs may include duplicates.
             We need to skip them to avoid creating duplicate edges between nodes *)
          let referencing_defs_dedup =
            Hash_set.of_list (module Symbol_def.With_reason) referencing_defs
          in
          Hash_set.iter
            referencing_defs_dedup
            ~f:(fun { symbol_def = referencing_def; reason } ->
              let referencing_symbol_s : Symbol_def.with_strategy =
                {
                  symbol_def = referencing_def;
                  strategy = Expansion_strategy.of_reason reason;
                }
              in
              ignore
                (maybe_add_node
                   referencing_symbol_s
                   new_distance
                   (Some (node, reason))));
          List.iter referencing_test_files ~f:(fun referencing_test_file ->
              if not (at_test_file_limit ()) then
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
            (Symbol_def.show_with_strategy symbol_s)
            msg)
      | _ -> ()
    in

    bfs ();

    let* () = check_invariants nodes in

    Result.Ok
      (Hashtbl.fold test_files ~init:[] ~f:(fun ~key ~data acc ->
           let open ServerCommandTypes.Find_my_tests in
           { file_path = key; distance = data } :: acc))
end

let go
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~(max_distance : int)
    ~(max_test_files : int option)
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
        let root : Symbol_def.with_strategy =
          match action with
          | Class { class_name } ->
            let reason = Reason.Classish CRoot in
            Symbol_def.class_from_name ctx (Utils.strip_ns class_name) reason
          | Method { class_name; member_name } ->
            let reason = Reason.Method MRoot in
            let strategy = Expansion_strategy.of_reason reason in
            let symbol_def =
              Method { class_name = Utils.strip_ns class_name; member_name }
            in
            { symbol_def; strategy }
          | Typedef { name } ->
            let reason = Reason.Typedef TdRoot in
            let strategy = Expansion_strategy.of_reason reason in
            { symbol_def = Typedef { name = Utils.strip_ns name }; strategy }
          | Typeconst { class_name; member_name } ->
            let reason = Reason.Typeconst TcRoot in
            let strategy = Expansion_strategy.of_reason reason in
            let symbol_def =
              Typeconst { class_name = Utils.strip_ns class_name; member_name }
            in
            { symbol_def; strategy }
        in

        Result.Ok (root :: acc))
  in
  Selection_graph.build ~ctx ~genv ~env max_distance max_test_files roots

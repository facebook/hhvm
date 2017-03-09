(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module Syntax = Full_fidelity_editable_syntax
module TriviaKind = Full_fidelity_trivia_kind
module Trivia = Full_fidelity_editable_trivia
module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)
open Syntax
open Core
(* open Hackfmt_utils *)

(* TODO: move this to a config file *)
let __INDENT_WIDTH = 2

type open_span = {
  open_span_start: int;
  open_span_cost: Cost.t;
}

type rule_state =
  | NoRule
  | RuleKind of Rule.kind
  | LazyRuleID of int

let open_span start cost = {
  open_span_start = start;
  open_span_cost = cost;
}

let builder = object (this)
  val open_spans = Stack.create ();
  val mutable rules = [];
  val mutable lazy_rules = ISet.empty;

  val mutable chunks = [];

  val mutable next_split_rule = NoRule;
  val mutable next_lazy_rules = ISet.empty;
  val mutable pending_spans = [];
  val mutable space_if_not_split = false;
  val mutable pending_comma = None;

  val mutable pending_whitespace = "";

  val mutable chunk_groups = [];
  val mutable rule_alloc = Rule_allocator.make ();
  val mutable nesting_alloc = Nesting_allocator.make ();
  val mutable span_alloc = Span_allocator.make ();
  val mutable block_indent = 0;

  val mutable in_range = Chunk_group.No;
  val mutable start_char = 0;
  val mutable end_char = max_int;
  val mutable seen_chars = 0;

  (* TODO: Make builder into an instantiable class instead of
   * having this reset method
   *)
  method reset start_c end_c =
    Stack.clear open_spans;
    rules <- [];
    lazy_rules <- ISet.empty;
    chunks <- [];
    next_split_rule <- NoRule;
    next_lazy_rules <- ISet.empty;
    pending_spans <- [];
    space_if_not_split <- false;
    pending_comma <- None;
    pending_whitespace <- "";
    chunk_groups <- [];
    rule_alloc <- Rule_allocator.make ();
    nesting_alloc <- Nesting_allocator.make ();
    span_alloc <- Span_allocator.make ();
    block_indent <- 0;

    in_range <- Chunk_group.No;
    start_char <- start_c;
    end_char <- end_c;
    seen_chars <- 0;
    ()

  method add_string ?(is_trivia=false) s =
    chunks <- (match chunks with
      | hd :: tl when hd.Chunk.is_appendable ->
        let text = hd.Chunk.text ^ pending_whitespace ^ s in
        {hd with Chunk.text = text} :: tl
      | _ -> begin
          let nesting = nesting_alloc.Nesting_allocator.current_nesting in
          let handle_started_next_split_rule () =
            let cs = Chunk.make s (List.hd rules) nesting :: chunks in
            this#end_rule ();
            next_split_rule <- NoRule;
            cs
          in
          match next_split_rule with
            | NoRule -> Chunk.make s (List.hd rules) nesting :: chunks
            | LazyRuleID rule_id ->
              this#start_lazy_rule rule_id;
              handle_started_next_split_rule ()
            | RuleKind rule_kind ->
              this#start_rule_kind ~rule_kind ();
              handle_started_next_split_rule ()
      end
    );

    if not is_trivia then begin
      lazy_rules <- ISet.union next_lazy_rules lazy_rules;
      next_lazy_rules <- ISet.empty;

      let rev_pending_spans_stack = List.rev pending_spans in
      pending_spans <- [];
      List.iter rev_pending_spans_stack ~f:(fun cost ->
        Stack.push (open_span (List.length chunks - 1) cost) open_spans
      );
    end;

    pending_whitespace <- ""

  method set_pending_comma () =
    chunks <- (match chunks with
      | hd :: tl when not hd.Chunk.is_appendable ->
        {hd with Chunk.comma_rule = Some (List.hd_exn rules)} :: tl;
      | _ -> pending_comma <- Some (List.hd_exn rules); chunks;
    );

  method set_pending_whitespace s =
    pending_whitespace <- s

  method split space =
    chunks <- (match chunks with
      | hd :: tl when hd.Chunk.is_appendable ->
        let rule_id = hd.Chunk.rule in
        let rule = match rules with
          | [] when rule_id = Rule.null_rule_id ->
            this#create_rule (Rule.Simple Cost.Base)
          | hd :: _ when rule_id = Rule.null_rule_id -> hd
          | _ -> rule_id
        in
        let chunk = Chunk.finalize hd rule rule_alloc
          space_if_not_split pending_comma in
        space_if_not_split <- space;
        pending_comma <- None;
        chunk :: tl
      | _ -> chunks
    )

  method private create_rule rule_kind =
    let ra, rule = Rule_allocator.make_rule rule_alloc rule_kind in
    rule_alloc <- ra;
    rule.Rule.id

  method create_lazy_rule ?(rule_kind=Rule.Simple Cost.Base) () =
    let id = this#create_rule rule_kind in
    next_lazy_rules <- ISet.add id next_lazy_rules;
    id

  (* TODO: after unit tests, make this idempotency a property of method split *)
  method simple_space_split_if_unsplit () =
    match chunks with
      | hd :: _ when hd.Chunk.is_appendable -> this#simple_space_split ()
      | _ -> ()

  method simple_split_if_unsplit () =
    match chunks with
      | hd :: _ when hd.Chunk.is_appendable -> this#simple_split ()
      | _ -> ()

  (* TODO: is this the right whitespace strategy? *)
  method add_always_empty_chunk () =
    let nesting = nesting_alloc.Nesting_allocator.current_nesting in
    let create_empty_chunk () =
      let rule = this#create_rule Rule.Always in
      let chunk = Chunk.make "" (Some rule) nesting in
      Chunk.finalize chunk rule rule_alloc false None
    in
    (chunks <- match chunks with
      | [] -> create_empty_chunk () :: chunks
      | hd :: _ when not hd.Chunk.is_appendable ->
        create_empty_chunk () :: chunks
      (* TODO: this is probably wrong, figure out what this means *)
      | _ -> chunks)

  method simple_space_split ?(cost=Cost.Base) () =
    this#split true;
    this#set_next_split_rule (RuleKind (Rule.Simple cost));
    ()

  method simple_split ?(cost=Cost.Base) () =
    this#split false;
    this#set_next_split_rule (RuleKind (Rule.Simple cost));
    ()

  method hard_split () =
    this#split false;
    this#set_next_split_rule (RuleKind Rule.Always);
    ()

  method set_next_split_rule rule_type =
    next_split_rule <- (match next_split_rule with
      | RuleKind Rule.Always -> next_split_rule
      | _ -> rule_type
    )

  method nest ?(amount=__INDENT_WIDTH) ?(skip_parent=false) () =
    nesting_alloc <- Nesting_allocator.nest nesting_alloc amount skip_parent

  method unnest () =
    nesting_alloc <- Nesting_allocator.unnest nesting_alloc

  method private start_rule_id rule_id =
    rule_alloc <- Rule_allocator.mark_dependencies
      rule_alloc lazy_rules rules rule_id;
    rules <- rule_id :: rules

  method start_rule_kind ?(rule_kind=Rule.Simple Cost.Base) () =
    (* Override next_split_rule unless it's an Always rule *)
    next_split_rule <- (match next_split_rule with
      | RuleKind kind when kind <> Rule.Always -> NoRule
      | _ -> next_split_rule
    );
    let rule = this#create_rule rule_kind in
    this#start_rule_id rule

  method start_lazy_rule lazy_rule_id =
    if ISet.mem lazy_rule_id next_lazy_rules then begin
      next_lazy_rules <- ISet.remove lazy_rule_id next_lazy_rules;
      this#start_rule_id lazy_rule_id
    end else if ISet.mem lazy_rule_id lazy_rules then begin
      lazy_rules <- ISet.remove lazy_rule_id lazy_rules;
      this#start_rule_id lazy_rule_id
    end else
      raise (Failure "Called start_lazy_rule with a rule id is not a lazy rule")

  method end_rule () =
    rules <- match rules with
      | hd :: tl -> tl
      | [] -> [] (*TODO: error *)

  method has_rule_kind kind =
    List.exists rules ~f:(fun id ->
      (Rule_allocator.get_rule_kind rule_alloc id) = kind
    )

  method start_span ?(cost=Cost.Base) () =
    pending_spans <- cost :: pending_spans;

  method end_span () =
    pending_spans <- match pending_spans with
      | hd :: tl -> tl
      | [] ->
        let os = Stack.pop open_spans in
        let sa, span = Span_allocator.make_span span_alloc os.open_span_cost in
        span_alloc <- sa;
        let r_chunks = List.rev chunks in
        chunks <- List.rev_mapi r_chunks ~f:(fun n x ->
        if n <= os.open_span_start then
          x
        else
          (* TODO: handle required hard splits *)
          {x with Chunk.spans = span :: x.Chunk.spans}
        );
        []

  (*
    TODO: find a better way to represt the rules empty case
    for end chunks and block nesting
  *)
  method start_block_nest () =
    if List.is_empty rules && not (Nesting_allocator.is_nested nesting_alloc)
    then block_indent <- block_indent + 2
    else this#nest ()

  method end_block_nest () =
    if List.is_empty rules && not (Nesting_allocator.is_nested nesting_alloc)
    then block_indent <- block_indent - 2
    else this#unnest ()

  method end_chunks () =
    this#hard_split ();
    if List.is_empty rules &&
      not (List.is_empty chunks) &&
      not (Nesting_allocator.is_nested nesting_alloc)
    then this#push_chunk_group ()

  method push_chunk_group () =
    let print_range = Chunk_group.(match in_range with
      | StartAt n when n = List.length chunks -> No
      | EndAt n when n = List.length chunks -> All
      | _ -> in_range
    ) in

    chunk_groups <- Chunk_group.({
      chunks = (List.rev chunks);
      rule_map = rule_alloc.Rule_allocator.rule_map;
      rule_dependency_map = rule_alloc.Rule_allocator.dependency_map;
      block_indentation = block_indent;
      print_range;
    }) :: chunk_groups;

    in_range <- Chunk_group.(match in_range with
      | StartAt _ -> All
      | All -> All
      | Range _ -> No
      | EndAt _ -> No
      | No -> No
    );
    chunks <- [];
    rule_alloc <- Rule_allocator.make ();
    nesting_alloc <- Nesting_allocator.make ();
    span_alloc <- Span_allocator.make ()

  method _end () =
    (*TODO: warn if not empty? *)
    if not (List.is_empty chunks) then this#end_chunks ();
    List.rev_filter
      chunk_groups ~f:(fun cg -> Chunk_group.(cg.print_range <> No))

  (* TODO: move the partial formatting logic to it's own module somehow *)
  method advance n =
    seen_chars <- seen_chars + n;

  (* TODO: make this idempotent *)
  method check_range () =
    in_range <-
      if seen_chars <> start_char
      then in_range
      else begin match chunks with
        | [] -> Chunk_group.All
        | hd :: _ ->
          next_split_rule <- RuleKind Rule.Always;
          this#simple_split_if_unsplit ();
          Chunk_group.StartAt (List.length chunks)
      end;
    in_range <-
      if seen_chars <> end_char
      then in_range
      else begin
        if List.is_empty chunks
        then Chunk_group.No
        else begin
          let end_at = List.length chunks in
          match in_range with
            | Chunk_group.StartAt start_at ->
              Chunk_group.Range (start_at, end_at)
            | Chunk_group.Range _ -> in_range
            | _ -> Chunk_group.EndAt end_at
        end;
      end

  method closing_brace_token x =
    (* TODO: change this to split and recompose *)
    let is_not_end_of_line t = Trivia.kind t <> TriviaKind.EndOfLine in
    let rev_leading_trivia = List.rev @@ EditableToken.leading x in
    let (rev_after_last_newline, rev_rest_including_last_newline) =
      List.split_while rev_leading_trivia ~f:is_not_end_of_line in

    this#handle_trivia ~is_leading:true @@
      this#break_out_delimited @@ List.rev rev_rest_including_last_newline;
    this#end_chunks();
    this#end_block_nest ();
    this#handle_trivia ~is_leading:true @@
      this#break_out_delimited @@ List.rev rev_after_last_newline;
    this#handle_token x;
    this#handle_trivia ~is_leading:false @@
      this#break_out_delimited (EditableToken.trailing x);
    ()

  (* TODO: Handle (aka remove) excess whitespace inside of an ast node *)
  method handle_trivia ~is_leading trivia_list =
    if not (List.is_empty trivia_list) then begin
      let handle_newlines ~is_trivia newlines =
        match newlines with
          | 0 when is_trivia -> this#simple_space_split_if_unsplit ()
          | 0 -> ()
          | 1 -> this#hard_split ()
          | _ ->
            this#hard_split ();
            this#add_always_empty_chunk ()
      in

      this#check_range ();
      let newlines, only_whitespace  = List.fold trivia_list ~init:(0, true)
        ~f:(fun (newlines, only_whitespace) t ->
          this#advance (Trivia.width t);
          (match Trivia.kind t with
            | TriviaKind.WhiteSpace ->
              (* Needed for the XHP whitespace hack, see XHPExpression *)
              this#check_range ();
              newlines, only_whitespace
            | TriviaKind.EndOfLine ->
              if only_whitespace && is_leading then
                this#add_always_empty_chunk ();
              this#check_range ();
              newlines + 1, false
            | TriviaKind.Unsafe
            | TriviaKind.UnsafeExpression
            | TriviaKind.FallThrough
            | TriviaKind.FixMe
            | TriviaKind.IgnoreError
            | TriviaKind.SingleLineComment
            | TriviaKind.DelimitedComment ->
              handle_newlines ~is_trivia:true newlines;
              this#add_string ~is_trivia:true @@ Trivia.text t;
              (match Trivia.kind t with
                | TriviaKind.Unsafe
                | TriviaKind.FallThrough
                | TriviaKind.SingleLineComment ->
                  this#hard_split ();
                | _ -> ()
              );
              0, false
        )
      ) in
      if is_leading
        then handle_newlines ~is_trivia:(not only_whitespace) newlines;
    end;

  method handle_token t =
    this#check_range ();
    this#add_string (EditableToken.text t);
    this#advance (EditableToken.width t);

  method token x =
    this#handle_trivia
      ~is_leading:true (this#break_out_delimited @@ EditableToken.leading x);
    this#handle_token x;
    this#handle_trivia
      ~is_leading:false (this#break_out_delimited @@ EditableToken.trailing x);
    ()

  method token_trivia_only x =
    this#handle_trivia
      ~is_leading:true (this#break_out_delimited @@ EditableToken.leading x);
    this#check_range ();
    this#advance (EditableToken.width x);
    this#handle_trivia
      ~is_leading:false (this#break_out_delimited @@ EditableToken.trailing x);
    ()

  method break_out_delimited trivia =
    let new_line_regex = Str.regexp "\n" in
    List.concat_map trivia ~f:(fun triv ->
      match Trivia.kind triv with
        | TriviaKind.UnsafeExpression
        | TriviaKind.FixMe
        | TriviaKind.IgnoreError
        | TriviaKind.DelimitedComment ->
          let delimited_lines =
            Str.split new_line_regex @@ Trivia.text triv in
          let map_tail str =
            let prefix_space_count str =
              let len = String.length str in
              let rec aux i =
                if i = len || str.[i] <> ' '
                then 0
                else 1 + (aux (i + 1))
              in
              aux 0
            in
            let start_index = min block_indent (prefix_space_count str) in
            let len = String.length str - start_index in
            let dc = Trivia.make_delimited_comment @@
              String.sub str start_index len in
            let spacer dc = if 0 = start_index then dc else
              Trivia.make_whitespace (String.make start_index ' ') :: dc
            in
            Trivia.make_eol "\n" :: spacer [dc]
          in

          Trivia.make_delimited_comment (List.hd_exn delimited_lines) ::
            List.concat_map (List.tl_exn delimited_lines) ~f:map_tail
        | _ ->
          [triv]
    )

end

let split ?space:(space=false) () =
  builder#split space

let token = builder#token

let add_space () =
  builder#add_string " "

let pending_space () =
  builder#set_pending_whitespace " "

let rec transform node =
  let t = transform in
  let span = Some Cost.Base in
  let nest = true in
  let space = true in

  let () = match syntax node with
  | Missing -> ()
  | Token x ->
    token x;
    ()
  | SyntaxList _ ->
    raise (Failure (Printf.sprintf
      "Error: SyntaxList should never be handled directly;
      offending text is '%s'." (text node)));
  | ScriptHeader x ->
    let (lt, q, lang_kw) = get_script_header_children x in
    t lt;
    t q;
    t lang_kw;
    builder#end_chunks ();
  | EndOfFile x ->
    let token = get_end_of_file_children x in
    t token;
  | Script x ->
    let (header, declarations) = get_script_children x in
    t header;
    handle_possible_list declarations;
  | ScriptFooter x -> t @@ get_script_footer_children x
  | SimpleTypeSpecifier x -> t @@ get_simple_type_specifier_children x
  | LiteralExpression x ->
    (* Double quoted string literals can create a list *)
    handle_possible_list @@ get_literal_expression_children x
  | VariableExpression x -> t @@ get_variable_expression_children x
  | QualifiedNameExpression x -> t @@ get_qualified_name_expression_children x
  | PipeVariableExpression x -> t @@ get_pipe_variable_expression_children x
  | EnumDeclaration x ->
    let (attr, kw, name, colon_kw, base, enum_type, left_b, enumerators,
      right_b) = get_enum_declaration_children x in
    t attr;
    (* TODO: create a "when_present" abstraction to replace this pattern *)
    if not (is_missing attr) then builder#end_chunks ();
    t kw;
    pending_space ();
    t name;
    t colon_kw;
    builder#simple_space_split ();
    tl_with ~nest ~f:(fun () ->
      pending_space ();
      t base;
      pending_space ();
      t enum_type;
      pending_space ();
      t left_b;
    ) ();
    builder#end_chunks ();
    builder#start_block_nest ();
    handle_possible_list enumerators;
    transform_and_unnest_closing_brace right_b;
    builder#end_chunks ();
  | Enumerator x ->
    let (name, eq_kw, value, semi) = get_enumerator_children x in
    t name;
    pending_space ();
    t eq_kw;
    builder#simple_space_split ();
    t_with ~nest value;
    t semi;
    builder#end_chunks ();
  | AliasDeclaration x ->
    (* TODO: revisit this for long names *)
    let (attr, kw, name, generic, type_constraint, eq_kw, alias_type, semi) =
      get_alias_declaration_children x in
    t attr;
    if not (is_missing attr) then builder#end_chunks ();
    t kw;
    pending_space ();
    t name;
    t generic;
    pending_space ();
    t type_constraint;
    pending_space ();
    t eq_kw;
    builder#simple_space_split ();
    t_with ~nest alias_type;
    t semi;
    builder#end_chunks ();
  | PropertyDeclaration x ->
    let (modifiers, prop_type, declarators, semi) =
      get_property_declaration_children x in
    handle_possible_list ~after_each:(fun _ -> add_space ()) modifiers;
    t prop_type;
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list ~before_each:(split ~space) declarators;
    ) ();
    t semi;
    builder#end_chunks ();
    ()
  | PropertyDeclarator x ->
    let (name, prop_initializer) = get_property_declarator_children x in
    t name;
    t prop_initializer;
    ()
  | NamespaceDeclaration x ->
    let (kw, name, body) = get_namespace_declaration_children x in
    t kw;
    pending_space ();
    t name;
    t body;
    builder#end_chunks ();
    ()
  | NamespaceBody x ->
    let (left_b, decls, right_b) = get_namespace_body_children x in
    pending_space ();
    t left_b;
    builder#end_chunks ();
    builder#start_block_nest ();
    tl_with ~f:(fun () -> handle_possible_list decls) ();
    transform_and_unnest_closing_brace right_b;
    ()
  | NamespaceUseDeclaration x ->
    let (kw, use_kind, clauses, semi) =
      get_namespace_use_declaration_children x in
    t kw;
    pending_space ();
    t use_kind;
    if not (is_missing use_kind) then pending_space ();
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list clauses ~after_each:after_each_argument;
    ) ();
    t semi;
    builder#end_chunks ();
    ()
  | NamespaceGroupUseDeclaration x ->
    let (kw, use_kind, prefix, left_b, clauses, right_b, semi) =
      get_namespace_group_use_declaration_children x in
    t kw;
    pending_space ();
    t use_kind;
    if not (is_missing use_kind) then pending_space ();
    t prefix;
    transform_argish left_b clauses right_b;
    t semi;
    builder#end_chunks ();
    ()
  | NamespaceUseClause x ->
    let (use_kind, name, as_kw, alias) = get_namespace_use_clause_children x in
    t use_kind;
    t name;
    if not (is_missing as_kw) then pending_space ();
    t as_kw;
    if not (is_missing alias) then pending_space ();
    t alias;
    ()
  | FunctionDeclaration x ->
    let (attr, header, body) = get_function_declaration_children x in
    t attr;
    if not (is_missing attr) then builder#end_chunks ();
    t header;
    handle_possible_compound_statement body;
    builder#end_chunks ();
    ()
  | FunctionDeclarationHeader x ->
    transform_function_declaration_header ~span_started:false x;
    ()
  | WhereClause x ->
    let (where, constraints) = get_where_clause_children x in
    t where;
    add_space ();
    t constraints;
    ()
  | WhereConstraint x ->
    let (left, op, right) = get_where_constraint_children x in
    t left;
    add_space ();
    t op;
    add_space ();
    t right;
    ()
  | MethodishDeclaration x ->
    let (attr, modifiers, func_decl, body, semi) =
      get_methodish_declaration_children x
    in
    t attr;
    if not (is_missing attr) then builder#end_chunks ();
    builder#start_span ();
    handle_possible_list ~after_each:(fun _ -> add_space ()) modifiers;
    (match syntax func_decl with
      | FunctionDeclarationHeader x ->
        transform_function_declaration_header ~span_started:true x
      | _ ->
        raise (Failure
          "invalid parse tree provided, expecting a function declaration header"
        )
    );
    if not (is_missing body) then handle_possible_compound_statement body;
    t semi;
    builder#end_chunks ();
    ()
  | ClassishDeclaration x ->
    let (attr, modifiers, kw, name, type_params, extends_kw, extends,
      impl_kw, impls, body) = get_classish_declaration_children x
    in
    t attr;
    if not (is_missing attr) then builder#end_chunks ();
    tl_with ~span ~f:(fun () ->
      handle_possible_list ~after_each:(fun _ -> add_space ()) modifiers;
      t kw;
      split ~space ();
      tl_with ~nest ~f:(fun () ->
        t name;
        t type_params;
      ) ();
    ) ();

    let after_each_ancestor is_last = if is_last then () else split ~space () in

    if not (is_missing extends_kw) then begin
      split ~space ();
      tl_with ~span ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
        t extends_kw;
        split ~space ();
        tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
          handle_possible_list ~after_each:after_each_ancestor extends
        ) ();
      ) ();
      ()
    end;

    if not (is_missing impl_kw) then begin
      split ~space ();
      tl_with ~span ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
        t impl_kw;
        split ~space ();
        tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
          handle_possible_list ~after_each:after_each_ancestor impls
        ) ();
      ) ();
      ()
    end;
    t body;
    ()
  | ClassishBody x ->
    let (left_b, body, right_b) = get_classish_body_children x in
    add_space ();
    t left_b;
    builder#end_chunks ();
    builder#start_block_nest ();
    handle_possible_list body;
    transform_and_unnest_closing_brace right_b;
    builder#end_chunks ();
    ()
  | TraitUse x ->
    let (kw, elements, semi) = get_trait_use_children x in
    t kw;
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list ~before_each:(split ~space) elements;
    ) ();
    t semi;
    builder#end_chunks();
    ()
  | RequireClause x ->
    let (kw, kind, name, semi) = get_require_clause_children x in
    t kw;
    add_space ();
    t kind;
    split ~space ();
    t name;
    t semi;
    builder#end_chunks ();
    ()
  | ConstDeclaration x ->
    let (abstr, kw, const_type, declarators, semi) =
      get_const_declaration_children x in
    t abstr;
    if not (is_missing abstr) then add_space ();
    t kw;
    if not (is_missing const_type) then add_space ();
    t const_type;
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list ~before_each:(split ~space) declarators;
    ) ();
    t semi;
    builder#end_chunks ();
    ()
  | ConstantDeclarator x ->
    let (name, const_initializer) = get_constant_declarator_children x in
    t name;
    t const_initializer;
    ()
  | TypeConstDeclaration x ->
    let (abs, kw, type_kw, name, type_constraint, eq, type_spec, semi) =
      get_type_const_declaration_children x in
    t abs;
    pending_space ();
    t kw;
    pending_space ();
    t type_kw;
    pending_space ();
    t name;
    pending_space ();
    t type_constraint;
    pending_space ();
    t eq;
    builder#simple_space_split ();
    t_with ~nest type_spec;
    t semi;
    builder#end_chunks ();
    ()
  | DecoratedExpression x ->
    let (decorator, expr) = get_decorated_expression_children x in
    t decorator;
    t expr;
    ()
  | ParameterDeclaration x ->
    let (attr, visibility, param_type, name, default) =
      get_parameter_declaration_children x
    in
    t attr;
    t visibility;
    pending_space ();
    t param_type;
    if is_missing visibility && is_missing param_type
    then t name
    else begin
      builder#simple_space_split ();
      t_with ~nest name
    end;
    t default;
  | VariadicParameter x ->
    let ellipsis = get_variadic_parameter_children x in
    t ellipsis;
  | AttributeSpecification x ->
    let (left_da, attrs, right_da) = get_attribute_specification_children x in
    transform_argish left_da attrs right_da;
    ()
  | Attribute x ->
    let (name, left_p, values, right_p) = get_attribute_children x in
    t name;
    transform_argish left_p values right_p;
    ()
  | InclusionExpression x ->
    let (kw, expr) = get_inclusion_expression_children x in
    t kw;
    builder#simple_space_split ();
    t expr;
  | InclusionDirective x ->
    let (expr, semi) = get_inclusion_directive_children x in
    t expr;
    t semi;
    builder#end_chunks ()
  | CompoundStatement x ->
    handle_possible_compound_statement node;
  | ExpressionStatement x ->
    t x.expression_statement_expression;
    t x.expression_statement_semicolon;
    builder#end_chunks()
  | UnsetStatement x ->
    let (kw, left_p, args, right_p, semi) = get_unset_statement_children x in
    t kw;
    transform_argish left_p args right_p;
    t semi;
  | WhileStatement x ->
    t x.while_keyword;
    add_space ();
    t x.while_left_paren;
    split ();
    tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      t_with ~nest x.while_condition;
      split ();
      t x.while_right_paren;
    ) ();
    handle_possible_compound_statement x.while_body;
    builder#end_chunks ();
    ()
  | IfStatement x ->
    let (kw, left_p, condition, right_p, if_body, elseif_clauses, else_clause) =
      get_if_statement_children x in
    t kw;
    add_space ();
    transform_condition left_p condition right_p;
    handle_possible_compound_statement if_body;
    handle_possible_list elseif_clauses;
    t else_clause;
    builder#end_chunks ();
    ()
  | ElseifClause x ->
    let (kw, left_p, condition, right_p, body) = get_elseif_clause_children x in
    t kw;
    pending_space ();
    transform_condition left_p condition right_p;
    handle_possible_compound_statement x.elseif_statement;
    ()
  | ElseClause x ->
    t x.else_keyword;
    let _ = match syntax x.else_statement with
      | IfStatement _ ->
        pending_space ();
        t x.else_statement;
        pending_space ();
        ()
      | _ -> handle_possible_compound_statement x.else_statement in
    ()
  | TryStatement x ->
    (* TODO: revisit *)
    let (kw, body, catch_clauses, finally_clause) =
      get_try_statement_children x in
    t kw;
    handle_possible_compound_statement body;
    handle_possible_list catch_clauses;
    t finally_clause;
    builder#end_chunks ();
  | CatchClause x ->
    let (kw, left_p, ex_type, var, right_p, body) =
      get_catch_clause_children x in
    t kw;
    pending_space ();
    t left_p;
    split ();
    tl_with ~nest ~f:(fun () ->
      t ex_type;
      pending_space ();
      t var;
      split ();
    ) ();
    t right_p;
    handle_possible_compound_statement body;
    ();
  | FinallyClause x ->
    let (kw, body) = get_finally_clause_children x in
    t kw;
    pending_space ();
    handle_possible_compound_statement body;
  | DoStatement x ->
    let (do_kw, body, while_kw, left_p, cond, right_p, semi) =
      get_do_statement_children x in
    t do_kw;
    pending_space ();
    handle_possible_compound_statement body;
    t while_kw;
    pending_space ();
    transform_condition left_p cond right_p;
    t semi;
    builder#end_chunks ();
  | ForStatement x ->
    let (kw, left_p, init, semi1, control, semi2, after_iter, right_p, body) =
      get_for_statement_children x in
    t kw;
    add_space ();
    t left_p;
    tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      split ();
      tl_with ~nest ~f:(fun () ->
        handle_possible_list init;
        t semi1;
        split ~space ();
        handle_possible_list control;
        t semi2;
        split ~space ();
        handle_possible_list after_iter;
      ) ();
      split ();
      t right_p;
    ) ();
    handle_possible_compound_statement body;
    builder#end_chunks ();
    ()
  | ForeachStatement x ->
    let (kw, left_p, collection, await_kw, as_kw, key, arrow, value, right_p,
      body) = get_foreach_statement_children x in
    t kw;
    pending_space ();
    t left_p;
    t collection;
    pending_space ();
    t await_kw;
    pending_space ();
    t as_kw;
    pending_space ();
    t key;
    pending_space ();
    t arrow;
    split ~space ();
    t value;
    split ();
    t right_p;
    handle_possible_compound_statement body;
    builder#end_chunks ();
    ()
  | SwitchStatement x ->
    let (kw, left_p, expr, right_p, left_b, sections, right_b) =
      get_switch_statement_children x in
    t kw;
    add_space ();
    t left_p;
    split ();
    tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      tl_with ~nest ~f:(fun () -> t expr) ();
      t right_p;
    ) ();
    let () = handle_switch_body left_b sections right_b in
    builder#end_chunks ();
    ()
  | SwitchSection x ->
    raise (Failure "SwitchSection should be handled by handle_switch_body")
  | CaseLabel x ->
    raise (Failure "CaseLabel should be handled by handle_switch_body")
  | DefaultLabel x ->
    raise (Failure "DefaultLabel should be handled by handle_switch_body")
  | SwitchFallthrough x ->
    raise (Failure "SwitchFallthrough should be handled by handle_switch_body")
  | ReturnStatement x ->
    let (kw, expr, semi) = get_return_statement_children x in
    transform_keyword_expression_statement kw expr semi;
    ()
  | ThrowStatement x ->
    let (kw, expr, semi) = get_throw_statement_children x in
    transform_keyword_expression_statement kw expr semi;
    ()
  | BreakStatement x ->
    let (kw, expr, semi) = get_break_statement_children x in
    transform_keyword_expression_statement kw expr semi;
    ()
  | ContinueStatement x ->
    let (kw, level, semi) = get_continue_statement_children x in
    transform_keyword_expression_statement kw level semi;
  | FunctionStaticStatement x ->
    let (static_kw, declarators, semi) =
      get_function_static_statement_children x in
    transform_keyword_expr_list_statement static_kw declarators semi;
  | StaticDeclarator x ->
    let (name, static_initializer) = get_static_declarator_children x in
    t name;
    t static_initializer;
  | EchoStatement x ->
    let (kw, expr_list, semi) = get_echo_statement_children x in
    transform_keyword_expr_list_statement kw expr_list semi;
  | GlobalStatement x ->
    let (kw, var_list, semi) = get_global_statement_children x in
    transform_keyword_expr_list_statement kw var_list semi;
  | SimpleInitializer x ->
    let (eq_kw, value) = get_simple_initializer_children x in
    pending_space ();
    t eq_kw;
    builder#simple_space_split ~cost:Cost.Assignment ();
    t_with ~nest value;
  | AnonymousFunction x ->
    let (async_kw, fun_kw, lp, params, rp, colon, ret_type, use, body) =
      get_anonymous_function_children x in
    t async_kw;
    if not (is_missing async_kw) then add_space ();
    t fun_kw;
    transform_argish_with_return_type ~in_span:false lp params rp colon
      ret_type;
    t use;
    handle_possible_compound_statement body;
    builder#end_chunks ();
    ()
  | AnonymousFunctionUseClause x ->
    (* TODO: Revisit *)
    let (kw, left_p, vars, right_p) =
      get_anonymous_function_use_clause_children x in
    add_space ();
    t kw;
    add_space ();
    transform_argish left_p vars right_p;
    ()
  | LambdaExpression x ->
    let (async, signature, arrow, body) = get_lambda_expression_children x in
    t async;
    if not (is_missing async) then add_space ();
    t signature;
    pending_space ();
    t arrow;
    handle_lambda_body body;
    ()
  | LambdaSignature x ->
    let (lp, params, rp, colon, ret_type) = get_lambda_signature_children x in
    transform_argish_with_return_type ~in_span:false lp params rp colon
      ret_type;
    ()
  | CastExpression x ->
    let (lp, cast_type, rp, op) = get_cast_expression_children x in
    tl_with ~span ~f:(fun () ->
      t lp;
      t cast_type;
      t rp;
      t op;
    ) ();
    ()
  | ScopeResolutionExpression x ->
    let (qual, operator, name) = get_scope_resolution_expression_children x in
    t qual;
    t operator;
    t name;
    ()
  | MemberSelectionExpression x ->
    handle_possible_chaining
      (get_member_selection_expression_children x)
      None
  | SafeMemberSelectionExpression x ->
    handle_possible_chaining
      (get_safe_member_selection_expression_children x)
      None
  | YieldExpression x ->
    let (kw, operand) = get_yield_expression_children x in
    t kw;
    builder#simple_space_split ();
    t_with ~nest operand;
  | PrintExpression x ->
    let (kw, expr) = get_print_expression_children x in
    t kw;
    builder#simple_space_split ();
    t_with ~nest expr;
  | PrefixUnaryExpression x ->
    let (operator, operand) = get_prefix_unary_expression_children x in
    t operator;
    (* TODO: should this just live in transform's Token case?
      long term is to make await it's own syntax kind *)
    (match syntax operator with
      | Token x ->
        if EditableToken.kind x = EditableToken.TokenKind.Await then
          pending_space ();
      | _ -> ();
    );
    t operand;
  | PostfixUnaryExpression x ->
    let (operand, operator) = get_postfix_unary_expression_children x in
    t operand;
    t operator;
  | BinaryExpression x ->
    transform_binary_expression ~is_nested:false x
  | InstanceofExpression x ->
    let (left, kw, right) = get_instanceof_expression_children x in
    t left;
    add_space ();
    t kw;
    builder#simple_space_split ();
    t_with ~nest right;
    ()
  | ConditionalExpression x ->
    let (test_expr, q_kw, true_expr, c_kw, false_expr) =
      get_conditional_expression_children x in
    let lazy_argument_rule = builder#create_lazy_rule
      ~rule_kind:(Rule.Argument) () in
    t test_expr;
    tl_with ~nest ~rule:(LazyRuleID lazy_argument_rule) ~f:(fun () ->
      split ~space ();
      t q_kw;
      if not (is_missing true_expr) then begin
        pending_space ();
        t true_expr;
        split ~space ();
      end;
      t c_kw;
      pending_space ();
      t false_expr;
    ) ();
    ()
  | FunctionCallExpression x ->
    handle_function_call_expression x
  | EvalExpression x ->
    let (kw, left_p, arg, right_p) = get_eval_expression_children x in
    t kw;
    transform_braced_item left_p arg right_p;
  | EmptyExpression x ->
    let (kw, left_p, arg, right_p) = get_empty_expression_children x in
    t kw;
    transform_braced_item left_p arg right_p;
  | IssetExpression x ->
    let (kw, left_p, args, right_p) = get_isset_expression_children x in
    t kw;
    transform_argish left_p args right_p;
  | DefineExpression x ->
    let (kw, left_p, args, right_p) = get_define_expression_children x in
    t kw;
    transform_argish left_p args right_p;
  | ParenthesizedExpression x ->
    let (left_p, expr, right_p) = get_parenthesized_expression_children x in
    t left_p;
    split ();
    tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      tl_with ~nest ~f:(fun () ->
        (* TODO t14945172: remove this
          This is required due to the xhp children declaration split, th
        *)
        handle_possible_list ~after_each:after_each_argument expr;
      ) ();
      t right_p
    ) ();
    ()
  | BracedExpression x ->
    (* TODO: revisit this *)
    let (left_b, expr, right_b) = get_braced_expression_children x in
    t left_b;
    split ();
    tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      t_with ~nest expr;
      split ();
      t right_b
    ) ();
    ()
  | ListExpression x ->
    let (kw, lp, members, rp) = get_list_expression_children x in
    t kw;
    transform_argish lp members rp;
    ()
  | CollectionLiteralExpression x ->
    let (name, left_b, initializers, right_b) =
      get_collection_literal_expression_children x
    in
    t name;
    add_space ();
    t left_b;
    if is_missing initializers then begin
      t right_b;
      ()
    end else begin
      split ~space ();
      tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
        tl_with ~nest ~f:(fun () ->
          handle_possible_list ~after_each:after_each_literal initializers
        ) ();
        t right_b;
      ) ();
      ()
    end
  | ObjectCreationExpression x ->
    let (kw, obj_type, left_p, arg_list, right_p) =
      get_object_creation_expression_children x
    in
    t kw;
    add_space ();
    t obj_type;
    transform_argish left_p arg_list right_p;
    ()
  | ArrayCreationExpression x ->
    let (left_b, members, right_b) = get_array_creation_expression_children x in
    transform_argish left_b members right_b;
  | ArrayIntrinsicExpression x ->
    let (kw, left_p, members, right_p) =
      get_array_intrinsic_expression_children x
    in
    t kw;
    transform_argish left_p members right_p;
    ()
  | DictionaryIntrinsicExpression x ->
    let (kw, left_p, members, right_p) =
      get_dictionary_intrinsic_expression_children x
    in
    t kw;
    transform_argish left_p members right_p;
    ()
  | KeysetIntrinsicExpression x ->
    let (kw, left_p, members, right_p) =
      get_keyset_intrinsic_expression_children x
    in
    t kw;
    transform_argish left_p members right_p;
    ()
  | VectorIntrinsicExpression x ->
    let (kw, left_p, members, right_p) =
      get_vector_intrinsic_expression_children x
    in
    t kw;
    transform_argish left_p members right_p;
    ()
  | ElementInitializer x ->
    let (key, arrow, value) = get_element_initializer_children x in
    transform_mapish_entry key arrow value;
  | SubscriptExpression x ->
    let (receiver, lb, expr, rb) = get_subscript_expression_children x in
    t receiver;
    transform_braced_item lb expr rb;
    ()
  | AwaitableCreationExpression x ->
    let (kw, body) = get_awaitable_creation_expression_children x in
    t kw;
    pending_space ();
    (* TODO: rethink possible one line bodies *)
    (* TODO: correctly handle spacing after the closing brace *)
    handle_possible_compound_statement body;
  | XHPChildrenDeclaration x ->
    let (kw, expr, semi) = get_xhp_children_declaration_children x in
    t kw;
    pending_space ();
    t expr;
    t semi;
    builder#end_chunks();
  | XHPCategoryDeclaration x ->
    let (kw, categories, semi) = get_xhp_category_declaration_children x in
    t kw;
    (* TODO: Eliminate code duplication *)
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list ~before_each:(split ~space) categories;
    ) ();
    t semi;
    builder#end_chunks();
  | XHPEnumType x ->
    let (kw, left_b, values, right_b) = get_xhp_enum_type_children x in
    t kw;
    pending_space ();
    transform_argish left_b values right_b;
  | XHPRequired x ->
    let (at, kw) = get_xhp_required_children x in
    t at;
    t kw;
  | XHPClassAttributeDeclaration x ->
    let (kw, xhp_attributes, semi) =
      get_xhp_class_attribute_declaration_children x in
    t kw;
    tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      handle_possible_list ~before_each:(split ~space) xhp_attributes;
    ) ();
    t semi;
    builder#end_chunks();
  | XHPClassAttribute x ->
    (* TODO: figure out nesting here *)
    let (attr_type, name, init, req) = get_xhp_class_attribute_children x in
    t attr_type;
    pending_space ();
    t name;
    if not (is_missing init) then pending_space ();
    t init;
    if not (is_missing req) then pending_space ();
    t req;
  | XHPSimpleClassAttribute x ->
    let attr_type = get_xhp_simple_class_attribute_children x in
    t attr_type;
  | XHPAttribute x ->
    let (name, eq, expr) = get_xhp_attribute_children x in
    tl_with ~span ~f:(fun () ->
      t name;
      t eq;
      builder#simple_split ~cost:Cost.Assignment ();
      t_with ~nest expr
    ) ();
    ()
  | XHPOpen x ->
    let (name, attrs, right_a) = get_xhp_open_children x in
    t name;
    if not (is_missing attrs) then begin
      split ~space ();
      tl_with ~nest ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
        handle_possible_list ~after_each:(fun is_last ->
          if not is_last then split ~space (); ()
        ) attrs;
      ) ();
    end;
    handle_xhp_open_right_angle_token right_a;
    ()
  | XHPExpression x ->
    (**
     * XHP breaks the normal rules of trivia. If there is a newline after the
     * XHPOpen tag then it becomes leading trivia for the first token in the
     * XHPBody instead of trailing trivia for the open tag.
     *
     * To deal with this we map these newlines to whitespace. We do this instead
     * of removing the trivia in order to maintain an accurate character
     * processed count for partial formatting.
     *)
    let token_has_trailing_trivia_kind trivia_kind node =
      List.exists (Syntax.trailing_trivia node)
        ~f:(fun trivia -> Trivia.kind trivia = trivia_kind)
    in
    let token_has_trailing_newline =
      token_has_trailing_trivia_kind TriviaKind.EndOfLine in
    let token_has_trailing_whitespace =
      token_has_trailing_trivia_kind TriviaKind.WhiteSpace in
    let remove_leading_trivia node =
      let found = ref false in
      let rewritten_node, _ = Rewriter.rewrite_pre (fun rewrite_node ->
        match syntax rewrite_node with
          | Token t when not !found ->
            found := true;
            let new_triv = List.map (EditableToken.leading t) ~f:(fun triv ->
              match Trivia.kind triv with
                | TriviaKind.EndOfLine -> Trivia.make_whitespace @@
                  (String.make (Trivia.width triv) ' ')
                | _ -> triv
            ) in
            Some (
              Syntax.make_token {t with EditableToken.leading = new_triv},
              true
            )
          | _  -> Some (rewrite_node, false)
      ) node in
      rewritten_node
    in

    let handle_xhp_body body =
      let body_with_stripped_hd = remove_leading_trivia body in
      let after_each_with_node node is_last =
        if token_has_trailing_whitespace node then pending_space ();
        if not is_last && token_has_trailing_newline node
          then builder#hard_split ();
      in
      let rec aux l = match l with
        | hd :: tl -> t hd; after_each_with_node hd (List.is_empty tl); aux tl;
        | [] -> ()
      in
      match syntax body_with_stripped_hd with
        | Missing -> ()
        | SyntaxList sl -> split (); aux sl
        | _ -> split (); aux [body]
    in

    let (xhp_open, body, close) = get_xhp_expression_children x in
    let handle_body_close = (fun () ->
      tl_with ~nest ~f:(fun () ->
        handle_xhp_body body
      ) ();
      if not (is_missing close) then split ();
      t (remove_leading_trivia close);
      ()
    ) in

    if builder#has_rule_kind Rule.XHPExpression then begin
      let expr_rule = builder#create_lazy_rule
        ~rule_kind:(Rule.XHPExpression) () in
      t xhp_open;
      tl_with ~rule:(LazyRuleID expr_rule) ~f:(handle_body_close) ();
    end else begin
      tl_with ~rule:(RuleKind Rule.XHPExpression) ~f:(fun () ->
        t xhp_open;
        handle_body_close ();
      ) ();
    end;
    ()
  | XHPClose x ->
    let (left_a, name, right_a) = get_xhp_close_children x in
    t left_a;
    t name;
    t right_a;
  | TypeConstant x ->
    let (left_type, separator, right_type) = get_type_constant_children x in
    t left_type;
    t separator;
    t right_type;
  | VectorArrayTypeSpecifier x ->
    let (kw, left_a, vec_type, right_a) =
      get_vector_array_type_specifier_children x in
    t kw;
    transform_braced_item left_a vec_type right_a;
    ()
  | VectorTypeSpecifier x ->
    let (kw, left_a, vec_type, right_a) =
      get_vector_type_specifier_children x in
    t kw;
    transform_braced_item left_a vec_type right_a;
    ()
  | KeysetTypeSpecifier x ->
    let (kw, left_a, ks_type, right_a) =
      get_keyset_type_specifier_children x in
    t kw;
    transform_braced_item left_a ks_type right_a;
    ()
  | TypeParameter x ->
    let (variance, name, constraints) = get_type_parameter_children x in
    t variance;
    t name;
    if syntax constraints <> Missing then
      pending_space ();
    handle_possible_list constraints;
  | TypeConstraint x ->
    let (kw, constraint_type) = get_type_constraint_children x in
    t kw;
    pending_space ();
    t constraint_type;
  | MapArrayTypeSpecifier x ->
    let (kw, left_a, key, comma_kw, value, right_a) =
      get_map_array_type_specifier_children x in
    t kw;
    let key_list_item = make_list_item key comma_kw in
    let val_list_item = make_list_item value (make_missing ()) in
    let args = make_list [key_list_item; val_list_item] in
    transform_argish ~allow_trailing:false left_a args right_a;
  | DictionaryTypeSpecifier x ->
    let (kw, left_a, members, right_a) =
      get_dictionary_type_specifier_children x
    in
    t kw;
    transform_argish left_a members right_a;
  | ClosureTypeSpecifier x ->
    let (outer_left_p, kw, inner_left_p, param_types, inner_right_p, colon,
      ret_type, outer_right_p) = get_closure_type_specifier_children x in
    t outer_left_p;
    t kw;
    transform_argish_with_return_type ~in_span:false
      inner_left_p param_types inner_right_p colon ret_type;
    t outer_right_p;
  | ClassnameTypeSpecifier x ->
    let (kw, left_a, class_type, right_a) =
      get_classname_type_specifier_children x in
    t kw;
    transform_braced_item left_a class_type right_a;
  | FieldSpecifier x ->
    let (name, arrow_kw, field_type) = get_field_specifier_children x in
    transform_mapish_entry name arrow_kw field_type;
  | FieldInitializer x ->
    let (name, arrow_kw, value) = get_field_initializer_children x in
    transform_mapish_entry name arrow_kw value;
  | ShapeTypeSpecifier x ->
    let (shape_kw, left_p, type_fields, right_p) =
      get_shape_type_specifier_children x in
    t shape_kw;
    transform_argish left_p type_fields right_p;
  | ShapeExpression x ->
    let (shape_kw, left_p, fields, right_p) = get_shape_expression_children x in
    t shape_kw;
    transform_argish left_p fields right_p;
  | TupleExpression x ->
    let (kw, left_p, items, right_p) = get_tuple_expression_children x in
    t kw;
    transform_argish left_p items right_p;
  | GenericTypeSpecifier x ->
    let (class_type, type_args) = get_generic_type_specifier_children x in
    t class_type;
    t type_args;
    ()
  | NullableTypeSpecifier x ->
    let (question, ntype) = get_nullable_type_specifier_children x in
    t question;
    t ntype;
    ()
  | SoftTypeSpecifier x ->
    let (at, stype) = get_soft_type_specifier_children x in
    t at;
    t stype;
    ()
  | TypeArguments x ->
    let (left_a, type_list, right_a) = get_type_arguments_children x in
    transform_argish left_a type_list right_a;
  | TypeParameters x ->
    let (left_a, param_list, right_a) = get_type_parameters_children x in
    transform_argish left_a param_list right_a;
  | TupleTypeSpecifier x ->
    let (left_p, types, right_p) = get_tuple_type_specifier_children x in
    transform_argish left_p types right_p;
  | ErrorSyntax _ ->
    raise Hackfmt_error.InvalidSyntax
  | ListItem x ->
    t x.list_item;
    t x.list_separator
  in
  ()

and transform_and_unnest_closing_brace right_b =
  let token = match syntax right_b with
    | Token x -> x
    | _ -> raise (Failure "expecting node to be a right closing brace")
  in

  builder#closing_brace_token token;
  ()

and tl_with ?(nest=false) ?(rule=NoRule) ?(span=None) ~f () =
  (match rule with
    | NoRule -> ()
    | LazyRuleID id -> builder#start_lazy_rule id
    | RuleKind rule_kind -> builder#start_rule_kind ~rule_kind ()
  );
  if nest then builder#nest ();
  Option.iter span ~f:(fun cost -> builder#start_span ~cost ());

  f ();

  Option.iter span ~f:(fun _ -> builder#end_span ());
  if nest then builder#unnest ();
  (match rule with
    | NoRule -> ()
    | _ -> builder#end_rule ()
  )

and t_with ?(nest=false) ?(rule=NoRule) ?(span=None) ?(f=transform) node =
  tl_with ~nest ~rule ~span ~f:(fun () -> f node) ();
  ()

and after_each_argument is_last =
  split ~space:(not is_last) ();

and after_each_literal is_last =
  split ~space:true ();

and handle_lambda_body node =
  match syntax node with
    | CompoundStatement x ->
      handle_compound_statement x;
    | _ ->
      tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
        split ~space:true ();
        t_with ~nest:true node;
      ) ()

and handle_possible_compound_statement node =
  match syntax node with
    | CompoundStatement x ->
      handle_compound_statement x;
      pending_space ();
      ()
    | _ ->
      builder#end_chunks ();
      builder#start_block_nest ();
      t_with node;
      builder#end_block_nest ();
      ()

and handle_compound_statement cs =
  let (left_b, statements, right_b) = get_compound_statement_children cs in
  pending_space ();
  transform left_b;
  builder#end_chunks ();
  builder#start_block_nest ();
  tl_with ~f:(fun () ->
    handle_possible_list statements;
  ) ();
  transform_and_unnest_closing_brace right_b;
  ()

and handle_possible_list
    ?(before_each=(fun () -> ()))
    ?(after_each=(fun is_last -> ()))
    ?(handle_last=transform)
    node =
  let rec aux l = (
    match l with
      | hd :: [] ->
        before_each ();
        handle_last hd;
        after_each true;
      | hd :: tl ->
        before_each ();
        transform hd;
        after_each false;
        aux tl
      | [] -> ()
  ) in
  match syntax node with
    | Missing -> ()
    | SyntaxList x -> aux x
    | _ -> aux [node]

and handle_xhp_open_right_angle_token t =
  match syntax t with
    | Token token ->
      if EditableToken.text token = "/>" then add_space ();
      transform t
    | _ -> raise (Failure "expected xhp_open right_angle token")

and handle_function_call_expression fce =
  let (receiver, lp, args, rp) = get_function_call_expression_children fce in
  match syntax receiver with
    | MemberSelectionExpression mse ->
      handle_possible_chaining
        (get_member_selection_expression_children mse)
        (Some (lp, args, rp))
    | SafeMemberSelectionExpression smse ->
      handle_possible_chaining
        (get_safe_member_selection_expression_children smse)
        (Some (lp, args, rp))
    | _ ->
      transform receiver;
      transform_argish lp args rp

and handle_possible_chaining (obj, arrow1, member1) argish =
  let rec handle_chaining obj =
    let handle_mse_or_smse (obj, arrow, member) fun_paren_args =
      let (obj, l) = handle_chaining obj in
      obj, l @ [(arrow, member, fun_paren_args)]
    in
    match syntax obj with
      | FunctionCallExpression x ->
        let (receiver, lp, args, rp) =
          get_function_call_expression_children x in
        (match syntax receiver with
          | MemberSelectionExpression mse ->
            handle_mse_or_smse
              (get_member_selection_expression_children mse)
              (Some (lp, args, rp))
          | SafeMemberSelectionExpression smse ->
            handle_mse_or_smse
              (get_safe_member_selection_expression_children smse)
              (Some (lp, args, rp))
          | _ -> obj, []
        )
      | MemberSelectionExpression mse ->
        handle_mse_or_smse
          (get_member_selection_expression_children mse) None
      | SafeMemberSelectionExpression smse ->
        handle_mse_or_smse
          (get_safe_member_selection_expression_children smse) None
      | _ -> obj, []
  in

  let (obj, chain_list) = handle_chaining obj in
  let chain_list = chain_list @ [(arrow1, member1, argish)] in

  let transform_chain (arrow, member, argish) =
    transform arrow;
    transform member;
    Option.iter ~f:(fun (lp, args, rp) -> transform_argish lp args rp) argish;
  in
  (match chain_list with
    | hd :: [] ->
      transform obj;
      builder#simple_split ();
      tl_with ~nest:true ~f:(fun () -> transform_chain hd) ();
    | hd :: tl ->
      let lazy_argument_rule = builder#create_lazy_rule
        ~rule_kind:(Rule.Argument) () in
      transform obj;
      split ();
      tl_with ~nest:true ~rule:(LazyRuleID lazy_argument_rule) ~f:(fun () ->
        transform_chain hd;
        List.iter tl ~f:(fun x -> split (); transform_chain x);
      ) ();
    | _ -> raise (Failure "Expected a chain of at least length 1")
  );
  ()

and handle_switch_body left_b sections right_b =
  add_space ();
  transform left_b;
  builder#end_chunks ();
  builder#start_block_nest ();
  tl_with ~f:(fun () ->
    let handle_fallthrough fallthrough =
      match syntax fallthrough with
      | SwitchFallthrough x ->
        let (kw, semi) = get_switch_fallthrough_children x in
        transform kw;
        transform semi;
        ()
      | _ -> ()
    in
    let handle_label label =
      match syntax label with
      | CaseLabel x ->
        let (kw, expr, colon) = get_case_label_children x in
        transform kw;
        split ~space:true ();
        transform expr;
        transform colon;
        builder#end_chunks ();
        ()
      | DefaultLabel x ->
        let (kw, colon) = get_default_label_children x in
        transform kw;
        transform colon;
        builder#end_chunks ();
        ()
      | _ -> ()
    in

    let handle_statement statement =
      builder#start_block_nest ();
      transform statement;
      builder#end_block_nest ();
      ()
    in

    let handle_section section =
      match syntax section with
      | SwitchSection s ->
        List.iter
          (syntax_node_to_list s.switch_section_labels) ~f:handle_label;
        List.iter
          (syntax_node_to_list s.switch_section_statements) ~f:handle_statement;
        handle_fallthrough s.switch_section_fallthrough;
        ()
      | _ -> ()
    in

    List.iter (syntax_node_to_list sections) ~f:handle_section
  ) ();
  transform_and_unnest_closing_brace right_b;
  ()

and transform_function_declaration_header ~span_started x =
  let (async, kw, amp, name, type_params, leftp, params, rightp, colon,
    ret_type, where) = get_function_declaration_header_children x
  in

  if not span_started then builder#start_span ();

  transform async;
  if not (is_missing async) then add_space ();
  transform kw;
  add_space ();
  transform amp;
  transform name;
  transform type_params;
  transform_argish_with_return_type ~in_span:true leftp params rightp colon
    ret_type;
  transform where;

and transform_argish_with_return_type ~in_span left_p params right_p colon
    ret_type =
  transform left_p;
  split ();
  if in_span then builder#end_span ();

  tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
    tl_with ~nest:true ~f:(fun () ->
      handle_possible_list
      ~after_each:after_each_argument
      ~handle_last:(transform_last_arg ~allow_trailing:true) params
    ) ();
    transform right_p;
    transform colon;
    if not (is_missing colon) then add_space ();
    transform ret_type;
  ) ();
  ()

and transform_argish ?(allow_trailing=true) left_p arg_list right_p =
  transform left_p;
  if not (is_missing arg_list) then begin
    split ();
    tl_with ~span:(Some Cost.Base) ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
      tl_with ~nest:true ~f:(fun () ->
        handle_possible_list
          ~after_each:after_each_argument
          ~handle_last:(transform_last_arg ~allow_trailing) arg_list
      ) ();
      split ();
      transform right_p;
    ) ();
  end else transform right_p;
  ()

and transform_braced_item left_p item right_p =
  transform left_p;
  split ();
  tl_with ~span:(Some Cost.Base) ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
    tl_with ~nest:true ~f:(fun () -> transform item) ();
    split ();
    transform right_p;
  ) ();

and remove_trailing_trivia node =
  let trailing_token = match Syntax.trailing_token node with
    | Some t -> t
    | None -> failwith "Expected trailing token"
  in
  let rewritten_node, changed = Rewriter.rewrite_pre (fun rewrite_node ->
    match syntax rewrite_node with
      | Token t when t == trailing_token ->
        Some (Syntax.make_token {t with EditableToken.trailing = []}, true)
      | _  -> Some (rewrite_node, false)
  ) node in
  if not changed then failwith "Trailing token not rewritten";
  rewritten_node, EditableToken.trailing trailing_token

and transform_last_arg ~allow_trailing node =
  let set_pending_comma () =
    if allow_trailing then builder#set_pending_comma () in
  match syntax node with
    | ListItem x ->
      let (item, separator) = get_list_item_children x in
      (match syntax separator with
        | Token x ->
          transform item;
          set_pending_comma ();
          builder#token_trivia_only x;
        | Missing ->
          let item, trailing_trivia = remove_trailing_trivia item in
          transform item;
          set_pending_comma ();
          builder#handle_trivia ~is_leading:false trailing_trivia;
        | _ -> raise (Failure "Expected separator to be a token");
      );
    | _ ->
      failwith "Expected ListItem"

and transform_mapish_entry key arrow value =
  transform key;
  pending_space ();
  transform arrow;
  builder#simple_space_split ~cost:Cost.Assignment ();
  t_with ~nest:true value;

and transform_keyword_expression_statement kw expr semi =
  transform kw;
  if not (is_missing expr) then begin
    builder#simple_space_split ();
    tl_with ~nest:true ~f:(fun () ->
      transform expr;
    ) ();
  end;
  transform semi;
  builder#end_chunks ();
  ()

and transform_keyword_expr_list_statement kw expr_list semi =
  transform kw;
  tl_with ~nest:true ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
    handle_possible_list ~before_each:(split ~space:true) expr_list;
  ) ();
  transform semi;
  builder#end_chunks ();

and transform_condition left_p condition right_p =
  transform left_p;
  split ();
  tl_with ~rule:(RuleKind Rule.Argument) ~f:(fun () ->
    t_with ~nest:true condition;
    split ();
    transform right_p;
  ) ()

and transform_binary_expression ~is_nested expr =
  let get_operator_type op =
    match syntax op with
      | Token t -> Full_fidelity_operator.trailing_from_token
        (EditableToken.kind t)
      | _ -> raise (Failure "Operator should always be a token")
  in
  let operator_has_surrounding_spaces op =
    match op with
      | None -> failwith "operator_has_surrounding_spaces: Operator expected"
      | Some op ->
        match get_operator_type op with
          | Full_fidelity_operator.ConcatenationOperator -> false
          | _ -> true
  in

  let (left, operator, right) = get_binary_expression_children expr in
  let operator_t = get_operator_type operator in

  if Full_fidelity_operator.is_comparison operator_t then begin
    let lazy_argument_rule =
      builder#create_lazy_rule ~rule_kind:(Rule.Argument) () in
    transform left;
    pending_space ();
    transform operator;
    tl_with ~rule:(LazyRuleID lazy_argument_rule) ~f:(fun () ->
      builder#split true;
      t_with ~nest:true right;
    ) ();
  end else if Full_fidelity_operator.is_assignment operator_t then begin
    transform left;
    pending_space ();
    transform operator;
    builder#simple_space_split ~cost:Cost.Assignment ();
    t_with ~nest:true right;
  end else begin
    let precedence = Full_fidelity_operator.precedence operator_t in

    let rec flatten_expression expr =
      match syntax expr with
        | BinaryExpression x ->
          let (left, operator, right) = get_binary_expression_children x in
          let operator_t = get_operator_type operator in
          let op_precedence = Full_fidelity_operator.precedence operator_t in
          if (op_precedence = precedence) then
            (flatten_expression left) @ (operator :: flatten_expression right)
          else [expr]
        | _ -> [expr]
    in

    let transform_operand operand =
      match syntax operand with
        | BinaryExpression x -> transform_binary_expression ~is_nested:true x
        | _ -> transform operand
    in

    let binary_expresion_syntax_list =
      flatten_expression (make_binary_expression left operator right) in
    match binary_expresion_syntax_list with
      | hd :: tl ->
        let lazy_argument_rule =
          builder#create_lazy_rule ~rule_kind:(Rule.Argument) () in
        transform_operand hd;
        if not is_nested then builder#nest ~skip_parent:true ();
        tl_with ~rule:(LazyRuleID lazy_argument_rule) ~nest:is_nested ~f:(
          fun () ->
            ignore @@ List.foldi tl ~init:None ~f:(fun i last_op x ->
              if (i mod 2) = 0 then begin
                let op = x in
                let op_has_spaces = operator_has_surrounding_spaces (Some op) in
                if op_has_spaces then pending_space ();
                transform op;
                Some op
              end
              else begin
                let operand = x in
                let op_has_spaces = operator_has_surrounding_spaces last_op in
                split ~space:op_has_spaces ();
                transform_operand operand;
                None
              end
            )
        ) ();
        if not is_nested then builder#unnest ();
      | _ ->
        raise (Failure "Expected non empty list of binary expression pieces")
  end

let format_node ?(debug=false) node start_char end_char =
  builder#reset start_char end_char;
  transform node;
  let chunk_groups = builder#_end () in
  chunk_groups

let format_content content =
  let source_text = SourceText.make content in
  let syntax_tree = SyntaxTree.make source_text in
  if not @@ List.is_empty @@ SyntaxTree.errors syntax_tree
    then raise Hackfmt_error.InvalidSyntax;
  let editable = Full_fidelity_editable_syntax.from_tree syntax_tree in
  format_node editable

(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

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

type string_type =
  | DocStringClose
  | Number
  | Concat
  | Other

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
  val mutable pending_space = false;

  val mutable chunk_groups = [];
  val mutable rule_alloc = Rule_allocator.make ();
  val mutable nesting_alloc = Nesting_allocator.make ();
  val mutable span_alloc = Span_allocator.make ();
  val mutable block_indent = 0;

  val mutable seen_chars = 0;
  val mutable last_chunk_end = 0;

  val mutable last_string_type = Other;

  (* TODO: Make builder into an instantiable class instead of
   * having this reset method
   *)
  method private reset () =
    Stack.clear open_spans;
    rules <- [];
    lazy_rules <- ISet.empty;
    chunks <- [];
    next_split_rule <- NoRule;
    next_lazy_rules <- ISet.empty;
    pending_spans <- [];
    space_if_not_split <- false;
    pending_comma <- None;
    pending_space <- false;
    chunk_groups <- [];
    rule_alloc <- Rule_allocator.make ();
    nesting_alloc <- Nesting_allocator.make ();
    span_alloc <- Span_allocator.make ();
    block_indent <- 0;

    seen_chars <- 0;
    last_chunk_end <- 0;

    last_string_type <- Other;
    ()

  method private add_string ?(is_trivia=false) s width =
    if last_string_type = DocStringClose && (is_trivia || s <> ";")
      then this#hard_split ();

    chunks <- (match chunks with
      | hd :: tl when hd.Chunk.is_appendable ->
        let text = hd.Chunk.text ^ (if pending_space then " " else "") ^ s in
        pending_space <- false;
        {hd with Chunk.text = text} :: tl
      | _ -> begin
          space_if_not_split <- pending_space;
          pending_space <- false;
          let nesting = nesting_alloc.Nesting_allocator.current_nesting in
          let handle_started_next_split_rule () =
            let cs =
              Chunk.make s (List.hd rules) nesting last_chunk_end :: chunks in
            this#end_rule ();
            next_split_rule <- NoRule;
            cs
          in
          match next_split_rule with
            | NoRule ->
              Chunk.make s (List.hd rules) nesting last_chunk_end :: chunks
            | LazyRuleID rule_id ->
              this#start_lazy_rule rule_id;
              handle_started_next_split_rule ()
            | RuleKind rule_kind ->
              this#start_rule_kind ~rule_kind ();
              handle_started_next_split_rule ()
      end
    );

    this#advance width;

    if not is_trivia then begin
      lazy_rules <- ISet.union next_lazy_rules lazy_rules;
      next_lazy_rules <- ISet.empty;

      let rev_pending_spans_stack = List.rev pending_spans in
      pending_spans <- [];
      List.iter rev_pending_spans_stack ~f:(fun cost ->
        Stack.push (open_span (List.length chunks - 1) cost) open_spans
      );

      if last_string_type = DocStringClose && s = ";"
        then this#hard_split ();
    end;

    last_string_type <- Other;

  method private set_pending_comma () =
    if last_string_type <> DocStringClose then
      chunks <- (match chunks with
        | hd :: tl when not hd.Chunk.is_appendable ->
          {hd with Chunk.comma_rule = Some (List.hd_exn rules)} :: tl;
        | _ -> pending_comma <- Some (List.hd_exn rules); chunks;
      );

  method private add_space () =
    pending_space <- true;
    if Option.is_some pending_comma then
      this#simple_split_if_unsplit ();

  method private split () =
    chunks <- (match chunks with
      | hd :: tl when hd.Chunk.is_appendable ->
        let rule_id = hd.Chunk.rule in
        let rule = if rule_id <> Rule.null_rule_id then rule_id else
          match rules with
          | [] -> this#create_rule (Rule.Simple Cost.Base)
          | hd :: _ -> hd
        in
        let chunk = Chunk.finalize hd rule rule_alloc
          space_if_not_split pending_comma seen_chars in
        last_chunk_end <- seen_chars;
        space_if_not_split <- pending_space;
        pending_comma <- None;
        chunk :: tl
      | _ -> chunks
    )

  method private create_rule rule_kind =
    let ra, rule = Rule_allocator.make_rule rule_alloc rule_kind in
    rule_alloc <- ra;
    rule.Rule.id

  method private create_lazy_rule ?(rule_kind=Rule.Simple Cost.Base) () =
    let id = this#create_rule rule_kind in
    next_lazy_rules <- ISet.add id next_lazy_rules;
    id

  (* TODO: after unit tests, make this idempotency a property of method split *)
  method private simple_split_if_unsplit () =
    match chunks with
      | hd :: _ when hd.Chunk.is_appendable -> this#simple_split ()
      | _ -> ()

  (* TODO: is this the right whitespace strategy? *)
  method private add_always_empty_chunk () =
    let nesting = nesting_alloc.Nesting_allocator.current_nesting in
    let create_empty_chunk () =
      let rule = this#create_rule Rule.Always in
      let chunk = Chunk.make "" (Some rule) nesting last_chunk_end in
      last_chunk_end <- seen_chars;
      Chunk.finalize chunk rule rule_alloc false None seen_chars
    in
    (chunks <- match chunks with
      | [] -> create_empty_chunk () :: chunks
      | hd :: _ when not hd.Chunk.is_appendable ->
        create_empty_chunk () :: chunks
      (* TODO: this is probably wrong, figure out what this means *)
      | _ -> chunks)

  method private simple_split ?(cost=Cost.Base) () =
    this#split ();
    this#set_next_split_rule (RuleKind (Rule.Simple cost));
    ()

  method private hard_split () =
    begin match chunks with
      | [] -> ()
      | _ ->
        this#split ();
        this#set_next_split_rule (RuleKind Rule.Always);
    end;
    if List.is_empty rules &&
      not (List.is_empty chunks) &&
      not (Nesting_allocator.is_nested nesting_alloc)
    then this#push_chunk_group ()

  method private set_next_split_rule rule_type =
    next_split_rule <- (match next_split_rule with
      | RuleKind Rule.Always -> next_split_rule
      | _ -> rule_type
    )

  method private nest ?(amount=__INDENT_WIDTH) ?(skip_parent=false) () =
    nesting_alloc <- Nesting_allocator.nest nesting_alloc amount skip_parent

  method private unnest () =
    nesting_alloc <- Nesting_allocator.unnest nesting_alloc

  method private start_rule_id rule_id =
    rule_alloc <- Rule_allocator.mark_dependencies
      rule_alloc lazy_rules rules rule_id;
    rules <- rule_id :: rules

  method private start_rule_kind ?(rule_kind=Rule.Simple Cost.Base) () =
    (* Override next_split_rule unless it's an Always rule *)
    next_split_rule <- (match next_split_rule with
      | RuleKind kind when kind <> Rule.Always -> NoRule
      | _ -> next_split_rule
    );
    let rule = this#create_rule rule_kind in
    this#start_rule_id rule

  method private start_lazy_rule lazy_rule_id =
    if ISet.mem lazy_rule_id next_lazy_rules then begin
      next_lazy_rules <- ISet.remove lazy_rule_id next_lazy_rules;
      this#start_rule_id lazy_rule_id
    end else if ISet.mem lazy_rule_id lazy_rules then begin
      lazy_rules <- ISet.remove lazy_rule_id lazy_rules;
      this#start_rule_id lazy_rule_id
    end else
      raise (Failure "Called start_lazy_rule with a rule id is not a lazy rule")

  method private end_rule () =
    rules <- match rules with
      | hd :: tl -> tl
      | [] -> [] (*TODO: error *)

  method private has_rule_kind kind =
    List.exists rules ~f:(fun id ->
      (Rule_allocator.get_rule_kind rule_alloc id) = kind
    )

  method private start_span ?(cost=Cost.Base) () =
    pending_spans <- cost :: pending_spans;

  method private end_span () =
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
  method private start_block_nest () =
    if List.is_empty rules && not (Nesting_allocator.is_nested nesting_alloc)
    then block_indent <- block_indent + 2
    else this#nest ()

  method private end_block_nest () =
    if List.is_empty rules && not (Nesting_allocator.is_nested nesting_alloc)
    then block_indent <- block_indent - 2
    else this#unnest ()

  method private push_chunk_group () =
    chunk_groups <- Chunk_group.({
      chunks = (List.rev chunks);
      rule_map = rule_alloc.Rule_allocator.rule_map;
      rule_dependency_map = rule_alloc.Rule_allocator.dependency_map;
      block_indentation = block_indent;
    }) :: chunk_groups;

    chunks <- [];
    rule_alloc <- Rule_allocator.make ();
    nesting_alloc <- Nesting_allocator.make ();
    span_alloc <- Span_allocator.make ()

  method private _end () =
    this#hard_split ();
    let last_chunk_empty = match chunks with
      | hd :: tl -> hd.Chunk.text = ""
      | [] ->
        match chunk_groups with
        | [] -> true
        | hd :: tl ->
          match List.rev hd.Chunk_group.chunks with
          | [] -> true
          | hd :: tl -> hd.Chunk.text = ""
    in
    if not last_chunk_empty then
      this#add_always_empty_chunk ();
    this#push_chunk_group ();
    List.rev chunk_groups

  method private advance n =
    seen_chars <- seen_chars + n;

  method build_chunk_groups node =
    this#reset ();
    this#consume_fmt_node node;
    this#_end ()

  method private consume_fmt_node node =
    let open Fmt_node in
    match node with
    | Nothing ->
      ()
    | Fmt nodes ->
      List.iter nodes this#consume_fmt_node
    | Text (text, width) ->
      this#add_string text width;
    | Comment (text, width) ->
      this#add_string ~is_trivia:true text width;
    | Ignore (_, width) ->
      this#advance width;
    | DocLiteral node ->
      this#consume_fmt_node node;
      last_string_type <- DocStringClose;
    | NumericLiteral node ->
      if last_string_type = Concat then this#add_space ();
      this#consume_fmt_node node;
      last_string_type <- Number;
    | ConcatOperator node ->
      if last_string_type = Number then this#add_space ();
      this#consume_fmt_node node;
      last_string_type <- Concat;
    | Split ->
      this#split ()
    | SplitWith cost ->
      this#simple_split ~cost ()
    | Newline ->
      this#hard_split ()
    | BlankLine ->
      this#hard_split ();
      this#add_always_empty_chunk ();
    | Space ->
      this#add_space ()
    | Span nodes ->
      this#start_span ();
      List.iter nodes this#consume_fmt_node;
      this#end_span ();
    | Nest nodes ->
      this#nest ();
      List.iter nodes this#consume_fmt_node;
      this#unnest ();
    | ConditionalNest nodes ->
      this#nest ~skip_parent:true ();
      List.iter nodes this#consume_fmt_node;
      this#unnest ();
    | BlockNest nodes ->
      this#start_block_nest ();
      List.iter nodes this#consume_fmt_node;
      this#end_block_nest ();
    | WithRule (rule_kind, action) ->
      this#start_rule_kind ~rule_kind ();
      this#consume_fmt_node action;
      this#end_rule ();
    | WithLazyRule (rule_kind, before, action) ->
      let rule = this#create_lazy_rule ~rule_kind () in
      this#consume_fmt_node before;
      this#start_lazy_rule rule;
      this#consume_fmt_node action;
      this#end_rule ();
    | WithPossibleLazyRule (rule_kind, before, action) ->
      if this#has_rule_kind rule_kind then begin
        let rule = this#create_lazy_rule ~rule_kind () in
        this#consume_fmt_node before;
        this#start_lazy_rule rule;
        this#consume_fmt_node action;
        this#end_rule ();
      end else begin
        this#start_rule_kind ~rule_kind ();
        this#consume_fmt_node before;
        this#consume_fmt_node action;
        this#end_rule ();
      end
    | TrailingComma ->
      this#set_pending_comma ()
end

let build node =
  builder#build_chunk_groups node

(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Format_env

type open_span = { open_span_start: int }

type rule_state =
  | NoRule
  | RuleKind of Rule.kind
  | LazyRuleID of int

let open_span start = { open_span_start = start }

type string_type =
  | DocStringClose
  | Number
  | ConcatOp
  | LineComment
  | Other
[@@deriving eq]

let is_doc_string_close = function
  | DocStringClose -> true
  | Number
  | ConcatOp
  | LineComment
  | Other ->
    false

let builder =
  object (this)
    val open_spans = Stdlib.Stack.create ()

    val mutable env = Env.default

    val mutable rules = []

    val mutable lazy_rules = ISet.empty

    val mutable chunks = []

    val mutable next_split_rule = NoRule

    val mutable next_lazy_rules = ISet.empty

    val mutable num_pending_spans = 0

    val mutable space_if_not_split = false

    val mutable pending_comma = None

    val mutable pending_space = false

    val mutable chunk_groups = []

    val mutable rule_alloc = Rule_allocator.make ()

    val mutable nesting_alloc = Nesting_allocator.make ()

    val mutable span_alloc = Span_allocator.make ()

    val mutable block_indent = 0

    val mutable seen_chars = 0

    val mutable last_chunk_end = 0

    val mutable last_string_type = Other

    val mutable after_next_string = None

    (* TODO: Make builder into an instantiable class instead of
     * having this reset method
     *)
    method private reset new_env =
      env <- new_env;
      Stdlib.Stack.clear open_spans;
      rules <- [];
      lazy_rules <- ISet.empty;
      chunks <- [];
      next_split_rule <- RuleKind Rule.Always;
      next_lazy_rules <- ISet.empty;
      num_pending_spans <- 0;
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

    method private add_string ?(is_trivia = false) ?(multiline = false) s width
        =
      if is_doc_string_close last_string_type && (is_trivia || String.(s <> ";"))
      then
        this#hard_split ();

      chunks <-
        (match chunks with
        | hd :: tl when hd.Chunk.is_appendable ->
          let leading_space = pending_space in
          pending_space <- false;
          let indentable = hd.Chunk.indentable && not multiline in
          let hd = Chunk.{ hd with indentable } in
          let hd = Chunk.add_atom hd ~leading_space s width seen_chars in
          hd :: tl
        | _ ->
          space_if_not_split <- pending_space;
          pending_space <- false;
          let nesting = nesting_alloc.Nesting_allocator.current_nesting in
          let handle_started_next_split_rule () =
            let chunk = Chunk.make (List.hd rules) nesting last_chunk_end in
            let chunk = Chunk.add_atom chunk s width seen_chars in
            let chunk = { chunk with Chunk.indentable = not multiline } in
            let cs = chunk :: chunks in
            this#end_rule ();
            next_split_rule <- NoRule;
            cs
          in
          (match next_split_rule with
          | NoRule ->
            let chunk = Chunk.make (List.hd rules) nesting last_chunk_end in
            let chunk = Chunk.add_atom chunk s width seen_chars in
            let chunk = { chunk with Chunk.indentable = not multiline } in
            chunk :: chunks
          | LazyRuleID rule_id ->
            this#start_lazy_rule rule_id;
            handle_started_next_split_rule ()
          | RuleKind rule_kind ->
            this#start_rule_kind ~rule_kind ();
            handle_started_next_split_rule ()));

      this#advance width;

      if not is_trivia then (
        lazy_rules <- ISet.union next_lazy_rules lazy_rules;
        next_lazy_rules <- ISet.empty;

        for _ = 1 to num_pending_spans do
          Stdlib.Stack.push (open_span (List.length chunks - 1)) open_spans
        done;
        num_pending_spans <- 0;

        if is_doc_string_close last_string_type && String.equal s ";" then (
          (* Normally, we'd have already counted the newline in the semicolon's
           * trailing trivia before splitting. Since here we're splitting before
           * getting the chance to handle that trailing trivia, we temporarily
           * advance to make sure the new chunk has the correct start_char. *)
          this#advance 1;
          this#hard_split ();
          this#advance (-1)
        )
      );

      last_string_type <- Other;

      Option.call ~f:after_next_string ();
      after_next_string <- None

    method private set_pending_comma present_in_original_source =
      if not (is_doc_string_close last_string_type) then (
        let range =
          if not present_in_original_source then
            None
          else
            Some (seen_chars, seen_chars + 1)
        in
        chunks <-
          (match chunks with
          | hd :: tl when not hd.Chunk.is_appendable ->
            { hd with Chunk.comma = Some (List.hd_exn rules, range) } :: tl
          | _ ->
            pending_comma <- Some (List.hd_exn rules, range);
            chunks);
        if Env.version_gte env 3 then last_string_type <- Other
      )

    method private add_space () =
      pending_space <- true;
      if Option.is_some pending_comma then this#simple_split_if_unsplit ()

    method private split () =
      chunks <-
        (match chunks with
        | hd :: tl when hd.Chunk.is_appendable ->
          let rule_id = hd.Chunk.rule in
          let rule =
            if rule_id <> Rule.null_rule_id then
              rule_id
            else
              match rules with
              | [] -> this#create_rule (Rule.Simple Cost.Base)
              | hd :: _ -> hd
          in
          let chunk =
            Chunk.finalize
              hd
              rule
              rule_alloc
              space_if_not_split
              pending_comma
              seen_chars
          in
          last_chunk_end <- seen_chars;
          space_if_not_split <- pending_space;
          pending_comma <- None;
          chunk :: tl
        | _ -> chunks)

    method private create_rule rule_kind =
      let (ra, rule) = Rule_allocator.make_rule rule_alloc rule_kind in
      rule_alloc <- ra;
      rule.Rule.id

    method private create_lazy_rule ?(rule_kind = Rule.Simple Cost.Base) () =
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
        let chunk = Chunk.make (Some rule) nesting last_chunk_end in
        let chunk = Chunk.add_atom chunk "" 0 last_chunk_end in
        last_chunk_end <- seen_chars;
        Chunk.finalize chunk rule rule_alloc false None seen_chars
      in
      chunks <-
        (match chunks with
        | [] -> create_empty_chunk () :: chunks
        | hd :: _ when not hd.Chunk.is_appendable ->
          create_empty_chunk () :: chunks
        (* TODO: this is probably wrong, figure out what this means *)
        | _ -> chunks)

    method private simple_split ?(cost = Cost.Base) () =
      this#split ();
      this#set_next_split_rule (RuleKind (Rule.Simple cost));
      ()

    method private is_at_chunk_group_boundry () =
      List.is_empty rules
      && ISet.is_empty lazy_rules
      && ISet.is_empty next_lazy_rules
      && not (Nesting_allocator.is_nested nesting_alloc)

    method private hard_split () =
      begin
        match chunks with
        | [] -> ()
        | _ ->
          this#split ();
          this#set_next_split_rule (RuleKind Rule.Always)
      end;
      if this#is_at_chunk_group_boundry () && not (List.is_empty chunks) then
        this#push_chunk_group ()

    method private set_next_split_rule rule_type =
      next_split_rule <-
        (match next_split_rule with
        | RuleKind Rule.Always -> next_split_rule
        | _ -> rule_type)

    method private nest ?(skip_parent = false) () =
      nesting_alloc <- Nesting_allocator.nest nesting_alloc skip_parent

    method private unnest () =
      nesting_alloc <- Nesting_allocator.unnest nesting_alloc

    method private start_rule_id rule_id =
      rule_alloc <-
        Rule_allocator.mark_dependencies rule_alloc lazy_rules rules rule_id;
      rules <- rule_id :: rules

    method private start_rule_kind ?(rule_kind = Rule.Simple Cost.Base) () =
      (* Override next_split_rule unless it's an Always rule *)
      next_split_rule <-
        (match next_split_rule with
        | RuleKind kind when not (Rule.is_always kind) -> NoRule
        | _ -> next_split_rule);
      let rule = this#create_rule rule_kind in
      this#start_rule_id rule

    method private start_lazy_rule lazy_rule_id =
      if ISet.mem lazy_rule_id next_lazy_rules then (
        next_lazy_rules <- ISet.remove lazy_rule_id next_lazy_rules;
        this#start_rule_id lazy_rule_id
      ) else if ISet.mem lazy_rule_id lazy_rules then (
        lazy_rules <- ISet.remove lazy_rule_id lazy_rules;
        this#start_rule_id lazy_rule_id
      ) else
        raise
          (Failure "Called start_lazy_rule with a rule id is not a lazy rule")

    method private end_rule () =
      rules <-
        (match rules with
        | _ :: tl -> tl
        | [] -> [])

    (*TODO: error *)
    method private has_rule_kind kind =
      List.exists rules ~f:(fun id ->
          Rule.equal_kind (Rule_allocator.get_rule_kind rule_alloc id) kind)

    method private start_span () = num_pending_spans <- num_pending_spans + 1

    method private end_span () =
      num_pending_spans <-
        (match num_pending_spans with
        | 0 ->
          let os = Stdlib.Stack.pop open_spans in
          let (sa, span) = Span_allocator.make_span span_alloc in
          span_alloc <- sa;
          let r_chunks = List.rev chunks in
          chunks <-
            List.rev_mapi r_chunks ~f:(fun n x ->
                if n <= os.open_span_start then
                  x
                else
                  (* TODO: handle required hard splits *)
                  { x with Chunk.spans = span :: x.Chunk.spans });
          0
        | _ -> num_pending_spans - 1)

    (*
    TODO: find a better way to represt the rules empty case
    for end chunks and block nesting
  *)
    method private start_block_nest () =
      if this#is_at_chunk_group_boundry () then
        block_indent <- block_indent + 1
      else
        this#nest ()

    method private end_block_nest () =
      if this#is_at_chunk_group_boundry () then
        block_indent <- block_indent - 1
      else
        this#unnest ()

    method private push_chunk_group () =
      chunk_groups <-
        Chunk_group.
          {
            chunks = List.rev chunks;
            rule_map = rule_alloc.Rule_allocator.rule_map;
            rule_dependency_map = rule_alloc.Rule_allocator.dependency_map;
            block_indentation = block_indent;
          }
        :: chunk_groups;

      chunks <- [];
      rule_alloc <- Rule_allocator.make ();
      nesting_alloc <- Nesting_allocator.make ();
      span_alloc <- Span_allocator.make ()

    method private _end () =
      this#hard_split ();
      if not (List.is_empty chunks) then
        failwith
          ("The impossible happened: Chunk_builder attempted to end "
          ^ "when not at a chunk group boundary");
      List.rev chunk_groups

    method private advance n = seen_chars <- seen_chars + n

    method build_chunk_groups env node =
      this#reset env;
      this#consume_doc node;
      this#_end ()

    method private consume_doc node =
      Doc.(
        match node with
        | Nothing -> ()
        | Concat nodes -> List.iter nodes ~f:this#consume_doc
        | Text (text, width) -> this#add_string text width
        | Comment (text, width) -> this#add_string ~is_trivia:true text width
        | SingleLineComment (text, width) ->
          this#add_string ~is_trivia:true text width;
          last_string_type <- LineComment
        | Ignore (_, width) -> this#advance width
        | MultilineString (strings, width) ->
          let prev_seen = seen_chars in
          begin
            match strings with
            | hd :: tl ->
              this#add_string hd (String.length hd);
              List.iter tl ~f:(fun s ->
                  this#advance 1;
                  this#hard_split ();
                  this#add_string ~multiline:true s (String.length s))
            | [] -> ()
          end;
          seen_chars <- prev_seen + width
        | DocLiteral node ->
          if not (Env.version_gte env 3) then
            this#set_next_split_rule (RuleKind (Rule.Simple Cost.Base));
          this#consume_doc node;
          last_string_type <- DocStringClose
        | NumericLiteral node ->
          if equal_string_type last_string_type ConcatOp then this#add_space ();
          this#consume_doc node;
          last_string_type <- Number
        | ConcatOperator node ->
          if equal_string_type last_string_type Number then this#add_space ();
          this#consume_doc node;
          last_string_type <- ConcatOp
        | Split -> this#split ()
        | SplitWith cost -> this#simple_split ~cost ()
        | Newline -> this#hard_split ()
        | BlankLine ->
          this#hard_split ();
          this#add_always_empty_chunk ()
        | Space -> this#add_space ()
        | Span nodes ->
          this#start_span ();
          List.iter nodes ~f:this#consume_doc;
          this#end_span ()
        | Nest nodes ->
          this#nest ();
          List.iter nodes ~f:this#consume_doc;
          this#unnest ()
        | ConditionalNest nodes ->
          this#nest ~skip_parent:true ();
          List.iter nodes ~f:this#consume_doc;
          this#unnest ()
        | BlockNest nodes ->
          this#start_block_nest ();
          List.iter nodes ~f:this#consume_doc;
          this#end_block_nest ()
        | WithRule (rule_kind, body) ->
          this#start_rule_kind ~rule_kind ();
          this#consume_doc body;
          this#end_rule ()
        | WithLazyRule (rule_kind, before, body) ->
          let rule = this#create_lazy_rule ~rule_kind () in
          this#consume_doc before;
          this#start_lazy_rule rule;
          this#consume_doc body;
          this#end_rule ()
        | WithOverridingParentalRule body ->
          let start_parental_rule =
            this#start_rule_kind ~rule_kind:Rule.Parental
          in
          (* If we have a pending split which is not a hard split, replace it with a
             Parental rule, which will break if the body breaks. *)
          let override_independent_split =
            match next_split_rule with
            | RuleKind Rule.Always -> false
            | RuleKind _ -> true
            | _ -> false
          in
          (* Starting a new rule will override the independent split, controlling
             that split with the new Parental rule instead. *)
          if override_independent_split then start_parental_rule ();

          (* Regardless of whether we intend to override the preceding split, start
             a new Parental rule to govern the contents of the body. *)
          after_next_string <- Some start_parental_rule;
          this#consume_doc body;
          this#end_rule ();
          if override_independent_split then this#end_rule ()
        | TrailingComma present_in_original_source ->
          if not (equal_string_type last_string_type LineComment) then
            this#set_pending_comma present_in_original_source
          else (
            this#add_string "," 1;
            this#advance (-1)
          );
          if not (Env.version_gte env 3) then last_string_type <- Other)
  end

let build env node = builder#build_chunk_groups env node

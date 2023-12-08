(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t =
  | Nothing  (** Null case *)
  | Concat of t list  (** List of concatenated nodes *)
  | Text of string * int  (** Text which will appear in the formatted file *)
  | Comment of string * int
      (** A comment which will appear in the formatted file *)
  | SingleLineComment of string * int
      (** A comment which must be followed by a Newline. No text may be inserted
      between this comment and the Newline following it. *)
  | Ignore of string * int
      (** Text from the original source which will not appear in the formatted file.
      Keeping track of these allows range formatting. *)
  | Split  (** Breaking point for the line solver *)
  | SplitWith of Cost.t
      (** Allow splitting here, but with the given cost instead of the cost of the
      nearest containing rule *)
  | Newline
      (** Always split here. Consecutive Newlines are ignored by the Chunk_builder
      (use BlankLine to insert a blank line). *)
  | BlankLine  (** Always split here and leave a blank line *)
  | Space  (** Add a space *)
  | Span of t list  (** Items in a Span prefer to stay on the same line *)
  | Nest of t list  (** Nested items are indented if they break across lines *)
  | BlockNest of t list  (** All lines in a BlockNest are indented *)
  | WithRule of Rule.kind * t
      (** Splits in a WithRule region will default to using that region's rule *)
  | WithLazyRule of Rule.kind * t * t (*** Special cases ***)
      (** Splits in a WithLazyRule region will default to using this lazy rule.
      Lazy rules are necessary when needing to emit formatted text between the
      location where the rule's dependencies are computed and the region to which
      the lazy rule applies. The first Doc is for content before the lazy rule
      becomes active, while the second is for content the lazy rule applies
      to. *)
  | MultilineString of string list * int
      (** Tokens representing part of a string literal spanning multiple lines are
      split on newlines and passed as a MultilineString. These strings are not
      indented (since indenting would insert whitespace into the literal). *)
  | DocLiteral of t
      (** Heredoc and Nowdoc literals end with a closing identifier, which must be
      the only characters on their line (other than a semicolon). Chunk_builder
      needs to know if the last string was a docstring close in order to ensure a
      newline after it (except if the next token is a semicolon). *)
  | NumericLiteral of t
      (** Chunk_builder needs to know if the last literal was numeric in order to
      avoid adding a concat operator directly next to it (since it would then be
      parsed as a decimal point). *)
  | ConcatOperator of t
      (** Chunk_builder needs to know if the last token was a concat operator in
      order to avoid adding a numeric literal directly next to it (since it would
      then be parsed as a decimal point). *)
  | ConditionalNest of t list
      (** Set Nesting.skip_parent_if_nested on this nesting *)
  | WithOverridingParentalRule of t
      (** Splits in a WithOverridingParentalRule region are controlled by a Parental
     rule. If an independent split (i.e., SplitWith) precedes the region,
     override that split to be governed by the Parental rule instead.

  This is useful for XHP, where we would never like to join multiline XHP
  expressions onto a previous line:

    $x = <p>
      foo
    </p>;

  Instead, it is conventional to start a multiline XHP expression on a new
  line:

    $x =
      <p>
        foo
      </p>;

  Using WithOverridingParentalRule overrides the independent split following
  the assignment operator, causing it to be governed by the Parental rule
  governing the rest of the splits in this region instead of the Simple rule
  originally created for the independent split.

  Note that this "overriding" behavior is actually the *default* behavior of
  WithRule--if a split is pending when a WithRule region begins, that split
  will be considered to be part of the WithRule region, even if it was
  already associated with another rule. What is special about the
  WithOverridingParentalRule construct is that it *doesn't* trample the rule
  association of a preceding Split which is not independent.

  Yes, this is confusing. Diffs which make this more sensible and consistent
  would be more than welcome! *)
  | TrailingComma of bool
      (** Add a comma only if the next split is broken on. The bool indicates whether
      a trailing comma was present in the original source. *)

let is_nothing = function
  | Nothing -> true
  | _ -> false

let space () = Space

let split () = Split

let space_split () = Concat [Space; Split]

let newline () = Newline

let text s = Text (s, String.length s)

let rec has_printable_content node =
  match node with
  | Text _
  | Comment _
  | SingleLineComment _
  | MultilineString _
  | DocLiteral _
  | NumericLiteral _
  | ConcatOperator _ ->
    true
  | Concat nodes
  | Span nodes
  | Nest nodes
  | ConditionalNest nodes
  | BlockNest nodes ->
    List.exists nodes ~f:has_printable_content
  | WithRule (_, body)
  | WithOverridingParentalRule body ->
    has_printable_content body
  | WithLazyRule (_, before, body) ->
    has_printable_content before || has_printable_content body
  | Nothing
  | Ignore _
  | Split
  | SplitWith _
  | Newline
  | BlankLine
  | Space
  | TrailingComma _ ->
    false

let rec has_split node =
  match node with
  | Split
  | SplitWith _
  | Newline
  | BlankLine
  | MultilineString _
  | DocLiteral _ ->
    true
  | Concat nodes
  | Span nodes
  | Nest nodes
  | ConditionalNest nodes
  | BlockNest nodes ->
    List.exists nodes ~f:has_split
  | WithRule (_, body)
  | WithOverridingParentalRule body ->
    has_split body
  | WithLazyRule (_, before, body) -> has_split before || has_split body
  | Text _
  | Comment _
  | SingleLineComment _
  | NumericLiteral _
  | ConcatOperator _
  | Nothing
  | Ignore _
  | Space
  | TrailingComma _ ->
    false

(** Add "dump @@" before any Doc.t expression to dump it to stderr. *)
let dump ?(ignored = false) node =
  Printf.(
    let rec aux = function
      | Nothing -> ()
      | Concat nodes -> List.iter nodes ~f:aux
      | Text (text, _) -> print (sprintf "Text \"%s\"" text)
      | Comment (text, _) -> print (sprintf "Comment \"%s\"" text)
      | SingleLineComment (text, _) ->
        print (sprintf "SingleLineComment \"%s\"" text)
      | Ignore (text, _) ->
        if ignored then
          print
            (sprintf
               "Ignored \"%s\""
               (String.concat
                  ~sep:"\\n"
                  (Str.split_delim (Str.regexp "\n") text)))
      | MultilineString (strings, _) ->
        print "MultilineString [";
        indent := !indent + 2;
        List.iter strings ~f:(fun s -> print (sprintf "\"%s\"" s));
        indent := !indent - 2;
        print "]"
      | DocLiteral node -> dump_list "DocLiteral" [node]
      | NumericLiteral node -> dump_list "NumericLiteral" [node]
      | ConcatOperator node -> dump_list "ConcatOperator" [node]
      | Split -> print "Split"
      | SplitWith cost ->
        print
          ("SplitWith "
          ^
          match cost with
          | Cost.NoCost -> "Cost.NoCost"
          | Cost.Base -> "Cost.Base"
          | Cost.Moderate -> "Cost.Moderate"
          | Cost.High -> "Cost.High")
      | Newline -> print "Newline"
      | BlankLine -> print "BlankLine"
      | Space -> print "Space"
      | Span nodes -> dump_list "Span" nodes
      | Nest nodes -> dump_list "Nest" nodes
      | ConditionalNest nodes -> dump_list "ConditionalNest" nodes
      | BlockNest nodes -> dump_list "BlockNest" nodes
      | WithRule (_, body) -> dump_list "WithRule" [body]
      | WithLazyRule (_, before, body) ->
        print "WithLazyRule ([";
        dump_list_items [before];
        print "], [";
        dump_list_items [body];
        print "])"
      | WithOverridingParentalRule body ->
        print "WithOverridingParentalRule ([";
        dump_list_items [body];
        print "])"
      | TrailingComma present_in_original_source ->
        print (sprintf "TrailingComma %b" present_in_original_source)
    and indent = ref 0
    and print s = eprintf "%s%s\n" (String.make !indent ' ') s
    and dump_list name nodes =
      print
        (if String.equal name "" then
          "["
        else
          name ^ " [");
      dump_list_items nodes;
      print "]"
    and dump_list_items nodes =
      indent := !indent + 2;
      List.iter nodes ~f:aux;
      indent := !indent - 2
    in
    if is_nothing node then
      print "Nothing"
    else
      aux node;
    eprintf "%!";
    node)

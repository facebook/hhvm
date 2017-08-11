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

type t =
  (* Null case *)
  | Nothing
  (* List of concatenated nodes *)
  | Concat of t list
  (* Text which will appear in the formatted file *)
  | Text of string * int
  (* A comment which will appear in the formatted file *)
  | Comment of string * int
  (* Text from the original source which will not appear in the formatted file.
   * Keeping track of these allows range formatting. *)
  | Ignore of string * int
  (* Breaking point for the line solver *)
  | Split
  (* Allow splitting here, but with the given cost instead of the cost of the
   * nearest containing rule *)
  | SplitWith of Cost.t
  (* Always split here. Consecutive Newlines are ignored by the Chunk_builder
   * (use BlankLine to insert a blank line). *)
  | Newline
  (* Always split here and leave a blank line *)
  | BlankLine
  (* Add a space *)
  | Space
  (* Items in a Span prefer to stay on the same line *)
  | Span of t list
  (* Nested items are indented if they break across lines *)
  | Nest of t list
  (* All lines in a BlockNest are indented *)
  | BlockNest of t list
  (* Splits in a WithRule region will default to using that region's rule *)
  | WithRule of Rule.kind * t
  (* Splits in a WithLazyRule region will default to using this lazy rule.
   * Lazy rules are necessary when needing to emit formatted text between the
   * location where the rule's dependencies are computed and the region to which
   * the lazy rule applies. The first Doc is for content before the lazy rule
   * becomes active, while the second is for content the lazy rule applies
   * to. *)
  | WithLazyRule of Rule.kind * t * t

  (*** Special cases ***)

  (* Tokens representing part of a string literal spanning multiple lines are
   * split on newlines and passed as a MultilineString. These strings are not
   * indented (since indenting would insert whitespace into the literal). *)
  | MultilineString of string list * int
  (* Heredoc and Nowdoc literals end with a closing identifier, which must be
   * the only characters on their line (other than a semicolon). Chunk_builder
   * needs to know if the last string was a docstring close in order to ensure a
   * newline after it (except if the next token is a semicolon). *)
  | DocLiteral of t
  (* Chunk_builder needs to know if the last literal was numeric in order to
   * avoid adding a concat operator directly next to it (since it would then be
   * parsed as a decimal point). *)
  | NumericLiteral of t
  (* Chunk_builder needs to know if the last token was a concat operator in
   * order to avoid adding a numeric literal directly next to it (since it would
   * then be parsed as a decimal point). *)
  | ConcatOperator of t
  (* Set Nesting.skip_parent_if_nested on this nesting *)
  | ConditionalNest of t list
  (* Enable this lazy rule only if we are within another region with the same
   * rule kind. Otherwise, enable the rule as a non-lazy rule. *)
  | WithPossibleLazyRule of Rule.kind * t * t
  (* Add a comma only if the next split is broken on *)
  | TrailingComma

let space () = Space
let split () = Split
let space_split () = Concat [Space; Split]
let newline () = Newline

let rec has_printable_content node =
  match node with
  | Text _
  | Comment _
  | MultilineString _
  | DocLiteral _
  | NumericLiteral _
  | ConcatOperator _ -> true

  | Concat nodes
  | Span nodes
  | Nest nodes
  | ConditionalNest nodes
  | BlockNest nodes ->
    List.exists nodes has_printable_content

  | WithRule (_, action) ->
    has_printable_content action

  | WithLazyRule (_, before, action)
  | WithPossibleLazyRule (_, before, action) ->
    has_printable_content before || has_printable_content action

  | Nothing
  | Ignore _
  | Split
  | SplitWith _
  | Newline
  | BlankLine
  | Space
  | TrailingComma -> false

(* Add "dump @@" before any Doc.t expression to dump it to stderr. *)
let dump ?(ignored=false) node =
  let open Printf in
  let rec aux = function
    | Nothing ->
      ()
    | Concat nodes ->
      List.iter nodes aux
    | Text (text, _) ->
      print (sprintf "Text \"%s\"" text)
    | Comment (text, _) ->
      print (sprintf "Comment \"%s\"" text)
    | Ignore (text, _) ->
      if ignored then
        print (sprintf "Ignored \"%s\""
          (String.concat "\\n"
            (Str.split_delim (Str.regexp "\n") text)))
    | MultilineString (strings, _) ->
      print "MultilineString [";
      indent := !indent + 2;
      List.iter strings (fun s -> print (sprintf "\"%s\"" s));
      indent := !indent - 2;
      print "]";
    | DocLiteral node ->
      dump_list "DocLiteral" [node]
    | NumericLiteral node ->
      dump_list "NumericLiteral" [node]
    | ConcatOperator node ->
      dump_list "ConcatOperator" [node]
    | Split ->
      print "Split"
    | SplitWith cost ->
      print ("SplitWith " ^ match cost with
        | Cost.NoCost -> "Cost.NoCost"
        | Cost.Base -> "Cost.Base"
        | Cost.SimpleMemberSelection -> "Cost.SimpleMemberSelection")
    | Newline ->
      print "Newline"
    | BlankLine ->
      print "BlankLine"
    | Space ->
      print "Space"
    | Span nodes ->
      dump_list "Span" nodes
    | Nest nodes ->
      dump_list "Nest" nodes
    | ConditionalNest nodes ->
      dump_list "ConditionalNest" nodes
    | BlockNest nodes ->
      dump_list "BlockNest" nodes
    | WithRule (_, action) ->
      dump_list "WithRule" [action]
    | WithLazyRule (_, before, action) ->
      print "WithLazyRule ([";
      dump_list_items [before];
      print "], [";
      dump_list_items [action];
      print "])";
    | WithPossibleLazyRule (_, before, action) ->
      print "WithPossibleLazyRule ([";
      dump_list_items [before];
      print "], [";
      dump_list_items [action];
      print "])";
    | TrailingComma ->
      print "TrailingComma"
  and indent = ref 0
  and print s = eprintf "%s%s\n" (String.make !indent ' ') s
  and dump_list name nodes =
    print (if name = "" then "[" else (name ^ " ["));
    dump_list_items nodes;
    print "]";
  and dump_list_items nodes =
    indent := !indent + 2;
    List.iter nodes aux;
    indent := !indent - 2;
  in
  if node = Nothing then
    print "Nothing"
  else
    aux node;
  eprintf "%!";
  node

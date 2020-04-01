(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Positioned token
 *
 * A positioned token stores the original source text,
 * the offset of the leading trivia, the width of the leading trivia,
 * node proper, and trailing trivia. From all this information we can
 * rapidly compute the absolute position of any portion of the node,
 * or the text.
 *
 *)

module MinimalToken = Full_fidelity_minimal_token
module MinimalTrivia = Full_fidelity_minimal_trivia
module Trivia = Full_fidelity_positioned_trivia
module SourceText = Full_fidelity_source_text
module TokenKind = Full_fidelity_token_kind
module TriviaKind = Full_fidelity_trivia_kind

module LazyTrivia : sig
  type t [@@deriving show]

  val has_trivia_kind : t -> TriviaKind.t -> bool

  val from : Trivia.t list * Trivia.t list -> t

  val update_trailing : t -> (unit -> Trivia.t list) -> Trivia.t list -> t

  val update_leading : t -> Trivia.t list -> (unit -> Trivia.t list) -> t

  val leading :
    t ->
    (int -> Trivia.MinimalTrivia.t list) ->
    Trivia.SourceText.t ->
    int ->
    int ->
    Trivia.t list

  val trailing :
    t ->
    (int -> Trivia.MinimalTrivia.t list) ->
    Trivia.SourceText.t ->
    int ->
    int ->
    Trivia.t list
end = struct
  (** This looks horrifying, but allow me to explain. For most trivia, we really
    don't care what it is, and even if we do, we can find out what it is by
    running the lexer over the trivia range again. To optimize for this case,
    we only need to store what types of trivia we're surrounded by, and since
    trivia kinds are stored using an enum we can just pack that into an int.

    Unfortunately, there are some types of trivia that are not only incredibly
    important to handle properly, they also can't be regenerated just by
    running the lexer. If any of our trivia is within that set, we need to
    store the trivia itself at all times.

    The standard, reasonable way to represent this would be to use a variant
    of two cases, something like
    [type SurroundingTrivia = Lexable of int | Unlexable of (Trivia.t list * Trivia.t list)].
    Unfortunately, the first case is at least nine orders of magnitude more
    common than the second case, since the second case is only needed for
    [ExtraTokenError], which only show up in incorrect files that wouldn't pass typecheck
    anyway. Therefore, in order to optimize for the general case, we use the
    [Obj] library to store the trivia kinds packed into an int as an immediate
    value, unless we detect that it's not an int, in which case we can pull it
    out as a tuple of leading and trailing trivia. In this way we can go from
    24 bytes in the base case (8 for pointer to variant, 8 for block header,
    8 for int value) to just 8 bytes for the base case.

    Base case: [trivia] is a packed int where the top [n] bits are used to
      store whether a trivia of the [n]th [TriviaKind] is present.
    Special case: [trivia] is [(Trivia.t list * Trivia.t list)], corresponding
      to the leading and trailing trivia. *)
  type t = Obj.t

  (** Internal representation used for printing and pattern matching. *)
  type internal_t =
    | Packed of int
    | Expanded of (Trivia.t list * Trivia.t list)
  [@@deriving show]

  let to_internal trivia =
    if Obj.is_int trivia then
      Packed (Obj.obj trivia)
    else
      Expanded (Obj.obj trivia)

  let from_internal = function
    | Packed trivia -> Obj.repr trivia
    | Expanded trivia -> Obj.repr trivia

  let pp formatter trivia = pp_internal_t formatter (to_internal trivia)

  let show trivia = show_internal_t (to_internal trivia)

  let trivia_kind_mask kind = 1 lsl (62 - TriviaKind.to_enum kind)

  let fold_kind acc kind = acc lor trivia_kind_mask kind

  let get_trivia_kinds trivia_lists =
    let get_trivia_kinds trivia_list =
      let kinds = List.map (fun trivia -> trivia.Trivia.kind) trivia_list in
      List.fold_left fold_kind 0 kinds
    in
    let trivia_kinds = List.map get_trivia_kinds trivia_lists in
    List.fold_left ( lor ) 0 trivia_kinds

  let is_special = function
    | TriviaKind.ExtraTokenError -> true
    | _ -> false

  let has_special trivia_lists =
    trivia_lists
    |> List.exists (List.exists (fun trivia -> is_special trivia.Trivia.kind))

  let has_trivia_kind trivia kind =
    match to_internal trivia with
    | Packed trivia ->
      let kind_mask = trivia_kind_mask kind in
      trivia land kind_mask <> 0
    | Expanded (leading, trailing) ->
      [leading; trailing]
      |> List.exists (List.exists (fun trivia -> trivia.Trivia.kind = kind))

  let from (leading, trailing) =
    let internal =
      if has_special [leading; trailing] then
        Expanded (leading, trailing)
      else
        Packed (get_trivia_kinds [leading; trailing])
    in
    internal |> from_internal

  (** Updates a [LazyTrivia.t] with new values.

      @param trivia the trivia to update
      @param load_func function to load the trivia that we're not updating, if
        the trivia we're passed in is lazy
      @param new_trivia the new trivia which we're using to update
      @param get_func function to get the trivia that we're not updating, if
        the trivia we're passed in is not lazy
      @param update_func function to take the gotten trivia and pack it into a
        new tuple, if the trivia we're passed in is not lazy *)
  let update
      (trivia : t)
      (load_func : unit -> Trivia.t list)
      (new_trivia : Trivia.t list)
      (get_func : Trivia.t list * Trivia.t list -> Trivia.t list)
      (update_func : Trivia.t list -> Trivia.t list * Trivia.t list) : t =
    match (to_internal trivia, has_special [new_trivia]) with
    | (Packed _, true) ->
      let loaded = load_func () in
      from_internal (Expanded (update_func loaded))
    | (Packed trivia, false) ->
      from_internal (Packed (trivia lor get_trivia_kinds [new_trivia]))
    | (Expanded trivia, _) ->
      let loaded = get_func trivia in
      from_internal (Expanded (update_func loaded))

  let update_leading trivia new_leading load_trailing =
    update trivia load_trailing new_leading snd (fun loaded ->
        (new_leading, loaded))

  let update_trailing trivia load_leading new_trailing =
    update trivia load_leading new_trailing fst (fun loaded ->
        (loaded, new_trailing))

  let load_trivia scanner source_text offset width =
    if width = 0 then
      []
    else
      let trivia = scanner offset in
      Trivia.from_minimal_list source_text trivia offset

  let leading trivia scanner source_text offset width =
    match to_internal trivia with
    | Packed _ -> load_trivia scanner source_text offset width
    | Expanded (leading, _) -> leading

  let trailing trivia scanner source_text offset width =
    match to_internal trivia with
    | Packed _ -> load_trivia scanner source_text offset width
    | Expanded (_, trailing) -> trailing
end

type t = {
  kind: TokenKind.t;
  source_text: SourceText.t;
  offset: int;
  (* Beginning of first trivia *)
  leading_width: int;
  width: int;
  (* Width of actual token, not counting trivia *)
  trailing_width: int;
  trivia: LazyTrivia.t;
}
[@@deriving show]

let fold_width sum trivia = sum + Trivia.width trivia

let make kind source_text offset width leading trailing =
  let leading_width = List.fold_left fold_width 0 leading in
  let trailing_width = List.fold_left fold_width 0 trailing in
  let trivia = LazyTrivia.from (leading, trailing) in
  { kind; source_text; offset; leading_width; width; trailing_width; trivia }

let kind token = token.kind

let with_kind token kind = { token with kind }

let has_trivia_kind token = LazyTrivia.has_trivia_kind token.trivia

let source_text token = token.source_text

let leading_width token = token.leading_width

let width token = token.width

let trailing_width token = token.trailing_width

let full_width token = leading_width token + width token + trailing_width token

let is_in_xhp token =
  match token.kind with
  | TokenKind.XHPBody -> true
  | _ -> false

let make_rust_scanner token fn (offset : int) = fn token.source_text offset

let leading token =
  let scanner =
    match is_in_xhp token with
    | true ->
      make_rust_scanner token Rust_lazy_trivia_ffi.scan_leading_xhp_trivia
    | false ->
      make_rust_scanner token Rust_lazy_trivia_ffi.scan_leading_php_trivia
  in
  LazyTrivia.leading
    token.trivia
    scanner
    token.source_text
    token.offset
    token.leading_width

let trailing token =
  let scanner =
    match is_in_xhp token with
    | true ->
      make_rust_scanner token Rust_lazy_trivia_ffi.scan_trailing_xhp_trivia
    | false ->
      make_rust_scanner token Rust_lazy_trivia_ffi.scan_trailing_php_trivia
  in
  let offset = token.offset + token.leading_width + token.width in
  LazyTrivia.trailing
    token.trivia
    scanner
    token.source_text
    offset
    token.trailing_width

let with_leading new_leading token =
  {
    token with
    trivia =
      LazyTrivia.update_leading token.trivia new_leading (fun () ->
          trailing token);
  }

let with_trailing new_trailing token =
  {
    token with
    trivia =
      LazyTrivia.update_trailing
        token.trivia
        (fun () -> leading token)
        new_trailing;
  }

let filter_leading_trivia_by_kind token kind =
  List.filter (fun t -> Trivia.kind t = kind) (leading token)

let leading_start_offset token = token.offset

let leading_end_offset token =
  let w = leading_width token - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  leading_start_offset token + w

let start_offset token = leading_start_offset token + leading_width token

let end_offset token =
  let w = width token - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  start_offset token + w

let trailing_start_offset token =
  leading_start_offset token + leading_width token + width token

let trailing_end_offset token =
  let w = full_width token - 1 in
  let w =
    if w < 0 then
      0
    else
      w
  in
  leading_start_offset token + w

let leading_start_position token =
  SourceText.offset_to_position (source_text token) (leading_start_offset token)

let leading_end_position token =
  SourceText.offset_to_position (source_text token) (leading_end_offset token)

let start_position token =
  SourceText.offset_to_position (source_text token) (start_offset token)

let end_position token =
  SourceText.offset_to_position (source_text token) (end_offset token)

let trailing_start_position token =
  SourceText.offset_to_position
    (source_text token)
    (trailing_start_offset token)

let trailing_end_position token =
  SourceText.offset_to_position (source_text token) (trailing_end_offset token)

let leading_span token =
  (leading_start_position token, leading_end_position token)

let span token = (start_position token, end_position token)

let trailing_span token =
  (trailing_start_position token, trailing_end_position token)

let full_span token = (leading_start_position token, trailing_end_position token)

let full_text token =
  SourceText.sub
    (source_text token)
    (leading_start_offset token)
    (full_width token)

let leading_text token =
  SourceText.sub
    (source_text token)
    (leading_start_offset token)
    (leading_width token)

let trailing_text token =
  SourceText.sub
    (source_text token)
    (end_offset token + 1)
    (trailing_width token)

let text token =
  SourceText.sub (source_text token) (start_offset token) (width token)

let from_minimal source_text minimal_token offset =
  let kind = MinimalToken.kind minimal_token in
  let leading_width = MinimalToken.leading_width minimal_token in
  let width = MinimalToken.width minimal_token in
  let leading =
    Trivia.from_minimal_list
      source_text
      (MinimalToken.leading minimal_token)
      offset
  in
  let trailing =
    Trivia.from_minimal_list
      source_text
      (MinimalToken.trailing minimal_token)
      (offset + leading_width + width)
  in
  make kind source_text offset width leading trailing

let concatenate b e =
  {
    b with
    width = end_offset e + 1 - start_offset b;
    trailing_width = e.trailing_width;
    trivia =
      LazyTrivia.update_trailing b.trivia (fun () -> leading b) (trailing e);
  }

let trim_left ~n t =
  { t with leading_width = t.leading_width + n; width = t.width - n }

let trim_right ~n t =
  { t with trailing_width = t.trailing_width + n; width = t.width - n }

let to_json token =
  Hh_json.(
    let (line_number, _) = start_position token in
    JSON_Object
      [
        ("kind", JSON_String (TokenKind.to_string token.kind));
        ("text", JSON_String (text token));
        ("offset", int_ token.offset);
        ("leading_width", int_ token.leading_width);
        ("width", int_ token.width);
        ("trailing_width", int_ token.trailing_width);
        ("leading", JSON_Array (List.map Trivia.to_json (leading token)));
        ("trailing", JSON_Array (List.map Trivia.to_json (trailing token)));
        ("line_number", int_ line_number);
      ])

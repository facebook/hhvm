(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Empty
  | File of {ic: in_channel; curr: int option ref}
  | And of t * t
  | Or of t * t
  (* Returns all non-equal values of the left iterator
     when compared to the right *)
  | Diff of t * t
  | List of {values: int list ref}

let empty = Empty

let rec close = function
  | Empty | List _ -> ()
  | File {ic; curr} -> curr := None; close_in ic
  | And (l, r) | Or (l, r) | Diff (l, r) -> close l; close r

let read_int_from_file ic =
  let curr = try Some (input_line ic) with End_of_file -> None in
  match curr with
  | None -> None
  | Some curr -> int_of_string_opt curr

let from_file path =
  let ic = try Some (open_in path) with _ -> None in
  match ic with
  | None -> Empty
  | Some ic ->
    let curr = read_int_from_file ic in
    match curr with
    | None -> Empty
    | Some curr ->
      let curr = ref (Some curr) in
      File {ic; curr}

let from_list values =
  if List.length values <> 0 then (List {values = ref values;})
  else Empty

let make_and t1 t2 = And (t1, t2)

let make_or t1 t2 = Or (t1, t2)

let make_diff t1 t2 = Diff (t1, t2)

let rec file_next_gte id ic curr =
  match !curr with
  | None -> None
  | Some curr_val ->
    let next = read_int_from_file ic in
    if next = None then close_in ic;
    curr := next;
    if (curr_val >= id) then Some curr_val
    else file_next_gte id ic curr

let rec file_peek_gte id ic curr =
  match !curr with
  | None -> None
  | Some curr_val ->
    if curr_val >= id
    then Some curr_val
    else
      let next = read_int_from_file ic in
      if next = None then close_in ic;
      curr := next;
      file_peek_gte id ic curr

let rec list_next_gte (id: int) values =
  match !values with
  | hd :: tl ->
    ignore (values := tl);
    if hd >= id then Some hd
    else list_next_gte id values
  | [] -> None

let rec list_peek_gte id values =
  match !values with
  | hd :: tl ->
    if hd >= id then Some hd
    else begin
      ignore (values := tl);
      list_peek_gte id values
    end
  | [] -> None

let rec peek_and l r =
  match peek l, peek r with
  | Some left_val, Some right_val ->
    if left_val = right_val
    then Some left_val
    else begin
      if left_val > right_val
      then ignore (next r)
      else ignore (next l);
      peek_and l r
    end
  | Some _, None -> close l; None
  | None, Some _ -> close r; None
  | None, None -> None

and peek_diff l r =
  match peek l, peek r with
  | Some left_val, Some right_val ->
    if left_val <> right_val
    then begin
      if left_val > right_val
      then begin
        ignore (peek_gte left_val r);
        peek_diff l r
      end
      else Some left_val
    end
    else begin
      ignore (next l);
      ignore (next r);
      peek_diff l r
    end
  | Some left_val, None -> Some left_val
  | None, Some _ -> close r; None
  | None, None -> None

and peek_gte id = function
  | Empty -> None
  | File {ic; curr} -> file_peek_gte id ic curr
  | List {values} -> list_peek_gte id values
  | And (left, right) ->
    let _ = peek_gte id left in
    let _ = peek_gte id right in
    peek_and left right
  | Diff (left, right) ->
    let _ = peek_gte id left in
    let _ = peek_gte id right in
    peek_diff left right
  | Or (left, right) ->
    let left = peek_gte id left in
    let right = peek_gte id right in
    Option.merge left right min

and peek t = peek_gte (-1) t

and next_and l r =
  match peek l, peek r with
  | Some left_val, Some right_val ->
    if left_val = right_val
    then begin
      ignore (next l);
      next r
    end
    else begin
      if left_val > right_val
      then ignore (next r)
      else ignore (next l);
      next_and l r
    end
  | Some _, None -> close l; None
  | None, Some _ -> close r; None
  | None, None -> None

and next_diff l r =
  match peek l, peek r with
  | Some left_val, Some right_val ->
    (* Only values that are the same and greater than the right iterator's value are returned *)
    if left_val <> right_val
    then begin
      if left_val > right_val
      then begin
        ignore (next r);
        next_diff l r
      end
      else next l
    end
    else begin
    (* Advance both iterators since they're the same value *)
      ignore (next l);
      ignore (next r);
      next_diff l r
    end
  | Some _, None -> next l;
  | None, Some _ -> close r; None
  | None, None -> None

and next_gte id = function
  | Empty -> None
  | File {ic; curr} -> file_next_gte id ic curr
  | List {values} -> list_next_gte id values
  | And (left, right) ->
    let _ = peek_gte id left in
    let _ = peek_gte id right in
    next_and left right
  | Diff (left, right) ->
    let _ = peek_gte id left in
    let _ = peek_gte id right in
    next_diff left right
  | Or (left, right) ->
    let left_val = peek_gte id left in
    let right_val = peek_gte id right in
    match left_val, right_val with
    | None, None -> None
    | Some _, None -> next_gte id left
    | None, Some _ -> next_gte id right
    | Some left_val, Some right_val ->
      if left_val < right_val
      then next_gte id left
      else if left_val > right_val
      then next_gte id right
      else begin
        ignore (next_gte id left);
        next_gte id right
      end

and next t = next_gte (-1) t

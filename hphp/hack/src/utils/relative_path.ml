(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections
open Utils
open String_utils

type prefix =
  | Root
  | Hhi
  | Dummy
  | Tmp
[@@deriving eq, show, enum, ord]

let is_hhi = function
  | Hhi -> true
  | Root
  | Dummy
  | Tmp ->
    false

let is_root = function
  | Root -> true
  | Hhi
  | Dummy
  | Tmp ->
    false

let root = ref None

let hhi = ref None

let tmp = ref None

let path_ref_of_prefix = function
  | Root -> root
  | Hhi -> hhi
  | Tmp -> tmp
  | Dummy -> ref (Some "")

let string_of_prefix = function
  | Root -> "root"
  | Hhi -> "hhi"
  | Tmp -> "tmp"
  | Dummy -> ""

let path_of_prefix prefix =
  match !(path_ref_of_prefix prefix) with
  | Some path -> path
  | None ->
    let message =
      Printf.sprintf "Prefix '%s' has not been set!" (string_of_prefix prefix)
    in
    raise (Invalid_argument message)

let set_path_prefix prefix v =
  let v = Path.to_string v in
  assert (String.length v > 0);

  (* Ensure that there is a trailing slash *)
  let v =
    if string_ends_with v Filename.dir_sep then
      v
    else
      v ^ Filename.dir_sep
  in
  match prefix with
  | Dummy -> raise (Failure "Dummy is always represented by an empty string")
  | _ -> path_ref_of_prefix prefix := Some v

type t = prefix * string [@@deriving eq, show, ord]

type relative_path = t

let prefix (p : t) = fst p

let suffix (p : t) = snd p

let default = (Dummy, "")

let is_partial p = String.is_suffix (suffix p) ~suffix:".hackpartial"

(* We could have simply used Marshal.to_string here, but this does slightly
 * better on space usage. *)
let storage_to_string (p, rest) = string_of_prefix p ^ "|" ^ rest

let index_opt str ch = String.index str ch

let storage_of_string str =
  match index_opt str '|' with
  | Some idx ->
    let (a, a') = (0, idx) in
    let b = idx + 1 in
    let b' = String.length str - b in
    let prefix = String.sub str a a' in
    let content = String.sub str b b' in
    let prefix =
      match prefix with
      | "root" -> Root
      | "hhi" -> Hhi
      | "tmp" -> Tmp
      | "" -> Dummy
      | _ -> failwith "invalid prefix"
    in
    (prefix, content)
  | None -> failwith "not a Relative_path.t"

module S = struct
  type t = relative_path

  let compare : t -> t -> int = compare

  let to_string = storage_to_string
end

let to_absolute (p, rest) = path_of_prefix p ^ rest

let to_tmp (_, rest) = (Tmp, rest)

let to_root (_, rest) = (Root, rest)

module Set = struct
  include Reordered_argument_set (Caml.Set.Make (S))

  let pp_limit ?(max_items = None) fmt x =
    Format.fprintf fmt "@[<2>{";
    ignore
    @@ List.fold_left (elements x) ~init:0 ~f:(fun i s ->
           (match max_items with
           | Some max_items when i >= max_items -> ()
           | _ ->
             if i > 0 then Format.fprintf fmt ";@ ";
             pp fmt s);
           i + 1);
    Format.fprintf fmt "@,}@]"

  let pp = pp_limit ~max_items:None

  let show x = Format.asprintf "%a" pp x

  let pp_large ?(max_items = 5) fmt sset =
    let l = cardinal sset in
    if l <= max_items then
      pp fmt sset
    else
      Format.fprintf
        fmt
        "<only showing %d of %d elems: %a>"
        max_items
        l
        (pp_limit ~max_items:(Some max_items))
        sset

  let show_large ?(max_items = 5) sset =
    Format.asprintf "%a" (pp_large ~max_items) sset
end

module Map = struct
  include Reordered_argument_map (WrappedMap.Make (S))

  let pp pp_data = make_pp pp pp_data

  let show pp_data x = Format.asprintf "%a" (pp pp_data) x
end

let create prefix s =
  let prefix_s = path_of_prefix prefix in
  let prefix_len = String.length prefix_s in
  if not (string_starts_with s prefix_s) then (
    Printf.eprintf "%s is not a prefix of %s" prefix_s s;
    assert_false_log_backtrace None
  );
  (prefix, String.sub s prefix_len (String.length s - prefix_len))

let create_detect_prefix s =
  let file_prefix =
    [Root; Hhi; Tmp]
    |> List.find ~f:(fun prefix ->
           String_utils.string_starts_with s (path_of_prefix prefix))
    |> fun x ->
    match x with
    | Some prefix -> prefix
    | None -> Dummy
  in
  create file_prefix s

(* Strips the root and relativizes the file if possible, otherwise returns
  original string *)
let strip_root_if_possible s =
  let prefix_s = path_of_prefix Root in
  let prefix_len = String.length prefix_s in
  if not (string_starts_with s prefix_s) then
    None
  else
    Some (String.sub s prefix_len (String.length s - prefix_len))

let from_root ~(suffix : string) : t = (Root, suffix)

let relativize_set prefix m =
  SSet.fold m ~init:Set.empty ~f:(fun k a -> Set.add a (create prefix k))

let set_of_list xs = List.fold_left xs ~f:Set.add ~init:Set.empty

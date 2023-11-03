(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Set.Make (StringKey)

let pp_limit ?(max_elts = None) fmt sset =
  Format.fprintf fmt "@[<2>{";
  let elements = elements sset in
  (match elements with
  | [] -> ()
  | _ -> Format.fprintf fmt " ");
  let (_ : int) =
    List.fold_left
      (fun i s ->
        (match max_elts with
        | Some max_elts when i >= max_elts -> ()
        | _ ->
          if i > 0 then Format.fprintf fmt ";@ ";
          Format.fprintf fmt "%S" s);
        i + 1)
      0
      elements
  in
  (match elements with
  | [] -> ()
  | _ -> Format.fprintf fmt " ");
  Format.fprintf fmt "@,}@]"

let pp = pp_limit ~max_elts:None

let show sset = Format.asprintf "%a" pp sset

let to_string = show

let pp_large ?(max_elts = 5) fmt sset =
  let l = cardinal sset in
  if l <= max_elts then
    pp fmt sset
  else
    Format.fprintf
      fmt
      "<only showing %d of %d elems: %a>"
      max_elts
      l
      (pp_limit ~max_elts:(Some max_elts))
      sset

let show_large ?(max_elts = 5) sset =
  Format.asprintf "%a" (pp_large ~max_elts) sset

let yojson_of_t t =
  elements t
  |> List.sort StringKey.compare
  |> yojson_of_list StringKey.yojson_of_t

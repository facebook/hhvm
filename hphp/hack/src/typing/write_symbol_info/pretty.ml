(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_context_from_hint ctx h =
  let mode = FileInfo.Mhhi in
  let decl_env = Decl_env.{ mode; droot = None; droot_member = None; ctx } in
  let tcopt = Provider_context.get_tcopt ctx in
  Typing_print.full_decl ~msg:false tcopt (Decl_hint.context_hint decl_env h)

let get_type_from_hint ctx h =
  let mode = FileInfo.Mhhi in
  let decl_env = Decl_env.{ mode; droot = None; droot_member = None; ctx } in
  let tcopt = Provider_context.get_tcopt ctx in
  Typing_print.full_decl ~msg:false tcopt (Decl_hint.hint decl_env h)
  |> Utils.strip_ns

let get_type_from_hint_strip_ns ctx h =
  let mode = FileInfo.Mhhi in
  let decl_env = Decl_env.{ mode; droot = None; droot_member = None; ctx } in
  let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
  Typing_print.full_strip_ns_decl
    ~msg:false
    ~verbose_fun:false
    env
    (Decl_hint.hint decl_env h)

type pos = {
  start: int;
  length: int;
}
[@@deriving ord]

let string_of_type ctx (t : Aast.hint) =
  let queue = Queue.create () in
  let cur = ref 0 in
  let xrefs = ref [] in
  let enqueue ?annot str =
    let length = String.length str in
    let pos = { start = !cur; length } in
    Queue.enqueue queue str;
    cur := !cur + length;
    Option.iter annot ~f:(fun file_pos -> xrefs := (file_pos, pos) :: !xrefs)
  in
  let rec parse t =
    let open Aast in
    match snd t with
    | Hoption t ->
      enqueue "?";
      parse t
    | Hlike t ->
      enqueue "~";
      parse t
    | Hsoft t ->
      enqueue "@";
      parse t
    | Happly ((file_pos, cn), hs) ->
      enqueue ~annot:file_pos (Typing_print.strip_ns cn);
      parse_list ("<", ">") hs
    | Htuple hs -> parse_list ("(", ")") hs
    | Hprim p -> enqueue (Aast_defs.string_of_tprim p)
    | Haccess (h, sids) ->
      parse h;
      List.iter sids ~f:(fun (file_pos, sid) ->
          enqueue "::";
          enqueue ~annot:file_pos sid)
    | _ ->
      (* fall back on old pretty printer - without xrefs - for things
         not implemented yet TODO *)
      enqueue (get_type_from_hint_strip_ns ctx t)
  and parse_list (op, cl) = function
    | [] -> ()
    | [h] ->
      enqueue op;
      parse h;
      enqueue cl
    | h :: hs ->
      enqueue op;
      parse h;
      List.iter hs ~f:(fun h ->
          enqueue ", ";
          parse h);
      enqueue cl
  in
  parse t;
  let toks = Queue.to_list queue in
  (String.concat toks, !xrefs)

let hint_to_string_and_symbols ctx h =
  let ty_pp_ref = get_type_from_hint_strip_ns ctx h in
  let (ty_pp, xrefs) = string_of_type ctx h in
  match String.equal ty_pp ty_pp_ref with
  | true -> (ty_pp, xrefs)
  | false ->
    (* This is triggered only for very large (truncated) types.
       We use ty_pp_ref in that case since it guarantees an
       upper bound on the size of types. *)
    Hh_logger.log "pretty-printers mismatch: %s %s" ty_pp ty_pp_ref;
    (ty_pp_ref, [])

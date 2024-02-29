(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type pos = {
  start: int;
  length: int;
}
[@@deriving ord]

let buf = Buffer.create 1024

let hint_to_string_and_symbols ~is_ctx (hint : Aast.hint) =
  Buffer.reset buf;
  let xrefs = ref [] in
  let append ?annot str =
    let length = String.length str in
    let pos = { start = Buffer.length buf; length } in
    Buffer.add_string buf str;
    Option.iter annot ~f:(fun file_pos -> xrefs := (file_pos, pos) :: !xrefs)
  in
  let parse_gen_seq ~sep ~f = function
    | [] -> ()
    | [x] -> f x
    | x :: xs ->
      f x;
      List.iter xs ~f:(fun x ->
          append sep;
          f x)
  in
  let rec parse ~is_ctx t =
    let open Aast in
    match snd t with
    | Hmixed -> append "mixed"
    | Hwildcard -> append "_"
    | Hnonnull -> append "nonnull"
    | Hthis -> append "this"
    | Hdynamic -> append "dynamic"
    | Hnothing -> append "nothing"
    | Hoption hint ->
      append "?";
      parse ~is_ctx hint
    | Hlike hint ->
      append "~";
      parse ~is_ctx hint
    | Hsoft hint ->
      append "@";
      parse ~is_ctx hint
    | Hvar v -> append v
    | Habstr (name, []) -> append (Typing_print.strip_ns name)
    | Habstr (name, hints) ->
      append (Typing_print.strip_ns name);
      append "<";
      parse_gen_seq ~sep:", " ~f:(parse ~is_ctx) hints;
      append ">"
    | Happly ((_, "\\HH\\supportdyn"), [hint]) -> parse ~is_ctx hint
    | Happly ((file_pos, cn), []) ->
      append ~annot:file_pos (Typing_print.strip_ns cn)
    | Happly ((file_pos, cn), hints) ->
      append ~annot:file_pos (Typing_print.strip_ns cn);
      append "<";
      parse_gen_seq ~sep:", " ~f:(parse ~is_ctx) hints;
      append ">"
    | Htuple hints ->
      append "(";
      parse_gen_seq ~sep:", " ~f:(parse ~is_ctx) hints;
      append ")"
    | Hprim p -> append (Aast_defs.string_of_tprim p)
    | Haccess (hint, sids) ->
      parse ~is_ctx hint;
      List.iter sids ~f:(fun (file_pos, sid) ->
          append "::";
          append ~annot:file_pos sid)
    | Hfun
        {
          hf_param_tys;
          hf_param_info;
          hf_variadic_ty;
          hf_return_ty;
          hf_ctxs;
          _;
        } ->
      append "(function(";
      parse_fun hf_param_tys hf_param_info hf_variadic_ty;
      append ")";
      Option.iter hf_ctxs ~f:(fun (_, ctxs) ->
          append "[";
          parse_gen_seq ~sep:", " ~f:(parse ~is_ctx:true) ctxs;
          append "]");
      append ": ";
      parse ~is_ctx hf_return_ty;
      append ")"
    | Hfun_context name ->
      append "ctx ";
      append (Typing_print.strip_ns name)
    | Hclass_args hint ->
      append "class<";
      parse ~is_ctx hint;
      append ">"
    | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
      append "shape(";
      parse_shape nsi_allows_unknown_fields nsi_field_map;
      append ")"
    | Hrefinement (hint, members) ->
      parse ~is_ctx:false hint;
      append " with { ";
      parse_gen_seq ~sep:"; " ~f:parse_member members;
      append " }"
    | Hvec_or_dict (None, vhint) ->
      append "vec_or_dict";
      append "<";
      parse ~is_ctx vhint;
      append ">"
    | Hvec_or_dict (Some khint, vhint) ->
      append "vec_or_dict";
      append "<";
      parse ~is_ctx khint;
      append ", ";
      parse ~is_ctx vhint;
      append ">"
    | Hunion hints ->
      append "(";
      parse_gen_seq ~sep:"|" ~f:(parse ~is_ctx) hints;
      append ")"
    | Hintersection hints when is_ctx ->
      append "[";
      parse_gen_seq ~sep:", " ~f:(parse ~is_ctx) hints;
      append "]"
    | Hintersection hints ->
      append "(";
      parse_gen_seq ~sep:"&" ~f:(parse ~is_ctx) hints;
      append ")"
  and parse_bound ~is_ctx (kind, hint) =
    let constraint_ =
      match kind with
      | `E -> "= "
      | `L -> "super "
      | `U -> "as "
    in
    append constraint_;
    parse ~is_ctx hint
  and parse_member = function
    | Aast.Rtype (ident, ref) ->
      append "type ";
      append (snd ident);
      append " ";
      let bounds =
        match ref with
        | Aast.TRexact hint -> [(`E, hint)]
        | Aast.TRloose { Aast.tr_lower; tr_upper } ->
          List.map tr_lower ~f:(fun x -> (`L, x))
          @ List.map tr_upper ~f:(fun x -> (`U, x))
      in
      parse_gen_seq ~sep:" " ~f:(parse_bound ~is_ctx:false) bounds
    | Aast.Rctx (ident, ref) ->
      append "ctx ";
      append (snd ident);
      append " ";
      let bounds =
        match ref with
        | Aast.CRexact hint -> [(`E, hint)]
        | Aast.CRloose { Aast.cr_lower; cr_upper } ->
          let opt_map = Option.value_map ~default:[] in
          opt_map cr_lower ~f:(fun x -> [(`L, x)])
          @ opt_map cr_upper ~f:(fun x -> [(`U, x)])
      in
      parse_gen_seq ~sep:" " ~f:(parse_bound ~is_ctx:true) bounds
  and parse_shape open_ = function
    | [] -> if open_ then append "..."
    | hs ->
      parse_gen_seq ~sep:", " ~f:parse_shape_field hs;
      if open_ then append ", ..."
  and parse_shape_field Aast.{ sfi_optional; sfi_name; sfi_hint } =
    if sfi_optional then append "?";
    parse_shape_field_name sfi_name;
    append " => ";
    parse ~is_ctx:false sfi_hint
  and parse_shape_field_name = function
    | Ast_defs.SFlit_int (_, s) -> append s
    | Ast_defs.SFlit_str (_, s) -> append ("'" ^ s ^ "'")
    | Ast_defs.SFclass_const ((pos, c), (_, s)) ->
      append ~annot:pos (Typing_print.strip_ns c);
      append "::";
      append s
  and parse_variadic ~first variadic =
    Option.iter variadic ~f:(fun hint ->
        if not first then append ", ";
        parse ~is_ctx:false hint;
        append "...")
  and parse_fun hf_param_tys hf_param_info variadic =
    (* TODO check if other modifiers are needed *)
    let hf_param_kinds =
      List.map hf_param_info ~f:(fun i ->
          Option.bind i ~f:(fun i ->
              match i.Aast.hfparam_kind with
              | Ast_defs.Pnormal -> None
              | Ast_defs.Pinout p -> Some (Ast_defs.Pinout p)))
    in
    match List.zip_exn hf_param_kinds hf_param_tys with
    | [] -> parse_variadic ~first:true variadic
    | ats ->
      parse_gen_seq ~sep:", " ~f:parse_annotated_hint ats;
      parse_variadic ~first:false variadic
  and parse_annotated_hint (a, hint) =
    Option.iter a ~f:(fun _ -> append "inout ");
    parse ~is_ctx:false hint
  in
  parse ~is_ctx hint;
  (Buffer.contents buf, !xrefs)

let hint_to_string ~is_ctx (hint : Aast.hint) =
  hint_to_string_and_symbols ~is_ctx hint |> fst

let expr_to_string source_text (_, pos, _) =
  let strip_nested_quotes str =
    let len = String.length str in
    let firstc = str.[0] in
    let lastc = str.[len - 1] in
    if
      len >= 2
      && ((Char.equal '"' firstc && Char.equal '"' lastc)
         || (Char.equal '\'' firstc && Char.equal '\'' lastc))
    then
      String.sub str ~pos:1 ~len:(len - 2)
    else
      str
  in
  (* Replace any codepoints that are not valid UTF-8 with
     the unrepresentable character. *)
  let check_utf8 str =
    let b = Buffer.create (String.length str) in
    let replace_malformed () _index = function
      | `Uchar u -> Uutf.Buffer.add_utf_8 b u
      | `Malformed _ -> Uutf.Buffer.add_utf_8 b Uutf.u_rep
    in
    Uutf.String.fold_utf_8 replace_malformed () str;
    Buffer.contents b
  in
  let source_at_span source_text pos =
    let st = Pos.start_offset pos in
    let fi = Pos.end_offset pos in
    let source_text = Full_fidelity_source_text.sub source_text st (fi - st) in
    check_utf8 source_text
  in
  source_at_span source_text pos |> strip_nested_quotes

let strip_tparams name =
  match String.index name '<' with
  | None -> name
  | Some i -> String.sub name ~pos:0 ~len:i

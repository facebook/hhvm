(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hack
open Src

let is_enum_or_enum_class = function
  | Ast_defs.Cenum
  | Ast_defs.Cenum_class _ ->
    true
  | Ast_defs.(Cinterface | Cclass _ | Ctrait) -> false

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

let source_at_span source_text pos =
  let st = Pos.start_offset pos in
  let fi = Pos.end_offset pos in
  let source_text = Full_fidelity_source_text.sub source_text st (fi - st) in
  check_utf8 source_text

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

let strip_tparams name =
  match String.index name '<' with
  | None -> name
  | Some i -> String.sub name ~pos:0 ~len:i

let ends_in_newline source_text =
  let last_char =
    Full_fidelity_source_text.(get source_text (source_text.length - 1))
  in
  Char.equal '\n' last_char || Char.equal '\r' last_char

let has_tabs_or_multibyte_codepoints source_text =
  let open Full_fidelity_source_text in
  let check_codepoint (num, found) _index = function
    | `Uchar u -> (num + 1, found || Uchar.equal u (Uchar.of_char '\t'))
    | `Malformed _ -> (num + 1, true)
  in
  let (num_chars, found_tab_or_malformed) =
    Uutf.String.fold_utf_8 check_codepoint (0, false) source_text.text
  in
  found_tab_or_malformed || num_chars < source_text.length

let split_name (s : string) : (string * string) option =
  match String.rindex s '\\' with
  | None -> None
  | Some pos ->
    let name_start = pos + 1 in
    let name =
      String.sub s ~pos:name_start ~len:(String.length s - name_start)
    in
    let parent_namespace = String.sub s ~pos:0 ~len:(name_start - 1) in
    if String.is_empty parent_namespace || String.is_empty name then
      None
    else
      Some (parent_namespace, name)

let ast_expr_to_string_stripped source_text (_, pos, _) =
  strip_nested_quotes (source_at_span source_text pos)

let ast_expr_to_string source_text (_, pos, _) = source_at_span source_text pos

module Token = Full_fidelity_positioned_syntax.Token

let tokens_to_pos_id st ~hd ~tl =
  let path = st.Full_fidelity_source_text.file_path in
  let start_offset = Token.leading_start_offset hd in
  let name = String.concat ~sep:"\\" (List.map (hd :: tl) ~f:Token.text) in
  let end_offset = start_offset + String.length name in
  let pos =
    Full_fidelity_source_text.relative_pos path st start_offset end_offset
  in
  (pos, name)

exception Ast_error

exception Empty_namespace

let namespace_ast_to_pos_id ns_ast st =
  let open Full_fidelity_positioned_syntax in
  let f item =
    match item.syntax with
    | ListItem { list_item = { syntax = Token t; _ }; _ } -> t
    | _ -> raise Ast_error
  in
  let (hd, tl) =
    match ns_ast with
    | Token t -> (t, [])
    | QualifiedName
        { qualified_name_parts = { syntax = SyntaxList (hd :: tl); _ } } ->
      (f hd, List.map ~f tl)
    | Missing -> raise Empty_namespace
    | _ -> raise Ast_error
  in
  tokens_to_pos_id st ~hd ~tl

type pos = {
  start: int;
  length: int;
}
[@@deriving ord]

(* Pretty-printer for hints. Also generate
   xrefs.  TODO: This covers most of the types but needs
   to be extended OR move the xrefs generartion logic
   to Typing_print.full_strip_ns_decl *)
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
         not implemented yet *)
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

let remove_generated_tparams tparams =
  let param_name Aast_defs.{ tp_name = (_, name); _ } = name in
  Typing_print.split_desugared_ctx_tparams_gen ~tparams ~param_name |> snd

(* Remove leading slash, if present, so names such as
   Exception and \Exception are captured by the same fact *)
let make_name name = Name.Key (Utils.strip_ns name)

let rec make_namespaceqname ns =
  let open NamespaceQName in
  Key
    (match split_name ns with
    | None -> { name = make_name ns; parent = None }
    | Some (parent_ns, namespace) ->
      {
        name = make_name namespace;
        parent = Some (make_namespaceqname parent_ns);
      })

let make_qname qname =
  let open QName in
  Key
    (match split_name qname with
    | None -> { name = make_name qname; namespace_ = None }
    | Some (ns, name) ->
      { name = make_name name; namespace_ = Some (make_namespaceqname ns) })

let make_constraint_kind = function
  | Ast_defs.Constraint_as -> ConstraintKind.As
  | Ast_defs.Constraint_eq -> ConstraintKind.Equal
  | Ast_defs.Constraint_super -> ConstraintKind.Super

let make_visibility = function
  | Aast.Private -> Visibility.Private
  | Aast.Protected -> Visibility.Protected
  | Aast.Public -> Visibility.Public
  | Aast.Internal -> Visibility.Internal

let make_type_const_kind = function
  | Aast.TCAbstract _ -> TypeConstKind.Abstract
  | Aast.TCConcrete _ -> TypeConstKind.Concrete

let make_byte_span pos =
  ByteSpan.{ start = fst (Pos.info_raw pos); length = Pos.length pos }

let make_variance =
  let open Variance in
  function
  | Ast_defs.Contravariant -> Contravariant
  | Ast_defs.Covariant -> Covariant
  | Ast_defs.Invariant -> Invariant

let make_reify_kind =
  let open ReifyKind in
  function
  | Ast_defs.Erased -> Erased
  | Ast_defs.Reified -> Reified
  | Ast_defs.SoftReified -> SoftReified

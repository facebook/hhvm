(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Ast_defs
open Hh_prelude
open Hack
open Src

let method_decl meth_name con_name con_type =
  let qname = Util.make_qname con_name in
  let name = Util.make_name meth_name in
  let container = Predicate.container_decl_qname con_type qname in
  MethodDeclaration.(Key { name; container })

let attributes source_text attrs =
  List.map attrs ~f:(fun attr ->
      let (_, name) = attr.ua_name in
      let name = Util.make_name name in
      let parameters =
        List.fold_right attr.ua_params ~init:[] ~f:(fun expr acc ->
            Pretty.expr_to_string source_text expr :: acc)
      in
      UserAttribute.(Key { name; parameters; qname = None }))

let call_arguments arguments =
  let argument span arg_opt = CallArgument.{ span; argument = arg_opt } in
  let f (fields, last_start) (arg_opt, pos) =
    let (start, _) = Pos.info_raw pos in
    let length = Pos.length pos in
    let rel_span = RelByteSpan.{ offset = start - last_start; length } in
    (argument rel_span arg_opt :: fields, start)
  in
  List.fold arguments ~init:([], 0) ~f |> fst |> List.rev

let constraint_ (kind, hint) =
  let type_string = Pretty.hint_to_string ~is_ctx:false hint in
  Constraint.
    {
      constraint_kind = Util.make_constraint_kind kind;
      type_ = Type.Key type_string;
    }

let signature
    source_text params (ctxs_hints : Aast.contexts option) ~ret_ty ~return_info
    =
  let hint_to_ctx hint =
    Context_.Key (Pretty.hint_to_string ~is_ctx:true hint)
  in
  let f (_pos, hint) = List.map ~f:hint_to_ctx hint in
  let ctxs_hints = Option.map ctxs_hints ~f in
  let param (p, type_xref, ty) =
    Parameter.
      {
        name = Util.make_name p.param_name;
        type_ = Option.map ~f:(fun x -> Type.Key x) ty;
        default_value =
          Option.map p.param_expr ~f:(Pretty.expr_to_string source_text);
        is_inout =
          (match p.param_callconv with
          | Pinout _ -> true
          | Pnormal -> false);
        is_variadic = p.param_is_variadic;
        attributes = attributes source_text p.param_user_attributes;
        type_info = Option.map ~f:(fun x -> TypeInfo.Id x) type_xref;
        readonly =
          Option.map ~f:(fun _ -> ReadonlyKind.Readonly) p.param_readonly;
      }
  in
  let parameters = List.map params ~f:param in
  Signature.(
    Key
      {
        returns = Option.map ~f:(fun x -> Type.Key x) ret_ty;
        parameters;
        contexts = ctxs_hints;
        returns_type_info = Option.map ~f:(fun x -> TypeInfo.Id x) return_info;
      })

let type_param source_text tp =
  let (_, name) = tp.tp_name in
  let name = Util.make_name name in
  let constraints = List.map tp.tp_constraints ~f:constraint_ in
  TypeParameter.
    {
      name;
      variance = Util.make_variance tp.tp_variance;
      reify_kind = Util.make_reify_kind tp.tp_reified;
      constraints;
      attributes = attributes source_text tp.tp_user_attributes;
    }

let generic_xrefs (sym_pos : (XRefTarget.t * Pretty.pos list) Seq.t) =
  let xrefs =
    Stdlib.Seq.fold_left
      (fun acc (target, pos_list) ->
        let sorted_pos = Stdlib.List.sort_uniq Pretty.compare_pos pos_list in
        let (rev_byte_spans, _) =
          List.fold
            sorted_pos
            ~init:([], 0)
            ~f:(fun (spans, last_start) Pretty.{ start; length } ->
              let span = RelByteSpan.{ offset = start - last_start; length } in
              (span :: spans, start))
        in
        let byte_spans = List.rev rev_byte_spans in
        let xref = XRef.{ ranges = byte_spans; target } in
        xref :: acc)
      []
      sym_pos
  in
  (* there's no specified order for xref arrays, but it helps to avoid non-determinism
     when diffing dbs *)
  List.sort ~compare:XRef.compare xrefs

let xrefs (fact_map : Xrefs.fact_map) =
  let f (_fact_id, (target, pos_list)) =
    let util_pos_list =
      List.map pos_list ~f:(fun pos ->
          let start = fst (Pos.info_raw pos) in
          let length = Pos.length pos in
          Pretty.{ start; length })
    in
    (target, util_pos_list)
  in
  let sym_pos = Fact_id.Map.to_seq fact_map |> Stdlib.Seq.map f in
  generic_xrefs sym_pos

let hint_xrefs sym_pos = Stdlib.List.to_seq sym_pos |> generic_xrefs

let module_membership decl_id ~internal =
  ModuleMembership.{ declaration = ModuleDeclaration.Id decl_id; internal }

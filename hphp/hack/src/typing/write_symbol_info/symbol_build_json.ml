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
open Symbol_glean_schema.Hack
open Symbol_glean_schema.Src
module Util = Symbol_json_util
module Fact_id = Symbol_fact_id
module XRefs = Symbol_xrefs
module Predicate = Symbol_predicate

let build_method_decl meth_name con_name con_type =
  let qname = QName.(Key (of_string con_name)) in
  let container = Predicate.container_decl_qname con_type qname in
  MethodDeclaration.(Key { name = Name.Key meth_name; container })

let build_attributes source_text attrs =
  List.map attrs ~f:(fun attr ->
      let (_, name) = attr.ua_name in
      let parameters =
        List.fold_right attr.ua_params ~init:[] ~f:(fun expr acc ->
            Util.ast_expr_to_string_stripped source_text expr :: acc)
      in
      UserAttribute.(Key { name = Name.Key name; parameters; qname = None }))

let build_call_arguments arguments =
  let argument span arg_opt = CallArgument.{ span; argument = arg_opt } in
  let f (fields, last_start) (arg_opt, pos) =
    let (start, _) = Pos.info_raw pos in
    let length = Pos.length pos in
    let rel_span = RelByteSpan.{ offset = start - last_start; length } in
    (argument rel_span arg_opt :: fields, start)
  in
  List.fold arguments ~init:([], 0) ~f |> fst |> List.rev

let build_constraint ctx (kind, hint) =
  let type_string = Util.get_type_from_hint ctx hint in
  Constraint.
    {
      constraintKind = ConstraintKind.of_ast_constraint_kind kind;
      type_ = Type.Key type_string;
    }

let build_signature
    ctx
    source_text
    params
    (ctxs_hints : Aast.contexts option)
    ~ret_ty
    ~return_info =
  let hint_to_ctx hint = Context.Key (Util.get_context_from_hint ctx hint) in
  let f (_pos, hint) = List.map ~f:hint_to_ctx hint in
  let ctxs_hints = Option.map ctxs_hints ~f in
  let build_param (p, type_xref, ty) =
    Parameter.
      {
        name = Name.Key p.param_name;
        type_ = Option.map ~f:(fun x -> Type.Key x) ty;
        defaultValue =
          Option.map p.param_expr ~f:(fun expr ->
              Util.ast_expr_to_string source_text expr);
        isInout =
          (match p.param_callconv with
          | Pinout _ -> true
          | Pnormal -> false);
        isVariadic = p.param_is_variadic;
        attributes = build_attributes source_text p.param_user_attributes;
        typeInfo = Option.map ~f:(fun x -> TypeInfo.Id x) type_xref;
        readonly =
          Option.map ~f:(fun _ -> ReadonlyKind.ReadOnly) p.param_readonly;
      }
  in
  let parameters = List.map params ~f:build_param in
  Signature.(
    Key
      {
        returns = Option.map ~f:(fun x -> Type.Key x) ret_ty;
        parameters;
        contexts = ctxs_hints;
        returnsTypeInfo = Option.map ~f:(fun x -> TypeInfo.Id x) return_info;
      })

let build_type_param ctx source_text tp =
  let (_, name) = tp.tp_name in
  let constraints = List.map tp.tp_constraints ~f:(build_constraint ctx) in
  TypeParameter.
    {
      name = Name.Key name;
      variance = Variance.of_ast_variance tp.tp_variance;
      reifyKind = ReifyKind.of_ast_reifyKind tp.tp_reified;
      constraints;
      attributes = build_attributes source_text tp.tp_user_attributes;
    }

let build_generic_xrefs (sym_pos : (XRefTarget.t * Util.pos list) Seq.t) =
  let xrefs =
    Caml.Seq.fold_left
      (fun acc (target, pos_list) ->
        let sorted_pos = Caml.List.sort_uniq Util.compare_pos pos_list in
        let (rev_byte_spans, _) =
          List.fold
            sorted_pos
            ~init:([], 0)
            ~f:(fun (spans, last_start) Util.{ start; length } ->
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

let build_xrefs (fact_map : XRefs.fact_map) =
  let f (_fact_id, (target, pos_list)) =
    let util_pos_list =
      List.map pos_list ~f:(fun pos ->
          let start = fst (Pos.info_raw pos) in
          let length = Pos.length pos in
          Util.{ start; length })
    in
    (target, util_pos_list)
  in
  let sym_pos = Fact_id.Map.to_seq fact_map |> Caml.Seq.map f in
  build_generic_xrefs sym_pos

let build_hint_xrefs sym_pos = Caml.List.to_seq sym_pos |> build_generic_xrefs

let build_module_membership decl_id ~internal =
  ModuleMembership.{ declaration = ModuleDeclaration.Id decl_id; internal }

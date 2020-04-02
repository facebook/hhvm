(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json
open Tast
open Typing_defs
module Nast = Aast
module Tast = Aast
module Phase = Typing_phase
module Cls = Decl_provider.Class

type autocomplete_data_entry = {
  candidates: string;
  context: string;
  token: string;
}

class visitor =
  object (self)
    inherit Tast_visitor.iter as super

    (* Indicator of whether we are visiting a node inside of a method *)
    val mutable visit_method : bool = false

    (* Acclass_get completion data *)
    val mutable entries : autocomplete_data_entry list = []

    (* Previous tokens as context for each Acclass_get autocompletion *)
    val context_tokens : string list ref = ref []

    (* Candidate list for each Acclass_get autocompletion *)
    val candidates : string list ref = ref []

    method get_entries ctx tast =
      self#go ctx tast;
      entries

    (* Helper functions to extract candidate list
     * These are borrowed from autcompleteService.ml
     *)
    method get_class_elt_types env class_ cid elts =
      let is_visible (_, elt) =
        Tast_env.is_visible env (elt.ce_visibility, get_ce_lsb elt) cid class_
      in
      elts
      |> List.filter ~f:is_visible
      |> List.map ~f:(fun (id, { ce_type = (lazy ty); _ }) -> (id, ty))

    method autocomplete_member ~is_static env class_ cid =
      let match_both_static_and_instance =
        match cid with
        | Some Nast.CIparent -> true
        | _ -> false
      in
      let add _ (name, _) = candidates := name :: !candidates in
      if is_static || match_both_static_and_instance then (
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.smethods class_))
          ~f:(add SearchUtils.SI_ClassMethod);
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.sprops class_))
          ~f:(add SearchUtils.SI_Property);
        List.iter (Cls.consts class_) ~f:(fun (name, cc) ->
            add SearchUtils.SI_ClassConstant (name, cc.cc_type))
      );
      if (not is_static) || match_both_static_and_instance then (
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.methods class_))
          ~f:(add SearchUtils.SI_ClassMethod);
        List.iter
          (self#get_class_elt_types env class_ cid (Cls.props class_))
          ~f:(add SearchUtils.SI_Property)
      )

    method autocomplete_typed_member ~is_static env class_ty cid =
      candidates := [];
      Tast_env.get_class_ids env class_ty
      |> List.iter ~f:(fun cname ->
             Decl_provider.get_class (Tast_env.get_ctx env) cname
             |> Option.iter ~f:(fun class_ ->
                    let cid = Option.map cid to_nast_class_id_ in
                    self#autocomplete_member ~is_static env class_ cid))

    method autocomplete_static_member env ((_, ty), cid) =
      self#autocomplete_typed_member ~is_static:true env ty (Some cid)

    (* Add token to context *)
    method add_token token =
      if visit_method then context_tokens := token :: !context_tokens

    (* Add the completion token and wrap up the context and candidate list *)
    method add_entry token =
      let context = !context_tokens |> List.rev |> String.concat ~sep:" " in
      let candidates = !candidates |> List.rev |> String.concat ~sep:" " in
      let entry = { candidates; context; token } in
      entries <- entry :: entries

    method! on_Id env id =
      self#add_token (snd id);
      super#on_Id env id

    method! on_Lvar env lid =
      let text = Local_id.get_name (snd lid) in
      self#add_token text;
      super#on_Lvar env lid

    method! on_CI env sid =
      self#add_token (snd sid);
      super#on_CI env sid

    method! on_This env =
      self#add_token "$this";
      super#on_This env

    method! on_Class_get env cid mid =
      let res = super#on_Class_get env cid mid in
      (match mid with
      | Tast.CGstring p ->
        self#autocomplete_static_member env cid;
        self#add_token (snd p);
        self#add_entry (snd p)
      | Tast.CGexpr _ -> ());
      res

    method! on_Class_const env cid mid =
      let res = super#on_Class_const env cid mid in
      self#autocomplete_static_member env cid;
      self#add_token (snd mid);
      self#add_entry (snd mid);
      res

    method! on_Obj_get env obj mid ognf =
      let res = super#on_Obj_get env obj mid ognf in
      (match mid with
      | (_, Tast.Id mid) ->
        self#autocomplete_typed_member ~is_static:false env (get_type obj) None;
        self#add_entry (snd mid)
      | _ -> ());
      res

    (* Set the visit_method indicator *)
    method! on_method_ env m =
      visit_method <- true;
      let res = super#on_method_ env m in
      visit_method <- false;
      context_tokens := [];
      res
  end

let serialize_tast tast tcopt =
  let dummy_filename = Relative_path.default in
  let env = Typing_env.empty tcopt dummy_filename ~droot:None in
  let print_pos_and_ty (pos, ty) =
    Format.asprintf "(%a, %s)" Pos.pp pos (Typing_print.full_strip_ns env ty)
  in
  let pp_fb fmt fb =
    let s =
      match fb with
      | HasUnsafeBlocks -> "Has unsafe blocks"
      | NoUnsafeBlocks -> "No unsafe blocks"
    in
    Format.pp_print_string fmt s
  in
  let pp_en fmt _ = Format.pp_print_string fmt "()" in
  let pp_ex fmt ex = Format.pp_print_string fmt (print_pos_and_ty ex) in
  let pp_hi fmt ty =
    Format.asprintf "(%s)" (Typing_print.full_strip_ns env ty)
    |> Format.pp_print_string fmt
  in
  Aast.show_program pp_ex pp_fb pp_en pp_hi tast

let go_ctx ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    string =
  (* Extract TAST and convert to string *)
  (* TODO(ljw): surely this doesn't need quarantine? *)
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let serialized_tast = serialize_tast tast ctx in
  (* Visit the TAST and extract autocompleteion items for Acclass_get *)
  let data = (new visitor)#get_entries ctx tast in
  let completions =
    List.map data ~f:(fun entry ->
        JSON_Object
          [
            ("candidates", JSON_String entry.candidates);
            ("context", JSON_String entry.context);
            ("token", JSON_String entry.token);
          ])
  in
  let output =
    JSON_Object
      [
        ("completions", JSON_Array completions);
        ("tast", JSON_String serialized_tast);
      ]
  in
  Hh_json.json_to_string output

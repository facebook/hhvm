(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t =
  | Naming of Naming_error.t
  | Typing of Typing_error.Primary.t
  | Nast_check of Nast_check_error.t
  | Like_type of Pos.t
  | Unexpected_hint of Pos.t
  | Malformed_access of Pos.t
  | Supportdyn of Pos.t

let naming err = Naming err

let typing err = Typing err

let nast_check err = Nast_check err

let like_type pos = Like_type pos

let unexpected_hint pos = Unexpected_hint pos

let malformed_access pos = Malformed_access pos

let supportdyn pos = Supportdyn pos

type agg = {
  naming: Naming_error.t list;
  (* TODO[mjt] either these errors shouldn't be raised in naming or they aren't
     really typing errors *)
  typing: Typing_error.Primary.t list;
  (* TODO[mjt] as above, either these should be naming errors or we should
     be raising in NAST checks *)
  nast_check: Nast_check_error.t list;
  (* TODO[mjt] these errors are not represented by any of our conventional
     phase errors; presumably they should all be naming errors? *)
  like_types: Pos.t list;
  unexpected_hints: Pos.t list;
  malformed_accesses: Pos.t list;
  supportdyns: Pos.t list;
}

let empty =
  {
    naming = [];
    typing = [];
    nast_check = [];
    like_types = [];
    unexpected_hints = [];
    malformed_accesses = [];
    supportdyns = [];
  }

let add t = function
  | Naming err -> { t with naming = err :: t.naming }
  | Typing err -> { t with typing = err :: t.typing }
  | Nast_check err -> { t with nast_check = err :: t.nast_check }
  | Like_type err -> { t with like_types = err :: t.like_types }
  | Unexpected_hint err ->
    { t with unexpected_hints = err :: t.unexpected_hints }
  | Malformed_access err ->
    { t with malformed_accesses = err :: t.malformed_accesses }
  | Supportdyn err -> { t with supportdyns = err :: t.supportdyns }

let emit
    {
      naming;
      typing;
      nast_check;
      like_types;
      unexpected_hints;
      malformed_accesses;
      supportdyns;
    } =
  List.iter ~f:Errors.add_naming_error naming;
  List.iter ~f:Errors.add_nast_check_error nast_check;
  List.iter
    ~f:(fun err -> Errors.add_typing_error @@ Typing_error.primary err)
    typing;
  List.iter
    ~f:(fun pos -> Errors.experimental_feature pos "like-types")
    like_types;
  List.iter
    ~f:(fun pos ->
      Errors.internal_error pos "Unexpected hint not present on legacy AST")
    unexpected_hints;
  List.iter
    ~f:(fun pos ->
      Errors.internal_error
        pos
        "Malformed hint: expected Haccess (Happly ...) from ast_to_nast")
    malformed_accesses;
  List.iter
    ~f:(fun p -> Errors.experimental_feature p "supportdyn type hint")
    supportdyns

(* Helper for constructing expression to be substituted for invalid expressions
   TODO[mjt] this probably belongs with the AAST defs
*)
let invalid_expr_ pos =
  let throw =
    ( pos,
      Aast.Throw
        ( (),
          pos,
          Aast.New
            ( ((), pos, Aast.CI (pos, "\\Exception")),
              [],
              [((), pos, Aast.String "invalid expression")],
              None,
              () ) ) )
  in
  Aast.Call
    ( ( (),
        pos,
        Aast.Lfun
          ( {
              Aast.f_span = pos;
              f_readonly_this = None;
              f_annotation = ();
              f_readonly_ret = None;
              f_ret = ((), None);
              f_tparams = [];
              f_where_constraints = [];
              f_params = [];
              f_ctxs = None;
              f_unsafe_ctxs = None;
              f_body = { Aast.fb_ast = [throw] };
              f_fun_kind = Ast_defs.FSync;
              f_user_attributes = [];
              f_external = false;
              f_doc_comment = None;
            },
            [] ) ),
      [],
      [],
      None )

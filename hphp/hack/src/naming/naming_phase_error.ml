(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type t = {
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

let suppress_like_type_errors t = { t with like_types = [] }

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

(* -- Monoid for use in visitors -------------------------------------------- *)

module Free_monoid = struct
  type 'a t =
    | Zero
    | One of 'a
    | Plus of 'a t * 'a t

  let zero = Zero

  let plus e1 e2 =
    match (e1, e2) with
    | (Zero, _) -> e2
    | (_, Zero) -> e1
    | _ -> Plus (e1, e2)

  let mk x = One x

  let fold t ~init ~f =
    let rec aux t ~acc ~k =
      match t with
      | Zero -> k acc
      | One x -> k @@ f acc x
      | Plus (x, y) -> aux y ~acc ~k:(fun acc -> aux x ~acc ~k)
    in
    aux t ~acc:init ~k:(fun x -> x)

  class virtual ['a] monoid =
    object (_ : 'self)
      inherit ['a t] Visitors_runtime.monoid

      method private zero = zero

      method private plus = plus
    end
end

type err =
  | Naming of Naming_error.t
  | Typing of Typing_error.Primary.t
  | Nast_check of Nast_check_error.t
  | Like_type of Pos.t
  | Unexpected_hint of Pos.t
  | Malformed_access of Pos.t
  | Supportdyn of Pos.t

let naming err = Free_monoid.mk @@ Naming err

let nast_check err = Free_monoid.mk @@ Nast_check err

let typing err = Free_monoid.mk @@ Typing err

let like_type pos = Free_monoid.mk @@ Like_type pos

let unexpected_hint pos = Free_monoid.mk @@ Unexpected_hint pos

let malformed_access pos = Free_monoid.mk @@ Malformed_access pos

let supportdyn pos = Free_monoid.mk @@ Supportdyn pos

class monoid = [err] Free_monoid.monoid

(* Convert the result of reduce / mapreduce visitor to our error representation *)
let from_monoid ?(init = empty) err =
  let f acc = function
    | Naming err -> { acc with naming = err :: acc.naming }
    | Typing err -> { acc with typing = err :: acc.typing }
    | Nast_check err -> { acc with nast_check = err :: acc.nast_check }
    | Like_type pos -> { acc with like_types = pos :: acc.like_types }
    | Unexpected_hint pos ->
      { acc with unexpected_hints = pos :: acc.unexpected_hints }
    | Malformed_access pos ->
      { acc with malformed_accesses = pos :: acc.malformed_accesses }
    | Supportdyn pos -> { acc with supportdyns = pos :: acc.supportdyns }
  in

  Free_monoid.fold err ~init ~f

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
              f_name = (pos, "invalid_expr");
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

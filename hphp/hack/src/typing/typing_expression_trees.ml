(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Spliced Expressions within Expression Trees are typed in a different
 * environment than the other code in an Expression Tree. Spliced
 * Expressions are treated as if they were lifted out of the Expression
 * Tree and executed before constructing the Expression Tree itself.
 *
 * `extract_spliced_expressions` uses a reduce visitor to extract all
 * of the Spliced Expressions from an Expression Tree. It then uses an
 * endo visitor to replace the expressions in the Expression Tree with
 * a unique id. This unique id is later used in typing to get the
 * type of the extracted expression.
 *)
let starting_id = 0

class ['a, 'b, 'c, 'd] et_reduce =
  object (_self)
    inherit ['a] Aast.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_expr env e =
      match snd e with
      | Aast.ET_Splice e -> [e]
      | _ -> super#on_expr env e
  end

class ['a, 'b, 'c, 'd] et_endo =
  let make_id pos id = Aast.Id (pos, string_of_int id) in
  object (_self)
    inherit ['a] Aast.endo as super

    val mutable unique_id = starting_id

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    method! on_expr env e =
      match snd e with
      | Aast.ET_Splice (p, _) ->
        let new_id = make_id p unique_id in
        let _ = unique_id <- unique_id + 1 in
        (fst e, Aast.ET_Splice (p, new_id))
      | _ -> super#on_expr env e
  end

let reducer = new et_reduce

let extract_spliced_expressions et =
  let extracted_expressions = reducer#on_expr () et in
  let (extracted_expressions, _) =
    List.fold
      ~f:(fun (acc, id) e -> ((id, e) :: acc, id + 1))
      ~init:([], starting_id)
      extracted_expressions
  in
  let extracted_expressions = List.rev extracted_expressions in
  let new_expression_tree = (new et_endo)#on_expr () et in
  (extracted_expressions, new_expression_tree)

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Gen = Milner_generate
module Syntax = Milner_syntax

type family =
  | Atom
  | Callable

let return value = Syntax.Return (Some value)

let apply ty parameters body arguments =
  Syntax.Call (Syntax.Lambda (parameters, ["defaults"], ty, body), arguments)

let zero = Syntax.Atom "0"

let consume budget =
  if !budget = 0 then
    false
  else (
    decr budget;
    true
  )

let random_family () =
  if Random.int 4 = 0 then
    Atom
  else
    Callable

let rec expression budget family ty value =
  match family with
  | Atom -> value
  | Callable ->
    if not (consume budget) then
      value
    else
      let child value = expression budget (random_family ()) ty value in
      callable child ty value

and callable child ty value =
  match Random.int 5 with
  | 0 -> apply ty [] [return (child value)] []
  | 1 ->
    let callback = Syntax.fresh_local "callback" in
    apply
      ty
      [Syntax.parameter ("(function()[defaults]: " ^ ty ^ ")") callback]
      [return (Syntax.Call (Syntax.Local callback, []))]
      [Syntax.Lambda ([], ["defaults"], ty, [return (child value)])]
  | 2 ->
    let box = Syntax.fresh_local "box" in
    let target = Syntax.fresh_local "target" in
    let box_ty = "vec<" ^ ty ^ ">" in
    let replace =
      Syntax.Lambda
        ( [Syntax.parameter ("inout " ^ box_ty) target],
          ["defaults"],
          "void",
          [
            Syntax.Assign
              (Syntax.Local target, Syntax.Array ("vec", [child value]));
          ] )
    in
    apply
      ty
      [Syntax.parameter box_ty box]
      [
        Syntax.Eval (Syntax.Call (replace, [Syntax.Inout (Syntax.Local box)]));
        return (Syntax.Index (Syntax.Local box, zero));
      ]
      [Syntax.Array ("vec", [])]
  | 3 ->
    let head = Syntax.fresh_local "head" in
    let rest = Syntax.fresh_local "rest" in
    let tail = List.init (1 + Random.int 3) ~f:(fun _ -> value) in
    let arguments =
      value
      ::
      (if Random.bool () then
        [Syntax.Unpack (Syntax.Array ("vec", tail))]
      else
        tail)
    in
    apply
      ty
      [Syntax.parameter ty head; Syntax.parameter ~variadic:true ty rest]
      [return (child (Syntax.Index (Syntax.Local rest, zero)))]
      arguments
  | _ ->
    let argument = Syntax.fresh_local "argument" in
    let index = Syntax.fresh_local "index" in
    apply
      ty
      [Syntax.parameter ty argument; Syntax.parameter ~default:zero "int" index]
      [
        return
          (child
             (Syntax.Index
                ( Syntax.Array ("vec", [Syntax.Local argument]),
                  Syntax.Local index )));
      ]
      (if Random.bool () then
        [value]
      else
        [value; zero])

let operation family ~ty =
  let ty = Gen.Type.show ty in
  let value = Syntax.fresh_local "input" in
  let budget = ref (4 + Random.int 5) in
  Syntax.Lambda
    ( [Syntax.parameter ty value],
      ["defaults"],
      ty,
      [return (expression budget family ty (Syntax.Local value))] )

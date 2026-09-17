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
  | Flow
  | Callable

let return value = Syntax.Return (Some value)

let scope ty body = Syntax.Call (Syntax.Lambda ([], ["defaults"], ty, body), [])

let bind ty value body =
  let local = Syntax.fresh_local "value" in
  scope ty (Syntax.Bind (local, value) :: body (Syntax.Local local))

let apply ty parameters body arguments =
  Syntax.Call (Syntax.Lambda (parameters, ["defaults"], ty, body), arguments)

let integer value = Syntax.Atom (string_of_int value)

let zero = integer 0

let boolean () =
  Syntax.Atom
    (if Random.bool () then
      "true"
    else
      "false")

let random_family () =
  match Random.int 3 with
  | 0 -> Atom
  | 1 -> Flow
  | _ -> Callable

let consume budget =
  if !budget = 0 then
    false
  else (
    decr budget;
    true
  )

let rec condition budget =
  if not (consume budget) then
    boolean ()
  else
    match Random.int 5 with
    | 0 -> Syntax.Unary ("!", condition budget)
    | 1 -> Syntax.Binary ("&&", condition budget, condition budget)
    | 2 ->
      let operand = integer (Random.int 8) in
      Syntax.Binary
        ( "<",
          Syntax.Binary ("+", operand, integer (Random.int 8)),
          integer (Random.int 16) )
    | 3 ->
      Syntax.Binary
        ( "===",
          Syntax.Call
            ( Syntax.Atom "strlen",
              [Syntax.Binary (".", Syntax.Atom "'a'", Syntax.Atom "'bc'")] ),
          integer (Random.int 5) )
    | _ ->
      let item = Syntax.fresh_local "integer" in
      let widened =
        apply
          "mixed"
          [Syntax.parameter "int" item]
          [return (Syntax.Local item)]
          [integer (Random.int 8)]
      in
      if Random.bool () then
        Syntax.Is (widened, "int")
      else
        Syntax.Binary ("<", Syntax.As (widened, "int"), integer (Random.int 8))

let rec expression budget family ty value =
  match family with
  | Atom -> value
  | Flow
  | Callable ->
    if not (consume budget) then
      value
    else
      let child value = expression budget (random_family ()) ty value in
      (match family with
      | Atom -> value
      | Flow -> flow budget child ty value
      | Callable -> callable child ty value)

and flow budget child ty value =
  match Random.int 14 with
  | 0 -> bind ty value (fun local -> [return (child local)])
  | 1 ->
    scope
      ty
      [
        Syntax.If
          (condition budget, [return (child value)], [return (child value)]);
      ]
  | 2 -> Syntax.Index (Syntax.Tuple [child value], integer 0)
  | 3 -> Syntax.Index (Syntax.Array ("vec", [child value]), integer 0)
  | 4 ->
    Syntax.Index (Syntax.Shape [("'value'", child value)], Syntax.Atom "'value'")
  | 5 ->
    let result = Syntax.fresh_local "result" in
    let count = Syntax.fresh_local "count" in
    scope
      ty
      [
        Syntax.Bind (result, value);
        Syntax.Bind (count, integer 0);
        Syntax.While
          ( Syntax.Binary ("<", Syntax.Local count, integer (Random.int 4)),
            [
              Syntax.Assign (Syntax.Local result, child (Syntax.Local result));
              Syntax.Assign
                ( Syntax.Local count,
                  Syntax.Binary ("+", Syntax.Local count, integer 1) );
            ] );
        return (Syntax.Local result);
      ]
  | 6 ->
    let result = Syntax.fresh_local "result" in
    let item = Syntax.fresh_local "item" in
    let items = List.init (Random.int 4) ~f:(fun _ -> value) in
    scope
      ty
      [
        Syntax.Bind (result, value);
        Syntax.Foreach
          ( Syntax.Array ("vec", items),
            item,
            [Syntax.Assign (Syntax.Local result, child (Syntax.Local item))] );
        return (Syntax.Local result);
      ]
  | 7 ->
    let caught = Syntax.fresh_local "exception" in
    scope
      ty
      [
        Syntax.Try
          ( [Syntax.Throw (Syntax.New ("Exception", []))],
            [("Exception", caught, [return (child value)])],
            [] );
      ]
  | 8 ->
    scope
      ty
      [
        Syntax.Try
          ( [return (child value)],
            [],
            [Syntax.Bind (Syntax.fresh_local "cleanup", child value)] );
      ]
  | 9 ->
    nullable_box ty value (fun box ->
        [
          Syntax.If
            ( Syntax.Is (box, "null"),
              [return (child value)],
              [return (child (Syntax.Index (box, Syntax.Atom "'value'")))] );
        ])
  | 10 ->
    nullable_box ty value (fun box ->
        [
          return
            (child
               (Syntax.Index
                  ( Syntax.Binary ("??", box, Syntax.Shape [("'value'", value)]),
                    Syntax.Atom "'value'" )));
        ])
  | 11 ->
    let box = Syntax.fresh_local "box" in
    let local = Syntax.Local box in
    let field = Syntax.Index (local, Syntax.Atom "'value'") in
    apply
      ty
      [Syntax.parameter ("shape(?'value' => " ^ ty ^ ")") box]
      [
        Syntax.If
          ( Syntax.Unary
              ( "!",
                Syntax.Call
                  ( Syntax.StaticMember ("Shapes", "keyExists"),
                    [local; Syntax.Atom "'value'"] ) ),
            [Syntax.Assign (field, value)],
            [] );
        return (child field);
      ]
      [
        Syntax.Shape
          (if Random.bool () then
            []
          else
            [("'value'", value)]);
      ]
  | 12 ->
    let receiver = Syntax.fresh_local "receiver" in
    let wrapper = "Vector<" ^ ty ^ ">" in
    apply
      ty
      [Syntax.parameter ("?" ^ wrapper) receiver]
      [
        return
          (child
             (Syntax.Binary
                ( "??",
                  Syntax.Call
                    (Syntax.NullsafeMember (Syntax.Local receiver, "at"), [zero]),
                  value )));
      ]
      [
        (if Random.bool () then
          Syntax.Atom "null"
        else
          Syntax.New (wrapper, [Syntax.Array ("vec", [value])]));
      ]
  | _ ->
    let box = Syntax.fresh_local "box" in
    let slot = Syntax.Index (Syntax.Local box, zero) in
    apply
      ty
      [Syntax.parameter ("vec<vec<" ^ ty ^ ">>") box]
      [
        Syntax.Assign (slot, Syntax.Array ("vec", [child value]));
        return (Syntax.Index (slot, zero));
      ]
      [Syntax.Array ("vec", [Syntax.Array ("vec", [])])]

and nullable_box ty value body =
  let box = Syntax.fresh_local "nullable" in
  apply
    ty
    [Syntax.parameter ("?shape('value' => " ^ ty ^ ")") box]
    (body (Syntax.Local box))
    [
      (if Random.bool () then
        Syntax.Atom "null"
      else
        Syntax.Shape [("'value'", value)]);
    ]

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

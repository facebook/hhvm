(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Milner_syntax

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
    match Random.int 7 with
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
    | 4 ->
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
    | 5 -> Syntax.Binary ("||", condition budget, condition budget)
    | _ ->
      Syntax.Binary
        ( "!==",
          Syntax.Array ("vec", [integer (Random.int 2)]),
          Syntax.Array ("vec", [integer (Random.int 2)]) )

let rec expression budget operations ty value =
  if not (consume budget) then
    value
  else
    let child value = expression budget operations ty value in
    match Random.int (4 + List.length operations) with
    | 0 -> value
    | 1 -> flow budget child ty value
    | 2 -> async budget child ty value
    | 3 -> callable child ty value
    | index ->
      let operation = List.nth_exn operations (index - 4) in
      bind ty (child value) (fun local -> [return (operation local)])

and flow budget child ty value =
  match Random.int 22 with
  | 0 -> bind ty value (fun local -> [return (child local)])
  | 1 ->
    let predicate = condition budget in
    scope
      ty
      [Syntax.If (predicate, [return (child value)], [return (child value)])]
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
            None,
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
    let wrapper = "Vector<shape('value' => " ^ ty ^ ")>" in
    let boxed = Syntax.Shape [("'value'", value)] in
    apply
      ty
      [Syntax.parameter ("?" ^ wrapper) receiver]
      [
        return
          (child
             (Syntax.Index
                ( Syntax.Binary
                    ( "??",
                      Syntax.Call
                        ( Syntax.NullsafeMember (Syntax.Local receiver, "at"),
                          [zero] ),
                      boxed ),
                  Syntax.Atom "'value'" )));
      ]
      [
        (if Random.bool () then
          Syntax.Atom "null"
        else
          Syntax.New (wrapper, [Syntax.Array ("vec", [boxed])]));
      ]
  | 13 ->
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
  | 14 ->
    let box = Syntax.fresh_local "values" in
    apply
      ty
      [Syntax.parameter ("vec<" ^ ty ^ ">") box]
      [
        Syntax.Assign (Syntax.Append (Syntax.Local box), child value);
        return (Syntax.Index (Syntax.Local box, integer 1));
      ]
      [Syntax.Array ("vec", [value])]
  | 15 ->
    let box = Syntax.fresh_local "mapping" in
    let slot = Syntax.Index (Syntax.Local box, Syntax.Atom "'second'") in
    apply
      ty
      [Syntax.parameter ("dict<string, " ^ ty ^ ">") box]
      [Syntax.Assign (slot, child value); return slot]
      [Syntax.Array ("dict", [Syntax.KeyValue (Syntax.Atom "'first'", value)])]
  | 16 ->
    let box = Syntax.fresh_local "mapping" in
    let key = Syntax.fresh_local "key" in
    let item = Syntax.fresh_local "item" in
    let result = Syntax.fresh_local "result" in
    apply
      ty
      [Syntax.parameter ("dict<string, " ^ ty ^ ">") box]
      [
        Syntax.Bind (result, value);
        Syntax.Foreach
          ( Syntax.Local box,
            Some key,
            item,
            [
              Syntax.Eval
                (Syntax.Call
                   ( Syntax.Atom "invariant",
                     [
                       Syntax.Binary
                         ( "===",
                           Syntax.Array
                             ( "vec",
                               [
                                 Syntax.Index
                                   (Syntax.Local box, Syntax.Local key);
                               ] ),
                           Syntax.Array ("vec", [Syntax.Local item]) );
                       Syntax.Atom "'keyed iteration preserves values'";
                     ] ));
              Syntax.Assign (Syntax.Local result, child (Syntax.Local item));
            ] );
        return (Syntax.Local result);
      ]
      [
        Syntax.Array
          ( "dict",
            [
              Syntax.KeyValue (Syntax.Atom "'first'", value);
              Syntax.KeyValue (Syntax.Atom "'second'", child value);
            ] );
      ]
  | 17 ->
    let box = Syntax.fresh_local "optional_tuple" in
    let slot = "shape('value' => " ^ ty ^ ")" in
    let boxed = Syntax.Shape [("'value'", value)] in
    apply
      ty
      [Syntax.parameter ("(" ^ slot ^ ", optional " ^ slot ^ ")") box]
      [
        return
          (child
             (Syntax.Index
                ( Syntax.Binary
                    ( "??",
                      Syntax.Index (Syntax.Local box, integer 1),
                      Syntax.Index (Syntax.Local box, zero) ),
                  Syntax.Atom "'value'" )));
      ]
      [
        Syntax.Tuple
          (if Random.bool () then
            [boxed]
          else
            [boxed; boxed]);
      ]
  | 18 -> iterable child ty value
  | 19 ->
    let box = Syntax.fresh_local "container" in
    apply
      ty
      [Syntax.parameter ("KeyedContainer<string, " ^ ty ^ ">") box]
      [return (child (Syntax.Index (Syntax.Local box, Syntax.Atom "'value'")))]
      [Syntax.Array ("dict", [Syntax.KeyValue (Syntax.Atom "'value'", value)])]
  | 20 ->
    let box = Syntax.fresh_local "widened_bottom" in
    let bottom =
      Syntax.Call
        ( Syntax.Lambda
            ([], [], "vec<nothing>", [return (Syntax.Array ("vec", []))]),
          [] )
    in
    apply
      ty
      [Syntax.parameter ("vec<" ^ ty ^ ">") box]
      [
        Syntax.Assign (Syntax.Append (Syntax.Local box), child value);
        return (Syntax.Index (Syntax.Local box, zero));
      ]
      [bottom]
  | _ ->
    let box = Syntax.fresh_local "record" in
    let field = Syntax.Index (Syntax.Local box, Syntax.Atom "'value'") in
    apply
      ty
      [Syntax.parameter ("shape('value' => " ^ ty ^ ")") box]
      [Syntax.Assign (field, child value); return field]
      [Syntax.Shape [("'value'", value)]]

and iterable child ty value =
  let vec = Syntax.Array ("vec", [value]) in
  let dict =
    Syntax.Array ("dict", [Syntax.KeyValue (Syntax.Atom "'value'", value)])
  in
  let iterator collection =
    Syntax.Call (Syntax.Member (collection, "getIterator"), [])
  in
  let (hint, input, keyed) =
    match Random.int 7 with
    | 0 -> ("Traversable<" ^ ty ^ ">", vec, false)
    | 1 -> ("Container<" ^ ty ^ ">", vec, false)
    | 2 ->
      ( "Iterator<" ^ ty ^ ">",
        iterator (Syntax.New ("Vector<" ^ ty ^ ">", [vec])),
        false )
    | 3 -> ("KeyedTraversable<string, " ^ ty ^ ">", dict, true)
    | 4 -> ("KeyedContainer<string, " ^ ty ^ ">", dict, true)
    | 5 ->
      ( "KeyedIterator<string, " ^ ty ^ ">",
        iterator (Syntax.New ("Map<string, " ^ ty ^ ">", [dict])),
        true )
    | _ ->
      ( "vec_or_dict<"
        ^ (if Random.bool () then
            ""
          else
            "arraykey, ")
        ^ ty
        ^ ">",
        (if Random.bool () then
          vec
        else
          dict),
        true )
  in
  let input_local = Syntax.fresh_local "iterable" in
  let item = Syntax.fresh_local "item" in
  let result = Syntax.fresh_local "result" in
  let iteration =
    if keyed then
      let key = Syntax.fresh_local "key" in
      let selected =
        Syntax.Index
          ( Syntax.Array
              ("dict", [Syntax.KeyValue (Syntax.Local key, Syntax.Local item)]),
            Syntax.Local key )
      in
      Syntax.Foreach
        ( Syntax.Local input_local,
          Some key,
          item,
          [Syntax.Assign (Syntax.Local result, child selected)] )
    else
      Syntax.Foreach
        ( Syntax.Local input_local,
          None,
          item,
          [Syntax.Assign (Syntax.Local result, child (Syntax.Local item))] )
  in
  apply
    ty
    [Syntax.parameter hint input_local]
    [Syntax.Bind (result, value); iteration; return (Syntax.Local result)]
    [input]

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
  match Random.int 6 with
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
      (* T288960552: direct multi-tail calls can infer invalid dynamic bounds. *)
      (if Random.bool () || List.length tail > 1 then
        [Syntax.Unpack (Syntax.Array ("vec", tail))]
      else
        tail)
    in
    apply
      ty
      [Syntax.parameter ty head; Syntax.parameter ~variadic:true ty rest]
      [return (child (Syntax.Index (Syntax.Local rest, zero)))]
      arguments
  | 4 ->
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
  | _ ->
    bind ty value (fun value ->
        let (contexts, result) =
          match Random.int 3 with
          | 0 -> ([], value)
          | 1 ->
            ( ["write_props"],
              Syntax.Call
                ( Syntax.Member
                    ( Syntax.New
                        ("Vector<" ^ ty ^ ">", [Syntax.Array ("vec", [value])]),
                      "at" ),
                  [zero] ) )
          | _ -> (["defaults"], child value)
        in
        [
          return
            (child
               (Syntax.Call
                  ( Syntax.Atom ("milner_invoke<" ^ ty ^ ">"),
                    [Syntax.Lambda ([], contexts, ty, [return result])] )));
        ])

and async budget child ty value =
  let task body =
    Syntax.Call
      (Syntax.AsyncLambda ([], ["defaults"], "Awaitable<" ^ ty ^ ">", body), [])
  in
  let suspend =
    Syntax.Eval
      (Syntax.Await
         (Syntax.Call
            ( Syntax.StaticMember ("RescheduleWaitHandle", "create"),
              [integer 0; integer 0] )))
  in
  let rec awaitable value =
    if not (consume budget) then
      task [return value]
    else
      match Random.int 5 with
      | 0 -> task [return (child value)]
      | 1 -> task [suspend; return (child value)]
      | 2 ->
        let result = Syntax.fresh_local "awaited" in
        task
          [
            Syntax.Bind (result, Syntax.Await (awaitable value));
            return (child (Syntax.Local result));
          ]
      | 3 ->
        let results =
          List.init
            (2 + Random.int 2)
            ~f:(fun _ -> (Syntax.fresh_local "concurrent", awaitable value))
        in
        let (selected, _) =
          List.nth_exn results (Random.int (List.length results))
        in
        task
          [
            Syntax.Concurrent
              (List.map results ~f:(fun (result, pending) ->
                   Syntax.Bind (result, Syntax.Await pending)));
            return (child (Syntax.Local selected));
          ]
      | _ ->
        task
          [
            Syntax.Try ([return (Syntax.Await (awaitable value))], [], [suspend]);
          ]
  in
  let join pending = Syntax.Call (Syntax.Atom "HH\\Asio\\join", [pending]) in
  if Random.int 4 = 0 then
    let caught = Syntax.fresh_local "exception" in
    scope
      ty
      [
        Syntax.Try
          ( [
              Syntax.Bind
                ( Syntax.fresh_local "unreachable_result",
                  join
                    (task
                       [
                         suspend;
                         Syntax.Throw
                           (Syntax.New
                              ( "Exception",
                                [Syntax.Atom "'milner expected async throw'"] ));
                       ]) );
            ],
            [("Exception", caught, [return (child value)])],
            [] );
        return value;
      ]
  else
    join (awaitable value)

let compose ~ty ~value ~operations =
  let budget = ref (4 + Random.int 5) in
  bind ty value (fun local -> [return (expression budget operations ty local)])

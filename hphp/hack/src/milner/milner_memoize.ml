(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Milner_syntax
module Expression = Milner_expression

type key = {
  parameter: Syntax.parameter;
  value: Syntax.expr;
}

type supply =
  | Explicit
  | Omitted
  | Partial

let integer value = Syntax.Atom (string_of_int value)

let primitive () =
  match Random.int 4 with
  | 0 -> ("int", integer (Random.int 11 - 5))
  | 1 ->
    ( "string",
      Syntax.Atom (List.nth_exn ["''"; "'apple'"; "'pear'"] (Random.int 3)) )
  | 2 ->
    ( "bool",
      Syntax.Atom
        (if Random.bool () then
          "true"
        else
          "false") )
  | _ ->
    ("float", Syntax.Atom (List.nth_exn ["0.5"; "-1.25"; "3.0"] (Random.int 3)))

let rec literal depth =
  if depth = 0 then
    primitive ()
  else
    match Random.int 6 with
    | 0 -> primitive ()
    | 1 ->
      let (hint, value) = primitive () in
      ( "?" ^ hint,
        if Random.bool () then
          value
        else
          Syntax.Atom "null" )
    | 2 ->
      let (left_hint, left) = literal (depth - 1) in
      let (right_hint, right) = literal (depth - 1) in
      ("(" ^ left_hint ^ ", " ^ right_hint ^ ")", Syntax.Tuple [left; right])
    | 3 ->
      let (left_hint, left) = literal (depth - 1) in
      let (right_hint, right) = literal (depth - 1) in
      ( "shape('left' => " ^ left_hint ^ ", 'right' => " ^ right_hint ^ ")",
        Syntax.Shape [("'left'", left); ("'right'", right)] )
    | 4 ->
      let (hint, value) = literal (depth - 1) in
      ( "vec<" ^ hint ^ ">",
        Syntax.Array ("vec", List.init (Random.int 3) ~f:(fun _ -> value)) )
    | _ ->
      let (hint, value) = literal (depth - 1) in
      let fields =
        List.take ["'left'"; "'right'"] (Random.int 3)
        |> List.map ~f:(fun key -> Syntax.KeyValue (Syntax.Atom key, value))
      in
      ("dict<string, " ^ hint ^ ">", Syntax.Array ("dict", fields))

let make_key ~optional =
  let (hint, value) = literal (1 + Random.int 2) in
  let default =
    if optional then
      Some value
    else
      None
  in
  {
    parameter = Syntax.parameter ?default hint (Syntax.fresh_local "memo_key");
    value;
  }

let same_parameter left right =
  String.equal
    (Syntax.local_name left.Syntax.local)
    (Syntax.local_name right.Syntax.local)

let assert_identity ~same left right message =
  Syntax.Eval
    (Syntax.Call
       ( Syntax.Atom "invariant",
         [
           Syntax.Binary
             ( (if same then
                 "==="
               else
                 "!=="),
               Syntax.Local left,
               Syntax.Local right );
           Syntax.Atom ("'" ^ message ^ "'");
         ] ))

let make_operation ~allow_named ~ty ~value =
  let payload = Syntax.fresh_local "memo_payload" in
  let callable = Syntax.fresh_local "memo_callable" in
  let discriminator =
    Syntax.parameter "int" (Syntax.fresh_local "memo_discriminator")
  in
  let keys =
    List.init (Random.int 3) ~f:(fun _ -> make_key ~optional:false)
    @ List.init (1 + Random.int 3) ~f:(fun _ -> make_key ~optional:true)
  in
  let parameters =
    Expression.promote_parameters
      ~allow_named
      (discriminator :: List.map keys ~f:(fun key -> key.parameter))
  in
  let is_async = Random.bool () in
  let suspension =
    if is_async && Random.bool () then
      [
        Syntax.Eval
          (Syntax.Await
             (Syntax.Call
                ( Syntax.StaticMember ("RescheduleWaitHandle", "create"),
                  [integer 0; integer 0] )));
      ]
    else
      []
  in
  let declaration =
    {
      Syntax.type_parameters = [];
      is_async;
      parameters;
      contexts = [];
      return_hint =
        (if is_async then
          "Awaitable<stdClass>"
        else
          "stdClass");
      body = suspension @ [Syntax.Return (Some (Syntax.New ("stdClass", [])))];
      memoize = true;
    }
  in
  let invoke discriminator_value supply =
    let arguments =
      Expression.call_arguments parameters ~value:(fun parameter ->
          if same_parameter parameter discriminator then
            Some (integer discriminator_value)
          else
            let key =
              List.find_exn keys ~f:(fun key ->
                  same_parameter parameter key.parameter)
            in
            let omit =
              Option.is_some parameter.Syntax.default
              &&
              match supply with
              | Explicit -> false
              | Omitted -> true
              | Partial -> Random.bool ()
            in
            if omit then
              None
            else
              Some key.value)
    in
    let call = Syntax.Call (Syntax.Local callable, arguments) in
    if is_async then
      Syntax.Await call
    else
      call
  in
  let first = Syntax.fresh_local "memo_first" in
  let repeated = Syntax.fresh_local "memo_repeated" in
  let different = Syntax.fresh_local "memo_different" in
  let revisited = Syntax.fresh_local "memo_revisited" in
  let key = Random.int 11 - 5 in
  let initial_calls =
    [
      Syntax.Bind (first, invoke key Explicit);
      Syntax.Bind (repeated, invoke key Omitted);
    ]
  in
  let initial_calls =
    if is_async && Random.bool () then
      [Syntax.Concurrent initial_calls]
    else
      initial_calls
  in
  let body =
    [
      Syntax.Bind (payload, value);
      Syntax.Bind (callable, Syntax.DeclaredCallable (declaration, []));
    ]
    @ initial_calls
    @ [
        Syntax.Bind (different, invoke (key + 1) Partial);
        Syntax.Bind (revisited, invoke key Partial);
        assert_identity
          ~same:true
          first
          repeated
          "memoized defaults preserve cache keys";
        assert_identity
          ~same:false
          first
          different
          "distinct memoized keys execute the body";
        assert_identity
          ~same:true
          first
          revisited
          "memoized keys retain independent results";
        Syntax.Return (Some (Syntax.Local payload));
      ]
  in
  if is_async then
    Syntax.Call
      ( Syntax.Atom "HH\\Asio\\join",
        [
          Syntax.Call
            ( Syntax.AsyncLambda
                ([], ["defaults"], "Awaitable<" ^ ty ^ ">", body),
              [] );
        ] )
  else
    Syntax.Call (Syntax.Lambda ([], ["defaults"], ty, body), [])

(* T289176741: memo wrappers do not forward named arguments to their bodies. *)
let operation ~ty ~value = make_operation ~allow_named:false ~ty ~value

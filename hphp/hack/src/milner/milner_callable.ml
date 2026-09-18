(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Milner_syntax

type callable = {
  is_async: bool;
  parameters: Syntax.parameter list;
  contexts: string list;
  return_hint: string;
  body: Syntax.stmt list;
}

type form =
  | Free_function
  | Static_method
  | Instance_method
  | Constructor

let bind bound local = Syntax.local_name local :: bound

let bound_local bound local =
  List.mem bound (Syntax.local_name local) ~equal:String.equal

let rec closed_expression bound = function
  | Syntax.Atom source -> not (String.is_substring source ~substring:"$")
  | Syntax.Local local
  | Syntax.DynamicStaticMember (local, _) ->
    bound_local bound local
  | Syntax.NewDynamic (local, arguments) ->
    bound_local bound local
    && List.for_all arguments ~f:(closed_expression bound)
  | Syntax.Unary (_, expression)
  | Syntax.As (expression, _)
  | Syntax.Is (expression, _)
  | Syntax.NullsafeMember (expression, _)
  | Syntax.Member (expression, _)
  | Syntax.Await expression
  | Syntax.Append expression
  | Syntax.Inout expression
  | Syntax.Unpack expression
  | Syntax.NamedArgument (_, expression) ->
    closed_expression bound expression
  | Syntax.Binary (_, left, right)
  | Syntax.Index (left, right)
  | Syntax.KeyValue (left, right) ->
    closed_expression bound left && closed_expression bound right
  | Syntax.Call (callee, arguments) ->
    closed_expression bound callee
    && List.for_all arguments ~f:(closed_expression bound)
  | Syntax.New (_, arguments)
  | Syntax.Array (_, arguments)
  | Syntax.Tuple arguments ->
    List.for_all arguments ~f:(closed_expression bound)
  | Syntax.Shape fields ->
    List.for_all fields ~f:(fun (_, value) -> closed_expression bound value)
  | Syntax.Xhp (_, attributes, children) ->
    List.for_all attributes ~f:(fun (_, value) -> closed_expression bound value)
    && List.for_all children ~f:(closed_expression bound)
  | Syntax.Lambda (parameters, contexts, _, body)
  | Syntax.AsyncLambda (parameters, contexts, _, body) ->
    closed_body bound parameters contexts body
  | Syntax.Async body -> fst (closed_statements bound body)
  | Syntax.Quote _
  | Syntax.Splice _ ->
    false
  | Syntax.Nameof _
  | Syntax.StaticMember _
  | Syntax.EnumLabel _
  | Syntax.StaticProperty _ ->
    true

and closed_body bound parameters contexts body =
  let bound =
    List.fold parameters ~init:bound ~f:(fun bound parameter ->
        bind bound parameter.Syntax.local)
  in
  List.for_all parameters ~f:(fun parameter ->
      Option.for_all parameter.Syntax.default ~f:(closed_expression []))
  && List.for_all contexts ~f:(fun context ->
         (not (String.is_substring context ~substring:"$"))
         || List.exists bound ~f:(fun local ->
                String.equal context ("ctx $" ^ local)))
  && fst (closed_statements bound body)

and closed_statements bound = function
  | [] -> (true, bound)
  | statement :: statements ->
    let (closed, bound) = closed_statement bound statement in
    if closed then
      closed_statements bound statements
    else
      (false, bound)

and closed_statement bound = function
  | Syntax.Bind (local, value)
  | Syntax.Assign (Syntax.Local local, value) ->
    (closed_expression bound value, bind bound local)
  | Syntax.Assign (target, value) ->
    (closed_expression bound target && closed_expression bound value, bound)
  | Syntax.Eval expression
  | Syntax.Throw expression ->
    (closed_expression bound expression, bound)
  | Syntax.Return expression ->
    (Option.for_all expression ~f:(closed_expression bound), bound)
  | Syntax.If (condition, consequent, alternative) ->
    ( closed_expression bound condition
      && fst (closed_statements bound consequent)
      && fst (closed_statements bound alternative),
      bound )
  | Syntax.While (condition, body) ->
    ( closed_expression bound condition && fst (closed_statements bound body),
      bound )
  | Syntax.Foreach (collection, key, value, body) ->
    let body_bound = bind bound value in
    let body_bound =
      Option.value_map key ~default:body_bound ~f:(bind body_bound)
    in
    ( closed_expression bound collection
      && fst (closed_statements body_bound body),
      bound )
  | Syntax.Try (body, catches, finally) ->
    ( fst (closed_statements bound body)
      && List.for_all catches ~f:(fun (_, local, body) ->
             fst (closed_statements (bind bound local) body))
      && fst (closed_statements bound finally),
      bound )
  | Syntax.Block statements -> closed_statements bound statements
  | Syntax.Concurrent statements ->
    let statements = List.map statements ~f:(closed_statement bound) in
    ( List.for_all statements ~f:fst,
      List.concat_map statements ~f:snd @ bound
      |> List.dedup_and_sort ~compare:String.compare )

let fresh_name hint =
  Syntax.local_name (Syntax.fresh_local ("callable_" ^ hint))

let permute_parameters parameters =
  let (variadic, fixed) =
    List.partition_tf parameters ~f:(fun parameter -> parameter.Syntax.variadic)
  in
  Milner_expression.permute_named
    (fun parameter -> parameter.Syntax.named)
    fixed
  @ variadic

let arguments parameters =
  List.map parameters ~f:(fun parameter ->
      let value = Syntax.Local parameter.Syntax.local in
      if parameter.Syntax.variadic then
        Syntax.Unpack value
      else if parameter.Syntax.named then
        Syntax.NamedArgument (Syntax.local_name parameter.Syntax.local, value)
      else
        value)
  |> Milner_expression.permute_named (function
         | Syntax.NamedArgument _ -> true
         | _ -> false)

let primitive_hint hint =
  List.mem
    [
      "mixed";
      "nonnull";
      "dynamic";
      "nothing";
      "null";
      "bool";
      "int";
      "float";
      "string";
      "num";
      "arraykey";
      "resource";
    ]
    hint
    ~equal:String.equal

let hierarchy_eligible parameters =
  (* T289079753: subtype checking can compare named slots against a concrete
     variadic tail when there is no fixed positional parameter. *)
  List.exists parameters ~f:(fun parameter ->
      (not parameter.Syntax.named) && not parameter.Syntax.variadic)
  || List.for_all parameters ~f:(fun parameter ->
         (not parameter.Syntax.variadic)
         || String.equal parameter.Syntax.hint "mixed")

let call_body ~is_async ~return_hint call =
  let call =
    if is_async then
      Syntax.Await call
    else
      call
  in
  if
    String.equal return_hint "void"
    || is_async
       && List.mem
            ["Awaitable<void>"; "HH\\Awaitable<void>"]
            return_hint
            ~equal:String.equal
  then
    [Syntax.Eval call; Syntax.Return None]
  else if
    String.equal return_hint "nothing"
    || is_async
       && List.mem
            ["Awaitable<nothing>"; "HH\\Awaitable<nothing>"]
            return_hint
            ~equal:String.equal
  then
    [
      Syntax.Eval call;
      Syntax.Throw
        (Syntax.New
           ("Exception", [Syntax.Atom "'milner nothing method returned'"]));
    ]
  else
    [Syntax.Return (Some call)]

let rec constructor_body target body =
  List.map body ~f:(function
      | Syntax.Return (Some value) ->
        Syntax.Block [Syntax.Assign (target, value); Syntax.Return None]
      | Syntax.If (condition, consequent, alternative) ->
        Syntax.If
          ( condition,
            constructor_body target consequent,
            constructor_body target alternative )
      | Syntax.While (condition, body) ->
        Syntax.While (condition, constructor_body target body)
      | Syntax.Foreach (collection, key, value, body) ->
        Syntax.Foreach (collection, key, value, constructor_body target body)
      | Syntax.Try (body, catches, finally) ->
        Syntax.Try
          ( constructor_body target body,
            List.map catches ~f:(fun (hint, local, body) ->
                (hint, local, constructor_body target body)),
            constructor_body target finally )
      | Syntax.Concurrent body ->
        Syntax.Concurrent (constructor_body target body)
      | Syntax.Block body -> Syntax.Block (constructor_body target body)
      | ( Syntax.Bind _ | Syntax.Assign _ | Syntax.Eval _ | Syntax.Throw _
        | Syntax.Return None ) as statement ->
        statement)

let materialize
    ~allow_unsafe_named_parameter_order ~avoid_method_override expression =
  let remaining = ref 8 in
  let definitions = ref [] in
  let add_definition source = definitions := source :: !definitions in
  let declaration ~prefix ~name ~abstract callable =
    let { is_async; parameters; contexts; return_hint; body } = callable in
    let canonical =
      (not abstract)
      && (not allow_unsafe_named_parameter_order)
      && not
           (List.for_all parameters ~f:(fun parameter ->
                primitive_hint parameter.Syntax.hint))
    in
    let parameters =
      permute_parameters parameters |> Syntax.render_parameters ~canonical
    in
    let async =
      if is_async && not abstract then
        "async "
      else
        ""
    in
    Format.sprintf
      "%s%sfunction %s(%s)[%s]: %s%s"
      prefix
      async
      name
      parameters
      (String.concat ~sep:", " contexts)
      return_hint
      (if abstract then
        ";"
      else
        " { " ^ Syntax.render_body body ^ " }")
  in
  let materialized callable call_arguments =
    let { parameters; contexts; return_hint; _ } = callable in
    let invoke callee =
      Option.value_map
        call_arguments
        ~default:(Syntax.Atom ("(" ^ Syntax.render_expr callee ^ "<>)"))
        ~f:(fun arguments -> Syntax.Call (callee, arguments))
    in
    let forms =
      if
        hierarchy_eligible parameters
        && not (avoid_method_override ~is_async:callable.is_async ~return_hint)
      then
        [Free_function; Static_method; Instance_method]
      else
        [Free_function]
    in
    let forms =
      if
        (not callable.is_async)
        && not (List.mem ["void"; "nothing"] return_hint ~equal:String.equal)
      then
        Constructor :: forms
      else
        forms
    in
    match List.nth_exn forms (Random.int (List.length forms)) with
    | Free_function ->
      let name = fresh_name "function" in
      add_definition (declaration ~prefix:"" ~name ~abstract:false callable);
      invoke (Syntax.Atom name)
    | Constructor ->
      let name = fresh_name "Constructor" in
      let target =
        Syntax.Member (Syntax.Local (Syntax.named_local "this"), "value")
      in
      let constructor =
        declaration
          ~prefix:"public "
          ~name:"__construct"
          ~abstract:false
          {
            callable with
            return_hint = "void";
            body = constructor_body target callable.body;
          }
      in
      let getter =
        declaration
          ~prefix:"public "
          ~name:"result"
          ~abstract:false
          {
            callable with
            parameters = [];
            contexts = [];
            body = [Syntax.Return (Some target)];
          }
      in
      add_definition
        (Format.sprintf
           "class %s { private %s $value; %s %s }"
           name
           return_hint
           constructor
           getter);
      let invoke arguments =
        Syntax.Call (Syntax.Member (Syntax.New (name, arguments), "result"), [])
      in
      Option.value_map
        call_arguments
        ~default:
          (Syntax.Lambda
             ( parameters,
               contexts,
               return_hint,
               [Syntax.Return (Some (invoke (arguments parameters)))] ))
        ~f:invoke
    | (Static_method | Instance_method) as form ->
      let is_static =
        match form with
        | Static_method -> true
        | Free_function
        | Constructor
        | Instance_method ->
          false
      in
      let interface = fresh_name "Interface" in
      let base = fresh_name "Base" in
      let child = fresh_name "Child" in
      let method_name = fresh_name "method" in
      let prefix =
        if is_static then
          "public static "
        else
          "public "
      in
      let signature =
        declaration ~prefix ~name:method_name ~abstract:true callable
      in
      let implementation =
        declaration ~prefix ~name:method_name ~abstract:false callable
      in
      add_definition (Format.sprintf "interface %s { %s }" interface signature);
      add_definition
        (Format.sprintf
           "class %s implements %s { %s }"
           base
           interface
           implementation);
      let derived ~name ~parent =
        let call =
          Syntax.Call
            (Syntax.StaticMember ("parent", method_name), arguments parameters)
        in
        let body = call_body ~is_async:callable.is_async ~return_hint call in
        let method_ =
          declaration
            ~prefix:("<<__Override>> " ^ prefix)
            ~name:method_name
            ~abstract:false
            { callable with body }
        in
        add_definition
          (Format.sprintf "class %s extends %s { %s }" name parent method_)
      in
      derived ~name:child ~parent:base;
      let leaf =
        if Random.bool () then (
          let grandchild = fresh_name "Grandchild" in
          derived ~name:grandchild ~parent:child;
          grandchild
        ) else
          child
      in
      let adapter ~hint ~receiver ~callee original_arguments =
        let local = Syntax.fresh_local "receiver" in
        let call = Syntax.Call (callee local, arguments parameters) in
        Syntax.Call
          ( Syntax.Lambda
              ( Syntax.parameter hint local :: parameters,
                contexts,
                return_hint,
                call_body ~is_async:false ~return_hint call ),
            receiver :: original_arguments )
      in
      if is_static then
        match call_arguments with
        | Some original_arguments when Random.bool () ->
          adapter
            ~hint:("classname<" ^ base ^ ">")
            ~receiver:(Syntax.Nameof leaf)
            ~callee:(fun local ->
              Syntax.DynamicStaticMember (local, method_name))
            original_arguments
        | _ -> invoke (Syntax.StaticMember (leaf, method_name))
      else
        let callee = Syntax.Member (Syntax.New (leaf, []), method_name) in
        (match call_arguments with
        | Some original_arguments ->
          if Random.bool () then
            Syntax.Call (callee, original_arguments)
          else
            adapter
              ~hint:
                (if Random.bool () then
                  interface
                else
                  base)
              ~receiver:(Syntax.New (leaf, []))
              ~callee:(fun local ->
                Syntax.Member (Syntax.Local local, method_name))
              original_arguments
        | None ->
          Syntax.Lambda
            ( parameters,
              contexts,
              return_hint,
              call_body
                ~is_async:false
                ~return_hint
                (Syntax.Call (callee, arguments parameters)) ))
  in
  let rec map_expression = function
    | Syntax.Call
        (Syntax.Lambda (parameters, contexts, return_hint, body), arguments) ->
      map_callable false parameters contexts return_hint body (Some arguments)
    | Syntax.Call
        (Syntax.AsyncLambda (parameters, contexts, return_hint, body), arguments)
      ->
      map_callable true parameters contexts return_hint body (Some arguments)
    | Syntax.Lambda (parameters, contexts, return_hint, body) ->
      map_callable false parameters contexts return_hint body None
    | Syntax.AsyncLambda (parameters, contexts, return_hint, body) ->
      map_callable true parameters contexts return_hint body None
    | Syntax.Unary (operator, value) ->
      Syntax.Unary (operator, map_expression value)
    | Syntax.Binary (operator, left, right) ->
      Syntax.Binary (operator, map_expression left, map_expression right)
    | Syntax.As (value, hint) -> Syntax.As (map_expression value, hint)
    | Syntax.Is (value, hint) -> Syntax.Is (map_expression value, hint)
    | Syntax.NullsafeMember (value, name) ->
      Syntax.NullsafeMember (map_expression value, name)
    | Syntax.Member (value, name) -> Syntax.Member (map_expression value, name)
    | Syntax.Await value -> Syntax.Await (map_expression value)
    | Syntax.Xhp (name, attributes, children) ->
      Syntax.Xhp
        ( name,
          List.map attributes ~f:(fun (name, value) ->
              (name, map_expression value)),
          List.map children ~f:map_expression )
    | Syntax.New (name, arguments) ->
      Syntax.New (name, List.map arguments ~f:map_expression)
    | Syntax.NewDynamic (name, arguments) ->
      Syntax.NewDynamic (name, List.map arguments ~f:map_expression)
    | Syntax.Call (callee, arguments) ->
      Syntax.Call (map_expression callee, List.map arguments ~f:map_expression)
    | Syntax.NamedArgument (name, value) ->
      Syntax.NamedArgument (name, map_expression value)
    | Syntax.Index (value, index) ->
      Syntax.Index (map_expression value, map_expression index)
    | Syntax.Append value -> Syntax.Append (map_expression value)
    | Syntax.Inout value -> Syntax.Inout (map_expression value)
    | Syntax.Unpack value -> Syntax.Unpack (map_expression value)
    | Syntax.Array (kind, values) ->
      Syntax.Array (kind, List.map values ~f:map_expression)
    | Syntax.KeyValue (key, value) ->
      Syntax.KeyValue (map_expression key, map_expression value)
    | Syntax.Tuple values -> Syntax.Tuple (List.map values ~f:map_expression)
    | Syntax.Shape fields ->
      Syntax.Shape
        (List.map fields ~f:(fun (key, value) -> (key, map_expression value)))
    | Syntax.Async body -> Syntax.Async (List.map body ~f:map_statement)
    | ( Syntax.Atom _ | Syntax.Local _ | Syntax.DynamicStaticMember _
      | Syntax.Nameof _ | Syntax.StaticMember _ | Syntax.EnumLabel _
      | Syntax.StaticProperty _ | Syntax.Quote _ | Syntax.Splice _ ) as
      expression ->
      expression
  and map_callable is_async parameters contexts return_hint body call_arguments
      =
    let lift =
      !remaining > 0
      && List.exists parameters ~f:(fun parameter -> parameter.Syntax.named)
      && List.for_all parameters ~f:(fun parameter ->
             not (String.is_substring parameter.Syntax.hint ~substring:"inout "))
      && Option.for_all call_arguments ~f:(fun arguments ->
             List.for_all arguments ~f:(function
                 | Syntax.Inout _ -> false
                 | _ -> true))
      && closed_body [] parameters contexts body
      && Random.int 4 <> 0
    in
    if lift then decr remaining;
    let parameters =
      List.map parameters ~f:(fun parameter ->
          {
            parameter with
            Syntax.default =
              Option.map parameter.Syntax.default ~f:map_expression;
          })
    in
    let body = List.map body ~f:map_statement in
    let call_arguments =
      Option.map call_arguments ~f:(List.map ~f:map_expression)
    in
    let callable = { is_async; parameters; contexts; return_hint; body } in
    if lift then
      materialized callable call_arguments
    else
      let lambda =
        if is_async then
          Syntax.AsyncLambda (parameters, contexts, return_hint, body)
        else
          Syntax.Lambda (parameters, contexts, return_hint, body)
      in
      Option.value_map call_arguments ~default:lambda ~f:(fun arguments ->
          Syntax.Call (lambda, arguments))
  and map_statement = function
    | Syntax.If (condition, consequent, alternative) ->
      Syntax.If
        ( map_expression condition,
          List.map consequent ~f:map_statement,
          List.map alternative ~f:map_statement )
    | Syntax.While (condition, body) ->
      Syntax.While (map_expression condition, List.map body ~f:map_statement)
    | Syntax.Foreach (collection, key, value, body) ->
      Syntax.Foreach
        (map_expression collection, key, value, List.map body ~f:map_statement)
    | Syntax.Try (body, catches, finally) ->
      Syntax.Try
        ( List.map body ~f:map_statement,
          List.map catches ~f:(fun (hint, local, body) ->
              (hint, local, List.map body ~f:map_statement)),
          List.map finally ~f:map_statement )
    | Syntax.Concurrent body ->
      Syntax.Concurrent (List.map body ~f:map_statement)
    | Syntax.Bind (local, value) -> Syntax.Bind (local, map_expression value)
    | Syntax.Assign (target, value) ->
      Syntax.Assign (map_expression target, map_expression value)
    | Syntax.Eval value -> Syntax.Eval (map_expression value)
    | Syntax.Return value -> Syntax.Return (Option.map value ~f:map_expression)
    | Syntax.Throw value -> Syntax.Throw (map_expression value)
    | Syntax.Block body -> Syntax.Block (List.map body ~f:map_statement)
  in
  let expression = map_expression expression in
  (List.rev !definitions, expression)

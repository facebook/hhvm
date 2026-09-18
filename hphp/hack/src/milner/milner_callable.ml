(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Milner_syntax

type callable = Syntax.declaration = {
  type_parameters: string list;
  is_async: bool;
  parameters: Syntax.parameter list;
  contexts: string list;
  return_hint: string;
  body: Syntax.stmt list;
  memoize: bool;
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
  | Syntax.DeclaredCallable (declaration, _) ->
    closed_body [] declaration.parameters declaration.contexts declaration.body
  | Syntax.DeclaredCall (declaration, _, arguments) ->
    closed_body [] declaration.parameters declaration.contexts declaration.body
    && List.for_all arguments ~f:(closed_expression bound)
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
  Milner_expression.call_arguments parameters ~value:(fun parameter ->
      Some (Syntax.Local parameter.Syntax.local))

let add_named_arguments original additions =
  let names =
    List.map additions ~f:(fun (parameter, _) ->
        Syntax.local_name parameter.Syntax.local)
  in
  let additions =
    List.map additions ~f:(fun (parameter, value) ->
        Syntax.NamedArgument (Syntax.local_name parameter.Syntax.local, value))
  in
  (* Existing argument order can carry evaluation or inference dependencies. *)
  Milner_expression.permute_named
    (function
      | Syntax.NamedArgument (name, _) ->
        List.mem names name ~equal:String.equal
      | _ -> false)
    (additions @ original)

let fresh_named_parameter ?default hint parameters =
  let last =
    List.filter_map parameters ~f:(fun parameter ->
        if parameter.Syntax.named then
          Some (Syntax.local_name parameter.Syntax.local)
        else
          None)
    |> List.max_elt ~compare:String.compare
    |> Option.value ~default:""
  in
  Syntax.parameter
    ~named:true
    ?default
    hint
    (Syntax.named_local (last ^ "_" ^ fresh_name "probe"))

let probe_value = function
  | "int" -> Syntax.Atom (string_of_int (Random.int 100))
  | "string" -> Syntax.Atom ("'probe_" ^ string_of_int (Random.int 100) ^ "'")
  | "bool" -> Syntax.Atom (string_of_bool (Random.bool ()))
  | _ -> invalid_arg "Milner: non-primitive probe domain"

let widen_probe_hint = function
  | "int" ->
    if Random.bool () then
      "num"
    else
      "arraykey"
  | "string" -> "arraykey"
  | "bool"
  | "num"
  | "arraykey"
  | "nonnull" ->
    (* T289176736: these named probes acquire defaults later in the hierarchy. *)
    "nonnull"
  | _ -> invalid_arg "Milner: non-primitive probe hint"

let check_parameter parameter expected =
  Syntax.Eval
    (Syntax.Call
       ( Syntax.Atom "invariant",
         [
           Syntax.Binary ("===", Syntax.Local parameter.Syntax.local, expected);
           Syntax.Atom
             "'method dispatch preserves named arguments and defaults'";
         ] ))

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

let callable_hint callable =
  let parameters =
    List.map callable.parameters ~f:(fun parameter ->
        let hint = parameter.Syntax.hint in
        if parameter.Syntax.variadic then
          hint ^ "..."
        else
          (if Option.is_some parameter.Syntax.default then
            "optional "
          else
            "")
          ^
          if parameter.Syntax.named then
            "named " ^ hint ^ " $" ^ Syntax.local_name parameter.Syntax.local
          else
            hint)
  in
  "(function("
  ^ String.concat ~sep:", " parameters
  ^ ")["
  ^ String.concat ~sep:", " callable.contexts
  ^ "]: "
  ^ callable.return_hint
  ^ ")"

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
  let declaration
      ?(attributes = []) ?(memoize_lsb = false) ~prefix ~name ~abstract callable
      =
    let {
      type_parameters;
      is_async;
      parameters;
      contexts;
      return_hint;
      body;
      memoize;
    } =
      callable
    in
    let attributes =
      if memoize && not abstract then
        (if memoize_lsb then
          "__MemoizeLSB"
        else
          "__Memoize")
        :: attributes
      else
        attributes
    in
    let attributes =
      if List.is_empty attributes then
        ""
      else
        "<<" ^ String.concat ~sep:", " attributes ^ ">> "
    in
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
      "%s%s%sfunction %s%s(%s)[%s]: %s%s"
      attributes
      prefix
      async
      name
      (if List.is_empty type_parameters then
        ""
      else
        "<" ^ String.concat ~sep:", " type_parameters ^ ">")
      parameters
      (String.concat ~sep:", " contexts)
      return_hint
      (if abstract then
        ";"
      else
        " { " ^ Syntax.render_body body ^ " }")
  in
  let materialized ?(type_arguments = []) callable call_arguments =
    let { parameters; contexts; return_hint; _ } = callable in
    let typed callee =
      if List.is_empty type_arguments then
        callee
      else
        Syntax.Atom
          (Syntax.render_expr callee
          ^ "<"
          ^ String.concat ~sep:", " type_arguments
          ^ ">")
    in
    let declared_only =
      (not (List.is_empty callable.type_parameters))
      || List.exists contexts ~f:(fun context ->
             String.is_prefix context ~prefix:"ctx ")
    in
    let invoke callee =
      Option.value_map
        call_arguments
        ~default:(Syntax.Atom ("(" ^ Syntax.render_expr callee ^ "<>)"))
        ~f:(fun arguments -> Syntax.Call (typed callee, arguments))
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
        (not declared_only)
        && (not callable.memoize)
        && (not callable.is_async)
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
      let memoize_lsb = is_static && callable.memoize && Random.bool () in
      let domain = List.nth_exn ["int"; "string"; "bool"] (Random.int 3) in
      let probe =
        (* T289176741: memo wrappers do not forward named arguments. *)
        if callable.memoize then
          None
        else
          Some (fresh_named_parameter domain parameters)
      in
      let signature =
        declaration
          ~prefix
          ~name:method_name
          ~abstract:true
          { callable with parameters = Option.to_list probe @ parameters }
      in
      add_definition (Format.sprintf "interface %s { %s }" interface signature);
      let names =
        [base; child]
        @
        if Random.bool () then
          [fresh_name "Grandchild"]
        else
          []
      in
      let optional_from = Random.int (List.length names) in
      let add_optional_names =
        (* T289098968: trailing new names must not shift required positional
           slots beyond the parent's required prefix. *)
        Option.is_some probe
        && not
             (List.exists parameters ~f:(fun parameter ->
                  (not parameter.Syntax.named)
                  && (not parameter.Syntax.variadic)
                  && Option.is_none parameter.Syntax.default))
      in
      let rec levels index probe extras = function
        | [] -> []
        | name :: names ->
          let probe =
            Option.map probe ~f:(fun probe ->
                {
                  probe with
                  Syntax.hint = widen_probe_hint probe.Syntax.hint;
                  default =
                    (if index >= optional_from then
                      Some (probe_value domain)
                    else
                      None);
                })
          in
          let extras =
            List.map extras ~f:(fun parameter ->
                { parameter with Syntax.default = Some (probe_value "int") })
          in
          let extras =
            if add_optional_names && Random.bool () then
              extras
              @ [
                  fresh_named_parameter
                    ~default:(probe_value "int")
                    "int"
                    (Option.to_list probe @ extras @ parameters);
                ]
            else
              extras
          in
          (name, probe, extras) :: levels (index + 1) probe extras names
      in
      let levels = levels 0 probe [] names in
      let (leaf, leaf_probe, leaf_extras) = List.last_exn levels in
      let omit_probe = Option.is_some leaf_probe && Random.bool () in
      let expected =
        Option.map leaf_probe ~f:(fun probe ->
            if omit_probe then
              Option.value_exn probe.Syntax.default
            else
              probe_value domain)
      in
      List.iteri levels ~f:(fun index (name, probe, extras) ->
          let (parent, attributes, body) =
            if index = 0 then
              ("implements " ^ interface, [], callable.body)
            else
              let (parent, parent_probe, _) = List.nth_exn levels (index - 1) in
              let probe_argument =
                Option.to_list
                  (Option.map parent_probe ~f:(fun parameter ->
                       (parameter, Syntax.Local parameter.Syntax.local)))
              in
              let call =
                Syntax.Call
                  ( Syntax.StaticMember
                      ( "parent",
                        method_name
                        ^
                        if List.is_empty callable.type_parameters then
                          ""
                        else
                          "<"
                          ^ String.concat ~sep:", " callable.type_parameters
                          ^ ">" ),
                    add_named_arguments (arguments parameters) probe_argument )
              in
              ( "extends " ^ parent,
                ["__Override"],
                call_body ~is_async:callable.is_async ~return_hint call )
          in
          let checks =
            Option.to_list
              (Option.map probe ~f:(fun parameter ->
                   check_parameter parameter (Option.value_exn expected)))
            @ List.map extras ~f:(fun parameter ->
                  check_parameter
                    parameter
                    (Option.value_exn parameter.Syntax.default))
          in
          let method_ =
            declaration
              ~attributes
              ~memoize_lsb
              ~prefix
              ~name:method_name
              ~abstract:false
              {
                callable with
                parameters = Option.to_list probe @ extras @ parameters;
                body = checks @ body;
              }
          in
          add_definition
            (Format.sprintf "class %s %s { %s }" name parent method_));
      let method_arguments ~omit original =
        add_named_arguments
          original
          (if omit then
            []
          else
            Option.to_list
              (Option.map leaf_probe ~f:(fun parameter ->
                   (parameter, Option.value_exn expected))))
      in
      let adapter ~hint ~receiver ~callee original_arguments =
        let local = Syntax.fresh_local "receiver" in
        let call =
          Syntax.Call
            (callee local, method_arguments ~omit:false (arguments parameters))
        in
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
        | Some original_arguments when (not declared_only) && Random.bool () ->
          adapter
            ~hint:("classname<" ^ base ^ ">")
            ~receiver:(Syntax.Nameof leaf)
            ~callee:(fun local ->
              Syntax.DynamicStaticMember (local, method_name))
            original_arguments
        | Some original_arguments ->
          Syntax.Call
            ( typed (Syntax.StaticMember (leaf, method_name)),
              method_arguments ~omit:omit_probe original_arguments )
        | None when Option.is_none leaf_probe && List.is_empty leaf_extras ->
          invoke (Syntax.StaticMember (leaf, method_name))
        | None ->
          Syntax.Lambda
            ( parameters,
              contexts,
              return_hint,
              call_body
                ~is_async:false
                ~return_hint
                (Syntax.Call
                   ( Syntax.StaticMember (leaf, method_name),
                     method_arguments ~omit:omit_probe (arguments parameters) ))
            )
      else
        let callee = Syntax.Member (Syntax.New (leaf, []), method_name) in
        (match call_arguments with
        | Some original_arguments ->
          if declared_only || Random.bool () then
            Syntax.Call
              ( typed callee,
                method_arguments ~omit:omit_probe original_arguments )
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
          let receiver = Syntax.fresh_local "receiver" in
          let closure =
            Syntax.Lambda
              ( parameters,
                contexts,
                return_hint,
                call_body
                  ~is_async:false
                  ~return_hint
                  (Syntax.Call
                     ( Syntax.Member (Syntax.Local receiver, method_name),
                       method_arguments ~omit:omit_probe (arguments parameters)
                     )) )
          in
          Syntax.Call
            ( Syntax.Lambda
                ( [Syntax.parameter leaf receiver],
                  [],
                  callable_hint callable,
                  [Syntax.Return (Some closure)] ),
              [Syntax.New (leaf, [])] ))
  in
  let rec map_expression = function
    | Syntax.DeclaredCall (callable, type_arguments, arguments) ->
      materialized
        ~type_arguments
        (map_declaration callable)
        (Some (List.map arguments ~f:map_expression))
    | Syntax.DeclaredCallable (callable, type_arguments) ->
      if
        (not (List.is_empty callable.type_parameters))
        || (not (List.is_empty type_arguments))
        || List.exists callable.contexts ~f:(fun context ->
               String.is_substring context ~substring:"$")
      then
        invalid_arg
          "Milner: callable values require monomorphic contexts and types";
      materialized (map_declaration callable) None
    | Syntax.Quote (visitor, Syntax.Splice value) ->
      Syntax.Quote (visitor, Syntax.Splice (map_expression value))
    | Syntax.Splice value -> Syntax.Splice (map_expression value)
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
      | Syntax.StaticProperty _ | Syntax.Quote _ ) as expression ->
      expression
  and map_declaration callable =
    if not (closed_body [] callable.parameters callable.contexts callable.body)
    then
      failwith "Milner: declared callable captures a local";
    {
      callable with
      parameters =
        List.map callable.parameters ~f:(fun parameter ->
            {
              parameter with
              Syntax.default =
                Option.map parameter.Syntax.default ~f:map_expression;
            });
      body = List.map callable.body ~f:map_statement;
    }
  and map_callable is_async parameters contexts return_hint body call_arguments
      =
    let lift =
      !remaining > 0
      && List.exists parameters ~f:(fun parameter -> parameter.Syntax.named)
      && List.for_all parameters ~f:(fun parameter ->
             (not (String.is_empty parameter.Syntax.hint))
             && not
                  (String.is_substring
                     parameter.Syntax.hint
                     ~substring:"inout "))
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
    let callable =
      {
        type_parameters = [];
        memoize = false;
        is_async;
        parameters;
        contexts;
        return_hint;
        body;
      }
    in
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

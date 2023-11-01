(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open HoverService
module SN = Naming_special_names

(** When we get a Class occurrence and a Method occurrence, that means that the
user is hovering over an invocation of the constructor, and would therefore only
want to see information about the constructor, rather than getting both the
class and constructor back in the hover. *)
let filter_class_and_constructor results =
  let result_is_constructor result =
    SymbolOccurrence.is_constructor (fst result)
  in
  let result_is_class result = SymbolOccurrence.is_class (fst result) in
  let has_class = List.exists results ~f:result_is_class in
  let has_constructor = List.exists results ~f:result_is_constructor in
  if has_class && has_constructor then
    List.filter results ~f:result_is_constructor |> List.map ~f:snd
  else
    results |> List.map ~f:snd

let docs_url_markdown name url : string =
  Printf.sprintf
    "See the [documentation for %s](%s)."
    (Markdown_lite.md_codify (Utils.strip_ns name))
    url

let typedef_docs_url ctx name : string option =
  let qualified_name = "\\" ^ name in
  Option.(
    Decl_provider.get_typedef ctx qualified_name >>= fun decl ->
    decl.Typing_defs.td_docs_url >>| fun url -> docs_url_markdown name url)

(* If [classish_name] (or any of its parents) has a documentation URL,
   return the docs of the closest type. *)
let classish_docs_url ctx classish_name : string option =
  let docs_url name =
    match Decl_provider.get_class ctx name with
    | Some decl -> Decl_provider.Class.get_docs_url decl
    | None -> None
  in

  let qualified_name = "\\" ^ classish_name in
  let ancestors =
    match Decl_provider.get_class ctx qualified_name with
    | Some decl -> Decl_provider.Class.all_ancestor_names decl
    | None -> []
  in
  List.find_map (qualified_name :: ancestors) ~f:(fun ancestor ->
      match docs_url ancestor with
      | Some url -> Some (docs_url_markdown ancestor url)
      | None -> None)

let docs_url ctx def : string option =
  let open SymbolDefinition in
  match def.kind with
  | Class
  | Enum
  | Interface
  | Trait ->
    classish_docs_url ctx def.name
  | Typedef -> typedef_docs_url ctx def.name
  | Function
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | LocalVar
  | TypeVar
  | Param
  | Typeconst
  | Module ->
    None

let make_hover_doc_block ctx entry occurrence def_opt =
  match def_opt with
  | Some def when not occurrence.SymbolOccurrence.is_declaration ->
    (* The docblock is useful at the call site, but it's redundant at
       the definition site. *)
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    let doc_block_hover =
      ServerDocblockAt.go_comments_for_symbol_ctx
        ~ctx
        ~entry
        ~def
        ~base_class_name
      |> Option.to_list
    in
    (match docs_url ctx def with
    | Some info -> info :: doc_block_hover
    | None -> doc_block_hover)
  | None
  | Some _ ->
    []

(* Given a function/method call receiver, find the position of the
   definition site. *)
let callee_def_pos ctx recv : Pos_or_decl.t option =
  SymbolOccurrence.(
    match recv with
    | FunctionReceiver fun_name ->
      let f = Decl_provider.get_fun ctx fun_name in
      Option.map f ~f:(fun fe -> fe.Typing_defs.fe_pos)
    | MethodReceiver { cls_name; _ } ->
      let c = Decl_provider.get_class ctx cls_name in
      Option.map c ~f:Decl_provider.Class.pos)

(* Return the name of the [n]th parameter in [params], handling
   variadics correctly. *)
let nth_param_name (params : ('a, 'b) Aast.fun_param list) (n : int) :
    string option =
  let param =
    if n >= List.length params then
      match List.last params with
      | Some param when param.Aast.param_is_variadic -> Some param
      | _ -> None
    else
      List.nth params n
  in
  Option.map param ~f:(fun param ->
      if param.Aast.param_is_variadic then
        "..." ^ param.Aast.param_name
      else
        param.Aast.param_name)

(* Return the name of the [n]th parameter of function [fun_name]. *)
let nth_fun_param tast fun_name n : string option =
  List.find_map tast ~f:(fun def ->
      match def with
      | Aast.Fun { Aast.fd_fun; fd_name; _ } ->
        if String.equal fun_name (snd fd_name) then
          nth_param_name fd_fun.Aast.f_params n
        else
          None
      | _ -> None)

(* Return the name of the [n]th parameter of this method. *)
let nth_meth_param tast ~cls_name ~meth_name ~is_static ~arg_n =
  let class_methods =
    List.find_map tast ~f:(fun def ->
        match def with
        | Aast.Class c ->
          if String.equal cls_name (snd c.Aast.c_name) then
            Some c.Aast.c_methods
          else
            None
        | _ -> None)
    |> Option.value ~default:[]
  in
  let class_method =
    List.find_map class_methods ~f:(fun m ->
        if
          String.equal meth_name (snd m.Aast.m_name)
          && Bool.equal is_static m.Aast.m_static
        then
          Some m
        else
          None)
  in
  match class_method with
  | Some m -> nth_param_name m.Aast.m_params arg_n
  | None -> None

let nth_param ctx recv i : string option =
  match callee_def_pos ctx recv with
  | Some pos ->
    let path = Pos_or_decl.filename pos in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in
    let tast = tast.Tast_with_dynamic.under_normal_assumptions in
    SymbolOccurrence.(
      (match recv with
      | FunctionReceiver fun_name -> nth_fun_param tast fun_name i
      | MethodReceiver { cls_name; meth_name; is_static } ->
        nth_meth_param tast ~cls_name ~meth_name ~is_static ~arg_n:i))
  | None -> None

let make_hover_const_definition entry def_opt =
  Option.map def_opt ~f:(fun def ->
      Pos.get_text_from_pos
        ~content:(Provider_context.read_file_contents_exn entry)
        def.SymbolDefinition.span)

(* Return a markdown description of built-in Hack attributes. *)
let make_hover_attr_docs name =
  Option.first_some
    (SMap.find_opt name SN.UserAttributes.as_map)
    (SMap.find_opt name SN.UserAttributes.systemlib_map)
  |> Option.map ~f:(fun attr_info -> attr_info.SN.UserAttributes.doc)
  |> Option.to_list

let pure_context_info =
  "This function has an empty context list, so it has no capabilities."
  ^ "\nIt may only read properties, access constants, or call other pure functions."

let hh_fixme_info =
  "`HH_FIXME[N]` disables type checker error N on the next line. It does
not change runtime behavior.

**`HH_FIXME` is almost always a bad idea**. You might get an
exception, or you might get an unexpected value. Even scarier, you
might cause an exception in a totally different part of the codebase.

```
/* HH_FIXME[4110] My reason here */
takes_num(\"x\") + takes_num(\"y\");
```

In this example, the type checker will accept the code, but the code
will still crash when you run it (`TypeHintViolationException` when
calling `takes_num`).

Note that `HH_FIXME` applies to all occurrences of the error on the
next line.

## You can fix it!

It is always possible to write code without `HH_FIXME`. This usually
requires changing type signatures and some refactoring. Your code will
be more reliable, the type checker can help you, and future changes
will be less scary.

See also `HH\\FIXME\\UNSAFE_CAST()`, which is still bad, but much more
precise."

let keyword_info (khi : SymbolOccurrence.keyword_with_hover_docs) : string =
  let await_explanation =
    "\n\nThis does not give you threads. Only one function is running at any point in time."
    ^ " Instead, the runtime may switch to another function at an `await` expression, and come back to this function later."
    ^ "\n\nThis allows data fetching (e.g. database requests) to happen in parallel."
  in

  match khi with
  | SymbolOccurrence.Class ->
    "A `class` contains methods, properties and constants that together solve a problem."
  | SymbolOccurrence.Interface ->
    "An `interface` defines signatures for methods that classes must implement."
  | SymbolOccurrence.Trait ->
    "A `trait` provides methods and properties that can be `use`d in classes."
    ^ "\n\nTraits are often used to provide default implementations for methods declared in an `interface`."
    ^ "\n\nWhen in doubt, use a class rather than a trait. You only need a trait when you want the same method in multiple classes that don't inherit from each other."
  | SymbolOccurrence.Enum ->
    "An `enum` is a fixed set of string or integer values."
    ^ "\n\nYou can use `switch` with an `enum` and the type checker will ensure you handle every possible value."
  | SymbolOccurrence.EnumClass ->
    "An `enum class` is a fixed set of values that can be used as typed constants."
    ^ "\n\nEnum classes enable you to write generic getter and setter methods."
    ^ "\n\n```
enum class PersonFields: BaseField {
  Field<string> name = Field::string();
  Field<string> email = Field::string();
  Field<int> age = Field::int();
}
```"
    ^ "\n\nYou can refer to items within an enum class by name (using the `#` syntax), and Hack knows the exact type."
    ^ "\n\n```
function person_get(
  Person $p,
  HH\\EnumClass\\Label<PersonFields, Field<T>> $field_name
  ): T {
  // ... implementation here ...
}

person_get($a_person, #email); // string
```"
    ^ "\n\nSee also `HH\\EnumClass\\Label` and `HH\\MemberOf`."
  | SymbolOccurrence.Type ->
    "A `type` is an alias for another type."
    ^ "\n\n`type` aliases are transparent, so you can use the original type and its alias interchangeably."
    ^ "\n\nSee also `newtype` for opaque type aliases."
  | SymbolOccurrence.Newtype ->
    "A `newtype` is a type alias that is opaque."
    ^ "\n\nInside the current file, code can see the underlying type. In all other files, code cannot see the underlying type."
    ^ " This enables you to hide implementation details."
    ^ "\n\nSee also `type` for transparent type aliases."
  | SymbolOccurrence.FinalOnClass ->
    "A `final` class cannot be extended by other classes.\n\nTo restrict which classes can extend this, use `<<__Sealed()>>`."
  | SymbolOccurrence.FinalOnMethod ->
    "A `final` method cannot be overridden in child classes."
  | SymbolOccurrence.AbstractOnClass ->
    "An `abstract` class can only contain `static` methods and `abstract` instance methods.\n\n"
    ^ "`abstract` classes cannot be instantiated directly. You can only use `new` on child classes that aren't `abstract`."
  | SymbolOccurrence.AbstractOnMethod ->
    "An `abstract` method has a signature but no body. Child classes must provide an implementation."
  | SymbolOccurrence.ExtendsOnClass ->
    "Extending a class allows your class to inherit methods from another class."
    ^ "\n\nInheritance allows your class to:"
    ^ "\n * Reuse methods from the parent class"
    ^ "\n * Call `protected` methods on the parent class"
    ^ "\n * Be passed as a parameter whenever an instance of the parent class is expected"
    ^ "\n\nHack does not support multiple inheritance on classes. If you need to share functionality between"
    ^ " unrelated classes, use traits."
  | SymbolOccurrence.ExtendsOnInterface ->
    "Extending an interface allows your interface to include methods from other interfaces."
    ^ "\n\nAn interface can extend multiple interfaces."
  | SymbolOccurrence.ReadonlyOnMethod ->
    "A `readonly` method treats `$this` as `readonly`."
  | SymbolOccurrence.ReadonlyOnExpression
  | SymbolOccurrence.ReadonlyOnParameter ->
    "A `readonly` value is a reference that cannot modify the underlying value."
  | SymbolOccurrence.ReadonlyOnReturnType ->
    "This function/method may return a `readonly` value."
  | SymbolOccurrence.XhpAttribute ->
    "`attribute` declares which attributes are permitted on the current XHP class."
    ^ "\n\nAttributes are optional unless marked with `@required`."
  | SymbolOccurrence.XhpChildren ->
    "`children` declares which XHP types may be used as children when creating instances of this class."
    ^ "\n\nFor example, `children (:p)+` means that users may write `<my-class><p>hello</p><p>world</p></my_class>`."
    ^ "\n\n**`children` is not enforced by the type checker**, but an XHP framework can choose to validate it at runtime."
  | SymbolOccurrence.ConstGlobal -> "A `const` is a global constant."
  | SymbolOccurrence.ConstOnClass ->
    "A class constant."
    ^ "\n\nClass constants have public visibility, so you can access `MyClass::MY_CONST` anywhere."
  | SymbolOccurrence.ConstType ->
    "A `const type` declares a type constant inside a class. You can refer to type constants in signatures with `this::TMyConstType`."
    ^ "\n\nType constants are also a form of generics."
    ^ "\n\n```"
    ^ "\nabstract class Pet {"
    ^ "\n  abstract const type TFood;"
    ^ "\n}"
    ^ "\n"
    ^ "\nclass Cat extends Pet {"
    ^ "\n  const type TFood = Fish;"
    ^ "\n}"
    ^ "\n```"
    ^ "\n\nType constants are static, not per-instance. All instances of `Cat` have the same value for `TFood`, whereas `MyObject<T>` generics can differ between instances."
    ^ "\n\nThis enables type constants to be used outside the class, e.g. `Cat::TFood`."
  | SymbolOccurrence.StaticOnMethod ->
    "A static method can be called without an instance, e.g. `MyClass::my_method()`."
  | SymbolOccurrence.StaticOnProperty ->
    "A static property is shared between all instances of a class. It can be accessed with `MyClass::$myProperty`."
  | SymbolOccurrence.Use ->
    "Include all the items (methods, properties etc) from a trait in this class/trait."
    ^ "\n\nIf this class/trait already has an item of the same name, the trait item is not copied."
  | SymbolOccurrence.FunctionOnMethod ->
    "A `function` inside a class declares a method."
  | SymbolOccurrence.FunctionGlobal -> "A standalone global function."
  | SymbolOccurrence.Async ->
    "An `async` function can use `await` to get results from other `async` functions. You may still return plain values, e.g. `return 1;` is permitted in an `Awaitable<int>` function."
    ^ await_explanation
  | SymbolOccurrence.AsyncBlock ->
    "An `async` block is syntactic sugar for an `async` lambda that is immediately called."
    ^ "\n\n```"
    ^ "\n$f = async { return 1; };"
    ^ "\n// Equivalent to:"
    ^ "\n$f = (async () ==> { return 1; })();"
    ^ "\n```"
    ^ "\n\nThis is useful when building more complex async expressions."
    ^ "\n\n```"
    ^ "\nconcurrent {"
    ^ "\n  $group_name = await async {"
    ^ "\n    return $group is null ? '' : await $group->genName();"
    ^ "\n  };"
    ^ "\n  await async {"
    ^ "\n    try {"
    ^ "\n      await gen_log_request();"
    ^ "\n    } catch (LogRequestFailed $_) {}"
    ^ "\n  }"
    ^ "\n}"
    ^ "\n```"
  | SymbolOccurrence.Await ->
    "`await` waits for the result of an `Awaitable<_>` value."
    ^ await_explanation
  | SymbolOccurrence.Concurrent ->
    "`concurrent` allows you to `await` multiple values at once. This is similar to `Vec\\map_async`, but `concurrent` allows awaiting unrelated values of different types."
  | SymbolOccurrence.Public ->
    "A `public` method or property has no restrictions on access. It can be accessed from any part of the codebase."
    ^ "\n\nSee also `protected` and `private`."
  | SymbolOccurrence.Protected ->
    "A `protected` method or property can only be accessed from methods defined on the current class, or methods on subclasses."
    ^ "\n\nIf the current class `use`s a trait, the trait methods can also access `protected` methods and properties."
    ^ "\n\nSee also `public` and `private`."
  | SymbolOccurrence.Private ->
    "A `private` method or property can only be accessed from methods defined on the current class."
    ^ "\n\nPrivate items can be accessed on any instance of the current class. "
    ^ "For example, if you have a private property `name`, you can access both `$this->name` and `$other_instance->name`."
    ^ "\n\nSee also `public` and `protected`."
  | SymbolOccurrence.Internal ->
    "An `internal` symbol can only be accessed from files that belong to the current `module`."
  | SymbolOccurrence.ModuleInModuleDeclaration ->
    "`new module Foo {}` defines a new module but does not associate any code with it."
    ^ "\n\nYou must use `module Foo;` to mark all the definitions in a given file as associated with the `Foo` module and enable them to use `internal`."
  | SymbolOccurrence.ModuleInModuleMembershipDeclaration ->
    "`module Foo;` marks all the definitions in the current file as associated with the `Foo` module, and enables them to use `internal`."
    ^ "\n\nYou must also define this module with `new module Foo {}` inside or outside this file."

let split_class_name (full_name : string) : string =
  match String.lsplit2 full_name ~on:':' with
  | Some (class_name, _member) -> class_name
  | None -> full_name

let fun_defined_in def_opt : string =
  match def_opt with
  | Some { SymbolDefinition.full_name; _ } ->
    let abs_name = "\\" ^ full_name in
    if SN.PseudoFunctions.is_pseudo_function abs_name then
      ""
    else (
      match String.rsplit2 (Utils.strip_hh_lib_ns abs_name) ~on:'\\' with
      | Some (namespace, _) when not (String.equal namespace "") ->
        Printf.sprintf "// Defined in namespace %s\n" namespace
      | _ -> ""
    )
  | _ -> ""

let make_hover_info under_dynamic_result ctx env_and_ty entry occurrence def_opt
    =
  SymbolOccurrence.(
    Typing_defs.(
      let defined_in =
        match def_opt with
        | Some def ->
          Printf.sprintf
            "// Defined in %s\n"
            (split_class_name def.SymbolDefinition.full_name)
        | None -> ""
      in

      let snippet =
        match (occurrence, env_and_ty) with
        | ({ name; _ }, None) -> Utils.strip_hh_lib_ns name
        | ({ type_ = Method (ClassName classname, name); _ }, Some (env, ty))
          when String.equal name Naming_special_names.Members.__construct ->
          let snippet_opt =
            Option.Monad_infix.(
              Decl_provider.get_class ctx classname >>= fun c ->
              fst (Decl_provider.Class.construct c) >>| fun elt ->
              let ty = Lazy.force_val elt.ce_type in
              Tast_env.print_ty_with_identity env (DeclTy ty) occurrence def_opt)
          in
          defined_in
          ^
          (match snippet_opt with
          | Some s -> s
          | None ->
            Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt)
        | ({ type_ = BestEffortArgument (recv, i); _ }, _) ->
          let param_name = nth_param ctx recv i in
          Printf.sprintf "Parameter: %s" (Option.value ~default:"$_" param_name)
        | ({ type_ = Method _; _ }, Some (env, ty))
        | ({ type_ = ClassConst _; _ }, Some (env, ty))
        | ({ type_ = Property _; _ }, Some (env, ty)) ->
          let ty = Tast_env.strip_dynamic env ty in
          defined_in
          ^ Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
        | ({ type_ = GConst; _ }, Some (env, ty)) ->
          (match make_hover_const_definition entry def_opt with
          | Some def_txt -> def_txt
          | None ->
            Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt)
        | ({ type_ = Function; _ }, Some (env, ty)) ->
          fun_defined_in def_opt
          ^ Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
        | (occurrence, Some (env, ty)) ->
          Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
          ^ under_dynamic_result
      in
      let addendum =
        match occurrence with
        | { name; type_ = Attribute _; _ } ->
          List.concat
            [
              make_hover_attr_docs name;
              make_hover_doc_block ctx entry occurrence def_opt;
            ]
        | { type_ = Keyword info; _ } -> [keyword_info info]
        | { type_ = HhFixme; _ } -> [hh_fixme_info]
        | { type_ = PureFunctionContext; _ } -> [pure_context_info]
        | { type_ = BuiltInType bt; _ } ->
          [SymbolOccurrence.built_in_type_hover bt]
        | _ -> make_hover_doc_block ctx entry occurrence def_opt
      in
      HoverService.
        { snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }))

let make_hover_info_with_fallback under_dynamic_result results =
  let class_fallback =
    List.hd
      (List.filter results ~f:(fun (_, _, _, occurrence, _) ->
           SymbolOccurrence.is_class occurrence))
  in
  List.map
    ~f:(fun (ctx, env_and_ty, entry, occurrence, def_opt) ->
      if
        SymbolOccurrence.is_constructor occurrence
        && List.is_empty (make_hover_doc_block ctx entry occurrence def_opt)
      then
        (* Case where constructor docblock is empty. *)
        let hover_info =
          make_hover_info
            under_dynamic_result
            ctx
            env_and_ty
            entry
            occurrence
            def_opt
        in
        match class_fallback with
        | Some (ctx, _, entry, class_occurrence, def_opt) ->
          let fallback_doc_block =
            make_hover_doc_block ctx entry class_occurrence def_opt
          in
          ( occurrence,
            HoverService.
              {
                snippet = hover_info.snippet;
                addendum = List.concat [fallback_doc_block; hover_info.addendum];
                pos = hover_info.pos;
              } )
        | None -> (occurrence, hover_info)
      else
        ( occurrence,
          make_hover_info
            under_dynamic_result
            ctx
            env_and_ty
            entry
            occurrence
            def_opt ))
    results

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : HoverService.result =
  let identities =
    ServerIdentifyFunction.go_quarantined ~ctx ~entry ~line ~column
  in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let env_and_ty =
    ServerInferType.human_friendly_type_at_pos
      ~under_dynamic:false
      ctx
      tast
      line
      column
  in
  let env_and_ty_dynamic =
    ServerInferType.human_friendly_type_at_pos
      ~under_dynamic:true
      ctx
      tast
      line
      column
  in
  let under_dynamic_result =
    match env_and_ty_dynamic with
    | Some (env, ty') ->
      (match env_and_ty with
      | None -> ""
      | Some (_, ty) ->
        if Typing_defs.ty_equal ty ty' then
          ""
        else
          Printf.sprintf
            " (%s when called dynamically)"
            (Tast_env.print_ty env ty'))
    | None -> ""
  in
  let result =
    match (identities, env_and_ty) with
    | ([], Some (env, ty)) ->
      (* There are no identities (named entities) at the cursor, but we
         know the type of the expression. Just show the type.

         This can occur if the user hovers over a literal such as `123`. *)
      [
        {
          snippet = Tast_env.print_ty env ty ^ under_dynamic_result;
          addendum = [];
          pos = None;
        };
      ]
    | ( [
          ( {
              SymbolOccurrence.type_ =
                SymbolOccurrence.BestEffortArgument (recv, i);
              _;
            },
            _ );
        ],
        _ ) ->
      (* There are no identities (named entities) at the cursor, but we
         know the type of the expression and the name of the parameter
         from the definition site.

         This can occur if the user hovers over a literal in a call,
         e.g. `foo(123)`. *)
      let ty_result =
        match env_and_ty with
        | Some (env, ty) ->
          [
            {
              snippet = Tast_env.print_ty env ty ^ under_dynamic_result;
              addendum = [];
              pos = None;
            };
          ]
        | None -> []
      in
      let param_result =
        match nth_param ctx recv i with
        | Some param_name ->
          [
            {
              snippet = Printf.sprintf "Parameter: %s" param_name;
              addendum = [];
              pos = None;
            };
          ]
        | None -> []
      in
      ty_result @ param_result
    | (identities, _) ->
      (* We have a list of named things at the cursor. Show the
         docblock and type of each thing. *)
      identities
      |> List.map ~f:(fun (occurrence, def_opt) ->
             (* If we're hovering over a type hint, we're not interested
                in the type of the enclosing expression. *)
             let env_and_ty =
               match occurrence.SymbolOccurrence.type_ with
               | SymbolOccurrence.TypeVar -> None
               | SymbolOccurrence.BuiltInType _ -> None
               | _ -> env_and_ty
             in
             let path =
               def_opt
               |> Option.map ~f:(fun def -> def.SymbolDefinition.pos)
               |> Option.map ~f:Pos.filename
               |> Option.value ~default:entry.Provider_context.path
             in
             let (ctx, entry) =
               Provider_context.add_entry_if_missing ~ctx ~path
             in
             (ctx, env_and_ty, entry, occurrence, def_opt))
      |> make_hover_info_with_fallback under_dynamic_result
      |> filter_class_and_constructor
      |> List.remove_consecutive_duplicates ~equal:equal_hover_info
  in
  result

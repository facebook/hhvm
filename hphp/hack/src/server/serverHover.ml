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

let make_hover_doc_block ctx entry occurrence def_opt =
  match def_opt with
  | Some def when not occurrence.SymbolOccurrence.is_declaration ->
    (* The docblock is useful at the call site, but it's redundant at
       the definition site. *)
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    ServerDocblockAt.go_comments_for_symbol_ctx
      ~ctx
      ~entry
      ~def
      ~base_class_name
    |> Option.to_list
  | None
  | Some _ ->
    []

let make_hover_const_definition entry def_opt =
  match def_opt with
  | Some def ->
    [
      Pos.get_text_from_pos
        ~content:(Provider_context.read_file_contents_exn entry)
        def.SymbolDefinition.span;
    ]
  | _ -> []

let make_hover_full_name env_and_ty occurrence def_opt =
  SymbolOccurrence.(
    let found_it () =
      let name =
        match def_opt with
        | Some def -> def.SymbolDefinition.full_name
        | None -> occurrence.name
      in
      [Printf.sprintf "Full name: `%s`" (Utils.strip_ns name)]
    in
    Typing_defs.(
      match (occurrence, env_and_ty) with
      | ({ type_ = Method _; _ }, Some (_, _ty)) -> found_it ()
      | ( { type_ = Property _ | ClassConst _ | XhpLiteralAttr _; _ },
          Some (_, ty) )
        when is_fun ty ->
        found_it ()
      | _ -> []))

(* Return a markdown description of built-in Hack attributes. *)
let make_hover_attr_docs name =
  let special_name_docs attr_name map =
    Option.map ~f:snd (SMap.find_opt attr_name map)
  in
  Option.first_some
    (special_name_docs name SN.UserAttributes.as_map)
    (special_name_docs name SN.UserAttributes.systemlib_map)
  |> Option.to_list

let pure_context_info =
  "This function has an empty context list, so it has no capabilities."
  ^ "\nIt may only read properties, access constants, or call other pure functions."

let built_in_type_info (bt : SymbolOccurrence.built_in_type_hint) : string =
  match bt with
  | SymbolOccurrence.BIprimitive prim ->
    (match prim with
    | Aast_defs.Tnull -> "The value `null`. The opposite of `nonnull`."
    | Aast_defs.Tvoid ->
      "A `void` return type indicates a function that never returns a value. `void` functions usually have side effects."
    | Aast_defs.Tint -> "A 64-bit integer."
    | Aast_defs.Tbool -> "A boolean value, `true` or `false`."
    | Aast_defs.Tfloat -> "A 64-bit floating-point number."
    | Aast_defs.Tstring -> "A sequence of characters."
    | Aast_defs.Tresource ->
      "An external resource, such as a file handle or database connection."
    | Aast_defs.Tnum -> "An `int` or a `float`."
    | Aast_defs.Tarraykey ->
      "An `int` or a `string`. `arraykey` is a common key type for `dict`s."
    | Aast_defs.Tnoreturn ->
      "A `noreturn` function always errors or loops forever.")
  | SymbolOccurrence.BImixed ->
    "Any value at all. It's usually better to use a more specific type, or a generic."
  | SymbolOccurrence.BIdynamic ->
    "Any value at all. Unlike `mixed`, the type checker allows any operation on a `dynamic` value, even if e.g. a method doesn't actually exist.\n\n"
    ^ "All operations on a `dynamic` value return another `dynamic` value. Prefer more specific types so the type checker can help you.\n\n"
    ^ "To convert a `generic` value to something specific, use `$foo as SomeSpecificType`. This will check the type at runtime and the "
    ^ "type checker will verify types after this point."
  | SymbolOccurrence.BInothing ->
    "The `nothing` type has no values, it's the empty type.\n\nA function returning `nothing` either loops forever or throws an exception. A `vec<nothing>` is always empty."
  | SymbolOccurrence.BInonnull -> "Any value except `null`."
  | SymbolOccurrence.BIshape ->
    "A shape is a key-value data structure where the keys are known."
    ^ " Shapes are value types, just like `dict` and `vec`.\n\n"
    ^ " A closed shape, such as `shape('x' => int)`, has a fixed number of keys. "
    ^ " An open shape, such as `shape('x' => int, ...)`, may have additional keys."
  | SymbolOccurrence.BIthis ->
    "`this` represents the current class.\n\n"
    ^ "`this` refers to the type of the current instance at runtime. In a child class, `this` refers to the child class, even if the method is defined in the parent."
  | SymbolOccurrence.BIoption ->
    "The type `?Foo` allows either `Foo` or `null`."

let keyword_info (khi : SymbolOccurrence.keyword_with_hover_docs) : string =
  let await_explanation =
    "\n\nThis does not give you threads. Only one function is running at any point in time."
    ^ " Instead, the runtime may switch to another function at an `await` expression, and come back to this function later."
    ^ "\n\nThis allows data fetching (e.g. database requests) to happen in parallel."
  in

  match khi with
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
    ^ "\n\nIf the current class `use`s a trait, the trait methods can also access `private` methods and properties."
    ^ "\n\nSee also `public` and `protected`."

let make_hover_info ctx env_and_ty entry occurrence def_opt =
  SymbolOccurrence.(
    Typing_defs.(
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
          begin
            match snippet_opt with
            | Some s -> s
            | None ->
              Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
          end
        | (occurrence, Some (env, ty)) ->
          Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
      in
      let addendum =
        match occurrence with
        | { name; type_ = Attribute _; _ } ->
          List.concat
            [
              make_hover_attr_docs name;
              make_hover_doc_block ctx entry occurrence def_opt;
            ]
        | { type_ = GConst; _ } ->
          List.concat
            [
              make_hover_doc_block ctx entry occurrence def_opt;
              make_hover_const_definition entry def_opt;
            ]
        | { type_ = Keyword info; _ } -> [keyword_info info]
        | { type_ = PureFunctionContext; _ } -> [pure_context_info]
        | { type_ = BuiltInType bt; _ } -> [built_in_type_info bt]
        | _ ->
          List.concat
            [
              make_hover_doc_block ctx entry occurrence def_opt;
              make_hover_full_name env_and_ty occurrence def_opt;
            ]
      in
      HoverService.
        { snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }))

let make_hover_info_with_fallback results =
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
          make_hover_info ctx env_and_ty entry occurrence def_opt
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
        (occurrence, make_hover_info ctx env_and_ty entry occurrence def_opt))
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
    ServerInferType.human_friendly_type_at_pos ctx tast line column
  in
  (* There are legitimate cases where we expect to have no identities returned,
     so just format the type. *)
  match identities with
  | [] ->
    begin
      match env_and_ty with
      | Some (env, ty) ->
        [{ snippet = Tast_env.print_ty env ty; addendum = []; pos = None }]
      | None -> []
    end
  | identities ->
    identities
    |> List.map ~f:(fun (occurrence, def_opt) ->
           let env_and_ty =
             match occurrence.SymbolOccurrence.type_ with
             | SymbolOccurrence.TypeVar -> None
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
    |> make_hover_info_with_fallback
    |> filter_class_and_constructor
    |> List.remove_consecutive_duplicates ~equal:equal_hover_info

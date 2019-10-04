(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open HoverService

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
    List.filter results ~f:result_is_constructor
  else
    results

let make_hover_doc_block file occurrence def_opt =
  match def_opt with
  | Some def ->
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    ServerDocblockAt.go_comments_for_symbol ~def ~base_class_name ~file
    |> Option.to_list
  | None -> []

let make_hover_return_type env_and_ty occurrence =
  SymbolOccurrence.(
    Typing_defs.(
      match (occurrence, env_and_ty) with
      | ({ type_ = Function | Method _; _ }, Some (env, (_, Tfun ft))) ->
        [
          Printf.sprintf
            "Return type: `%s`"
            (Tast_env.print_ty env ft.ft_ret.et_type);
        ]
      | _ -> []))

let make_hover_full_name env_and_ty occurrence def_opt =
  SymbolOccurrence.(
    Typing_defs.(
      match (occurrence, env_and_ty) with
      | ({ type_ = Method _; _ }, _)
      | ({ type_ = Property _ | ClassConst _; _ }, Some (_, (_, Tfun _))) ->
        let name =
          match def_opt with
          | Some def -> def.SymbolDefinition.full_name
          | None -> occurrence.name
        in
        [Printf.sprintf "Full name: `%s`" (Utils.strip_ns name)]
      | _ -> []))

(* Return a markdown description of built-in Hack attributes. *)
let make_hover_attr_docs name =
  match name with
  | "__AcceptDisposable" ->
    [
      "Allows passing values that implement `IDisposable` or `IAsyncDisposable`."
      ^ " Normally these values cannot be passed to functions."
      ^ "\n\nYou cannot save references to `__AcceptDisposable` parameters, to ensure they are disposed at the end of their using block.";
    ]
  | "__AllowStatic" ->
    [
      "Allows this instance method to be called statically on the class."
      ^ "\n\nThis purely for PHP compatibility.";
    ]
  | "__AtMostRxAsArgs" ->
    [
      "Marks a reactive function as being conditionally reactive, depending on the type of its arguments."
      ^ "\n\nThe function is 'at most as reactive as its arguments'.."
      ^ " For example, a `__RxLocal` argument makes this function `__RxLocal`"
      ^ "\n\nThis attribute must be used with `__OnlyRxIfImpl` or `__AtMostRxAsFunc` parameters.";
    ]
  | "__AtMostRxAsFunc" ->
    [
      "Marks a reactive function as being conditionally reactive, depending on the type of this function argument."
      ^ "\n\nThe enclosing function is 'at most as reactive as this function'."
      ^ " For example, a `__RxLocal` function argument makes the enclosing function `__RxLocal`";
    ]
  | "__ALWAYS_INLINE" ->
    [
      "Instructs HHVM to always inline this function."
      ^ " Only used for testing HHVM."
      ^ "\n\nSee also `__NEVER_INLINE`.";
    ]
  | "__ConsistentConstruct" ->
    [
      "Requires all child classes to have the same constructor signature. "
      ^ " This allows `new static(...)` and `new $the_class_name(...)`.";
    ]
  | "__Const" ->
    [
      "Marks a class or property as immutable."
      ^ " When applied to a class, all the properties are considered `__Const`."
      ^ " `__Const` properties can only be set in the constructor.";
    ]
  | "__Deprecated" ->
    [
      "Mark a function/method as deprecated. "
      ^ " The type checker will show an error at call sites, and a runtime warning is logged if this function/method is called."
      ^ "\n\nThe optional second argument specifies a rate limit for warning logs."
      ^ " If the rate limit is 100, a warning is only issued every 1/100 calls.";
    ]
  | "__DynamicallyCallable" ->
    [
      "Allows this function/method to be called dynamically, based on a string of its name. "
      ^ " HHVM will warn or error (depending on settings) on dynamic calls to functions without this attribute."
      ^ "\n\nSee also `HH\\dynamic_fun()` and ``HH\\dynamic_fun()`.";
    ]
  | "__DynamicallyConstructible" ->
    [
      "Allows this class to be instantiated dynamically, based on a string of its name."
      ^ " HHVM will warn or error (depending on settings) on dynamic instantiations without this attribute.";
    ]
  | "__Enforceable" ->
    [
      "Ensures that this type is enforceable."
      ^ " Enforceable types can be used with `is` and `as`."
      ^ " This forbids usage of function types and erased (not reified) generics.";
    ]
  | "__EntryPoint" ->
    [
      "Execution of the program will start here."
      ^ " This only applies in the first file executed, `__EntryPoint` in required or autoloaded files has no effect.";
    ]
  | "__Explicit" ->
    [
      "Requires callers to explicitly specify this type."
      ^ "\n\nNormally Hack allows generics to be inferred at the call site.";
    ]
  | "__HasReifiedParent" ->
    [
      "Marks a class as extending a class that uses reified generics."
      ^ " This is an internal attribute used for byte compilation, and is banned in user code.";
    ]
  | "__HipHopSpecific" ->
    [
      "Marks a class or function as specific to HHVM, so it is shown on http://docs.hhvm.com.";
    ]
  | "__IsFoldable" ->
    [
      "Marks this function can be constant-folded if all arguments are constants."
      ^ " Used by hhbbc.";
    ]
  | "__LateInit" ->
    [
      "Marks a property as late initialized."
      ^ " Normally properties are required to be initialized in the constructor.";
    ]
  | "__LSB" ->
    [
      "Marks this property as implicitly redeclared on all subclasses."
      ^ " This ensures each subclass has its own value for the property.";
    ]
  | "__MockClass" ->
    [
      "Allows subclasses of final classes and overriding of final methods."
      ^ " This is useful for writing mock classes."
      ^ "\n\nYou cannot use this to subclass `vec`, `keyset`, `dict`, `Vector`, `Map` or `Set`.";
    ]
  | "__Memoize" ->
    [
      "Cache the return values from this function/method."
      ^ " Calls with the same arguments will return the cached value."
      ^ "\n\nCaching is per-request and shared between subclasses (see also `__MemoizeLSB`).";
    ]
  | "__MemoizeLSB" ->
    [
      "Cache the return values from this method."
      ^ " Calls with the same arguments will return the cached value."
      ^ "\n\nCaching is per-request and has Late Static Binding, so subclasses do not share the cache.";
    ]
  | "__MaybeMutable" ->
    [
      "Allows a reactive function/method to accept both mutable and immutable values."
      ^ " This combines the restrictions of immutable values (no modification) with `__Mutable` (no aliases)."
      ^ "\n\nThis applies to the parameter specified, or `$this` when `__MaybeMutable` is used on a method.";
    ]
  | "__Mutable" ->
    [
      "Allows mutation of this parameter inside this reactive function/method. "
      ^ " When `__Mutable` is used on a method, allows mutation of `$this`."
      ^ "\n\nAnnotated values are marked as borrowed mutable, unliked `__OwnedMutable`.";
    ]
  | "__MutableReturn" ->
    [
      "Marks this reactive function/method as returning a mutable value that is owned by the caller.";
    ]
  | "__Native" ->
    [
      "Declares a native function."
      ^ " This declares the signature, the implementation will be in an HHVM extension (usually C++).";
    ]
  | "__NativeData" ->
    [
      "Associates this class with a native data type (usually a C++ class)."
      ^ " When instantiating this class, the corresponding native object will also be allocated.";
    ]
  | "__Newable" ->
    [
      "Ensures the class can be constructed."
      ^ "\n\nThis forbids abstract classes, and ensures that the constructor has a consistent signature."
      ^ " Classes must use `__ConsistentConstruct` or be final.";
    ]
  | "__NonRx" ->
    [
      "Mark this function as intentionally not reactive, so readers do not attempt to refactor it to being reactive."
      ^ " When used on methods and global functions, a reason argument is required.";
    ]
  | "__NoFlatten" ->
    [
      "Instructs hhbbc to never inline this trait into classes that use it."
      ^ " Used for testing hhbbc optimizations.";
    ]
  | "__NEVER_INLINE" ->
    [
      "Instructs HHVM to never inline this function."
      ^ " Only used for testing HHVM."
      ^ "\n\nSee also `__ALWAYS_INLINE`.";
    ]
  | "__OnlyRxIfImpl" ->
    [
      "Marks a reactive function as being conditionally reactive, depending on the type of this argument."
      ^ " If the argument implements the interface or extends the class specified, then the function is reactive.";
    ]
  | "__Override" -> ["Ensures there's a parent method being overridden."]
  | "__PHPStdLib" ->
    [
      "Ignore this built-in function or class, so the type checker errors if code uses it."
      ^ " This only applies to code in .hhi files by default, but can apply everywhere with `deregister_php_stdlib`.";
    ]
  | "__PPL" ->
    [
      "Converts all methods to coroutines, except the constructor."
      ^ " Local method calls are converted suspend calls."
      ^ " The functions `sample`, `factor`, `observe` and `condition` are converted to method calls on an implicit `Infer` object.";
    ]
  | "__ProvenanceSkipFrame" ->
    [
      "Don't track Hack arrays created by this function."
      ^ " This is useful when migrating code from PHP arrays to Hack arrays."
      ^ "\n\nThe HHVM option LogArrayProvenance is necessary to observe array provenance.";
    ]
  | "__Reifiable" ->
    [
      "Requires this type to be reifiable."
      ^ " This bans PHP arrays (varray and darray).";
    ]
  | "__Reified" ->
    [
      "Marks a function as taking reified generics."
      ^ " This is an internal attribute used for byte compilation, and is banned in user code.";
    ]
  | "__ReturnDisposable" ->
    [
      "Allows a function/method to return a value that implements `IDisposable` or `IAsyncDisposable`."
      ^ " The function must return a fresh disposable value by either instantiating a class or "
      ^ " returning a value from another method/function marked `__ReturnDisposable`.";
    ]
  | "__ReturnsVoidToRx" ->
    [
      "Requires that reactive functions do not use the return value of this function/method."
      ^ " This enables classes to provide a fluent builder API `->setFoo()->setBar()` but "
      ^ " ensures that reactive functions do `$x->setFoo(); $x->setBar();`.";
    ]
  | "__Rx" ->
    [
      "Ensures this function/method is fully reactive (pure)."
      ^ " It cannot access global state, cannot read from unmanaged sources, and cannot have side effects."
      ^ " Function calls are only permitted to other `__Rx` functions."
      ^ "\n\nFully reactive functions can use the reactive runtime, unlike `__RxShallow` and `__RxLocal`.";
    ]
  | "__RxLocal" ->
    [
      "Ensures this function/method is reactive, ignoring function calls."
      ^ " All the restrictions of `__Rx` apply, but calls to non-rx functions are permitted."
      ^ "\n\nThis allows gradual migration of code to being fully reactive, and permits calls from `__RxShallow`.";
    ]
  | "__RxShallow" ->
    [
      "Ensures this function/method is reactive, but allows calls to `__RxShallow` and `__RxLocal` functions."
      ^ " All the restrictions of `__Rx` apply otherwise."
      ^ "\n\nThis allows gradual migration of code to being fully reactive.";
    ]
  | "__Sealed" ->
    [
      "Only the named classes can extend this class or interface."
      ^ " Child classes may still be extended unless they are marked `final`.";
    ]
  | "__Soft" ->
    [
      "A runtime type mismatch on this parameter/property will not throw a TypeError/Error."
      ^ " This is useful for migrating partial code where you're unsure about the type."
      ^ "\n\nThe type checker will ignore this attribute, so your code will still get type checked."
      ^ " If the type is wrong at runtime, a warning will be logged and code execution will continue.";
    ]
  | "__VMSwitchMode" ->
    [
      "Used by HHVM to decide how it handles catch data when switching to the debugger.";
    ]
  | "__Warn" ->
    [
      "Ensures that incorrect reified types are a warning rather than error."
      ^ "\n\nThis is intended to help gradually migrate code to reified types.";
    ]
  | _ -> []

let make_hover_info env_and_ty file (occurrence, def_opt) =
  SymbolOccurrence.(
    Typing_defs.(
      let snippet =
        match (occurrence, env_and_ty) with
        | ({ name; _ }, None) -> Utils.strip_ns name
        | ({ type_ = Method (classname, name); _ }, Some (env, ty))
          when name = Naming_special_names.Members.__construct ->
          let snippet_opt =
            Option.Monad_infix.(
              Decl_provider.get_class classname
              >>= fun c ->
              fst (Decl_provider.Class.construct c)
              >>| fun elt ->
              let ty = Lazy.force_val elt.ce_type in
              Tast_env.print_ty_with_identity
                env
                (DeclTy ty)
                occurrence
                def_opt)
          in
          begin
            match snippet_opt with
            | Some s -> s
            | None ->
              Tast_env.print_ty_with_identity
                env
                (LoclTy ty)
                occurrence
                def_opt
          end
        | (occurrence, Some (env, ty)) ->
          Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
      in
      let addendum =
        match occurrence with
        | { name; type_ = Attribute; _ } -> make_hover_attr_docs name
        | _ ->
          List.concat
            [
              make_hover_doc_block file occurrence def_opt;
              make_hover_return_type env_and_ty occurrence;
              make_hover_full_name env_and_ty occurrence def_opt;
            ]
      in
      HoverService.
        { snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }))

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : HoverService.result =
  let identities = ServerIdentifyFunction.go_ctx ~ctx ~entry ~line ~column in
  let env_and_ty =
    ServerInferType.type_at_pos
      (Provider_utils.compute_tast ~ctx ~entry)
      line
      column
    |> Option.map ~f:(fun (env, ty) -> (env, Tast_expand.expand_ty env ty))
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
    |> filter_class_and_constructor
    |> List.map ~f:(fun (occurrence, def_opt) ->
           let path =
             def_opt
             |> Option.map ~f:(fun def -> def.SymbolDefinition.pos)
             |> Option.map ~f:Pos.filename
             |> Option.value ~default:entry.Provider_context.path
           in
           let file_input = Provider_context.get_file_input ~ctx ~path in
           make_hover_info env_and_ty file_input (occurrence, def_opt))
    |> List.remove_consecutive_duplicates ~equal:( = )

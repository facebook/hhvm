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

(* Hovering doesn't introduce new dependencies *)
[@@@alert "-dependencies"]

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
    Decl_provider.get_typedef ctx qualified_name |> Decl_entry.to_option
    >>= fun decl ->
    decl.Typing_defs.td_docs_url >>| fun url -> docs_url_markdown name url)

(* If [classish_name] (or any of its parents) has a documentation URL,
   return the docs of the closest type. *)
let classish_docs_url ctx classish_name : string option =
  let docs_url name =
    match Decl_provider.get_class ctx name with
    | Decl_entry.Found decl -> Folded_class.get_docs_url decl
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      None
  in

  let qualified_name = "\\" ^ classish_name in
  let ancestors =
    match Decl_provider.get_class ctx qualified_name with
    | Decl_entry.Found decl -> Folded_class.all_ancestor_names decl
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      []
  in
  List.find_map (qualified_name :: ancestors) ~f:(fun ancestor ->
      match docs_url ancestor with
      | Some url -> Some (docs_url_markdown ancestor url)
      | None -> None)

let docs_url ctx def : string option =
  let open SymbolDefinition in
  match def.kind with
  | Classish _ -> classish_docs_url ctx def.name
  | Typedef -> typedef_docs_url ctx def.name
  | Function
  | Member _
  | GlobalConst
  | LocalVar
  | TypeVar
  | Param
  | Module ->
    None

let make_hover_doc_block ctx entry occurrence def_opt =
  match def_opt with
  | Some def when Option.is_none occurrence.SymbolOccurrence.is_declaration ->
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
      let f = Decl_provider.get_fun ctx fun_name |> Decl_entry.to_option in
      Option.map f ~f:(fun fe -> fe.Typing_defs.fe_pos)
    | MethodReceiver { cls_name; _ } ->
      let c = Decl_provider.get_class ctx cls_name |> Decl_entry.to_option in
      Option.map c ~f:Folded_class.pos)

(* Return the name of the [n]th parameter in [params], handling
   variadics and splat parameters correctly.
   For splat parameters, we suffix [i] on the parameter name to indicate
   that it's the i'th element of the splat tuple.
*)
let nth_param_name (params : ('a, 'b) Aast.fun_param list) (n : int) :
    string option =
  let param =
    if n >= List.length params then
      match List.last params with
      | Some param
        when Aast_utils.is_param_variadic param
             || Aast_utils.is_param_splat param ->
        Some param
      | _ -> None
    else
      List.nth params n
  in
  Option.map param ~f:(fun param ->
      if Aast_utils.is_param_variadic param then
        "..." ^ param.Aast.param_name
      else if Aast_utils.is_param_splat param then
        Printf.sprintf
          "%s[%d]"
          param.Aast.param_name
          (n - List.length params + 1)
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

let hh_ignore_info =
  "`HH_IGNORE[N]` silences type checker warning N on the next line. It does
not change runtime behavior.

Note that `HH_IGNORE` applies to all occurrences of the warning on the
next line."

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

(** For strings like "preffix::suffix", return "prefix", otherwise
  return the full string. *)
let split_class_name (full_name : string) : string =
  match String.lsplit2 full_name ~on:':' with
  | Some (class_name, _member) -> class_name
  | None -> full_name

let make_fun_defined_in_section def_opt : string option =
  let open Option.Let_syntax in
  let* def = def_opt in
  let abs_name = "\\" ^ SymbolDefinition.full_name def in
  if SN.PseudoFunctions.is_pseudo_function abs_name then
    None
  else
    match String.rsplit2 (Utils.strip_hh_lib_ns abs_name) ~on:'\\' with
    | Some (namespace, _) when not (String.equal namespace "") ->
      Some (Printf.sprintf "Defined in namespace `%s`" namespace)
    | _ -> None

(** Return the decl type of a class member or function represented by a SymbolDefinition.t.
    If passed a class member, also return the type parameters of that class *)
let get_decl_ty
    ctx
    (def_opt : _ SymbolDefinition.t option)
    (occurrence : SymbolOccurrence.kind) :
    Typing_defs.decl_tparam list option * Typing_defs.decl_ty option =
  Option.value ~default:(None, None)
  @@
  let open Option.Let_syntax in
  let* def = def_opt in
  let { SymbolDefinition.name; kind; modifiers; _ } = def in
  match kind with
  | SymbolDefinition.Member { class_name; member_kind } ->
    let* cls =
      Decl_provider.get_class ctx (Utils.add_ns class_name)
      |> Decl_entry.to_option
    in
    let is_static = SymbolDefinition.is_static modifiers in
    let member_ty =
      match member_kind with
      | SymbolDefinition.Method ->
        let+ member = Folded_class.get_any_method ~is_static cls name in
        member.Typing_defs.ce_type |> Lazy.force
      | SymbolDefinition.Property ->
        let+ property =
          if is_static then
            Folded_class.get_sprop cls name
          else
            Folded_class.get_prop cls name
        in
        property.Typing_defs.ce_type |> Lazy.force
      | SymbolDefinition.ClassConst ->
        let+ class_const = Folded_class.get_const cls name in
        class_const.Typing_defs.cc_type
      | SymbolDefinition.TypeConst -> None
    in
    Some (Some (Folded_class.tparams cls), member_ty)
  | SymbolDefinition.Function ->
    let* (func : Typing_defs.fun_elt) =
      Decl_provider.get_fun ctx (Utils.add_ns name) |> Decl_entry.to_option
    in
    Some (None, Some func.Typing_defs.fe_type)
  | SymbolDefinition.Classish _ ->
    (match occurrence with
    | SymbolOccurrence.Method (_, construct_name)
      when String.equal construct_name Naming_special_names.Members.__construct
      ->
      (* If the constructor is not explicitly defined, then the SymbolDefinition.t
         will be a Classish instead of Method *)
      let* cls =
        Decl_provider.get_class ctx (Utils.add_ns name) |> Decl_entry.to_option
      in
      let ty =
        match Folded_class.construct cls |> fst with
        | Some ce -> ce.Typing_defs.ce_type |> Lazy.force
        | None -> Typing_make_type.default_construct Typing_reason.none
      in
      Some (Some (Folded_class.tparams cls), Some ty)
    | _ -> None)
  | _ -> None

let make_hack_marked_code s = Lsp.MarkedCode ("hack", s)

(** Make the 'instantiation' section of the hover card, something like:

---
Instantiation:
```
  T = int;
  Ta = string;
```
*)
let make_instantiation_section
    env (subst : Derive_type_instantiation.Instantiation.t) :
    Lsp.markedString list option =
  if Derive_type_instantiation.Instantiation.is_empty subst then
    None
  else
    let Derive_type_instantiation.Instantiation.{ this; subst } = subst in
    let header = Lsp.MarkedString "Instantiation:" in
    let section =
      let print_tparam (tparam, ty) =
        Printf.sprintf "  %s = %s;" tparam (Tast_env.print_ty env ty)
      in
      let printed_this =
        Option.map this ~f:(fun ty -> print_tparam ("this", ty))
      in
      let printed_tparams =
        SMap.elements subst |> List.rev_map ~f:print_tparam
      in
      let printed_tparams =
        match printed_this with
        | None -> printed_tparams
        | Some printed_this -> printed_this :: printed_tparams
      in
      String.concat ~sep:"\n" printed_tparams |> make_hack_marked_code
    in
    Some [header; section]

(** Make the "Defined in" section of the hover card, which indicates where a member
    is defined *)
let make_defined_in_section def_opt (tparams : Typing_defs.decl_tparam list) =
  let open Option.Let_syntax in
  let+ def = def_opt in
  let tparams =
    match tparams with
    | [] -> ""
    | _ ->
      "<"
      ^ (String.concat ~sep:", "
        @@ List.map tparams ~f:(fun param -> snd param.Typing_defs.tp_name))
      ^ ">"
  in
  Printf.sprintf
    "Defined in `%s%s`"
    (split_class_name @@ SymbolDefinition.full_name def)
    tparams

(** Return the hover card section for the uninstantiated signature and the
    hover card section showing the instantiation.
    So when hovering on a method call for example, shows something like

---
```
public function m<T, Tr>(T $x): Tr;
```
---
Instantiation:
```
  T = int;
  Tr = string;
```
*)
let show_type_with_instantiation
    (occurrence : _ SymbolOccurrence.t)
    (def_opt : _ SymbolDefinition.t option)
    decl_ty
    (type_info : ServerInferType.t) : string * Lsp.markedString list option =
  let env = ServerInferType.get_env type_info in
  let locl_ty = ServerInferType.get_type type_info in
  let snippet =
    Tast_env.print_decl_ty_with_identity env decl_ty occurrence def_opt
  in
  let (env, instantiation) =
    Tast_env.derive_instantiation env decl_ty locl_ty
  in
  let instantiation_section = make_instantiation_section env instantiation in
  (snippet, instantiation_section)

let make_hover_info
    under_dynamic_result
    ctx
    (info_opt : ServerInferType.t option)
    entry
    (occurrence : _ SymbolOccurrence.t)
    def_opt : hover_info =
  let print_locl_ty_with_identity ?(do_not_strip_dynamic = false) info =
    let env = ServerInferType.get_env info in
    let ty = ServerInferType.get_type info in
    let ty =
      if do_not_strip_dynamic then
        ty
      else
        Tast_env.strip_dynamic env ty
    in
    Tast_env.print_ty_with_identity env ty occurrence def_opt
  in
  let { SymbolOccurrence.name; type_; is_declaration = _; pos = _ } =
    occurrence
  in
  let (defined_in, snippet, instantiation_section) =
    match (type_, info_opt) with
    | (_, None) -> (None, Utils.strip_hh_lib_ns name, None)
    | (SymbolOccurrence.BestEffortArgument (recv, i), _) ->
      let param_name = nth_param ctx recv i in
      ( None,
        Printf.sprintf "Parameter: %s" (Option.value ~default:"$_" param_name),
        None )
    | (SymbolOccurrence.Method _, Some info)
    | (SymbolOccurrence.ClassConst _, Some info)
    | (SymbolOccurrence.Property _, Some info) ->
      let ((snippet, instantiation_section), class_tparams) =
        match get_decl_ty ctx def_opt type_ with
        | (Some class_tparams, Some decl_ty) ->
          ( show_type_with_instantiation occurrence def_opt decl_ty info,
            class_tparams )
        | _ -> ((print_locl_ty_with_identity info, None), [])
      in
      ( make_defined_in_section def_opt class_tparams,
        snippet,
        instantiation_section )
    | (SymbolOccurrence.GConst, Some info) ->
      ( None,
        (match make_hover_const_definition entry def_opt with
        | Some def_txt -> def_txt
        | None -> print_locl_ty_with_identity info),
        None )
    | (SymbolOccurrence.Function, Some info) ->
      let (snippet, instantiation_section) =
        match get_decl_ty ctx def_opt type_ with
        | (_, Some decl_ty) ->
          show_type_with_instantiation occurrence def_opt decl_ty info
        | _ ->
          (print_locl_ty_with_identity ~do_not_strip_dynamic:true info, None)
      in
      (make_fun_defined_in_section def_opt, snippet, instantiation_section)
    | ( SymbolOccurrence.(
          ( Class _ | Module | Typeconst _ | Attribute _ | EnumClassLabel _
          | Keyword _ | BuiltInType _ | LocalVar | TypeVar | XhpLiteralAttr _
          | HhFixme | HhIgnore | PureFunctionContext )),
        Some info ) ->
      ( None,
        print_locl_ty_with_identity ~do_not_strip_dynamic:true info
        ^ under_dynamic_result,
        None )
  in
  let addendum =
    match type_ with
    | SymbolOccurrence.Attribute _ ->
      List.concat
        [
          make_hover_attr_docs name;
          make_hover_doc_block ctx entry occurrence def_opt;
        ]
    | SymbolOccurrence.Keyword info -> [keyword_info info]
    | SymbolOccurrence.HhFixme -> [hh_fixme_info]
    | SymbolOccurrence.HhIgnore -> [hh_ignore_info]
    | SymbolOccurrence.PureFunctionContext -> [pure_context_info]
    | SymbolOccurrence.BuiltInType bt ->
      [SymbolOccurrence.built_in_type_hover bt]
    | _ -> make_hover_doc_block ctx entry occurrence def_opt
  in
  let addendum = List.map addendum ~f:(fun s -> Lsp.MarkedString s) in
  let main_section =
    match instantiation_section with
    | None -> []
    | Some instantiation_section -> instantiation_section :: []
  in
  let main_section = [make_hack_marked_code snippet] :: main_section in
  let main_section =
    match defined_in with
    | None -> main_section
    | Some defined_in -> [Lsp.MarkedString defined_in] :: main_section
  in
  let snippet =
    List.intersperse main_section ~sep:[Lsp.MarkedString "---"] |> List.concat
  in
  HoverService.{ snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }

let make_hover_info_with_fallback under_dynamic_result results =
  let class_fallback =
    List.hd
      (List.filter results ~f:(fun (_, _, _, occurrence, _) ->
           SymbolOccurrence.is_class occurrence))
  in
  List.map results ~f:(fun (ctx, env_and_ty, entry, occurrence, def_opt) ->
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
            |> List.map ~f:(fun s -> Lsp.MarkedString s)
          in
          ( occurrence,
            HoverService.
              {
                snippet = hover_info.snippet;
                addendum = fallback_doc_block @ hover_info.addendum;
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

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    (pos : File_content.Position.t) : HoverService.result =
  let identities : (_ SymbolOccurrence.t * _ SymbolDefinition.t option) list =
    ServerIdentifyFunction.go_quarantined ~ctx ~entry pos
  in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let info_opt : ServerInferType.t option =
    ServerInferType.human_friendly_type_at_pos ~under_dynamic:false ctx tast pos
  in
  let info_dynamic_opt : ServerInferType.t option =
    ServerInferType.human_friendly_type_at_pos ~under_dynamic:true ctx tast pos
  in
  let under_dynamic_result : string =
    match info_dynamic_opt with
    | Some info_dynamic ->
      let ty_dynamic = ServerInferType.get_type info_dynamic in
      (match info_opt with
      | None -> ""
      | Some info ->
        let ty = ServerInferType.get_type info in
        let env = ServerInferType.get_env info_dynamic in
        (* If under dynamic the type is no worse then don't
         * bother presenting it. Example: ~int in static mode, dynamic under dynamic mode.
         *)
        if Tast_env.is_sub_type env ty_dynamic ty then
          ""
        else
          Printf.sprintf
            " (%s when called dynamically)"
            (Tast_env.print_ty env ty_dynamic))
    | None -> ""
  in
  match (identities, info_opt) with
  | ([], Some info) ->
    let ty = ServerInferType.get_type info in
    let env = ServerInferType.get_env info in
    (* There are no identities (named entities) at the cursor, but we
       know the type of the expression. Just show the type.

       This can occur if the user hovers over a literal such as `123`. *)
    let addendum =
      match Typing_defs.get_node ty with
      | Typing_defs.Tgeneric name when String.equal name SN.Typehints.this ->
        let upper_bounds =
          Tast_env.get_upper_bounds env SN.Typehints.this
          |> Typing_set.to_list
          |> List.map ~f:(Tast_env.print_ty env)
          |> String.concat ~sep:", "
        in
        [
          Lsp.MarkedString
            ("this has the following upper bounds: " ^ upper_bounds);
        ]
      | _ -> []
    in
    [
      {
        snippet =
          [
            make_hack_marked_code
              (Tast_env.print_ty env ty ^ under_dynamic_result);
          ];
        addendum;
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
      match info_opt with
      | Some info ->
        let ty = ServerInferType.get_type info in
        [
          {
            snippet =
              [
                make_hack_marked_code
                  (Tast_env.print_ty (ServerInferType.get_env info) ty
                  ^ under_dynamic_result);
              ];
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
            snippet =
              [
                make_hack_marked_code (Printf.sprintf "Parameter: %s" param_name);
              ];
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
           let info_opt =
             match occurrence.SymbolOccurrence.type_ with
             | SymbolOccurrence.TypeVar -> None
             | SymbolOccurrence.BuiltInType _ -> None
             | _ -> info_opt
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
           (ctx, info_opt, entry, occurrence, def_opt))
    |> make_hover_info_with_fallback under_dynamic_result
    |> filter_class_and_constructor
    |> List.remove_consecutive_duplicates ~equal:equal_hover_info

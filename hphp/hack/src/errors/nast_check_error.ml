(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Error_code = Error_codes.NastCheck

type t =
  | Repeated_record_field_name of {
      pos: Pos.t;
      name: string;
      prev_pos: Pos_or_decl.t;
    }
  | Dynamically_callable_reified of Pos.t
  | No_construct_parent of Pos.t
  | Nonstatic_method_in_abstract_final_class of Pos.t
  | Constructor_required of {
      pos: Pos.t;
      class_name: string;
      prop_names: string list;
    }
  | Not_initialized of {
      pos: Pos.t;
      class_name: string;
      props: (Pos_or_decl.t * string) list;
    }
  | Call_before_init of {
      pos: Pos.t;
      prop_name: string;
    }
  | Abstract_with_body of Pos.t
  | Not_abstract_without_typeconst of Pos.t
  | Typeconst_depends_on_external_tparam of {
      pos: Pos.t;
      ext_pos: Pos.t;
      ext_name: string;
    }
  | Interface_with_partial_typeconst of Pos.t
  | Partially_abstract_typeconst_definition of Pos.t
  | Refinement_in_typestruct of {
      pos: Pos.t;
      kind: string;
    }
  | Multiple_xhp_category of Pos.t
  | Return_in_gen of Pos.t
  | Return_in_finally of Pos.t
  | Toplevel_break of Pos.t
  | Toplevel_continue of Pos.t
  | Continue_in_switch of Pos.t
  | Await_in_sync_function of {
      pos: Pos.t;
      func_pos: Pos.t option;
    }
  | Interface_uses_trait of Pos.t
  | Static_memoized_function of Pos.t
  | Magic of {
      pos: Pos.t;
      meth_name: string;
    }
  | Non_interface of {
      pos: Pos.t;
      name: string;
      verb: [ `req_implement | `implement ];
    }
  | ToString_returns_string of Pos.t
  | ToString_visibility of Pos.t
  | Uses_non_trait of {
      pos: Pos.t;
      name: string;
      kind: string;
    }
  | Requires_non_class of {
      pos: Pos.t;
      name: string;
      kind: string;
    }
  | Requires_final_class of {
      pos: Pos.t;
      name: string;
    }
  | Abstract_body of Pos.t
  | Interface_with_member_variable of Pos.t
  | Interface_with_static_member_variable of Pos.t
  | Illegal_function_name of {
      pos: Pos.t;
      name: string;
    }
  | Entrypoint_arguments of Pos.t
  | Entrypoint_generics of Pos.t
  | Variadic_memoize of Pos.t
  | Abstract_method_memoize of Pos.t
  | Instance_property_in_abstract_final_class of Pos.t
  | Inout_params_special of Pos.t
  | Inout_params_memoize of {
      pos: Pos.t;
      param_pos: Pos.t;
    }
  | Inout_in_transformed_pseudofunction of {
      pos: Pos.t;
      fn_name: string;
    }
  | Reading_from_append of Pos.t
  | List_rvalue of Pos.t
  | Illegal_destructor of Pos.t
  | Illegal_context of {
      pos: Pos.t;
      name: string;
    }
  | Case_fallthrough of {
      switch_pos: Pos.t;
      case_pos: Pos.t;
      next_pos: Pos.t option;
    }
  | Default_fallthrough of Pos.t
  | Php_lambda_disallowed of Pos.t
  | Internal_method_with_invalid_visibility of {
      pos: Pos.t;
      vis: Ast_defs.visibility;
    }
  | Private_and_final of Pos.t
  | Internal_member_inside_public_trait of {
      member_pos: Pos.t;
      trait_pos: Pos.t;
    }
  | Attribute_conflicting_memoize of {
      pos: Pos.t;
      second_pos: Pos.t;
    }
  | Soft_internal_without_internal of Pos.t

let repeated_record_field_name pos name prev_pos =
  User_error.make
    Error_code.(to_enum RepeatedRecordFieldName)
    ( pos,
      Printf.sprintf "Duplicate record field %s" (Markdown_lite.md_codify name)
    )
    [(prev_pos, "Previous field is here")]

let dynamically_callable_reified attr_pos =
  User_error.make
    Error_code.(to_enum DynamicallyCallableReified)
    ( attr_pos,
      "`__DynamicallyCallable` cannot be used on reified functions or methods"
    )
    []

let no_construct_parent pos =
  User_error.make
    Error_code.(to_enum NoConstructParent)
    ( pos,
      Utils.sl
        [
          "You are extending a class that needs to be initialized\n";
          "Make sure you call `parent::__construct`.\n";
        ] )
    []

let nonstatic_method_in_abstract_final_class pos =
  User_error.make
    Error_code.(to_enum NonstaticMethodInAbstractFinalClass)
    ( pos,
      "Abstract final classes cannot have nonstatic methods or constructors." )
    []

let constructor_required pos name prop_names =
  let name = Render.strip_ns name in
  let props_str =
    List.map ~f:Markdown_lite.md_codify prop_names |> String.concat ~sep:" "
  in
  User_error.make
    Error_code.(to_enum ConstructorRequired)
    ( pos,
      "Lacking `__construct`, class "
      ^ Markdown_lite.md_codify name
      ^ " does not initialize its private member(s): "
      ^ props_str )
    []

let not_initialized pos cname props =
  let cname = Render.strip_ns cname in
  let prop_msgs =
    List.map props ~f:(fun (pos, prop) ->
        ( pos,
          Markdown_lite.md_codify ("$this->" ^ prop) ^ " is not initialized." ))
  in
  User_error.make
    Error_code.(to_enum NotInitialized)
    ( pos,
      "Class "
      ^ Markdown_lite.md_codify cname
      ^ " has properties that cannot be null and aren't always set in `__construct`."
    )
    prop_msgs

let call_before_init pos cv =
  User_error.make
    Error_code.(to_enum CallBeforeInit)
    ( pos,
      Utils.sl
        ([
           "Until the initialization of `$this` is over,";
           " you can only call private methods\n";
           "The initialization is not over because ";
         ]
        @
        if String.equal cv "parent::__construct" then
          ["you forgot to call `parent::__construct`"]
        else
          [
            Markdown_lite.md_codify ("$this->" ^ cv);
            " can still potentially be null";
          ]) )
    []

let abstract_with_body pos =
  User_error.make
    Error_code.(to_enum AbstractWithBody)
    (pos, "This method is declared as abstract, but has a body")
    []

let not_abstract_without_typeconst pos =
  User_error.make
    Error_code.(to_enum NotAbstractWithoutTypeconst)
    ( pos,
      "This type constant is not declared as abstract, it must have"
      ^ " an assigned type" )
    []

let typeconst_depends_on_external_tparam pos ext_pos ext_name =
  let claim =
    ( pos,
      "A type constant can only use type parameters declared in its own"
      ^ " type parameter list" )
  and reasons =
    [
      ( Pos_or_decl.of_raw_pos ext_pos,
        Format.sprintf
          "%s was declared as a type parameter here"
          (Markdown_lite.md_codify ext_name) );
    ]
  in
  User_error.make
    Error_code.(to_enum TypeconstDependsOnExternalTparam)
    claim
    reasons

let interface_with_partial_typeconst pos =
  User_error.make
    Error_code.(to_enum InterfaceWithPartialTypeconst)
    (pos, "An interface cannot contain a partially abstract type constant")
    []

let partially_abstract_typeconst_definition pos =
  User_error.make
    Error_code.(to_enum PartiallyAbstractTypeconstDefinition)
    (pos, "`as` constraints are only legal on abstract type constants")
    []

let refinement_in_typestruct kind pos =
  User_error.make
    Error_code.(to_enum RefinementInTypeStruct)
    ( pos,
      "Type refinements cannot appear on the right-hand side of " ^ kind ^ "."
    )
    []

let multiple_xhp_category pos =
  User_error.make
    Error_code.(to_enum MultipleXhpCategory)
    (pos, "XHP classes can only contain one category declaration")
    []

let return_in_gen pos =
  User_error.make
    Error_code.(to_enum ReturnInGen)
    ( pos,
      "You cannot return a value in a generator (a generator"
      ^ " is a function that uses `yield`)" )
    []

let return_in_finally pos =
  User_error.make
    Error_code.(to_enum ReturnInFinally)
    ( pos,
      "Don't use `return` in a `finally` block;"
      ^ " there's nothing to receive the return value" )
    []

let toplevel_break pos =
  User_error.make
    Error_code.(to_enum ToplevelBreak)
    (pos, "`break` can only be used inside loops or `switch` statements")
    []

let toplevel_continue pos =
  User_error.make
    Error_code.(to_enum ToplevelContinue)
    (pos, "`continue` can only be used inside loops")
    []

let continue_in_switch pos =
  User_error.make
    Error_code.(to_enum ContinueInSwitch)
    ( pos,
      "In PHP, `continue;` inside a switch statement is equivalent to `break;`."
      ^ " Hack does not support this; use `break` if that is what you meant." )
    []

let await_in_sync_function pos func_pos =
  let quickfixes =
    match func_pos with
    | None -> Some []
    | Some fix_pos ->
      let (_, start_col) = Pos.line_column fix_pos in
      let fix_pos = Pos.set_col_end (start_col - 9) fix_pos in
      let fix_pos = Pos.set_col_start (start_col - 9) fix_pos in
      Some
        [
          Quickfix.make
            ~title:("Make function " ^ Markdown_lite.md_codify "async")
            ~new_text:"async "
            fix_pos;
        ]
  in
  User_error.make
    Error_code.(to_enum AwaitInSyncFunction)
    ?quickfixes
    (pos, "`await` can only be used inside `async` functions")
    []

let interface_uses_trait pos =
  User_error.make
    Error_code.(to_enum InterfaceUsesTrait)
    (pos, "Interfaces cannot use traits")
    []

let static_memoized_function pos =
  User_error.make
    Error_code.(to_enum StaticMemoizedFunction)
    ( pos,
      "`memoize` is not allowed on static methods in classes that aren't final "
    )
    []

let magic pos meth_name =
  User_error.make
    Error_code.(to_enum Magic)
    ( pos,
      Format.sprintf "%s is a magic method and cannot be called directly"
      @@ Markdown_lite.md_codify meth_name )
    []

let toString_returns_string pos =
  User_error.make
    Error_code.(to_enum ToStringReturnsString)
    (pos, "`__toString` should return a string")
    []

let toString_visibility pos =
  User_error.make
    Error_code.(to_enum ToStringVisibility)
    (pos, "`__toString` must have public visibility and cannot be static")
    []

let abstract_body pos =
  User_error.make
    Error_code.(to_enum AbstractBody)
    (pos, "This method shouldn't have a body")
    []

let interface_with_member_variable pos =
  User_error.make
    Error_code.(to_enum InterfaceWithMemberVariable)
    (pos, "Interfaces cannot have member variables")
    []

let interface_with_static_member_variable pos =
  User_error.make
    Error_code.(to_enum InterfaceWithStaticMemberVariable)
    (pos, "Interfaces cannot have static variables")
    []

let illegal_function_name pos name =
  User_error.make
    Error_code.(to_enum IllegalFunctionName)
    ( pos,
      Format.sprintf
        "Illegal function name: %s"
        (Render.strip_ns name |> Markdown_lite.md_codify) )
    []

let entrypoint_arguments pos =
  User_error.make
    Error_code.(to_enum EntryPointArguments)
    (pos, "`__EntryPoint` functions cannot take arguments.")
    []

let entrypoint_generics pos =
  User_error.make
    Error_code.(to_enum EntryPointGenerics)
    (pos, "`__EntryPoint` functions cannot have generic parameters.")
    []

let variadic_memoize pos =
  User_error.make
    Error_code.(to_enum VariadicMemoize)
    (pos, "Memoized functions cannot be variadic.")
    []

let abstract_method_memoize pos =
  User_error.make
    Error_code.(to_enum AbstractMethodMemoize)
    (pos, "Abstract methods cannot be memoized.")
    []

let instance_property_in_abstract_final_class pos =
  User_error.make
    Error_code.(to_enum InstancePropertyInAbstractFinalClass)
    (pos, "Abstract final classes cannot have instance properties.")
    []

let inout_params_special pos =
  User_error.make
    Error_code.(to_enum InoutParamsSpecial)
    (pos, "Methods with special semantics cannot have `inout` parameters.")
    []

let inout_params_memoize pos param_pos =
  User_error.make
    Error_code.(to_enum InoutParamsMemoize)
    (pos, "Functions with `inout` parameters cannot be memoized")
    [(Pos_or_decl.of_raw_pos param_pos, "This is an `inout` parameter")]

let inout_in_transformed_pseudofunction pos fn_name =
  User_error.make
    Error_code.(to_enum InoutInTransformedPsuedofunction)
    (pos, Printf.sprintf "Unexpected `inout` argument for `%s`" fn_name)
    []

let reading_from_append pos =
  User_error.make
    Error_code.(to_enum ReadingFromAppend)
    (pos, "Cannot use `[]` for reading")
    []

let list_rvalue pos =
  User_error.make
    Error_code.(to_enum ListRvalue)
    ( pos,
      "`list()` can only be used for destructuring assignment. Did you mean `tuple()` or `vec[]`?"
    )
    []

let illegal_destructor pos =
  User_error.make
    Error_code.(to_enum IllegalDestructor)
    ( pos,
      "Destructors are not supported in Hack; use other patterns like "
      ^ "`IDisposable`/`using` or `try`/`catch` instead." )
    []

let illegal_context pos name =
  User_error.make
    Error_code.(to_enum IllegalContext)
    ( pos,
      Format.sprintf
        "Illegal context: %s\nCannot use a context defined outside namespace %s"
        (Markdown_lite.md_codify name)
        Naming_special_names.Coeffects.contexts )
    []

let case_fallthrough switch_pos case_pos next_pos =
  let quickfixes =
    match next_pos with
    | None -> []
    | Some next_pos ->
      let (_, start_col) = Pos.line_column next_pos in
      let offset = String.length "case " in
      let new_pos =
        Pos.set_col_end (start_col - offset)
        @@ Pos.set_col_start (start_col - offset) next_pos
      in
      let new_text =
        "  // FALLTHROUGH\n" ^ String.make (start_col - offset) ' '
      in

      [
        Quickfix.make
          ~title:"Mark this `case` as explicitly falling through"
          ~new_text
          new_pos;
      ]
  in

  let claim =
    ( switch_pos,
      "This `switch` has a `case` that implicitly falls through and is "
      ^ "not annotated with `// FALLTHROUGH`" )
  and reasons =
    [
      ( Pos_or_decl.of_raw_pos case_pos,
        "This `case` implicitly falls through. Did you forget to add `break` or `return`?"
      );
    ]
  in
  User_error.make ~quickfixes Error_code.(to_enum CaseFallthrough) claim reasons

let default_fallthrough pos =
  User_error.make
    Error_code.(to_enum DefaultFallthrough)
    ( pos,
      "This `switch` has a default case that implicitly falls "
      ^ "through and is not annotated with `// FALLTHROUGH`" )
    []

let php_lambda_disallowed pos =
  User_error.make
    Error_code.(to_enum PhpLambdaDisallowed)
    (pos, "PHP style anonymous functions are not allowed.")
    []

let non_interface pos name verb =
  let verb_str =
    match verb with
    | `implement -> "implement"
    | `req_implement -> "require implementation of"
  in
  User_error.make
    Error_code.(to_enum NonInterface)
    ( pos,
      Format.sprintf
        "Cannot %s %s - it is not an interface"
        verb_str
        (Markdown_lite.md_codify @@ Render.strip_ns name) )
    []

let uses_non_trait pos name kind =
  User_error.make
    Error_code.(to_enum UsesNonTrait)
    ( pos,
      Format.sprintf
        "%s is not a trait. It is %s."
        (Markdown_lite.md_codify @@ Render.strip_ns name)
        kind )
    []

let requires_non_class pos name kind =
  User_error.make
    Error_code.(to_enum RequiresNonClass)
    ( pos,
      Format.sprintf
        "%s is not a class. It is %s."
        (Markdown_lite.md_codify @@ Render.strip_ns name)
        kind )
    []

let requires_final_class pos name =
  User_error.make
    Error_code.(to_enum RequiresFinalClass)
    ( pos,
      Format.sprintf
        "%s is a `final` class, so it cannot be extended."
        (Markdown_lite.md_codify @@ Render.strip_ns name) )
    []

let internal_method_with_invalid_visibility pos vis =
  let vis_str =
    String.lowercase @@ Markdown_lite.md_codify @@ Ast_defs.show_visibility vis
  in
  let msg =
    Format.sprintf
      "`__Internal` methods must be public, they cannot be %s"
      vis_str
  in
  User_error.make Error_code.(to_enum InternalProtectedOrPrivate) (pos, msg) []

let private_and_final pos =
  User_error.make
    Error_code.(to_enum PrivateAndFinal)
    (pos, "Class methods cannot be both `private` and `final`.")
    []

let soft_internal_without_internal pos =
  User_error.make
    Error_code.(to_enum Soft_internal_without_internal)
    ( pos,
      "<<__SoftInternal>> can only be used on internal symbols. Try adding internal to this symbol."
    )
    []

let internal_member_inside_public_trait member_pos trait_pos =
  User_error.make
    Error_code.(to_enum InternalMemberInsidePublicTrait)
    (member_pos, "You cannot make this trait member `internal`")
    [
      ( Pos_or_decl.of_raw_pos trait_pos,
        "Only `internal` traits can have `internal` members" );
    ]

let attribute_conflicting_memoize pos second_pos =
  User_error.make
    Error_code.(to_enum AttributeConflictingMemoize)
    ( pos,
      Printf.sprintf
        "Methods cannot be both %s and %s."
        (Markdown_lite.md_codify Naming_special_names.UserAttributes.uaMemoize)
        (Markdown_lite.md_codify
           Naming_special_names.UserAttributes.uaMemoizeLSB) )
    [
      ( Pos_or_decl.of_raw_pos second_pos,
        "Conflicting memoize attribute is here" );
    ]

(* --------------------------------------------- *)
let to_user_error = function
  | Repeated_record_field_name { pos; name; prev_pos } ->
    repeated_record_field_name pos name prev_pos
  | Dynamically_callable_reified pos -> dynamically_callable_reified pos
  | No_construct_parent pos -> no_construct_parent pos
  | Nonstatic_method_in_abstract_final_class pos ->
    nonstatic_method_in_abstract_final_class pos
  | Constructor_required { pos; class_name; prop_names } ->
    constructor_required pos class_name prop_names
  | Not_initialized { pos; class_name; props } ->
    not_initialized pos class_name props
  | Call_before_init { pos; prop_name } -> call_before_init pos prop_name
  | Abstract_with_body pos -> abstract_with_body pos
  | Not_abstract_without_typeconst pos -> not_abstract_without_typeconst pos
  | Typeconst_depends_on_external_tparam { pos; ext_pos; ext_name } ->
    typeconst_depends_on_external_tparam pos ext_pos ext_name
  | Interface_with_partial_typeconst pos -> interface_with_partial_typeconst pos
  | Partially_abstract_typeconst_definition pos ->
    partially_abstract_typeconst_definition pos
  | Refinement_in_typestruct { pos; kind } -> refinement_in_typestruct kind pos
  | Multiple_xhp_category pos -> multiple_xhp_category pos
  | Return_in_gen pos -> return_in_gen pos
  | Return_in_finally pos -> return_in_finally pos
  | Toplevel_break pos -> toplevel_break pos
  | Toplevel_continue pos -> toplevel_continue pos
  | Continue_in_switch pos -> continue_in_switch pos
  | Await_in_sync_function { pos; func_pos } ->
    await_in_sync_function pos func_pos
  | Interface_uses_trait pos -> interface_uses_trait pos
  | Static_memoized_function pos -> static_memoized_function pos
  | Magic { pos; meth_name } -> magic pos meth_name
  | ToString_returns_string pos -> toString_returns_string pos
  | ToString_visibility pos -> toString_visibility pos
  | Abstract_body pos -> abstract_body pos
  | Interface_with_member_variable pos -> interface_with_member_variable pos
  | Interface_with_static_member_variable pos ->
    interface_with_static_member_variable pos
  | Illegal_function_name { pos; name } -> illegal_function_name pos name
  | Entrypoint_arguments pos -> entrypoint_arguments pos
  | Entrypoint_generics pos -> entrypoint_generics pos
  | Variadic_memoize pos -> variadic_memoize pos
  | Abstract_method_memoize pos -> abstract_method_memoize pos
  | Instance_property_in_abstract_final_class pos ->
    instance_property_in_abstract_final_class pos
  | Inout_params_special pos -> inout_params_special pos
  | Inout_params_memoize { pos; param_pos } ->
    inout_params_memoize pos param_pos
  | Inout_in_transformed_pseudofunction { pos; fn_name } ->
    inout_in_transformed_pseudofunction pos fn_name
  | Reading_from_append pos -> reading_from_append pos
  | List_rvalue pos -> list_rvalue pos
  | Illegal_destructor pos -> illegal_destructor pos
  | Illegal_context { pos; name } -> illegal_context pos name
  | Case_fallthrough { switch_pos; case_pos; next_pos } ->
    case_fallthrough switch_pos case_pos next_pos
  | Default_fallthrough pos -> default_fallthrough pos
  | Php_lambda_disallowed pos -> php_lambda_disallowed pos
  | Non_interface { pos; name; verb } -> non_interface pos name verb
  | Uses_non_trait { pos; name; kind } -> uses_non_trait pos name kind
  | Requires_non_class { pos; name; kind } -> requires_non_class pos name kind
  | Requires_final_class { pos; name } -> requires_final_class pos name
  | Internal_method_with_invalid_visibility { pos; vis } ->
    internal_method_with_invalid_visibility pos vis
  | Private_and_final pos -> private_and_final pos
  | Internal_member_inside_public_trait { member_pos; trait_pos } ->
    internal_member_inside_public_trait member_pos trait_pos
  | Attribute_conflicting_memoize { pos; second_pos } ->
    attribute_conflicting_memoize pos second_pos
  | Soft_internal_without_internal pos -> soft_internal_without_internal pos

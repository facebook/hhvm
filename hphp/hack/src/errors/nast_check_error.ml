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
  | Multiple_xhp_category of Pos.t
  | Return_in_gen of Pos.t
  | Return_in_finally of Pos.t
  | Toplevel_break of Pos.t
  | Toplevel_continue of Pos.t
  | Continue_in_switch of Pos.t
  | Await_in_sync_function of Pos.t
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
  | Inout_argument_bad_expr of Pos.t
  | Illegal_destructor of Pos.t
  | Switch_non_terminal_default of Pos.t
  | Switch_multiple_default of Pos.t
  | Illegal_context of {
      pos: Pos.t;
      name: string;
    }
  | Case_fallthrough of {
      switch_pos: Pos.t;
      case_pos: Pos.t;
    }
  | Default_fallthrough of Pos.t
  | Php_lambda_disallowed of Pos.t
  | Internal_method_with_invalid_visibility of {
      pos: Pos.t;
      vis: Ast_defs.visibility;
    }
  | Private_and_final of Pos.t

let to_user_error = function
  | Repeated_record_field_name { pos; name; prev_pos } ->
    User_error.make
      Error_code.(to_enum RepeatedRecordFieldName)
      ( pos,
        Printf.sprintf
          "Duplicate record field %s"
          (Markdown_lite.md_codify name) )
      [(prev_pos, "Previous field is here")]
  | Dynamically_callable_reified pos ->
    User_error.make
      Error_code.(to_enum DynamicallyCallableReified)
      ( pos,
        "`__DynamicallyCallable` cannot be used on reified functions or methods"
      )
      []
  | No_construct_parent pos ->
    User_error.make
      Error_code.(to_enum NoConstructParent)
      ( pos,
        Utils.sl
          [
            "You are extending a class that needs to be initialized\n";
            "Make sure you call `parent::__construct`.\n";
          ] )
      []
  | Nonstatic_method_in_abstract_final_class pos ->
    User_error.make
      Error_code.(to_enum NonstaticMethodInAbstractFinalClass)
      ( pos,
        "Abstract final classes cannot have nonstatic methods or constructors."
      )
      []
  | Constructor_required { pos; class_name; prop_names } ->
    let props_str =
      List.map ~f:Markdown_lite.md_codify prop_names |> String.concat ~sep:" "
    in
    User_error.make
      Error_code.(to_enum ConstructorRequired)
      ( pos,
        Format.sprintf
          "Lacking `__construct`, class %s does not initialize its private member(s): %s"
          (Markdown_lite.md_codify @@ Render.strip_ns class_name)
          props_str )
      []
  | Not_initialized { pos; class_name; props } ->
    let claim =
      ( pos,
        Format.sprintf
          "Class %s has properties that cannot be null and aren't always set in `__construct`."
        @@ Markdown_lite.md_codify
        @@ Render.strip_ns class_name )
    and reasons =
      List.map props ~f:(fun (pos, prop) ->
          ( pos,
            Markdown_lite.md_codify ("$this->" ^ prop) ^ " is not initialized."
          ))
    in
    User_error.make Error_code.(to_enum NotInitialized) claim reasons
  | Call_before_init { pos; prop_name } ->
    let claim =
      ( pos,
        Utils.sl
          ([
             "Until the initialization of `$this` is over,";
             " you can only call private methods\n";
             "The initialization is not over because ";
           ]
          @
          if String.equal prop_name "parent::__construct" then
            ["you forgot to call `parent::__construct`"]
          else
            [
              Markdown_lite.md_codify ("$this->" ^ prop_name);
              " can still potentially be null";
            ]) )
    in
    User_error.make Error_code.(to_enum CallBeforeInit) claim []
  | Abstract_with_body pos ->
    User_error.make
      Error_code.(to_enum AbstractWithBody)
      (pos, "This method is declared as abstract, but has a body")
      []
  | Not_abstract_without_typeconst pos ->
    User_error.make
      Error_code.(to_enum NotAbstractWithoutTypeconst)
      ( pos,
        "This type constant is not declared as abstract, it must have"
        ^ " an assigned type" )
      []
  | Typeconst_depends_on_external_tparam { pos; ext_pos; ext_name } ->
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
  | Interface_with_partial_typeconst pos ->
    User_error.make
      Error_code.(to_enum InterfaceWithPartialTypeconst)
      (pos, "An interface cannot contain a partially abstract type constant")
      []
  | Partially_abstract_typeconst_definition pos ->
    User_error.make
      Error_code.(to_enum PartiallyAbstractTypeconstDefinition)
      (pos, "`as` constraints are only legal on abstract type constants")
      []
  | Multiple_xhp_category pos ->
    User_error.make
      Error_code.(to_enum MultipleXhpCategory)
      (pos, "XHP classes can only contain one category declaration")
      []
  | Return_in_gen pos ->
    User_error.make
      Error_code.(to_enum ReturnInGen)
      ( pos,
        "You cannot return a value in a generator (a generator"
        ^ " is a function that uses `yield`)" )
      []
  | Return_in_finally pos ->
    User_error.make
      Error_code.(to_enum ReturnInFinally)
      ( pos,
        "Don't use `return` in a `finally` block;"
        ^ " there's nothing to receive the return value" )
      []
  | Toplevel_break pos ->
    User_error.make
      Error_code.(to_enum ToplevelBreak)
      (pos, "`break` can only be used inside loops or `switch` statements")
      []
  | Toplevel_continue pos ->
    User_error.make
      Error_code.(to_enum ToplevelContinue)
      (pos, "`continue` can only be used inside loops")
      []
  | Continue_in_switch pos ->
    User_error.make
      Error_code.(to_enum ContinueInSwitch)
      ( pos,
        "In PHP, `continue;` inside a switch statement is equivalent to `break;`."
        ^ " Hack does not support this; use `break` if that is what you meant."
      )
      []
  | Await_in_sync_function pos ->
    User_error.make
      Error_code.(to_enum AwaitInSyncFunction)
      (pos, "`await` can only be used inside `async` functions")
      []
  | Interface_uses_trait pos ->
    User_error.make
      Error_code.(to_enum InterfaceUsesTrait)
      (pos, "Interfaces cannot use traits")
      []
  | Static_memoized_function pos ->
    User_error.make
      Error_code.(to_enum StaticMemoizedFunction)
      ( pos,
        "`memoize` is not allowed on static methods in classes that aren't final "
      )
      []
  | Magic { pos; meth_name } ->
    User_error.make
      Error_code.(to_enum Magic)
      ( pos,
        Format.sprintf "%s is a magic method and cannot be called directly"
        @@ Markdown_lite.md_codify meth_name )
      []
  | ToString_returns_string pos ->
    User_error.make
      Error_code.(to_enum ToStringReturnsString)
      (pos, "`__toString` should return a string")
      []
  | ToString_visibility pos ->
    User_error.make
      Error_code.(to_enum ToStringVisibility)
      (pos, "`__toString` must have public visibility and cannot be static")
      []
  | Abstract_body pos ->
    User_error.make
      Error_code.(to_enum AbstractBody)
      (pos, "This method shouldn't have a body")
      []
  | Interface_with_member_variable pos ->
    User_error.make
      Error_code.(to_enum InterfaceWithMemberVariable)
      (pos, "Interfaces cannot have member variables")
      []
  | Interface_with_static_member_variable pos ->
    User_error.make
      Error_code.(to_enum InterfaceWithStaticMemberVariable)
      (pos, "Interfaces cannot have static variables")
      []
  | Illegal_function_name { pos; name } ->
    User_error.make
      Error_code.(to_enum IllegalFunctionName)
      ( pos,
        Format.sprintf
          "Illegal function name: %s"
          (Render.strip_ns name |> Markdown_lite.md_codify) )
      []
  | Entrypoint_arguments pos ->
    User_error.make
      Error_code.(to_enum EntryPointArguments)
      (pos, "`__EntryPoint` functions cannot take arguments.")
      []
  | Entrypoint_generics pos ->
    User_error.make
      Error_code.(to_enum EntryPointGenerics)
      (pos, "`__EntryPoint` functions cannot have generic parameters.")
      []
  | Variadic_memoize pos ->
    User_error.make
      Error_code.(to_enum VariadicMemoize)
      (pos, "Memoized functions cannot be variadic.")
      []
  | Abstract_method_memoize pos ->
    User_error.make
      Error_code.(to_enum AbstractMethodMemoize)
      (pos, "Abstract methods cannot be memoized.")
      []
  | Instance_property_in_abstract_final_class pos ->
    User_error.make
      Error_code.(to_enum InstancePropertyInAbstractFinalClass)
      (pos, "Abstract final classes cannot have instance properties.")
      []
  | Inout_params_special pos ->
    User_error.make
      Error_code.(to_enum InoutParamsSpecial)
      (pos, "Methods with special semantics cannot have `inout` parameters.")
      []
  | Inout_params_memoize { pos; param_pos } ->
    User_error.make
      Error_code.(to_enum InoutParamsMemoize)
      (pos, "Functions with `inout` parameters cannot be memoized")
      [(Pos_or_decl.of_raw_pos param_pos, "This is an `inout` parameter")]
  | Inout_in_transformed_pseudofunction { pos; fn_name } ->
    User_error.make
      Error_code.(to_enum InoutInTransformedPsuedofunction)
      (pos, Printf.sprintf "Unexpected `inout` argument for `%s`" fn_name)
      []
  | Reading_from_append pos ->
    User_error.make
      Error_code.(to_enum ReadingFromAppend)
      (pos, "Cannot use `[]` for reading")
      []
  | List_rvalue pos ->
    User_error.make
      Error_code.(to_enum ListRvalue)
      ( pos,
        "`list()` can only be used for destructuring assignment. Did you mean `tuple()` or `vec[]`?"
      )
      []
  | Inout_argument_bad_expr pos ->
    User_error.make
      Error_code.(to_enum InoutArgumentBadExpr)
      ( pos,
        "`inout` arguments may only be local variables or array indexing expressions "
      )
      []
  | Illegal_destructor pos ->
    User_error.make
      Error_code.(to_enum IllegalDestructor)
      ( pos,
        "Destructors are not supported in Hack; use other patterns like "
        ^ "`IDisposable`/`using` or `try`/`catch` instead." )
      []
  | Switch_non_terminal_default pos ->
    User_error.make
      Error_code.(to_enum SwitchNonTerminalDefault)
      (pos, "Default case in `switch` must be terminal")
      []
  | Switch_multiple_default pos ->
    User_error.make
      Error_code.(to_enum SwitchMultipleDefault)
      (pos, "There can be only one `default` case in `switch`")
      []
  | Illegal_context { pos; name } ->
    User_error.make
      Error_code.(to_enum IllegalContext)
      ( pos,
        Format.sprintf
          "Illegal context: %s\nCannot use a context defined outside namespace %s"
          (Markdown_lite.md_codify name)
          Naming_special_names.Coeffects.contexts )
      []
  | Case_fallthrough { switch_pos; case_pos } ->
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
    User_error.make Error_code.(to_enum CaseFallthrough) claim reasons
  | Default_fallthrough pos ->
    User_error.make
      Error_code.(to_enum DefaultFallthrough)
      ( pos,
        "This `switch` has a default case that implicitly falls "
        ^ "through and is not annotated with `// FALLTHROUGH`" )
      []
  | Php_lambda_disallowed pos ->
    User_error.make
      Error_code.(to_enum PhpLambdaDisallowed)
      (pos, "PHP style anonymous functions are not allowed.")
      []
  | Non_interface { pos; name; verb } ->
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
  | Uses_non_trait { pos; name; kind } ->
    User_error.make
      Error_code.(to_enum UsesNonTrait)
      ( pos,
        Format.sprintf
          "%s is not a trait. It is %s."
          (Markdown_lite.md_codify @@ Render.strip_ns name)
          kind )
      []
  | Requires_non_class { pos; name; kind } ->
    User_error.make
      Error_code.(to_enum RequiresNonClass)
      ( pos,
        Format.sprintf
          "%s is not a class. It is %s."
          (Markdown_lite.md_codify @@ Render.strip_ns name)
          kind )
      []
  | Requires_final_class { pos; name } ->
    User_error.make
      Error_code.(to_enum RequiresFinalClass)
      ( pos,
        Format.sprintf
          "%s is not an extendable class."
          (Markdown_lite.md_codify @@ Render.strip_ns name) )
      []
  | Internal_method_with_invalid_visibility { pos; vis } ->
    let vis_str =
      String.lowercase
      @@ Markdown_lite.md_codify
      @@ Ast_defs.show_visibility vis
    in
    let msg =
      Format.sprintf
        "`__Internal` methods must be public, they cannot be %s"
        vis_str
    in
    User_error.make
      Error_code.(to_enum InternalProtectedOrPrivate)
      (pos, msg)
      []
  | Private_and_final pos ->
    User_error.make
      Error_code.(to_enum PrivateAndFinal)
      (pos, "Class methods cannot be both `private` and `final`.")
      []

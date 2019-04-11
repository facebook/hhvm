(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO: Integrate these with the rest of the Hack error messages. *)

type error_type = ParseError | RuntimeError [@@deriving show]

type t = {
  child        : t option;
  start_offset : int;
  end_offset   : int;
  error_type   : error_type;
  message      : string;
} [@@deriving show]

exception ParserFatal of t * Pos.t

let make
  ?(child = None) ?(error_type = ParseError) start_offset end_offset message =
  { child; error_type; start_offset; end_offset; message }

let rec to_positioned_string error offset_to_position =
  let child =
    match error.child with
    | Some child ->
      Printf.sprintf "\n  %s" (to_positioned_string child offset_to_position)
    | _ -> "" in
  let (sl, sc) = offset_to_position error.start_offset in
  let (el, ec) = offset_to_position error.end_offset in
  Printf.sprintf "(%d,%d)-(%d,%d) %s%s" sl sc el ec error.message child

let compare err1 err2 =
  if err1.start_offset < err2.start_offset then -1
  else if err1.start_offset > err2.start_offset then 1
  else if err1.end_offset < err2.end_offset then -1
  else if err1.end_offset > err2.end_offset then 1
  else 0

let exactly_equal err1 err2 =
  err1.start_offset = err2.start_offset &&
    err1.end_offset = err2.end_offset &&
    err1.message = err2.message

let expected_as_or_insteadof =
  "The 'as' keyword or the 'insteadof' keyword is expected here."

let error_type err = err.error_type

let message err = err.message

let start_offset err = err.start_offset
let end_offset err = err.end_offset

(* Lexical errors *)
let error0001 = "A hexadecimal literal needs at least one digit."
let error0002 = "A binary literal needs at least one digit."
let error0003 = "A floating point literal with an exponent needs at least " ^
                "one digit in the exponent."
let error0006 = "This character is invalid."
let error0007 = "This delimited comment is not terminated."
let error0008 = "A name is expected here."
let error0010 = "A single quote is expected here."
let error0011 = "A newline is expected here."
let error0012 = "This string literal is not terminated."
let error0013 = "This XHP body is not terminated."
let error0014 = "This XHP comment is not terminated."

(* Syntactic errors *)
let error1001 = "A .php file must begin with '<?hh'."
let error1003 = "The 'function' keyword is expected here."
let error1004 = "A name is expected here."
let error1006 = "A right brace ('}') is expected here."
let error1007 = "A type specifier is expected here."
let error1008 = "A variable name is expected here."
let error1010 = "A semicolon (';') is expected here."
let error1011 = "A right parenthesis (')') is expected here."
let error1013 = "A closing angle bracket ('>') is expected here."
let error1014 = "A closing angle bracket ('>') or comma is expected here."
let error1015 = "An expression is expected here."
let error1016 = "An assignment is expected here."
let error1017 = "An XHP attribute value is expected here."
let error1018 = "The 'while' keyword is expected here."
let error1019 = "A left parenthesis ('(') is expected here."
let error1020 = "A colon (':') is expected here."
let error1021 = "An opening angle bracket ('<') is expected here."
(* TODO: Remove this; redundant to 1009. *)
let error1022 = "A right parenthesis ('>') or comma (',') is expected here."
let error1023 = "An 'as' keyword is expected here."
let error1025 = "A shape field name is expected here."
let error1026 = "An opening square bracket ('[') is expected here."
let error1028 = "An arrow ('=>') is expected here."
let error1029 = "A closing double angle bracket ('>>') is expected here."
let error1031 = "A comma (',') or a closing square bracket (']') is expected here."
let error1032 = "A closing square bracket (']') is expected here."
(* TODO: Break this up according to classish type *)
let error1033 = "A class member, method, type, trait usage, trait require, " ^
  "xhp attribute, xhp use, or xhp category is expected here."
let error1034 = "A left brace ('{') is expected here."
let error1035 = "The 'class' keyword is expected here."
let error1036 = "An equals sign ('=') is expected here."
let error1038 = "A semicolon (';') or a namespace body is expected here."
let error1039 = "A closing XHP tag is expected here."
let error1041 = "A function body or a semicolon (';') is expected here."
let error1044 = "A name, __construct, or __destruct keyword is expected here."
let error1045 = "An 'extends' or 'implements' keyword is expected here."
let error1046 = "A lambda arrow ('==>') is expected here."
let error1047 = "A scope resolution operator ('::') is expected here."
let error1048 = "A name, variable name or 'class' is expected here."
let error1050 = "A name or variable name is expected here."
let error1051 = "The 'required' keyword is expected here."
let error1052 = "An XHP category name beginning with a '%' is expected here."
let error1053 = "An XHP name or category name is expected here."
let error1054 = "A comma (',') is expected here."
let error1055 = "A fallthrough directive can only appear at the end of" ^
  " a switch section."
(* TODO(20052790): use the specific token's text in the message body. *)
let error1056 = "This token is not valid as part of a function declaration."
let error1057 text = "Encountered unexpected token '" ^ text ^ "'."
let error1058 received required = Printf.sprintf ("Encountered unexpected " ^^
  "token '%s'. Did you mean '%s'?") received required
let error1059 terminator = Printf.sprintf
  "An '%s' is required when using alternate block syntax."
  (Full_fidelity_token_kind.to_string terminator)
let error1060 = "Leading markup and `<?hh` are not permitted in `.hack` "^
  "files, which are always strict."
let error1061 = "A Pocket Universes operator (':@') is expected here."
let error1062 = "References in use lists are not supported in Hack."

let error2001 = "A type annotation is required in strict mode."
let error2003 = "A case statement may only appear directly inside a switch."
let error2004 = "A default statement may only appear directly inside a switch."
let error2005 = "A break statement may only appear inside a switch or loop."
let error2006 = "A continue statement may only appear inside a loop."
let error2007 = "A try statement requires a catch or a finally clause."
let error2008 = "The first statement inside a switch statement must " ^
  "be a case or default label statement."
let error2009 class_name method_name =
  Printf.sprintf "Constructor %s::%s() cannot be static"
    class_name method_name

let error2010 = "Parameters cannot have visibility modifiers (except in " ^
  "parameter lists of constructors)."
let error2011 = "A destructor must have an empty parameter list."
let error2012 = "A destructor can only have visibility modifiers."
let error2013 = "A method declaration cannot have duplicate modifiers."
let error2014 = "An abstract method cannot have a method body."
let error2015 class_name method_name =
  Printf.sprintf "Non-abstract method %s::%s must contain body"
    class_name method_name
let error2016 class_name method_name =
  Printf.sprintf "Cannot declare abstract method %s::%s private"
    class_name method_name
let error2017 =
  "A method declaration cannot have multiple visibility modifiers."
let error2018 =
  "A constructor or destructor cannot have a non-void type annotation."
let error2019 class_name method_name =
  Printf.sprintf "Cannot declare abstract method %s::%s final"
    class_name method_name
let error2020 = "Use of the '{}' subscript operator is deprecated; " ^
  " use '[]' instead."
let error2021 = "A variadic parameter ('...') may only appear at the end of " ^
  "a parameter list."
let error2022 = "A variadic parameter ('...') may not be followed by a comma."

let error2029 = "Only traits and interfaces may use 'require extends'."
let error2030 = "Only traits may use 'require implements'."
let error2031 =
  "A class, interface, or trait declaration cannot have duplicate modifiers."
let error2032 = "The array type is not allowed in strict mode."
let error2033 = "The splat operator ('...') for unpacking variadic arguments " ^
  "may only appear at the end of an argument list."
let error2034 = "A type alias declaration cannot both use 'type' and have a " ^
  "constraint. Did you mean 'newtype'?"
let error2035 = "Only classes may implement interfaces."
let error2036 = "Only interfaces and classes may extend other interfaces and "
  ^ "classes."
let error2037 = "A class may extend at most one other class."
let error2038 constructor_name =
  "A constructor initializing an object must be passed a (possibly empty) " ^
  "list of arguments. Did you mean 'new " ^ constructor_name ^ "()'?"
let error2039 classish_keyword classish_name function_name = Printf.sprintf
  ("Cannot define a class, interface, or trait inside a function. Currently " ^^
  "%s '%s' is inside function '%s'.") classish_keyword classish_name function_name
let error2040 = "Invalid use of 'list(...)'. A list expression may only be " ^
  "used as the left side of a simple assignment, the value clause of a " ^
  "foreach loop, or a list item nested inside another list expression."
let error2041 = "Unexpected method body: interfaces may contain only" ^
  " method signatures, and not method implementations."
let error2042 = "Interfaces may not be declared 'abstract'."
let error2043 = "Traits may not be declared 'abstract'."
let error2044 class_name method_name = Printf.sprintf ("Classes cannot both " ^^
  "contain abstract methods and be non-abstract. Either declare 'abstract " ^^
  "class %s', or make 'function %s' non-abstract.") class_name method_name
let error2045 = "No method inside an interface may be declared 'abstract'."
let error2046 = "The 'async' annotation cannot be used on 'abstract' methods " ^
  "or methods inside of interfaces."
let error2047 visibility_modifier = "Methods inside of interfaces may not be " ^
  "marked '" ^ visibility_modifier ^ "'; only 'public' visibility is allowed."
let error2048 = "Expected group use prefix to end with '\\'"
let error2049 = "A namespace use clause may not specify the kind here."
let error2050 = "A concrete constant declaration must have an initializer."
let error2051 = "An abstract constant declaration must not have an initializer."
let error2052 = "Cannot mix bracketed namespace declarations with " ^
  "unbracketed namespace declarations"
let error2053 = "Use of 'var' as synonym for 'public' in declaration disallowed in Hack. " ^
  "Use 'public' instead."
let error2054 = "Method declarations require a visibility modifier " ^
  "such as public, private or protected."
let error2055 = "At least one enumerated item is expected."
let error2056 = "First unbracketed namespace occurrence here"
let error2057 = "First bracketed namespace occurrence here"
let error2058 = "Property may not be abstract."
let invalid_shape_field_name = "Shape field name must be a nonempty single-quoted string or a class constant"
let shape_field_int_like_string = "Shape field name must not be an int-like string (i.e. \"123\")"
let error2061 = "Non-static instance variables are not allowed in abstract " ^
  "final classes."
let error2062 = "Non-static methods are not allowed in abstract final classes."
let error2063 = "Expected integer or string literal."
let error2064 = "Reference methods are not allowed in strict mode."
let error2065 = "A variadic parameter ('...') must not have a default value."
(* This was typing error 4077. *)
let error2066 = "A previous parameter has a default value. Remove all the " ^
  "default values for the preceding parameters, or add a default value to " ^
  "this one."
let error2067 = "A hack source file cannot contain '?>'."
let error2068 = "hh blocks and php blocks cannot be mixed."
let error2069 = "Operator '?->' is only allowed in Hack."
let error2070 ~open_tag ~close_tag =
  Printf.sprintf "XHP: mismatched tag: '%s' not the same as '%s'"
    close_tag open_tag
let error2071 s = "Decimal number is too big: " ^ s
let error2072 s = "Hexadecimal number is too big: " ^ s
let error2073 = "A variadic parameter ('...') cannot have a modifier " ^
  "that changes the calling convention, like 'inout'."
let error2074 call_modifier = "An '" ^ call_modifier ^ "' parameter must not " ^
  "have a default value."
let error2075 call_modifier = "An '" ^ call_modifier ^ "' parameter cannot " ^
  "be passed by reference ('&')."
let error2076 = "Cannot use both 'inout' and '&' on the same argument."
let error2077 = "Cannot use empty list"
let error2078 = "Superglobals may not be taken by reference."
let list_must_be_lvar = "list() can only be used as an lvar. Did you mean to use tuple()?"

(* Start giving names rather than numbers *)
let async_not_last = "The 'async' modifier must be directly before the 'function' keyword."
let list_as_subscript = "A subscript index cannot be a list"
let vdarray_in_php = "varray and darray are only allowed in Hack files"
let uppercase_kw text = "Keyword " ^ text ^ " must be written in lowercase"
let using_st_function_scoped_top_level =
  "Using statement in function scoped form may only be used at the top " ^
  "level of a function or a method"
let const_in_trait = "Traits cannot have constants"
let const_visibility = "Class constants cannot have visibility modifiers in " ^
  "Hack files"
let strict_namespace_hh =
  "To use strict hack, place // strict after the open tag. " ^
  "If it's already there, remove this line. " ^
  "Hack is strict already."
let strict_namespace_not_hh =
  "Strict mode is not supported for non-Hack files."

let original_definition = "Original definition"

let name_is_already_in_use_php ~name ~short_name =
  "Cannot use " ^ name ^ " as " ^ short_name ^
  " because the name is already in use"

let name_is_already_in_use_hh ~line_num ~name ~short_name =
  "Cannot use " ^ name ^ " as " ^ short_name ^
  " because the name was explicitly used earlier via a `use' statement on line "
  ^ (string_of_int line_num)

let name_is_already_in_use_implicit_hh ~line_num ~name ~short_name =
  "Cannot use " ^ name ^ " as " ^ short_name ^ " because the name was implicitly used on line "
  ^ (string_of_int line_num) ^
  "; implicit use of names from the HH namespace can be suppressed by adding \
   an explicit `use' statement earlier in the current namespace block"

let declared_name_is_already_in_use_implicit_hh ~line_num ~name ~short_name:_ =
  "Cannot declare class " ^ name ^ " because the name was implicitly used on line "
  ^ (string_of_int line_num) ^
  "; implicit use of names from the HH namespace can be suppressed by adding \
   an explicit `use' statement earlier in the current namespace block"

let declared_name_is_already_in_use ~line_num ~name ~short_name:_ =
  "Cannot declare class " ^ name ^
  " because the name was explicitly used earlier via a `use' statement on line "
  ^ (string_of_int line_num)

let namespace_name_is_already_in_use ~name ~short_name =
  "Cannot use namespace " ^ name ^ " as " ^ short_name ^
  " because the name is already in use"

let function_name_is_already_in_use ~name ~short_name =
  "Cannot use function " ^ name ^ " as " ^ short_name ^
  " because the name is already in use"

let empty_method_name =
  "Expected a method name"

let const_name_is_already_in_use ~name ~short_name =
  "Cannot use const " ^ name ^ " as " ^ short_name ^
  " because the name is already in use"

let type_name_is_already_in_use ~name ~short_name =
  "Cannot use type " ^ name ^ " as " ^ short_name ^
  " because the name is already in use"

let variadic_reference = "Variadic '...' should be followed by a '$variable'"
let double_variadic = "Parameter redundantly marked as variadic ('...')."
let double_reference = "Parameter redundantly marked as reference ('&')."
let global_in_const_decl = "Cannot have globals in constant declaration"
let parent_static_const_decl = "Cannot use static or parent::class in constant declaration"
let parent_static_prop_decl = "Cannot use static or parent::class in property declaration"

let conflicting_trait_require_clauses ~name =
  "Conflicting requirements for '" ^ name ^ "'"

let shape_type_ellipsis_without_trailing_comma =
  "A comma is required before the ... in a shape type"

let yield_in_magic_methods =
  "'yield' is not allowed in constructor, destructor, or magic methods"

let reference_not_allowed_on_key = "Key of collection element cannot " ^
  "be marked as reference"
let reference_not_allowed_on_value = "Value of collection element cannot " ^
  "be marked as reference"
let reference_not_allowed_on_element = "Collection element cannot " ^
  "be marked as reference"
let yield_outside_function =
  "Yield can only be used inside a function"

let coloncolonclass_on_dynamic =
  "Dynamic class names are not allowed in compile-time ::class fetch"
let enum_elem_name_is_class =
  "Enum element cannot be named 'class'"
let expected_dotdotdot =
  "'...' is expected here."
let not_allowed_in_write what =
  what ^ " is not allowed in write context"
let references_not_allowed =
  "References are only allowed as function call arguments"
let reassign_this =
  "Cannot re-assign $this"
let this_in_static =
  "Don't use $this in a static method, use static:: instead"
let strict_types_first_statement =
  "strict_types declaration must be the very first statement in the script"
let async_magic_method ~name =
  "cannot declare constructors, destructors, and magic methods such as '"^ name ^ "' async"

let reserved_keyword_as_class_name class_name =
  "Cannot use '" ^ class_name ^ "' as class name as it is reserved"
let xhp_class_multiple_category_decls =
  "An XHP class cannot have multiple category declarations"
let inout_param_in_generator =
  "Parameters may not be marked inout on generators"
let inout_param_in_async_generator =
  "Parameters may not be marked inout on async generators"
let inout_param_in_async =
  "Parameters may not be marked inout on async functions"
let inout_param_in_construct =
  "Parameters may not be marked inout on constructors"
let fun_arg_inout_set =
  "You cannot set an inout decorated argument while calling a function"
let fun_arg_inout_const =
  "You cannot decorate a constant as inout"
let fun_arg_invalid_arg =
  "You cannot decorate this argument as inout"
let fun_arg_inout_containers =
  "Parameters marked inout must be contained in locals, vecs, dicts, keysets," ^
  " and arrays"
let memoize_with_inout =
  "<<__Memoize>> cannot be used on functions with inout parameters"
let fn_with_inout_and_ref_params =
  "Functions may not use both reference and inout parameters"
let method_calls_on_xhp_attributes =
  "Method calls are not allowed on XHP attributes"

let method_calls_on_xhp_expression =
  "Please add parentheses around the XHP component"
let invalid_constant_initializer =
  "Invalid expression in constant initializer"
let no_args_in_halt_compiler =
  "__halt_compiler function does not accept any arguments"

let no_async_before_lambda_body =
  "Unexpected use of async {...} as lambda expression"

let halt_compiler_top_level_only =
  "__halt_compiler function should appear only at the top level"
let trait_alias_rule_allows_only_final_and_visibility_modifiers =
  "Only 'final' and visibility modifiers are allowed in trait alias rule"
let namespace_decl_first_statement =
  "Namespace declaration statement has to be the very first statement in the script"
let code_outside_namespace =
  "No code may exist outside of namespace {}"
let strict_types_in_declare_block_mode =
  "strict_types declaration must not use block mode"
let invalid_number_of_args name n =
  "Method " ^ name ^ " must take exactly " ^ (string_of_int n) ^ " arguments"
let invalid_args_by_ref name =
  "Method " ^ name ^ " cannot take arguments by reference"
let redeclaration_error name =
  "Cannot redeclare " ^ name
let reference_to_static_scope_resolution =
  "Cannot take a reference to a static scope resolution expression"
let class_with_abstract_method name =
  "Class " ^ name ^ " contains an abstract method and must " ^
  "therefore be declared abstract"
let interface_has_private_method =
  "Access type for interface method must be omitted"
let redeclaration_of_function ~name ~loc =
  "Cannot redeclare " ^ name ^ "() (previously declared in " ^ loc ^ ")"
let redeclaration_of_method ~name =
  "Redeclared method " ^ name
let self_or_parent_colon_colon_class_outside_of_class name =
  "Cannot access " ^ name ^ "::class when no class scope is active"
let variadic_param_with_type_in_php name type_ =
  "Parameter " ^ name ^ " is variadic and has a type constraint ("
  ^ type_ ^ "); variadic params with type constraints are not "
  ^ "supported in non-Hack files"
let final_property = "Properties cannot be declared final"
let var_property = "Properties cannot be declared as var; a type is required"
let property_has_multiple_visibilities name =
  "Multiple access type modifiers are not allowed: properties of " ^ name
let property_requires_visibility =
  "Property declarations require a visibility modifier " ^
  "such as public, private or protected."
let invalid_is_as_expression_hint n hint =
  hint ^ " typehints cannot be used with " ^ n ^ "-expressions"
let elvis_operator_space = "An Elvis operator ('?:') is expected here."
let autoload_takes_one_argument =
  "__autoload() must take exactly 1 argument"
let clone_destruct_takes_no_arguments class_name method_name =
  Printf.sprintf "Method %s::%s cannot accept any arguments"
    class_name method_name
let class_destructor_cannot_be_static class_name method_name =
  Printf.sprintf "Destructor %s::%s() cannot be static"
  class_name method_name

let clone_cannot_be_static class_name method_name =
  Printf.sprintf "Clone method %s::%s() cannot be static"
  class_name method_name

let namespace_not_a_classname =
  "Namespace cannot be used as a classname"

let missing_double_quote = (* error0010 analogue *)
  "A double quote is expected here."

let for_with_as_expression =
  "For loops can not use as-expressions. Did you mean foreach?"

let sealed_val_not_classname = "Values in sealed whitelist must be classname constants."
let sealed_final = "Classes cannot be both final and sealed."
let sealed_enum = "Enums cannot be sealed."

let interface_implements =
  "Interfaces may not implement other interfaces or classes"

let memoize_on_lambda = "<<__Memoize>> attribute is not allowed on lambdas or \
  anonymous functions."

let memoize_lsb_on_non_static =
  "<<__MemoizeLSB>> can only be applied to static methods"

let memoize_lsb_on_non_method =
  "<<__MemoizeLSB>> can only be applied to methods"

let constants_as_attribute_arguments =
  "User-defined constants are not allowed in user attribute expressions"

let instanceof_paren x =
  Printf.sprintf "`instanceof (%s)` is not allowed because it is ambiguous. Is `%s` a class or a constant?"
    x x

let instanceof_invalid_scope_resolution = "A scope resolution (::) on the right side of an " ^
  "instanceof operator must start with a class name, `self`, `parent`, or `static`, and end with " ^
  "a variable"

let instanceof_memberselection_inside_scoperesolution = "A scope resolution (::) on the right " ^
  "side of an instanceof operator cannot contain a member selection (->)"

let instanceof_missing_subscript_index = "A subscript expression ([]) on the right side of an " ^
  "instanceof operator must have an index"

let instanceof_new_unknown_node msg =
  Printf.sprintf "Unexpected node on right hand side of new or instanceof: %s" msg

let instanceof_reference = "References are not allowed on the right side of an instanceof operation"

let instanceof_disabled = "The 'instanceof' operator is not supported in Hack; \
  use the 'is' operator or 'is_a()'"

let invalid_await_use = "Await cannot be used as an expression"

let toplevel_await_use = "Await cannot be used in a toplevel statement"

let invalid_default_argument s = s ^ " expression is not permitted \
  as the default value to a function parameter"

let invalid_constructor_method_call = "Method call following immediate constructor call \
  requires parentheses around constructor call."

let do_not_use_xor =
  "Do not use \"xor\", it has surprising precedence. Cast to bool and use \"!==\" instead"

let do_not_use_or =
  "Do not use \"or\", it has surprising precedence. Use \"||\" instead"

let do_not_use_and =
  "Do not use \"and\", it has surprising precedence. Use \"&&\" instead"

let do_not_use_ltgt =
  "Do not use '<>', it performs type coercion. Use '!==' instead"

let invalid_foreach_element = "An arrow ('=>') or right parenthesis (')') \
  is expected here."
let invalid_scope_resolution_qualifier = "Only classnames and variables are allowed before '::'."
let invalid_variable_name =
  "A valid variable name starts with a letter or underscore, followed \
  by any number of letters, numbers, or underscores"

let invalid_reference = "Only variables, members, and the results of function calls \
  can be used as references"

let invalid_variable_variable = "Variable Variables are not legal"

let function_modifier s =
  Printf.sprintf "Top-level function cannot have modifier '%s'" s

let invalid_yield =
  "Yield can only appear as a statement or on the right of an assignment"

let invalid_yield_from =
  "`yield from` can only appear as a statement, after `return`, or on the right of an assignment"

let invalid_class_in_collection_initializer =
  "Cannot use collection initialization for non-collection class."
let invalid_brace_kind_in_collection_initializer =
  "Initializers of 'vec', 'dict' and 'keyset' should use '[...]' instead of '{...}'."

let nested_ternary =
  "Nested ternary expressions inside ternary expressions are ambiguous. Please add parentheses"

let alternate_control_flow =
  "Alternate control flow syntax is not allowed in Hack files"
let execution_operator =
  "The execution operator is not allowed in Hack files"
let goto = "The `goto` operator is not allowed in Hack files"
let invalid_octal_integer = "Invalid octal integers"
let php7_anonymous_function =
  "Type annotations should occur before the use() clause"

let prefixed_invalid_string_kind = "Only double-quoted strings may be prefixed."

let non_re_prefix = "Only `re`-prefixed strings allowed."

let collection_intrinsic_generic =
  "Cannot initialize collection builtins with type parameters"

let collection_intrinsic_many_typeargs =
  "Collection expression must have less than three type arguments"

let invalid_hack_mode =
  "Incorrect comment; possible values include strict, partial, or empty"

let pair_initializer_needed =
  "Initializer needed for Pair object"

let pair_initializer_arity =
  "Pair objects must have exactly 2 elements"

let nested_unary_reference = "References cannot be followed by unary operators"

let toplevel_statements =
  "Toplevel statements besides requires are not allowed in strict files"
let invalid_reified =
  "Reify keyword can only appear at function or class type parameter position"
let reified_in_interface =
  "Invalid to use a reified type within an interface's type parameters"
let shadowing_reified =
  "You may not shadow a reified parameter"
let static_property_in_reified_class =
  "You may not use static properties in a class with reified type parameters"
let cls_reified_generic_in_static_method =
  "You may not use reified generics of the class in a static method"
let static_method_reified_obj_creation =
  "You may not use object creation for potentially reified self or parent from a static method"
let non_invariant_reified_generic =
  "Reified generics cannot be covariant or contravariant"
let no_generics_on_constructors =
  "Generic type parameters are not allowed on constructors. Consider adding a type parameter to the class"
let no_type_parameters_on_dynamic_method_calls =
  "Generics type parameters are disallowed on dynamic method calls"

let dollar_unary = "The dollar sign ('$') cannot be used as a unary operator"

let decl_outside_global_scope = "Declarations are not supported outside global scope"

let experimental_in_codegen_without_hacksperimental =
  "Experimental mode files are not allowed during codegen unless the hacksperimental flag is set"

let expected_simple_offset_expression =
  "A simple offset expression is expected here"

let illegal_interpolated_brace_with_embedded_dollar_expression =
  "The only legal expressions inside a {$...}-expression embedded in a string are " ^
  "variables, function calls, subscript expressions, and member access expressions"

let type_alias_to_type_constant =
  "Type aliases to type constants are not supported"

let interface_with_memoize =
  "Memoize is not allowed on interface methods"

let xhp_class_attribute_type_constant =
  "Type constants are not allowed on xhp class attributes"

let inline_function_def =
  "Inline function definitions are not supported in Hack"

let lowering_parsing_error text syntax =
  "Encountered unexpected text '"^text^"', was expecting a "^syntax^"."

let multiple_reactivity_annotations =
  "Only one of following annotations is allowed: \
  __Rx, __RxShallow, __RxLocal, __NonRx."

let functions_cannot_implement_reactive =
  "__OnlyRxIfImpl annotations are only valid on class methods."

let missing_reactivity_for_condition =
  "__OnlyRxIfImpl and __AtMostRxAsArgs annotations cannot " ^
  "be used without __Rx, __RxShallow, or __RxLocal."

let misplaced_owned_mutable =
  "__OwnedMutable annotation can only be placed on parameters."

let conflicting_mutable_and_owned_mutable_attributes =
  "Parameter cannot have both __Mutable and __OwnedMutable annotations."

let conflicting_mutable_and_maybe_mutable_attributes =
  "Parameter cannot have both __Mutable and __MaybeMutable annotations."

let conflicting_owned_mutable_and_maybe_mutable_attributes =
  "Parameter cannot have both __OwnedMutable and __MaybeMutable annotations."

let mutably_owned_attribute_on_non_rx_function =
  "__OwnedMutable annotated parameters are only allowed in reactive functions."

let invalid_non_rx_argument_for_lambda =
  "Invalid argument list for __NonRx attribute that is placed on \
  anonymous function. Argument list for __NonRx attribute that is used \
  in this position should be empty."

let invalid_non_rx_argument_for_declaration  =
  "Invalid argument list for __NonRx attribute that is placed on \
  a declaration of function or method. Argument list for __NonRx attribute that is \
  used in this position should contain only one string literal value."

let nested_concurrent_blocks =
  "Concurrent blocks cannot be nested."

let less_than_two_statements_in_concurrent_block =
  "Less than 2 statements in concurrent block."

let invalid_syntax_concurrent_block =
  "Concurrent block must contain a compound statement of two or " ^
  "more expression statements, IE concurrent { <expr>; <expr>; }."

let statement_without_await_in_concurrent_block =
  "Statement without an await in a concurrent block"

let concurrent_is_disabled =
  "Concurrent is disabled"

let static_closures_are_disabled =
  "Static closures are not supported in Hack"

let invalid_await_position = "Await cannot be used as an expression in this " ^
  "location because it's conditionally executed."

let misplaced_reactivity_annotation =
  "Reactive annotations are not allowed on classes, interfaces or traits."

let mutability_annotation_on_static_method =
  "__Mutable and __MaybeMutable annotations are not allowed on static methods."

let mutability_annotation_on_inout_parameter =
  "__Mutable, __MaybeMutable and __OwnedMutable annotations are not allowed on inout parameters."

let mutable_parameter_in_memoize_function ~is_this =
  "Memoized functions cannot have mutable " ^ (if is_this then "$this." else "parameters.")

let mutable_return_in_memoize_function =
  "Memoized functions cannot return mutable objects."

let vararg_and_mutable =
  "__Mutable, __OwnedMutable and __MaybeMutable annotations cannot be used with \
  variadic parameters."

let expected_user_attribute = "A user attribute is expected here."

let tparams_in_tconst = "Type parameters are not allowed on class type constants"

let targs_not_allowed = "Type arguments are not allowed in this position"

let reified_attribute = "__Reified and __HasReifiedParent attributes may not be provided by the user"

let lval_as_expression =
  "Lval can no longer be used as an expression. Pull it out into it's own statement."

let pocket_universe_final_expected =
  "The 'final' keyword is expected here."

let pocket_universe_enum_expected =
  "The 'enum' keyword is expected here."

let pocket_universe_invalid_field =
  "Invalid pocket universe field syntax."

let type_keyword =
    "The 'type' keyword is expected here."

(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO: Integrate these with the rest of the Hack error messages. *)

type error_type =
  | ParseError
  | RuntimeError
[@@deriving show]

type t = {
  child: t option;
  start_offset: int;
  end_offset: int;
  error_type: error_type;
  message: string;
}
[@@deriving show]

exception ParserFatal of t * Pos.t

let make
    ?(child = None) ?(error_type = ParseError) start_offset end_offset message
    =
  { child; error_type; start_offset; end_offset; message }

let rec to_positioned_string error offset_to_position =
  let child =
    match error.child with
    | Some child ->
      Printf.sprintf "\n  %s" (to_positioned_string child offset_to_position)
    | _ -> ""
  in
  let (sl, sc) = offset_to_position error.start_offset in
  let (el, ec) = offset_to_position error.end_offset in
  Printf.sprintf "(%d,%d)-(%d,%d) %s%s" sl sc el ec error.message child

let compare err1 err2 =
  if err1.start_offset < err2.start_offset then
    -1
  else if err1.start_offset > err2.start_offset then
    1
  else if err1.end_offset < err2.end_offset then
    -1
  else if err1.end_offset > err2.end_offset then
    1
  else
    0

let exactly_equal err1 err2 =
  err1.start_offset = err2.start_offset
  && err1.end_offset = err2.end_offset
  && err1.message = err2.message

let error_type err = err.error_type

let message err = err.message

let start_offset err = err.start_offset

let end_offset err = err.end_offset

let error1001 = "A .php file must begin with '<?hh'."

let error1060 =
  "Leading markup and `<?hh` are not permitted in `.hack` "
  ^ "files, which are always strict."

let invalid_shape_field_name =
  "Shape field name must be a nonempty single-quoted string or a class constant"

let shape_field_int_like_string =
  "Shape field name must not be an int-like string (i.e. \"123\")"

let empty_method_name = "Expected a method name"

let shape_type_ellipsis_without_trailing_comma =
  "A comma is required before the ... in a shape type"

let this_in_static = "Don't use $this in a static method, use static:: instead"

let autoload_takes_one_argument = "__autoload() must take exactly 1 argument"

let constants_as_attribute_arguments =
  "User-defined constants are not allowed in user attribute expressions"

let toplevel_await_use = "Await cannot be used in a toplevel statement"

let invalid_constructor_method_call =
  "Method call following immediate constructor call requires parentheses around constructor call."

let invalid_variable_name =
  "A valid variable name starts with a letter or underscore, followed by any number of letters, numbers, or underscores"

let invalid_variable_variable = "Variable Variables are not legal"

let invalid_yield =
  "Yield can only appear as a statement or on the right of an assignment"

let invalid_yield_from =
  "`yield from` can only appear as a statement, after `return`, or on the right of an assignment"

let goto = "The `goto` operator is not allowed in Hack files"

let goto_label =
  "Labels are used only for `goto`, which is not allowed in Hack files"

let invalid_octal_integer = "Invalid octal integers"

let non_re_prefix = "Only `re`-prefixed strings allowed."

let collection_intrinsic_many_typeargs =
  "Collection expression must have less than three type arguments"

let toplevel_statements =
  "Toplevel statements are not allowed. Use __EntryPoint attribute instead"

let invalid_reified =
  "Reify keyword can only appear at function or class type parameter position"

let cls_reified_generic_in_static_method =
  "You may not use reified generics of the class in a static method"

let static_method_reified_obj_creation =
  "You may not use object creation for potentially reified self or parent from a static method"

let non_invariant_reified_generic =
  "Reified generics cannot be covariant or contravariant"

let no_generics_on_constructors =
  "Generic type parameters are not allowed on constructors. Consider adding a type parameter to the class"

let experimental_in_codegen_without_hacksperimental =
  "Experimental mode files are not allowed during codegen unless the hacksperimental flag is set"

let xhp_class_attribute_type_constant =
  "Type constants are not allowed on xhp class attributes"

let lowering_parsing_error text syntax =
  "Encountered unexpected text '" ^ text ^ "', was expecting a " ^ syntax ^ "."

let statement_without_await_in_concurrent_block =
  "Statement without an await in a concurrent block"

let static_closures_are_disabled = "Static closures are not supported in Hack"

let halt_compiler_is_disabled = "__halt_compiler() is not supported in Hack"

let tparams_in_tconst =
  "Type parameters are not allowed on class type constants"

let targs_not_allowed = "Type arguments are not allowed in this position"

let reified_attribute =
  "__Reified and __HasReifiedParent attributes may not be provided by the user"

let only_soft_allowed = "Only the __Soft attribute is allowed here."

let soft_no_arguments = "The __Soft attribute does not take arguments."

let outside_dollar_str_interp =
  "The ${x} syntax is disallowed in Hack. Use {$x} instead."

let no_silence = "The error suppression operator @ is not allowed"

let const_mutation = "Cannot mutate a class constant"

let no_attributes_on_variadic_parameter =
  "Attributes on variadic parameters are not allowed"

let invalid_typehint_alias alias hint =
  "Invalid type hint '" ^ alias ^ "'. Use '" ^ hint ^ "' instead"

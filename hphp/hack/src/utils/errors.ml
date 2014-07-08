(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

(*****************************************************************************)
(* Errors accumulator. *)
(*****************************************************************************)

type error = (Pos.t * string) list
type t = error list

let (error_list: t ref) = ref []

let add pos msg =
  error_list := [pos, msg] :: !error_list

let add_list pos_msg_l =
  error_list := pos_msg_l :: !error_list

let add_error error =
  error_list := error :: !error_list

let to_list (error: error) = (error: (Pos.t * string) list)
let get_pos (error: error) = fst (List.hd error)
let filename (error: error) = Pos.filename (get_pos error)
let make_error (x: (Pos.t * string) list) = (x: error)

(*****************************************************************************)
(* Lexer error *)
(*****************************************************************************)

let fixme_format pos =
  add pos "HH_FIXME wrong format, expected '/* HH_FIXME[ERROR_NUMBER] */'"

let alok (pos, x) =
  add pos (
  "You probably forgot to bind this type parameter right?\nAdd <"^x^
  "> somewhere (after the function name definition, or after the class name)\nExamples: "^"function foo<T> or class A<T>")

let unexpected_eof pos =
  add pos "Unexpected end of file"

let missing_field pos1 pos2 name =
  add_list
    [pos1, "The field '"^name^"' is missing";
     pos2, "The field '"^name^"' is defined"]

let generic_class_var pos =
  add pos "A class variable cannot be generic"

let explain_constraint pos name (error: error) =
  add_list (
  error @
  [pos, "Considering the constraint on the type '"^name^"'"]
)

let too_many_args pos =
  add pos "Too many arguments"

let unexpected_arrow pos cname =
  add pos ("Keys may not be specified for " ^ cname ^ " initialization")

let missing_arrow pos cname =
  add pos ("Keys must be specified for " ^ cname ^ " initialization")

let disallowed_xhp_type pos name =
  add pos (name ^ " is not a valid type. Use :xhp or XHPChild.")

let overflow p =
  add p "Value is too large"

let unterminated_comment pos =
  add pos "unterminated comment"

let unterminated_xhp_comment pos =
  add pos "unterminated xhp comment"

let name_already_bound x p1 p2 =
  let x = Utils.strip_ns x in
  add_list [
    p1, "Name already bound: "^x;
    p2, "Previous definition is here"
]

let method_name_already_bound p name =
  add p ("Method name already bound: "^name)

let error_name_already_bound hhi_root x p1 p2 =
(* hhi_root =
*)
  let x = Utils.strip_ns x in
  let errs = [
    p1, "Name already bound: "^x;
    p2, "Previous definition is here"
  ] in
  let hhi_msg =
    "This appears to be defined in an hhi file included in your project "^
    "root. The hhi files for the standard library are now a part of the "^
    "typechecker and must be removed from your project. Typically, you can "^
    "do this by deleting the \"hhi\" directory you copied into your "^
    "project when first starting with Hack." in
  (* unsafe_opt since init stack will refuse to continue if we don't have an
   * hhi root. *)
  let errs =
    if str_starts_with p1.Pos.pos_file hhi_root
    then errs @ [p2, hhi_msg]
    else if str_starts_with p2.Pos.pos_file hhi_root
    then errs @ [p1, hhi_msg]
    else errs in
  add_list errs

let unbound_name p x =
  add p ("Unbound name: "^(strip_ns x))

let different_scope p x p' =
  add_list [p, ("The variable "^ x ^" is defined");
                   p', ("But in a different scope")]

let undefined p x =
  add p ("Undefined variable: "^x)

let this_reserved pos =
  add pos "The type parameter \"this\" is reserved"

let start_with_T pos =
  add pos
    "Please make your type parameter start with the letter T (capital)"

let already_bound pos x =
  add pos ("Argument already bound: "^x)

let unexpected_typedef pos def_pos =
  add_list [
  pos, "Unexpected typedef";
  def_pos, "Definition is here";
]

let fd_name_already_bound pos =
  add pos "Field name already bound"

let primitive_toplevel p =
  add p ("Primitive type annotations are always available and may no \
                  longer be referred to in the toplevel namespace.")

let integer_instead_of_int p =
  add p "Invalid Hack type. Using \"integer\" in Hack is considered \
    an error. Use \"int\" instead, to keep the codebase \
    consistent."

let boolean_instead_of_bool p =
  add p "Invalid Hack type. Using \"boolean\" in Hack is considered \
    an error. Use \"bool\" instead, to keep the codebase \
    consistent."

let double_instead_of_float p =
  add p "Invalid Hack type. Using \"double\" in Hack is considered \
    an error. Use \"float\" instead. They are equivalent data types \
    and the codebase remains consistent."

let real_instead_of_float p =
  add p "Invalid Hack type. Using \"real\" in Hack is considered \
    an error. Use \"float\" instead. They are equivalent data types and \
    the codebase remains consistent."

let this_no_argument p =
  add p "\"this\" expects no arguments"

let this_outside_of_class p =
   add p "Cannot use \"this\" outside of a class"

let this_must_be_return p =
  add p "The type \"this\" can only be used as a return type, \
to instantiate a covariant type variable, \
  or as a private non-static member variable"

let lowercase_this p x =
  add p ("Invalid Hack type \""^x^"\". Use \"this\" instead")

let tparam_with_tparam p x =
  add p (Printf.sprintf "%s is a type parameter. Type parameters cannot \
        themselves take type parameters (e.g. %s<int> doesn't make sense)" x x)

let shadowed_type_param p pos name =
  add_list [
    p, Printf.sprintf "You cannot re-bind the type parameter %s" name;
    pos, Printf.sprintf "%s is already bound here" name
  ]

let missing_typehint p =
  add p "Please add a type hint"

let expected_variable p =
  add p "Was expecting a variable name"

let too_few_arguments p =
  add p "Too few arguments"

let too_many_arguments p =
  add p "Too many arguments"

let expected_collection p cn =
  add p ("Unexpected collection type " ^ (Utils.strip_ns cn))

let illegal_CLASS p =
  add p "Using __CLASS__ outside a class"

let dynamic_method_call p =
  add p "Dynamic method call"

let illegal_fun p =
  let msg = "The argument to fun() must be a single-quoted, constant "^
    "literal string representing a valid function name." in
  add p msg

let illegal_meth_fun p =
  let msg = "String argument to fun() contains ':';"^
    " for static class methods, use"^
    " class_meth(Cls::class, 'method_name'), not fun('Cls::method_name')" in
  add p msg

let illegal_inst_meth p =
  let msg = "The argument to inst_meth() must be an expression and a "^
    "constant literal string representing a valid method name." in
  add p msg

let illegal_meth_caller p =
  let msg =
    "The two arguments to meth_caller() must be:"
    ^"\n - first: ClassOrInterface::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a non-static method of that class" in
  add p msg

let illegal_class_meth p =
  let msg =
    "The two arguments to class_meth() must be:"
    ^"\n - first: ValidClassname::class"
    ^"\n - second: a single-quoted string literal containing the name"
    ^" of a static method of that class" in
  add p msg

let assert_arity p =
  add p "assert expects exactly one argument"

let gena_arity p =
  add p "gena() expects exactly 1 argument"

let genva_arity p =
  add p "genva() expects at least 1 argument"

let gen_array_rec_arity p =
  add p "gen_array_rec() expects exactly 1 argument"

let gen_array_va_rec_arity p =
  add p
    "gen_array_va_rec_DEPRECATED() expects at least 1 argument"

let dynamic_class p =
  add p "Don't use dynamic classes"

let typedef_constraint pos =
  add pos "Constraints on typedefs are not supported"

let add_a_typehint pos =
  add pos "Please add a type hint"

let local_const var_pos =
  add var_pos "You cannot use a local variable in a constant definition"

let illegal_constant pos =
  add pos "Illegal constant value"

let cyclic_constraint p =
  add p "Cyclic constraint"

let parsing_error (p, msg) =
  add p msg

let format_string  pos snippet s class_pos fname class_suggest =
  add_list [
  (pos, "I don't understand the format string " ^ snippet ^ " in " ^ s);
  (class_pos,
   "You can add a new format specifier by adding "
   ^fname^"() to "^class_suggest)]

let expected_literal_string pos =
  add pos "This argument must be a literal string"

let generic_array_strict p =
  add p "You cannot have an array without generics in strict mode"

let nullable_void p =
  add p "?void is a nonsensical typehint"

let tuple_syntax p =
  add p ("Did you want a tuple? Try (X,Y), not tuple<X,Y>")

let class_arity pos class_name arity =
  add pos ("The class "^(Utils.strip_ns class_name)^" expects "^
           soi arity^" arguments")

let dynamic_yield_private pos =
  add_list
    [pos, "DynamicYield cannot see private methods in subclasses"]

let expecting_type_hint p =
  add p "Was expecting a type hint"

let expecting_type_hint_suggest p ty =
  add p ("Was expecting a type hint (what about: "^ty^")")

let expecting_return_type_hint p =
  add p "Was expecting a return type hint"

let expecting_return_type_hint_suggest p ty =
  add p ("Was expecting a return type hint (what about: ': "^ty^"')")

let field_kinds p1 p2 =
  add_list [p1, "You cannot use this kind of field (value)";
            p2, "Mixed with this kind of field (key => value)"]

let unbound_name_typing pos name =
  add pos ("Unbound name, Typing: "^(strip_ns name))

let previous_default p =
  add p
    ("A previous parameter has a default value.\n"^
     "Remove all the default values for the preceding parameters,\n"^
     "or add a default value to this one.")

let void_parameter p =
  add p "Cannot have a void parameter"

let nullable_parameter pos =
  add pos "Please add a ?, this argument can be null"

let return_in_void p1 p2 =
  add_list [
  p1,
  "You cannot return a value";
  p2,
  "This is a void function"
]

let this_in_static p =
  add p "Don't use $this in a static method"

let this_outside_class p =
  add p "Can't use $this outside of a class"

let unbound_global cst_pos =
  add cst_pos "Unbound global constant (Typing)"

let private_inst_meth method_pos p =
  add_list [
  method_pos, "This is a private method";
  p, "you cannot use it with inst_meth (whether you are in the same class or not)."
]

let protected_inst_meth method_pos p =
  add_list [
  method_pos, "This is a protected method";
  p, "you cannot use it with inst_meth (whether you are in the same class hierarchy or not)."
]

let private_class_meth p1 p2 =
  add_list [
  p1, "This is a private method";
  p2, "you cannot use it with class_meth (whether you are in the same class or not)."
]

let protected_class_meth p1 p2 =
  add_list [
  p1, "This is a protected method";
  p2, "you cannot use it with class_meth (whether you are in the same class hierarchy or not)."
]

let array_cast p =
  add p "(array) cast forbidden in strict mode; arrays with unspecified \
    key and value types are not allowed"

let anonymous_recursive p =
  add p "Anonymous functions cannot be recursive"

let new_static_outside_class p =
  add p "Can't use new static() outside of a class"

let new_self_outside_class p =
  add p "Can't use new self() outside of a class"

let abstract_instantiate p cname =
  add p ("Can't instantiate " ^ Utils.strip_ns cname)

let pair_arity p =
  add p "A pair has exactly 2 elements"

let tuple_arity p2 size2 p1 size1 =
  add_list [
  p2, "This tuple has "^ string_of_int size2^" elements";
  p1, string_of_int size1 ^ " were expected"]

let undefined_parent pos =
  add pos "The parent class is undefined"

let parent_construct_in_trait pos =
  add pos "Don't call parent::__construct from a trait"

let parent_outside_class pos =
  add pos "parent is undefined outside of a class"

let dont_use_isset p =
  add p "Don't use isset!"

let array_get_arity p1 name p2 =
  add_list [
  p1, "You cannot use this "^(Utils.strip_ns name);
  p2, "It is missing its type parameters"
]

let static_overflow p =
  add p "Static integer overflow"

let typing_error p msg =
  add p msg

let typing_error_l err =
  add_error err

let undefined_field p name =
  add p ("The field "^name^" is undefined")

let shape_access p =
  add p "Was expecting a constant string (for shape access)"

let array_access p1 p2 ty =
  add_list ((p1, "This is not a container, this is "^ty) ::
            if p2 != Pos.none
            then [p2, "You might want to check this out"]
            else [])

let array_append p1 p2 ty =
  add_list ((p1, ty^" does not allow array append") ::
            if p2 != Pos.none
            then [p2, "You might want to check this out"]
            else [])

let const_mutation p1 p2 ty =
  add_list ((p1, "You cannot mutate this") ::
            if p2 != Pos.none
            then [(p2, "This is " ^ ty)]
            else [])

let negative_tuple_index p =
  add p "You cannot use a negative value here"

let tuple_index_too_large p =
  add p "Cannot access this field"

let expected_static_int p =
  add p "Please use a static integer"

let expected_class p =
  add p "Was expecting a class"

let snot_found_hint = function
  | `no_hint ->
      []
  | `closest (pos, v) ->
      [pos, "The closest thing is "^v^" but it's not a static method"]
  | `did_you_mean (pos, v) ->
      [pos, "Did you mean: "^v]

let string_of_class_member_kind = function
  | `class_constant -> "class constant"
  | `static_method  -> "static method"
  | `class_variable -> "class variable"

let smember_not_found kind pos member_name hint =
  let kind = string_of_class_member_kind kind in
  add_list ((pos, "Could not find "^kind^" "^member_name)
            :: snot_found_hint hint)

let not_found_hint = function
  | `no_hint ->
      []
  | `closest (pos, v) ->
      [pos, "The closest thing is "^v^" but it's a static method"]
  | `did_you_mean (pos, v) ->
      [pos, "Did you mean: "^v]

let member_not_found kind pos (cpos, class_name) member_name hint =
  let kind =
    match kind with
    | `method_ -> "method "
    | `member -> "member "
  in
  let msg = "The "^kind^member_name^" is undefined "
    ^"in an object of type "^(strip_ns class_name)
  in
  add_list
    ((pos, msg) :: (cpos, "Check this out") ::
     not_found_hint hint)

let parent_in_trait p =
  add p
    ("parent:: inside a trait is undefined"
     ^" without 'require extends' of a class defined in <?hh")

let parent_undefined p =
  add p "parent is undefined"

let constructor_no_args p =
  add p "This constructor expects no argument"

let visibility p msg1 p_vis msg2 =
  add_list [p, msg1; p_vis, msg2]

let typing_too_many_args pos pos_def =
  add_list [pos, "Too many arguments"; pos_def, "Definition is here"]

let typing_too_few_args pos pos_def =
  add_list  [pos, "Too few arguments"; pos_def, "Definition is here"]

let anonymous_recursive_call pos =
  add pos "recursive call to anonymous function"

let bad_call p ty =
  add p ("This call is invalid, this is not a function, it is "^ty)

let sketchy_null_check p =
  add p ("You are using a sketchy null check ...\n"^
         "Use is_null, or $x === null instead")

let sketchy_null_check_primitive p =
  add p
    ("You are using a sketchy null check on a primitive type ...\n"^
     "Use is_null, or $x === null instead")

let extend_final position =
  add position "You cannot extend a class declared as final"

let read_before_write (p, v) =
  add p (
  sl[
  "Read access to $this->"; v; " before initialization"
])

let interface_final pos =
  add pos "Interfaces cannot be final"

let abstract_class_final pos =
  add pos "Abstract classes cannot be final"

let trait_final pos =
  add pos "Traits cannot be final"

let implement_abstract p1 p2 x =
  add_list [
  p1,
  "This class must provide an implementation for the abstract method "^x;
  p2,
  "The abstract method "^x^" is defined here";
]

let generic_static p x =
  add p ("This static variable cannot use the type parameter "^x^".")

let fun_too_many_args p1 p2 =
  add_list [p1, ("Too many mandatory arguments");
            p2, "Because of this definition"]

let fun_too_few_args p1 p2 =
  add_list [p1, ("Too few arguments");
            p2, "Because of this definition"]

let expected_tparam pos n =
  add pos ("Expected " ^
           (match n with
           | 0 -> "no type parameter"
           | 1 -> "a type parameter"
           | n -> string_of_int n ^ " type parameters"
           ))

let field_missing k p1 p2 =
  add_list [p2, "The field '"^k^"' is defined";
            p1, "The field '"^k^"' is missing"]

let object_string p1 p2 =
  add_list [
  p1, "You cannot use this object as a string";
  p2, "This object doesn't implement __toString"]

let untyped_string p =
  add_list [
  p,
  "You cannot use this object as a string, it is an untyped value"
]

let type_param_arity pos x n =
  add pos ("The type "^x^" expects "^n^" parameters")

let cyclic_typedef p =
  add p "Cyclic typedef"

let type_arity_mismatch p1 n1 p2 n2 =
  add_list [p1, "This type has "^n1^
            " arguments";
            p2, "This one has "^n2]

let this_final id p2 (error: error) =
  let n = Utils.strip_ns (snd id) in
  let message1 = "Since "^n^" is not final" in
  let message2 = "this might not be a "^n in
  add_list (error @ [(fst id, message1); (p2, message2)])

let tuple_arity_mismatch p1 n1 p2 n2 =
  add_list [p1, "This tuple has "^n1^" elements";
            p2, "This one has "^n2^" elements"]

let fun_arity_mismatch p1 p2 =
  add_list [p1, ("Arity mismatch"); p2, "Because of this definition"]

let discarded_awaitable p1 p2 =
  add_list [
  p1, "This expression is of type Awaitable, but it's "^
  "either being discarded or used in a dangerous way before "^
  "being awaited";
  p2, "This is why I think it is Awaitable"
]

let gena_expects_array p1 p2 ty_str =
  add_list [
  p1, "gena expects an array";
  p2, "It is incompatible with " ^ ty_str;
]

let unify_error left right =
  add_list (left @ right)

let static_dynamic static_position dyn_position method_name =
  let msg_static = "The function "^method_name^" is static" in
  let msg_dynamic = "It is defined as dynamic here" in
  add_list [static_position, msg_static; dyn_position, msg_dynamic]

let null_member s p r =
  add_list ([
  p,
  "You are trying to access the member "^s^
  " but this object can be null. "
] @ r
)

let non_object_member s p1 ty p2 =
  add_list [p1,
            ("You are trying to access the member "^s^
             " but this is not an object, it is "^
             ty);
            p2,
            "Check this out"]

let null_container p null_witness =
  add_list (
  [
   p,
   "You are trying to access an element of this container"^
   " but the container could be null. "
 ] @ null_witness)

let option_mixed pos =
  add pos "?mixed is a redundant typehint - just use mixed"

(*****************************************************************************)
(* Typing decl errors *)
(*****************************************************************************)

let wrong_extend_kind child_pos child parent_pos parent =
  let msg1 = child_pos, child^" cannot extend "^parent in
  let msg2 = parent_pos, "This is "^parent in
  add_list [msg1; msg2]

let unsatisfied_req pos req =
  add pos ("Failure to satisfy requirement: "^(Utils.strip_ns req))

let cyclic_class_def stack pos =
  let stack = SSet.fold (fun x y -> (Utils.strip_ns x)^" "^y) stack "" in
  add pos ("Cyclic class definition : "^stack)

let override_final ~parent ~child =
  add_list [child, "You cannot override this method";
            parent, "It was declared as final"]

let should_be_override pos class_id id =
  add pos
    ((Utils.strip_ns class_id)^"::"^id^"() is marked as override; \
       no non-private parent definition found \
       or overridden parent is defined in non-<?hh code")

let override_per_trait class_name id m_pos =
    let c_pos, c_name = class_name in
    let err_msg =
      ("Method "^(Utils.strip_ns c_name)^"::"^id^" is should be an override \
        per the declaring trait; no non-private parent definition found \
        or overridden parent is defined in non-<?hh code")
    in add_list [
      c_pos, err_msg;
      m_pos, "Declaration of "^id^"() is here"
    ]

let missing_assign pos =
  add pos "Please assign a value"

let private_override pos class_id id =
  add pos ((Utils.strip_ns class_id)^"::"^id
          ^": combining private and override is nonsensical")

(*****************************************************************************)
(* Init check errors *)
(*****************************************************************************)

let no_construct_parent p =
  add p (sl["You are extending a class that needs to be initialized\n";
            "Make sure you call parent::__construct.\n"
          ])

let not_initialized (p, c) =
  if c = "parent::__construct" then no_construct_parent p else
  add p (sl[
                "The class member "; c;" is not always properly initialized\n";
                "Make sure you systematically set $this->"; c;
                " when the method __construct is called\n";
                "Alternatively, you can define the type as optional (?...)\n"
              ])

let call_before_init p cv =
  add p (
  sl([
     "Until the initialization of $this is over,";
     " you can only call private methods\n";
     "The initialization is not over because ";
   ] @
     if cv = "parent::__construct"
     then ["you forgot to call parent::__construct"]
     else ["$this->"; cv; " can still potentially be null"])
 )

(*****************************************************************************)
(* Nast errors check *)
(*****************************************************************************)

let type_arity pos name nargs =
  add pos
    (sl ["The type ";(Utils.strip_ns name);" expects ";nargs;" type parameter(s)"])

let abstract_outside (p, _) =
  add p
    "This method is declared as abstract, in a class that isn't"

let invalid_req_implements p =
  add p "Only traits may use 'require implements'"

let invalid_req_extends p =
  add p "Only traits and interfaces may use 'require extends'"

let interface_with_body (p, _) =
  add p
    "A method cannot have a body in an interface"

let abstract_with_body (p, _) =
  add p
    "This method is declared as abstract, but has a body"

let not_abstract_without_body (p, _) =
  add p
    "This method is not declared as abstract, it must have a body"

let return_in_gen p =
  add p
    ("Don't use return in a generator (a generator"^
     " is a function that uses yield)")

let return_in_finally p =
  add p
    ("Don't use return in a finally block;"^
     " there's nothing to receive the return value")

let yield_in_async_function p =
  add p
    "Don't use yield in an async function"

let await_in_sync_function p =
  add p
    "await can only be used inside async functions"

let magic (p, s) =
  add p
    ("Don't call "^s^" it's one of these magic things we want to avoid")

let non_interface (p : Pos.t) (c2: string) (verb: string): 'a =
  add p ("Cannot " ^ verb ^ " " ^ (strip_ns c2) ^ " - it is not an interface")

let toString_returns_string pos =
  add pos "__toString should return a string"

let toString_visibility pos =
  add pos "__toString must have public visibility and cannot be static"

let uses_non_trait (p: Pos.t) (n: string) (t: string) =
  add p ((Utils.strip_ns n) ^ " is not a trait. It is " ^ t ^ ".")

let requires_non_class (p: Pos.t) (n: string) (t: string) =
  add p ((Utils.strip_ns n) ^ " is not a class. It is " ^ t ^ ".")

let abstract_body pos =
  add pos "This method shouldn't have a body"

let not_public_interface pos =
  add pos "Access type for interface method must be public"

let interface_with_member_variable pos =
  add pos "Interfaces cannot have member variables"

let interface_with_static_member_variable pos =
  add pos "Interfaces cannot have static variables"

let dangerous_method_name p =
  add p ("This is a dangerous method name, "^
         "if you want to define a constructor, use "^
         "__construct")

(*****************************************************************************)
(* Nast terminality *)
(*****************************************************************************)

let case_fallthrough p1 p2 =
  add_list [
  p1, ("This switch has a case that implicitly falls through and is "^
      "not annotated with // FALLTHROUGH");
  p2, "This case implicitly falls through"
]

let default_fallthrough p =
  add p
    ("This switch has a default case that implicitly falls "^
     "through and is not annotated with // FALLTHROUGH")

(*****************************************************************************)
(* Typing inheritance *)
(*****************************************************************************)

let visibility_extends vis pos parent_pos parent_vis =
  let msg1 = pos, "This member visibility is: " ^ vis in
  let msg2 = parent_pos, parent_vis ^ " was expected" in
  add_list [msg1; msg2]

let member_not_implemented member_name parent_pos pos defn_pos =
  let msg1 = pos, "This object doesn't implement the method "^member_name in
  let msg2 = parent_pos, "Which is required by this interface" in
  let msg3 = defn_pos, "As defined here" in
  add_list [msg1; msg2; msg3]

let override parent_pos parent_name pos name (error: error) =
  let msg1 = pos, ("This object is of type "^(strip_ns name)) in
  let msg2 = parent_pos,
    ("It is incompatible with this object of type "^(strip_ns parent_name)^
     "\nbecause some of their methods are incompatible."^
     "\nRead the following to see why:"
    ) in
  (* This is a cascading error message *)
  add_list (msg1 :: msg2 :: error)

let missing_constructor pos =
  add pos "The constructor is not implemented"

(*****************************************************************************)
(* Enum checking *)
(*****************************************************************************)

let typedef_trail_entry pos = pos, "Typedef definition comes from here"

let add_with_trail pos s trail =
  add_list ((pos, s) :: List.map typedef_trail_entry trail)

let enum_constant_type_bad pos ty trail =
  add_with_trail pos ("Enum constants must be an int or string, not " ^ ty)
    trail

let enum_type_bad pos ty trail =
  add_with_trail pos
    ("Enums must have int, string, or mixed type, not " ^ ty)
    trail

let enum_type_typedef_mixed pos =
  add pos "Can't use typedef that resolves to mixed in Enum"

(*****************************************************************************)
(* Printing *)
(*****************************************************************************)

let to_json (e : error) = Hh_json.(
  let elts = List.map (fun (p, w) ->
                        let line, scol, ecol = Pos.info_pos p in
                        JAssoc [ "descr", JString w;
                                 "path",  JString p.Pos.pos_file;
                                 "line",  JInt line;
                                 "start", JInt scol;
                                 "end",   JInt ecol
                               ]
                      ) e
  in
  JAssoc [ "message", JList elts ]
)

let to_string (e : error) : string =
  let buf = Buffer.create 50 in
  (match e with
  | [] -> assert false
  | (pos1, msg1) :: rest_of_error ->
      Buffer.add_string buf begin
        Printf.sprintf "%s\n%s\n" (Pos.string pos1) msg1
      end;
      List.iter begin fun (p, w) ->
        let msg = Printf.sprintf "%s\n%s\n" (Pos.string p) w in
        Buffer.add_string buf msg
      end rest_of_error
  );
  Buffer.contents buf

(*****************************************************************************)
(* Try if errors. *)
(*****************************************************************************)

let try_ f1 f2 =
  let error_list_copy = !error_list in
  error_list := [];
  let result = f1 () in
  let errors = !error_list in
  error_list := error_list_copy;
  match List.rev errors with
  | [] -> result
  | l :: _ -> f2 l

let try_with_error f1 f2 =
  try_ f1 (fun err -> add_error err; f2())

let try_add_err pos err f1 f2 =
  try_ f1 begin fun l ->
    add_list ((pos, err) :: l);
    f2()
  end

(*****************************************************************************)
(* Do. *)
(*****************************************************************************)

let do_ f =
  let error_list_copy = !error_list in
  error_list := [];
  let result = f () in
  let out_errors = !error_list in
  error_list := error_list_copy;
  List.rev out_errors, result

let try_when f ~when_ ~do_ =
  try_ f begin fun (error: error) ->
    if when_()
    then do_ error
    else add_error error
  end

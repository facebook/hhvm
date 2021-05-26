#![allow(non_upper_case_globals)]
// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//

use crate::token_kind::TokenKind;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use std::{borrow::Cow, cmp::Ordering};

// many errors are static strings, but not all of them
pub type Error = Cow<'static, str>;

#[derive(Debug, Clone, FromOcamlRep, ToOcamlRep, PartialEq, Eq)]
pub enum ErrorType {
    ParseError,
    RuntimeError,
}

#[derive(Debug, Clone, FromOcamlRep, ToOcamlRep, PartialEq, Eq)]
pub struct SyntaxError {
    pub child: Option<Box<SyntaxError>>,
    pub start_offset: usize,
    pub end_offset: usize,
    pub error_type: ErrorType,
    pub message: Error,
}

impl SyntaxError {
    pub fn make_with_child_and_type(
        child: Option<SyntaxError>,
        start_offset: usize,
        end_offset: usize,
        error_type: ErrorType,
        message: Error,
    ) -> Self {
        Self {
            child: child.map(|x| Box::new(x)),
            start_offset,
            end_offset,
            error_type,
            message,
        }
    }

    pub fn make(start_offset: usize, end_offset: usize, message: Error) -> Self {
        Self::make_with_child_and_type(
            None,
            start_offset,
            end_offset,
            ErrorType::ParseError,
            message,
        )
    }

    pub fn compare_offset(e1: &Self, e2: &Self) -> Ordering {
        (e1.start_offset, e1.end_offset).cmp(&(e2.start_offset, e2.end_offset))
    }

    pub fn equal_offset(e1: &Self, e2: &Self) -> bool {
        Self::compare_offset(e1, e2) == Ordering::Equal
    }

    pub fn weak_equal(e1: &Self, e2: &Self) -> bool {
        e1.start_offset == e2.start_offset
            && e1.end_offset == e2.end_offset
            && e1.message == e2.message
    }
}

// Lexical errors
pub const error0001: Error = Cow::Borrowed("A hexadecimal literal needs at least one digit.");
pub const error0002: Error = Cow::Borrowed("A binary literal needs at least one digit.");
pub const error0003: Error = Cow::Borrowed(concat!(
    "A floating point literal with an exponent needs at least ",
    "one digit in the exponent."
));
pub const error0006: Error = Cow::Borrowed("This character is invalid.");
pub const error0007: Error = Cow::Borrowed("This delimited comment is not terminated.");
pub const error0008: Error = Cow::Borrowed("A name is expected here.");
pub const error0010: Error = Cow::Borrowed("A single quote is expected here.");
pub const error0011: Error = Cow::Borrowed("A newline is expected here.");
pub const error0012: Error = Cow::Borrowed("This string literal is not terminated.");
pub const error0013: Error = Cow::Borrowed("This XHP body is not terminated.");
pub const error0014: Error = Cow::Borrowed("This XHP comment is not terminated.");

// Syntactic errors
pub const error1001: Error = Cow::Borrowed("A .php file must begin with `<?hh`.");
pub const error1003: Error = Cow::Borrowed("The `function` keyword is expected here.");
pub const error1004: Error = Cow::Borrowed("A name is expected here.");
pub const error1006: Error = Cow::Borrowed("A right brace `}` is expected here.");
pub const error1007: Error = Cow::Borrowed("A type specifier is expected here.");
pub const error1008: Error = Cow::Borrowed("A variable name is expected here.");
pub const error1010: Error = Cow::Borrowed("A semicolon `;` is expected here.");
pub const error1011: Error = Cow::Borrowed("A right parenthesis `)` is expected here.");
pub const error1013: Error = Cow::Borrowed("A closing angle bracket `>` is expected here.");
pub const error1014: Error =
    Cow::Borrowed("A closing angle bracket `>` or comma is expected here.");
pub const error1015: Error = Cow::Borrowed("An expression is expected here.");
pub const error1016: Error = Cow::Borrowed("An assignment is expected here.");
pub const error1017: Error = Cow::Borrowed("An XHP attribute value is expected here.");
pub const error1018: Error = Cow::Borrowed("The `while` keyword is expected here.");
pub const error1019: Error = Cow::Borrowed("A left parenthesis `(` is expected here.");
pub const error1020: Error = Cow::Borrowed("A colon `:` is expected here.");
pub const error1021: Error = Cow::Borrowed("An opening angle bracket `<` is expected here.");
// TODO: Remove this; redundant to 1009.
pub const error1022: Error =
    Cow::Borrowed("A right parenthesis `)` or comma `,` is expected here.");
pub const error1023: Error = Cow::Borrowed("An `as` keyword is expected here.");
pub const error1025: Error = Cow::Borrowed("A shape field name is expected here.");
pub const error1026: Error = Cow::Borrowed("An opening square bracket `[` is expected here.");
pub const error1028: Error = Cow::Borrowed("An arrow `=>` is expected here.");
pub const error1029: Error = Cow::Borrowed("A closing double angle bracket `>>` is expected here.");
pub const error1031: Error =
    Cow::Borrowed("A comma `,` or a closing square bracket `]` is expected here.");
pub const error1032: Error = Cow::Borrowed("A closing square bracket `]` is expected here.");
// TODO: Break this up according to classish type
pub const error1033: Error = Cow::Borrowed(concat!(
    "A class member, method, type, trait usage, trait require, ",
    "xhp attribute, xhp use, or xhp category is expected here."
));
pub const error1034: Error = Cow::Borrowed("A left brace `{` is expected here.");
pub const error1035: Error = Cow::Borrowed("The `class` keyword is expected here.");
pub const error1036: Error = Cow::Borrowed("An equals sign `=` is expected here.");
pub const error1037: Error = Cow::Borrowed("The `record` keyword is expected here.");
pub const error1038: Error = Cow::Borrowed("A semicolon `;` or a namespace body is expected here.");
pub const error1039: Error = Cow::Borrowed("A closing XHP tag is expected here.");
pub const error1041: Error = Cow::Borrowed("A function body or a semicolon `;` is expected here.");
pub const error1044: Error = Cow::Borrowed("A name or `__construct` keyword is expected here.");
pub const error1045: Error =
    Cow::Borrowed("An `extends` or `implements` keyword is expected here.");
pub const error1046: Error = Cow::Borrowed("A lambda arrow `==>` is expected here.");
pub const error1047: Error = Cow::Borrowed("A scope resolution operator `::` is expected here.");
pub const error1048: Error = Cow::Borrowed("A name, variable name or `class` is expected here.");
pub const error1050: Error = Cow::Borrowed("A name or variable name is expected here.");
pub const error1051: Error =
    Cow::Borrowed("The `required` or `lateinit` keyword is expected here.");
pub const error1052: Error =
    Cow::Borrowed("An XHP category name beginning with a `%` is expected here.");
pub const error1053: Error = Cow::Borrowed("An XHP name or category name is expected here.");
pub const error1054: Error = Cow::Borrowed("A comma `,` is expected here.");
pub const error1055: Error = Cow::Borrowed(concat!(
    "A fallthrough directive can only appear at the end of",
    " a `switch` section."
));
// TODO(20052790): use the specific token's text in the message body.
pub const error1056: Error =
    Cow::Borrowed("This token is not valid as part of a function declaration.");
pub fn error1057(text: &str) -> Error {
    // TODO (kasper): T52404885: why does removing to_string() here segfaults
    Cow::Owned(format!(
        "Encountered unexpected token `{}`.",
        text.to_string()
    ))
}
pub fn uppercase_kw(text: &str) -> Error {
    Cow::Owned(format!(
        "Keyword `{}` must be written in lowercase",
        text.to_string()
    ))
}
pub fn error1058(received: &str, required: &str) -> Error {
    Cow::Owned(format!(
        "Encountered unexpected token `{}`. Did you mean `{}`?",
        received.to_string(),
        required.to_string()
    ))
}
pub fn error1059(terminator: TokenKind) -> Error {
    Cow::Owned(format!(
        "An `{}` is required when using alternate block syntax.",
        terminator.to_string().to_string(),
    ))
}
pub fn error1060(extension: &str) -> Error {
    let kind = if extension == "hack" {
        "strict"
    } else {
        "partial"
    };
    Cow::Owned(format!(
        "Leading markup and `<?hh` are not permitted in `.{}` files, which are always `{}`.",
        extension.to_string(),
        kind.to_string()
    ))
}
pub const error1063: Error = Cow::Borrowed("Expected matching separator here.");
pub const error1064: Error = Cow::Borrowed("XHP children declarations are no longer supported.");
pub const error1065: Error = Cow::Borrowed("A backtick ``` is expected here.");
pub const error2001: Error = Cow::Borrowed("A type annotation is required in `strict` mode.");
pub const error2003: Error =
    Cow::Borrowed("A `case` statement may only appear directly inside a `switch`.");
pub const error2004: Error =
    Cow::Borrowed("A `default` statement may only appear directly inside a `switch`.");
pub const error2005: Error =
    Cow::Borrowed("A `break` statement may only appear inside a `switch` or loop.");
pub const error2006: Error = Cow::Borrowed("A `continue` statement may only appear inside a loop.");
pub const error2007: Error =
    Cow::Borrowed("A `try` statement requires a `catch` or a `finally` clause.");
pub const error2008: Error = Cow::Borrowed(concat!(
    "The first statement inside a `switch` statement must ",
    "be a `case` or `default` label statement."
));
pub fn error2009(class_name: &str, method_name: &str) -> Error {
    Cow::Owned(format!(
        "Constructor `{}::{}()` cannot be static",
        class_name.to_string(),
        method_name.to_string(),
    ))
}
pub const error2010: Error = Cow::Borrowed(concat!(
    "Parameters cannot have visibility modifiers (except in ",
    "parameter lists of constructors)."
));
pub const error2014: Error = Cow::Borrowed("An abstract method cannot have a method body.");
pub const error2015: Error = Cow::Borrowed("A method must have a body or be marked `abstract`.");

pub fn error2016(class_name: &str, method_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot declare abstract method `{}::{}` `private`",
        class_name.to_string(),
        method_name.to_string(),
    ))
}
pub const error2018: Error =
    Cow::Borrowed("A constructor cannot have a non-`void` type annotation.");
pub fn error2019(class_name: &str, method_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot declare abstract method `{}::{}` `final`",
        class_name.to_string(),
        method_name.to_string(),
    ))
}
pub const error2020: Error = Cow::Borrowed(concat!(
    "Use of the `{}` subscript operator is deprecated; ",
    " use `[]` instead."
));
pub const error2021: Error = Cow::Borrowed(concat!(
    "A variadic parameter `...` may only appear at the end of ",
    "a parameter list."
));
pub const error2023: Error =
    Cow::Borrowed("Abstract constructors cannot have parameters with visibility modifiers");
pub const error2024: Error =
    Cow::Borrowed("Traits or interfaces cannot have parameters with visibility modifiers");
pub const error2022: Error =
    Cow::Borrowed("A variadic parameter `...` may not be followed by a comma.");
pub fn error2025(class_name: &str, prop_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot redeclare `{}::{}`",
        class_name.to_string(),
        prop_name.to_string(),
    ))
}
pub const error2029: Error = Cow::Borrowed("Only traits and interfaces may use `require extends`.");
pub const error2030: Error = Cow::Borrowed("Only traits may use `require implements`.");
pub const error2032: Error = Cow::Borrowed("The array type is not allowed in `strict` mode.");
pub const error2033: Error = Cow::Borrowed(concat!(
    "The splat operator `...` for unpacking variadic arguments ",
    "may only appear at the **end** of an argument list."
));
pub const error2034: Error = Cow::Borrowed(concat!(
    "A type alias declaration cannot both use `type` and have a ",
    "constraint. Did you mean `newtype`?"
));
pub const error2035: Error = Cow::Borrowed("Only classes may implement interfaces.");
pub const error2036: Error = Cow::Borrowed(concat!(
    "Only interfaces and classes may extend other interfaces and ",
    "classes."
));
pub const error2037: Error = Cow::Borrowed("A class may extend at most **one** other class.");
pub fn error2038(constructor_name: &str) -> Error {
    Cow::Owned(format!(
        concat!(
            "A constructor initializing an object must be passed a (possibly empty) ",
            "list of arguments. Did you mean `new {}()`?",
        ),
        constructor_name.to_string(),
    ))
}
pub const error2040: Error = Cow::Borrowed(concat!(
    "Invalid use of `list(...)`. A list expression may only be ",
    "used as the left side of a simple assignment, the value clause of a ",
    "`foreach` loop, or a list item nested inside another list expression."
));
pub const error2041: Error = Cow::Borrowed(concat!(
    "Unexpected method body: interfaces may contain only",
    " method signatures, and **not** method implementations."
));
pub const error2042: Error = Cow::Borrowed("Only classes may be declared `abstract`.");
pub fn error2046(method_type: &str) -> Error {
    Cow::Owned(format!(
        "`async` cannot be used on {}. Use an `Awaitable<...>` return type instead.",
        method_type.to_string(),
    ))
}

pub const error2048: Error = Cow::Borrowed("Expected group `use` prefix to end with `\\`");
pub const error2049: Error =
    Cow::Borrowed("A namespace `use` clause may not specify the kind here.");
pub const error2050: Error =
    Cow::Borrowed("A concrete constant declaration must have an initializer.");
pub const error2051: Error =
    Cow::Borrowed("An abstract constant declaration must not have an initializer.");
pub const error2052: Error = Cow::Borrowed(concat!(
    "Cannot mix bracketed namespace declarations with ",
    "unbracketed namespace declarations"
));
pub const error2053: Error = Cow::Borrowed(concat!(
    "Use of `var` as synonym for `public` in declaration disallowed in Hack. ",
    "Use `public` instead."
));
pub const error2054: Error = Cow::Borrowed(concat!(
    "Method declarations require a visibility modifier ",
    "such as `public`, `private` or `protected`."
));
pub const error2055: Error = Cow::Borrowed("At least one enumerated item is expected.");
pub const error2056: Error = Cow::Borrowed("First unbracketed namespace occurrence here");
pub const error2057: Error = Cow::Borrowed("First bracketed namespace occurrence here");
pub const invalid_shape_field_name: Error =
    Cow::Borrowed("Shape field name must be a nonempty single-quoted string or a class constant");
pub const shape_field_int_like_string: Error =
    Cow::Borrowed("Shape field name must not be an int-like string (i.e. \"123\")");
pub const error2061: Error = Cow::Borrowed(concat!(
    "Non-static instance variables are not allowed in abstract ",
    "final classes."
));
pub const error2062: Error =
    Cow::Borrowed("Non-static methods are not allowed in `abstract final` classes.");
pub const error2063: Error = Cow::Borrowed("Expected integer or string literal.");
pub const error2065: Error =
    Cow::Borrowed("A variadic parameter `...` must not have a default value.");
// This was typing error 4077.
pub const error2066: Error = Cow::Borrowed(concat!(
    "A previous parameter has a default value. Remove all the ",
    "default values for the preceding parameters, or add a default value to ",
    "this one."
));
pub const error2068: Error = Cow::Borrowed("`hh` blocks and `php` blocks cannot be mixed.");
pub fn invalid_integer_digit(int_kind: ocaml_helper::IntKind) -> Error {
    Cow::Owned(format!("Invalid digit for {} integers", int_kind))
}
pub const prefixed_invalid_string_kind: Error =
    Cow::Borrowed("Only double-quoted strings may be prefixed.");
pub const illegal_interpolated_brace_with_embedded_dollar_expression: Error =
    Cow::Borrowed(concat!(
        "The only legal expressions inside a `{$...}`-expression embedded in a string are ",
        "variables, function calls, subscript expressions, and member access expressions"
    ));
pub const expected_dotdotdot: Error = Cow::Borrowed("`...` is expected here.");
pub const invalid_foreach_element: Error = Cow::Borrowed(
    "An arrow `=>` or right parenthesis `)` \
     is expected here.",
);
pub const inline_function_def: Error =
    Cow::Borrowed("Inline function definitions are not supported in Hack");
pub const decl_outside_global_scope: Error =
    Cow::Borrowed("Declarations are not supported outside global scope");
pub const type_keyword: Error = Cow::Borrowed("The `type` keyword is expected here.");
pub const expected_simple_offset_expression: Error =
    Cow::Borrowed("A simple offset expression is expected here");
pub const expected_user_attribute: Error = Cow::Borrowed("A user attribute is expected here.");
pub const expected_as_or_insteadof: Error =
    Cow::Borrowed("The `as` keyword or the `insteadof` keyword is expected here.");
pub const missing_double_quote: Error = /* error0010 analogue */
    Cow::Borrowed("A double quote is expected here.");
pub const instanceof_disabled: Error = Cow::Borrowed(
    "The `instanceof` operator is not supported in Hack; use the `is` operator or `is_a()`",
);
pub const abstract_instance_property: Error =
    Cow::Borrowed("Instance property may not be abstract.");
pub const memoize_lsb_on_non_static: Error =
    Cow::Borrowed("`<<__MemoizeLSB>>` can only be applied to static methods");
pub const memoize_lsb_on_non_method: Error =
    Cow::Borrowed("`<<__MemoizeLSB>>` can only be applied to methods");
pub const expression_as_attribute_arguments: Error =
    Cow::Borrowed("Attribute arguments must be literals");
pub const instanceof_invalid_scope_resolution: Error = Cow::Borrowed(concat!(
    "A scope resolution `::` on the right side of an ",
    "`instanceof` operator must start with a class name, `self`, `parent`, or `static`, and end with ",
    "a variable",
));
pub const instanceof_memberselection_inside_scoperesolution: Error = Cow::Borrowed(concat!(
    "A scope resolution `::` on the right ",
    "side of an instanceof operator cannot contain a member selection `->`",
));
pub const instanceof_missing_subscript_index: Error = Cow::Borrowed(concat!(
    "A subscript expression `[]` on the right side of an ",
    "instanceof operator must have an index",
));
pub fn new_unknown_node(msg: &str) -> Error {
    Cow::Owned(format!(
        "`new` requires a class name or local variable, but got: `{}`",
        msg.to_string(),
    ))
}
pub const invalid_async_return_hint: Error =
    Cow::Borrowed("`async` functions must have an `Awaitable` return type.");
pub const invalid_awaitable_arity: Error =
    Cow::Borrowed("`Awaitable<_>` takes exactly one type argument.");
pub const invalid_await_use: Error = Cow::Borrowed("`await` cannot be used as an expression");
pub const toplevel_await_use: Error =
    Cow::Borrowed("`await` cannot be used in a toplevel statement");
pub const invalid_constructor_method_call: Error = Cow::Borrowed(
    "Method call following immediate constructor call requires parentheses around constructor call.",
);
pub const invalid_scope_resolution_qualifier: Error =
    Cow::Borrowed("Only classnames and variables are allowed before `::`.");
pub const invalid_variable_name: Error = Cow::Borrowed(
    "A valid variable name starts with a letter or underscore, followed by any number of letters, numbers, or underscores",
);
pub const invalid_yield: Error =
    Cow::Borrowed("`yield` can only appear as a statement or on the right of an assignment");
pub const invalid_class_in_collection_initializer: Error =
    Cow::Borrowed("Cannot use collection initialization for non-collection class.");
pub const invalid_brace_kind_in_collection_initializer: Error = Cow::Borrowed(
    "Initializers of `vec`, `dict` and `keyset` should use `[...]` instead of `{...}`.",
);
pub fn invalid_value_initializer(name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use value initializer for `{}`. It requires `key => value`.",
        name.to_string(),
    ))
}
pub fn invalid_key_value_initializer(name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use key value initializer for `{}`. It does not allow keys.",
        name.to_string(),
    ))
}
pub const nested_ternary: Error = Cow::Borrowed(
    "Nested ternary expressions inside ternary expressions are ambiguous. Please add parentheses",
);
pub const alternate_control_flow: Error =
    Cow::Borrowed("Alternate control flow syntax is not allowed in Hack files");
pub const execution_operator: Error =
    Cow::Borrowed("The execution operator is not allowed in Hack files");
pub const non_re_prefix: Error = Cow::Borrowed("Only `re`-prefixed strings allowed.");
pub const collection_intrinsic_generic: Error =
    Cow::Borrowed("Cannot initialize collection builtins with type parameters");
pub const collection_intrinsic_many_typeargs: Error =
    Cow::Borrowed("Collection expression must have less than three type arguments");
pub const invalid_hack_mode: Error =
    Cow::Borrowed("Incorrect comment; possible values include `strict`, `partial`, or empty");
pub const pair_initializer_needed: Error = Cow::Borrowed("Initializer needed for Pair object");
pub const pair_initializer_arity: Error =
    Cow::Borrowed("Pair objects must have exactly 2 elements");
pub const toplevel_statements: Error =
    Cow::Borrowed("Toplevel statements are not allowed. Use `__EntryPoint` attribute instead");
pub const invalid_reified: Error =
    Cow::Borrowed("`reify` keyword can only appear at function or class type parameter position");
pub fn reified_in_invalid_classish(s: &str) -> Error {
    Cow::Owned(format!(
        "Invalid to use a reified type within {}'s type parameters",
        s.to_string(),
    ))
}
pub const shadowing_reified: Error = Cow::Borrowed("You may not shadow a reified parameter");
pub const static_property_in_reified_class: Error =
    Cow::Borrowed("You may not use static properties in a class with reified type parameters");
pub const cls_reified_generic_in_static_method: Error =
    Cow::Borrowed("You may not use reified generics of the class in a static method");
pub const static_method_reified_obj_creation: Error = Cow::Borrowed(
    "You may not use object creation for potentially reified `self` or `parent` from a static method",
);
pub const non_invariant_reified_generic: Error =
    Cow::Borrowed("Reified generics cannot be covariant or contravariant");
pub const no_generics_on_constructors: Error = Cow::Borrowed(
    "Generic type parameters are not allowed on constructors. Consider adding a type parameter to the class",
);
pub const no_type_parameters_on_dynamic_method_calls: Error =
    Cow::Borrowed("Generics type parameters are disallowed on dynamic method calls");
pub const dollar_unary: Error =
    Cow::Borrowed("The dollar sign `$` cannot be used as a unary operator");
pub const type_alias_to_type_constant: Error =
    Cow::Borrowed("Type aliases to type constants are not supported");
pub const interface_with_memoize: Error =
    Cow::Borrowed("`__Memoize` is not allowed on interface methods");
pub const nested_concurrent_blocks: Error = Cow::Borrowed("`concurrent` blocks cannot be nested.");
pub const fewer_than_two_statements_in_concurrent_block: Error = Cow::Borrowed(concat!(
    "Expected 2 or more statements in concurrent block. `concurrent` wrapping ",
    "nothing or a single statement is not useful or already implied.",
));
pub const invalid_syntax_concurrent_block: Error = Cow::Borrowed(concat!(
    "`concurrent` block must contain a compound statement of two or ",
    "more expression statements, IE concurrent `{ <expr>; <expr>; }`.",
));
pub const statement_without_await_in_concurrent_block: Error =
    Cow::Borrowed("Statement without an `await` in a concurrent block");
pub const concurrent_is_disabled: Error = Cow::Borrowed("`concurrent` is disabled");
pub const invalid_await_position: Error = Cow::Borrowed(concat!(
    "`await` cannot be used as an expression in this ",
    "location because it's conditionally executed.",
));
pub const invalid_await_position_dependent: Error = Cow::Borrowed(concat!(
    "`await` cannot be used as an expression inside another await expression. ",
    "Pull the inner `await` out into its own statement.",
));
pub const tparams_in_tconst: Error =
    Cow::Borrowed("Type parameters are not allowed on class type constants");
pub const targs_not_allowed: Error =
    Cow::Borrowed("Type arguments are not allowed in this position");
pub const reified_attribute: Error = Cow::Borrowed(
    "`__Reified` and `__HasReifiedParent` attributes may not be provided by the user",
);
pub const lval_as_expression: Error = Cow::Borrowed(
    "Assignments can no longer be used as expressions. Pull the assignment out into a separate statement.",
);
pub fn elt_abstract_private(elt: &str) -> Error {
    Cow::Owned(format!(
        "Cannot declare abstract {} `private`.",
        elt.to_string(),
    ))
}
pub const only_soft_allowed: Error = Cow::Borrowed("Only the `__Soft` attribute is allowed here.");
pub const soft_no_arguments: Error =
    Cow::Borrowed("The `__Soft` attribute does not take arguments.");
pub const no_legacy_soft_typehints: Error = Cow::Borrowed(
    "The `@` syntax for soft typehints is not allowed. Use the `__Soft` attribute instead.",
);
pub const outside_dollar_str_interp: Error =
    Cow::Borrowed("The `${x}` syntax is disallowed in Hack. Use `{$x}` instead.");
pub const no_const_interfaces_traits_enums: Error =
    Cow::Borrowed("Interfaces, traits and enums may not be declared `__Const`");
pub const no_const_late_init_props: Error =
    Cow::Borrowed("`__Const` properties may not also be `__LateInit`");
pub const no_const_static_props: Error = Cow::Borrowed("Static properties may not be `__Const`");
pub const no_const_abstract_final_class: Error =
    Cow::Borrowed("Cannot apply `__Const` attribute to an abstract final class");
pub const no_legacy_attribute_syntax: Error = Cow::Borrowed(
    "The `<<...>>` syntax for user attributes is not allowed. Use the `@` syntax instead.",
);
pub const no_silence: Error = Cow::Borrowed("The error suppression operator `@` is not allowed");
pub const const_mutation: Error = Cow::Borrowed("Cannot mutate a class constant");
pub const no_attributes_on_variadic_parameter: Error =
    Cow::Borrowed("Attributes on variadic parameters are not allowed");
pub const invalid_constant_initializer: Error =
    Cow::Borrowed("Expected constant expression for initializer");
pub const parent_static_prop_decl: Error =
    Cow::Borrowed("Cannot use `static` or `parent::class` in property declaration");
pub fn error2070(open_tag: &str, close_tag: &str) -> Error {
    Cow::Owned(format!(
        "XHP: mismatched tag: `{}` not the same as `{}`",
        close_tag.to_string(),
        open_tag.to_string(),
    ))
}
pub fn error2071(s: &str) -> Error {
    Cow::Owned(format!("Decimal number is too big: `{}`", s.to_string(),))
}
pub fn error2072(s: &str) -> Error {
    Cow::Owned(format!(
        "Hexadecimal number is too big: `{}`",
        s.to_string(),
    ))
}
pub const error2073: Error = Cow::Borrowed(concat!(
    "A variadic parameter `...` cannot have a modifier ",
    "that changes the calling convention, like `inout`.",
));
pub fn error2074(call_modifier: &str) -> Error {
    Cow::Owned(format!(
        "An `{}` parameter must not have a default value.",
        call_modifier.to_string(),
    ))
}
pub const error2077: Error = Cow::Borrowed("Cannot use empty list");
pub fn not_allowed_in_write(what: &str) -> Error {
    Cow::Owned(format!(
        "{} is not allowed in write context",
        what.to_string(),
    ))
}
pub const reassign_this: Error = Cow::Borrowed("Cannot re-assign `$this`");
pub const enum_elem_name_is_class: Error = Cow::Borrowed("Enum element cannot be named `class`");
pub const property_requires_visibility: Error = Cow::Borrowed(concat!(
    "Property declarations require a visibility modifier ",
    "such as `public`, `private` or `protected`.",
));
pub const abstract_prop_init: Error =
    Cow::Borrowed("An `abstract` property must not have an initializer.");
pub const const_static_prop_init: Error =
    Cow::Borrowed("A `const static` property must have an initializer.");
pub fn namespace_name_is_already_in_use(name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use namespace `{}` as `{}` because the name is already in use",
        name.to_string(),
        short_name.to_string()
    ))
}
pub const strict_namespace_hh: Error = Cow::Borrowed(concat!(
    "To use strict Hack, place `// strict` after the open tag. ",
    "If it's already there, remove this line. ",
    "Hack is strict already.",
));
pub fn name_is_already_in_use_hh(line_num: isize, name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use `{}` as `{}` because the name was explicitly used earlier via a `use` statement on line {}",
        name.to_string(),
        short_name.to_string(),
        line_num.to_string(),
    ))
}
pub fn name_is_already_in_use_implicit_hh(line_num: isize, name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        concat!(
            "Cannot use `{}` as `{}` because the name was implicitly used on line {}",
            "; implicit use of names from the HH namespace can be suppressed by adding an explicit",
            " `use` statement earlier in the current namespace block",
        ),
        name.to_string(),
        short_name.to_string(),
        line_num.to_string(),
    ))
}
pub fn name_is_already_in_use_php(name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use `{}` as `{}` because the name is already in use",
        name.to_string(),
        short_name.to_string(),
    ))
}
pub const original_definition: Error = Cow::Borrowed("Original definition");
pub fn function_name_is_already_in_use(name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use function `{}` as `{}` because the name is already in use",
        name.to_string(),
        short_name.to_string(),
    ))
}
pub fn const_name_is_already_in_use(name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use const `{}` as `{}` because the name is already in use",
        name.to_string(),
        short_name.to_string(),
    ))
}
pub fn type_name_is_already_in_use(name: &str, short_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use type `{}` as `{}` because the name is already in use",
        name.to_string(),
        short_name.to_string(),
    ))
}
pub const namespace_decl_first_statement: Error = Cow::Borrowed(
    "Namespace declaration statement has to be the very first statement in the script",
);
pub const code_outside_namespace: Error =
    Cow::Borrowed("No code may exist outside of namespace {}");
pub const global_in_const_decl: Error =
    Cow::Borrowed("Cannot have globals in constant declaration");
pub const parent_static_const_decl: Error =
    Cow::Borrowed("Cannot use `static` or `parent::class` in constant declaration");
pub const no_async_before_lambda_body: Error =
    Cow::Borrowed("Don't use `() ==> async { ... }`. Instead, use: `async () ==> { ... }`");
pub fn invalid_number_of_args(name: &str, n: usize) -> Error {
    Cow::Owned(format!(
        "Method `{}` must take exactly {} arguments",
        name.to_string(),
        n.to_string(),
    ))
}
pub fn invalid_inout_args(name: &str) -> Error {
    Cow::Owned(format!(
        "Method `{}` cannot take inout arguments",
        name.to_string(),
    ))
}
pub fn redeclaration_error(name: &str) -> Error {
    Cow::Owned(format!("Cannot redeclare `{}`", name.to_string(),))
}
pub fn declared_name_is_already_in_use_implicit_hh(
    line_num: usize,
    name: &str,
    _short_name: &str,
) -> Error {
    Cow::Owned(format!(
        concat!(
            "Cannot declare `{}` because the name was implicitly used on line {}; ",
            "implicit use of names from the HH namespace can be suppressed by adding an explicit ",
            "`use` statement earlier in the current namespace block",
        ),
        name.to_string(),
        line_num.to_string(),
    ))
}
pub fn declared_name_is_already_in_use(line_num: usize, name: &str, _short_name: &str) -> Error {
    Cow::Owned(format!(
        concat!(
            "Cannot declare `{}` because the name was explicitly used earlier via a `use` ",
            "statement on line {}",
        ),
        name.to_string(),
        line_num.to_string(),
    ))
}
pub const sealed_val_not_classname: Error =
    Cow::Borrowed("Values in sealed whitelist must be classname constants.");
pub const sealed_qualifier_invalid: Error =
    Cow::Borrowed("`__Sealed` can only be used with named types, e.g. `Foo::class`");
pub const list_must_be_lvar: Error =
    Cow::Borrowed("`list()` can only be used as an lvar. Did you mean to use `tuple()`?");
pub const async_not_last: Error =
    Cow::Borrowed("The `async` modifier must be directly before the `function` keyword.");
pub const using_st_function_scoped_top_level: Error = Cow::Borrowed(concat!(
    "Using statement in function scoped form may only be used at the top ",
    "level of a function or a method",
));
pub const double_variadic: Error = Cow::Borrowed("Parameter redundantly marked as variadic `...`.");
pub fn conflicting_trait_require_clauses(name: &str) -> Error {
    Cow::Owned(format!(
        "Conflicting requirements for `{}`",
        name.to_string(),
    ))
}
pub const shape_type_ellipsis_without_trailing_comma: Error =
    Cow::Borrowed("A comma is required before the `...` in a shape type");
pub const yield_in_magic_methods: Error =
    Cow::Borrowed("`yield` is not allowed in constructors or magic methods");
pub const yield_outside_function: Error =
    Cow::Borrowed("`yield` can only be used inside a function");
pub const coloncolonclass_on_dynamic: Error =
    Cow::Borrowed("Dynamic class names are not allowed in compile-time `::class` fetch");
pub const this_in_static: Error =
    Cow::Borrowed("Don't use `$this` in a static method, use `static::` instead");
pub fn async_magic_method(name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot declare constructors and magic methods like `{}` as `async`",
        name.to_string(),
    ))
}
pub fn unsupported_magic_method(name: &str) -> Error {
    Cow::Owned(format!(
        "Magic `{}` methods are no longer supported",
        name.to_string(),
    ))
}
pub fn reserved_keyword_as_class_name(class_name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use `{}` as class name as it is reserved",
        class_name.to_string(),
    ))
}
pub const xhp_class_multiple_category_decls: Error =
    Cow::Borrowed("An XHP class cannot have multiple category declarations");
pub const xhp_class_multiple_children_decls: Error =
    Cow::Borrowed("An XHP class cannot have multiple children declarations");
pub const inout_param_in_generator: Error =
    Cow::Borrowed("Parameters may not be marked `inout` on generators");
pub const inout_param_in_async_generator: Error =
    Cow::Borrowed("Parameters may not be marked `inout` on `async` generators");
pub const inout_param_in_async: Error =
    Cow::Borrowed("Parameters may not be marked `inout` on `async` functions");
pub const inout_param_in_construct: Error =
    Cow::Borrowed("Parameters may not be marked `inout` on constructors");
pub const fun_arg_inout_set: Error =
    Cow::Borrowed("You cannot set an `inout` decorated argument while calling a function");
pub const fun_arg_inout_const: Error = Cow::Borrowed("You cannot decorate a constant as `inout`");
pub const fun_arg_invalid_arg: Error =
    Cow::Borrowed("You cannot decorate this argument as `inout`");
pub const fun_arg_inout_containers: Error = Cow::Borrowed(concat!(
    "Parameters marked `inout` must be contained in locals, vecs, dicts, keysets,",
    " and arrays",
));
pub const memoize_with_inout: Error =
    Cow::Borrowed("`<<__Memoize>>` cannot be used on functions with `inout` parameters");
pub const method_calls_on_xhp_attributes: Error =
    Cow::Borrowed("Method calls are not allowed on XHP attributes");
pub const method_calls_on_xhp_expression: Error =
    Cow::Borrowed("Please add parentheses around the XHP component");
pub fn class_with_abstract_method(name: &str) -> Error {
    Cow::Owned(format!(
        concat!(
            "Class `{}` contains an abstract method and must ",
            "therefore be declared `abstract`",
        ),
        name.to_string(),
    ))
}
pub const interface_has_private_method: Error =
    Cow::Borrowed("Interface methods must be `public` or `protected`");
pub fn redeclaration_of_function(name: &str, loc: &str) -> Error {
    Cow::Owned(format!(
        "Cannot redeclare `{}()` (previously declared in {})",
        name.to_string(),
        loc.to_string()
    ))
}
pub fn redeclaration_of_method(name: &str) -> Error {
    Cow::Owned(format!("Redeclared method `{}`", name.to_string(),))
}
pub fn self_or_parent_colon_colon_class_outside_of_class(name: &str) -> Error {
    Cow::Owned(format!(
        "Cannot access `{}::class` when no class scope is active",
        name.to_string(),
    ))
}
pub fn invalid_is_as_expression_hint(n: &str, hint: &str) -> Error {
    Cow::Owned(format!(
        "`{}` typehints cannot be used with `{}` expressions",
        hint.to_string(),
        n.to_string(),
    ))
}
pub const elvis_operator_space: Error = Cow::Borrowed("An Elvis operator `?:` is expected here.");
pub fn clone_takes_no_arguments(class_name: &str, method_name: &str) -> Error {
    Cow::Owned(format!(
        "Method `{}::{}` cannot accept any arguments",
        class_name.to_string(),
        method_name.to_string(),
    ))
}
pub fn clone_cannot_be_static(class_name: &str, method_name: &str) -> Error {
    Cow::Owned(format!(
        "Clone method `{}::{}()` cannot be static",
        class_name.to_string(),
        method_name.to_string(),
    ))
}
pub const namespace_not_a_classname: Error =
    Cow::Borrowed("Namespace cannot be used as a classname");
pub const for_with_as_expression: Error =
    Cow::Borrowed("For loops can not use `as` expressions. Did you mean `foreach`?");
pub const sealed_final: Error = Cow::Borrowed("Classes cannot be both `final` and `sealed`.");
pub const interface_implements: Error =
    Cow::Borrowed("Interfaces may not implement other interfaces or classes");
pub const memoize_on_lambda: Error =
    Cow::Borrowed("`<<__Memoize>>` attribute is not allowed on lambdas or anonymous functions.");
pub fn declared_final(elt: &str) -> Error {
    Cow::Owned(format!("{} cannot be declared `final`.", elt.to_string(),))
}
pub fn invalid_xhp_classish(elt: &str) -> Error {
    Cow::Owned(format!("{} are not valid xhp classes.", elt.to_string(),))
}
pub const empty_method_name: Error = Cow::Borrowed("Expected a method name");
pub fn lowering_parsing_error(text: &str, syntax: &str) -> Error {
    Cow::Owned(format!(
        "Encountered unexpected text `{}`, was expecting a {}.",
        text.to_string(),
        syntax.to_string(),
    ))
}
pub const xhp_class_attribute_type_constant: Error =
    Cow::Borrowed("Type constants are not allowed on xhp class attributes");
pub const globals_disallowed: Error =
    Cow::Borrowed("`$GLOBALS` variable is removed from the language. Use HH\\global functions");
pub const invalid_this: Error =
    Cow::Borrowed("`$this` cannot be used in functions and static methods");
pub const cannot_unset_this: Error = Cow::Borrowed("`$this` cannot be unset");
pub const invalid_await_position_pipe: Error =
    Cow::Borrowed("`await` cannot be used as an expression right of a pipe operator.");
pub fn invalid_modifier_for_declaration(decl: &str, modifier: &str) -> Error {
    Cow::Owned(format!(
        "{} cannot be declared `{}`",
        decl.to_string(),
        modifier.to_string(),
    ))
}
pub fn duplicate_modifiers_for_declaration(decl: &str) -> Error {
    Cow::Owned(format!(
        "{} cannot have duplicate modifiers",
        decl.to_string(),
    ))
}
pub fn multiple_visibility_modifiers_for_declaration(decl: &str) -> Error {
    Cow::Owned(format!(
        "{} cannot have multiple visibility modifiers",
        decl.to_string(),
    ))
}
pub const break_continue_n_not_supported: Error =
    Cow::Borrowed("`break`/`continue N` operators are not supported.");
pub fn invalid_typehint_alias(alias: &str, hint: &str) -> Error {
    Cow::Owned(format!(
        "Invalid type hint `{}`. Use `{}` instead",
        alias.to_string(),
        hint.to_string(),
    ))
}

pub const function_pointer_bad_recv: Error = Cow::Borrowed(concat!(
    "Function pointers `<>` can only be created with toplevel functions and explicitly named static methods. ",
    "Use lambdas `(...) ==> {...}` for other cases."
));

pub const local_variable_with_type: Error =
    Cow::Borrowed("Local variables cannot have type annotations in Hack.");

pub const empty_expression_illegal: Error =
    Cow::Borrowed("The `empty()` expression has been removed from Hack.");

pub const empty_switch_cases: Error =
    Cow::Borrowed("`switch` statements need to have at least one `case` or a `default` block");

pub const preceding_backslash: Error = Cow::Borrowed("Unnecessary preceding backslash");

pub fn multiple_entrypoints(loc: &str) -> Error {
    Cow::Owned(format!(
        "Only one `__EntryPoint` annotation is permitted per file (previous `__EntryPoint` annotation in {})",
        loc.to_string()
    ))
}

pub fn cannot_use_feature(feature: &str) -> Error {
    Cow::Owned(format!(
        "Cannot use unstable feature: `{}`",
        feature.to_string()
    ))
}

pub fn cannot_enable_unstable_feature(message: &str) -> Error {
    Cow::Owned(format!(
        "Cannot enable unstable feature: {}",
        message.to_string()
    ))
}

pub fn invalid_use_of_enable_unstable_feature(message: &str) -> Error {
    Cow::Owned(format!(
        "This is an invalid use of `__EnableUnstableFeatures` because {}",
        message.to_string()
    ))
}

pub const splice_outside_et: Error =
    Cow::Borrowed("Splicing can only occur inside expression tree literals (between backticks)");

pub const invalid_enum_class_enumerator: Error = Cow::Borrowed("Invalid enum class constant");

pub fn fun_disabled(func_name: &str) -> Error {
    Cow::Owned(format!(
        "`fun()` is disabled; switch to first-class references like `{}<>`",
        func_name
            .trim_end_matches('\'')
            .trim_start_matches('\'')
            .to_string()
    ))
}

pub const fun_requires_const_string: Error = Cow::Borrowed("Constant string expected in fun()");

pub const class_meth_disabled: Error =
    Cow::Borrowed("`class_meth()` is disabled; switch to first-class references like `C::bar<>`");

pub fn ctx_var_invalid_parameter(param_name: &str) -> Error {
    Cow::Owned(format!(
        "Could not find parameter {} for dependent context",
        param_name.to_string()
    ))
}

pub fn ctx_var_missing_type_hint(param_name: &str) -> Error {
    Cow::Owned(format!(
        "Parameter {} used for dependent context must have a type hint",
        param_name.to_string()
    ))
}

pub fn ctx_var_variadic(param_name: &str) -> Error {
    Cow::Owned(format!(
        "Parameter {} used for dependent context cannot be variadic",
        param_name.to_string()
    ))
}

pub fn ctx_var_invalid_type_hint(param_name: &str) -> Error {
    Cow::Owned(format!(
        "Type hint for parameter {} used for dependent context must be a class or a generic",
        param_name.to_string()
    ))
}

pub fn ctx_fun_invalid_type_hint(param_name: &str) -> Error {
    Cow::Owned(format!(
        "Type hint for parameter {} used for contextful function must be a function type hint whose context is exactly `[_]`, e.g. `(function (ts)[_]: t)`",
        param_name.to_string()
    ))
}

pub fn effect_polymorphic_memoized(kind: &str) -> Error {
    Cow::Owned(format!(
        "This {} cannot be memoized because it has a polymorphic context",
        kind
    ))
}

pub fn effect_policied_memoized(kind: &str) -> Error {
    Cow::Owned(format!(
        "This {} can only be memoized using __PolicyShardedMemoize because it has policied context",
        kind
    ))
}

pub const lambda_effect_polymorphic: Error =
    Cow::Borrowed("A lambda cannot have polymorphic context");

pub const inst_meth_disabled: Error =
    Cow::Borrowed("`inst_meth()` is disabled; use a lambda `(...) ==> {...}` instead");

pub const invalid_atom_location: Error =
    Cow::Borrowed("`__Atom` attribute can only appear on the first parameter of a function");

pub const as_mut_single_argument: Error =
    Cow::Borrowed("HH\\Readonly\\as_mut takes a single value-typed expression as an argument.");

pub fn out_of_int_range(int: &str) -> Error {
    Cow::Owned(format!(
        "{} is out of the range of 64-bit float values",
        int.to_string(),
    ))
}

pub fn out_of_float_range(float: &str) -> Error {
    Cow::Owned(format!(
        "{} is out of the range of 64-bit float values",
        float.to_string(),
    ))
}

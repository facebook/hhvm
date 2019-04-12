#![allow(non_upper_case_globals)]
/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::token_kind::TokenKind;
use std::borrow::Cow;

// many errors are static strings, but not all of them
pub type Error = Cow<'static, str>;

#[derive(Debug, Clone)]
pub struct SyntaxError {
    pub start_offset: usize,
    pub end_offset: usize,
    pub message: Error,
}

impl SyntaxError {
    pub fn make(start_offset: usize, end_offset: usize, message: Error) -> Self {
        Self {
            start_offset,
            end_offset,
            message,
        }
    }
}

/* Lexical errors */
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

/* Syntactic errors */
pub const error1001: Error = Cow::Borrowed("A .php file must begin with '<?hh'.");
pub const error1003: Error = Cow::Borrowed("The 'function' keyword is expected here.");
pub const error1004: Error = Cow::Borrowed("A name is expected here.");
pub const error1006: Error = Cow::Borrowed("A right brace ('}') is expected here.");
pub const error1007: Error = Cow::Borrowed("A type specifier is expected here.");
pub const error1008: Error = Cow::Borrowed("A variable name is expected here.");
pub const error1010: Error = Cow::Borrowed("A semicolon (';') is expected here.");
pub const error1011: Error = Cow::Borrowed("A right parenthesis (')') is expected here.");
pub const error1013: Error = Cow::Borrowed("A closing angle bracket ('>') is expected here.");
pub const error1014: Error =
    Cow::Borrowed("A closing angle bracket ('>') or comma is expected here.");
pub const error1015: Error = Cow::Borrowed("An expression is expected here.");
pub const error1016: Error = Cow::Borrowed("An assignment is expected here.");
pub const error1017: Error = Cow::Borrowed("An XHP attribute value is expected here.");
pub const error1018: Error = Cow::Borrowed("The 'while' keyword is expected here.");
pub const error1019: Error = Cow::Borrowed("A left parenthesis ('(') is expected here.");
pub const error1020: Error = Cow::Borrowed("A colon (':') is expected here.");
pub const error1021: Error = Cow::Borrowed("An opening angle bracket ('<') is expected here.");
/* TODO: Remove this; redundant to 1009. */
pub const error1022: Error =
    Cow::Borrowed("A right parenthesis ('>') or comma (',') is expected here.");
pub const error1023: Error = Cow::Borrowed("An 'as' keyword is expected here.");
pub const error1025: Error = Cow::Borrowed("A shape field name is expected here.");
pub const error1026: Error = Cow::Borrowed("An opening square bracket ('[') is expected here.");
pub const error1028: Error = Cow::Borrowed("An arrow ('=>') is expected here.");
pub const error1029: Error =
    Cow::Borrowed("A closing double angle bracket ('>>') is expected here.");
pub const error1031: Error =
    Cow::Borrowed("A comma (',') or a closing square bracket (']') is expected here.");
pub const error1032: Error = Cow::Borrowed("A closing square bracket (']') is expected here.");
/* TODO: Break this up according to classish type */
pub const error1033: Error = Cow::Borrowed(concat!(
    "A class member, method, type, trait usage, trait require, ",
    "xhp attribute, xhp use, or xhp category is expected here."
));
pub const error1034: Error = Cow::Borrowed("A left brace ('{') is expected here.");
pub const error1035: Error = Cow::Borrowed("The 'class' keyword is expected here.");
pub const error1036: Error = Cow::Borrowed("An equals sign ('=') is expected here.");
pub const error1038: Error =
    Cow::Borrowed("A semicolon (';') or a namespace body is expected here.");
pub const error1039: Error = Cow::Borrowed("A closing XHP tag is expected here.");
pub const error1041: Error =
    Cow::Borrowed("A function body or a semicolon (';') is expected here.");
pub const error1044: Error =
    Cow::Borrowed("A name, __construct, or __destruct keyword is expected here.");
pub const error1045: Error =
    Cow::Borrowed("An 'extends' or 'implements' keyword is expected here.");
pub const error1046: Error = Cow::Borrowed("A lambda arrow ('==>') is expected here.");
pub const error1047: Error = Cow::Borrowed("A scope resolution operator ('::') is expected here.");
pub const error1048: Error = Cow::Borrowed("A name, variable name or 'class' is expected here.");
pub const error1050: Error = Cow::Borrowed("A name or variable name is expected here.");
pub const error1051: Error = Cow::Borrowed("The 'required' keyword is expected here.");
pub const error1052: Error =
    Cow::Borrowed("An XHP category name beginning with a '%' is expected here.");
pub const error1053: Error = Cow::Borrowed("An XHP name or category name is expected here.");
pub const error1054: Error = Cow::Borrowed("A comma (',') is expected here.");
pub const error1055: Error = Cow::Borrowed(concat!(
    "A fallthrough directive can only appear at the end of",
    " a switch section."
));
/* TODO(20052790): use the specific token's text in the message body. */
pub const error1056: Error =
    Cow::Borrowed("This token is not valid as part of a function declaration.");
pub fn error1057(text: &str) -> Error {
    // TODO (kasper): why does removing to_string() here segfaults
    Cow::Owned(format!(
        "Encountered unexpected token '{}'.",
        text.to_string()
    ))
}
pub fn uppercase_kw(text: &str) -> Error {
    Cow::Owned(format!(
        "Keyword {} must be written in lowercase",
        text.to_string()
    ))
}
pub fn error1058(received: &str, required: &str) -> Error {
    Cow::Owned(format!(
        "Encountered unexpected token '{}'. Did you mean '{}'?",
        received.to_string(),
        required.to_string()
    ))
}
pub fn error1059(terminator: TokenKind) -> Error {
    Cow::Owned(format!(
        "An '{}' is required when using alternate block syntax.",
        terminator.to_string().to_string(),
    ))
}
pub const error1060: Error = Cow::Borrowed(concat!(
    "Leading markup and `<?hh` are not permitted in `.hack` ",
    "files, which are always strict."
));
pub const error1061: Error = Cow::Borrowed("A Pocket Universes operator (':@') is expected here.");
pub const error1062: Error = Cow::Borrowed("References in use lists are not supported in Hack.");
pub const error2001: Error = Cow::Borrowed("A type annotation is required in strict mode.");
pub const error2003: Error =
    Cow::Borrowed("A case statement may only appear directly inside a switch.");
pub const error2004: Error =
    Cow::Borrowed("A default statement may only appear directly inside a switch.");
pub const error2005: Error =
    Cow::Borrowed("A break statement may only appear inside a switch or loop.");
pub const error2006: Error = Cow::Borrowed("A continue statement may only appear inside a loop.");
pub const error2007: Error = Cow::Borrowed("A try statement requires a catch or a finally clause.");
pub const error2008: Error = Cow::Borrowed(concat!(
    "The first statement inside a switch statement must ",
    "be a case or default label statement."
));
pub const error2010: Error = Cow::Borrowed(concat!(
    "Parameters cannot have visibility modifiers (except in ",
    "parameter lists of constructors)."
));
pub const error2011: Error = Cow::Borrowed("A destructor must have an empty parameter list.");
pub const error2012: Error = Cow::Borrowed("A destructor can only have visibility modifiers.");
pub const error2013: Error = Cow::Borrowed("A method declaration cannot have duplicate modifiers.");
pub const error2014: Error = Cow::Borrowed("An abstract method cannot have a method body.");
pub const error2017: Error =
    Cow::Borrowed("A method declaration cannot have multiple visibility modifiers.");
pub const error2018: Error =
    Cow::Borrowed("A constructor or destructor cannot have a non-void type annotation.");
pub const error2020: Error = Cow::Borrowed(concat!(
    "Use of the '{}' subscript operator is deprecated; ",
    " use '[]' instead."
));
pub const error2021: Error = Cow::Borrowed(concat!(
    "A variadic parameter ('...') may only appear at the end of ",
    "a parameter list."
));
pub const error2022: Error =
    Cow::Borrowed("A variadic parameter ('...') may not be followed by a comma.");
pub const error2029: Error = Cow::Borrowed("Only traits and interfaces may use 'require extends'.");
pub const error2030: Error = Cow::Borrowed("Only traits may use 'require implements'.");
pub const error2031: Error =
    Cow::Borrowed("A class, interface, or trait declaration cannot have duplicate modifiers.");
pub const error2032: Error = Cow::Borrowed("The array type is not allowed in strict mode.");
pub const error2033: Error = Cow::Borrowed(concat!(
    "The splat operator ('...') for unpacking variadic arguments ",
    "may only appear at the end of an argument list."
));
pub const error2034: Error = Cow::Borrowed(concat!(
    "A type alias declaration cannot both use 'type' and have a ",
    "constraint. Did you mean 'newtype'?"
));
pub const error2035: Error = Cow::Borrowed("Only classes may implement interfaces.");
pub const error2036: Error = Cow::Borrowed(concat!(
    "Only interfaces and classes may extend other interfaces and ",
    "classes."
));
pub const error2037: Error = Cow::Borrowed("A class may extend at most one other class.");
pub const error2040: Error = Cow::Borrowed(concat!(
    "Invalid use of 'list(...)'. A list expression may only be ",
    "used as the left side of a simple assignment, the value clause of a ",
    "foreach loop, or a list item nested inside another list expression."
));
pub const error2041: Error = Cow::Borrowed(concat!(
    "Unexpected method body: interfaces may contain only",
    " method signatures, and not method implementations."
));
pub const error2042: Error = Cow::Borrowed("Interfaces may not be declared 'abstract'.");
pub const error2043: Error = Cow::Borrowed("Traits may not be declared 'abstract'.");
pub const error2045: Error =
    Cow::Borrowed("No method inside an interface may be declared 'abstract'.");
pub const error2046: Error = Cow::Borrowed(concat!(
    "The 'async' annotation cannot be used on 'abstract' methods ",
    "or methods inside of interfaces."
));
pub const error2048: Error = Cow::Borrowed("Expected group use prefix to end with '\\'");
pub const error2049: Error = Cow::Borrowed("A namespace use clause may not specify the kind here.");
pub const error2050: Error =
    Cow::Borrowed("A concrete constant declaration must have an initializer.");
pub const error2051: Error =
    Cow::Borrowed("An abstract constant declaration must not have an initializer.");
pub const error2052: Error = Cow::Borrowed(concat!(
    "Cannot mix bracketed namespace declarations with ",
    "unbracketed namespace declarations"
));
pub const error2053: Error = Cow::Borrowed(concat!(
    "Use of 'var' as synonym for 'public' in declaration disallowed in Hack. ",
    "Use 'public' instead."
));
pub const error2054: Error = Cow::Borrowed(concat!(
    "Method declarations require a visibility modifier ",
    "such as public, private or protected."
));
pub const error2055: Error = Cow::Borrowed("At least one enumerated item is expected.");
pub const error2056: Error = Cow::Borrowed("First unbracketed namespace occurrence here");
pub const error2057: Error = Cow::Borrowed("First bracketed namespace occurrence here");
pub const error2058: Error = Cow::Borrowed("Property may not be abstract.");
pub const invalid_shape_field_name: Error =
    Cow::Borrowed("Shape field name must be a nonempty single-quoted string or a class constant");
pub const shape_field_int_like_string: Error =
    Cow::Borrowed("Shape field name must not be an int-like string (i.e. \"123\");");
pub const error2061: Error = Cow::Borrowed(concat!(
    "Non-static instance variables are not allowed in abstract ",
    "final classes."
));
pub const error2062: Error =
    Cow::Borrowed("Non-static methods are not allowed in abstract final classes.");
pub const error2063: Error = Cow::Borrowed("Expected integer or string literal.");
pub const error2064: Error = Cow::Borrowed("Reference methods are not allowed in strict mode.");
pub const error2065: Error =
    Cow::Borrowed("A variadic parameter ('...') must not have a default value.");
/* This was typing error 4077. */
pub const error2066: Error = Cow::Borrowed(concat!(
    "A previous parameter has a default value. Remove all the ",
    "default values for the preceding parameters, or add a default value to ",
    "this one."
));
pub const error2067: Error = Cow::Borrowed("A hack source file cannot contain '?>'.");
pub const error2068: Error = Cow::Borrowed("hh blocks and php blocks cannot be mixed.");
pub const error2069: Error = Cow::Borrowed("Operator '?->' is only allowed in Hack.");
pub const prefixed_invalid_string_kind: Error =
    Cow::Borrowed("Only double-quoted strings may be prefixed.");
pub const illegal_interpolated_brace_with_embedded_dollar_expression: Error =
    Cow::Borrowed(concat!(
        "The only legal expressions inside a {$...}-expression embedded in a string are ",
        "variables, function calls, subscript expressions, and member access expressions"
    ));
pub const expected_dotdotdot: Error = Cow::Borrowed("'...' is expected here.");
pub const invalid_foreach_element: Error = Cow::Borrowed(
    "An arrow ('=>') or right parenthesis (')') \
     is expected here.",
);
pub const inline_function_def: Error =
    Cow::Borrowed("Inline function definitions are not supported in Hack");
pub const decl_outside_global_scope: Error =
    Cow::Borrowed("Declarations are not supported outside global scope");
pub const pocket_universe_final_expected: Error =
    Cow::Borrowed("The 'final' keyword is expected here.");
pub const pocket_universe_enum_expected: Error =
    Cow::Borrowed("The 'enum' keyword is expected here.");
pub const pocket_universe_invalid_field: Error =
    Cow::Borrowed("Invalid pocket universe field syntax.");
pub const type_keyword: Error = Cow::Borrowed("The 'type' keyword is expected here.");
pub const expected_simple_offset_expression: Error =
    Cow::Borrowed("A simple offset expression is expected here");
pub const expected_user_attribute: Error = Cow::Borrowed("A user attribute is expected here.");
pub const expected_as_or_insteadof: Error =
    Cow::Borrowed("The 'as' keyword or the 'insteadof' keyword is expected here.");
pub const missing_double_quote : Error = /* error0010 analogue */ Cow::Borrowed(
  "A double quote is expected here.");

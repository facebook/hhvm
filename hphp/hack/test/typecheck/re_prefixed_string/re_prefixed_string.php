<?hh

function f(): void {
  $x = re"/Hello/";

  // $x is an HH\Lib\Regex\Pattern
  $y = goodbye($x);

  // Can access the shape $y with an integer key
  // Since HH\Lib\Regex\Match es are always shapes with string values,
  // $y_0 will be a string
  $y_0 = $y[0];

  $x_single = re'/Hello/';
  // $x_single is an HH\Lib\Regex\Pattern, but with single quotes
  $y_single = goodbye($x_single);
  $y_single0 = $y_single[0];

  // `re`-prefixed strings can still be concatenated like strings
  $z = $x.", world!";

  // $z is a string but not an HH\Lib|Regex\Pattern
  $w = goodbye($z);
}

function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
}

/* Bad regex patterns that can't be compiled by PCRE give type errors */
function g(): void {
  $good0 = re"/Hel(\D)(?'o'\D)/";
  $bad1 = re"/He(?'l'\D)(?'l'\D)o/";
  $bad2 = re"/He(?'42'\D)lo/";
  $bad3 = re"/\c/";
}

/* Only Ast.String s are allowed to be `re`-prefixed. Non-strings and
strings with embedded expressions (Ast.String2 s) are not supported. */
function h(): void {
  $world = "world";
  $string2 = re"/Hello, {$world}!/";
}

/* Regex patterns must be delimited by non-alphanumeric, non-whitespace,
   non-backslash matching characters. */
function missing_delimiter(): void {
  $bad0 = re"Hello";
  $good1 = re"/Hello/";
  $bad2 = re"/Hello";
  $good3 = re"#Hello#";
  $bad4 = re"#Hello/";
  $bad5 = re"#Hello";
  $good6 = re"(Hello)";
  $good7 = re")Hello)";
  $bad8 = re"(Hello(";
  $good9 = re"[Hello]";
  $bad10 = re"\\Hello\\";
  $bad11 = re"HelloH";
}

/* Only certain characters may appear as global options at the end of delimited
   patterns. Within a pattern, a character that matches the first delimiter must
   be escaped with a backslash to not to be seen as ending the pattern. In the
   case of a bracket-style first delimiter, both the left and right varieties of
   that delimiter must be escaped. */
function invalid_global_option(): void {
  $good0 = re"#Hello#imsxADSUXu";
  $bad1 = re"#Hello#42";
  $bad2 = re"#What # are you#";
  $good3 = re"#What \# are you#";
}

function parentheses_are_weird(): void {
  // $good0 is good in the typechecker but confuses the parser for some reason
  $good0 = re"(He\(?'one'\D\)\(\D\)\(?'three'\D\)\(\D\)\(\D\)\(?'six'\D\))";
  $bad1 = re"(He(?'one'\D)(\D)(?'three'\D)(\D)(\D)(?'six'\D))";
  $bad2 = re"(He(?'one'\D\)(\D\)(?'three'\D\)(\D\)(\D\)(?'six'\D\))";
}

function empty_pattern(): void {
  $empty = re"";
}

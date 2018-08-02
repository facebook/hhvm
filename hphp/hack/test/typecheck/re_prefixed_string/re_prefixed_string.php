<?hh

function f(): void {
  $x = re"/Hello/";

  // $x is an HH\Lib\Regex\Pattern
  $y = goodbye($x);

  // Can access the shape $y with an integer key
  // Since HH\Lib\Regex\Match es are always shapes with string values,
  // $y_0 will be a string
  $y_0 = $y[0];

  // `re`-prefixed strings can still be concatenated like strings
  $z = $x.", world!";

  // $z is a string but not an HH\Lib|Regex\Pattern
  $w = goodbye($z);
}

function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
  // UNSAFE_BLOCK
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
  $good10 = re"(He(?'one'\D)(\D)(?'three'\D)(\D)(\D)(?'six'\D))";
  $bad11 = re"\\Hello\\";
  $bad12 = re"HelloH";
}

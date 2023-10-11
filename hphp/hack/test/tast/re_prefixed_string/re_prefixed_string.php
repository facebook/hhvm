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
}

function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
  throw new Exception();
}

/* Bad regex patterns that can't be compiled by PCRE give type errors */
/* Test that good ones get turned into the right shape keys */
function bad_pattern(): void {
  $good0 = re"/Hel(\D)(?'o'\D)/";
  $bad1 = re"/He(?'l'\D)(?'l'\D)o/";
  $bad2 = re"/He(?'42'\D)lo/";
  $bad3 = re"/\c/";
  $good4 = re"/He(?'one'\D)(\D)(?'three'\D)(\D)(\D)(?'six'\D)/";
  $good5 = re"/a(b)(?<c>c)(d)(?<e>e)/";
  $good6 = re"/Hello/";
  $good7 = re"/a(?<b>b)(?<c>c)(?<d>d)(?<e>e)/";
  $good8 = re"/a(b)(c)(d)?e/";
  $good9 = re"/WoS Action: .*Disable.+\[(\w+)/s";
  $good10 = re"/User disabled for having at least \d+ spam reports/";
  $good11 = re"/(?<!\w)youtube.com$/";
  $good12 = re"/.+(?P<version>(:s(\d)+)*:c(\d)+)$/U";
  $good13 = re"/^t=((?<m>\d+)m)?(?<s>\d+)s?$/i";
  $good14 = re"/(?<=\(currently )[^)](?=\))/";
  $good15 = re"/(?<=\(currently )[^)]+(?=\))/";
  $good16 = re"/(a)(b)(c)(?<d>d)(e)(f)(g)(?<h>h)(i)(j)(k)(l)(m)(?<n>n)/";
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

function parentheses_are_weird(): void {
  // $good0 is good in the typechecker but useless capture groups-wise and
  // confuses the parser for some reason
  $good0 = re"(He\(?'one'\D\)\(\D\)\(?'three'\D\)\(\D\)\(\D\)\(?'six'\D\))";
}

<?hh

function f(): void {
  $x = re"Hello";

  // $x is an HH\Lib\Regex\Pattern
  $y = goodbye($x);

  // Can access the shape $y with an integer key
  // Since HH\Lib\Regex\Match es are always shapes with string values,
  // $y_0 will be a string
  $y_0 = $y[0];

  // `re`-prefixed strings can still be concatenated like strings
  $z = $x.", world!";
}

/* HH_FIXME[4110] Don't care what this returns, just that it is of type T */
function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
}

/* Bad regex patterns that can't be compiled by PCRE give type errors */
/* Test that good ones get turned into the right shape keys */
function g(): void {
  $good0 = re"Hel(\D)(?'o'\D)";
  $bad1 = re"He(?'l'\D)(?'l'\D)o";
  $bad2 = re"He(?'42'\D)lo";
  $bad3 = re"\c";
  $good4 = re"He(?'one'\D)(\D)(?'three'\D)(\D)(\D)(?'six'\D)";
  $good5 = re"a(b)(?<c>c)(d)(?<e>e)";
  $good6 = re"Hello";
  $good7 = re"a(?<b>b)(?<c>c)(?<d>d)(?<e>e)";
  $good8 = re"a(b)(c)(d)?e";
  $good9 = re"/WoS Action: .*Disable.+\[(\w+)/s";
  $good10 = re"/User disabled for having at least \d+ spam reports/";
  $good11 = re"/(?<!\w)youtube.com$/";
  $good12 = re"/.+(?P<version>(:s(\d)+)*:c(\d)+)$/U";
  $good13 = re"/^t=((?<m>\d+)m)?(?<s>\d+)s?$/i";
  $good14 = re"/(?<=\(currently )[^)](?=\))/";
  $good15 = re"/(?<=\(currently )[^)]+(?=\))/";
  $good16 = re"(a)(b)(c)(?<d>d)(e)(f)(g)(?<h>h)(i)(j)(k)(l)(m)(?<n>n)";
}

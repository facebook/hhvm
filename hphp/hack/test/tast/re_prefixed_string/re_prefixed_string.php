<?hh

function f(): void {
  $x = re"Hello";

  // $x is an HH\Lib\Regex\Pattern
  $y = goodbye($x);

  // Can access the shape $y with an integer key
  // $y[0] is a string by the definition of T
  $y_0 = $y[0];

  // `re`-prefixed strings can still be treated as strings
  $z = $x.", world!";
}

/* HH_FIXME[4110] Don't care what this returns, just that it is of type T */
function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
}

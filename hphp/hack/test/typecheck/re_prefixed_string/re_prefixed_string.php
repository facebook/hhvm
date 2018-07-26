<?hh

function f(): void {
  $x = re"Hello";

  // $x is an HH\Lib\Regex\Pattern
  $y = goodbye($x);

  // Can access the shape $y with an integer key
  // Since HH\Lib\Regex\Match es are always shapes with string values,
  // $y_0 will be a string
  $y_0 = $y[0];

  // `re`-prefixed strings can still be treated as strings
  $z = $x.", world!";

  // $z is a string but not an HH\Lib|Regex\Pattern
  $w = goodbye($z);
}

function goodbye<T as HH\Lib\Regex\Match>(HH\Lib\Regex\Pattern<T> $pattern): T {
  // UNSAFE_BLOCK
}

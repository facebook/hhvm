<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function foo(
  int $x = 0,
  int $y = 1,
  int $z = 2,
  named $a = shape('a' => 2),
  named int $b = 7,
) {
  var_dump(vec[$x, $y, $z, $a, $b]);
}

<<__EntryPoint>>
function main() {
  foo();
  foo(a=shape('a' => 1));
  foo(b=42);
  foo(84, b=42);
}

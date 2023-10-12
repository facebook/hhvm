<?hh // strict

function test(vec<string> $x): Unknown {
  return new Unknown(3, 's', vec[3], ...$x);
}

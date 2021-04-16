<?hh // strict

function good_math(num $a, int $b): void {
  1 + 2;
  1 - 2;
  1 * 2;
  1/2;
  1 ** 2;
  20 % 4;
  $a += 2;
  $a -= $a;
  $a *= 2.5;
  $a /= 2;
  $b %= 1;
}

class Foo {}

function bad_math(int $a, Foo $b, string $c): void {
  true + 2;
  "bar" - "foo";
  "foo" * $c;
  $b/2;
  1 ** $b;
  "foo" % 2;
  $a += $c;
  $c -= $a;
  $b *= 4;
  $c /= $b;
  $c %= $b;
}

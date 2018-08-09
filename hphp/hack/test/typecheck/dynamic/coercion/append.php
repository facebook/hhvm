<?hh // strict

function foo(array<dynamic> $a): array<dynamic> {
  $a[] = 5;
  $a[0] = "hi";
  return $a;
}

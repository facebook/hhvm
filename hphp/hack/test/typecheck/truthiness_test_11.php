<?hh // strict

function test(bool $b, Traversable<int> $t, vec<int> $v): void {
  $x = $b ? $t : $v;
  // Error--$t may be an always-truthy value like a user-defined Iterator.
  if ($x) {
  }
}

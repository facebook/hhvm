<?hh // strict

function test(bool $b, Traversable<int> $t, vec<int> $v): void {
  $x = $b ? $t : $v;
  // Should be an error, but isn't yet--$t may be an always-truthy value like a user-defined Iterator.
  if ($x) {
  }
}

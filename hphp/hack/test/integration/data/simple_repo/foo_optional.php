<?hh

function foo_optional((function(int, optional bool): int) $x): void {
  $y = $x;

}

<?hh



function expectAnyTuple((mixed...) $_): void {}
function expectAnyArraykeyTuple((arraykey...) $x): void {
  expectAnyTuple($x);
}
function expectAtLeastInt((int, mixed...) $x): void {
  expectAnyTuple($x);
}
function expectIntThenMaybeString((int, optional string) $x): void {
  expectAnyTuple($x);
  expectAnyArraykeyTuple($x);
}
function expectIntThenMaybeArraykeyThenMaybeBool(
  (int, optional arraykey, optional bool) $x,
): void {
  expectAnyTuple($x);
  expectAtLeastInt($x);
}

function test_closed_sub(): void {
  $none = tuple();
  $one = tuple(1);
  $two = tuple(1, 'a');
  expectAnyTuple($none);
  expectAnyTuple($one);
  expectAnyTuple($two);
  expectAnyArraykeyTuple($none);
  expectAnyArraykeyTuple($one);
  expectAnyArraykeyTuple($two);
  expectAtLeastInt($one);
  expectAtLeastInt($two);
  expectIntThenMaybeString($one);
  expectIntThenMaybeString($two);
  expectIntThenMaybeArraykeyThenMaybeBool($one);
  expectIntThenMaybeArraykeyThenMaybeBool($two);
}

<?hh



function expectAnyTuple((mixed...) $x): void {
  var_dump($x);
}
function expectAnyArraykeyTuple((arraykey...) $x): void {
  expectAnyTuple($x);
  var_dump($x);
}
function expectAtLeastInt((int, mixed...) $x): void {
  expectAnyTuple($x);
  var_dump($x);
}
function expectIntThenMaybeString((int, optional string) $x): void {
  expectAnyTuple($x);
  expectAnyArraykeyTuple($x);
  var_dump($x);
}
function expectIntThenMaybeArraykeyThenMaybeBool(
  (int, optional arraykey, optional bool) $x,
): void {
  expectAnyTuple($x);
  expectAtLeastInt($x);
  var_dump($x);
}

<<__EntryPoint>>
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

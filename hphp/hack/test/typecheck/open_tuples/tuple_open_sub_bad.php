<?hh



function expectAnyTuple((mixed...) $x): void {
  expectAnyArraykeyTuple($x);
  expectAtLeastInt($x);
  expectIntThenMaybeString($x);
}
function expectAnyArraykeyTuple((arraykey...) $x): void {
  expectAnyTuple($x);
}
function expectAtLeastInt((int, mixed...) $x): void {
  expectAnyTuple($x);
}
function expectIntThenMaybeString((int, optional string) $x): void {
}
function expectIntThenArraykey((int, arraykey, bool...) $_): void {}

function expectIntThenMaybeArraykeyThenMaybeBool(
  (int, optional arraykey, optional bool) $x,
): void {
  expectIntThenMaybeString($x);
  expectIntThenArraykey($x);
}

function test_closed_sub(): void {
  $none = tuple();
  $one = tuple(1);
  $two = tuple(1, 'a');
  $two2 = tuple(false, 'a');
  expectAnyArraykeyTuple($two2);
  expectAtLeastInt($two2);
  expectAtLeastInt($none);
}

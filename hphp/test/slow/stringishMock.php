<?hh

<<__MockClass>>
class MockStringish implements Stringish {}

<<__EntryPoint>>
function mytest(): void {
  $ms = new MockStringish();
  var_dump(
    $ms is Stringish,
    $ms is StringishObject,
    $ms is XHPChild,
    is_a($ms, Stringish::class),
    is_a($ms, StringishObject::class),
    is_a($ms, XHPChild::class),
  );
}

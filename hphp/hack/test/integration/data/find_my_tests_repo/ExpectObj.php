<?hh

function expect<T>(readonly T $obj, mixed ...$args)[]: ExpectObj<T> {
  return new ExpectObj();
}

class ExpectObj<T> {

  <<__Overlapping('T', 'T2')>>
  final public function toEqual<T2>(T2 $expected)[]: void {}

}

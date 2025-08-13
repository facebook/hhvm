<?hh

<<__DynamicallyCallable, __NoAutoLikes>>
function expect<T>(T $obj)[]: ExpectObj<T> {
  return new ExpectObj($obj);
}

final class ExpectObj<T> {
  final public function toNotBeEmpty()[]: void where T as Container<mixed> {}

  final public function __construct(protected T $obj)[] {}
}

function f(dynamic $d): void {
  $d = $d as dict<_, _>;
  $v = $d[0];
  $e = expect($v);
  $e->toNotBeEmpty();
}

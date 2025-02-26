<?hh

interface I {}
class C implements I {}
class R<reify T> implements I {}

function _is(mixed $m): class<I> {
  if ($m is class<C>) {
    return $m;
  }
  if ($m is class<R<int>>) {
    return $m;
  }
  return C::class;
}

function _as(bool $b, mixed $m): class<I> {
  if ($b) {
    return $m as class<C>;
  } else {
    return $m is class<R<int>>;
  }
}

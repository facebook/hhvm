<?hh

class C<T> {}

function f<T as arraykey>(C<T> $c): C<T> {
  return $c;
}

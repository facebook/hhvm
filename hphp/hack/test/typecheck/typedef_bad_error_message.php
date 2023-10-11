<?hh

type Blah<T> = Vector<T>;

function f(): Blah<int> {
  return Vector { 'hi' };
}

<?hh //strict

class A {
  public function __construct(?Vector<int> $values = null) {}
}

function f(bool $b, varray<mixed> $values): ?A {
  if ($b) {
    return null; // create return continuation where $values is a varray<mixed>
  }
  $values = varray[1];
  $result = new A(unique(new Vector($values)));
  return $result; // make sure 'mixed' from the previously created continuation
  // does not get unified with the 'int' from line 4
}

function unique<T as arraykey>(Traversable<T> $collection): Vector<T> {
  return new Vector($collection);
}

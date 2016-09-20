<?hh // strict

class Foo<T> {
  const where = 0; // 'where' shouldn't be a keyword everywhere

  public function bar<Tx>(
  ): void
  where
    T as Tx,
    Tx = Vector<int>,
  {}

  public function baz<Ta, Tb, Tc>(): Foo<Ta>
  where
    KeyedTraversable<Ta, Tb> super Map<Tc,T>,
  {
    return new Foo();
  }

  public function qux<Tx>(Tx $x): Tx where Tx = T {
    return $x;
  }
}

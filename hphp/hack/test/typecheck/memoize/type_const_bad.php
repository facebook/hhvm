<?hh

interface GenericEnt<T> {}

class Bar {}

class EntPhoto implements GenericEnt<this::TID> {
  const type TID = Bar;
  <<__Memoize>>
  public function someMethod(this::TID $i): void {}
}

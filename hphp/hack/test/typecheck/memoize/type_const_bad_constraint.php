<?hh

interface GenericEnt<T> {}

class Bar {}

abstract class EntPhoto implements GenericEnt<this::TID> {
  abstract const type TID as Bar;
  <<__Memoize>>
  public function someMethod(this::TID $i): void {}
}

<?hh

interface GenericEnt<T> {}

class EntPhoto implements GenericEnt<this::TID> {
  const type TID = int;
  <<__Memoize>>
  public function someMethod(this::TID $i): void {}
}

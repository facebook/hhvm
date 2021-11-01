<?hh

interface GenericEnt<T> {}

abstract class EntPhoto implements GenericEnt<this::TID> {
  abstract const type TID as IMemoizeParam;
  <<__Memoize>>
  public function someMethod(this::TID $i): void {}
}

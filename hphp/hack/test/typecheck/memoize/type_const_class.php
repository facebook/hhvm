<?hh

interface GenericEnt<T> {}

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

class EntPhoto implements GenericEnt<this::TID> {
  const type TID = Bar;
  <<__Memoize>>
  public function someMethod(this::TID $i): void {}
}

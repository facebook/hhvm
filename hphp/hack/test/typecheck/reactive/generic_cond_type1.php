<?hh // partial

interface IRxReactiveItem<T> {
  <<__Rx>>
  public function use(): void;
}
class NonReactiveItem {
  public function use(): void {}
}
interface IRxTestClass {
  <<__Rx>>
  public function getItem<T>(): IRxReactiveItem<T>;
}
class TestClass_ {
  <<__Rx>>
  public function getItem(): NonReactiveItem {
    return new NonReactiveItem();
  }
  <<__Rx, __OnlyRxIfImpl(IRxTestClass::class)>>
  public function useItem(): void {
    $item = $this->getItem();
    $item->use();
  }
}

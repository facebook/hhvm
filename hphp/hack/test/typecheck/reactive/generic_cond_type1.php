<?hh
interface IRxReactiveItem<T> {

  public function use(): void;
}
class NonReactiveItem {
  public function use(): void {}
}
interface IRxTestClass {

  public function getItem<T>(): IRxReactiveItem<T>;
}
class TestClass_ {

  public function getItem(): NonReactiveItem {
    return new NonReactiveItem();
  }

  public function useItem(): void {
    $item = $this->getItem();
    $item->use();
  }
}

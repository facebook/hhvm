<?hh

interface IHasGeneric<T> {}

final class HasString implements IHasGeneric<string> {}

final class MyTestClass<T> {

  public function __construct(private IHasGeneric<T> $field) {}

  public function getT(): T {
    $x = $this->field;
    if ($x is HasString) {
      return 'string';
    }
    throw new Exception();
  }
}

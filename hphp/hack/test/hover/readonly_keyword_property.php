<?hh

class Foo {
  public function __construct(public int $prop) {}

  public readonly function isEven(): bool {
    //      ^ hover-at-caret
    $prop = $this->prop;
    return $prop % 2 == 0;
  }
}

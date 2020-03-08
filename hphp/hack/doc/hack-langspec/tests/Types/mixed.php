<?hh // strict

namespace NS_mixed;

class C {
  const mixed THING = 'abc';
  private mixed $prop = true;

  public function setProp(mixed $val): void {
    $this->prop = $val;
  }

  public function getProp(): mixed {
    return $this->prop;
  }
}

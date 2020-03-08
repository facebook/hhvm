<?hh // strict

namespace NS_float;

class C {
  const float LIMIT = 99.99;
  private float $prop = 10.5;

  public function setProp(float $val): void {
    $this->prop = $val;
  }

  public function getProp(): float {
    return $this->prop;
  }
}

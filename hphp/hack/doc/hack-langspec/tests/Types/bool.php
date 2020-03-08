<?hh // strict

namespace NS_bool;

class C {
  const bool ON = true;
  private bool $prop = true;

  public function setProp(bool $val): void {
    $this->prop = $val;
  }

  public function getProp(): bool {
    return $this->prop;
  }
}

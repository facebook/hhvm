<?hh // strict

namespace NS_tuple;

class C {
//  const (int, string) T = tuple(2, 'aa'); // *** no way to write a tuple constant expression
  private (int, string) $prop = tuple(12, 'green');

  public function setProp((int, string) $val): void {
    $this->prop = $val;
  }

  public function getProp(): (int, string) {
    return $this->prop;
  }
}

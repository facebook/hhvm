<?hh

class C {
  public function __construct(public int $a, public int $b) {}
  public function getA() : int {
    return $this->a;
  }
  // Should parser error
  public function test(<<__External>> int $x): void {
    try {
    $x = $this->getA();
    } catch (Exception $_){}
  }
}

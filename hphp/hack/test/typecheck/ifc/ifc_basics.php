<?hh

class C {
  public function __construct(public int $a, public int $b) {}

  <<__Policied("A")>>
  public function getA() : int {
    return $this->a;
  }

  <<__Policied("PUBLIC")>>
  public function test(): void {
    try {
    $x = $this->getA();
    } catch (Exception $_){}
  }
}

<?hh
<<file:__EnableUnstableFeatures('ifc')>>

class C {
  public function __construct(public int $a, public int $b) {}

  public function getA() : int {
    return $this->a;
  }

  public function test(): void {
    try {
    $x = $this->getA();
    } catch (Exception $_){}
  }
}

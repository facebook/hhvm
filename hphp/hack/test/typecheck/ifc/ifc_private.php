<?hh
<<file:__EnableUnstableFeatures('ifc')>>

class C {
  public function __construct(public int $a, public int $b) {}

  <<__Policied("PRIVATE")>>
  public function getA() : int {
    return $this->a;
  }

  <<__Policied("PRIVATE")>>
  public function test(): void {
    $x = $this->getA();
  }
}

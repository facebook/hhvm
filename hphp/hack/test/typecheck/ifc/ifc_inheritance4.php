<?hh
<<file:__EnableUnstableFeatures('ifc')>>

interface IA {
  <<__Policied("A")>>
  public function foo() : void;
}

class B implements IA {
  <<__Policied("A")>>
  public function foo(): void {}

}

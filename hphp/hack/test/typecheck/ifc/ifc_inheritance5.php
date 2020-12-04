<?hh
<<file:__EnableUnstableFeatures('ifc')>>

interface IA {
  public function foo() : void;
}

class B implements IA {
  <<__Policied("PUBLIC")>> // explicit vs. implicit public, should be good
  public function foo(): void {}

}

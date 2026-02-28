<?hh

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo() : void {
    $this->bar();
  }
}

trait T0 {
  reqAUTO332

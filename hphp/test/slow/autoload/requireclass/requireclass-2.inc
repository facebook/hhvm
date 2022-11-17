<?hh

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function bar(): int { return $this->foo(); }
}

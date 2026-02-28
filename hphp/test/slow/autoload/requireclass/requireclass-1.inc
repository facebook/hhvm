<?hh

<<file:__EnableUnstableFeatures('require_class')>>

final class C {
  use T;
  public function foo(): int { return 42; }
}

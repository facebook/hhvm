<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type MultiClass = I | C;

interface IB {
  require extends B<int>;
  public function i(): int;
}

interface I extends IB {}

class B<T> {}

class C extends B<int> {
  public function g(): int { return 0; }
}

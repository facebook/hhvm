<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

interface I {
  abstract const type Ti super num;
}

class A {
  public function f(num $c): void {}
}

class B<Tb as I> extends A {
  public function f<T>(T $c): void where T = Tb::Ti {}
}

<?hh

interface I {
  abstract const type Ti as num;
}

class A<Ta as I> {
  public function f<T>(T $t): void where T = Ta::Ti {}
}

class B<Tb as I> extends A<Tb> {
  public function f(num $t): void {}
}

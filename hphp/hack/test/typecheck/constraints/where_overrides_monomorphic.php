<?hh

class Contra<-T> {}

interface I {
  abstract const type Ti as num;
}

class A {
  public function f(Contra<num> $c): void {}
}

class B<Tb as I> extends A {
  // TODO: upddate this example to remove Contra when super syntax is available
  public function f<T>(Contra<T> $c): void where T = Tb::Ti {}
}

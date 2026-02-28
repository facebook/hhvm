<?hh

class Cov<+Tc> {}
class A {}

class Base<Tb> {
  public function foo<Tf as A>(): void where Tb as Cov<Tf> {}
}

class Derived<Td> extends Base<Cov<Td>> {
  public function foo<Tf as A>(): void where Td as A {}
}

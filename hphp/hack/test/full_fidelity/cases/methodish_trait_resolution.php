<?hh

trait T1 {
  public function animal(): void {
    echo "Dog";
  }
}

trait T2 {
  public function animal(): void {
    echo "Cat";
  }
}

class C {
  use T1, T2;

  public function animal(): void = T1::animal;
}

class D {
  public function animal(): void = animal;
  public function animal(): void = T1::class;
  public function animal(): void = \MyNamespace\T2::foo; // ok
}

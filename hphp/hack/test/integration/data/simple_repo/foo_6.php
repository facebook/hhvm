<?hh

interface IFoo {
  public function i(): int;
}

class FooDependentClass implements IFoo {
  public function i(): int {
    return 1;
  }
}

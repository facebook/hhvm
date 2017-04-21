<?hh

class Foo<
  Ta,
  Tb,
  Tc,
> {
  public function getRandomNumber(): int {
    return 4;
  }
}

$foo = new Foo();
var_dump($foo->getRandomNumber());

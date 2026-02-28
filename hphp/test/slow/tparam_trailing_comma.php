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


<<__EntryPoint>>
function main_tparam_trailing_comma() :mixed{
$foo = new Foo();
var_dump($foo->getRandomNumber());
}

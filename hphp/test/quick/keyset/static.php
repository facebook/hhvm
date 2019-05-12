<?hh

<<TestAttr(keyset[1, "1"])>>
class Foo {
  const BAR = "bar";
  const ABC = keyset[1, "1", 2, "2"];
  public $XYZ = keyset[1, "1", 2, "2"];
  public $FOO = keyset[Foo::BAR];
}

<<__EntryPoint>> function main(): void {
  $f = new Foo;
  var_dump($f::ABC);
  var_dump($f->XYZ);
  var_dump($f->FOO);
  var_dump((new ReflectionClass('Foo'))->getAttributes());
}

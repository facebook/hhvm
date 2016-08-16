<?hh

<<TestAttr(keyset[1, "1"])>>
class Foo {
  const ABC = keyset[1, "1", 2, "2"];
  public $XYZ = keyset[1, "1", 2, "2"];
}

function main() {
  $f = new Foo;
  var_dump($f::ABC);
  var_dump($f->XYZ);
  var_dump((new ReflectionClass('Foo'))->getAttributes());
}

main();

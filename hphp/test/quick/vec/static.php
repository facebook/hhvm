<?hh

<<TestAttr(vec[1, "1"])>>
class Foo {
  const ABC = vec[1, "1", 2, "2", "alpha", "beta"];
  public $XYZ = vec["hello", "world", 1, 2];
}

<<__EntryPoint>> function main(): void {
  $f = new Foo;
  var_dump($f::ABC);
  var_dump($f->XYZ);
  var_dump((new ReflectionClass('Foo'))->getAttributes());
}

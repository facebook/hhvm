<?hh

<<TestAttr(dict[1 => "a", "1" => "b"])>>
class Foo {
  const ABC = dict[1 => "alpha", "1" => "ALPHA", 2 => "beta", "2" => "BETA"];
  public $XYZ = dict[
    1 => "hello",
    "1" => "world",
    2 => "foo",
    "2" => "bar"
  ];
}

<<__EntryPoint>> function main(): void {
  $f = new Foo;
  var_dump($f::ABC);
  var_dump($f->XYZ);
  var_dump((new ReflectionClass('Foo'))->getAttributes());
}

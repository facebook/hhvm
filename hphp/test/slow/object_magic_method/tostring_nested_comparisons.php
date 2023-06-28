<?hh

class C {
  public function __toString()[] :mixed{
    echo "__toString called\n";
    return "string";
  }
}

class Box {
  public function __construct(public $p)[] {}
}

<<__EntryPoint>>
function test() :mixed{
  echo "==== arrays ====\n";
  var_dump(HH\Lib\Legacy_FIXME\lte(vec[new C()], vec["a"]));
  var_dump(HH\Lib\Legacy_FIXME\gte(vec[new C()], vec["a"]));
  var_dump(HH\Lib\Legacy_FIXME\lte(vec["a"], vec[new C()]));
  var_dump(HH\Lib\Legacy_FIXME\gte(vec["a"], vec[new C()]));

  echo "==== objects ====\n";
  var_dump(HH\Lib\Legacy_FIXME\lte(new Box(new C()), new Box("a")));
  var_dump(HH\Lib\Legacy_FIXME\gte(new Box(new C()), new Box("a")));
  var_dump(HH\Lib\Legacy_FIXME\lte(new Box("a"), new Box(new C())));
  var_dump(HH\Lib\Legacy_FIXME\gte(new Box("a"), new Box(new C())));
}

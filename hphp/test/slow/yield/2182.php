<?hh

class X {
  private $a = varray[1,2,3];
  function foo() {
    yield $this->a;
  }
}
function test() {
  $z = new Z;
  foreach ($z->foo() as $v) {
    var_dump($v);
  }
}
<<__EntryPoint>>
function entrypoint_2182(): void {
  if (isset($g)) {
    include '2182-1.inc';
  } else {
    include '2182-2.inc';
  }
  include '2182-after.inc';
  test();
}

<?hh

class X {
  private $a = varray[1,2,3];
  function foo() {
    yield $this->a;
  }
}
if (isset($g)) {
  include '2182-1.inc';
} else {
  include '2182-2.inc';
}
class Z extends Y {}
function test() {
  $z = new Z;
  foreach ($z->foo() as $v) {
    var_dump($v);
  }
}
test();

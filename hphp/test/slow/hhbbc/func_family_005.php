<?hh

abstract class Base {
  abstract public function foo(string $x, string $y):mixed;
}

class Unrelated {
  public function foo(inout string $x, inout string $y) :mixed{ $l = $x . $y; return $l; }
}

class D1 extends Base {
  public function foo(string $x, string $y) :mixed{ return $x; }
}
class D2 extends Base {
  public function foo(string $x, string $y) :mixed{ return $x; }
}

function main(Base $b, string $l) :mixed{
  $x = $b->foo('a', $l);
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_005() :mixed{
$l = (string)mt_rand();
main(new D2, $l);
main(new D1, $l);
}

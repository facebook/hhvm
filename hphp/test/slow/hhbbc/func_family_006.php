<?hh

abstract class Base {
  abstract public function foo(string $x, string $y);
}

class D1 extends Base {
  public function foo(string $x, string $y) { return $x; }
}
class D2 extends Base {
  public function foo(string $x, string& $y) { return $x; }
}

function main(Base $b, string $l, bool $ref) {
  $x = $ref ? $b->foo('a', &$l) : $b->foo('a', $l);
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_006() {
$l = (string)mt_rand();
main(new D2, $l, true);
main(new D1, $l, false);
}

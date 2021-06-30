<?hh

class X {
  protected $prot_over_prot = 1;
  public $pub_over_pub = 2;
  protected $pub_over_prot = 3;
}
class Y extends X {
  protected $prot_over_prot = 4;
  public $pub_over_pub = 5;
  public $pub_over_prot = 6;
  protected $prot_base = 7;
  public $pub_base = 8;
}
class Z extends Y {
  public $prot_over_prot = 9;
  public $pub_over_pub = 10;
  public $pub_over_prot = 11;
  public $prot_base = 12;
  public $pub_base = 13;
}
function foo($x) {
  $s = serialize($x);
  var_dump($s);
  $y = unserialize($s);
  var_dump($y);
  if (function_exists('apc_store')) {
    apc_store('foo', $y);
    $z = __hhvm_intrinsics\apc_fetch_no_check('foo');
  }
 else {
    $z = clone $y;
  }
  var_dump($z);
  unset($z, $y);
  var_dump($x);
}

<<__EntryPoint>>
function main_697() {
$y = new Y;
foo($y);
$z = new Z;
foo($z);
unset($z, $y);
}

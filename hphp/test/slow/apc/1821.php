<?hh

class A {
  public function __construct($i, $j, $k) {
    $this->a = $i * $i;
    $this->b = $j * $j;
    $this->c = $k * $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aa';
  protected $bb = false;
  private $cc = 1.22;
}
class B extends A {
  public function __construct($i, $j, $k) {
    $this->a = $i + $i;
    $this->b = $j + $j;
    $this->c = $k + $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aaa';
  protected $bb = 4;
  private $cc = 1.222;
}
class C extends B {
  public function __construct($i, $j, $k) {
    $this->a = $i + $i + $i;
    $this->b = $j + $j + $j;
    $this->c = $k + $k + $k;
  }
  public $a;
  protected $b;
  private $c;
  public $aa = 'aaaa';
  protected $bb = 40;
  private $cc = 1.333;
}
class D extends C {
  public function __construct($i, $j, $k) {
    $this->a = $i + $i + $i;
    $this->b = $j + $j + $j;
    $this->c = $k + $k + $k;
  }
  public $a;
  public $b;
  private $c;
  public $aa = 'aaaaa';
  public $bb = 400;
  private $cc = 1.3333;
}
function foo() :mixed{
  $obj = new A(111, 222, 333);
  apc_store('foobar', $obj);
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  var_dump($obj);
  $obj = new B(111, 222, 333);
  apc_store('foobar', $obj);
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  var_dump($obj);
  $obj = new C(111, 222, 333);
  apc_store('foobar', $obj);
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  var_dump($obj);
  $obj = new D(111, 222, 333);
  apc_store('foobar', $obj);
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  $obj = __hhvm_intrinsics\apc_fetch_no_check('foobar');
  var_dump($obj);
}

<<__EntryPoint>>
function main_1821() :mixed{
foo();
}

<?hh

// With __construct method
class C<reify C1, reify C2> {
  public function __construct() {}
  public function g<reify T1>(){
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
  }
  public function f<reify T1>(){
    $this->g<reify (C1, (C2, int), T1)>();
  }
}

// No __construct method
class D<reify C1, reify C2> {
  public function g<reify T1>(){
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
  }
  public function f<reify T1>(){
    $this->g<reify (C1, (C2, int), T1)>();
  }
}

$c = new C<reify int, reify string>();
$c->f<reify int>();

$d = new D<reify int, reify string>();
$d->f<reify int>();

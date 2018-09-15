<?hh

// With __construct method
class C<reified C1, reified C2> {
  public function __construct() {}
  public function g<reified T1>(){
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
  }
  public function f<reified T1>(){
    $this->g<reified (C1, (C2, int), T1)>();
  }
}

// No __construct method
class D<reified C1, reified C2> {
  public function g<reified T1>(){
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
  }
  public function f<reified T1>(){
    $this->g<reified (C1, (C2, int), T1)>();
  }
}

$c = new C<reified int, reified string>();
$c->f<reified int>();

$d = new D<reified int, reified string>();
$d->f<reified int>();

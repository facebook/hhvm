<?hh

// With __construct method
class C<reify C1, reify C2> {
  public function __construct() {}
  public function g<reify T1>(){
    var_dump(HH\ReifiedGenerics\getType<T1>());
  }
  public function f<reify T1>(){
    $this->g<(C1, (C2, int), T1)>();
  }
}

// No __construct method
class D<reify C1, reify C2> {
  public function g<reify T1>(){
    var_dump(HH\ReifiedGenerics\getType<T1>());
  }
  public function f<reify T1>(){
    $this->g<(C1, (C2, int), T1)>();
  }
}

$c = new C<int, string>();
$c->f<int>();

$d = new D<int, string>();
$d->f<int>();

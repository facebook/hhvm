<?hh

class A {
  public $v = 10;
  function f() {
    $this->v = 100;
  }
}

<<__EntryPoint>>
function main() {
  $a = varray[varray[1, 2, 3], new A()];
  apc_store('0', $a);
  $b = __hhvm_intrinsics\apc_fetch_no_check(0);
  var_dump($b[1]->v);
  $b[1]->f();
  var_dump($b[1]->v);
  $b[2] = 1;
  var_dump($b[1]->v);
}

<?hh

class A {
  public $v = 10;
  function f() :mixed{
    $this->v = 100;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = vec[vec[1, 2, 3], new A()];
  apc_store('0', $a);
  $b = __hhvm_intrinsics\apc_fetch_no_check(0);
  var_dump($b[1]->v);
  $b[1]->f();
  var_dump($b[1]->v);
  $b[] = 1;
  var_dump($b[1]->v);
}

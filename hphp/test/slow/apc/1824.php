<?hh

class A {
  public function gen($a, $b) :AsyncGenerator<mixed,mixed,void>{
    yield $a;
    yield $b;
  }
}


<<__EntryPoint>>
function main_1824() :mixed{
$x = new A;
$x->cache_gen = $x->gen('a', 'b');
foreach ($x->cache_gen as $v) {
 var_dump($v);
 }
apc_store('key', $x);
$y = __hhvm_intrinsics\apc_fetch_no_check('key');
var_dump($y->cache_gen);
}

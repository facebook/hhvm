<?hh
class X {
  use LazyIterable;
  public function getIterator() { yield null; }
}
function test() {
  $x = new X;
  var_dump($x->lazy());
}

<<__EntryPoint>>
function main_sys_trait() {
test();
}

<?hh
class X {
  use LazyIterable;
  public function getIterator() :AsyncGenerator<mixed,mixed,void>{ yield null; }
}
function test() :mixed{
  $x = new X;
  var_dump($x->lazy());
}

<<__EntryPoint>>
function main_sys_trait() :mixed{
test();
}

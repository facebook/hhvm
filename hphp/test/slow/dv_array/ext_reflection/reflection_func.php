<?hh

<<A(1, 2), B>>
function foo($a, $b = null, $c = null): void {
  var_dump($a);
}

class Foo {
  <<Bar>>
  public function a(): void {}
}

class Baz extends Foo {
  <<Bing>>
  public function a($a): void {
    var_dump($a);
  }
}


<<__EntryPoint>>
function main() :mixed{
  $foo_fn = new ReflectionFunction('foo');
  var_dump($foo_fn->getAttributes());
  var_dump($foo_fn->getParameters());
  var_dump($foo_fn->getReturnType());
  $foo_fn->invoke(vec[1]);
  $a_meth = new ReflectionMethod(Baz::class, 'a');
  var_dump($a_meth->getAttributes());
  var_dump($a_meth->getParameters());
  $a_meth->invokeArgs(new Baz(), vec[2]);
}

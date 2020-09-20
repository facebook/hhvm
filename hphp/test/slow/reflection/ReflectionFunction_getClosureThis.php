<?hh

class Foo {
  public function createcl() {
    return function () {};
  }
}

<<__EntryPoint>>
function main_reflection_function_get_closure_this() {
  $closure = function () {};
  $closure_with_this = (new Foo())->createcl();
  var_dump((new ReflectionFunction($closure))->getClosureThis());
  var_dump((new ReflectionFunction($closure_with_this))->getClosureThis());
}

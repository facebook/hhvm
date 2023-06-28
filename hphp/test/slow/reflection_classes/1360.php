<?hh

class Base {
  function foo() :mixed{
    $m = new ReflectionMethod(get_class($this), 'bar');
    var_dump($m->name);
  }
}
<<__EntryPoint>>
function entrypoint_1360(): void {
  $condition = 123;
  if ($condition) {
    include '1360-1.inc';
  } else {
    include '1360-2.inc';
  }
  include '1360-class.inc';
  $obj = new B();
  $obj->foo();
}

<?hh

function foo() {
 echo 'Caught';
 exit;
}
class X {
  function foo() {
    var_dump($this);
  }
}
class Y {
  <<__DynamicallyCallable>> function bar(X $a) {
    $a->foo();
  }
}
function test($y,$z) {
  $y->$z($y);
}

<<__EntryPoint>>
function main_735() {
  set_error_handler(foo<>, E_ALL);
  test(new Y, 'bar');
}

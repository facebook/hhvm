<?hh

function foo() :mixed{
 echo 'Caught';
 exit;
}
class X {
  function foo() :mixed{
    var_dump($this);
  }
}
class Y {
  <<__DynamicallyCallable>> function bar(X $a) :mixed{
    $a->foo();
  }
}
function test($y,$z) :mixed{
  $y->$z($y);
}

<<__EntryPoint>>
function main_735() :mixed{
  set_error_handler(foo<>, E_ALL);
  test(new Y, 'bar');
}

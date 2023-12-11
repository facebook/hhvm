<?hh

class Foo {
  function __construct() {
    $count = -1;
    $s = 'preg_replace() is broken';
    var_dump(preg_replace_callback(
               '/broken/',
               vec[$this, 'bar'],
               $s,
               -1,
               inout $count
             ));
  }

  <<__DynamicallyCallable>> function bar() :mixed{
    return 'working';
  }
}

<<__EntryPoint>>
function main() :mixed{
  $o = new Foo;
}

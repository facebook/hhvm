<?hh

class Foo {
  function __construct() {
    $count = -1;
    $s = 'preg_replace() is broken';
    var_dump(preg_replace_callback(
               '/broken/',
               array($this, 'bar'),
               $s,
               -1,
               inout $count
             ));
  }

  function bar() {
    return 'working';
  }
}

<<__EntryPoint>>
function main() {
  $o = new Foo;
}

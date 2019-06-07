<?hh

class Foo {
  function __construct() {
    $s = 'preg_replace() is broken';
    var_dump(preg_replace_callback(
               '/broken/',
               array($this, 'bar'),
               $s
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

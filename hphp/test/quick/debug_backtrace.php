<?php // uses RenameFunction to ensure that all arguments are retained

class A {
  static function foo() {
    var_dump(debug_backtrace());
  }

  function bar($a, $b, $c = null) {
    $this->foo();
  }
}

function bar() {
  $a = new A();
  $a->bar(1, "str", array(1, 2, 3));
  hphp_invoke_method($a, "A", "bar", array(1, 2));
  hphp_invoke_method($a, "A", "bar", Map {'a' => 1, 'b' => 2});
}
function foo() {
  call_user_func("bar");
}

function error_handler($errno, $errstr, $errfile, $errline, $errcontext) {
  // Make sure this function shows up in a backtrace
  var_dump(debug_backtrace());
}

function main() {
  foo();

  set_error_handler('error_handler');
  return FakeConstant;
}
main();

<?hh

// Make sure that we can tell which class was called for intercepted static
// methods

class A {
  public static function foo() {
    echo 'foo called';
  }
}

class B extends A {
 }


<<__EntryPoint>>
function main_1201() {
fb_intercept('A::foo', function($_1, $called_on, inout $_3, $_4, inout $_5) {
  var_dump($called_on);
}
);

A::foo();
B::foo();

// Trigger run_intercept_handler_for_invokefunc codepath
$class = 'B';
$c = 'call_user_fun';
$c .= 'c';
$c(varray[$class, 'foo']);
}

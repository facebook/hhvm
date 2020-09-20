<?hh

class X {
 public static $g;
 public static function foo() {
 return self::$g;
 }
}
function bar($_1, $_2, inout $_3, $_4, inout $_5) {
  var_dump('Intercepted');
}
function test() {
  X::foo();
  fb_intercept('X::foo', 'bar', 'bar');
  X::foo();
}

<<__EntryPoint>>
function main_1200() {
test();
}

<?hh

class X {
 public static $g;
 public static function foo() {
 return self::$g;
 }
}
function bar($_1, $_2, inout $_3) {
  var_dump('Intercepted');
  return shape('value' => null);
}
function test() {
  X::foo();
  fb_intercept2('X::foo', bar<>);
  X::foo();
}

<<__EntryPoint>>
function main_1200() {
test();
}

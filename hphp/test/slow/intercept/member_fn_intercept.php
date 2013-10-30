<?hh

class A {
  public static function foo() {
   var_dump('failed');
   return 12;
  }
}

class lol { public function __destruct() { echo "lol\n"; } }
class B {
  public static function & bar() {
    var_dump(func_get_args());
    $x = new lol();
    return $x;
  }
}

fb_intercept('A::foo', 'B::bar', "hello");

function main() {
  var_dump(A::foo(1,2));
  $l = A::foo(1,2);
}

main();
echo "done\n";


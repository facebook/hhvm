<?hh

  trait T {
 public static function foo() {
 echo "Foo\n";
 }
 }
  class C {
 use T;
 }
$x = 1;

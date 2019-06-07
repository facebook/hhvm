<?hh

trait T {
  public static function m() {
 echo "original
";
 }
}
class A {
 use T;
 }
class B {
 use T;
 }

<<__EntryPoint>>
function main_2079() {
T::m();
A::m();
fb_intercept("T::m", function() {
 echo "new
";
 }
);
A::m();
B::m();
T::m();
}

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
fb_intercept("T::m", function($_1, $_2, inout $_3, $_4, inout $_5) {
 echo "new
";
 }
);
A::m();
B::m();
T::m();
}

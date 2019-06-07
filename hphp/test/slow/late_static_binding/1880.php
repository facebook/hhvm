<?hh

function f() {
 return true;
}
if (f()) {
  include '1880-1.inc';
} else {
  include '1880-2.inc';
}
class B extends A {
 static $a = 'B';
}
$b = new B;
$b->g();

<?hh

trait T {
 function bar() {
 yield 1;
 }
 }
class X {
 use T;
 }
function test() {
  $r = new ReflectionClass('X');
  foreach ($r->getMethods() as $m) {
    var_dump($m->name);
  }
}

<<__EntryPoint>>
function main_1342() {
test();
}

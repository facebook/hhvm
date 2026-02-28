<?hh

trait T {
 function bar() :AsyncGenerator<mixed,mixed,void>{
 yield 1;
 }
 }
class X {
 use T;
 }
function test() :mixed{
  $r = new ReflectionClass('X');
  foreach ($r->getMethods() as $m) {
    var_dump($m->name);
  }
}

<<__EntryPoint>>
function main_1342() :mixed{
test();
}

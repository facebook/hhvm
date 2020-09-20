<?hh

trait T {
  function hello() {
 echo "Hello from T!
";
 }
}
class B {
  use T;
  function hello() {
 echo "Hello from B!
";
 }
}
class C extends B {
}
class D extends C {
}

<<__EntryPoint>>
function main_2119() {
$ob = new B();
$ob->hello();
$oc = new C();
$oc->hello();
$od = new D();
$od->hello();
}

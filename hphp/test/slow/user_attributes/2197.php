<?php

<< Foo1, Bar1(), Baz1('blah',array(1,2)) >> interface I {
}
<< Foo2, Bar2(), Baz2('blah',array(1,2)) >> trait T {
}
<< Foo3, Bar3(), Baz3('blah',array(1,2)) >> function f() {
}
<< Foo4, Bar4(), Baz4('blah',array(1,2)) >>
function g() {
 yield null;
 }
class C {
  << Foo5, Bar5(), Baz5('blah',array(1,2)) >>
  public function f() {
}
  << Foo6, Bar6(), Baz6('blah',array(1,2)) >>
  public function g() {
 yield null;
 }
}
echo "Done\n";

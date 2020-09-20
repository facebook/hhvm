<?hh

<< Foo1, Bar1(), Baz1('blah',varray[1,2]) >> interface I {
}
<< Foo2, Bar2(), Baz2('blah',varray[1,2]) >> trait T {
}
<< Foo3, Bar3(), Baz3('blah',varray[1,2]) >> function f() {
}
<< Foo4, Bar4(), Baz4('blah',varray[1,2]) >>
function g() {
 yield null;
 }
class C {
  << Foo5, Bar5(), Baz5('blah',varray[1,2]) >>
  public function f() {
}
  << Foo6, Bar6(), Baz6('blah',varray[1,2]) >>
  public function g() {
 yield null;
 }
}

<<__EntryPoint>>
function main_2197() {
echo "Done\n";
}

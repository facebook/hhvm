<?hh

<< Foo1, Bar1(), Baz1('blah',varray[1,2]) >> interface I {
}
<< Foo2, Bar2(), Baz2('blah',varray[1,2]) >> trait T {
}
<< Foo3, Bar3(), Baz3('blah',varray[1,2]) >> function f() :mixed{
}
<< Foo4, Bar4(), Baz4('blah',varray[1,2]) >>
function g() :AsyncGenerator<mixed,mixed,void>{
 yield null;
 }
class C {
  << Foo5, Bar5(), Baz5('blah',varray[1,2]) >>
  public function f() :mixed{
}
  << Foo6, Bar6(), Baz6('blah',varray[1,2]) >>
  public function g() :AsyncGenerator<mixed,mixed,void>{
 yield null;
 }
}

<<__EntryPoint>>
function main_2197() :mixed{
echo "Done\n";
}

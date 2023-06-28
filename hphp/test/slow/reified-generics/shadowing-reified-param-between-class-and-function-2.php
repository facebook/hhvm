<?hh

class Foo<T> {
  function foo4<<<__Enforceable>> reify T>(T $x) :mixed{
    var_dump($x);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $o = new Foo;
  $o->foo4<string>(1);
}

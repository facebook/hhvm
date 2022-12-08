<?hh

class C {
  <<MyAttr('a', 'b'), __Memoize(#KeyedByIC)>>
  function foo() {}
}


<<__EntryPoint>>
function main() {
  var_dump((new ReflectionMethod(C::class, 'foo'))->getAttributes());
}

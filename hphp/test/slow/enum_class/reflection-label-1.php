<?hh

class C {
  <<MyAttr('a', 'b'), __Memoize(#KeyedByIC)>>
  function foo() :mixed{}
}


<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionMethod(C::class, 'foo'))->getAttributes());
}

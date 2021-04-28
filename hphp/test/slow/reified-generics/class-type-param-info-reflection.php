<?hh

class Foobar {}
class FoobarGenerics<T1, T2>{}
class FoobarReified<reify T1, T2, <<__Warn, __Soft>> reify T3> {}

<<__EntryPoint>>
function main(): void {
  var_dump((new ReflectionClass('Foobar'))->getReifiedTypeParamInfo());
  var_dump(
    (new ReflectionClass('FoobarGenerics'))->getReifiedTypeParamInfo()
  );
  var_dump(
    (new ReflectionClass('FoobarReified'))->getReifiedTypeParamInfo()
  );
}

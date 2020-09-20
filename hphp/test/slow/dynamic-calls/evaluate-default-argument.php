<?hh

const FOO = 42;

<<__EntryPoint>>
function test(): void {
  $f = ($x = FOO + 1) ==> {};
  var_dump((new ReflectionFunction($f))->getParameters()[0]->getDefaultValue());
}

<?hh

function foo(BuiltinPseudoConstantMethod $method): void {
  echo __METHOD__;
  echo "\n";
  echo $method;
}

<<__EntryPoint>>
function main(): void {
  foo(__METHOD__);
}

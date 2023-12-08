<?hh

function expectFunctionRef(HH\FunctionRef<(function(int):string)> $f): string {
  var_dump($f);
  return ($f)(3);
}

function top(int $x):string {
  return "A";
}

<<__EntryPoint>>
function main():void {
  expectFunctionRef(top<>);
}

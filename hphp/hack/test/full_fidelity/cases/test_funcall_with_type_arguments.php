<?hh // strict

function foo<T1>(T1 $x) : T1 {
  return $x;
}

function bar() : string {
  return $foo<string>("hi");
}

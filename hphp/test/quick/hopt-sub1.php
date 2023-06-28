<?hh

function foo() :mixed{
  $x = 5;
  $y = 3;
  return $x - $y;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}

<?hh

function foo($x) :mixed{
  $x = $x + 1;
  return $x;
}

function main() :mixed{
  $y = foo(1);
  return $y;
}
<<__EntryPoint>> function main_entry(): void {
var_dump(main());
}

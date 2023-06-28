<?hh

function foo() :mixed{
  $x = 123;
  var_dump((string)$x);
  $y = -456;
  var_dump((string)$y);
}
<<__EntryPoint>>
function main_entry(): void {

  foo();
}

<?hh

function f() :mixed{
  return 2 + 3 + 4;
}

function g() :mixed{
  return 1 + 7;
}

<<__EntryPoint>> function main(): void {
$x = 2 + 3;
if($x < 4) {
  $y = 1 + 2 * 3 / 4 % 5;
} else {
  $y = 6 + 7 - 8 * 9 / 10 % 11;
}

echo "$y " . f() . " " . g() . "\n";
}

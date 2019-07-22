<?hh
function foo(&$x, &$y) { $x = 1; echo $y ; }

function foo2($x, &$y, $z)
{
  echo $x; // 0
  echo $y; // 1
  $y = 2;
}

<<__EntryPoint>> function main(): void {
$x = 0;
foo(&$x, &$x); // prints 1 ..

$x = 0;

foo2($x, &$x, $x = 1);
echo $x; // 2
}

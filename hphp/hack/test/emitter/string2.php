<?hh // strict

function test(): void {
  $x = 'hello';
  $y = 10;
  echo "hello$y\n";
  echo "$x, world\n";
  echo "$x$y";
  echo "a $x b $y c\n";
  echo "{btyfd\n";
  $a = varray[10,20];
  echo "{$a[0]}\n";
  echo "{ { {";
  echo "\n";

  var_dump("$y$y");
}

<?hh

function f1($b)
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b = STDOUT;

    echo "After '\$b = STDOUT', \$b is $b\n";
}

function f2()
{
    $b = STDOUT;

    echo "After '\$b = STDOUT', \$b is $b\n";

    return $b;
}

//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  ///*
  echo "----------------- resource value assignment ----------------------\n";

  $a = STDIN;

  echo "After '\$a = STDIN', \$a is $a\n";

  $b = $a;

  echo "After '\$b = \$a', \$b is $b\n";

  $a = STDOUT;

  echo "After '\$a = STDOUT', \$b is $b, and \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value argument passing ----------------------\n";

  $a = STDIN;

  echo "After '\$a = STDIN', \$a is $a\n";

  f1($a);

  echo "After 'f1(\$a)', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value returning ----------------------\n";

  $a = f2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
  //*/
}

<?hh

function f1($b)
{
    $b__str = (string)($b);
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b__str\n";

    $b = STDOUT;

    $b__str = (string)($b);

    echo "After '\$b = STDOUT', \$b is $b__str\n";
}

function f2()
{
    $b = STDOUT;

    $b__str = (string)($b);

    echo "After '\$b = STDOUT', \$b is $b__str\n";

    return $b;
}

//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  ///*
  echo "----------------- resource value assignment ----------------------\n";

  $a = STDIN;

  $a__str = (string)($a);

  echo "After '\$a = STDIN', \$a is $a__str\n";

  $b = $a;

  $b__str = (string)($b);

  echo "After '\$b = \$a', \$b is $b__str\n";

  $a = STDOUT;

  $b__str = (string)($b);

  $a__str = (string)($a);

  echo "After '\$a = STDOUT', \$b is $b__str, and \$a is $a__str\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value argument passing ----------------------\n";

  $a = STDIN;

  $a__str = (string)($a);

  echo "After '\$a = STDIN', \$a is $a__str\n";

  f1($a);

  $a__str = (string)($a);

  echo "After 'f1(\$a)', \$a is $a__str\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value returning ----------------------\n";

  $a = f2();

  $a__str = (string)($a);

  echo "After '\$a = f2()', \$a is $a__str\n";
  echo "Done\n";
  //*/
}

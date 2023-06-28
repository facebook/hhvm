<?hh

function f1($b)
:mixed{
    $b__str = (string)($b);
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b__str\n";

    $b = HH\stdout();

    $b__str = (string)($b);

    echo "After '\$b = HH\\stdout()', \$b is $b__str\n";
}

function f2()
:mixed{
    $b = HH\stdout();

    $b__str = (string)($b);

    echo "After '\$b = HH\\stdout()', \$b is $b__str\n";

    return $b;
}

//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  ///*
  echo "----------------- resource value assignment ----------------------\n";

  $a = HH\stdin();

  $a__str = (string)($a);

  echo "After '\$a = HH\\stdin()', \$a is $a__str\n";

  $b = $a;

  $b__str = (string)($b);

  echo "After '\$b = \$a', \$b is $b__str\n";

  $a = HH\stdout();

  $b__str = (string)($b);

  $a__str = (string)($a);

  echo "After '\$a = HH\\stdout()', \$b is $b__str, and \$a is $a__str\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value argument passing ----------------------\n";

  $a = HH\stdin();

  $a__str = (string)($a);

  echo "After '\$a = HH\\stdin()', \$a is $a__str\n";

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

<?hh

function byRefAssignmentResource(&$a, &$c) {
  $a = STDIN;

  echo "After '\$a = STDIN', \$a is $a\n";

  $a = STDOUT;    // this causes $c to also alias 99

  echo "After '\$a = STDOUT', \$c is $c, and \$a is $a\n";

  unset($a);

  echo "After 'unset(\$a)', \$c is $c, and \$a is undefined\n";
}

function f1($b)
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b = STDOUT;

    echo "After '\$b = STDOUT', \$b is $b\n";
}

function g1(&$b)
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

function g2()
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
  echo "----------------- resource byRef assignment ----------------------\n";
  byRefAssignmentResource(&$a, &$a);
  unset($a);
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
  echo "----------------- resource byRef argument passing ----------------------\n";

  $a = STDIN;

  echo "After '\$a = STDIN', \$a is $a\n";

  g1(&$a);

  echo "After 'g1(\$a)', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource value returning ----------------------\n";

  $a = f2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- resource byRef returning ----------------------\n";

  $a = g2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
}

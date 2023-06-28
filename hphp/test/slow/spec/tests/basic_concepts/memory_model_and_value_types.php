<?hh

const CON = TRUE;

function f1($b)
:mixed{
    $b__str = (string)($b);
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b__str\n";

    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";
}

function f2()
:mixed{
    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";

    return $b;
}

function h1()
:mixed{
    $b = 10;
    return $b + 5;
//  return 12;
//  return CON;
}
//*/

///*
function h2()
:mixed{
    $b = 10;
//  return $b + 5;  // Only variable references should be returned by reference
//  return 12;
//  return CON;
}
//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  ///*
  echo "----------------- simple assignment of value types ----------------------\n";

  $a = 123;

  echo "After '\$a = 123', \$a is $a\n";

  $b = $a;

  echo "After '\$b = \$a', \$b is $b\n";

  ++$b;

  echo "After '++\$b', \$b is $b, and \$a is $a\n";

  $a = 99;

  echo "After '\$a = 99', \$b is $b, and \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- value argument passing of value types ----------------------\n";

  $a = 123;

  echo "After '\$a = 123', \$a is $a\n";

  f1($a);

  echo "After 'f1(\$a)', \$a is $a\n";

  f1($a + 2);     // non-lvalue
  f1(999);        // non-lvalue
  f1(CON);        // non-lvalue
  echo "Done\n";
  //*/

  ///*
  echo "----------------- value returning of value types ----------------------\n";

  $a = f2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----- test using literals, constants, and arbitrary-complex expressions ----\n";

  echo "h1() is " . h1() . "\n";

  h2();
  echo "Done\n";
}

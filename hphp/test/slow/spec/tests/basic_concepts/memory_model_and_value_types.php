<?hh

const CON = TRUE;

function byRefAssignmentValue(&$a, &$c) {
  $a = 123;

  echo "After '\$a = 123', \$a is $a\n";

  ++$c;

  echo "After '++\$c', \$c is $c, and \$a is $a\n";

  $a = 99;

  echo "After '\$a = 99', \$c is $c, and \$a is $a\n";

  unset($a);

  echo "After 'unset(\$a)', \$c is $c, and \$a is undefined\n";
}

function f1($b)
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";
}

function g1(&$b)
{
    echo "\tInside function " . __FUNCTION__ . ", \$b is $b\n";

    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";
}

function f2()
{
    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";

    return $b;
}

function g2()
{
    $b = "abc";

    echo "After '\$b = \"abc\"', \$b is $b\n";

    return $b;
}

//$a =& 12;     // literals are disallowed
//$a =& CON;    // constants are disallowed
//$a =& $b;     // by-ref assignments are disallowed
//$a =& 5 + $b; // arbitrary-complex expressions are disallowed

function h1()
{
    $b = 10;
    return $b + 5;
//  return 12;
//  return CON;
}
//*/

///*
function h2()
{
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
  echo "----------------- byRef assignment of value types ----------------------\n";
  byRefAssignmentValue(&$a, &$a);
  unset($a);
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
  echo "-----------------  byRef argument passing of value types ----------------------\n";

  $a = 123;

  echo "After '\$a = 123', \$a is $a\n";

  g1(&$a);

  echo "After 'g1(\$a)', \$a is $a\n";

  //g1($a + 2);       // non-lvalue; can't be passed by reference
  //g1(999)           // non-lvalue; can't be passed by reference
  //g1(CON);          // non-lvalue; can't be passed by reference
  echo "Done\n";
  //*/

  ///*
  echo "----------------- value returning of value types ----------------------\n";

  $a = f2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----------------- byRef returning of value types ----------------------\n";

  $a = g2();

  echo "After '\$a = f2()', \$a is $a\n";
  echo "Done\n";
  //*/

  ///*
  echo "----- test using literals, constants, and arbitrary-complex expressions ----\n";

  echo "h1() is " . h1() . "\n";

  h2();
  echo "Done\n";
}

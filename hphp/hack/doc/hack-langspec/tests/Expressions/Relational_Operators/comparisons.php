<?hh // strict

namespace NS_comparisons;

function doit1(num $p1_num, ?int $p2_nint): void {
/*
  echo "====== checkout the type and value of the result ======\n\n";

  $a = 10 < 20;
  var_dump($a);	// bool(true)
  $a = 10 >= 20;
  var_dump($a);	// bool(false)	
  $a = "zz" > "xx";
  var_dump($a);	// bool(true)
*/

/*
// all incompatible pairs of types

  var_dump(null > true);
  var_dump(true >= 100);
  var_dump(null < 1.23);
  var_dump(false <= "123");

  var_dump($p1_num > $p2_nint);
  var_dump($p1_num >= $p2_nint);
  var_dump($p1_num < $p2_nint);
  var_dump($p1_num <= $p2_nint);
*/

/*
  echo "\n====== compare arithmetic ops ======\n\n";

  var_dump(100 > 12.3);		// true
  var_dump(100 >= $p1_num);	// true
  var_dump($p1_num < 5.6);	// false when $p1_num == 11
*/

/*
  echo "\n====== compare Boolean ops ======\n\n";

// surprise; Boolean comparisons are ordered!

  var_dump(true > false);		// true
  var_dump(true < false);		// false
  var_dump(false <= true);	// true
*/

/*
  echo "\n====== compare null ops ======\n\n";

// surprise; this compiles!

  var_dump(null < null);		// false
  var_dump(null > null);		// false
*/

/*
  echo "\n====== compare null with nullable ======\n\n";

// hmmm
// when $p2_nint == null

  var_dump($p2_nint);		// null
  var_dump(null > $p2_nint);	// false
  var_dump(null >= $p2_nint);	// true
  var_dump(null < $p2_nint);	// false
  var_dump(null <= $p2_nint);	// true

// when $p2_nint != null and non-zero, get false, false, true, true, so is doing numeric comparison

// when $p2_nint != null and is zero, get false, true, false, true, so can't distinguish between null and zero
*/

/*
  echo "\n====== compare numeric-string ops ======\n\n";

  var_dump('123' <= '4');		// false; is doing a numeric comparison
  var_dump('X123' <= 'X4');	// true; is doing a string comparison
  var_dump(123 <= 4);		// false; numeric comparison
// var_dump('123' <= 4);		// incompatible types
//  var_dump(123 <= '4');		// incompatible types
*/

/*
  echo "\n====== compare non-numeric-string ops ======\n\n";

  $oper1 = array("", "a", "aa", "a0", "aA");
  $oper2 = array("", "ab", "abc", "A", "AB");

  foreach ($oper1 as $e1) {
    foreach ($oper2 as $e2) {
      echo "{$e1} >  {$e2}  result: "; var_dump($e1 > $e2);
      echo "{$e2} <= {$e1}  result: "; var_dump($e2 <= $e1);
      echo "---\n";
      echo "{$e1} >= {$e2}  result: "; var_dump($e1 >= $e2);
      echo "{$e2} <  {$e1}  result: "; var_dump($e2 < $e1);
      echo "---\n";
      echo "{$e1} <  {$e2}  result: "; var_dump($e1 < $e2);
      echo "{$e2} >= {$e1}  result: "; var_dump($e2 >= $e1);
      echo "---\n";
      echo "{$e1} <= {$e2}  result: "; var_dump($e1 <= $e2);
      echo "{$e2} >  {$e1}  result: "; var_dump($e2 > $e1);
      echo "=======\n";
    }
    echo "-------------------------------------\n";
  }
*/

/*
  echo "\n====== compare resource ops ======\n\n";

  $infile1 = fopen("Testfile1.txt", 'r');
  $infile2 = fopen("Testfile2.txt", 'r');
  var_dump($infile1, $infile2);

// surprise; this compiles!

  var_dump($infile1 > $infile2);	// false (4 > 5)
  var_dump($infile1 < $infile2);	// true (4 < 5)
*/
}

function doit2(array<int> $p1, array<int> $p2, array<float> $p3): void {
  var_dump($p1, $p2);
  echo "{\$p1} >  {\$p2}  result: "; var_dump($p1 > $p2);
  echo "{\$p2} <= {\$p1}  result: "; var_dump($p2 <= $p1);
  echo "---\n";
  echo "{\$p1} >= {\$p2}  result: "; var_dump($p1 >= $p2);
  echo "{\$p2} <  {\$p1}  result: "; var_dump($p2 < $p1);
  echo "---\n";
  echo "{\$p1} <  {\$p2}  result: "; var_dump($p1 < $p2);
  echo "{\$p2} >= {\$p1}  result: "; var_dump($p2 >= $p1);
  echo "---\n";
  echo "{\$p1} <= {\$p2}  result: "; var_dump($p1 <= $p2);
  echo "{\$p2} >  {\$p1}  result: "; var_dump($p2 > $p1);
  echo "---\n";
  echo "{\$p1} == {\$p2}  result: "; var_dump($p1 == $p2);
  echo "{\$p1} != {\$p2}  result: "; var_dump($p1 != $p2);
  echo "=======\n";
//  var_dump($p3);
//  echo "{\$p3} >  {\$p2}  result: "; var_dump($p3 > $p2);	// correctly rejected
}

function doit3(array<string, int> $p1, array<string, int> $p2): void {
  var_dump($p1, $p2);
  echo "{\$p1} >  {\$p2}  result: "; var_dump($p1 > $p2);
  echo "{\$p2} <= {\$p1}  result: "; var_dump($p2 <= $p1);
  echo "---\n";
  echo "{\$p1} >= {\$p2}  result: "; var_dump($p1 >= $p2);
  echo "{\$p2} <  {\$p1}  result: "; var_dump($p2 < $p1);
  echo "---\n";
  echo "{\$p1} <  {\$p2}  result: "; var_dump($p1 < $p2);
  echo "{\$p2} >= {\$p1}  result: "; var_dump($p2 >= $p1);
  echo "---\n";
  echo "{\$p1} <= {\$p2}  result: "; var_dump($p1 <= $p2);
  echo "{\$p2} >  {\$p1}  result: "; var_dump($p2 > $p1);
  echo "---\n";
  echo "{\$p1} == {\$p2}  result: "; var_dump($p1 == $p2);
  echo "{\$p1} != {\$p2}  result: "; var_dump($p1 != $p2);
  echo "=======\n";
}

function main(): void {
  doit1(11, 0);

/*
  $oper1 = array([], [10,20]);
  $oper2 = array([], [10,20], [10,20,30], [10,30]);

  foreach ($oper1 as $e1) {
    foreach ($oper2 as $e2) {
      doit2($e1, $e2, array(2.5, 6.7));
    }
  }
*/

///*
  doit3(array("red" => 0,"green" => 0), array("red" => 0,"green" => 0));
  doit3(array("red" => 0,"green" => 0), array("green" => 0,"red" => 0));
  doit3(array("red" => 0,"green" => 0), array("green" => 0,"red" => 5));
  doit3(array("red" => 0,"green" => 0), array("green" => -5,"red" => 5));
  doit3(array("red" => 0,"green" => 0), array("green" => -5,"red" => 0));

  doit3(array("red" => 0), array("green" => 0));
  doit3(array("green" => 0), array("red" => 0));
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();

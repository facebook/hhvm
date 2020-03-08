<?hh // strict

namespace NS_equality_comparisons;

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
  echo "====== mix-n-match operand types ======\n\n";

  var_dump(null == false);	// bool(true)
  var_dump(null === false);	// bool(false)

  var_dump(null == true);		// bool(false)
  var_dump(null === true);	// bool(false)

  var_dump(false == 0);		// bool(true)
//  var_dump(false === 0);		// always false

  var_dump(true == 1);		// bool(true)
//  var_dump(true === 1);		// always false

  var_dump(true == 100);		// bool(true)
//  var_dump(true === 100);		// always false

  var_dump(null != 1.23);		// bool(true)
  var_dump(null !== 1.23);	// bool(true)

  var_dump(false != "");		// bool(false)
//  var_dump(false !== "");		// always true

  var_dump(false != "0");		// bool(false)
//  var_dump(false !== "0");	// always true

  var_dump(false != "123");	// bool(true)
//  var_dump(false !== "123");	// always true

  var_dump(false != "X");		// bool(true)
//  var_dump(false !== "X");	// always true

  var_dump($p1_num == $p2_nint);	// bool(false)
  var_dump($p1_num === $p2_nint);	// bool(false)

  var_dump($p1_num != $p2_nint);	// bool(true)
  var_dump($p1_num !== $p2_nint);	// bool(true)
*/

/*
  echo "\n====== compare arithmetic ops ======\n\n";

  var_dump(100 == 12.3);		// false
  var_dump(100 != $p1_num);	// true
  var_dump($p1_num === 5.6);	// false when $p1_num == 11
*/

/*
  echo "\n====== compare Boolean ops ======\n\n";

  var_dump(true == false);	// false
  var_dump(true != false);	// true
  var_dump(false === true);	// false
*/

/*
  echo "\n====== compare null ops ======\n\n";

  var_dump(null === null);	// true
  var_dump(null != null);		// false
*/

/*
  echo "\n====== compare null with nullable ======\n\n";

// when $p2_nint == null

  var_dump($p2_nint);		// null
  var_dump(null == $p2_nint);	// true
  var_dump(null !== $p2_nint);	// false
  var_dump(null != $p2_nint);	// false
  var_dump(null === $p2_nint);	// true

// when $p2_nint != null and non-zero, get false, true, true, false, so is doing numeric comparison

// when $p2_nint != null and is zero, get true, true, false, false, so can't distinguish between null and zero
*/

/*
  echo "\n====== compare numeric-string ops ======\n\n";

  var_dump('123' == '4');		// false
  var_dump('123' == 4);		// false
  var_dump(123 != '4');		// true
  var_dump('123' == 123);		// true; is doing a numeric comparison
*/

/*
  echo "\n====== compare non-numeric-string ops ======\n\n";

  $oper1 = array("", "a", "aa", "a0", "aA");
  $oper2 = array("", "ab", "abc", "A", "AB");

  foreach ($oper1 as $e1) {
    foreach ($oper2 as $e2) {
      echo "{$e1} ==  {$e2}  result: "; var_dump($e1 == $e2);
      echo "{$e2} !=  {$e1}  result: "; var_dump($e2 != $e1);
      echo "{$e1} === {$e2}  result: "; var_dump($e1 === $e2);
      echo "{$e2} !== {$e1}  result: "; var_dump($e2 !== $e1);
    }
    echo "-------------------------------------\n";
  }
*/

/*
  echo "\n====== compare resource ops ======\n\n";

  $infile1 = fopen("Testfile1.txt", 'r');
  $infile2 = fopen("Testfile2.txt", 'r');
  var_dump($infile1, $infile2);

  var_dump($infile1 == $infile2);		// false (4 == 5)
  var_dump($infile1 !== $infile2);	// true (4 !== 5)
*/
}

class Button {}
class CustomButton extends Button {}

function array_proc(
  array<int> $p1, array<int> $p2,
  array<float> $p3, array<num> $p4,
  array<Button> $p5, array<CustomButton> $p6,
  array<int, int> $p7,
  array<int, string> $p8, array<int, string> $p9,
  array<int, bool> $p10,
  array<string, int> $p11, array<string, int> $p12,
  array<bool> $p13
): void {
/*
  echo "============= vector-like arrays ================\n";

  echo "result of p1 == p1 = "; var_dump($p1 == $p1);	// true
  echo "result of p1 === p1 = "; var_dump($p1 === $p1);	// true

  echo "result of p1 == p2 = "; var_dump($p1 == $p2);	// false
  echo "result of p1 == p3 = "; var_dump($p1 == $p3);	// true, 66 -> 66.0
  echo "result of p4 != p1 = "; var_dump($p4 != $p1);	// true
  echo "result of p4 === p3 = "; var_dump($p4 === $p3);	// false
  echo "result of p5 !== p6 = "; var_dump($p5 !== $p6);	// true

  echo "result of p1 == p13 = "; var_dump($p1 == $p13);	// false
//  echo "result of p1 == p5 = "; var_dump($p1 == $p5);	// tries to convert object to int & fails
*/

///*
  echo "qq============= map-like arrays ================\n";

  $v = $p1 == $p7;	// array<int> == array<int, int> is allowed !!!
				// can't mix vector-like and map-like arrays even with same key/value types
  echo "result of p1 == p7 = "; var_dump($v);

  $v = $p8 == $p9;	// array<int, string> == array<int, string> okay
  echo "result of p8 == p9 = "; var_dump($v);

  $v = $p7 == $p10;	// array<int, int> == array<int, bool> is allowed !!!
  echo "result of p7 == p10 = "; var_dump($v);

  $v = $p11 == $p12;	// array<string, int> == array<string, int> okay
  echo "result of p11 == p12 = "; var_dump($v);
//*/

///*
  echo "============= concating to an empty/untyped array ================\n";

  $p = $p1 == array();	// array<int> == array() allowed;
  echo "result of p1 == array() = "; var_dump($p);
  $p = array() == $p2;	// array<int> == array() allowed;
  echo "result of array() == p1 = "; var_dump($p);

  $v = $p10 == array();	// array<int, bool> == array() allowed;
  echo "result of p10 == array() = "; var_dump($v);

  $v = $p11 == array();	// array<string, int> == array() allowed
  echo "result of p11 == array() = "; var_dump($v);

  $p = array() == array();// array() == array() allowed;
  echo "result of array() == array() = "; var_dump($p);
//*/
}

function main(): void {
  doit1(11, null);

  array_proc(
    array(66),
    array(66, 200),
    array(66.0),
    array(66, 2.5),
    array(new Button()), array(new CustomButton()),
    array(6 => 66, 12 => 22),
    array(2 => 'aa', 12 => 'bb'), array(-4 => 'cc', 6 => 'dd'), 
    array(-3 => true),
    array('red' => 12, 'green' => 7), array('blue' => 3),
    array(true, false) 
  );
}

/* HH_FIXME[1002] call to main in strict*/
main();

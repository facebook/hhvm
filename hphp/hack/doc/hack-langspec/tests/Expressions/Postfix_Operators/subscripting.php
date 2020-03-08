<?hh // strict

namespace NS_subscripting;

function f(): array<int> {
  return [1000, 2000, 3000];
}

// See arrays.php for array creation and initialization
// This set of tests deals only with array-element access

function main(): void {
/*
// vector-like array

  $v1 = array(10, 20, 30);
  var_dump($v1);
  echo $v1[0] . "\n";
//  echo $v1[1.2] . "\n";		// index must have type int
//  echo $v1["1"] . "\n";		// index must have type int

  foreach ($v1 as $k => $e) {
    echo "key: " . $k . ", value: " . $e . "\n";
  }

// map-like array

  $v2 = array('x' => 40, 1 => 50, 'y' => 60);
  var_dump($v2);
  echo $v2[0] . "\n";
  echo $v2[1.2] . "\n";		// like PHP index need not have type int
  echo $v2["x"] . "\n";		// like PHP index need not have type int

  foreach ($v2 as $k => $e) {
    echo "key: " . $k . ", value: " . $e . "\n";
  }
*/
/*
// try to access non-existant elements

  $v = array(10, 20, 30);
  echo "[7] contains >".($v[7] == null ? "null" : "??")
    ."<, [12] contains >".($v[12] == null ? "null" : "??")."<\n";

  $v[1] = 1.234;		// change the value (and type) of an existing element
  $v[-10] = 19;		// insert a new element with int key -10
  $v[5] = 54;			// insert a new element with int key 5
  var_dump($v);

  foreach ($v as $k => $e) {
    echo "key: ".$k.", value: ".$e."\n";
  }
*/
/*
  $v = array('x' => 10, 1 => 20, 'y' => 30);
  $v["red"] = true;	// insert a new element with string key "red"
  $v[null] = 232;		// insert a new element with string key ""
  var_dump($v);

  foreach ($v as $k => $e) {
    echo "key: ".$k.", value: ".$e."\n";
  }

  echo $v["red"]." ".$v[null]." ".$v[""]."\n"; // access individual elements by string key
*/
/*
  $v = array(array(2,4,6,8), array(5,10), array(100,200,300));
  var_dump($v);

  foreach ($v as $k => $e) {
    echo "outer key: " . $k . "\n";
    foreach ($e as $k2 => $e2) {
      echo "inner key: ".$k2.", inner value: ".$e2."\n";
    }
  }

  var_dump($v[0]); 
  echo "[0][2]: ".$v[0][2]."\n"; // 6
  echo "[1][1]: ".$v[1][1]."\n"; // 10
*/
/*
// show that associativity of () and [] is left-to-right

  $z = array(array(2,4,6,8), array(5,10), array(100,200,300))[0][2];
  var_dump($z);

  $z = [array(2,4,6,8), array(5,10), array(100,200,300)][0][2];
  var_dump($z);

  $z = [[2,4,6,8], [5,10], [100,200,300]][0][2];	// acceses element with value 6
  var_dump($z);

  var_dump(["black", "white", "yellow"][1]);		// white
  var_dump(["black", "white", "yellow"][1][2]);	// 1st [] is for array, 2nd for string
*/
/*
  var_dump(f()[2]);	// acceses element with value 3000
*/

/*
// checkout order of evaluation

  $z = [[2,4,6,8], [5,10], [100,200,300]];
  $i = 0;
  var_dump($z[$i++][$i]);		// accesses [0][1] L->R int(4)
  $i = 0;
  var_dump($z[$i][$i++]);		// accesses [1][0] R->L int(5)
  $i = 0;
  var_dump($z[++$i][$i]);		// accesses [1][1] L->R int(10)
  $i = 0;
  var_dump($z[$i][++$i]);		// accesses [1][1] R->L int(10)
*/

/*
// unlike PHP, can't subscript scalars
//
//  $z = 10;
//  var_dump($z);
//  $v = $z[12];		// results in null
//  var_dump($v);
//  $v = $z["red"];		// results in null
//  var_dump($v);

  $z = [[2,4,6,8], [5,10], [100,200,300]];
  var_dump($z[0][2]);	// results in 6
//  var_dump($z[0][2][3]);	// results in null

  var_dump(f()[2]);	// results in 3000
//  var_dump(f()[2][1]);	// results in null

  $v = 10;
//  var_dump($v[1]);	// OK, results in null

  $v = 1.23;
//  var_dump($v[1]);	// OK, results in null

  $v = true;
//  var_dump($v[1]);	// OK, results in null

  $v = null;
//  var_dump($v[1]);	// OK, results in null
*/
/*
// subscript some strings

  var_dump("red"[1]);	// OK, results in "e"
  var_dump("red"[-1]);	// OK, results in ""
  var_dump("red"[10]);	// OK, results in ""

// as string[xxx] results in a string, can keep applying more [], indefinitely

  var_dump("red"[0]);		// OK, results in "r"
  var_dump("red"[0][0]);		// OK, results in "r"
  var_dump("red"[0][0][0]);	// OK, results in "r"
  var_dump("red"[0][0][0][0]);	// OK, results in "r"
*/
/*
// change a string

  $s = "red";
  var_dump($s);
  $s[1] = "X";		// OK; "e" -> "X"
  var_dump($s);
  $s[-5] = "Y";		// warning; string unchanged
  var_dump($s);
  $s[5] = "Z";		// extends string, padding with spaces
  var_dump($s);

  echo ">".$s[2]."<\n";
  echo ($s[2] == " ") ? "[2] is a space\n" : "[2] is not a space\n";
  echo ">".$s[3]."<\n";
  echo ($s[3] == " ") ? "[3] is a space\n" : "[3] is not a space\n";
  echo ">".$s[4]."<\n";
  echo ($s[4] == " ") ? "[4] is a space\n" : "[4] is not a space\n";
  echo ">".$s[5]."<\n";
  echo ($s[5] == " ") ? "[5] is a space\n" : "[5] is not a space\n";

  $s[0] = "DEF";		// "r" -> "D"; only 1 char changed
  var_dump($s);
  $s[0] = "MN";		// "D" -> "M"; only 1 char changed
  var_dump($s);
  $s[0] = "";			// "M" -> "\0"
  var_dump($s);
*/
/*
  echo "--------------------\n";

  $v = array();
  $v[] = 10;				// inserts using a key of the next available int value
  $v["XX"] = 3;
  $v[5] = 99;
  $v[] = -2.3;
  $v["AA"] = 234;
  $v[12] = 100;
  $v[] = 'red';
  var_dump($v);
*/
/*
// Hack does not support deprecated {}

  $colors = array("red", "white");
  var_dump($colors);

  var_dump($colors[0]);
  var_dump($colors { 0 } );

  $colors { 1 } = 123;
  var_dump($colors);

  ++$colors{1};
  var_dump($colors);

  $strs = [[10, 20], ["abc", "xyz"]];
  var_dump($strs);
  $strs[0][0] = 1.1;
  $strs[0]{1} = 2.2;
  $strs{1}[0] = 3.3;
  $strs{1}{1} = 4.4;
  var_dump($strs);
*/
}

/* HH_FIXME[1002] call to main in strict*/
main();

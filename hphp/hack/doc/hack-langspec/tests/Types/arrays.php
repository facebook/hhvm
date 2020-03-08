<?hh // strict

namespace NS_arrays2;

class C2a {}
class C2b extends C2a {}
class C2c extends C2b {}

class C3 {}

interface I {}
class C4a implements I {}
class C4b implements I {}

class Button {}
class CustomButton extends Button {}

interface MyCollection {}
class MyList implements MyCollection {}
class MyQueue implements MyCollection {}

class C1 {
///*
// use int or string key types

  private array<string> $a_defint_string = array('a', 'b');
  private array<int,string> $a_int_string = array('a', 'b');
  private array<string,bool> $a_string_bool = array('X1' => true, 'X2' => false);
//*/
///*
// use non-int/non-string key types

  private array<float,int> $a_float_int = array(5.5 => 10, 5.6 => 20);
  private array<bool,int> $a_bool_int = array(true => 10, false => 20);
  private array<num,int> $a_num_int = array(5 => 10, 5.5 => 30);
  private array<mixed,int> $a_mixed_int = array(5 => 10, null => 20, 5.5 => 30);
//*/
/*
// The following is accepted, but one can never add any elements to it. Is there a good use for it?

  private array<void> $a_void = array();

// Discovered to be no longer accepted as of Jan 18, 2016

*/
/*
// HHVM ==> SegFault
//  private array<resource> $a_resource = array(STDIN, STDOUT);
*/
///*
// for completeness, have int keys with values of each built-in type

  private array<bool> $a_bool = array(true, false);
  private array<int> $a_int = array(10, 20);
  private array<float> $a_float = array(3.1, -2.4);
  private array<num> $a_num = array(5.4, 234);
  private array<string> $a_string = array('a', 'b');
  private array<mixed> $a_mixed = array(10, 'b', false);
  private array<array<int>> $a_a_int = array(array(10,20), array(2,3));
  private array<resource> $a_resource;
//*/
///*
// have int keys with values of each nullable type

  private array<?bool> $a_nbool = array(true, null);
  private array<?int> $a_nint = array(10, null);
  private array<?float> $a_nfloat = array(3.1, null);
  private array<?num> $a_nnum = array(5.4, null);
  private array<?string> $a_nstring = array('a', null);
  private array<mixed> $a_nmixed = array(10, 'b', null);
//*/
///*
// Have element value types that are from an inheritance heirarchy

  private array<C2a> $a_C2a = array();
  private array<C2b> $a_C2b = array();
  private array<C2c> $a_C2c = array();
//*/
///*
// Have element value types that are classes implementing the same interface

  private array<I> $a_iI = array();
//*/
///*
// use map-like arrays, and compare with vector-like arrays

  private array<int> $a_int1 = array(10, 20);

// a vector-like array is not compatible with a map-like array
//  private array<int> $a_int2 = array(0 => 10, 1 => 20);

// in an array initializer, can't mix implied key and explicit key
//  private array<int,int> $a_int2 = array(0 => 10, 20);

  private array<int,int> $a_int2 = array(0 => 10, 1 => 20);
  private array<int,int> $a_int3 = array(1 => 20, 0 => 10);
  private array<int,int> $a_int4 = array(5 => 20, -1 => 10);
//*/
  public function __construct() {
    echo "Inside " . __METHOD__ . "\n";
///*
    $this->a_defint_string = array('a', 'b');
    $this->a_int_string = array('a', 'b');
    $this->a_string_bool = array('X1' => true, 'X2' => false);
    $this->a_float_int = array(5.5 => 10, 5.6 => 20);
    $this->a_bool_int = array(true => 10, false => 20);
    $this->a_num_int = array(5 => 10, 5.5 => 30);
    $this->a_mixed_int = array(5 => 10, false => 20, null => 30);

    $this->a_bool = array(true, false);
    $this->a_int = array(10, 20);
    $this->a_float = array(3.1, -2.4);
    $this->a_num = array(5.4, 234);
    $this->a_string = array('a', 'b');
    $this->a_mixed = array(10, 'b', false);
    $this->a_a_int = array(array(10,20), array(2,3));
    $this->a_resource = array(STDIN, STDOUT);

    $this->a_nbool = array(true, null);
    $this->a_nint = array(10, null);
    $this->a_nfloat = array(3.1, null);
    $this->a_nnum = array(5.4, null);
    $this->a_nstring = array('a', null);
    $this->a_nmixed = array(10, 'b', null);

// array type C2a exactly matches that of #1, #2, and #3
    $this->a_C2a = array(new C2a(), new C2a(), new C2a());

// array type C2a exactly matches #1, and is a base type of #2 and #3
    $this->a_C2a = array(new C2a(), new C2b(), new C2c());

// array type C2a is a base type of #1, #2, and #3
    $this->a_C2a = array(new C2b(), new C2b(), new C2b());

// array type C2a is a base type of #1, #2, and #3
    $this->a_C2a = array(new C2c(), new C2c(), new C2c());

// array type C2b exactly matches that of #1, #2, and #3
    $this->a_C2b = array(new C2b(), new C2b(), new C2b());

// array type C2c exactly matches that of #1, #2, and #3
    $this->a_C2c = array(new C2c(), new C2c(), new C2c());

// array type C2b exactly matches that of #1, #2, and is the base of #3
    $this->a_C2b = array(new C2b(), new C2b(), new C2c());

    $this->a_mixed = array(10, 'b', false, new C2b(), new C2c(), new C3());

// Both classes implement interface I
    $this->a_iI = array(new C4a(), new C4b());
//*/
  }

  private function f(): void {
//    $this->a_int = array(10, 20);
//    $this->a_num = array(10, 20);
//    $this->a_mixed = array(10, 20);

//    $this->a_num = array(10, 20.5);
//    $this->a_mixed = array(10, 20.5);
  }

  public function test_conversions(
    array<bool> $p_a_bool1,
    array<bool> $p_a_bool2,
    array<int> $p_a_int1,
    array<int> $p_a_int2
  ): void {
    echo "Inside " . __METHOD__ . "\n";

    $v = (bool)$p_a_bool1;
    echo "result of (bool) = $v\n";
    $v = (bool)$p_a_bool2;
    echo "result of (bool) = $v\n";

    $v = (int)$p_a_int1;
    echo "result of (int) = $v\n";
    $v = (int)$p_a_int2;
    echo "result of (int) = $v\n";

    $v = (string)$p_a_int1;
    echo "result of (string) = "; var_dump($v);
    $v = (string)$p_a_int2;
    echo "result of (string) = "; var_dump($v);
    $v = (string)$p_a_bool2;
    echo "result of (string) = "; var_dump($v);

// Can't cast to a typed array type
//$v = (array<bool>)$p_a_bool1;

/*
// Unbound name: NS_arrays\object

    $v = (object)$p_a_bool2;
    echo "result of (object) = "; var_dump($v); print_r($v);
    $v = (object)$p_a_int2;
    echo "result of (object) = "; var_dump($v); print_r($v);
*/
  }

  public function test_intrinsics(array<int> $p): void {
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p);
    list($min, $max, $avg) = $p;
    echo "\$min = $min, \$max = $max, \$avg = $avg\n";
  }

  public function array_proc(
    array<int> $p1, array<int> $p2,
    array<float> $p3, array<num> $p4,
    array<Button> $p5, array<CustomButton> $p6): void {
    echo "Inside " . __METHOD__ . "\n";

    $v = $p1 + $p2;
    echo "result of p1+p2 = "; var_dump($v);

// can't concat 2 arrays of different types; good

//    $v = $p1 + $p3;
//    echo "result of p1+p3 = "; var_dump($v);

//    $v = $p2 + $p3;
//    echo "result of p2+p3 = "; var_dump($v);

//    $v = $p4 + $p1;   // array<num> + array<int> disallowed
//    $v = $p4 + $p3;   // array<num> + array<float> disallowed

//    $v = $p5 + $p6;   // array<Button> + array<CustomButton> disallowed

    $v = $p1 < $p2;
    echo "result of p1<p2 = "; var_dump($v);
    $v = $p1 <= $p2;
    echo "result of p1<=p2 = "; var_dump($v);
    $v = $p1 > $p2;
    echo "result of p1>p2 = "; var_dump($v);
    $v = $p1 >= $p2;
    echo "result of p1>=p2 = "; var_dump($v);
    $v = $p1 != $p2;
    echo "result of p1!=p2 = "; var_dump($v);

// can't compare 2 arrays of different types; good
//    $v = $p1 < $p3;
//    $v = $p1 <= $p3;
//    $v = $p1 > $p3;
//    $v = $p1 >= $p3;

    $v = $p1 == $p2;
    echo "result of p1==p2 = $v\n";
    $v = $p1 != $p2;
    echo "result of p1!=p2 = $v\n";
//    $v = $p1 <> $p2;    // alternate form of not-equal is not supported
    $v = $p1 === $p2;
    echo "result of p1===p2 = $v\n";
    $v = $p1 !== $p2;
    echo "result of p1!==p2 = $v\n";

    $v = $p1 == $p3;
    echo "result of p1==p3 = $v\n";
    $v = $p1 != $p3;
    echo "result of p1!=p3 = $v\n";
    $v = $p1 === $p3;
    echo "result of p1===p3 = $v\n";
    $v = $p1 !== $p3;
    echo "result of p1!==p3 = $v\n";

    $v = $p1;
    echo "\$v[0] = " . $v[0] . "\n";
// can't subscript a vector-like array with a non-int key
//    echo "\$v['x']  = " . $v['x'] . "\n";
//    echo "\$v[0.5]  = " . $v[0.5] . "\n";
//    echo "\$v[true] = " . $v[true] . "\n";
//    echo "\$v[null] = " . $v[null] . "\n";

    var_dump($v);
    $v[] = 123;
    var_dump($v);
    $v[] = 456;
    var_dump($v);
    $v[1] = 333;
    var_dump($v);
    $v[-5] = -23;		// surprise! Lets me go outside the int-vector element range
    var_dump($v);
    $v[100] = 4444;
    var_dump($v);
  }
}

function main(): void {
  echo "================= Class C1 ===================\n\n";

  $c1 = new C1();
  var_dump($c1);

  echo "================= Class C1::test_conversions ===================\n\n";

  $c1->test_conversions(
    array(), array(true, false),
    array(), array(10,20)
  );

  echo "================= Class C1::test_intrinsics ===================\n\n";

  $c1->test_intrinsics(
    array(0, 100, 67)
  );

  echo "================= Class C1::array_proc ===================\n\n";

  $c1->array_proc(
    array(66),
    array(100, 200),
    array(1.1, 2.2, 3.3),
    array(1, 2.5),
    array(new Button()), array(new CustomButton()));

  echo "================= Class C5 ===================\n\n";

  var_dump(new C5());

/*
  echo "================= unset ===================\n\n";

// Unbound name: NS_arrays\unset

  $v = array(10, 20, 30);
  unset($v[0]);

  $v = array('a' => 10, 'b' => 20);
  unset($v[0]);
*/
}

class C5 {
  private array<string> $colorsV = array("red", "white", "blue");
  private array<int, string> $colorsM = array("red", "white", "blue");
  private array<int, string> $colorsM2 = array(0 => "red", 1 => "white", 2 => "blue");
  private array<int, string> $colorsM3 = array(2 => "blue", 0 => "red", 1 => "white");

  private array<string, bool> $flags = array("F1"=>true,"F2"=>false,"F3"=>false,"F4"=>true,);

  private array<num> $measurements = array(10, 25.55);
  private array<int, mixed> $items = array(true, 123, "red", null);
  private array<mixed, mixed> $things = array('X' => 10, 2 => true);

  private array<Button> $buttons = array();
  private array<MyCollection> $collections = array();

  public function __construct() {
    $this->buttons = array(new Button(), new CustomButton());
    $this->collections = array(new MyList(), new MyQueue());
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();

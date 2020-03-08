<?hh // strict

namespace NS_arrays;

function v(): void {}

class C1 {}
class C2 {
// the bool key initialiers are implicitly converted to int

  public array<bool, bool> $pr_bool_bool = array(true => true, false => true, true => false, false => false);
  public array<bool, int> $pr_bool_int = array(true => 25, false => -100);
  public array<bool, float> $pr_bool_float = array(false => 12.34, true => -2e23);
  public array<bool, num> $pr_bool_num = array(false => 12.34, true => 111);
  public array<bool, string> $pr_bool_string = array(true => "abc", false => "def");
//  public array<bool, void> $pr_bool_void = array();	// previously allowed; not now

// no errors from checker, but get hhvm error 
//  public array<bool, void> $pr_bool_void = array(true => v());

// the int key initialiers are used as is

  public array<int, bool> $pr_int_bool = array(5 => true, -2 => false);
  public array<int, int> $pr_int_int = array(123 => 12, -1 => 1111);
  public array<int, float> $pr_int_float = array(8 => 1.2, 2 => 3.4);
  public array<int, num> $pr_int_num = array(5 => 12.34, -6 => 111);
  public array<int, string> $pr_int_string = array(19 => "abc", -5 => "def");
//  public array<int, void> $pr_int_void = array();	// previously allowed; not now

// the float key initialiers are implicitly converted to int

  public array<float, bool> $pr_float_bool = array(1.2 => true, 1.4 => false, 1.5 => true);
  public array<float, int> $pr_float_int = array(12.7 => 10, 8.3 => -5);
  public array<float, float> $pr_float_float = array(11.2 => 1.2, -54.0 => 3.4);
  public array<float, num> $pr_float_num = array(1.4 => 12.34, 19.4 => 111);
  public array<float, string> $pr_float_string = array(4.5 => "abc", 9.88 => "def");
//  public array<float, void> $pr_float_void = array();	// previously allowed; not now

// the num key initialiers are implicitly converted to int

  public array<num, bool> $pr_num_bool = array(1.2 => true, 2 => false);
  public array<num, int> $pr_num_int = array(12.7 => 10, 8 => -5);
  public array<num, float> $pr_num_float = array(11.2 => 1.2, -54 => 3.4);
  public array<num, num> $pr_num_num = array(1.4 => 12.34, 19 => 111);
  public array<num, string> $pr_num_string = array(4.5 => "abc", 9 => "def");
//  public array<num, void> $pr_num_void = array();	// previously allowed; not now

// the string key initialiers are used as is

  public array<string, bool> $pr_string_bool = array("B" => true, "Z" => false);
  public array<string, int> $pr_string_int = array("X" => 10, "Y" => 20);
  public array<string, float> $pr_string_float = array('v' => 1.2, 'w' => 9.7);
  public array<string, num> $pr_string_num = array('zz' => 12.34, 'vv' => 111);
  public array<string, string> $pr_string_string = array('red' => 'yes', 'green' => 'no');
//  public array<string, void> $pr_string_void = array();	// previously allowed; not now

// there is no synax for writing a void expression for any key, so can use only an empty array

//  public array<void, bool> $pr_void_bool = array();	// previously allowed; not now
//  public array<void, int> $pr_void_int = array();	// previously allowed; not now
//  public array<void, float> $pr_void_float = array();	// previously allowed; not now
//  public array<void, num> $pr_void_num = array();	// previously allowed; not now
//  public array<void, string> $pr_void_string = array();	// previously allowed; not now
//  public array<void, void> $pr_void_void = array();	// previously allowed; not now

// Although the checker finds no errors, HHVM reports "Warning: Invalid operand type was used: Invalid 
// type used as key" and then ignores the initializer(s), resulting in an empty array

//  public array<array<bool>, bool> $pr_arraybool_bool = array(array(true) => true, array(false) => false);
//  public array<array<bool>, int> $pr_arraybool_int = array(array(true) => 10, array(false) => 20);
//  public array<array<bool>, float> $pr_arraybool_float = array(array(true) => 12.3, array(false) => -9.4);
//  public array<array<bool>, num> $pr_arraybool_num = array(array(true) => 12.3, array(false) => -9);
//  public array<array<bool>, string> $pr_arraybool_string = array(array(true) => "abc", array(false) => "def");

  public array<array<bool>, bool> $pr_arraybool_bool = array();
  public array<array<bool>, int> $pr_arraybool_int = array();
  public array<array<bool>, float> $pr_arraybool_float = array();
  public array<array<bool>, num> $pr_arraybool_num = array();
  public array<array<bool>, string> $pr_arraybool_string = array();
//  public array<array<bool>, void> $pr_arraybool_void = array();	// previously allowed; not now

// Although the checker finds no errors, HHVM reports "syntax error, unexpected T_NEW, expecting ')'"

//  public array<C1, bool> $pr_C1_bool = array(new C1() => true, new C1() => false);
//  public array<C1, int> $pr_C1_int = array(new C1() => 10, new C1() => -5);
//  public array<C1, float> $pr_C1_float = array(x => 2.9, x => 3.4);
//  public array<C1, num> $pr_C1_num = array(x => 11.2, x => 33);
//  public array<C1, string> $pr_C1_string = array(x => 'ccc', x => 'aaa');

  public array<C1, bool> $pr_C1_bool = array();
  public array<C1, int> $pr_C1_int = array();
  public array<C1, float> $pr_C1_float = array();
  public array<C1, num> $pr_C1_num = array();
  public array<C1, string> $pr_C1_string = array();
//  public array<C1, void> $pr_C1_void = array();	// previously allowed; not now

  public function __construct() {

// an array (used like a hashtable) does not allow array append

//    $this->pr_bool_int[] = 200;
//    $this->pr_float_float[] = 1.111;
//    $this->pr_num_num[] = 10; $this->pr_num_num[] = 6.7;
//    $this->pr_string_bool[] = true;
//    $this->pr_void_int[] = 123;
//    $this->pr_arraybool_int[] = 222;
//    $this->pr_C1_string[] = 'zzz';

    $this->pr_bool_int[true] = 200;
    $this->pr_float_float[23.5] = 1.111;
    $this->pr_num_num[7] = 10; $this->pr_num_num[-3.5] = 6.7;
    $this->pr_string_bool['ff'] = true;

// Re the next statement, the checker is happy and hhvm takes the default null returned by a function 
// not returning an explicit value, converts it to an empty string and uses that as the index of the 
// new element, whose value is 123. Interesting, but defies the idea that a void expression (like the 
// call to v()) has no value.

//    $this->pr_void_int[v()] = 123;	// previously allowed; not now

// The following appears to do nothing

    $this->pr_arraybool_bool[array(true)] = true;

// The following appears to do nothing

    $this->pr_C1_bool[new C1()] = true;

// For the following two cases:
// Although the checker finds no errors, HHVM reports "Warning: Invalid operand type was used: Invalid 
// type used as key" and then ignores the initializer(s), resulting in an empty array

//    $this->pr_arraybool_bool = array(array(true) => true, array(false) => false);
//    $this->pr_C1_bool = array(new C1() => true, new C1() => false);
  }
}

function main(): void {
  $c2 = new C2();

  echo "====== bool keys (implicitly converted to int) ======\n\n";
  echo "pr_bool_bool: "; var_dump($c2->pr_bool_bool);
  echo "pr_bool_int: "; var_dump($c2->pr_bool_int);
  echo "pr_bool_float: "; var_dump($c2->pr_bool_float);
  echo "pr_bool_num: "; var_dump($c2->pr_bool_num);
  echo "pr_bool_string: "; var_dump($c2->pr_bool_string);
//  echo "pr_bool_void: "; var_dump($c2->pr_bool_void);
	
  echo "\n====== int keys ======\n\n";
  echo "pr_int_bool: "; var_dump($c2->pr_int_bool);
  echo "pr_int_int: "; var_dump($c2->pr_int_int);
  echo "pr_int_float: "; var_dump($c2->pr_int_float);
  echo "pr_int_num: "; var_dump($c2->pr_int_num);
  echo "pr_int_string: "; var_dump($c2->pr_int_string);
//  echo "pr_int_void: "; var_dump($c2->pr_int_void);
	
  echo "\n====== float keys (implicitly converted to int) ======\n\n";
  echo "pr_float_bool: "; var_dump($c2->pr_float_bool);
  echo "pr_float_int: "; var_dump($c2->pr_float_int);
  echo "pr_float_float: "; var_dump($c2->pr_float_float);
  echo "pr_float_num: "; var_dump($c2->pr_float_num);
  echo "pr_float_string: "; var_dump($c2->pr_float_string);
//  echo "pr_float_void: "; var_dump($c2->pr_float_void);
	
  echo "\n====== num keys (implicitly converted to int) ======\n\n";
  echo "pr_num_bool: "; var_dump($c2->pr_num_bool);
  echo "pr_num_int: "; var_dump($c2->pr_num_int);
  echo "pr_num_float: "; var_dump($c2->pr_num_float);
  echo "pr_num_num: "; var_dump($c2->pr_num_num);
  echo "pr_num_string: "; var_dump($c2->pr_num_string);
//  echo "pr_num_void: "; var_dump($c2->pr_num_void);
	
  echo "\n====== string keys ======\n\n";
  echo "pr_string_bool: "; var_dump($c2->pr_string_bool);
  echo "pr_string_int: "; var_dump($c2->pr_string_int);
  echo "pr_string_float: "; var_dump($c2->pr_string_float);
  echo "pr_string_num: "; var_dump($c2->pr_string_num);
  echo "pr_string_string: "; var_dump($c2->pr_string_string);
//  echo "pr_string_void: "; var_dump($c2->pr_string_void);
	
  echo "\n====== void keys ======\n\n";
//  echo "pr_void_bool: "; var_dump($c2->pr_void_bool);
//  echo "pr_void_int: "; var_dump($c2->pr_void_int);
//  echo "pr_void_float: "; var_dump($c2->pr_void_float);
//  echo "pr_void_num: "; var_dump($c2->pr_void_num);
//  echo "pr_void_string: "; var_dump($c2->pr_void_string);
//  echo "pr_void_void: "; var_dump($c2->pr_void_void);
	
  echo "\n====== array<bool> keys ======\n\n";
  echo "pr_arraybool_bool: "; var_dump($c2->pr_arraybool_bool);
  echo "pr_arraybool_int: "; var_dump($c2->pr_arraybool_int);
  echo "pr_arraybool_float: "; var_dump($c2->pr_arraybool_float);
  echo "pr_arraybool_num: "; var_dump($c2->pr_arraybool_num);
  echo "pr_arraybool_string: "; var_dump($c2->pr_arraybool_string);
//  echo "pr_arraybool_void: "; var_dump($c2->pr_arraybool_void);
	
  echo "\n====== C1 keys ======\n\n";
  echo "pr_C1_bool: "; var_dump($c2->pr_C1_bool);
  echo "pr_C1_int: "; var_dump($c2->pr_C1_int);
  echo "pr_C1_float: "; var_dump($c2->pr_C1_float);
  echo "pr_C1_num: "; var_dump($c2->pr_C1_num);
  echo "pr_C1_string: "; var_dump($c2->pr_C1_string);
//  echo "pr_C1_void: "; var_dump($c2->pr_C1_void);
}

/* HH_FIXME[1002] call to main in strict*/
main();

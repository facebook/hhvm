<?hh // strict

namespace NS_array_concatenation;

class Button {}
class CustomButton extends Button {}

function array_proc(
  array<int> $p1, array<int> $p2,
  array<float> $p3, array<num> $p4,
  array<Button> $p5, array<CustomButton> $p6,
  array<int, int> $p7,
  array<int, string> $p8, array<int, string> $p9,
  array<int, bool> $p10,
  array<string, int> $p11, array<string, int> $p12
  ): void {
  echo "============= vector-like arrays ================\n";

  $v = $p1 + $p2;		// array<int> + array<int> okay
  echo "result of p1 + p2 = "; var_dump($v);

  $v = $p2 + $p1;		// okay, but of course, is not commutative
  echo "result of p2 + p1 = "; var_dump($v);

//  $v = $p1 + $p3;		// array<int> + array<float> disallowed
//  $v = $p4 + $p1;		// array<num> + array<int> disallowed
//  $v = $p4 + $p3;		// array<num> + array<float> disallowed
//  $v = $p5 + $p6;		// array<Button> + array<CustomButton> disallowed

  echo "============= map-like arrays ================\n";

//  $v = $p1 + $p7;		// array<int> + array<int, int> disallowed;
				// can't mix vector-like and map-like arrays even with same key/value types

  $v = $p8 + $p9;		// array<int, string> + array<int, string> okay
  echo "result of p8 + p9 = "; var_dump($v);

//  $v = $p7 + $p10;	// array<int, int> + array<int, bool> disallowed;

  $v = $p11 + $p12;	// array<string, int> + array<string, int> okay
  echo "result of p11 + p12 = "; var_dump($v);

  echo "============= concating to an empty/untyped array ================\n";

  $p2 = $p1 + array();	// array<int> + array() allowed;
  echo "result of p1 + array() = "; var_dump($p2);
  $p2 = array() + $p2;	// array<int> + array() allowed;
  echo "result of array() + p1 = "; var_dump($p2);

  $v = $p10 + array();	// array<int, bool> + array() allowed;
  echo "result of p10 + array() = "; var_dump($v);

  $v = $p11 + array();	// array<string, int> + array() allowed
  echo "result of p11 + array() = "; var_dump($v);

  $p2 = array() + array();	// array() + array() allowed;
  echo "result of array() + array() = "; var_dump($p2);
}

function main(): void {
  array_proc(
    array(66),
    array(100, 200),
    array(1.1, 2.2, 3.3),
    array(1, 2.5),
    array(new Button()), array(new CustomButton()),
    array(6 => 66, 12 => 22),
    array(2 => 'aa', 12 => 'bb'), array(-4 => 'cc', 6 => 'dd'), 
    array(-3 => true),
    array('red' => 12, 'green' => 7), array('blue' => 3), 
  );
}

/* HH_FIXME[1002] call to main in strict*/
main();

<?hh // strict

namespace NS_function_call;

function f(): void {
  echo "Inside function " . __FUNCTION__ . "\n";
}

function f2(int $p1, int $p2 = -100): void {}

function f3(int $p2 = -100, ...): void {}

function fx(int $p1, int $p2, int $p3, int $p4, int $p5): void { }
function fy(int $p1, int $p2, int $p3, int $p4, int $p5): void {
  echo "$p1, $p2, $p3, $p4, $p5\n";
}
function fz(int $p1, int $p2, int $p3, int $p4, int $p5): void { }

function main(): void {
//  $x = '\NS_function_call\f';
// $x();	// unlike PHP, Hack does not allow a function to be called using its raw string name
  $x = fun('\NS_function_call\f');	// must "wrap" using fun instead
  $x();

//  f2();			// too few argumemts
  f2(24);
//  f2(2.3);		// incompatible types
  f2(5, 10);
//  f2(9, true);		// incompatible types
//  f2(12, 123, 222);	// too many arguments


  f3();			// default value used
  f3(24);
//  f3(2.3);		// incompatible types
  f3(5, 10);		// excess of any number and type okay
  f3(9, true);		// excess of any number and type okay
  f3(12, 123, 222);	// excess of any number and type okay

  $funcTable = array(fun('\NS_function_call\fx'), fun('\NS_function_call\fy'),
    fun('\NS_function_call\fz'));	// list of 3 function designators 
  $i = 1;
  $funcTable[$i++]($i, ++$i, $i, $i = 12, --$i); // calls fy(2,3,3,12,11)
}

/* HH_FIXME[1002] call to main in strict*/
main();

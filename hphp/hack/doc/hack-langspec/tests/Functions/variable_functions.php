<?hh // strict

namespace NS_variable_functions;

require_once('TestInc.php');	// get access to \NS\TestInc\f2()

function f1(): void {
  echo "Inside function " . __FUNCTION__ . "\n";
}

function main(): void {
  f1();
//  namespace\f1();		// PHP allows this; Hack does not

  $v = fun('\NS_variable_functions\f1');
  $v();

  $v = fun('\NS_TestInc\f2');
  $v();
}

/* HH_FIXME[1002] call to main in strict*/
main();

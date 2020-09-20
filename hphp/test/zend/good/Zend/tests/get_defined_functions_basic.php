<?hh

function foo() {}

// mixed case function
function HelloWorld() {}

class C {
	function f1() {}
	static function f2() {}
}
<<__EntryPoint>>
function entrypoint_get_defined_functions_basic(): void {

  /* Prototype  : array get_defined_functions  ( void  )
   * Description: Gets an array of all defined functions.
   * Source code: Zend/zend_builtin_functions.c
  */

  echo "*** Testing get_defined_functions() : basic functionality ***\n";

  $func = get_defined_functions();

  if (!is_array($func)) {
  	echo "TEST FAILED: return type not an array\n";
  }


  if (!is_array($func["internal"])) {
   	echo "TEST FAILED: no element in result array with key 'internal'\n";
  }

  $internal = $func["internal"];

  //check for a few core functions
  if (!in_array("cos", $internal) || !in_array("strlen", $internal)) {
   	echo "TEST FAILED: missing elements from 'internal' array\n";
   	var_dump($internal);
  }

  if (!is_array($func["user"])) {
   	echo "TEST FAILED: no element in result array with key 'user'\n";
  }

  $user = $func["user"];
  if (count($user) == 3 && in_array("entrypoint_get_defined_functions_basic", $user) && in_array("foo", $user) && in_array("helloworld", $user)) {
  	echo "TEST PASSED\n";
  } else {
  	echo "TEST FAILED: missing elements from 'user' array\n";
  	var_dump($user);
  }
  echo "===Done===";
}

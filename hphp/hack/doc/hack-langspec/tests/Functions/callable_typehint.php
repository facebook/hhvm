<?hh // strict

namespace NS_callable_typehint;

function hello(): void {
  echo "Hello!\n";
}

//function f5(callable $p1): void {	// callable type hint not permitted
function f5((function (): void) $p1): void {
  echo "Inside " . __FUNCTION__ . "\n";

  var_dump($p1);
  $p1();
}

function main(): void {
  f5(fun('\NS_callable_typehint\hello'));
}

/* HH_FIXME[1002] call to main in strict*/
main();

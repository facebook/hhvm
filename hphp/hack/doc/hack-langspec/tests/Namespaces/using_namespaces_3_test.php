<?hh // strict

namespace NS_using_namespaces_3_test;

require_once "using_namespaces_3.php";

function main(): void {
  \NS18\f();
}

/* HH_FIXME[1002] call to main in strict*/
main();

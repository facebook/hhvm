<?hh // strict

namespace NS_namespaces2_test;

require_once "namespaces2.php";

function main(): void {
  \NS10\f10();
  f20();
  \NS20\f30();
}

/* HH_FIXME[1002] call to main in strict*/
main();
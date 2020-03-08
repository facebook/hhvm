// NOT YET IMPLEMENTED IN THE CHECKER <?hh // strict
/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS_use_groups;

require_once ("use_groups_require_file.php");

class CC implements I1 {};

function main(): void {

  echo "================ class names =================\n\n";

  var_dump(new \NS\C1());	// explicit qualification
  use \NS\C1;			// import C1
  var_dump(new C1());		// create a C1 without explicit qualification

  use \NS\{C1 as C1B};		// group of 1 allowed, but as name C1 already use'd, need to give it an alias
  var_dump(new C1B());		// create a C1/C1B without explicit qualification

  use \NS\  {  C2, C3, I1 };	// import C2, C3, and I1
  var_dump(new C2());		// create a C2 without explicit qualification
  var_dump(new C3());		// create a C3 without explicit qualification
  var_dump(new CC());

  echo "\n================ const names =================\n\n";

  var_dump(\NS\CON1);		// explicit qualification
  use const \NS\CON1;		// import CON1
  var_dump(CON1);		// access CON1 without explicit qualification

  echo "\n================ function names =================\n\n";

  \NS\f1();			// explicit qualification
  use function \NS\f1;		// import f1
  f1();				// call f1 without explicit qualification

  echo "\n================ class, const and function names =================\n\n";

  use \NS\ { C2 as CX, const CON2 as CZ, function f1 as FZ };
  var_dump(new CX());
  var_dump(CZ);
  fZ();
}

/* HH_FIXME[1002] call to main in strict*/
main();

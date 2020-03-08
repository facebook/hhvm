<?hh // strict

namespace NS_MathLibrary_test1;

require_once 'MathLibrary.php';

function main(): void {
//  $m = new \NS_MathLibrary\MathLibrary();	// can't instantiate a final class

  \NS_MathLibrary\MathLibrary::sin(2.34);
  \NS_MathLibrary\MathLibrary::cos(2.34);
  \NS_MathLibrary\MathLibrary::tan(2.34);
}

/* HH_FIXME[1002] call to main in strict*/
main();

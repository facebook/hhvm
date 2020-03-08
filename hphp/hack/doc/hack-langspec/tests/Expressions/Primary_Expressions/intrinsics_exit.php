<?hh // strict

namespace NS_intrinsics_exit;

require_once('Point2.php');

function cleanup1(): void {
  echo "Inside " . __METHOD__ . "\n";
}

function cleanup2(): void {
  echo "Inside " . __METHOD__ . "\n";
}

function main(): void {
  register_shutdown_function('cleanup2');
  register_shutdown_function('cleanup1');

  echo "--------- test with/without string -------------\n";

  $p1 = new Point2(5.0, 3.0);
  $p2 = new Point2();
  $p3 = new Point2();

  exit("goodbye\n");	// writes "goodbye", then destructors are called.
//  exit(99);		// writes nothing
//  exit();			// writes nothing
//  exit;			// writes nothing

  echo "end of script\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();

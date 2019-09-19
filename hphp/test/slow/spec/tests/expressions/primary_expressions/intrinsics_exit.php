<?hh

function cleanup1()
{
    echo "Inside " . __METHOD__ . "\n";
}

function cleanup2()
{
    echo "Inside " . __METHOD__ . "\n";
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  register_shutdown_function(fun('cleanup2'));
  register_shutdown_function(fun('cleanup1'));

  echo "--------- test with/without string -------------\n";

  include_once 'Point2.inc';

  $p1 = new Point2(5, 3);
  $p2 = new Point2;
  $p3 = new Point2;

  exit("goodbye\n");  // writes "goodbye", then destructors are called.
  //exit(99);         // writes nothing
  //exit();           // writes nothing
  //exit;             // writes nothing

  echo "end of script\n";
}

<?hh

class kls { public static function meth() { echo "heh\n"; } }

function get_bad_stuff() {
  return array('y' => array());
}

function foo() {
  $x = array('kls', 'meth');
  $y = "this is a string";

  /*
   * Make sure that we find a fixed point on the FPIKind correctly.
   * The first time through this loop, we know it is an array and
   * therefore can't be a call to extract.  Second time through, it is
   * in fact a call to extract, so we better handle merging the new
   * information into the ActRec properly.
   */
  for ($i = 0; $i < 2; ++$i) {
    $x(mt_rand() ? get_bad_stuff() : array('y' => array()));
    $x = "extract"; // bad times
  }
  return $y;
}

function main() {
  $y = foo();
  var_dump($y);
}

main();

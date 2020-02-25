<?hh

abstract final class quickIsset {
  public static $y;
}

function f() {


  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickIsset::$y).":\n";

  $x = 0;
  $GLOBALS['y'] = 0;
  quickIsset::$y = 0;
  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickIsset::$y).":\n";

  unset($x);
  unset($GLOBALS['y']);
  quickIsset::$y = null;
  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickIsset::$y).":\n";

  $a = darray[];
  $a["foo"] = null;
  var_dump(isset($a["foo"]));
}

function get_index() {
  echo "I've made a huge mistake\n";
  return 0;
}

function g($dontTake, inout $toFillIn, $id, $key, $value) {
  $toFillIn = darray[];
  if (isset($toFillIn[$id])) {
    $cur = $toFillIn[$id];
  }
  $toFillIn[$id] = $value;
}

/*********/
<<__EntryPoint>> function main(): void {
f();

$a = 4;
$arr = varray["get_index should not be called"];
var_dump(isset($a, $b, $arr[get_index()]));

/**
 * Check for a peculiar translator interaction with IssetM, where
 * a dirty, variant local in the same BB as IssetM could cause the
 * local to morph into a cell.
 */

$a = null;
g(null, inout $a, "127.0.0.1", null, null );
var_dump($a);
}

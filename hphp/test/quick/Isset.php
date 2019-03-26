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

  $a = array();
  $a["foo"] = null;
  var_dump(isset($a["foo"]));
}

f();

/*********/

function get_index() {
  echo "I've made a huge mistake\n";
  return 0;
}

$a = 4;
$arr = array("get_index should not be called");
var_dump(isset($a, $b, $arr[get_index()]));

/**
 * Check for a peculiar translator interaction with IssetM, where
 * a dirty, variant local in the same BB as IssetM could cause the
 * local to morph into a cell.
 */

function g($dontTake, &$toFillIn, $id, $key, $value) {
  $toFillIn = array();
  if (isset($toFillIn[$id])) {
    $cur = $toFillIn[$id];
  }
  $toFillIn[$id] = $value;
}

$a = null;
g(null, &$a, "127.0.0.1", null, null );
var_dump($a);

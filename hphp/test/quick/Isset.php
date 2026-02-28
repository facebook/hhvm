<?hh

abstract final class quickIsset {
  public static $y;
}

function f() :mixed{


  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickIsset::$y)).":\n";

  $x = 0;
  \HH\global_set('y', 0);
  quickIsset::$y = 0;
  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickIsset::$y)).":\n";

  unset($x);
  \HH\global_unset('y');
  quickIsset::$y = null;
  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickIsset::$y)).":\n";

  $a = dict[];
  $a["foo"] = null;
  var_dump(isset($a["foo"]));
}

function get_index() :mixed{
  echo "I've made a huge mistake\n";
  return 0;
}

function g($dontTake, inout $toFillIn, $id, $key, $value) :mixed{
  $toFillIn = dict[];
  if (isset($toFillIn[$id])) {
    $cur = $toFillIn[$id];
  }
  $toFillIn[$id] = $value;
}

/*********/
<<__EntryPoint>> function main(): void {
f();

$a = 4;
$arr = vec["get_index should not be called"];
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

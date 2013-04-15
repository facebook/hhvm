<?

function nop($unused) { /* value sink */ }

function testApc($before) {
  apc_delete("indep");
  if (!apc_add("indep", $before)) {
    echo "add failure. weird.\n";
    exit(1);
  }

  # fetch.
  $after = apc_fetch("indep");
  var_dump($after);
  if (!$after) {
    echo "fetch failure. surprising.\n";
    exit(2);
  }

  # cgetm.
  foreach ($before as $k => $v) {
    var_dump($after[$k]);
    if ($after[$k] != $v) {
      echo "fetched dubious values " . $after[$k] . " != " . "$v\n";
      var_dump($after[$k]);
      exit(3);
    }
    if (!isset($after[$k])) {
      echo "expected key not set. devestating.\n";
      var_dump($after[$k]);
      exit(4);
    }
  }

  # iterate over the APC array, too
  foreach ($after as $k => $v) {
    var_dump($after[$k]);
    if ($after[$k] != $v) {
      echo "incoherent APC iteration. lamentable.\n";
      var_dump($v);
      exit(5);
    }
  }

  # setM
  $after['newKey'] = array();
  var_dump($after);

  # unsetm
  foreach($after as $k => $v) {
    unset($after[$k]);
  }
  var_dump($after);
}

function testKeyTypes() {
  apc_add("keysarray", array(2 => 'two', '3' => 'three'));
  $arr = apc_fetch("keysarray");
  foreach (array(2, 3, '2', '3') as $k) {
    var_dump($arr[$k]);
  }
}

function main() {
  testApc(array(7, 4, 1776));
  testApc(array("sv0", "sv1"));
  testApc(array("sk0" => "sv0", "sk1" => "sv1"));

  // Also check that foreign arrays work for indirect calls
  apc_store('foo', array("a"));
  $a = apc_fetch('foo');
  $b = call_user_func_array("strtoupper", $a);
  var_dump($b);

  testKeyTypes();
}

main();

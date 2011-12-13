<?

$foo = array(7, 4, 1776);

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
      echo "fetched dubious values. foreboding.\n";
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

testApc(array(7, 4, 1776));
testApc(array("sv0", "sv1"));
testApc(array("sk0" => "sv0", "sk1" => "sv1"));


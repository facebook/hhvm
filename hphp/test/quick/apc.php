<?hh

function nop($unused) :mixed{ /* value sink */ }

function testApc($before) :mixed{
  apc_delete("indep");
  if (!apc_add("indep", $before)) {
    echo "add failure. weird.\n";
    exit(1);
  }

  // fetch.
  $after = __hhvm_intrinsics\apc_fetch_no_check("indep");
  var_dump($after);
  if (!$after) {
    echo "fetch failure. surprising.\n";
    exit(2);
  }

  // cgetm.
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

  // iterate over the APC array, too
  foreach ($after as $k => $v) {
    var_dump($after[$k]);
    if ($after[$k] != $v) {
      echo "incoherent APC iteration. lamentable.\n";
      var_dump($v);
      exit(5);
    }
  }

  // varrays don't support setStr or general-purpose unset.
  if (isset($after[0])) {
    $after[] = 'new varray entry';
    var_dump($after);
    return;
  }

  // setM
  $after['newKey'] = vec[];
  var_dump($after);

  // unsetm
  foreach($after as $k => $v) {
    unset($after[$k]);
  }
  var_dump($after);
}

function testKeyTypes() :mixed{
  apc_add("keysarray", dict[2 => 'two', '3' => 'three']);
  $arr = __hhvm_intrinsics\apc_fetch_no_check("keysarray");
  foreach (vec[2, 3, '2', '3'] as $k) {
    try { var_dump($arr[$k]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
}

function testInvalidKeys() :mixed{
    // Reject keys with null bytes
    apc_add("bar\x00baz", 10);
    apc_store("test\x00xyz", "hello");
    apc_store(dict["validkey" => "validvalue", "invalid\x00key" => "value"]);
    foreach (vec['bar', 'test', 'validkey', 'invalid'] as $k) {
        var_dump(__hhvm_intrinsics\apc_fetch_no_check($k));
    }
}

<<__EntryPoint>> function main(): void {
  testApc(vec[7, 4, 1776]);
  testApc(vec["sv0", "sv1"]);
  testApc(dict["sk0" => "sv0", "sk1" => "sv1"]);

  // Also check that foreign arrays work for indirect calls
  apc_store('foo', vec["a"]);
  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $b = call_user_func_array(strtoupper<>, $a);
  var_dump($b);

  testKeyTypes();
  testInvalidKeys();
}

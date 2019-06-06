<?hh

function test($a, $b, $c, $d, $e) {
  $k = array();
  foreach ($a as $id) {
    try { $k[$id] = foo($id, $b, $c, $d, $e); }
    catch (Exception $e) { echo $e->getMessage()."\n"; }
    try { $k[$id] = foo($k[$id], $b); }
    catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
}

function foo($a, $b) {
  return $a ?: $b;
}

<<__EntryPoint>> function main(): void {
  test(array(array('foo'), array('bar'), array('baz')), null, 1, 2, 3);
}

<?hh

function test($a, $b, $c, $d, $e) {
  $k = darray[];
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
  test(varray[varray['foo'], varray['bar'], varray['baz']], null, 1, 2, 3);
}

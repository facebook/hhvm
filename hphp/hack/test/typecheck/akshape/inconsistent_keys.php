<?hh //strict

class C1 {
  const KEY = 'k1';
}

class C2 {
  const KEY = 'k2';
}

/**
 * Mixing constant strings and class constants, or class constants from
 * different classes doesn't produce shape-like arrays.
 */
function test(): void {
  $a = array('x' => 4, C1::KEY => 'aaa');
  hh_show($a);

  $b = array(C1::KEY => 4, C2::KEY => 'aaa');
  hh_show($b);
}

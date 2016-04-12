<?hh //strict

/**
 * It's possible to iterate over shape-like array and it behaves as map-like
 * array - usage that should report errors.
 */

function test(): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 3.14;

  foreach ($a as $v) {
    take_int($v);
  }
}

function take_int(int $_): void {}

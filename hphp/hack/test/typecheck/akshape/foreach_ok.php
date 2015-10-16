<?hh //strict

/**
 * It's possible to iterate over shape-like array and it behaves as map-like
 * array - usage with no errors.
 */

function test(): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 3.14;

  foreach ($a as $v) {
    take_num($v);
  }
}

function take_num(num $_): void {}

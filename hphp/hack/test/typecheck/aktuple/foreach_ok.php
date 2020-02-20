<?hh //strict

/**
 * It's possible to iterate over tuple-like array and it behaves as vec-like
 * array - usage with no errors.
 */

function test(): void {
  $a = varray[4, 3.14];

  foreach ($a as $v) {
    take_num($v);
  }
}

function take_num(num $_): void {}

<?hh

/**
 * It's possible to iterate over tuple-like array and it behaves as vec-like
 * array - usage that should report errors.
 */

function test(): void {
  $a = vec[4, 3.14];

  foreach ($a as $v) {
    take_int($v);
  }
}

function take_int(int $_): void {}

<?hh //strict

/* Tuple-like arrays can be used with list(...) statement - usage that should
 * report errors. */
function test(): void {
  $a = array(4, 'aaa');

  list($int, $string) = $a;
  take_string($int);
}

function take_string(string $_): void {}

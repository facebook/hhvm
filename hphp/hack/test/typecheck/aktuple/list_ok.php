<?hh //strict

/* Tuple-like arrays can be used with list(...) statement - usage with no
 * errors. */
function test(): void {
  $a = varray[4, 'aaa'];
  list($int, $string) = $a;

  take_string($string);
  take_int($int);
}

function take_string(string $_): void {}
function take_int(int $_): void {}

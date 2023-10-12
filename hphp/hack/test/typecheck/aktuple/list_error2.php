<?hh //strict

/* Tuple-like arrays used with list(...) statement check arity */
function test(): void {
  $a = varray[4, 'aaa', "too many"];

  list($int, $string) = $a;
}

function take_string(string $_): void {}

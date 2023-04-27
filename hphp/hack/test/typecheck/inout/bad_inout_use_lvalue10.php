<?hh

function foobar(inout dynamic $x): void {
  foobar(inout $x[0][]);
}

<?hh
error_reporting(-1);
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    echo "E_RECOVERABLE_ERROR: $errmsg\n";
  } else {
    return false;
  }
}
set_error_handler('handler');
type X = num;
newtype Y = ?num;
newtype A = num;
type B = ?num;
newtype C = X;
type D = Y;
function a(A $x) {}
function b(B $x) {}
function c(C $x) {}
function d(D $x) {}
function main() {
  a(null);
  a(1);
  a(1.0);
  a('foo');
  b(null);
  b(1);
  b(1.0);
  b('foo');
  c(null);
  c(1);
  c(1.0);
  c('foo');
  d(null);
  d(1);
  d(1.0);
  d('foo');
  echo "Done\n";
}
main();

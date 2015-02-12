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
type X = resource;
newtype Y = ?resource;
newtype A = resource;
type B = ?resource;
newtype C = X;
type D = Y;
function a(A $x) {}
function b(B $x) {}
function c(C $x) {}
function d(D $x) {}
function main() {
  $x = imagecreate(10, 10);
  a(null);
  a(1);
  a($x);
  b(null);
  b(1);
  b($x);
  c(null);
  c(1);
  c($x);
  d(null);
  d(1);
  d($x);
  echo "Done\n";
}
main();

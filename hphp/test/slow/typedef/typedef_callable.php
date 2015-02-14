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
type X = callable;
newtype Y = ?callable;
newtype A = callable;
type B = ?callable;
newtype C = X;
type D = Y;
class Q {
  public static function foo() {}
}
function a(A $x) {}
function b(B $x) {}
function c(C $x) {}
function d(D $x) {}
function main() {
  a(null);
  a(1);
  a('var_dump');
  a('non_existent');
  a(array('Q','foo'));
  a(array('Q','non_existent'));
  a(function () { return 123; });
  b(null);
  b(1);
  b('var_dump');
  b('non_existent');
  b(array('Q','foo'));
  b(array('Q','non_existent'));
  b(function () { return 123; });
  c(null);
  c(1);
  c('var_dump');
  c('non_existent');
  c(array('Q','foo'));
  c(array('Q','non_existent'));
  c(function () { return 123; });
  d(null);
  d(1);
  d('var_dump');
  d('non_existent');
  d(array('Q','foo'));
  d(array('Q','non_existent'));
  d(function () { return 123; });
  echo "Done\n";
}
main();

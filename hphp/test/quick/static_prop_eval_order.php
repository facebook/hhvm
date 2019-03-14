<?hh
function f() { echo '1 '; }
function g() { echo '3 '; }
function __autoload($cls) {
  echo '2 ';
  if (strtolower($cls) === 'c') {
    include 'static_prop_eval_order.inc';
  }
}
$cls = 'C';
$cls::$x[0][f()] = g();
echo "\n";

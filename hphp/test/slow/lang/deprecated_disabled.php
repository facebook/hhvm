<?hh

<<__Deprecated('', 0)>>
function f() {}
<<__Deprecated('', 1)>>
function g() {}


<<__EntryPoint>>
function main_deprecated_disabled() {
$errors = 0;
set_error_handler(function () use (&$errors) {
  $errors++;
  return true;
});
error_reporting(-1);

echo "testing f(): ";
f();
echo "$errors errors\n";

echo "testing g(): ";
g();
echo "$errors errors\n";
}

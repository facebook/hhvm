<?hh

<<__Deprecated('', 0)>>
function f() :mixed{}
<<__Deprecated('', 1)>>
function g() :mixed{}

class Errors { public static int $errors = 0; }

<<__EntryPoint>>
function main_deprecated_disabled() :mixed{
$errors = 0;
set_error_handler(function () {
  Errors::$errors++;
  return true;
});
error_reporting(-1);

echo "testing f(): ";
f();
$errors = Errors::$errors;
echo "$errors errors\n";

echo "testing g(): ";
g();
$errors = Errors::$errors;
echo "$errors errors\n";
}

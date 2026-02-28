<?hh

<<__Deprecated('', 2)>>
function f() :mixed{}

class Errors { public static int $errors = 0; }

<<__EntryPoint>>
function main_deprecated_sampled() :mixed{
$errors = 0;
set_error_handler(function () {
  Errors::$errors++;
  return true;
});
error_reporting(-1);
for ($i = 0; $i < 1000; ++$i) f();

$errors = Errors::$errors;
echo $errors >= 400 && $errors <= 600 ? "ok" : "fail";
}

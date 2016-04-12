<?hh

$errors = 0;
set_error_handler(function () use (&$errors) {
  $errors++;
  return true;
});
error_reporting(-1);

<<__Deprecated('', 2)>>
function f() {}
for ($i = 0; $i < 1000; ++$i) f();

echo $errors >= 400 && $errors <= 600 ? "ok" : "fail";

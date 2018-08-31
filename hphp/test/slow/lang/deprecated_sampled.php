<?hh

<<__Deprecated('', 2)>>
function f() {}


<<__EntryPoint>>
function main_deprecated_sampled() {
$errors = 0;
set_error_handler(function () use (&$errors) {
  $errors++;
  return true;
});
error_reporting(-1);
for ($i = 0; $i < 1000; ++$i) f();

echo $errors >= 400 && $errors <= 600 ? "ok" : "fail";
}

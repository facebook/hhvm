<?hh
// package foo

class ASoftSampled {
  // Ok - package bar is in the same deployment as package foo
  <<__SoftRequirePackage("bar", 2)>>
  static function bar() : void {}

}

<<__SoftRequirePackage("bat", 2)>>
//  LOG - package bat is not deployed in the same deployment as package foo
function b_soft_sampled(): void {}


class Errors { public static int $errors = 0; }

<<__Entrypoint>>
function soft_sampled_main(): void {
  set_error_handler(function () {
    Errors::$errors++;
    return true;
  });
  error_reporting(-1);
  for ($i = 0; $i < 1000; ++$i) {
    ASoftSampled::bar();
    b_soft_sampled();
  }

  echo Errors::$errors >= 400 && Errors::$errors <= 600 ? "ok" : "fail";
}

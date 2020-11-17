<?hh

function err($x, $y) { echo $y; echo "\n"; }
abstract class Asd {
  private function __construct() {}
}

function x() { new Asd(); } <<__EntryPoint>>
function entrypoint_ctor_fail_abstract(): void {
  set_error_handler(err<>);
  x();
}

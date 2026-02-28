<?hh

function err($x, $y) :mixed{ echo $y; echo "\n"; }
class Asd {
  private function __construct()[] {}
}

function x() :mixed{ new Asd(); } <<__EntryPoint>>
function entrypoint_ctor_fail(): void {
  set_error_handler(err<>);
  x();
}

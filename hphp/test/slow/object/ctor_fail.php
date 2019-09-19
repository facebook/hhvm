<?hh

function err($x, $y) { echo $y; echo "\n"; }
set_error_handler(fun('err'));
class Asd {
  private function __construct() {}
}

function x() { new Asd(); } x();

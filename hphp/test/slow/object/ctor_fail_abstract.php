<?hh

function err($x, $y) { echo $y; echo "\n"; }
set_error_handler(fun('err'));
abstract class Asd {
  private function __construct() {}
}

function x() { new Asd(); } x();

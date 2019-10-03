<?hh

class foo {
  static function ioo($y, inout $x) {
    return new self(1);
  }
  function __construct($a, $b) {
}
}
function t() {
  $x = 1;
  $y = null;
  foo::ioo($x, inout $y);
}

<<__EntryPoint>>
function main_782() {
t();
}

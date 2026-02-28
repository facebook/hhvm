<?hh

class foo {
  static function ioo($y, inout $x) :mixed{
    return new self(1);
  }
  function __construct($a, $b) {
}
}
function t() :mixed{
  $x = 1;
  $y = null;
  foo::ioo($x, inout $y);
}

<<__EntryPoint>>
function main_782() :mixed{
t();
}

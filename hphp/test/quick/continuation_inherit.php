<?hh

function derp() {
  include 'continuation_inherit.inc';
}

$rc = new ReflectionClass('Generator');
var_dump($rc->isFinal());

derp();

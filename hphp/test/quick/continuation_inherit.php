<?

function derp() {
  class HerpDerp extends Generator {}
}

$rc = new ReflectionClass('Generator');
var_dump($rc->isFinal());

derp();

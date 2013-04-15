<?

function derp() {
  class HerpDerp extends Continuation {}
}

$rc = new ReflectionClass('Continuation');
var_dump($rc->isFinal());

derp();

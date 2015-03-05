<?php ;
class X {
  private $m;
  function __construct($m) {
    $this->m = $m;
  }
}
function prof($what, $fn) {
  if ($what == 'exit' && $fn == 'main') throw new Exception("Surprise");
}
function main() {
  return new X(null);
}
try {
  fb_setprofile('prof');
  var_dump(main());
} catch (Exception $e) {
  var_dump($e->getMessage());
}

<?php

class X { private function go(&$x) {} }
class N extends X {
  function __call($x, $y) { echo "ok\n"; }
}

function main(X $y) {
  $asd = 'asd';
  return $y->go($asd);
}

main(new N);

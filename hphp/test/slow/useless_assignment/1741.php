<?php

function out($a) {
  echo $a,'\n';
}
function test($a) {
  $a ? out('?a') : out(':a');
  $a ? out('+a') : 0;
  $a ? 0 : out('-a');
  $a && out('&&a');
  $a || out('||a');
  $a and out('and a');
  $a or out('or a');
  $b = $c = 0;
  $a || (($b = 5) + ($c = 6));
  out($b);
 out($c);
}
test(0);
test('foo');

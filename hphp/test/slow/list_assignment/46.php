<?php

function test($a, $b, $i) {
  list($a[$i++], $a[$i++], $a[$i++]) = $b;
  var_dump($a);
  }

<<__EntryPoint>>
function main_46() {
test(array(), array('x', 'y', 'z'), 0);
}

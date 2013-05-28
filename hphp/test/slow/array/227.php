<?php

function gen() {
  $a = array('a' => 1, 'b' => 2);
  foreach ($a as $b => &$c) {
    yield null;
    var_dump($b);
    unset($a['b']);
  }
}
foreach (gen() as $_) {
}

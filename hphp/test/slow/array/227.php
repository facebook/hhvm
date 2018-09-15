<?php

function gen() {
  $a = array('a' => 1, 'b' => 2);
  foreach ($a as $b => &$c) {
    yield null;
    var_dump($b);
    unset($a['b']);
  }
}

<<__EntryPoint>>
function main_227() {
foreach (gen() as $_) {
}
}

<?php

function test($a = 10, ...$more_args) {
  $args = array_merge(array($a), $more_args);
  var_dump(count($args));
  var_dump($args);
}

 <<__EntryPoint>>
function main_20() {
  test();
  test(1);
  test(1, 2);
}

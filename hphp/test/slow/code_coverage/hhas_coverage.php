<?php

function test($a) {
  fb_enable_code_coverage();
  var_dump(array_map(null, $a));
  fb_disable_code_coverage();
}

test(array(1,2,3));

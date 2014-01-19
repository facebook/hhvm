<?php

function test($t) {
  fb_enable_code_coverage();
  var_dump($t);
  fb_disable_code_coverage();
}

test("hello");

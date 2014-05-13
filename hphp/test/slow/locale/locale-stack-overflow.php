<?php

function test($len) {
  return locale_get_display_name(str_repeat('*', $len), 'a');
}

test(254);
test(255);
test(256);
test(257);

<?php

test(LC_TIME, 'nl_NL');

// This will fail, but it shouldn't mess up the already set LC_TIME category
test(LC_NUMERIC, 'nl_NL;');

test(LC_NUMERIC, 'nl_NL');

function test($cat, $locale) {
  var_dump(setlocale($cat, $locale));
  echo sprintf("%.3f: ", 3.142) . strftime("%A %e %B %Y\n", mktime(0, 0, 0, 12, 22, 1978));
}


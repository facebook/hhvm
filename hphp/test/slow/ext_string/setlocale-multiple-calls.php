<?php

test(LC_TIME, 'fr_FR');

// This will fail, but it shouldn't mess up the already set LC_TIME category
test(LC_NUMERIC, 'fr_FR;');

test(LC_NUMERIC, 'fr_FR');

function test($cat, $locale) {
  var_dump(setlocale($cat, $locale));
  $out = sprintf("%.3f: ", 3.142) .
         strftime("%A %e %B %Y", mktime(0, 0, 0, 12, 22, 1978));
  echo preg_replace_callback('/([\x00-\x1F\x7F-\xFF]+)/',
    function ($match) { return urlencode($match[1]); }, $out);
  echo "\n";
}

<?hh

function test($cat, $locale) :mixed{
  var_dump(setlocale($cat, $locale));
  $out = sprintf("%.3f: ", 3.142) .
         strftime("%A %e %B %Y", mktime(0, 0, 0, 12, 22, 1978));
  $count = -1;
  echo preg_replace_callback('/([\x00-\x1F\x7F-\xFF]+)/',
    function ($match) { return urlencode($match[1]); }, $out,
    -1, inout $count);
  echo "\n";
}


<<__EntryPoint>>
function main_setlocale_multiple_calls() :mixed{
test(LC_TIME, 'fr_FR');

// This will fail, but it shouldn't mess up the already set LC_TIME category
test(LC_NUMERIC, 'fr_FR;');

test(LC_NUMERIC, 'fr_FR');
}

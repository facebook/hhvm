<?hh
class MyClass { public int $myint = 0; }

$a = array();
function doSomethin() {
  global $a;
  global $longstr;
  for ($j = 0; $j < 200; $j++) {
    $a[] = json_decode(json_encode($longstr));
  }
}
doSomethin();

$start_peak = memory_get_peak_usage(true);
$memlimit = 6 * 1024 * 1024;
ini_set('memory_limit', $memlimit);

echo "Start mem: $start_peak\n";

function mem_threshold_callback_as_func() {
  echo "Threshold crossed again, peak: ".memory_get_peak_usage(true)."\n";
  exit();
}

HH\set_mem_threshold_callback($memlimit / 2, ()==> {
  echo "Threshold crossed, current peak: ".memory_get_peak_usage(true)."\n";
  HH\set_mem_threshold_callback($memlimit * 7 / 10, 'mem_threshold_callback_as_func');
});

$longstr =
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890".
  "123456789012345678901234567890123456789012345678901234567890";

// We will OOM for sure if not for the exit()
for ($i = 0; $i < 10000; $i++) {
  doSomethin();
  $curr_mem = memory_get_peak_usage(true);
}

<?hh
class MyClass {
  public static AnyArray $a = vec[];
  public function doSomethin($str) :mixed{
    for ($j = 0; $j < 200; $j++) {
      self::$a[] = $str.$j;
    }
  }
}

function mem_threshold_callback_as_func2() :mixed{
  echo "Threshold crossed again (2), peak: ".memory_get_peak_usage(true)."\n";
}

function mem_threshold_callback_as_func() :mixed{
  echo "Threshold crossed again, peak: ".memory_get_peak_usage(true)."\n";
  HH\set_mem_threshold_callback(70 * 1024 * 1024, mem_threshold_callback_as_func2<>);
}


/* Stages in this test
 * Memory reaches 40 -> first callback, registering second callback as a function
 * Memory reaches 50 -> second callback, registering third callback
 * Memory reaches 60 -> changing loop behavior to allocate without function calls
 * Memory reaches 70 -> third callback
 * Memory reaches 80 -> loop over
 * Memory reaches 90 -> OOM Error (never happens)
 */
<<__EntryPoint>>
function main_mem_threshold() :mixed{
ini_set('memory_limit', 90 * 1024 * 1024);

HH\set_mem_threshold_callback(40 * 1024 * 1024, ()==> {
  echo "Threshold crossed, current peak: ".memory_get_peak_usage(true)."\n";
  HH\set_mem_threshold_callback(50 * 1024 * 1024, mem_threshold_callback_as_func<>);
});

$longstr = "123456789012345678901234567890123456789012345678901234567890";
$longstr .= $longstr;
$longstr .= $longstr;
$longstr .= $longstr;
$longstr .= $longstr;
$longstr .= $longstr;
$longstr .= $longstr;

$myclass = new MyClass();
$curr_mem = memory_get_peak_usage(true);
echo "Memory before loop: $curr_mem\n";
// We will OOM for sure if not for the exit()
for ($i = 0; $i < 1000000; $i++) {
  $curr_mem = memory_get_peak_usage(true);

  if ($curr_mem > 80 * 1024 * 1024) {
    echo "breaking...\n";
    break;
  } else if ($curr_mem > 60 * 1024 * 1024) {
    // Allocate without function entry/exit
    for ($j = 0; $j < 200; $j++) {
      MyClass::$a[] = $longstr.$j;
    }
  } else {
    // Allocate with function entry/exit
    $myclass->doSomethin($longstr);
  }
}
}

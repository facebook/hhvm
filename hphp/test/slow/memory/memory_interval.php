<?hh

class testobj {
  private $x;

  public function __construct($y) {
    $this->x = $y;
  }
}

function alloc_lots($x) :mixed{
  $v = dict[];
  for ($i = 0; $i < $x * 5000; ++$i) {
    $m = new testobj($x * $i);
    $v[] = $m;
  }

  memory_get_usage(); // force stats update

  return $v;
}


<<__EntryPoint>>
function main_memory_interval() :mixed{
memory_get_usage(); // force stats update

// drive memory up to its highest peak in the script
alloc_lots(10);

hphp_memory_start_interval();
$u1 = hphp_memory_get_interval_peak_usage(true);
alloc_lots(5);
$u2 = hphp_memory_get_interval_peak_usage(true);
hphp_memory_stop_interval();
$u3 = hphp_memory_get_interval_peak_usage(true);
$u4 = memory_get_peak_usage(true);

if ($u2 >= $u4) {
  var_dump("interval peak was greater than or equal to global peak!");
}

if ($u1 >= $u2) {
  var_dump("interval memory didn't increase after more allocations!");
}

if ($u3 > 0) {
  var_dump("interval memory wasn't 0 after stopping the interval!");
}

var_dump($u1);
var_dump($u2);
var_dump($u3);
var_dump($u4);
}

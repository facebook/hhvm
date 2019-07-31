<?hh

function get_count() {
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) {
    $count = 0;
    apc_store('count', $count);
  }
  return $count;
}

$mycount = get_count();

if ($mycount % 2 == 0) {
    require 'apc2.inc';
} else {
    require 'apc3.inc';
}


require 'apc1.inc';

echo "\nCount: $mycount\n";
apc_store('count', $mycount+1);

$a = A['x' => true];
var_dump($a['x']);

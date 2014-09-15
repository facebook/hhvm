<?hh

class MyClass {}

// Get class count before
$before = objprof_get_data();
$arr = array_filter(
  $before,
  function($obj) { return $obj["class"] == "MyClass"; }
);
$count_before = $arr
  ? array_pop($arr)["instances"]
  : 0;
$before_debug = var_export($before, true);

// Create 3 instances
$container = array();
for ($i = 0;$i < 1000; ++$i) {
  $container[] = new MyClass();
}

// Get class count after
$after = objprof_get_data();
$arr = array_filter(
  $after,
  function($obj) { return $obj["class"] == "MyClass"; }
);
$count_after = $arr
  ? array_pop($arr)["instances"]
  : 0;
$after_debug = var_export($after, true);

$diff = $count_after - $count_before;
$result = ($diff < 1100 && $diff > 900) ? "True" : "False";
echo "$result";

// Debugging in case something was wrong
if ($result !== "True") {
  echo "BEFORE: \n";
  echo "$before_debug \n";
  echo "AFTER: \n";
  echo "$after_debug \n";
}

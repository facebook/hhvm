<?php
set_error_handler(function($code, $message) {
  var_dump($code, $message);
});

$comparator= null;
$list= [1, 4, 2, 3, -1];
usort($list, function($a, $b) use ($comparator) {
  try {
	  return $comparator->compare($a, $b);
  } catch (Error $e) {
	  var_dump($e->getCode(), $e->getMessage());
	  return 0;
  }
});
var_dump($list);
echo "Alive\n";
?>

<?php

function method($cache) {
	  $prepared = clone $cache;
	  var_dump($prepared->data);
	  $prepared->data = "bad";
	  return $prepared;
}

$cache = new stdClass();
$cache->data = "good";

for ($i = 0; $i < 5; ++$i) {
	   method($cache);
}
?>

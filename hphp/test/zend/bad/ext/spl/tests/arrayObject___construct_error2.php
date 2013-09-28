<?php
echo "Too many arguments:\n";
Class C implements Iterator {
	function current() {}
	function next() {}
	function key() {}
	function valid() {}
	function rewind() {}
}

try {
  var_dump(new ArrayObject(new stdClass, 0, "C", "extra"));
} catch (InvalidArgumentException $e) {
  echo $e->getMessage() . "(" . $e->getLine() .  ")\n";
}
?>
<?php
print "Before error\n";
$result = eval("echo foo");
if ($result) {
	echo "eval returns true\n";
} else {
	echo "eval returns false\n";
}
?>

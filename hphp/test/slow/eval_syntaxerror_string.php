<?php
print "Before error\n";
$result = eval("echo foo");
if ($result === false) {
  echo "eval returns false\n";
} else {
  echo "eval does not return false\n";
}

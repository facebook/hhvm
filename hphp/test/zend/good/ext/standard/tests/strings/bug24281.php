<?php
$string = "He had had to have had it";
$newstring = str_replace("had", "foo", $string, $count);
print "$count changes were made.\n";
$count = "foo";
$newstring = str_replace("had", "foo", $string, $count);
print "$count changes were made.\n";
?>
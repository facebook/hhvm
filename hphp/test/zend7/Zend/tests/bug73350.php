<?php
$e = new Exception();

// This line cause problem :(
// Comment it to see the difference.
(string) $e;

// This line show the clue (PHP Warning: ...).
var_export($e);
?>

<?php
ini_set('output_buffering', 0);

echo "*** Testing ob_start() : closures as output handlers ***\n";

ob_start(function ($output) {
  return 'Output (1): ' . $output;
});

ob_start(function ($output) {
  return 'Output (2): ' . $output;
});

echo "Test\nWith newlines\n";

$str1 = ob_get_contents ();

ob_end_flush();

$str2 = ob_get_contents ();

ob_end_flush();

echo $str1, $str2;

?>
===DONE===
<?php
$x = array("test", "55");
global ${$x[0]};
${$x[0]} = $x[1];
echo "Test: $test\n";;
?>
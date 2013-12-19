<?php
$a = array(array());
$a[0][0] =& $a[0];
unset($a);
echo "ok\n"
?>
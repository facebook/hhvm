<?php
$a = new SplFixedArray(2);
$a[0] = "foo";
var_dump(empty($a[0]), empty($a[1]), $a);
?>
<?php
$obj = new stdClass();
$obj->prop = imagecreate(10, 10);
$str = var_export($obj, true);
echo $str, "\nDone\n";

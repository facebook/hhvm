<?php

<<__EntryPoint>>
function main_footer_bug_3() {
$obj = new stdClass();
$obj->prop = imagecreate(10, 10);
$str = var_export($obj, true);
echo $str, "\nDone\n";
}

<?php
$f = fopen(__FILE__, "r");
$dummy = var_export(stream_get_meta_data($f), TRUE);
echo "I'm alive!\n";
?>
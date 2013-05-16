<?php

$fp = fopen(__FILE__, "r");

var_dump(stream_get_meta_data($fp));

fclose($fp);

?>
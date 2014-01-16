<?php

$file = str_repeat("A", 1024);

var_dump(highlight_file($file, true));
var_dump(ob_get_contents());

?>
===DONE===
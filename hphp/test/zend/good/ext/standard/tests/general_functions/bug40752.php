<?php

$file = dirname(__FILE__)."/bug40752.ini";
file_put_contents($file, '
foo   = 1;
foo[] = 1;
');

var_dump(parse_ini_file($file));

file_put_contents($file, '
foo[] = 1;
foo   = 1;
');

var_dump(parse_ini_file($file));

unlink($file);

echo "Done\n";
?>
<?php

$file = dirname(__FILE__)."/bug41445.ini";

$data = <<<DATA
[001099030277]
option1 = yes

[011099030277]
option2 = yes
DATA;

file_put_contents($file, $data);

var_dump(parse_ini_file($file, TRUE));
var_dump(parse_ini_file($file));

$data = <<<DATA
[23.44]
option1 = yes

[9633337363542736472364]
option2 = yes
DATA;

file_put_contents($file, $data);

var_dump(parse_ini_file($file, TRUE));
var_dump(parse_ini_file($file));

@unlink($file);

echo "Done\n";
?>
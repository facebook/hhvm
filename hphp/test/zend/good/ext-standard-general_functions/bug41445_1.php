<?php

$file = dirname(__FILE__)."/bug41445_1.ini";

$data = <<<DATA
[2454.33]
09 = yes

[9876543]
098765434567876543 = yes

[09876543]
987654345678765432456798765434567876543 = yes
DATA;

file_put_contents($file, $data);

var_dump(parse_ini_file($file, TRUE));
var_dump(parse_ini_file($file));

@unlink($file);

echo "Done\n";
?>
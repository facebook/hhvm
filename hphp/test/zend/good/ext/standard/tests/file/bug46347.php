<?php

$str = <<< EOF
[section]
part1.*.part2 = 1
EOF;

$file = __DIR__ . '/bug46347.ini';
file_put_contents($file, $str);

var_dump(parse_ini_file($file));
?>
<?php error_reporting(0); ?>
<?php
unlink(__DIR__.'/bug46347.ini');
?>
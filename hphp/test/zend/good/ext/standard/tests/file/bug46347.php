<?php

$str = <<< EOF
[section]
part1.*.part2 = 1
EOF;

$file = __DIR__ . '/parse.ini';
file_put_contents($file, $str);

var_dump(parse_ini_file($file));
?>
<?php
unlink(__DIR__.'/parse.ini');
?>
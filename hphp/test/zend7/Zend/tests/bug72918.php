<?php
$array = [-3 => 'foo'];
$string = 'abcde';

echo "$array[-3]\n";
echo "$string[-3]\n";
echo <<<EOT
$array[-3]
$string[-3]

EOT;
?>
===DONE===

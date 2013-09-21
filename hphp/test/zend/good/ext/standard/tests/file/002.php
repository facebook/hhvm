<?php

$data = <<<EOD
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
blah blah blah blah blah blah blah
EOD;

$name = tempnam("./ext/standard/tests/file/", "php");
$fp = fopen($name, "w");
fwrite($fp, $data);
fclose($fp);

//readfile($name);
echo file_get_contents($name);

unlink($name);

?>
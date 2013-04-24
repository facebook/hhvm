<?php
$fp = fopen(dirname(__FILE__)."/test.csv", "r");
while($line = fgetcsv($fp, 24)) {
	$line = str_replace("\x0d\x0a", "\x0a", $line);
	var_dump($line);
}
fclose($fp);
?>
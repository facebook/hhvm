<?php
$fp = fopen(dirname(__FILE__)."/test2.csv", "r");
while(($line = fgetcsv($fp, 1024))) {
	var_dump($line);
}
fclose($fp);
?>
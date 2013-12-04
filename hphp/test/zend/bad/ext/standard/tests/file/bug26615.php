<?php
ini_set('variables_order', E);

$out = array();
$status = -1;
if (substr(PHP_OS, 0, 3) != 'WIN') {
	exec($_ENV['TEST_PHP_EXECUTABLE'].' -n -r \'for($i=1;$i<=5000;$i++) print "$i\n";\' | tr \'\n\' \' \'', $out, $status);
} else {
	exec($_ENV['TEST_PHP_EXECUTABLE'].' -n -r "for($i=1;$i<=5000;$i++) echo $i,\' \';"', $out, $status);
}
print_r($out);
?>
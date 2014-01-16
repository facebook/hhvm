<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar';
copy(dirname(__FILE__) . '/files/links.tar', $fname);
try {
	$p = new PharData($fname);
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
var_dump($p['testit/link']->getContent());
var_dump($p['testit/hard']->getContent());
var_dump($p['testit/file']->getContent());
$p['testit/link'] = 'overwriting';
var_dump($p['testit/link']->getContent());
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.tar');
?>
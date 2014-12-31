<?php

mkdir(dirname(__FILE__).'/testdir4');
foreach(range(1, 4) as $i) {
    file_put_contents(dirname(__FILE__)."/testdir4/file$i.txt", "some content for file $i");
}

try {
	$phar = new Phar(dirname(__FILE__) . '/buildfromdirectory4.phar');
	$a = $phar->buildFromDirectory(dirname(__FILE__) . '/testdir4');
	asort($a);
	var_dump($a);
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}

var_dump(file_exists(dirname(__FILE__) . '/buildfromdirectory4.phar'));

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromdirectory4.phar');
foreach(range(1, 4) as $i) {
    unlink(dirname(__FILE__) . "/testdir4/file$i.txt");
}
rmdir(dirname(__FILE__) . '/testdir4');
?>
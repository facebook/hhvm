<?php

mkdir(dirname(__FILE__).'/testdir6', 0777);
foreach(range(1, 4) as $i) {
    file_put_contents(dirname(__FILE__)."/testdir6/file$i.txt", "some content for file $i");
}

try {
	$phar = new Phar(dirname(__FILE__) . '/buildfromdirectory6.phar');
	var_dump($phar->buildFromDirectory(dirname(__FILE__) . '/testdir6', '/\.php$/'));
} catch (Exception $e) {
	var_dump(get_class($e));
	echo $e->getMessage() . "\n";
}

var_dump(file_exists(dirname(__FILE__) . '/buildfromdirectory6.phar'));

?>
===DONE===
<?php error_reporting(0); ?>
<?php 
unlink(dirname(__FILE__) . '/buildfromdirectory6.phar');
foreach(range(1, 4) as $i) {
    unlink(dirname(__FILE__) . "/testdir6/file$i.txt");
}
rmdir(dirname(__FILE__) . '/testdir6');
?>
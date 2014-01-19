<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$dirName = dirname(__FILE__);
$pname = 'phar://' . $fname;
$pArchive="DataArchive.phar";
$p = new Phar($fname, 0, $pArchive);
for ($i = 0; $i < 4*1024; $i++){
	echo("$i\n");
	if (!is_dir($fileDir="$dirName/test_data"))
	mkdir($fileDir, 0777, true);
	file_put_contents("$fileDir/$i", "");
	$p->addFile("$fileDir/$i", "$dirName");
} 	
echo("\n Written Files($i)\n");
?>
===DONE===
<?php 
$dirName = dirname(__FILE__);
$fileDir="$dirName/test_data";
for ($i = 0; $i < 4*1024; $i++){
	unlink("$fileDir/$i");
} 	
rmdir($fileDir);
unlink($dirName . '/' . basename(__FILE__, '.php') . '.phar.php');
__HALT_COMPILER();
?>
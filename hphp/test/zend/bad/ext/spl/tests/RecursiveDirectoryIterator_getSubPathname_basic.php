<?php
$depth0 = "depth0";
$depth1 = "depth1";
$depth2 = "depth2";
$targetDir = __DIR__ . DIRECTORY_SEPARATOR . $depth0 . DIRECTORY_SEPARATOR . $depth1 . DIRECTORY_SEPARATOR . $depth2;
mkdir($targetDir, 0777, true);
touch($targetDir . DIRECTORY_SEPARATOR . 'getSubPathname_test_2.tmp');
touch(__DIR__ . DIRECTORY_SEPARATOR . $depth0 . DIRECTORY_SEPARATOR . $depth1 . DIRECTORY_SEPARATOR . 'getSubPathname_test_3.tmp');
touch(__DIR__ . DIRECTORY_SEPARATOR . $depth0 . DIRECTORY_SEPARATOR . 'getSubPathname_test_1.tmp');
$iterator = new RecursiveDirectoryIterator(__DIR__ . DIRECTORY_SEPARATOR . $depth0);
$it = new RecursiveIteratorIterator($iterator);

$list = [];
$it->rewind(); //see https://bugs.php.net/bug.php?id=62914
while($it->valid()) {
  $list[] = $it->getSubPathname();
  $it->next();
}
asort($list);
foreach ($list as $item) {
	echo $item . "\n";
}
?>
<?php
function rrmdir($dir) {
	foreach(glob($dir . '/*') as $file) {
	  if(is_dir($file)) {
		rrmdir($file);
	  } else {
		unlink($file);
	  }
	}

	rmdir($dir);
}

$targetDir = __DIR__ . DIRECTORY_SEPARATOR . "depth0";
rrmdir($targetDir);
?>